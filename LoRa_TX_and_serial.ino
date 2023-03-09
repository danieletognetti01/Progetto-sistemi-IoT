#include <SPI.h>
#include <RH_RF95.h>

//Define mac address of sender
#define MAC_ADDRESS "xx:xx:xx:xx:xx:xx"

//Set pin for the LoRa module
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
  
#elif defined(TEENSYDUINO)
  /* Teensy 3.x w/wing */
  #define RFM95_RST     9   // "A"
  #define RFM95_CS      10   // "B"
  #define RFM95_INT     4    // "C"
#endif

// Define LoRa frequency 868 MHz
#define RF95_FREQ 868.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() 
{
  
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  Serial1.begin(115200);
  delay(1000);

  Serial.println("Feather LoRa TX and serial!");
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
  rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128); //Ok, very fastbut short range
  //rf95.setModemConfig(RH_RF95::Bw31_25Cr48Sf512); //Failed
  //rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096);//Ok, slow but long range
  //rf95.setModemConfig(RH_RF95::Bw125Cr45Sf2048);//Failed
  //Tx power can be set between 5 and 23
  rf95.setTxPower(23, false);
}

void loop()
{
  String command = "";
  int num_pack = 0;
  String mac_address;
  //Waiting from serial for a send command 
  if(Serial1.available()){
     //Get mac address of destination and the number of packet to send
     command = Serial1.readStringUntil('\n');
     int i = command.indexOf(' ');
     num_pack = command.substring(0,i).toInt();
     String mando = "";
     String num_pack_s = String(num_pack);
     String separator = "-";
     mac_address = command.substring(i+1);
     mac_address = mac_address + "-" + MAC_ADDRESS;
     mando = num_pack_s + separator + mac_address;
     Serial.println("Command received:");
     Serial.println(mando); 
  }
  if(num_pack != 0){
    long time_tot = 0;
    int num_packet = num_pack;
    int count = 0;
    Serial.println(num_packet);
    //Create buffer to send
    char Buf[250];
    //For each packet to send
    for(int i=0; i<num_packet; i++){
      delay(1000); 
      Serial.println("Transmitting..."); 
      //Copy mac address of destination to buffer
      mac_address.toCharArray(Buf, 250);
      Serial.print("Sending "); Serial.println(Buf);
      
      Serial.println("Sending...");
      delay(10);
      rf95.send((uint8_t *)Buf, sizeof(Buf));
    
      Serial.println("Waiting for packet to complete..."); 
      delay(10);
      rf95.waitPacketSent();
      long time_s = millis();
      // Wait for a reply
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);
      Serial.println("Waiting for reply...");
      if (rf95.waitAvailableTimeout(10000))
      { 
        long time_f = millis();
        time_tot += (time_f - time_s);
        // Should be a reply message for us now   
        if (rf95.recv(buf, &len))
       {
        
        String buf_str = (char*)buf;
        Serial.print("Dimensione pacchetto ricevuto"); Serial.println(sizeof(buf));
        if(buf_str.equals(MAC_ADDRESS)){
          Serial.print("Got reply: ");
          Serial.println((char*)buf);
          Serial.print("RSSI: ");
          Serial.println(rf95.lastRssi(), DEC);
          count++;
        }
       }
        else
        {
          Serial.println("Receive failed");
        }
      }
      else
      {
        Serial.println("No reply, is there a listener around?");
      }
    }
    //Create a message with informations of the test
    String mando = "";
    String count_s = String(count);
    String separator = "#";
    String time_s = String((time_tot/count));
    String separator_2 = "_";
    int convertdata = static_cast<int>(sizeof(Buf));
    Serial.println(convertdata);
    String throughput_s = String((convertdata*1000*8)/(time_tot/count));
    mando = count_s + separator + time_s + separator_2 + throughput_s;
    //Send informations to serial port
    Serial1.print(mando); 
    Serial.println(mando);
  }

}
