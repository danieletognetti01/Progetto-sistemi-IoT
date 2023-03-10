
#include <SPI.h>
#include <PubSubClient.h>
#include <WiFi.h>

//Set SSID and password of the Wi-Fi
const char* ssid     = "XXXX";
const char* password = "XXXX";

//IP address of MQTT server
IPAddress server(XX, XX, XX, XX);

WiFiClient WifiClient;
PubSubClient client(WifiClient);

String num_pack_recv="";
String num_pack_tot="";
String tempo="";
bool modify = false;
bool isfirst = true;
String throughput ="";

//Callback called when a message from broker arrive
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  num_pack_tot="";
  for (int i=0;i<length;i++) {
    num_pack_tot += ((char)payload[i]);
    //Send command to serial interface
    Serial2.print((char)payload[i]);
  }
  Serial2.println();
  //Serial2.flush();
  Serial.println();
  int conta = 0;
  String datae;
  while(conta<2){
    //Wait information from serial interafce
    if(Serial2.available()){
       //Not consider the first message coming from serial interface if is the first time
       if(isfirst){
        isfirst = false;
      }
      else{
        conta++;
      }
      datae = Serial2.readStringUntil('\n');
      Serial.println(datae);
      conta++;
    }
  }
  Serial.println(datae);
  //Split the information received and set flag modify to true
  int index = datae.indexOf('#');
  int index_2 = datae.indexOf('_');
  num_pack_recv = datae.substring(0,index);
  tempo = datae.substring(index +1, index_2);
  throughput = datae.substring(index_2+1);
  modify = true;
  Serial.print("Pacchetti ricevuti: "); Serial.println(num_pack_recv);
  Serial.print("Pacchetti totali inviati: "); Serial.println(num_pack_tot);
  Serial.print("Tempo medio: "); Serial.println(tempo);
  Serial.print("Throughput: "); Serial.println(throughput);

  
}

//Reconnect to MQTT broker fuction
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("arduinoClient", "mqtt_user","Tognettigf_1305")) {
      Serial.println("connected");
    client.subscribe("/command/esp32_2");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200);

  //Set MQTT server ip and port
  client.setServer(server, 1883);
  client.setCallback(callback);

  //Connect to WI-Fi
  WiFi.begin(ssid, password);
  Serial.println(WiFi.macAddress());

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  delay(1500);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if(client.connected()){
    //If there are information to publish, publish them to the MQTT broker
    if(modify){
      client.publish("/result/esp32_2/num_pack_recv", (char*) num_pack_recv.c_str());
      client.publish("/result/esp32_2/num_pack_tot", (char*) num_pack_tot.c_str());
      client.publish("/result/esp32_2/tempo", (char*) tempo.c_str());
      client.publish("/result/esp32_2/throughput", (char*) throughput.c_str());
      modify = false;
    }
  }
}
