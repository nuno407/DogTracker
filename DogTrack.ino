//mode 0 dog    1 master
#define master 0

#define BUFFERLENGHT 128

#include <SPI.h> //esp32 to lora chip
#include <U8x8lib.h> //oled screen
#include <NMEAGPS.h> //GPS Library


//LoRa chip include
#include <LoRa.h> //lora chip
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


//phone to device
#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT
#include <BlynkSimpleEsp32_BLE.h>
#include <BLEDevice.h>
#include <BLEServer.h>


//Global variables

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "6e21948c963a422293d7b1c2fc4b2f2a";


//lcd blynk
WidgetLCD blynkLCD(V0);

//GPS
//Communication
//rxPin = 16    //CONFLICT BETWEEN RESET OF OLED 
//txPin = 17
HardwareSerial SerialGPS(1);
//GPS Module
NMEAGPS gps;
//Current gps fix
gps_fix currentFix;

int counter = 0;


String receivedText;
String receivedRssi;
char stringinfo[BUFFERLENGHT];
char lorabuffer[BUFFERLENGHT];
char chipid[10];  // 7 from + \0

// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

void setup() {

  //node identification
  sprintf(chipid, "%X", ESP.getEfuseMac());//The chip ID is essentially its MAC address(length: 6 bytes).


  //init LoRa chip and communication
  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);

  //int lcd
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  
  
  if(!master){
    //Child node code
    
    //init GPS serial
    //SerialGPS.begin(9600);
    //4 rx 2 tx
    SerialGPS.begin(9600, SERIAL_8N1, 17, 2, false);
    //trying to connect to gps module
    while (!SerialGPS);
    
  }else{
    //Master node code
    Blynk.begin(auth);
  }
  
  //init PC communication
  Serial.begin(115200);
  while (!Serial); //if just the the basic function, must connect to a computer

  delay(500);

  u8x8.drawString(0, 0, "VictoryTracker");
  u8x8.drawString(0, 3, "An hunter dog...");
  if(master){
    u8x8.drawString(0, 6, "MASTER NODE");
  }else{
    u8x8.drawString(0, 6, "SLAVE NODE");
  }

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    u8x8.drawString(0, 1, "Starting LoRa failed!");
    while (1);
  }

  delay(1000);
  u8x8.clear();

}

void loop() {

  if(master){
    Blynk.run();
    // try to parse packet
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      // received a packet
      u8x8.drawString(0, 0, "Received packet:");
  
      // read packet
      receivedText = LoRa.readString();
      receivedText.toCharArray(stringinfo, BUFFERLENGHT);
      Serial.print(receivedText);

      //parse msg
      receivedText.toCharArray(stringinfo, BUFFERLENGHT);
      char *part;
      char received_chipid[10];
      float received_lon;
      float received_lat;
      part = strtok (stringinfo,",");
      sprintf(received_chipid, "%s", part);
      part = strtok (NULL, ",");
      received_lon = atof(part);
      part = strtok (NULL, ",");
      received_lat = atof(part);

      //send to monitor
      sprintf (stringinfo, "Rlon: %f", received_lon);
      u8x8.drawString(0, 1, stringinfo);
      sprintf (stringinfo, "Rlat: %f", received_lat);
      u8x8.drawString(0, 2, stringinfo);
      sprintf (stringinfo, "RID: %s", received_chipid);
      u8x8.drawString(0, 3, stringinfo);

      //send to pc
      sprintf (stringinfo, "ID: %s, lon: %f, lat: %f", received_chipid, received_lon, received_lat);
      Serial.println(stringinfo);

      counter++;
      sprintf (stringinfo, "Ctr: %d", counter);
      u8x8.drawString(0, 5, stringinfo);

      //Send to blynk
      //blynkLCD.print(0, 0, );
    } 
  }else{
    while (gps.available(SerialGPS)){
      currentFix = gps.read();
      sprintf (stringinfo, "lon: %f, lat: %f, spd: %f", currentFix.longitude(), currentFix.latitude(), currentFix.speed());
      Serial.println(stringinfo);
      sprintf (stringinfo, "lon: %f", currentFix.longitude());
      u8x8.drawString(0, 0, stringinfo);
      sprintf (stringinfo, "lat: %f", currentFix.latitude());
      u8x8.drawString(0, 1, stringinfo);
      sprintf (stringinfo, "spd: %f", currentFix.speed());
      u8x8.drawString(0, 2, stringinfo);

      //send lora packet to master node
      sprintf (lorabuffer, "%s,%f,%f",chipid, currentFix.longitude(), currentFix.latitude());
      LoRa.beginPacket();
      LoRa.print(lorabuffer);
      LoRa.endPacket();
      Serial.println(chipid);
      u8x8.drawString(0, 3, chipid);
      
      counter++;
      sprintf (stringinfo, "Ctr: %d", counter);
      u8x8.drawString(0, 5, stringinfo);
    }
  }
  
}
