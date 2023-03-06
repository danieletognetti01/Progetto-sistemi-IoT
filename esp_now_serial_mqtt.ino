
#include <SPI.h>
#include <PubSubClient.h>
#include <WiFi.h>

const char* ssid     = "La Scintilla Gt 2.4Ghz";
const char* password = "Tognettigf_1305";

IPAddress server(192, 168, 86, 103);
WiFiClient WifiClient;
PubSubClient client(WifiClient);

String num_pack_recv="";
String num_pack_tot="";
String tempo="";
bool modify = false;
bool isfirst = true;
String throughput ="";

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  num_pack_tot="";
  for (int i=0;i<length;i++) {
    num_pack_tot += ((char)payload[i]);
    Serial2.print((char)payload[i]);
  }
  Serial2.println();
  //Serial2.flush();
  Serial.println();
  int conta = 0;
  String datae;
  while(conta<2){
    if(Serial2.available()){
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
  Serial.println("serial 2 disponibile");
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
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200);

  client.setServer(server, 1883);
  client.setCallback(callback);

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
    if(modify){
      // String datae = Serial2.readStringUntil('\n');
      //Serial.println(datae);
      client.publish("/result/esp32_2/num_pack_recv", (char*) num_pack_recv.c_str());
      client.publish("/result/esp32_2/num_pack_tot", (char*) num_pack_tot.c_str());
      client.publish("/result/esp32_2/tempo", (char*) tempo.c_str());
      client.publish("/result/esp32_2/throughput", (char*) throughput.c_str());
      modify = false;
    }
  }
  // if (Serial2.available() > 0) {
    // read the incoming string:
   // String datae = Serial2.readStringUntil('\n');
    //client.publish("/values/tognetti", (char*) datae.c_str());
    
    // prints the received data
    //Serial.print("I received: ");
    //Serial.println(datae);
  //}
}
