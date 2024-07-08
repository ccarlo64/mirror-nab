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

int led_red = 7;
int led_gre = 5;
int led_yel = 4;
int pulsante = 6;
int stato_pulsante=0;
bool premuto=false;
int funzione=0;

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



boolean Confronta( char*a, char*b,byte k){  
 
     for (byte i=0;i<k;i++){
         if (a[i]!=b[i])
             return false; // ho trovato un carattere diverso
     }

     return true; //il ciclo for è terminato non è uscito dalla funzione quindi tutti i carattere sono uguali
 
}



void setup() {

  pinMode(pulsante, INPUT);  
  pinMode(led_red,OUTPUT);  
  pinMode(led_gre,OUTPUT);  
  pinMode(led_yel,OUTPUT);  
  
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); //halt
  }
  Serial.print(F("\r\nMIRROR USB Library Started"));

  lcd.init(); // initialize the lcd
  lcd.backlight();  
  lcd.clear();                 // clear display
  lcd.setCursor(0, 0);         // move cursor to   (0, 0)  
  lcd.print("Wait TAG...");
  
}

char a[]="00";
uint8_t GETSTR[64];
uint8_t nab_red[]={0x02,0x01,00,00,0x08,0xD0,0x02,0x1A,0x03,0x53,0x04,0x46,0xB7};
uint8_t nab_gre[]={0x02,0x01,00,00,0x08,0xD0,0x02,0x1A,0x03,0x52,0xC1,0x4B,0x57};
uint8_t nab_yel[]={0x02,0x01,00,00,0x08,0xD0,0x02,0x18,0xC1,0x09,0x0B,0x9F,0x90};
uint8_t ret;


// ***************
// LOOP
// ***************
void loop() {
  int i;
  Usb.Task();

    //Serial.print(F("\r\n*"));
    if (MIRROR.MIRRORConnected) {


      stato_pulsante = digitalRead(pulsante); 
      if(!premuto && stato_pulsante==1) {
          lcd.clear();                 // clear display
          lcd.setCursor(0, 0);         // move cursor to   (0, 0)          
          
          Serial.print("\r\nrequest");          
          lcd.print("request");
          switch (funzione) {
            case 0:
              Serial.print(" ID ");          
              lcd.print(" ID");
              GETSTR[0]= 0x01;
              GETSTR[1]= 0x01;
              GETSTR[2]= 0x00;
              GETSTR[3]= 0x00;
              GETSTR[4]= 0x00;
              MIRROR.SendCommand(GETSTR,64);
              delay(500);              
              break;
            case 1:
              Serial.print(" APP ");          
              lcd.print(" APP");
              GETSTR[0]= 0x04;
              GETSTR[1]= 0x04;
              GETSTR[2]= 0x00;
              GETSTR[3]= 0x00;
              GETSTR[4]= 0x00;
              MIRROR.SendCommand(GETSTR,64);
              delay(500);              
              break;
            case 2:
              Serial.print(" BOOT ");          
              lcd.print(" BOOT");
              GETSTR[0]= 0x04;
              GETSTR[1]= 0x06;
              GETSTR[2]= 0x00;
              GETSTR[3]= 0x00;
              GETSTR[4]= 0x00;
              MIRROR.SendCommand(GETSTR,64);
              delay(500);               
              break;         
            case 3:
              Serial.print(" CHOREODATA ");          
              lcd.print(" CHOREODATA");
              GETSTR[0]= 0x03;
              GETSTR[1]= 0x07;
              GETSTR[2]= 0x00;
              GETSTR[3]= 0x00;
              GETSTR[4]= 0x00;
              MIRROR.SendCommand(GETSTR,64);
              delay(500);                
              break;          
            case 4:
              Serial.print(" CHOREODATAset ");          
              lcd.print(" CHOREODATAset");
              GETSTR[0]= 0x03;
              GETSTR[1]= 0x0A;
              GETSTR[2]= 0x00;
              GETSTR[3]= 0x00;
              GETSTR[4]= 0x04;
              GETSTR[5]= 0x04;
              GETSTR[6]= 0x64;
              GETSTR[7]= 0x00;
              GETSTR[8]= 0xf8;
              //046400f8
              MIRROR.SendCommand(GETSTR,64);
              delay(500);                
              break;                           
            default:
              Serial.print(" nada ");          
              lcd.print(" nada");
              break;                  
          }

          premuto=true;
          funzione++;
          // change for testing...
          if(funzione>3) funzione=0;          
      }
      if(premuto && stato_pulsante==0) {
          lcd.clear();                 // clear display
          lcd.setCursor(0, 0);         // move cursor to   (0, 0)          
          
          Serial.print("\r\npop ");
          
          lcd.print("pop");         
        premuto=false;
      }
      
      ret = MIRROR.GetTag(buffer);
      if(ret!=0){

        
        // *******************
        // Request ID
        // *******************
        if(ret==0x01 && buffer[1]==0x02) {
          lcd.clear();                 // clear display
          lcd.setCursor(0, 0);         // move cursor to   (0, 0)          
          
          Serial.print("\r\nID ");
          lcd.print("ID:");
          lcd.setCursor(0, 1);
          for( i=0 ; i < buffer[4] ; i++ ) {      
            sprintf(a,"%.02X",buffer[i+5]&0xFF);
            Serial.print(a);Serial.print(" ");
            lcd.print(a);
          }
        }



        // *******************
        // Request APP
        // *******************
        if(ret==0x04 && buffer[1]==0x05) {
          lcd.clear();                 // clear display
          lcd.setCursor(0, 0);         // move cursor to   (0, 0)          
          
          Serial.print("\r\nAPP ");
          lcd.print("APP:");
          lcd.setCursor(0, 1);
          for( i=0 ; i < buffer[4] ; i++ ) {      
            sprintf(a,"%.02X",buffer[i+5]&0xFF);
            Serial.print(a);Serial.print(" ");
            lcd.print(a);
          }
        }


        // *******************
        // Request BOOT
        // *******************
        if(ret==0x04 && buffer[1]==0x07) {
          lcd.clear();                 // clear display
          lcd.setCursor(0, 0);         // move cursor to   (0, 0)          
          
          Serial.print("\r\nBOOT ");
          lcd.print("BOOT:");
          lcd.setCursor(0, 1);
          for( i=0 ; i < buffer[4] ; i++ ) {      
            sprintf(a,"%.02X",buffer[i+5]&0xFF);
            Serial.print(a);Serial.print(" ");
            lcd.print(a);
          }
        }


        // *******************
        // Request CHOREODATA
        // *******************
        if(ret==0x03 && buffer[1]==0x08) {
          lcd.clear();                 // clear display
          lcd.setCursor(0, 0);         // move cursor to   (0, 0)          
          
          Serial.print("\r\nBCHOREODATA ");
          lcd.print("CHOREODATA:");
          lcd.setCursor(0, 1);
          for( i=0 ; i < buffer[4] ; i++ ) {      
            sprintf(a,"%.02X",buffer[i+5]&0xFF);
            Serial.print(a);Serial.print(" ");
            lcd.print(a);
          }
        }

        // *******************
        // Request orientation
        // *******************
        if(ret==0x01 && buffer[1]==0x05) {
          lcd.clear();                 // clear display
          lcd.setCursor(0, 0);         // move cursor to   (0, 0)                   
          lcd.print("DOWN..");
        }
        if(ret==0x01 && buffer[1]==0x04) {
          lcd.clear();                 // clear display
          lcd.setCursor(0, 0);         // move cursor to   (0, 0)                   
          lcd.print("UP..");
        }




        // *******************
        // Request TAG
        // *******************        
        if(ret==0x02) {
          lcd.clear();                 // clear display
          lcd.setCursor(0, 0);         // move cursor to   (0, 0)                   
          if(buffer[1]==0x01) {          
            lcd.print("tag on..");
            lcd.setCursor(0, 1);         // move cursor to   (0, 0)
          }
        
        
          if(buffer[1]==0x02) {
            lcd.print("tag off..");
            lcd.setCursor(0, 1);         // move cursor to   (0, 0)
          }
               
        for( i=0 ; i < buffer[4] ; i++ ) {
          sprintf(a,"%.02X",buffer[i+5]&0xFF);
          Serial.print(a);
          Serial.print(" ");
          lcd.print(a);
        }

        // ********
        // controllo tag
        // ********
// buffer contiene il tag
    if(memcmp(&buffer[0],nab_red,5+buffer[4])==0) {
    //if(Confronta(&buffer[0],nab_red,5+buffer[4])) {
      Serial.print("TROVATO"); 
      digitalWrite(led_red, HIGH);
    }else{
      digitalWrite(led_red, LOW);
    }
    if(memcmp(&buffer[0],nab_gre,5+buffer[4])==0) {
    //if(Confronta(&buffer[0],nab_red,5+buffer[4])) {
      Serial.print("TROVATO"); 
      digitalWrite(led_gre, HIGH);
    }else{
      digitalWrite(led_gre, LOW);
    }
    if(memcmp(&buffer[0],nab_yel,5+buffer[4])==0) {
    //if(Confronta(&buffer[0],nab_red,5+buffer[4])) {
      Serial.print("TROVATO"); 
      digitalWrite(led_yel, HIGH);
    }else{
      digitalWrite(led_yel, LOW);
    }
      }
    /*
  lcd.clear();                 // clear display
  lcd.setCursor(0, 0);         // move cursor to   (0, 0)
  lcd.print("Arduino");        // print message at (0, 0)

*/
/*
    if(ver==0) {
      ver++;
    //delay(500);
      GETIDSTR[0]= 0x01;
      GETIDSTR[1]= 0x01;
      GETIDSTR[2]= 0x00;
      GETIDSTR[3]= 0x00;
      GETIDSTR[4]= 0x00;
      
      MIRROR.SendCommand(GETIDSTR,64);
    delay(500);
      
    }
    
    if(ver==5) {
      ver++;
    //delay(500);
      
      GETAPPSTR[0]= 0x03;
      GETAPPSTR[1]= 0x07;
      GETAPPSTR[2]= 0x00;
      GETAPPSTR[3]= 0x00;
      GETAPPSTR[4]= 0x00;
      
      MIRROR.SendCommand(GETAPPSTR,64);
    delay(500);
      
    }
      */
    }
    
}
}


 
