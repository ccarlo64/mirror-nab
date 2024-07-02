/* Copyright (C) 2021 Aran Vink. All rights reserved.

 This software may be distributed and modified under the terms of the GNU
 General Public License version 2 (GPL2) as published by the Free Software
 Foundation and appearing in the file GPL2.TXT included in the packaging of
 this file. Please note that GPL2 Section 2[b] requires that all works based
 on this software must also be made publicly available under the terms of
 the GPL2 ("Copyleft").

 Contact information
 -------------------

 Aran Vink
  e-mail   :  aranvink@gmail.com
 */

#include "MIRROR.h"
// To enable serial debugging see "settings.h"
//#define EXTRADEBUG // Uncomment to get even more debugging data

#define EP_MAXPKTSIZE 0x40
uint8_t readBuf[EP_MAXPKTSIZE]; // General purpose buffer for input data

MIRROR::MIRROR(USB *p) :
pUsb(p), // pointer to USB class instance - mandatory
bAddress(0) // device address - mandatory
{
Notify(PSTR("\r\n creata istanza "), 0x80);
        for(uint8_t i = 0; i < MIRROR_MAX_ENDPOINTS; i++) {
                epInfo[i].epAddr = i;
                epInfo[i].maxPktSize = (i) ? 0 : 8;
                epInfo[i].bmSndToggle = 0;
                epInfo[i].bmRcvToggle = 0;
                epInfo[i].bmNakPower = (i) ? USB_NAK_NOWAIT : USB_NAK_MAX_POWER;
        }

        if(pUsb) // register in USB subsystem
                pUsb->RegisterDeviceClass(this); //set devConfig[] entry
      Notify(PSTR("\r\n fine creazione"), 0x80);
}

uint8_t MIRROR::Init(uint8_t parent, uint8_t port, bool lowspeed) {
        uint8_t buf[sizeof (USB_DEVICE_DESCRIPTOR)];
       
        USB_DEVICE_DESCRIPTOR * udd = reinterpret_cast<USB_DEVICE_DESCRIPTOR*>(buf);
        uint8_t rcode;
        UsbDevice *p = NULL;
        EpInfo *oldep_ptr = NULL;
        uint16_t PID;
        uint16_t VID;
//Notify(PSTR("\r\nMIRROR I11111nit"), 0x80);
        // get memory address of USB device address pool
        AddressPool &addrPool = pUsb->GetAddressPool();
#ifdef EXTRADEBUG
        Notify(PSTR("\r\nMIRROR Init"), 0x80);
#endif
        // check if address has already been assigned to an instance
        if(bAddress) {
#ifdef DEBUG_USB_HOST
                Notify(PSTR("\r\nAddress in use"), 0x80);
#endif
                return USB_ERROR_CLASS_INSTANCE_ALREADY_IN_USE;
        }

        // Get pointer to pseudo device with address 0 assigned
        p = addrPool.GetUsbDevicePtr(0);

        if(!p) {
#ifdef DEBUG_USB_HOST
                Notify(PSTR("\r\nAddress not found"), 0x80);
#endif
                return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;
        }

        if(!p->epinfo) {
#ifdef DEBUG_USB_HOST
                Notify(PSTR("\r\nepinfo is null"), 0x80);
#endif
                return USB_ERROR_EPINFO_IS_NULL;
        }

        // Save old pointer to EP_RECORD of address 0
        oldep_ptr = p->epinfo;

        // Temporary assign new pointer to epInfo to p->epinfo in order to avoid toggle inconsistence
        p->epinfo = epInfo;

        p->lowspeed = lowspeed;

        // Get device descriptor
        rcode = pUsb->getDevDescr(0, 0, sizeof (USB_DEVICE_DESCRIPTOR), (uint8_t*)buf); // Get device descriptor - addr, ep, nbytes, data
        // Restore p->epinfo
        p->epinfo = oldep_ptr;

     //   if(rcode)
   //             goto FailGetDevDescr;

        VID = udd->idVendor;
        PID = udd->idProduct;
Notify(PSTR("\r\nVID PID "), 0x80);
        if(VID != MIRROR_VID || (PID != MIRROR_PID))
        {
        Notify(PSTR("\r\nNON TROVAT: "), 0x80);
                goto FailUnknownDevice;
                }

        // Allocate new address according to device class
        bAddress = addrPool.AllocAddress(parent, false, port);

        if(!bAddress)
                return USB_ERROR_OUT_OF_ADDRESS_SPACE_IN_POOL;

        // Extract Max Packet Size from device descriptor
        epInfo[0].maxPktSize = udd->bMaxPacketSize0;

        // Assign new address to the device
        rcode = pUsb->setAddr(0, 0, bAddress);
                        Notify(PSTR("\r\nsetAddr: "), 0x80);
                D_PrintHex<uint8_t > (rcode, 0x80);
        if(rcode) {
                p->lowspeed = false;
                addrPool.FreeAddress(bAddress);
                bAddress = 0;
#ifdef DEBUG_USB_HOST
                Notify(PSTR("\r\nsetAddr: "), 0x80);
                D_PrintHex<uint8_t > (rcode, 0x80);
#endif

                return rcode;
        }
#ifdef EXTRADEBUG
        Notify(PSTR("\r\nAddr: "), 0x80);
        D_PrintHex<uint8_t > (bAddress, 0x80);
#endif
        Notify(PSTR("\r\nAddr: "), 0x80);
        D_PrintHex<uint8_t > (bAddress, 0x80);
        p->lowspeed = false;

        //get pointer to assigned address record
        p = addrPool.GetUsbDevicePtr(bAddress);
        if(!p)
                return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;

        p->lowspeed = lowspeed;

        // Assign epInfo to epinfo pointer - only EP0 is known
        rcode = pUsb->setEpInfoEntry(bAddress, 1, epInfo);
        if(rcode)
                goto FailSetDevTblEntry;


        /* The application will work in reduced host mode, so we can save program and data
           memory space. After verifying the PID and VID we will use known values for the
           configuration values for device, interface, endpoints for the MIRROR Controller */

        /* Initialize data structures for endpoints of device */
        epInfo[ MIRROR_OUTPUT_PIPE ].epAddr = MIRROR_ENDPOINT_OUT; // MIRROR output endpoint
        epInfo[ MIRROR_OUTPUT_PIPE ].epAttribs = USB_TRANSFER_TYPE_INTERRUPT;
        epInfo[ MIRROR_OUTPUT_PIPE ].bmNakPower = USB_NAK_NOWAIT; // Only poll once for interrupt endpoints
        epInfo[ MIRROR_OUTPUT_PIPE ].maxPktSize = MIRROR_EP_MAXPKTSIZE;
        epInfo[ MIRROR_OUTPUT_PIPE ].bmSndToggle = 0;
        epInfo[ MIRROR_OUTPUT_PIPE ].bmRcvToggle = 0;
        
        /* Initialize data structures for endpoints of device */
        epInfo[ MIRROR_INPUT_PIPE ].epAddr = MIRROR_ENDPOINT_IN; // MIRROR output endpoint
        epInfo[ MIRROR_INPUT_PIPE ].epAttribs = USB_TRANSFER_TYPE_INTERRUPT;
        epInfo[ MIRROR_INPUT_PIPE ].bmNakPower = USB_NAK_NOWAIT; // Only poll once for interrupt endpoints
        epInfo[ MIRROR_INPUT_PIPE ].maxPktSize = MIRROR_EP_MAXPKTSIZE;
        epInfo[ MIRROR_INPUT_PIPE ].bmSndToggle = 0;
        epInfo[ MIRROR_INPUT_PIPE ].bmRcvToggle = 0;        

        rcode = pUsb->setEpInfoEntry(bAddress, 3, epInfo);
        if(rcode)
                goto FailSetDevTblEntry;

        delay(200); //Give time for address change



        //For some reason this is need to make it work
        rcode = pUsb->setConf(bAddress, epInfo[ MIRROR_CONTROL_PIPE ].epAddr, 1);
        if(rcode)
                goto FailSetConfDescr;

        if(PID == MIRROR_PID || PID) {
                MIRRORConnected = true;
        }
        onInit();
        Notify(PSTR("\r\nCONFIG OK: "), 0x80);
        
        Notify(PSTR("\r\n"), 0x80);
        
//PrintEndpointDescriptor(     &oldep_ptr);    
        return 0; // Successful configuration

        /* Diagnostic messages */
FailGetDevDescr:
#ifdef DEBUG_USB_HOST
        NotifyFailGetDevDescr();
        Notify(PSTR("\r\nerrore NotifyFailGetDevDescr"), 0x80);
        goto Fail;
#endif

FailSetDevTblEntry:
#ifdef DEBUG_USB_HOST
        NotifyFailSetDevTblEntry();
        Notify(PSTR("\r\nerrore NotifyFailSetDevTblEntry"), 0x80);
        goto Fail;
#endif

FailSetConfDescr:
#ifdef DEBUG_USB_HOST
        NotifyFailSetConfDescr();
        Notify(PSTR("\r\nerrore NotifyFailSetConfDescr"), 0x80);
#endif
        goto Fail;

FailUnknownDevice:
#ifdef DEBUG_USB_HOST
        NotifyFailUnknownDevice(VID, PID);
#endif
        rcode = USB_DEV_CONFIG_ERROR_DEVICE_NOT_SUPPORTED;

Fail:
#ifdef DEBUG_USB_HOST
        Notify(PSTR("\r\nMIRROR Init Failed, error code: "), 0x80);
        NotifyFail(rcode);
#endif
        Release();
        return rcode;
}

/* Performs a cleanup after failed Init() attempt */
uint8_t MIRROR::Release() {
        MIRRORConnected = false;
        pUsb->GetAddressPool().FreeAddress(bAddress);
        bAddress = 0;
        return 0;
}

uint8_t MIRROR::Poll() {
        return 0;
}


void MIRROR::Light_Command(uint8_t *data, uint16_t nbytes) {
        #ifdef DEBUG_USB_HOST
        Notify(PSTR("\r\nLight command "), 0x80);
        #endif
        pUsb->outTransfer(bAddress, epInfo[ MIRROR_OUTPUT_PIPE ].epAddr, nbytes, data);
}

/*
void MIRROR::setLight(uint8_t ambx_light, uint8_t r, uint8_t g, uint8_t b) {
        writeBuf[0] = MIRROR_PREFIX_COMMAND;
        writeBuf[1] = ambx_light;
        writeBuf[2] = MIRROR_SET_COLOR_COMMAND;
        writeBuf[3] = r;
        writeBuf[4] = g;
        writeBuf[5] = b;
        Light_Command(writeBuf, MIRROR_LIGHT_COMMAND_BUFFER_SIZE);
}

*/

uint8_t  MIRROR::getTag( uint8_t *rr ) {
	uint8_t rcode;
	uint8_t pollInterval;
	pollInterval=10;
	uint16_t aa=0x40;
        uint16_t BUFFER_SIZE = EP_MAXPKTSIZE;
        
	rcode =     pUsb->inTransfer(bAddress, epInfo[ MIRROR_INPUT_PIPE ].epAddr, &BUFFER_SIZE, rr,pollInterval); // input on endpoint 1

#ifdef PRINTREPORT
      //  printReport(BUFFER_SIZE); // Uncomment "#define PRINTREPORT" to print the report send by the Xbox controller
#endif

	if(rr[0]!=0) {

		Notify(PSTR("\r\nlen: "), 0x80);
		D_PrintHex<uint8_t > (BUFFER_SIZE, 0x80);
		Notify(PSTR("\r\nrcode: "), 0x80);
		D_PrintHex<uint8_t > (rcode, 0x80);
		Notify(PSTR("BUFFER: "), 0x80);

		for(int i = 0; i < 32; i++) {
			D_PrintHex<uint8_t > (rr[i], 0x80);
		}
	}




/*
if(rcode!=0) {
    aa=8;
    uint8_t msg1[10] = { 0x1F, 0x01, 0x25, 0x00, 0xC4, 0x00, 0x25, 0x03 };
    rcode = pUsb->outTransfer( bAddress, epInfo[ MIRROR_INPUT_PIPE ].epAddr, aa, msg1 );    
Notify(PSTR("\r\nrcode1: "), 0x80);
D_PrintHex<uint8_t > (rcode, 0x80);    
    uint8_t msg2[10] = { 0x00, 0x01, 0x25, 0x00, 0xC4, 0x00, 0x25, 0x04 }; 
    rcode = pUsb->outTransfer( bAddress, epInfo[ MIRROR_INPUT_PIPE ].epAddr, aa, msg2 );    
Notify(PSTR("\r\nrcode2: "), 0x80);
D_PrintHex<uint8_t > (rcode, 0x80);    
    delay(500);
    uint8_t msg3[] = { 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x25, 0x05 };
    rcode = pUsb->outTransfer( bAddress, epInfo[ MIRROR_INPUT_PIPE ].epAddr, aa, msg3 ); 
  Notify(PSTR("\r\nrcode3: "), 0x80);
  D_PrintHex<uint8_t > (rcode, 0x80);
  Notify(PSTR("\r\nOUT: "), 0x80);
  }
*/ 

	return rr[0];
	}
/*
        writeBuf[0] = MIRROR_PREFIX_COMMAND;
        writeBuf[1] = ambx_light;
        writeBuf[2] = MIRROR_SET_COLOR_COMMAND;
        writeBuf[3] = r;
        writeBuf[4] = g;
        writeBuf[5] = b;
  
        Notify(PSTR("\r\nlegge mirror "), 0x80);
  
        rcode = pUsb->inTransfer( bAddress, epInfo[ MIRROR_OUTPUT_PIPE ].epAddr, &aa, writeBuf );     
Notify(PSTR("\r\nBUFFER: "), 0x80);
D_PrintHex<uint8_t > (writeBuf[0], 0x80);
D_PrintHex<uint8_t > (writeBuf[1], 0x80);
D_PrintHex<uint8_t > (writeBuf[2], 0x80);
D_PrintHex<uint8_t > (writeBuf[3], 0x80);
D_PrintHex<uint8_t > (writeBuf[4], 0x80);
D_PrintHex<uint8_t > (writeBuf[5], 0x80);
D_PrintHex<uint8_t > (writeBuf[6], 0x80);
D_PrintHex<uint8_t > (writeBuf[7], 0x80);
Notify(PSTR("\r\nrcode di intrasfer: "), 0x80);
D_PrintHex<uint8_t > (rcode, 0x80);    
    if (writeBuf[8] ==0 ){ 
    aa=8;
    uint8_t msg1[10] = { 0x1F, 0x01, 0x25, 0x00, 0xC4, 0x00, 0x25, 0x03 };
    rcode = pUsb->outTransfer( bAddress, epInfo[ MIRROR_OUTPUT_PIPE ].epAddr, aa, msg1 );    
Notify(PSTR("\r\nrcode1: "), 0x80);
D_PrintHex<uint8_t > (rcode, 0x80);    
    uint8_t msg2[10] = { 0x00, 0x01, 0x25, 0x00, 0xC4, 0x00, 0x25, 0x04 }; 
    rcode = pUsb->outTransfer( bAddress, epInfo[ MIRROR_OUTPUT_PIPE ].epAddr, aa, msg2 );    
Notify(PSTR("\r\nrcode2: "), 0x80);
D_PrintHex<uint8_t > (rcode, 0x80);    
    delay(500);
    uint8_t msg3[] = { 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x25, 0x05 };
    rcode = pUsb->outTransfer( bAddress, epInfo[ MIRROR_OUTPUT_PIPE ].epAddr, aa, msg3 ); 
  Notify(PSTR("\r\nrcode3: "), 0x80);
  D_PrintHex<uint8_t > (rcode, 0x80);
  Notify(PSTR("\r\nOUT: "), 0x80);
    return; } // return if nothing happened




                Notify(PSTR("\r\ncodice: "), 0x80);
                D_PrintHex<uint8_t > (rcode, 0x80);         
//        Light_Command(writeBuf, MIRROR_LIGHT_COMMAND_BUFFER_SIZE);

}
*/
/*

void MIRROR::xxx(uint8_t buf[sizeof (USB_DEVICE_DESCRIPTOR)]) {
uint8_t rcode;
rcode = Usb.inTransfer( 0, 0, 0x40, buf );


}
*/
/*
void MIRROR::setLight(AmbxLightsEnum ambx_light, AmbxColorsEnum color) { // Use this to set the Light with Color using the predefined in "MIRROREnums.h"
        setLight(ambx_light, (uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t)(color));
}

void MIRROR::setAllLights(AmbxColorsEnum color) { // Use this to set the Color using the predefined colors in "MIRROREnums.h"
        setLight(Sidelight_left, (uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t)(color));
        setLight(Sidelight_right, (uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t)(color));
        setLight(Wallwasher_center, (uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t)(color));
        setLight(Wallwasher_left, (uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t)(color));
        setLight(Wallwasher_right, (uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t)(color));
}
*/
void MIRROR::onInit() {
        #ifdef DEBUG_USB_HOST
        Notify(PSTR("\r\nOnInit execute "), 0x80);
        #endif
        if(pFuncOnInit)
                pFuncOnInit(); // Call the user function
       }         
                
                
                

