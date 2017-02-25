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

#define encoder0PinA  14
#define encoder0PinB  7
#define encoder0Select 3

#define SX1509_RELEASE_BUTTON 8
/***************************************************************************************/

#define heaterThreshold 180
#define coolerThreshold 60
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


int inputTemp = 50;
int encoder0Pos = 50;
int encoder0PinALast = LOW;
int n = LOW;

void doEncoder() {
  if (digitalRead(encoder0PinA) == io.digitalRead(encoder0PinB)) {
    inputTemp++;
    if(inputTemp >= 190)
      inputTemp = 190;
  } else {
    inputTemp--;
    if(inputTemp <= 50)
      inputTemp = 50;
  }
  Serial.println (inputTemp, DEC);
}

unsigned long previousMillis = 0;

int selectTemp() {
  
  while(true)
  {
    yield();
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 30000)
    {
      Serial.println("Time out for select temperature");
      return - 10;
    }
    if (currentMillis - previousMillis >= 100)
    {
      if (digitalRead(encoder0PinA) == io.digitalRead(encoder0PinB)) {
        encoder0Pos++;
        if(encoder0Pos >= 190)
          encoder0Pos = 190;
      } else {
        encoder0Pos--;
        if(encoder0Pos <= 50)
          encoder0Pos = 50;
      }
      Serial.println (encoder0Pos, DEC);
      if (digitalRead(encoder0Select))
        return encoder0Pos;
    }
  }
}

void buttonPress()
{
  Serial.print("Selected: ");
  Serial.println(inputTemp);
}


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
//  io.pinMode(SX1509_ENCODER_PINA, INPUT);
//  io./pinMode(SX1509_ENCODER_PINB, INPUT);
//  io.pinMode(SX1509_ENCODER_SELECT, INPUT);

  
  if (!io.begin(SX1509_ADDRESS))
    while (1);
    
  pinMode(encoder0Select, INPUT);
  digitalWrite(encoder0Select, HIGH);
  
  pinMode(encoder0PinA, INPUT); 
  digitalWrite(encoder0PinA, HIGH);       // turn on pull-up resistor
  io.pinMode(encoder0PinB, INPUT); 
  io.digitalWrite(encoder0PinB, HIGH);       // turn on pull-up resistor

//  attachInterrupt(digitalPinToInterrupt(encoder/0Select),buttonPress,FALLING);
//  attachInterrupt(digitalPinToInt/errupt(encoder0PinA), doEncoder, CHANGE);  // encoder pin on interrupt 0 - pin A
  Serial.begin (115200);
  Serial.println("\nstart");                // a personal quirk
  pinMode(3, FUNC_GPIO3);
  


  Wire.begin();
  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();

  last = 0;
  
  Serial.println();
  Serial.println();

//  emem.loadData();
//  
//  Serial.println(emem.getWifiSsid());
//  Serial.println(emem.getWifiPwd());
//  Serial.println(emem.getMqttServer());
//  Serial.println(emem.getMqttPort());
//  Serial.println(emem.getMqttUser());
//  Serial.println(emem.getMqttPwd());
//
//  Serial.println();
//  Serial.print("Connecting to ");
//  Serial.println(emem.getWifiSsid());
//
//  lcd.setCursor(0, 0);
//  lcd.print("Connecting to");
//  lcd.setCursor(0, 1);
//  lcd.print(emem.getWifiSsid());
//
//  delay(2000);
//
//  lcd.clear();
//  lcd.setCursor(0, 0);
  
//  WiFi.begin(emem.getWifiSsid().c_str(), emem.getWifiPwd().c_str());
//
//  for (int c = 0; c <= 30 and WiFi.status() != WL_CONNECTED; ++c) {
//    delay(500);
//    Serial.print(".");
//    lcd.print(".");  
//    if (c == 30) {
//      Serial.println();
//      Serial.println("Connection Time Out...");
//      Serial.println("Enter AP Mode...");
//      setup_wifi();
//      char data[100] = "";
//      data_setup(data);
//      emem.saveData(data);
//      delay(1000);
//      ESP.reset();
//    }
//  }
//
//
//  delay(1000);
//  lcd.clear();
//  
//  Serial.println("connected");
//
//  lcd.setCursor(0, 0);
//  lcd.print("Connected");
//  delay(1000);
//  
//  strcpy(mqtt_server, emem.getMqttServer().c_str());
//  client.setServer(mqtt_server, atoi(emem.getMqttPort().c_str()));
//  client.setCallback(callback);

  RequestRelayHotOn = "RqstHotON";
  RequestRelayHotOff = "RqstHotOFF";
  RequestRelayColdOn = "RqstColdON";
  RequestRelayColdOff = "RqstColdOFF";  
}
char msg[50];
int value = 0;
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
double heaterTemp;
double coolerTemp;

void loop() {

  bool mix = false;
  if (1 || io.digitalRead(SX1509_MOTION0))
  {
    Serial.println("Detect Motion. Turn on Heater and Cooler"); 
    int i = 0;
    while (digitalRead(encoder0Select))
    {
      delay(1000);
      yield();
      ++i;
      Serial.print(".");
      if (i == 30)
      {
        Serial.println("\nFalse Motion");
        break;
      }
    }
    if (!digitalRead(encoder0Select))
    {
      int inputTemp = selectTemp();
      while(inputTemp < 0) {
        yield();
        inputTemp = selectTemp();
      }
      Serial.print("Temperature selected: ");
      Serial.println(inputTemp);
      mix = true;

      if (mix)
      {
        heaterTemp = 180;
        coolerTemp = 60;


        int choice = 0;
        bool c;
        int stopPoint, diff;
    
        if (inputTemp > heaterTemp)
        {
          choice = 1;
          Serial.println("Turn on Heater");
          while(heaterTemp < inputTemp)
          {
            yield();
            delay(1000);
            heaterTemp++;
          }
          Serial.println("Turn off Heater");
        }
        else if (inputTemp < coolerTemp)
        {
          choice = 2;
          Serial.println("Turn on Cooler");
          while(coolerTemp > inputTemp)
          {
            yield();
            delay(1000);
            coolerTemp--;
          }
          Serial.println("Turn off Cooler");
        }
        else
        {
          choice = 3;
          int hotPortion = inputTemp - coolerTemp;
          int coldPortion = heaterTemp - inputTemp;
          int gcdr = gcd(hotPortion, coldPortion);
          hotPortion = hotPortion / gcdr;
          coldPortion = coldPortion / gcdr;



          
          Serial.print("Hot Portion = "); Serial.println(hotPortion);
          Serial.print("Cold Portion = "); Serial.println(coldPortion);

          if (hotPortion > coldPortion)
          {
            stopPoint = hotPortion;
            diff = hotPortion - coldPortion;
            c = true;
          }
          else
          {
            stopPoint = coldPortion;
            diff = coldPortion - hotPortion;
            c = false;
          }
        }

        i = 0;
        while(io.digitalRead(SX1509_RELEASE_BUTTON))
        {
          delay(1000);
          yield();
          ++i;
          Serial.print(".");
          if (i == 30)
          {
            Serial.println("Time Out");
            break;
          }
        }

        while(!io.digitalRead(SX1509_RELEASE_BUTTON))
        {
          yield();
          if (choice == 1)
          {
            Serial.println("Open Hot Solenoid");
          }
          else if (choice == 2)
          {
            Serial.println("Open Cold Solenoid");
          }
          else if (choice == 3)
          {
            Serial.println("\nOpen Hot Solenoid");
            Serial.println("Open Cold Solenoid");
            delay(stopPoint * 500);
            if (c)
              Serial.println("Close Cold Solenoid");
            else
              Serial.println("Close Hot Solenoid");
            delay(diff * 500);
            Serial.println("Close Cold Solenoid");
          }
          else if (choice == 0)
          {
            Serial.println("DEBUG");
          }
        }
      }
    }
  } 
  else
  {
    Serial.println("No Motion Detected");
    rtc.setTime(0, 0, 0, 6, 13, 01, 17);
    while(true)
    {
      yield();
      delay(1000);

      if (io.digitalRead(SX1509_MOTION0))
      {
        Serial.println("Motion Detected. Reset");
        break;
      }
      else
      {
        rtc.readTime( & second, & minute, & hour, & dayOfWeek, & dayOfMonth, & month, & year);
        int t = second + +60 * minute + 3600 * hour;
        Serial.print(t);
        Serial.print(" s\n");
        if (t >= 10)
        {
          Serial.println("Exceed 30 minutes");
          Serial.println("Turn off heater and cooler");

          heaterTemp = 190;
          coolerTemp = 60;
          
          if (heaterTemp < heaterThreshold)
          {
            Serial.println("Turn on heater");
          }
          else
          {
            Serial.println("Turn off heater");
          }

          if (coolerTemp > coolerThreshold)
          {
            Serial.println("Turn on cooler");
          }
          else
          {
            Serial.println("Turn off cooler");
          }
        }
      }
    }
  }
//  if (!client.connected())
//    reconnect();
//  client.loop();

//  x = analogRead(A0);
//  while(millis() <= last + 20) {};
//  if(dis > 80 || dis < 10)
//    Serial.println("Infared: Out of Range");
//  else {
//    Serial.print("Infared distance: ");
//    Serial.println(dis);
//  }
//
//
//  if (1 || io.digitalRead(SX1509_MOTION0)) {
//    
//    
//  } else {
//    Serial.println("NO MOTION");
//    Serial.println("RESET CLOCK");
//    rtc.setTime(0, 0, 0, 6, 13, 01, 17);
//    while(1) {
//      rtc.readTime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
//      int t = second + +60*minute + 3600*hour;
//      delay(5000);
//
//      // Print out current count time to LCD
//      lcd.clear();
//      lcd.setCursor(0,0);
//      lcd.print("Time: ");
//      lcd.setCursor(0, 1);
//      lcd.print(t);
// 
//      if (io.digitalRead(SX1509_MOTION0) || io.digitalRead(SX1509_MOTION1) || io.digitalRead(SX1509_MOTION2) || (10 < dis < 80)) {
//        lcd.clear();
//        lcd.setCursor(0,0);
//        lcd.print("Detect Motion");
//        Serial.println("DETECT MOTION");
////        lcd.setCursor(0, 1);
////        lcd.print("Turn ON");
////        snprintf(msg, 75, "RqstHotON", value);
////        client.publish("topic/1", msg);
//        
//        client.loop();
//        break;
//      } else {
//        if (t >= 10) {
//          lcd.clear();
//          lcd.setCursor(0, 0);
//          lcd.print("Exceed 30 minutes");
//          Serial.println("Exceed 30 min");
//          lcd.setCursor(0, 1);
//          lcd.print("Turn OFF");
//          Serial.println("Turn off");
//
//
//          //          snprintf(msg, 75, "RqstHotOFF", value);
//          //          client.publish("topic/1", msg);
//          //          snprintf(msg, 75, "RqstColdOFF", value);
//          //          client.publish("topic/1", msg);
//          Serial.println("RqstHotOFF");
//          Serial.println("RqstColdOFF");
//
//          double cH = thermoArray[0].readCelsius();
//          double cC = thermoArray[1].readCelsius();
//           if (cH < thresH) {
//            Serial.println("RqstHotOn");
//            //            snprintf(msg, 75, "RqstHotON", value);
//            //            client.publish("topic/1", msg);
//          } else {
//            Serial.println("RqstHotOff");
//            //            snprintf(msg, 75, "RqstHotOFF", value);
//            //            client.publish("topic/1", msg);
//          }
//
//          if (cC > thresC) {
//            Serial.println("RqstColdOn");
//            //            snprintf(msg, 75, "RqstColdON", value);
//            //            client.publish("topic/1", msg);
//          } else {
//            Serial.println("RqstCold Off");
//            //            snprintf(msg, 75, "RqstColdOFF", value);
//            //            client.publish("topic/1", msg);
//          }
//        }
//      }
//    }
//  }
}