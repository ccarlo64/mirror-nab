/*
 Example sketch for the MIRROR library - developed by Aran Vink <aranvink@gmail.com>
 */
#define DEBUG_USB_HOST 1
#define EXTRADEBUG 1

#define DEBUG_USB_HOST 1


#include <MIRROR.h>

#define MIRROR_ADDR        1
#define MIRROR_EP          0 //1
#define MIRROR_IF          0
char buf[ 0x40 ] = { 0 };      //Violet Mirror buffer
// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows

uint8_t buffer[64];
USB Usb;
MIRROR MIRROR(&Usb); // This will just create the instance  

uint8_t state = 0;
const long interval = 1000;
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); //halt
  }
  Serial.print(F("\r\nMIRROR USB Library Started"));
  //MIRROR.Init();

  lcd.init(); // initialize the lcd
  lcd.backlight();  
  lcd.clear();                 // clear display
  lcd.setCursor(0, 0);         // move cursor to   (0, 0)  
  lcd.print("Wait TAG...");
}
char a[]="00";
void loop() {
  int i;
  Usb.Task();
    //Serial.print(F("\r\n*"));
    if (MIRROR.MIRRORConnected) {
      //Serial.print(F("- "));
      MIRROR.getTag(buffer);
      if(buffer[0]!=0){
        lcd.clear();                 // clear display
        lcd.setCursor(0, 0);         // move cursor to   (0, 0)
        if(buffer[0]==0x01 && buffer[1]==0x05) {
          lcd.print("DOWN..");
        }
        if(buffer[0]==0x01 && buffer[1]==0x04) {
          lcd.print("UP..");
        }
        if(buffer[0]==0x02) {
          if(buffer[1]==0x01) {
            lcd.print("tag on..");
            lcd.setCursor(0, 1);         // move cursor to   (0, 0)
          }
        
        
          if(buffer[1]==0x02) {
            lcd.print("tag off..");
            lcd.setCursor(0, 1);         // move cursor to   (0, 0)
          }
               
        for( i=5 ; i < 0x10 ; i++ ) {
//if( buffer[ i ] == 0 ) { break; } //end of non-empty space         
//    if ( buf[i]<0x10 & buf[i] > 0){
//   Serial.print("0"); lcd.print("0");        // print message at (0, 0)     
// }
       sprintf(a,"%.02X",buffer[i]&0xFF);
       Serial.print(a);Serial.print(" ");
       //Serial.print((buffer[i]&0xFF),HEX);//Serial.print(" ");
        //lcd.print((buffer[i]&0xFF),HEX);        // print message at (0, 0)     
        lcd.print(a);
    }//for
      }
      }
    /*
  lcd.clear();                 // clear display
  lcd.setCursor(0, 0);         // move cursor to   (0, 0)
  lcd.print("Arduino");        // print message at (0, 0)

*/

/*
uint8_t i;
uint8_t rcode;
uint16_t read = 0x40;
uint8_t aa=1;

//rcode = Usb.inTransfer( 0, 0, 0x40, buf );
rcode = Usb.inTransfer(   aa, aa , &read, buf );// MIRROR_ADDR, MIRROR_EP
//rcode = InTransfer(pep, nak_limit, &read, dataptr);
Serial.print(F("\r\nRCODE:"));
Serial.print(rcode,HEX);

    if (buf[0] ==0 ){ 
    char msg1[] = { 0x1F, 0x01, 0x25, 0x00, 0xC4, 0x00, 0x25, 0x03 };
    rcode = Usb.outTransfer( MIRROR_ADDR, MIRROR_EP, 0x08, msg1 );    
    char msg2[] = { 0x00, 0x01, 0x25, 0x00, 0xC4, 0x00, 0x25, 0x04 }; 
    rcode = Usb.outTransfer( MIRROR_ADDR, MIRROR_EP, 0x08, msg2 );     
    delay(500);
    char msg3[] = { 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x25, 0x05 };
    rcode = Usb.outTransfer( MIRROR_ADDR, MIRROR_EP, 0x08, msg3 ); 
    Serial.print(".");   
     } // return if nothing happened
     else {
    Serial.print(buf[0],HEX);
    Serial.print(buf[1],HEX);Serial.print(" ");// ==1 for placed on ==2 for taken off
    Serial.print(buf[4],HEX);Serial.print(" ");// == no of bytes in label 4 or 8
    for( i=5 ; i < 0x40 ; i++ ) {
        if( buf[ i ] == 0 ) { break; } //end of non-empty space
    if ( buf[i]<0x10 & buf[i] > 0){Serial.print("0");}
    Serial.print((buf[i]&0xFF),HEX);Serial.print(" ");
    }
    Serial.println();
     }

*/



/*
if (buf[i] ==0 ){ 
  Serial.print(F("\r\nZERO"));
}

Serial.print(rcode,HEX);
Serial.print(F("\r\n*"));

Serial.print(buf[0],HEX);
*/
/*
    Serial.print(buf[1],HEX);Serial.print(" ");// ==1 for placed on ==2 for taken off
    Serial.print(buf[4],HEX);Serial.print(" ");// == no of bytes in label 4 or 8
    for( i=5 ; i < 0x10 ; i++ ) {
        if( buf[ i ] == 0 ) { break; } //end of non-empty space
    if ( buf[i]<0x10 & buf[i] > 0){Serial.print("0");}
    Serial.print((buf[i]&0xFF),HEX);Serial.print(" ");
  */  
      /*
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        
        if (state > 4) {
          state = 0;
        }
        if (state == 0) {
          Serial.print(F("\r\nRed"));
          MIRROR.setAllLights(Red);
        } else if (state == 1) {
          Serial.print(F("\r\nGreen"));
          MIRROR.setAllLights(Green);
        } else if (state == 2) {
          Serial.print(F("\r\nBlue"));
          MIRROR.setAllLights(Blue);
        } else if (state == 3) {
          Serial.print(F("\r\nWhite"));
          MIRROR.setAllLights(White);
        }*/
        //state++;
      //}
    }
    
  //Example using single light:
  //MIRROR.setLight(Wallwasher_center, White);
}
