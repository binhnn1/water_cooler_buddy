#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>

// The following include is for storing data to EEPROM
#include <LiquidCrystal_I2C.h>
#include <RTC_DS3231.h>
#include <EMem.h>




LiquidCrystal_I2C lcd(0x3F, 16, 2);
RTC_DS3231 rtc(0x68);
EMem emem;

// Pin Define
int relayMain_pin = 13;


String RequestRelayMainoff;
String RequestRelayMainon;
String StatusRelayMainoff;
String StatusRelayMainon;
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
//boolean LED;

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
  for (int i = 0; i < RequestRelayMainon.length(); i++){
    if((char)payload[i] == RequestRelayMainon[i]){
      j++;
    } 
  }
  if(j == RequestRelayMainon.length())
  /*if(sCompare(RequestRelayMainon,payload))*/{
    digitalWrite(relayMain_pin, HIGH);
//    LED = true;
  }
/*********************************************************/
 for (int i = 0; i < RequestRelayMainoff.length(); i++){
    if((char)payload[i] == RequestRelayMainoff[i]){
      k++;
    } 
  }
  if(k == RequestRelayMainoff.length())
  /*if(sCompare(RequestRelayMainoff,payload))*/{
    digitalWrite(relayMain_pin, LOW);
//    LED = false;
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
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to MQTT Server");
  lcd.setCursor(0, 1);
  while (!client.connected()) {
    Serial.print("Attempting a conenection to MQTT...");
    lcd.print(".");
    if (client.connect("ESP8266Client", emem.getMqttUser().c_str(), emem.getMqttPwd().c_str())) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Connected");
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
double x;
int dis;
int last;


/***********************************************************************************/
// INTERRUPT TRIGGERED BUTTON TO READ POTENTIOMETER
/***********************************************************************************/
int btn_readtemp_pin = 14;
int btn_startup_pin = 15;
int input_temp = 0;

void ISR_readtemp() {
  input_temp = analogRead(A0);
}

void ISR_startup() {
  ESP.reset();
}
/***********************************************************************************/



void setup() {
  pinMode(btn_readtemp_pin, INPUT);
  pinMode(btn_startup_pin, INPUT);
  pinMode(relayMain_pin, OUTPUT);
  
  attachInterrupt(btn_readtemp_pin, ISR_readtemp, RISING);
  attachInterrupt(btn_startup_pin, ISR_startup, RISING);

 
  Serial.begin(115200);

  Wire.begin();
  //Initialize LCD
  lcd.begin(16,2);
  lcd.init();
  lcd.backlight();


  //Initialize Infared Sensor
  last = 0;
  
  Serial.println();
  Serial.println();

  emem.loadData();

  Serial.println(emem.getWifiSsid());
  Serial.println(emem.getWifiPwd());
  Serial.println(emem.getMqttServer());
  Serial.println(emem.getMqttPort());
  Serial.println(emem.getMqttUser());
  Serial.println(emem.getMqttPwd());

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(emem.getWifiSsid());

  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print(emem.getWifiSsid());
  
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  
  WiFi.begin(emem.getWifiSsid().c_str(), emem.getWifiPwd().c_str());

  for (int c = 0; c <= 30 and WiFi.status() != WL_CONNECTED; ++c) {
    delay(500);
    Serial.print(".");
    lcd.print(".");  
    if (c == 30) {
      Serial.println();
      Serial.println("Connection Time Out...");
      Serial.println("Enter AP Mode...");
      setup_wifi();
      char data[100] = "";
      data_setup(data);
      emem.saveData(data);
      delay(1000);
      ESP.reset();
    }
  }


  delay(1000);
  lcd.clear();
  
  Serial.println("connected");

  lcd.setCursor(0, 0);
  lcd.print("connected");

  
  strcpy(mqtt_server, emem.getMqttServer().c_str());
  client.setServer(mqtt_server, atoi(emem.getMqttPort().c_str()));
  client.setCallback(callback);

  
  RequestRelayMainoff = "RqstOFF";
  RequestRelayMainon = "RqstON";
  StatusRelayMainoff = "StatusOFF";
  StatusRelayMainon = "StatusON";
  RequestAnalog = "RqstA";
  RequestStop = "RqstStop";
//  LED = false;
  readStop = false;
}

/***********************************************************************************/
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //RTC Operation
  rtc.displayTime();
  delay(3000);


  //Infared Sensor Operation
  x = analogRead(A0);

  while(millis() <= last + 20) {};
  
  last = millis();
  dis = 4800/(x-20);
    
  if(dis > 80 || dis < 10)
    Serial.println("Infared: Out of Range");
  else {
    Serial.print("Infared distance: ");
    Serial.println(dis);
  }
    

  if (digitalRead(12)) {
    Serial.println("MOTION DETECTED");
    Serial.println("RESET CLOCK");
    
//    digitalWrite(relayMain_pin, HIGH);

//    snprintf (msg, 75, "StatusOFF", value);
//    client.publish("topic/1", msg);
    snprintf (msg, 75, "RqstON", value);
    client.publish("topic/1", msg);

    
    rtc.setTime(0, 0, 0, 6, 13, 01, 17); // ss:mm:hh - WeekDay - dd:mm:yy
  } else {
    Serial.println("NO MOTION");
    Serial.println("RESET CLOCK");
    rtc.setTime(0, 0, 0, 6, 13, 01, 17);
    while(1) {
      byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
      rtc.readTime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
      int t = second + +60*minute + 3600*hour;
      delay(5000);

      // Print out current count time to LCD
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Time: ");
      lcd.setCursor(0, 1);
      lcd.print(t);

      if (digitalRead(12)) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Detect Motion");
        lcd.setCursor(0, 1);
        lcd.print("Turn ON");
        snprintf (msg, 75, "RqstON", value);
        client.publish("topic/1", msg);
        client.loop();
        break;
      } else {
        if (t >= 10) {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Exceed 30 minutes");
          lcd.setCursor(0, 1);
          lcd.print("Turn OFF");


          snprintf (msg, 75, "RqstOFF", value);
          client.publish("topic/1", msg);
          client.loop();
          break;
//
//          digitalWrite(relayMain_pin, LOW);

//          snprintf (msg, 75, "StatusON", value);
//          client.publish("topic/1", msg);
//          snprintf (msg, 75, "RqstOFF", value);
//          client.publish("topic/1", msg);
        }
      }
    }
  }
  
//  client.loop();
//  long now = millis();
//  if (now - lastMsg > 500) {
//    lastMsg = now;
//    printAnalog(readStop);
//    if(LED){
//      lcd.clear();
//      lcd.setCursor(0, 0);
//      lcd.print("STATUS ON");
//      lcd.setCursor(0, 1);
//      lcd.print("REQUEST OFF");
//      snprintf (msg, 75, "StatusON", value);
//      client.publish("topic/1", msg);
//      snprintf (msg, 75, "RqstOFF", value);
//      client.publish("topic/1", msg);
//    }
//    else{
//      lcd.clear();
//      lcd.setCursor(0, 0);
//      lcd.print("STATUS OFF");
//      lcd.setCursor(0, 1);
//      lcd.print("REQUEST ON");
//      snprintf (msg, 75, "StatusOFF", value);
//      client.publish("topic/1", msg);
//      snprintf (msg, 75, "RqstON", value);
//      client.publish("topic/1", msg);
//    }
//  }
}
