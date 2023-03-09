#include <SPI.h>
#include <PubSubClient.h>
#include <WiFi.h>

//Set Wi-Fi SSID and password
const char* ssid     = "XXX"
const char* password = "XXX";

//IP of broker server
IPAddress server(XX, XX, XX, XX);

WiFiClient WifiClient;
PubSubClient client(WifiClient);

String num_pack_recv;
String num_pack_tot;
String tempo;
String throughput;
bool modify = false;

//Callback called when a send command arrive from broker
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  num_pack_tot="";
  for (int i=0;i<length;i++) {
    num_pack_tot += ((char)payload[i]);
    //Send command to Serial port
    Serial2.print((char)payload[i]);
  }
  Serial.println();
  //Wait until there are the results coming from the serial port
  while(!Serial2.available()){
    ;
  }
  //Get data from serial and set flag modify to publish the results to the broker
  String datae = Serial2.readStringUntil('\n');
  Serial.println(datae);
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

//reconnect to broker fuction
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("arduinoClient", "mqtt_user","Tognettigf_1305")) {
      Serial.println("connected");
    client.subscribe("/command/esp32");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  //Initialize serial port
  Serial.begin(115200);
  Serial2.begin(115200);

  //Set ip and port of mqtt client
  client.setServer(server, 1883);
  //Set callback of client
  client.setCallback(callback);
  //Initialize Wi-Fi
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
  //Publish results to broker and set flag modify to false
  if(client.connected()){
    if(modify){
      client.publish("/result/esp32/num_pack_recv", (char*) num_pack_recv.c_str());
      client.publish("/result/esp32/num_pack_tot", (char*) num_pack_tot.c_str());
      client.publish("/result/esp32/tempo", (char*) tempo.c_str());
      if (!client.connected()) {
        reconnect();
      }
      client.publish("/result/esp32/throughput", (char*) throughput.c_str());
      modify = false;
    }
  }
}
