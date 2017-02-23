#include <Adafruit_MAX31855.h>
#include <Adafruit_TLC59711.h>
#include <DNSServer.h>
#include <EMem.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include <RTC_DS3231.h>
#include <SparkFunSX1509.h>
#include <SPI.h>
#include <WiFiManager.h>
#include <Wire.h>


#define thresH 70
#define thresC 30

/****************************SETUP FOR SX1509 GPIO EXTENDER*****************************/
const byte SX1509_ADDRESS = 0x3E;

#define SX1509_RELAY_HOT 0
#define SX1509_RELAY_COLD 1
#define SX1509_MOTION0 2
#define SX1509_MOTION1 3
#define SX1509_MOTION2 4
#define SX1509_ENCODER_PINA 5
#define SX1509_ENCODER_PINB 6
#define SX1509_ENCODER_SELECT 7
#define SX1509_RELEASE_BUTTON 8
/***************************************************************************************/


/********************************SETUP FOR THERMOCOUPLES********************************/
#define MAXDO 12
#define MAXCS0 13
#define MAXCS1 14
#define MAXCS2 16
#define MAXCLK 15

Adafruit_MAX31855 thermocouple0(MAXCLK, MAXCS0, MAXDO);
Adafruit_MAX31855 thermocouple1(MAXCLK, MAXCS1, MAXDO);
Adafruit_MAX31855 thermocouple2(MAXCLK, MAXCS2, MAXDO);
Adafruit_MAX31855 thermoArray[3] = {thermocouple0, thermocouple1, thermocouple2};
/***************************************************************************************/

String RequestRelayHotOn;
String RequestRelayHotOff;
String RequestRelayColdOn;
String RequestRelayColdOff;

//String StatusRelayMainoff;
//String StatusRelayMainon;
//String RequestAnalog;
String RequestStop;
boolean readStop;

/**********************************CREDENTIALS*******************************************/
char* wifi_ssid = "Binh";
char* wifi_pwd = "123456789";

char* mqtt_server = "m11.cloudmqtt.com";
char* mqtt_port = "12201";
char* mqtt_user = "hculdksf";
char* mqtt_pwd = "GCGrhTnLhphv";
/***************************************************************************************/


EMem emem;
LiquidCrystal_I2C lcd(0x3F, 16, 2);
RTC_DS3231 rtc(0x68);
SX1509 io;
WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient client(espClient);



/***************************************************************************************/
void printAnalog(boolean readStop){
  if(readStop){
    char messageBuff[100];
    String temp = String(analogRead(A0),DEC);
    //snprintf (msg, 75, temp, value);
    temp.toCharArray(messageBuff,temp.length()+1);
    client.publish("topic/2", messageBuff);
  }
}
/***************************************************************************************/
void callback(char* topic, byte* payload, unsigned int length1) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int j = 0;
  int k = 0;
  int l = 0;
  int m = 0;

/********************************CONTROL HEATER*****************************************/
  for (int i = 0; i < RequestRelayHotOn.length(); i++){
    if((char)payload[i] == RequestRelayHotOn[i]){
      j++;
    } 
  }
  if(j == RequestRelayHotOn.length()) {
    io.digitalWrite(SX1509_RELAY_HOT, HIGH);
  }
  
 for (int i = 0; i < RequestRelayHotOff.length(); i++){
    if((char)payload[i] == RequestRelayHotOff[i]){
      k++;
    } 
  }
  if(k == RequestRelayHotOff.length()) {
    io.digitalWrite(SX1509_RELAY_HOT, LOW);
  }  
/***************************************************************************************/


/********************************CONTROL COOLER*****************************************/
  for (int i = 0; i < RequestRelayColdOn.length(); i++){
    if((char)payload[i] == RequestRelayHotOn[i]){
      j++;
    } 
  }
  if(j == RequestRelayColdOn.length()) {
    io.digitalWrite(SX1509_RELAY_COLD, HIGH);
  }

 for (int i = 0; i < RequestRelayColdOff.length(); i++){
    if((char)payload[i] == RequestRelayHotOff[i]){
      k++;
    } 
  }
  if(k == RequestRelayColdOff.length()) {
    io.digitalWrite(SX1509_RELAY_COLD, LOW);
  }
/***************************************************************************************/
//
//  
//  for (int i = 0; i < RequestAnalog.length(); i++){
//    if((char)payload[i] == RequestAnalog[i]){
//      l++;
//    } 
//  }
//  if(l == RequestAnalog.length()){
//    readStop = true;
//    printAnalog(readStop);
//  }
///***************************************************************************************/
//  for (int i = 0; i < RequestStop.length(); i++){
//    if((char)payload[i] == RequestStop[i]){
//      m++;
//    } 
//  }
//  if(m == RequestStop.length()){
//    readStop = false;
//    printAnalog(readStop);
//  }
}

/***************************************************************************************/
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

/***************************************************************************************/
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
/***************************************************************************************/

/***************************************************************************************/
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
/***************************************************************************************/

void displayTemp(int i) {
  double c = thermoArray[i].readCelsius();
  if (isnan(c)) {
    Serial.println("Something wrong with thermocouple!");
  } else {
    Serial.print("Thermocouple ");
    Serial.print(i);
    Serial.print(". C = "); 
    Serial.println(c);
  }
}


double x;
int dis;
int last;


int gcd (int a, int b) {
  int c;
  while ( a != 0 ) {
     c = a; a = b%a; b = c;
  }
  return b;
}

void setup() {

  io.pinMode(SX1509_RELAY_HOT, OUTPUT);
  io.pinMode(SX1509_RELAY_COLD, OUTPUT);
  io.pinMode(SX1509_MOTION0, INPUT);
  io.pinMode(SX1509_MOTION1, INPUT);
  io.pinMode(SX1509_MOTION2, INPUT);
  io.pinMode(SX1509_ENCODER_PINA, INPUT);
  io.pinMode(SX1509_ENCODER_PINB, INPUT);
  io.pinMode(SX1509_ENCODER_SELECT, INPUT);

  Serial.begin(115200);
  
  if (!io.begin(SX1509_ADDRESS))
    while (1);

  Wire.begin();
  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();

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

  delay(2000);

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
  lcd.print("Connected");
  delay(1000);
  
  strcpy(mqtt_server, emem.getMqttServer().c_str());
  client.setServer(mqtt_server, atoi(emem.getMqttPort().c_str()));
  client.setCallback(callback);

  RequestRelayHotOn = "RqstHotON";
  RequestRelayHotOff = "RqstHotOFF";
  RequestRelayColdOn = "RqstColdON";
  RequestRelayColdOff = "RqstColdOFF";  
}
char msg[50];
int value = 0;
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

void loop() {
  if (!client.connected())
    reconnect();
  client.loop();

  x = analogRead(A0);
  while(millis() <= last + 20) {};
  if(dis > 80 || dis < 10)
    Serial.println("Infared: Out of Range");
  else {
    Serial.print("Infared distance: ");
    Serial.println(dis);
  }


  if (io.digitalRead(SX1509_MOTION0) || io.digitalRead(SX1509_MOTION1) || io.digitalRead(SX1509_MOTION2) || (10 < dis < 80))
  {
    snprintf(msg, 75, "RqstHotON", value);
    client.publish("topic/1", msg);

    snprintf(msg, 75, "RqstColdON", value);
    client.publish("topic/1", msg);

    
    bool stopRun = false;
    
    int n = LOW;
    int inputTemp = 0;
    int encoder0PinALast = LOW;
    
    while(!stopRun)
    {
      if(io.digitalRead(SX1509_ENCODER_SELECT))
      {
        rtc.setTime(0, 0, 0, 6, 13, 01, 17);
        bool change = true;
        bool mix = true;
        while(change)
        {
            n = digitalRead(SX1509_ENCODER_PINA);
            if ((encoder0PinALast == LOW) && (n == HIGH))
            {
             if (io.digitalRead(SX1509_ENCODER_PINB) == LOW)
             {
               inputTemp--;
             } else 
             {
               inputTemp++;
             }
             if(inputTemp >= 100)
              inputTemp = 100;
             else if(inputTemp <= 0)
              inputTemp = 0;
             Serial.println (inputTemp);
             //////////////
             // PRINT LCD//
             //////////////
             if (io.digitalRead(SX1509_ENCODER_SELECT))
             {
              change = false;       
             } else 
             {
              rtc.readTime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
              int t = second + +60*minute + 3600*hour;
              if (t >= 30)
              {
                change = false;
                mix = false;
              }
             }
             
            } 
             
        }
        if (mix)
        {
          
          double cH = thermoArray[0].readCelsius();
          double cC = thermoArray[1].readCelsius();
          
          int choice = 0;
          bool c;
          int stopPoint, diff;
          
          if (inputTemp > cH)
          {
            bool done = false;
            snprintf(msg, 75, "RqstHotON", value);
            client.publish("topic/1", msg);
            while (!done)
            {
              delay(1000);
              cH = thermoArray[0].readCelsius();
              if (cH >= inputTemp)
              {
                choice = 1;
                done = true;
              }
            }
          } else if (inputTemp < cC)
          {
            bool done = false;
            snprintf(msg, 75, "RqstColdON", value);
            client.publish("topic/1", msg);
            while (!done)
            {
              cC = thermoArray[0].readCelsius();
              if (cC <= inputTemp)
              {
                choice = 2;
                done = true;
              }
            }
          } else
          {
            choice = 3;
            int hotPortion = inputTemp - cC;
            int coldPortion = cH - inputTemp;
            int grcd = gcd(hotPortion, coldPortion);
            hotPortion = hotPortion/grcd;
            coldPortion = coldPortion/grcd;
            
            if (hotPortion > coldPortion) {
              stopPoint = hotPortion;
              diff = hotPortion - coldPortion;
              c = true;  
            } else {
              stopPoint = coldPortion;
              diff = coldPortion - hotPortion;
              c = false;
            }
          }

          bool exe = true;
          rtc.setTime(0, 0, 0, 6, 13, 01, 17);
          while(exe) {
            while (io.digitalRead(SX1509_RELEASE_BUTTON)) {
              exe = false;
              if (choice == 1) {
                Serial.println("Open hot solenoid");
              }
              else if (choice == 2) {
                Serial.println("Open cold solenoid");
              }
              else {
                Serial.println("Open Hot Solenoid");
                Serial.println("Open Cold Solenoid");
                delay(stopPoint*500);
                if (c)
                  Serial.println("Close Cold Solenoid");
                else
                  Serial.println("Close Hot Solenoid");
                delay(diff*500);
                Serial.println("Close Cold Solenoid");
              }
            }
            if (exe) {
              rtc.readTime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
              int t = second + +60*minute + 3600*hour;
              if (t > 30) {
                exe = false;
              }
            }
            
          }
          
        }
        
        
      } 
    }
    
    
  } else {
    Serial.println("NO MOTION");
    Serial.println("RESET CLOCK");
    rtc.setTime(0, 0, 0, 6, 13, 01, 17);
    while(1) {
      rtc.readTime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
      int t = second + +60*minute + 3600*hour;
      delay(5000);

      // Print out current count time to LCD
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Time: ");
      lcd.setCursor(0, 1);
      lcd.print(t);
 
      if (io.digitalRead(SX1509_MOTION0) || io.digitalRead(SX1509_MOTION1) || io.digitalRead(SX1509_MOTION2) || (10 < dis < 80)) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Detect Motion");
        
//        lcd.setCursor(0, 1);
//        lcd.print("Turn ON");
//        snprintf(msg, 75, "RqstHotON", value);
//        client.publish("topic/1", msg);
        
        client.loop();
        break;
      } else {
        if (t >= 10) {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Exceed 30 minutes");
          lcd.setCursor(0, 1);
          lcd.print("Turn OFF");


          snprintf(msg, 75, "RqstHotOFF", value);
          client.publish("topic/1", msg);
          snprintf(msg, 75, "RqstColdOFF", value);
          client.publish("topic/1", msg);

          double cH = thermoArray[0].readCelsius();
          double cC = thermoArray[1].readCelsius();
          
          if (cH < thresH) {
            snprintf(msg, 75, "RqstHotON", value);
            client.publish("topic/1", msg);
          } else {
            snprintf(msg, 75, "RqstHotOFF", value);
            client.publish("topic/1", msg);
          }

          if (cC > thresC) {
            snprintf(msg, 75, "RqstColdON", value);
            client.publish("topic/1", msg);
          } else {
            snprintf(msg, 75, "RqstColdOFF", value);
            client.publish("topic/1", msg);
          }
        }
      }
    }
  }
}
