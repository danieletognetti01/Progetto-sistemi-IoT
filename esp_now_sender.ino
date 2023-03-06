#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xC8, 0xF0, 0x9E, 0x7B, 0x15, 0xAC};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  int num[62];
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;
// callback function that will be executed when data is received

unsigned long start;
unsigned long finish;
int pack_lost = 0;
float tempo_medio = 0;
int num_pack =0 ;
float throughput = 0;
bool isfirst;
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  finish = millis();
  Serial.print("Tempo impiegato: ");
  Serial.println(finish-start);
  if (isfirst){
    isfirst = false;
    tempo_medio = finish-start;
    throughput = len*8*1000/tempo_medio;
  }
  else{
    tempo_medio = (tempo_medio + (finish-start))/2;
    throughput = (throughput + (len*8*1000/tempo_medio))/2;
  }
  Serial.print("Throughput: "); Serial.print(throughput); Serial.println(" bit/s");
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Num: ");
  Serial.println(myData.num[0]);
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? NULL : pack_lost++ );
  //ESP_NOW_SEND_SUCCESS ? NULL : pack_lost++;
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  Serial2.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
  if(Serial2.available()){
     String command = Serial2.readStringUntil('\n');
     //Serial2.end();
     isfirst = true;
     int index = command.indexOf(' ');
     num_pack = command.substring(0, index+1).toInt();
     String mac = command.substring(index+1);

       char str[18];
    mac.toCharArray(str, 18);
  uint8_t MAC[6];
  char* ptr;

  MAC[0] = strtol( strtok(str,":"), &ptr, HEX );
  for( uint8_t i = 1; i < 6; i++ )
  {
    MAC[i] = strtol( strtok( NULL,":"), &ptr, HEX );
  }



  Serial.print(MAC[0], HEX);
  for( uint8_t i = 1; i < 6; i++)
  {
    Serial.print(':');
    Serial.print( MAC[i], HEX);
  }
  if(!esp_now_is_peer_exist(MAC)){
     memcpy(peerInfo.peer_addr, MAC, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  }
  Serial.println();
  
     Serial.println(num_pack);
     Serial.println(mac);
      for(int i=0;i<num_pack;i++){
         myData.num[0] = i;
        // Send message via ESP-NOW
        //esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
        esp_err_t result = esp_now_send(MAC, (uint8_t *) &myData, sizeof(myData));
        start = millis();
        if (result == ESP_OK) {
          Serial.println("Sent with success");
        }
        else {
          pack_lost++;
          Serial.println("Error sending the data");
        }
           delay(2000);
      }
      Serial.print("Pacchetti persi:");
      Serial.println(pack_lost);
      Serial.print("Tempo medio:");
      Serial.println(tempo_medio);
      String mando = "";
      String count_s = String(num_pack - pack_lost);
      String separator = "#";
      String time_s = String(tempo_medio);
      String separator_2 = "_";
      String throughput_s = String(throughput);
      mando = count_s + separator + time_s + separator_2 + throughput_s;
      Serial.println(mando);
      //Serial2.begin(115200);
      Serial2.println(mando); 
      pack_lost = 0;
      tempo_medio = 0;
      throughput = 0;
      Serial.println("--------------------------");
    
  }

}
