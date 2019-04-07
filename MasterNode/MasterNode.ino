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

#define SERVICE_UUID        "e0d59370-7344-4f18-9465-ca1d13de5b44"
#define CHARACTERISTIC_UUID "092df0bc-34b3-485d-a44b-85f08596ac2b"

/**********************************************************/

/*
Librarys
*/

// BLE Server
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

//esp32 to lora chip
#include <SPI.h>

//oled screen
#include <U8x8lib.h>

//LoRa chip include
#include <LoRa.h> //lora chip

/**********************************************************/

/*
Global variables
*/

char chipid[10];  // 7 from + \0

// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

// Task for each processor
TaskHandle_t LoraHandling;
TaskHandle_t BLEServer;


//last received longitude
float last_lon;
//last received latitude
float last_lat;


/**********************************************************/

void BLECode( void * pvParameters ){
  for(;;){
    delay(1);
  }

}

void LoraCode( void * pvParameters ){
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
  
  Serial.print("LoraCode running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    delay(1);
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
  
        u8x8.drawString(0, 5, stringinfo);

        last_lon = received_lon;
        last_lat = received_lat;
      
      }else{
        u8x8.clear();
        u8x8.drawString(0, 0, "ERRORinrcvmsg!!!");
      }
    } 
  }
}


/*
Setup
*/

void setup() {

  BLEDevice::init("MyESP32");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue("Hello World says Neil");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  //node identification
  sprintf(chipid, "%X", ESP.getEfuseMac());//The chip ID is essentially its MAC address(length: 6 bytes).

  //init LoRa chip and communication
  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);
  LoRa.enableCrc();

  //int lcd
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  
  //init PC communication
  Serial.begin(115200);
  while (!Serial); //if just the the basic function, must connect to a computer

  delay(500);

  u8x8.drawString(0, 0, "VictoriaTracker");
  u8x8.drawString(0, 3, "An hunter dog...");
  u8x8.drawString(0, 6, "MASTER NODE");

  

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    u8x8.drawString(0, 1, "Starting LoRa failed!");
    while (1);
  }

  delay(1000);
  u8x8.clear();

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    LoraCode,   /* Task function. */
                    "LoraCodeTask",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &LoraHandling,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  
  xTaskCreatePinnedToCore(
                    BLECode,   /* Task function. */
                    "BLECodeTask",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &BLEServer,      /* Task handle to keep track of created task */
                    1);

}


/*
Main loop
*/

void loop() {

}
