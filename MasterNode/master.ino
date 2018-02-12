//CONFIG:   "C_Cpp.intelliSenseEngine": "Default" 

/*
Defines
*/
//define of buffer length 
#define BUFFERLENGHT 128

//define of lora pins
// WIFI_LoRa_32 ports
// GPIO5  -- SX1278's SCK
// GPIO19 -- SX1278's MISO
// GPIO27 -- SX1278's MOSI
// GPIO18 -- SX1278's CS
// GPIO14 -- SX1278's RESET
// GPIO26 -- SX1278's IRQ(Interrupt Request)
#define SS      18
#define RST     14
#define DI0     26
#define BAND    433E6

//Defines for blynk communication
#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT
/**********************************************************/

/*
Librarys
*/

//esp32 to lora chip
#include <SPI.h>

//oled screen
#include <U8x8lib.h>

//LoRa chip include
#include <LoRa.h> //lora chip

//Blynk module
#include <BlynkSimpleEsp32_BLE.h>
#include <BLEDevice.h>
#include <BLEServer.h>


/**********************************************************/

/*
Global variables
*/

//Blynk token
char auth[] = "6e21948c963a422293d7b1c2fc4b2f2a";

//Blynk lcd
WidgetMap PhoneMap(V1);

//count receive messages
int counter = 0;

char chipid[10];  // 7 from + \0

// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

/**********************************************************/


/*
Setup
*/

void setup() {

  //node identification
  sprintf(chipid, "%X", ESP.getEfuseMac());//The chip ID is essentially its MAC address(length: 6 bytes).

  //init LoRa chip and communication
  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);
  LoRa.enableCrc();

  //int lcd
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  
  //Master node code
  Blynk.begin(auth);
  
  //init PC communication
  Serial.begin(115200);
  while (!Serial); //if just the the basic function, must connect to a computer

  delay(500);

  u8x8.drawString(0, 0, "VictoryTracker");
  u8x8.drawString(0, 3, "An hunter dog...");
  u8x8.drawString(0, 6, "MASTER NODE");

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    u8x8.drawString(0, 1, "Starting LoRa failed!");
    while (1);
  }

  delay(1000);
  u8x8.clear();

}

/*
Main loop
*/

void loop() {

  //last received chipid
  char received_chipid[10];
  //last received longitude
  float received_lon;
  //last received latitude
  float received_lat;

  //information received by lora module
  String receivedText;
  //array used at various opt. act as buffer
  char stringinfo[BUFFERLENGHT];
  
    Blynk.run();
    // try to parse packet
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      // received a packet
      u8x8.drawString(0, 0, "Received packet:");
  
      // read packet
      receivedText = LoRa.readString();
      receivedText.toCharArray(stringinfo, BUFFERLENGHT);
      Serial.println(receivedText);

      //parse msg
      receivedText.toCharArray(stringinfo, BUFFERLENGHT);
      
      if(sscanf(stringinfo, "%[^,],%f,%f",received_chipid, &received_lon, &received_lat)==3){
        
        //send to lcd
        sprintf (stringinfo, "Rlon: %f", received_lon);
        u8x8.drawString(0, 1, stringinfo);
        sprintf (stringinfo, "Rlat: %f", received_lat);
        u8x8.drawString(0, 2, stringinfo);
        sprintf (stringinfo, "RID: %s", received_chipid);
        u8x8.drawString(0, 3, stringinfo);
        sprintf (stringinfo, "RSSI: %d", LoRa.packetRssi());
        u8x8.drawString(0, 4, stringinfo);
        
        //send to pc
        sprintf (stringinfo, "ID: %s, lon: %f, lat: %f", received_chipid, received_lon, received_lat);
        Serial.println(stringinfo);
  
        counter++;
        sprintf (stringinfo, "Ctr: %d", counter);
        u8x8.drawString(0, 5, stringinfo);
  
        //Send to blynk
        PhoneMap.location(1, received_lat, received_lon, "Vitoria");
      
      }else{
        u8x8.clear();
        u8x8.drawString(0, 0, "ERRORinrcvmsg!!!");
      }
    } 
}
