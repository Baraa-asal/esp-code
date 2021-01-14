#include "WiFi.h"
#include "PubSubClient.h"
#include "ArduinoJson.h"


String broker = "ip-160-153-252-170.ip.secureserver.net";
int port = 1883;
String topicIn = "loads-control";
String topicOut = "loads-updates";
const char* ssid ="im graduating";
const char* password ="passwordhh";
int wifiLedPort = 27;
int serverLedPort = 14;
const String id = "ESP-A";
float counter = 0;
String username = "espUser";
String sPassword = "3R_fv!PvDyDnD64wB@-$e9k";

const int ledPin = 12;
int buttonPin = 13 ;

StaticJsonDocument<300> loads; //Memory pool
String loadsNames [] = {"Loada1", "Loada2", "Loada3", "Loada4", "Loada5", "Loada6", "Loada7", "Loada8", "Loada9", "Loada10","Loada11"};
float loadsPower [] = {22.5, 12.5, 21.5, 12.5, 10, 7, 15, 3, 13, 14.5, 0};
int pins [] = {15, 2, 0, 4, 16, 17, 5, 18, 19, 21, 12};

bool statuses [] = {true,true,true,true,true,true,true,true,true,true, false};
int additionalLoadValues[] = {0, 10, 20, 30, 40};
int additionalLoadIndex = 0;
StaticJsonDocument<256> doc;

int indexOf (String arr[], String value) {
    int s = sizeof(arr)/sizeof(int);
    //Serial.println(s);
    for (int i=0; i < 17;i++) {
      if (arr[i] == value) {
        return i;
      }
    }
    return -1;
}

void parseMessage(String msg){
  Serial.println("parsing");
  Serial.println(msg);
  auto error = deserializeJson(doc, msg);
  if (error) { //Check for errors in parsing
  Serial.println("Parsing failed");
  delay(5000);
  return;}

  const char * lid = doc["id"]; 
  const char * loadsA = doc["loadA"]; 
  const char * loadsB = doc["loadB"];
  
  String s = doc["state"];
  String valueLoadStr = doc["ValueLoad"];
  bool state = s == "true";
  if (lid){
    Serial.println(lid);
    Serial.println(state);
    int index = indexOf(loadsNames, lid);
    if (index >= 0) {
    Serial.println(index);
    Serial.print(pins[index]);
    bool currentStatus = statuses[index];
    statuses[index] = state;
    if (valueLoadStr) {
     float loadValue = valueLoadStr.toFloat();
     loadsPower[index] = loadValue;
    }
    if (currentStatus != state) {
      publishStatus(index);
      }
    }
    }
    if (loadsA) {
      Serial.println(loadsA);
    }
 
  
  }

void callback(char* topic, byte* payload, unsigned int length) {

  if (String(topic) == topicIn){ //loadcontrol channel
    Serial.println("it is my topic"); 
    String msg = String((char*) payload); //convert from bytes to string
    msg = msg.substring(0, length); //to avoid noise
    parseMessage(msg);
    }
  }

WiFiClient mqttClient; //mqttClient is a variale of type wificlient 
PubSubClient client(mqttClient);

void publishStatus(int loadIndex) {
        String loadId = loadsNames[loadIndex];
        bool loadStatus = statuses[loadIndex];
        float value = loadsPower[loadIndex];
        String msg = String("{" + String('"') + "id" + String('"') + ":" + String('"') + loadId + String('"'));
        msg = msg + "," + String('"') + "state" + String('"')+ ":" + String('"') + String(loadStatus ? "true" : "false") + String('"');
        msg = msg + "," + String('"') + "ValueLoad" + String('"')+ ":" + String('"') + String(value) + String('"');
        msg = msg + "}";
        Serial.println(msg);
        if (client.publish(topicOut.c_str(), msg.c_str())) {}
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(id.c_str(),username.c_str(),sPassword.c_str())) { //if i got auth from authentication
      digitalWrite(serverLedPort, HIGH);
      Serial.println("connected");
        for (int i=0; i < 11; i++) {
          publishStatus(i);
        }
      client.subscribe(topicIn.c_str());
    } else {
      digitalWrite(serverLedPort, HIGH);
      delay(500);
      digitalWrite(serverLedPort, LOW);
      delay(500);
    }
  }
}


void setup() {

pinMode(wifiLedPort,OUTPUT);
pinMode(serverLedPort,OUTPUT);

pinMode(ledPin, OUTPUT);
pinMode(buttonPin, INPUT);
  for (int i =0; i < 11; i++) {
      pinMode(pins[i],OUTPUT);
  }
Serial.begin(9600); //to show output on serial monitor or plotter //number of bits per second sent from esp through serial 
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED)
{
  digitalWrite(wifiLedPort,HIGH);
  delay(300);
  digitalWrite(wifiLedPort,LOW);
  delay(300);
  //Serial.println(WiFi.status());

  }
  if (WiFi.status() == WL_CONNECTED){
    Serial.println(WiFi.localIP());
  client.setServer(broker.c_str(), port); //setup for server 
  client.setCallback(callback); //when you get data from server(broker), callback is the function to process this data. (asynchronous func)
  digitalWrite(wifiLedPort,HIGH);
  }

}

  int loadIndex = 0;
  bool dirtyBit = true;
void loop() {

  while (counter < 1000*60){ //takes one minute to send update
   counter++;
 for (int i =0; i < 11; i++) {
    if (statuses[i]) {
      digitalWrite(pins[i],HIGH);
    } else {
      digitalWrite(pins[i],LOW);
    }
  }
        /**/
  /*int ButtonState = 0;
  ButtonState = digitalRead(buttonPin); // read the button pin and store in ButtonState variable

  if (ButtonState == HIGH && dirtyBit) {
   
    dirtyBit = false;
    delay(200);
    if (ButtonState == HIGH ) {
        Serial.println(ButtonState ? "high":"low");
      loadIndex = (loadIndex + 1) % 5;
      statuses[10] = additionalLoadValues[loadIndex] > 0;
      loadsPower[10] = additionalLoadValues[loadIndex];
      publishStatus(10);
      delay(100);
      Serial.println(loadIndex);
     } 
  } else {
      delay (20);
      if (ButtonState == LOW) {
      dirtyBit = true;
      }
     }*/
    if (!client.connected()){ 
    reconnect(); }
    client.loop(); // to test if there are any new messages
  
  delay(1); 
  }

   counter = 0;
  delay(200);
if (!client.connected()) {
    reconnect();
    
  }
  else{
        for (int i=0; i < 11; i++) {
          publishStatus(i);
        }
  }
}
 
