#include <SPI.h>
#include <RH_RF95.h>

//for feather m0 RFM9x
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#if defined(ESP8266)
  /* for ESP w/featherwing */ 
  #define RFM95_CS  2    // "E"
  #define RFM95_RST 16   // "D"
  #define RFM95_INT 15   // "B"

#elif defined(ADAFRUIT_FEATHER_M0) || defined(ADAFRUIT_FEATHER_M0_EXPRESS) || defined(ARDUINO_SAMD_FEATHER_M0)
 // Feather M0 w/Radio
  #define RFM95_CS      8
  #define RFM95_INT     3
  #define RFM95_RST     4

#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2) || defined(ARDUINO_NRF52840_FEATHER) || defined(ARDUINO_NRF52840_FEATHER_SENSE)
  #define RFM95_INT     9  // "A"
  #define RFM95_CS      10  // "B"
  #define RFM95_RST     11  // "C"

#elif defined(ESP32)  
  /* ESP32 feather w/wing */
  #define RFM95_RST     27   // "A"
  #define RFM95_CS      33   // "B"
  #define RFM95_INT     12   //  next to A

#elif defined(ARDUINO_NRF52832_FEATHER)
  /* nRF52832 feather w/wing */
  #define RFM95_RST     7   // "A"
  #define RFM95_CS      11   // "B"
  #define RFM95_INT     31   // "C"
  #define LED           17
  
#elif defined(TEENSYDUINO)
  /* Teensy 3.x w/wing */
  #define RFM95_RST     9   // "A"
  #define RFM95_CS      10   // "B"
  #define RFM95_INT     4    // "C"
#endif


//Define frequency of LoRa to 868 MHz
#define RF95_FREQ 868.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Blinky on receipt
#define LED 13

//Set Mac address of receiver
String MAC_ADDRESS = "xx:xx:xx:xx:xx:xx";

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  
  Serial.begin(115200);
  delay(100);

  Serial.println("Feather LoRa RX!");

  // manual reset of LoRa module
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  //Set module to frequency 868 MHz
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Defaults after init are 868.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128); //Ok, fast but limited range
  //rf95.setModemConfig(RH_RF95::Bw31_25Cr48Sf512); //Failed
  //rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096); //ok, slow but long range
  //rf95.setModemConfig(RH_RF95::Bw125Cr45Sf2048);//Failed
  //Tx power can be set between 5 and 23
  rf95.setTxPower(23, false);
}

void loop()
{
  //Wait for a packet to receive
  if (rf95.available())
  {
    // Should be a message
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len))
    {
      //Get information from packet
      Serial.println("recv");
      String buf_string = (char*)buf;
      int i = buf_string.indexOf('-');
      String mac_address_received = buf_string.substring(0,i);
      String mac_address_server = buf_string.substring(i+1);
      Serial.println(mac_address_received);
      Serial.println(mac_address_server);
      //Check if the packed received is for us
      if(mac_address_received.equals(MAC_ADDRESS)){
        digitalWrite(LED, HIGH);
        Serial.print("Got: ");
        Serial.println(buf_string);
        Serial.print("RSSI: ");
        Serial.println(rf95.lastRssi(), DEC);
  
        // Send a reply to sender
        rf95.send((uint8_t *)mac_address_server.c_str(), mac_address_server.length()+1);
        rf95.waitPacketSent();
        Serial.println("Sent a reply");
        digitalWrite(LED, LOW);
      }
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
}
