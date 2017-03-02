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
#define sampleLoop 10
/****************************SETUP FOR SX1509 GPIO EXTENDER*****************************/
const byte SX1509_ADDRESS = 0x3E;

#define SX1509_RELAY_HEATER 0
#define SX1509_RELAY_COOLER 1
#define SX1509_MOTION0 2
#define SX1509_MOTION1 3
#define SX1509_MOTION2 4

#define encoder0PinA  5
#define encoder0PinB  6
#define encoder0Select 7

#define SX1509_RELEASE_BUTTON 8
#define SX1509_SOLENOID_HOT 9
#define SX1509_SOLENOID_COLD 10
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
    io.digitalWrite(SX1509_RELAY_HEATER, HIGH);
  }
  
 for (int i = 0; i < RequestRelayHotOff.length(); i++){
    if((char)payload[i] == RequestRelayHotOff[i]){
      k++;
    } 
  }
  if(k == RequestRelayHotOff.length()) {
    io.digitalWrite(SX1509_RELAY_HEATER, LOW);
  }  
/***************************************************************************************/


/********************************CONTROL COOLER*****************************************/
  for (int i = 0; i < RequestRelayColdOn.length(); i++){
    if((char)payload[i] == RequestRelayHotOn[i]){
      j++;
    } 
  }
  if(j == RequestRelayColdOn.length()) {
    io.digitalWrite(SX1509_RELAY_COOLER, HIGH);
  }

 for (int i = 0; i < RequestRelayColdOff.length(); i++){
    if((char)payload[i] == RequestRelayHotOff[i]){
      k++;
    } 
  }
  if(k == RequestRelayColdOff.length()) {
    io.digitalWrite(SX1509_RELAY_COOLER, LOW);
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
  double c = thermoArray[i].readFarenheit();
  if (isnan(c)) {
    Serial.println("Something wrong with thermocouple!");
  } else {
    Serial.print("Thermocouple ");
    Serial.print(i);
    Serial.print(". F = "); 
    Serial.println(c);
  }
}


double x;
int dis;
int last;


int inputTemp = 50;
int encoder0Pos = 100;
int encoder0PinALast = LOW;
int n = LOW;

unsigned long previousMillis = 0;

int selectTemp()
{  
  while(true)
  {
    yield();
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= 3)
    {
      previousMillis = currentMillis;
      n = io.digitalRead(encoder0PinA);
      if ((encoder0PinALast == LOW) && (n == HIGH))
      {
        if (io.digitalRead(encoder0PinB) == LOW)
        {
          encoder0Pos-=10;
        }
        else
        {
          encoder0Pos+=10;
        }
        if(encoder0Pos >= 155)
          encoder0Pos = 155;
        else if(encoder0Pos <= 50)
          encoder0Pos = 50;
        
        Serial.println(encoder0Pos, DEC);

//        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(encoder0Pos);
        
      }

      if (currentMillis - previousMillis >= 30000)
      {
      previousMillis = currentMillis;
      Serial.println("Time out for select temperature");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Time Out");
      delay(1000);
      
      return -10;
      }
      
      encoder0PinALast = n;
      if (!io.digitalRead(encoder0Select))
        return encoder0Pos;
    }
    
  }
}

int gcd (int a, int b) {
  int c;
  while ( a != 0 ) {
     c = a; a = b%a; b = c;
  }
  return b;
}

void setup() {


  if (!io.begin(SX1509_ADDRESS))
    while (1);
    
  io.pinMode(SX1509_RELAY_HEATER, OUTPUT);
  io.pinMode(SX1509_RELAY_COOLER, OUTPUT);
  io.pinMode(SX1509_MOTION0, INPUT);
  io.pinMode(SX1509_MOTION1, INPUT);
  io.pinMode(SX1509_MOTION2, INPUT);
  
  io.pinMode(encoder0Select, INPUT);
  io.digitalWrite(encoder0Select, HIGH);
  io.pinMode(encoder0PinA, INPUT); 
  io.digitalWrite(encoder0PinA, HIGH);       // turn on pull-up resistor
  io.pinMode(encoder0PinB, INPUT); 
  io.digitalWrite(encoder0PinB, HIGH);       // turn on pull-up resistor

  Serial.begin (115200);
  Serial.println("\nstart");
//  pinMode(3, FUNC_GPIO3);/
  


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
  yield();
  for (int c = 0; c <= 30 and WiFi.status() != WL_CONNECTED; ++c) {
    
    delay(500);
    Serial.print(".");
    lcd.print(".");  
    if (c == 30) {
      
      Serial.println();
      Serial.println("Connection Time Out...");
      Serial.println("Enter AP Mode...");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Time Out...");
      lcd.setCursor(0, 1);
      lcd.print("Enter AP Mode...");
      
      setup_wifi();
      char data[100] = "";
      data_setup(data);
      emem.saveData(data);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Saving Data");
      lcd.setCursor(0, 1);
      lcd.print("Rebooting");
      
      for (int i = 0; i < 3; ++i)
      {
        Serial.print(".");
        lcd.setCursor(10, 1);
        lcd.print(".");
      }
      lcd.clear();
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
//  yield(/);
}

char msg[50];
int value = 0;
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

double heaterTemp;
double coolerTemp;

//Thermocouple calibration constants
#define TC1_gain 0.988 //Thermocouple correction gain
#define TC1_offset -2.5  //Thermocouple correction offset
#define upper_hysteresis_1 1.5  //value for overshoot hysteresis (degrees)
#define lower_hysteresis_1 2  //value for undershoot hysteresis (degrees)
#define min_trans_time 5000

unsigned long lastswitcheventtime=0; //time in millis() since last switch event
unsigned long runtime = 0;
unsigned long lastruntime = 0;
byte TCError1=false;
byte overrideindicator=0;
byte RelayStatus_1=0;

void control(bool direct, int RelayCtrl_1_Pin, int thermoPin)
{
  int temp_read1=0; //initialize the temp variable for the averages
  double samples[sampleLoop]={0};
  for (int i=0; i<sampleLoop; i++)//read sequential samples
  {
   samples[i] = (TC1_gain*thermoArray[thermoPin].readFarenheit())+TC1_offset;  //Measurement and calibration of TC input 
  }

  for (int i=0; i<sampleLoop; i++) //average the sequential samples
  {
  temp_read1=samples[i]+temp_read1;
  }
  temp_read1=temp_read1/(double)sampleLoop;
  
  if(isnan(temp_read1)) //check for NAN, if this is not done, if the TC messes up, the controller can stick on!
  {
//    temp_read1=temp_set1; //fail safe!
    if (direct)
      temp_read1 = heaterThreshold;
    else
      temp_read1 = coolerThreshold;
    TCError1=true;
  }
  else
  {
    TCError1=false;
  }
 runtime=millis(); //set runtime
 if (lastruntime>runtime) //check for millis() rollover event, prepare accordingly, skip and wait until time reaccumulates
 {
  overrideindicator=1;
  lastruntime=runtime;
  delay (min_trans_time); //delay if overflow event is detected as failsafe
 }
 
  if (direct==true)
  {
    if (RelayStatus_1==0 && temp_read1<=heaterThreshold-lower_hysteresis_1 && min_trans_time<(lastruntime-runtime) && !overrideindicator)
    {
            digitalWrite(RelayCtrl_1_Pin, HIGH);
            lastswitcheventtime = runtime-lastswitcheventtime;  //reset transition time counter (verify no issue with millis() rollover)
            RelayStatus_1=1;  //toggle relay status indicator
            lastruntime=runtime;  //update lastruntime variable - used to check switchine period and to permit checking for millis() overflow event
            overrideindicator=0; //reset millis() overflow event indicator
            
    }
     else if (RelayStatus_1==1 && temp_read1>=heaterThreshold+upper_hysteresis_1 && min_trans_time<(lastruntime-runtime) && !overrideindicator)
      {
           digitalWrite(RelayCtrl_1_Pin, LOW);
           lastswitcheventtime =  runtime-lastswitcheventtime; //reset transition time counter (verify no issue with millis() rollover)
           RelayStatus_1=0;  //toggle relay status indicator
           lastruntime=runtime;   //update lastruntime variable - used to check switchine period and to permit checking for millis() overflow event
           overrideindicator=0; //reset millis() overflow event indicator
      } 
    else {}
  }
  
  if (direct==!true)
  {
    if (RelayStatus_1==1 && temp_read1<=coolerThreshold-lower_hysteresis_1 && min_trans_time<(lastruntime-runtime) && !overrideindicator)
    {
        digitalWrite(RelayCtrl_1_Pin, LOW);
        lastswitcheventtime = runtime-lastswitcheventtime;  //reset transition time counter (verify no issue with millis() rollover)
        RelayStatus_1=0;  //toggle relay status indicator
        lastruntime=runtime;  //update lastruntime variable - used to check switchine period and to permit checking for millis() overflow event
        overrideindicator=0; //reset millis() overflow event indicator
            
    }
     else if (RelayStatus_1==0 && temp_read1>=coolerThreshold+upper_hysteresis_1 && min_trans_time<(lastruntime-runtime) && !overrideindicator)
      {
           digitalWrite(RelayCtrl_1_Pin, HIGH);
           lastswitcheventtime = runtime-lastswitcheventtime;  //reset transition time counter (verify no issue with millis() rollover)
           RelayStatus_1=1;  //toggle relay status indicator
           lastruntime=runtime;  //update lastruntime variable - used to check switchine period and to permit checking for millis() overflow event
           overrideindicator=0; //reset millis() overflow event indicator
      } 
    else {}
  }
//  Serial.print("SetPoint(F): "); Serial.print(temp_set1); Serial.print(" ");
  Serial.print("AmbientCJTemp(C):"); //MAX31855 Internal Cold junction temp reading (in C) (roughly ambient temp to the IC)
  Serial.print(thermoArray[thermoPin].readInternal()); Serial.print(" "); //Read temp from TC controller internal cold junction  
  Serial.print("AvgCurrentTCTemp(F): "); Serial.print(temp_read1); Serial.print(" "); //Display averaged TC temperature
  Serial.print("RelayStatus: "); Serial.print(RelayStatus_1); Serial.print(" "); //Dis[play the present status of the thermal control relay
  Serial.print("TimeFromLastToPresentSwitchState(proportional ms): "); Serial.print(lastswitcheventtime); Serial.print(" "); Serial.print("TotalRuntime(ms): "); Serial.println(runtime);
}

double sampleTemp(int j)
{
  double tempRead = 0;
  int count = 0;
  for (int i = 0; i < sampleLoop; ++i)
  {
    if (!isnan(tempRead))
    {
      tempRead += (TC1_gain*thermoArray[j].readFarenheit())+TC1_offset;
      ++count;
    }
  }
  return tempRead / (double)count;
}

bool flag = false;
void loop() {
  if (!client.connected())
    reconnect();

  client.loop();

  x = analogRead(A0);

  while(millis() <= last + 20) {};
  
  last = millis();
  dis = 4800/(x-20);
  
  bool mix = false;
  if (flag || io.digitalRead(SX1509_MOTION0) || io.digitalRead(SX1509_MOTION1) || io.digitalRead(SX1509_MOTION2))
  {
    
    flag = false; 
    Serial.println("Detect Motion");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Detect Motion");

    int i = 0;
    while (io.digitalRead(encoder0Select))
    {
      delay(500);
      yield();
      ++i;
      Serial.print(".");
      lcd.print(".");
      if (i == 30)
      {
        Serial.println("\nFalse Motion");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("False Motion");
        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sleep Mode");
        break;
      }
    }
    if (!io.digitalRead(encoder0Select))
    {
      Serial.println("Please Select Desired Temperature");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Select Temp:");
      
      for(int i = 0; i < 3; ++i)
      {
        Serial.print(".");
        lcd.print(".");
        delay(1000);
      }
      int inputTemp = selectTemp();
      while(inputTemp < 0) {
        yield();
        inputTemp = selectTemp();
      }
      Serial.print("Temperature selected: ");
      Serial.println(inputTemp);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Selected Temperature:");
      lcd.setCursor(0, 1);
      lcd.print(inputTemp);
      
      mix = true;

      if (mix)
      {
//        heaterTemp = thermoArray[0].readFarenheit();

        heaterTemp = sampleTemp(0);
        coolerTemp = sampleTemp(1);

        
        int choice = 0;
        bool c;
        int stopPoint, diff;
    
        if (inputTemp > heaterTemp)
        {
          choice = 1;
          Serial.println("Turn on Heater");
          
//          lcd.clear();
//          lcd.setCursor(0, 0);
//          lcd.print("Heater: ON");
          
          while(heaterTemp < inputTemp)
          {
            yield();
            delay(1000);
            heaterTemp++;
          }

          Serial.println("Turn off Heater");
          
//          lcd.clear();
//          lcd.setCursor(0, 0);
//          lcd.print("Heater: OFF");
          
        }
        else if (inputTemp < coolerTemp)
        {
          choice = 2;
          Serial.println("Turn on Cooler");
          
//          lcd.clear();
//          lcd.setCursor(0, 0);
//          lcd.print("Cooler: ON");
          
          while(coolerTemp > inputTemp)
          {
            yield();
            delay(1000);
            coolerTemp--;
          }
          Serial.println("Turn off Cooler");

//          lcd.clear();
//          lcd.setCursor(0, 0);
//          lcd.print("Cooler: OFF");
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
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Ready to Dispense");
        lcd.setCursor(0, 1);
        while(io.digitalRead(SX1509_RELEASE_BUTTON))
        {
          delay(1000);
          yield();
          ++i;
          Serial.print(".");
          lcd.print(".");
          if (i == 30)
          {
            Serial.println("Time Out");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Time out");
            break;
          }
        }

        while(!io.digitalRead(SX1509_RELEASE_BUTTON))
        {
          yield();
          
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Dispensing Water");
          lcd.setCursor(0, 1);
          lcd.print(".");
          
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
            Serial.println("Close Hot Solenoid");
          }
          else if (choice == 0)
          {
            Serial.println("DEBUG");
          }
        }
        lcd.clear();
      }
    }
  } 
  else
  {
    Serial.println("No Motion Detected");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No Motion");
    
//    rtc.setTime(0, 0, 0, 6, 13, 01, 17);
    while(true)
    {
      yield();
//      delay(1000);

      if (io.digitalRead(SX1509_MOTION0) || io.digitalRead(SX1509_MOTION1) || io.digitalRead(SX1509_MOTION2))
      {
        Serial.println("Motion Detected. Reset1111111111111");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Detected Motion...");
        delay(3000);
        flag = true;
        break;
      }
      else
      {
        Serial.println("HERE");
        previousMillis = millis();
        unsigned long currentMillis = previousMillis;
        delay(1000);
        while (true)
        {
          yield();
          currentMillis = millis();
          
          control(true, SX1509_RELAY_HEATER, 0);
          control(false, SX1509_RELAY_COOLER, 1);
          
          if (currentMillis - previousMillis >= 10000)
          {
            Serial.println("Exceed 30 minutes. Turn Off");
            Serial.println("TURN OFF EVERYTHING");
            while(true)
            {
              yield();
              if (io.digitalRead(SX1509_MOTION0) || io.digitalRead(SX1509_MOTION1) || io.digitalRead(SX1509_MOTION2))
              {
                Serial.println("Motion Detected. Reset2222222222");
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Detected Motion...");
                delay(3000);
                flag = true;
                return;
              }             
            }
//            break;
          }
          if (io.digitalRead(SX1509_MOTION0) || io.digitalRead(SX1509_MOTION1) || io.digitalRead(SX1509_MOTION2))
          {
            Serial.println("Motion Detected. Reset2222222222");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Detected Motion...");
            delay(3000);
            flag = true;
            return;
          }
        }
//        rtc.readTime( & second, & minute, & hour, & dayOfWeek, & dayOfMonth, & month, & year);
//        int t = second + +60 * minute + 3600 * hour;
//        Serial.print(t);
//        Serial.print(" s\n");
//        if (t >= 10)
//        {
//          Serial.println("Exceed 30 minutes");
//          Serial.println("Turn off heater and cooler");
//
//          heaterTemp = sampleTemp(0);
//          coolerTemp = sampleTemp(1);
//          
//          if (heaterTemp < heaterThreshold)
//          {
//            Serial.println("Turn on heater");
//          }
//          else
//          {
//            Serial.println("Turn off heater");
//          }
//
//          if (coolerTemp > coolerThreshold)
//          {
//            Serial.println("Turn on cooler");
//          }
//          else
//          {
//            Serial.println("Turn off cooler");
//          }
//        }
      }
    }
  }
}
