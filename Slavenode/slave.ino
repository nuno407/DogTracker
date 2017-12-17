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

/**********************************************************/

/*
Librarys
*/

//esp32 to lora chip
#include <SPI.h>

//oled screen
#include <U8x8lib.h>

//GPS Library
#include <NMEAGPS.h> 

//LoRa chip include
#include <LoRa.h> //lora chip


/**********************************************************/

/*
Global variables
*/


//GPS
//Communication
HardwareSerial SerialGPS(1);
//GPS Module
NMEAGPS gps;
//Current gps fix
gps_fix currentFix;

//count sent messages
int counter = 0;

char chipid[10];  // 7 from + \0

// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

/**********************************************************/


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
  
  //init GPS serial with different port of UART2
  //17 rx 2 tx
  SerialGPS.begin(9600, SERIAL_8N1, 17, 2, false);
  //trying to connect to gps module
  while (!SerialGPS);
  
  //init PC communication
  Serial.begin(115200);
  while (!Serial); //if just the the basic function, must connect to a computer

  delay(500);

  u8x8.drawString(0, 0, "VictoryTracker");
  u8x8.drawString(0, 3, "An hunter dog...");
  u8x8.drawString(0, 6, "SLAVE NODE");

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    u8x8.drawString(0, 1, "Starting LoRa failed!");
    while (1);
  }

  delay(1000);
  u8x8.clear();

}



void loop() {

  //array used at various opt. act as buffer
  char stringinfo[BUFFERLENGHT];
  //buffer used by lora module
  char lorabuffer[BUFFERLENGHT];
  
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
