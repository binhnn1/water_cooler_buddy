#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>

//The following include is for led FSMs
#include <FiniteStateMachine.h>
#include <LED.h>

// The following include is for storing data to EEPROM
#include <EEPROM.h>

String RequestLEDoff;
String RequestLEDon;
String StatusLEDoff;
String StatusLEDon;
String RequestAnalog;
String RequestStop;
boolean readStop;

// SSID Credentials
char* wifi_ssid = "Binh";
char* wifi_pwd = "123456789";

// MQTT Server Credentials
char* mqtt_server = "m11.cloudmqtt.com";
char* mqtt_port = "12201";
char* mqtt_user = "hculdksf";
char* mqtt_pwd = "GCGrhTnLhphv";

WiFiManager wifiManager;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
boolean LED;

/*********************************************************/
void printAnalog(boolean readStop){
  if(readStop){
    char messageBuff[100];
    String temp = String(analogRead(A0),DEC);
    //snprintf (msg, 75, temp, value);
    temp.toCharArray(messageBuff,temp.length()+1);
    client.publish("topic/2", messageBuff);
  }
}
/***********************************************************************************/
void callback(char* topic, byte* payload, unsigned int length1) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int j = 0;
  int k = 0;
  int l = 0;
  int m = 0;
/*********************************************************/
  for (int i = 0; i < RequestLEDon.length(); i++){
    if((char)payload[i] == RequestLEDon[i]){
      j++;
    } 
  }
  if(j == RequestLEDon.length())
  /*if(sCompare(RequestLEDon,payload))*/{
    digitalWrite(13, HIGH);
    LED = true;
  }
/*********************************************************/
 for (int i = 0; i < RequestLEDoff.length(); i++){
    if((char)payload[i] == RequestLEDoff[i]){
      k++;
    } 
  }
  if(k == RequestLEDoff.length())
  /*if(sCompare(RequestLEDoff,payload))*/{
    digitalWrite(13, LOW);
    LED = false;
  }
/*********************************************************/
  for (int i = 0; i < RequestAnalog.length(); i++){
    if((char)payload[i] == RequestAnalog[i]){
      l++;
    } 
  }
  if(l == RequestAnalog.length()){
    readStop = true;
    printAnalog(readStop);
  }
/*********************************************************/
  for (int i = 0; i < RequestStop.length(); i++){
    if((char)payload[i] == RequestStop[i]){
      m++;
    } 
  }
  if(m == RequestStop.length()){
    readStop = false;
    printAnalog(readStop);
  }
}

/***********************************************************************************/
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting a conenection to MQTT...");

    if (client.connect("ESP8266Client", mqtt_user, mqtt_pwd)) {
      Serial.println("connected");
      client.publish("topic/1", "publishing-yes");
      client.subscribe("topic/1");//subscribe to data from topic/1
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" will try connecting again in 5 secs");
      delay(5000);
    }
  }
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
}


void resetWifi() {
  wifiManager.resetSettings();
}


void setup_wifi() { 
 wifiManager.setAPCallback(configModeCallback);
 Serial.println("Setting up Wifi");
    if (!wifiManager.autoConnect("DeviceConfig","config11")) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }
}

/***********************************************************************************/
// LOAD AND SAVE CREDENTIALS
/***********************************************************************************/
void save_data(char* data) {
  Serial.println("Write data to EEPROM");
  EEPROM.begin(512);
  for (int i = 0; i < strlen(data); ++i) {
    EEPROM.write(i, (int)data[i]);
    delay(1);
  }
  EEPROM.commit();
  Serial.println("Write data complete");
  delay(100);
}


void load_data() {
  EEPROM.begin(512);
  int count = 0;
  int address = 0;
  char data[100];
  while (count < 6) {
    char read_char = (char)EEPROM.read(address);
    delay(1);
    if (read_char == '#') {
      ++count;
      switch (count) {
        case 1: strcpy(wifi_ssid, data); break;
        case 2: strcpy(wifi_pwd, data); break;
        case 3: strcpy(mqtt_server, data); break;
        case 4: strcpy(mqtt_port, data); break;
        case 5: strcpy(mqtt_user, data); break;
        case 6: strcpy(mqtt_pwd, data); break;
      }
      strcpy(data,"");
    } 
    else {
      strncat(data, &read_char, 1);  
    }
    ++address;
  }
  delay(100);
}

void data_setup(char* data) {
  char* sep = "#";
  strcat(data, wifiManager._ssid.c_str());
  strcat(data, sep);
  strcat(data, wifiManager._pass.c_str());
  strcat(data, sep);
  strcat(data, wifiManager.MQTT_server.c_str());
  strcat(data, sep);
  strcat(data, wifiManager.MQTT_port.c_str());
  strcat(data, sep);
  strcat(data, wifiManager.MQTT_user.c_str());
  strcat(data, sep);
  strcat(data, wifiManager.MQTT_pass.c_str());
  strcat(data, sep); 
  Serial.println("This is the final string:");
  Serial.println(data);
  Serial.println();
}

/***********************************************************************************/
void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(115200);

  Serial.println();
  Serial.println();

  load_data();

  Serial.println(wifi_ssid);
  Serial.println(wifi_pwd);
  Serial.println(mqtt_server);
  Serial.println(mqtt_port);
  Serial.println(mqtt_user);
  Serial.println(mqtt_pwd);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  
  WiFi.begin(wifi_ssid, wifi_pwd);

  for (int c = 0; c <= 30 and WiFi.status() != WL_CONNECTED; ++c) {
    delay(500);
    Serial.print(".");
    if (c == 30) {
      Serial.println();
      Serial.println("Connection Time Out...");
      Serial.println("Enter AP Mode...");
      setup_wifi();
      char data[100] = "";
      data_setup(data);
      save_data(data);
      delay(1000);
      ESP.reset();
    }
  }
  
  Serial.println("connected");
    
  client.setServer(mqtt_server, atoi(mqtt_port));
  client.setCallback(callback);
  RequestLEDoff = "RqstOFF";
  RequestLEDon = "RqstON";
  StatusLEDoff = "StatusOFF";
  StatusLEDon = "StatusON";
  RequestAnalog = "RqstA";
  RequestStop = "RqstStop";
  LED = false;
  readStop = false;
}


/***********************************************************************************/
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 500) {
    lastMsg = now;
    printAnalog(readStop);
    if(LED){
      snprintf (msg, 75, "StatusON", value);
      client.publish("topic/1", msg);
      snprintf (msg, 75, "RqstOFF", value);
      client.publish("topic/1", msg);
    }
    else{
      snprintf (msg, 75, "StatusOFF", value);
      client.publish("topic/1", msg);
      snprintf (msg, 75, "RqstON", value);
      client.publish("topic/1", msg);
    }
  }
}
