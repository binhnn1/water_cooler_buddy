#include <ESP8266WiFi.h>
#include <PubSubClient.h>

String RequestLEDoff;
String RequestLEDon;
String StatusLEDoff;
String StatusLEDon;
String RequestAnalog;
String RequestStop;
boolean readStop;
const char* ssid = "Verizon-SM-G928V-5D4E";
const char* password = "hamatlon";

// MQTT Server Credentials
const char* mqtt_server = "m11.cloudmqtt.com";
const char* mqtt_user = "hculdksf";
const char* mqtt_pwd = "GCGrhTnLhphv";
const int mqtt_port = 12201;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
boolean LED;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
/*********************************************************/
/*bool sCompare(String s, byte* payload){
  int j;
  for (int i = 0; i < s.length(); i++){
    if((char)payload[i] == s[i]) j++;
  }
  return(j == s.length());
}*/

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
/*********************************************************/

/***********************************************************************************/
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting a conenection to MQTT...");

    if (client.connect("ESP8266Client", mqtt_user, mqtt_pwd)) {
      Serial.println("connected");
      client.publish("topic/1", "publishing-yes");
      client.subscribe("topic/1");//subscribe to data from topic/2
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" will try connecting again in 5 secs");
      delay(5000);
    }
  }
}
/***********************************************************************************/
void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
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
