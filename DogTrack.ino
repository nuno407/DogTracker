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


//mode of device 0 to dog 1 to master node
//#define mode 0

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
//rxPin = 9    //CONFLICT BETWEEN RESET OF OLED 
//txPin = 10
HardwareSerial SerialGPS(1);
//GPS Module
NMEAGPS gps;
//Current gps fix
gps_fix currentFix;


String receivedText;
String receivedRssi;
char stringinfo[128]  = "";


// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

void setup() {

  //init GPS serial
  SerialGPS.begin(9600);
  //trying to connect to gps module
  while (!SerialGPS);

  //init PC communication
  Serial.begin(115200);
  while (!Serial); //if just the the basic function, must connect to a computer

  
  //init LoRa chip and communication
  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);

  
  delay(500);

  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  Serial.println("LoRa Receiver");
  u8x8.drawString(0, 1, "LoRa Receiver");

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    u8x8.drawString(0, 1, "Starting LoRa failed!");
    while (1);
  }

  Blynk.begin(auth);
}

void loop() {

  Blynk.run();
  
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
    u8x8.drawString(0, 4, "PacketID");

    // read packet
    while (LoRa.available()) {
      receivedText = (char)LoRa.read();
      Serial.print(receivedText);
      char currentid[64];
      receivedText.toCharArray(currentid, 64);
      u8x8.drawString(9, 4, currentid);
      blynkLCD.print(0, 0, receivedText);
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
    u8x8.drawString(0, 5, "PacketRS");
    receivedRssi = LoRa.packetRssi();
    char currentrs[64];
    receivedRssi.toCharArray(currentrs, 64);
    u8x8.drawString(9, 5, currentrs);
  }

  while (gps.available(SerialGPS)){
    currentFix = gps.read();
    sprintf (stringinfo, "long: %c, lat: %c, spd: %d", currentFix.longitude(), currentFix.latitude(), currentFix.speed());currentFix.speed();
    Serial.println(stringinfo);
  }

  
}
