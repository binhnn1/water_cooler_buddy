#include <Adafruit_MAX31855.h>
#include <Adafruit_PWMServoDriver.h>
#include <DNSServer.h>
#include <EMem.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
//#include <RTC_DS3231.h>
#include <SparkFunSX1509.h>
#include <SPI.h>
#include <WiFiManager.h>
#include <Wire.h>
//#include <ADE7953.h>s
#include <Button.h>
//unsigned long lastTime = 0;
//unsigned long currentTime = 0;
//float lastEnergy = 0;
//float currentEnergy = 0;
//const char* host = "52.53.184.247";
//const int httpPort = 8080;
//String url = "/Test/page1";
//#define thresH 135
//#define thresC 60
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

#define SX1509_RELEASE_BUTTON 12
#define SX1509_SOLENOID_HOT 10
#define SX1509_SOLENOID_COLD 9


//#define RESET_WIFI_BUTTON 15        // D8
//#define RESET_MACHINE_BUTTON 16      // D9

//Button resetWifiButton = Button(RESET_WIFI_BUTTON, PULLUP);
//Button resetButton = Button(RESET_MACHINE_BUTTON, PULLUP);
/***************************************************************************************/

#define heaterThreshold 160
#define coolerThreshold 45
/********************************SETUP FOR THERMOCOUPLES********************************/
#define MAXDO 0       // D3
#define MAXCLK 2      // D4
#define MAXCS0 14     // D5
#define MAXCS1 12     // D6
#define MAXCS2 13     // D7


//#define CURCS 15

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();


//ADE7953 myADE7953(CURCS, 1000000);

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
char* wifi_ssid = "UCInet Mobile Access";
char* wifi_pwd = "";

char* mqtt_server = "m11.cloudmqtt.com";
char* mqtt_port = "12201";
char* mqtt_user = "hculdksf";
char* mqtt_pwd = "GCGrhTnLhphv";
/***************************************************************************************/
EMem emem;
LiquidCrystal_I2C lcd(0x3F, 16, 2);
//RTC_DS3231 rtc(0x68);
SX1509 io;
WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient client(espClient);

double heaterTemp, coolerTemp, mixTemp;

char msg[50];
int value = 0;
int a = 0;
/***************************************************************************************/
//void printAnalog(boolean readStop){
//  if(readStop){
//    char messageBuff[100];
//    String temp = String(analogRead(A0),DEC);
//    //snprintf (msg, 75, temp, value);
//    temp.toCharArray(messageBuff,temp.length()+1);
//    client.publish("topic/2", messageBuff);
//  }
//}
/***************************************************************************************/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int j = 0;
  int k = 0;
  int l = 0;
  int m = 0;

  /********************************CONTROL HEATER*****************************************/
  for (int i = 0; i < RequestRelayHotOn.length(); i++) {
    if ((char)payload[i] == RequestRelayHotOn[i]) {
      j++;
    }
  }
  if (j == RequestRelayHotOn.length()) {
    Serial.println("TURN ON HEATER");
    io.digitalWrite(SX1509_RELAY_HEATER, HIGH);
  }

  for (int i = 0; i < RequestRelayHotOff.length(); i++) {
    if ((char)payload[i] == RequestRelayHotOff[i]) {
      k++;
    }
  }
  if (k == RequestRelayHotOff.length()) {
    Serial.println("TURN OFF HEATER");
    io.digitalWrite(SX1509_RELAY_HEATER, LOW);
  }
  /***************************************************************************************/

  /********************************CONTROL COOLER*****************************************/
  for (int i = 0; i < RequestRelayColdOn.length(); i++) {
    if ((char)payload[i] == RequestRelayColdOn[i]) {
      l++;
    }
  }
  if (l == RequestRelayColdOn.length()) {
    Serial.println("TURN ON COOLER");
    io.digitalWrite(SX1509_RELAY_COOLER, HIGH);
  }

  for (int i = 0; i < RequestRelayColdOff.length(); i++) {
    if ((char)payload[i] == RequestRelayColdOff[i]) {
      m++;
    }
  }
  if (m == RequestRelayColdOff.length()) {
    Serial.println("TURN OFF COOLER");
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
    pwm.setPWM(10, 4096, 0);
    if (client.connect("ESP8266Client", emem.getMqttUser().c_str(), emem.getMqttPwd().c_str())) {
      lcd.clear();
      lcd.setCursor(0, 0);

      pwm.setPWM(9, 4096, 0);

      lcd.print("Connected");
      Serial.println("connected");
      client.publish("topic/1", "publishing-yes");
      client.subscribe("topic/1");//subscribe to data from topic/1
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" will try connecting again in 2 secs");
      delay(2000);
      pwm.setPWM(10, 0, 4096);
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
  if (!wifiManager.autoConnect("DeviceConfig", "config11")) {
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
int encoder0Pos = 75;
int encoder0PinALast = LOW;
int n = LOW;

unsigned long previousMillis = 0;

int selectTemp()
{
  //  heaterTemp = sampleTemp(0);
  //  coolerTemp = sampleTemp(1);
  encoder0Pos = 75;
  pwm.setPWM(2, 4096, 0);
  while (true)
  {
    yield();
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= 1)
    {
      previousMillis = currentMillis;
      n = io.digitalRead(encoder0PinA);
      if ((encoder0PinALast == LOW) && (n == HIGH))
      {
        if (io.digitalRead(encoder0PinB) == LOW)
        {
          encoder0Pos -= 5;
        }
        else
        {
          encoder0Pos += 5;
        }
        delay(1000);
        if (encoder0Pos >= heaterTemp)
        {
          encoder0Pos = heaterTemp;
          pwm.setPWM(2, 0, 4096);
          pwm.setPWM(1, 4096, 0);
          pwm.setPWM(0, 4096, 0);
          delay(500);
          pwm.setPWM(1, 0, 4096);
          pwm.setPWM(0, 0, 4096);
          delay(500);
          pwm.setPWM(1, 4096, 0);
          pwm.setPWM(0, 4096, 0);
        }
        else if (encoder0Pos <= coolerTemp)
        {
          encoder0Pos = coolerTemp;
          pwm.setPWM(2, 0, 4096);
          pwm.setPWM(4, 4096, 0);
          pwm.setPWM(3, 4096, 0);
          delay(500);
          pwm.setPWM(4, 0, 4096);
          pwm.setPWM(3, 0, 4096);
          delay(500);
          pwm.setPWM(4, 4096, 0);
          pwm.setPWM(3, 4096, 0);
        }
        else
        {
          Serial.print("ENCODER: ");
          Serial.println(encoder0Pos);
          if (coolerTemp < encoder0Pos && encoder0Pos < 62)
          {
            pwm.setPWM(0, 0, 4096);
            pwm.setPWM(1, 0, 4096);
            pwm.setPWM(2, 0, 4096);
            pwm.setPWM(3, 4096, 0);
            pwm.setPWM(4, 4096, 0);
          }
          else if (62 <= encoder0Pos && encoder0Pos < 75)
          {
            pwm.setPWM(0, 0, 4096);
            pwm.setPWM(1, 0, 4096);
            pwm.setPWM(2, 0, 4096);
            pwm.setPWM(3, 4096, 0);
            pwm.setPWM(4, 0, 4096);
          }
          else if (75 <= encoder0Pos && encoder0Pos < 100)
          {
            pwm.setPWM(0, 0, 4096);
            pwm.setPWM(1, 0, 4096);
            pwm.setPWM(2, 4096, 0);
            pwm.setPWM(3, 0, 4096);
            pwm.setPWM(4, 0, 4096);
          }
          else if (100 <= encoder0Pos && encoder0Pos < 137)
          {
            pwm.setPWM(0, 0, 4096);
            pwm.setPWM(1, 4096, 0);
            pwm.setPWM(2, 0, 4096);
            pwm.setPWM(3, 0, 4096);
            pwm.setPWM(4, 0, 4096);
          }
          else
          {
            pwm.setPWM(0, 4096, 0);
            pwm.setPWM(1, 4096, 0);
            pwm.setPWM(2, 0, 4096);
            pwm.setPWM(3, 0, 4096);
            pwm.setPWM(4, 0, 4096);
          }
          //          a = (encoder0Pos - 50) / 10 + 1;
          //          Serial.println(a);
          //          if (a < 4)
          //          {
          //            pwm.setPWM(4, (a - 1) * (4096 / 2), 0);
          //            pwm.setPWM(3, 0, 4096);
          //            pwm.setPWM(2, 0, 4096);
          //            pwm.setPWM(1, 0, 4096);
          //            pwm.setPWM(0, 0, 4096);
          //          }
          //          else if (a < 6)
          //          {
          //            pwm.setPWM(4, 4096, 0);
          //            pwm.setPWM(3, (a - 3) * (4096 / 2), 0);
          //            pwm.setPWM(2, 0, 4096);
          //            pwm.setPWM(1, 0, 4096);
          //            pwm.setPWM(0, 0, 4096);
          //          }
          //          else if (a < 8)
          //          {
          //            pwm.setPWM(4, 4096, 0);
          //            pwm.setPWM(3, 4096, 0);
          //            pwm.setPWM(2, (a - 5) * (4096 / 2), 0);
          //            pwm.setPWM(1, 0, 4096);
          //            pwm.setPWM(0, 0, 4096);
          //          }
          //          else if (a < 10)
          //          {
          //            pwm.setPWM(4, 4096, 0);
          //            pwm.setPWM(3, 4096, 0);
          //            pwm.setPWM(2, 4096, 0);
          //            pwm.setPWM(1, (a - 7) * (4096 / 2), 0);
          //            pwm.setPWM(0, 0, 4096);
          //          }
          //          else if (a < 12)
          //          {
          //            pwm.setPWM(4, 4096, 0);
          //            pwm.setPWM(3, 4096, 0);
          //            pwm.setPWM(2, 4096, 0);
          //            pwm.setPWM(1, 4096, 0);
          //            pwm.setPWM(0, (a - 9) * (4096 / 2), 0);
          //          }
        }
        Serial.println(encoder0Pos, DEC);

        //        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(encoder0Pos);
        if (encoder0Pos < 100)
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Select Temp:");
          lcd.setCursor(0, 1);
          lcd.print(encoder0Pos);
          lcd.setCursor(3 , 1);
          lcd.print("degrees F");
        }
        else
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Select Temp:");
          lcd.setCursor(0, 1);
          lcd.print(encoder0Pos);
          lcd.setCursor(4 , 1);
          lcd.print("degrees F");
        }
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
      {
        pwm.setPWMFreq(1000);
        pwm.setPWM(4, 4096, 0);
        pwm.setPWM(3, 4096, 0);
        pwm.setPWM(2, 4096, 0);
        pwm.setPWM(1, 4096, 0);
        pwm.setPWM(0, 4096, 0);
        delay(500);
        pwm.setPWM(4, 0, 4096);
        pwm.setPWM(3, 0, 4096);
        pwm.setPWM(2, 0, 4096);
        pwm.setPWM(1, 0, 4096);
        pwm.setPWM(0, 0, 4096);
        delay(500);
        pwm.setPWM(4, 0, 4096);
        pwm.setPWM(3, 0, 4096);
        pwm.setPWM(2, 0, 4096);
        pwm.setPWM(1, 0, 4096);
        pwm.setPWM(0, 0, 4096);
        return encoder0Pos;
      }
    }

  }
}

int gcd (int a, int b) {
  float x = a / 10;
  a = (int)floor(x + 0.5) * 10;
  float y = b / 10;
  b = (int)floor(b + 0.5) * 10;
  int c;
  while ( a != 0 ) {
    c = a; a = b % a; b = c;
  }
  return b;
}


bool detectMotion()
{
  if (io.digitalRead(SX1509_MOTION0))
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Detected Motion:");
    lcd.setCursor(0, 1);
    lcd.print("Front");
    delay(1000);
    return true;
  }

  if (io.digitalRead(SX1509_MOTION1))
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Detected Motion:");
    lcd.setCursor(0, 1);
    lcd.print("Left");
    delay(1000);
    return true;
  }

  if (io.digitalRead(SX1509_MOTION2))
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Detected Motion:");
    lcd.setCursor(0, 1);
    lcd.print("Right");
    delay(1000);
    return true;
  }
  return false;
}


void setup()
{


  Serial.begin (115200);
  Serial.println("\nstart");

  Serial.print("Connecting to SX1509");
  while (!io.begin(SX1509_ADDRESS)) {
    yield();
    Serial.print(".");
  }

  Serial.println();
  io.pinMode(SX1509_RELAY_HEATER, OUTPUT);
  io.pinMode(SX1509_RELAY_COOLER, OUTPUT);
  io.pinMode(SX1509_MOTION0, INPUT);
  io.pinMode(SX1509_MOTION1, INPUT);
  io.pinMode(SX1509_MOTION2, INPUT);
  io.pinMode(SX1509_RELEASE_BUTTON, INPUT);
  io.digitalWrite(SX1509_RELEASE_BUTTON, HIGH);

  io.pinMode(encoder0Select, INPUT);
  io.digitalWrite(encoder0Select, HIGH);
  io.pinMode(encoder0PinA, INPUT);
  io.digitalWrite(encoder0PinA, HIGH);       // turn on pull-up resistor
  io.pinMode(encoder0PinB, INPUT);
  io.digitalWrite(encoder0PinB, HIGH);       // turn on pull-up resistor


  io.pinMode(SX1509_SOLENOID_HOT, OUTPUT);
  io.pinMode(SX1509_SOLENOID_COLD, OUTPUT);
  io.digitalWrite(SX1509_SOLENOID_HOT, HIGH);
  io.digitalWrite(SX1509_SOLENOID_COLD, HIGH);
  io.digitalWrite(SX1509_RELAY_HEATER, LOW);
  io.digitalWrite(SX1509_RELAY_COOLER, LOW);

  //  pinMode(3, FUNC_GPIO3);
  //  pinMode(13, FUNC_GPIO13)




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



  pwm.begin();
  pwm.setPWMFreq(1000);

  pwm.setPWM(0, 0, 4096);
  pwm.setPWM(1, 0, 4096);
  pwm.setPWM(2, 0, 4096);
  pwm.setPWM(3, 0, 4096);
  pwm.setPWM(4, 0, 4096);
  pwm.setPWM(5, 0, 4096);
  pwm.setPWM(6, 0, 4096);
  pwm.setPWM(7, 0, 4096);
  pwm.setPWM(8, 0, 4096);
  pwm.setPWM(9, 0, 4096);
  pwm.setPWM(10, 0, 4096);



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
  //  WiFi.begin("iPhone", "123456789");
  yield();
  for (int c = 0; c <= 30 and WiFi.status() != WL_CONNECTED; ++c) {

    pwm.setPWM(10, 4096, 0);
    delay(500);
    pwm.setPWM(10, 0, 4096);

    Serial.print(".");
    lcd.print(".");
    if (c == 30) {


      pwm.setPWM(8, 4096, 0);

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

  pwm.setPWM(9, 4096, 0);

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

  delay(200);
  SPI.begin();
  delay(200);
  //  myADE7953.initialize();

  //  WiFiClient client;
  //  if (!client.connect(host, httpPort)) {
  //    Serial.println("connection failed");
  //    return;
  //  }
  //
  //  client.print(String("POST ") + url + "?energy=setup" + " HTTP/1.1\r\n" +
  //               "Host: " + host + "\r\n" +
  //               "Connection: close\r\n\r\n");
  //  delay(10);
  //
  //  Serial.print("Setup Respond:");
  //  int c = 0;
  //  while (client.available()) {
  //    String line = client.readStringUntil('\r');
  //    ++c;
  //    if (c >= 8 ) {
  //      Serial.print(line);
  //      lastEnergy = line.toFloat();
  //    }
  //  }

}




//Thermocouple calibration constants
#define TC1_gain 0.988 //Thermocouple correction gain
#define TC1_offset -2.5  //Thermocouple correction offset
#define upper_hysteresis_1 1.5  //value for overshoot hysteresis (degrees)
#define lower_hysteresis_1 2  //value for undershoot hysteresis (degrees)
#define min_trans_time 5000

unsigned long lastswitcheventtimeH = 0; //time in millis() since last switch event
unsigned long runtimeH = 0;
unsigned long lastruntimeH = 0;

unsigned long lastswitcheventtimeC = 0; //time in millis() since last switch event
unsigned long runtimeC = 0;
unsigned long lastruntimeC = 0;

byte TCError1 = false;
byte overrideindicator = 0;
byte RelayStatus_1 = 0;
byte RelayStatus_C = 0;

void control(bool direct, int RelayCtrl_1_Pin, int thermoPin)
{
  client.loop();
  delay(1000);


  int temp_read1 = 0; //initialize the temp variable for the averages
  double samples[sampleLoop] = {0};

  for (int i = 0; i < sampleLoop;) //read sequential samples
  {
    //    double readTemp = (TC1_gain*thermoArray[thermoPin].readFarenheit())+TC1_offset;
    double readTemp = thermoArray[thermoPin].readFarenheit();

    int falseCount = 0;

    if (!isnan(readTemp) && 0 < readTemp < 300)
    {
      if (thermoPin == 1)
        readTemp += 10;
      Serial.println(readTemp);
      samples[i] =  readTemp; //Measurement and calibration of TC input
      ++i;
    } else
    {
      falseCount++;
      if (falseCount == 20)
      {
        Serial.println("ERROR WITH THERMOCOUPLE");
        ESP.reset();
      }
    }

    //    samples[i] = thermoArray[thermoPin].readFarenheit();
  }

  for (int i = 0; i < sampleLoop; i++) //average the sequential samples
  {
    temp_read1 = samples[i] + temp_read1;
  }
  temp_read1 = temp_read1 / (double)sampleLoop;


  if (isnan(temp_read1) && temp_read1 > 300 && temp_read1 < 0) //check for NAN, if this is not done, if the TC messes up, the controller can stick on!
  {
    //    temp_read1=temp_set1; //fail safe!
    if (direct)
      temp_read1 = heaterThreshold;
    else
      temp_read1 = coolerThreshold;
    TCError1 = true;
  }
  else
  {
    TCError1 = false;
  }

  if (thermoPin == 3)
  {
    Serial.print("AmbientCJTemp(C):"); //MAX31855 Internal Cold junction temp reading (in C) (roughly ambient temp to the IC)
    Serial.print(thermoArray[thermoPin].readInternal()); Serial.print(" "); //Read temp from TC controller internal cold junction
    Serial.print("AvgCurrentTCTemp(F): "); Serial.print(temp_read1); Serial.print(" "); //Display averaged TC temperature
    Serial.print("RelayStatusHot: "); Serial.print(RelayStatus_1); Serial.print(" "); Serial.print("RelayStatusCold: "); Serial.print(RelayStatus_C); Serial.println(" "); //Dis[play the present status of the thermal control relay
    return;
  }
  if (direct)
  {
    runtimeH = millis(); //set runtime
    if (lastruntimeH > runtimeH) //check for millis() rollover event, prepare accordingly, skip and wait until time reaccumulates
    {
      overrideindicator = 1;
      lastruntimeH = runtimeH;
      delay (min_trans_time); //delay if overflow event is detected as failsafe
    }

    if (RelayStatus_1 == 0 && temp_read1 <= heaterThreshold - lower_hysteresis_1 && min_trans_time < (lastruntimeH - runtimeH) && !overrideindicator)
    {
      snprintf (msg, 75, "RqstHotON", value);
      client.publish("topic/1", msg);
      //    delay(1000);
      //    client.loop();
      //    io.digitalWrite(RelayCtrl_1_Pin, HIGH);

      lastswitcheventtimeH = runtimeH - lastswitcheventtimeH; //reset transition time counter (verify no issue with millis() rollover)
      RelayStatus_1 = 1; //toggle relay status indicator
      lastruntimeH = runtimeH; //update lastruntime variable - used to check switchine period and to permit checking for millis() overflow event
      overrideindicator = 0; //reset millis() overflow event indicator

    }
    else if (RelayStatus_1 == 1 && temp_read1 >= heaterThreshold + upper_hysteresis_1 && min_trans_time < (lastruntimeH - runtimeH) && !overrideindicator)
    {

      snprintf (msg, 75, "RqstHotOFF", value);
      client.publish("topic/1", msg);
      //    delay(1000);
      //    client.loop();
      //    io.digitalWrite(RelayCtrl_1_Pin, LOW);
      lastswitcheventtimeH =  runtimeH - lastswitcheventtimeH; //reset transition time counter (verify no issue with millis() rollover)
      RelayStatus_1 = 0; //toggle relay status indicator
      lastruntimeH = runtimeH; //update lastruntime variable - used to check switchine period and to permit checking for millis() overflow event
      overrideindicator = 0; //reset millis() overflow event indicator

    }
    else {}
  }
  else
  {
    runtimeC = millis(); //set runtime
    if (lastruntimeC > runtimeC) //check for millis() rollover event, prepare accordingly, skip and wait until time reaccumulates
    {
      overrideindicator = 1;
      lastruntimeC = runtimeC;
      delay (min_trans_time); //delay if overflow event is detected as failsafe
    }
    if (RelayStatus_C == 1 && temp_read1 <= coolerThreshold - lower_hysteresis_1 && min_trans_time < (lastruntimeC - runtimeC) && !overrideindicator)
    {
      snprintf (msg, 75, "RqstColdOFF", value);
      client.publish("topic/1", msg);
      //    delay(1000);
      //    client.loop();
      lastswitcheventtimeC = runtimeC - lastswitcheventtimeC; //reset transition time counter (verify no issue with millis() rollover)
      RelayStatus_C = 0; //toggle relay status indicator
      lastruntimeC = runtimeC; //update lastruntime variable - used to check switchine period and to permit checking for millis() overflow event
      overrideindicator = 0; //reset millis() overflow event indicator

    }
    else if (RelayStatus_C == 0 && temp_read1 >= coolerThreshold + upper_hysteresis_1 && min_trans_time < (lastruntimeC - runtimeC) && !overrideindicator)
    {
      snprintf (msg, 75, "RqstColdON", value);
      client.publish("topic/1", msg);
      //    delay(1000);
      //    client.loop();
      //       io.digitalWrite(RelayCtrl_1_Pin, HIGH);
      lastswitcheventtimeC = runtimeC - lastswitcheventtimeC; //reset transition time counter (verify no issue with millis() rollover)
      RelayStatus_C = 1; //toggle relay status indicator
      lastruntimeC = runtimeC; //update lastruntime variable - used to check switchine period and to permit checking for millis() overflow event
      overrideindicator = 0; //reset millis() overflow event indicator
    }
    else {}
  }

  //  Serial.print("SetPoint(F): "); Serial.print(temp_set1); Serial.print(" ");
  Serial.print("AmbientCJTemp(C):"); //MAX31855 Internal Cold junction temp reading (in C) (roughly ambient temp to the IC)
  Serial.print(thermoArray[thermoPin].readInternal()); Serial.print(" "); //Read temp from TC controller internal cold junction
  Serial.print("AvgCurrentTCTemp(F): "); Serial.print(temp_read1); Serial.print(" "); //Display averaged TC temperature
  Serial.print("RelayStatusHot: "); Serial.print(RelayStatus_1); Serial.print(" "); Serial.print("RelayStatusCold: "); Serial.print(RelayStatus_C); Serial.println(" "); //Dis[play the present status of the thermal control relay
  //  Serial.print("TimeFromLastToPresentSwitchState(proportional ms): "); Serial.print(lastswitcheventtime); Serial.print(" "); Serial.print("TotalRuntime(ms): "); Serial.println(runtime);
}

double sampleTemp(int j)
{
  double sum = 0;
  for (int i = 0; i < sampleLoop;)
  {
    double tempRead = thermoArray[j].readFarenheit();
    if (!isnan(tempRead) && 0 < tempRead < 300)
    {
      if (j == 1)
        tempRead += 10;
      Serial.println(tempRead);

      sum += tempRead;
      ++i;
    }
  }
  return sum / (double)sampleLoop;
}

//bool flag = false;
String readString;


//void printCurrentDetails() {
//  currentTime = millis();
//  unsigned long interval = currentTime - lastTime;
//  lastTime = currentTime;
//
//  Wire.requestFrom(8, 100);    // request 6 bytes from slave device #8
//
//
//  Serial.println();
//  Serial.println("Current Details: ");
//  int count = 0;
//  char data[20] = "";
//  while (Wire.available()) { // slave may send less than requested
//    char c = (Wire.read()); // receive a byte as character
//    if (c == '#')
//      break;
//    strncat(data, &c, 1);
//  }
//  Serial.println(data);
//
//  //  char* pch;
//  //  count = 0;
//  //  pch = strtok(data, "#");
//  //  float activePower;
//  //  while (pch != NULL) {
//  //    ++count;
//  //    switch(count) {
//  //        case 1: Serial.print("IrmsA (mA): "); Serial.println(pch); break;
//  //        case 2: Serial.print("Vrms (V): "); Serial.println(pch); break;
//  //        case 3: Serial.print("Apparent Power A (mW): "); Serial.println(pch); break;
//  //        case 4: Serial.print("Active Power A (mW): "); Serial.println(pch); activePower = atof(pch); break;
//  //        case 5: Serial.print("Reactive Power A (mW): "); Serial.println(pch); break;
//  ////        case 6: Serial.print("Power Factor A (x100): "); Serial.println(pch); break;
//  ////        case 7: Serial.print("Active Energy A (hex): "); Serial.println(pch); break;
//  //    }
//  //    pch = strtok(NULL, "#");
//  //  }
//
//  float activePower = atof(data);
//  Serial.print("Interval: "); Serial.println(interval / 1000);
//  Serial.print("Last Energy: "); Serial.println(lastEnergy);
//  currentEnergy = activePower * (interval / 1000) / 3600 + lastEnergy;
//  lastEnergy = currentEnergy;
//  Serial.print("Energy Consumed: ");
//  Serial.println(currentEnergy);
//
//  WiFiClient client;
//  if (!client.connect(host, httpPort)) {
//    Serial.println("connection failed");
//    return;
//  }
//
//  client.print(String("POST ") + url + "?energy=" + currentEnergy + " HTTP/1.1\r\n" +
//               "Host: " + host + "\r\n" +
//               "Connection: close\r\n\r\n");
//  delay(10);
//
//  Serial.println("Respond:");
//  int c = 0;
//  while (client.available()) {
//    String line = client.readStringUntil('\r');
//    ++c;
//    if (c >= 8 )
//      Serial.print(line);
//  }
//
//
//  Serial.println();
//}

bool sleep = false;
bool operate = false;
bool idle = true;

void loop() {

  if (!client.connected())
    reconnect();

  client.loop();


  //  printCurrentDetails();

  x = analogRead(A0);

  while (millis() <= last + 20) {};

  last = millis();
  dis = 4800 / (x - 20);
  bool mix = false;

  //  if (resetButton.uniquePress())
  //  {
  //    lcd.clear();
  //    lcd.setCursor(0, 0);
  //    lcd.print("Reset Button Pressed");
  //
  //    int rst = 0;
  //    while(resetButton.isPressed())
  //    {
  //      delay(1000);
  //      rst++;
  //      Serial.print("Reset Machine Button: ");
  //      Serial.println(rst);
  //      lcd.setCursor(0, 1);
  //      lcd.print(rst);
  //    }
  //
  //    if (rst > 10)
  //    {
  //      lcd.clear();
  //      lcd.setCursor(0, 0);
  //      lcd.print("Hard Reset");
  //      Serial.println("HARD RESET");
  //      delay(1000);
  //    }
  //    else if (5 < rst < 10)
  //    {
  //      lcd.clear();
  //      lcd.setCursor(0, 0);
  //      lcd.print("Soft Reset");
  //      Serial.println("SOFT RESET");
  //      delay(1000);
  //    }
  //  }
  //
  //
  //  if (resetWifiButton.uniquePress())
  //  {
  //    lcd.clear();
  //    lcd.setCursor(0, 0);
  //    lcd.print("Reset Wifi Pressed");
  //
  //    int rst = 0;
  //    while(resetWifiButton.isPressed())
  //    {
  //      delay(1000);
  //      rst++;
  //      Serial.print("Reset Wifi Button: ");
  //      Serial.println(rst);
  //      lcd.setCursor(0, 1);
  //      lcd.print(rst);
  //    }
  //    if (rst >= 5)
  //    {
  //      lcd.clear();
  //      lcd.setCursor(0, 0);
  //      lcd.print("Reset Network");
  //      Serial.println("RESET NETWORK");
  //      delay(1000);
  //    }
  //  }

  if (operate)
  {

    pwm.setPWM(5, 0, 4096);
    pwm.setPWM(6, 4096, 0);
    pwm.setPWM(7, 0, 4096);

    Serial.println("Operating Mode");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Operating Mode");

    lcd.setCursor(0, 1);
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
        Serial.println("\nFalse Motion. Enter Idle Mode");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("False Motion");
        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Back To Idle");

        //        idle = true;
        //        operate = false;
        //        idle = false;
        //
        //        previousMillis = millis();
        //
        //        return;


        operate = false;
        sleep = false;
        idle = true;

        previousMillis = millis();
        return;
      }
    }
    if (!io.digitalRead(encoder0Select))
    {
      Serial.println("Select Temperature");

      heaterTemp = sampleTemp(0);
      float x = heaterTemp / 10;
      heaterTemp = (int)floor(x + 0.5) * 10;
      coolerTemp = sampleTemp(1);
      float y = coolerTemp / 10;
      coolerTemp = (int)floor(y + 0.5) * 10;

      Serial.print("Heater: "); Serial.println(heaterTemp);
      Serial.print("Cooler: "); Serial.println(coolerTemp);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Select Temp:");

      //      for(int i = 0; i < 3; ++i)
      //      {
      //        Serial.print(".");
      //        lcd.print(".");
      //        delay(1000);
      //      }
      int inputTemp = selectTemp();
      while (inputTemp < 0) {
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
      if (inputTemp < 100)
      {
        lcd.setCursor(3 , 1);
        lcd.print("degrees F");
      }
      else
      {
        lcd.setCursor(4 , 1);
        lcd.print("degrees F");
      }

      mix = true;

      if (mix)
      {
        //        heaterTemp = thermoArray[0].readFarenheit();

        //        heaterTemp = sampleTemp(0);
        //        coolerTemp = sampleTempmp(1);


        int choice = 0;
        bool c;
        int stopPoint, diff;
        int hotPortion, coldPortion;
        if (inputTemp >= heaterTemp)
        {
          choice = 1;
          //          Serial.println("Turn on Heater");
          //          snprintf (msg, 75, "RqstHotON", value);
          //          client.publish("topic/1", msg);
          //          delay(1000);
          //          client.loop();
          //          io.digitalWrite(SX1509_RELAY_HEATER, HIGH);
          //          lcd.clear();
          //          lcd.setCursor(0, 0);
          //          lcd.print("Heater: ON");
          //
          //          while(heaterTemp < inputTemp)
          //          {
          //
          //            yield();
          //            delay(1000);
          //            client.loop();
          //            heaterTemp = sampleTemp(0);
          //            Serial.print("Current heater temp: ");
          //            Serial.println(heaterTemp);
          //          }
          ////          client.loop();
          //          Serial.println("Turn off Heater");
          //          snprintf (msg, 75, "RqstHotOFF", value);
          //          client.publish("topic/1", msg);
          //          delay(1000);
          //          client.loop();
          //          delay(1000);
          ////          io.digitalWrite(SX1509_RELAY_HEATER, LOW);
          //
          ////          lcd.clear();
          ////          lcd.setCursor(0, 0);
          ////          lcd.print("Heater: OFF");
          //
        }
        else if (inputTemp <= coolerTemp)
        {
          choice = 2;
          //          Serial.println("Turn on Cooler");
          //          snprintf (msg, 75, "RqstColdON", value);
          //          client.publish("topic/1", msg);
          ////          delay(5000);
          ////          client.loop();
          ////          io.digitalWrite(SX1509_RELAY_COOLER, HIGH);
          ////          lcd.clear();
          ////          lcd.setCursor(0, 0);
          ////          lcd.print("Cooler: ON");
          //
          //          while(coolerTemp > inputTemp)
          //          {
          //            yield();
          //            delay(1000);
          //            client.loop();
          //            coolerTemp = sampleTemp(1);
          //            Serial.print("Current cooler temp: ");
          //            Serial.println(coolerTemp);
          //          }
          //          Serial.println("Turn off Cooler");
          //          snprintf (msg, 75, "RqstColdOFF", value);
          //          client.publish("topic/1", msg);
          //          delay(1000);
          //          client.loop();
          //          delay(1000);
          ////          io.digitalWrite(SX1509_RELAY_COOLER, LOW);
          ////          lcd.clear();
          ////          lcd.setCursor(0, 0);
          ////          lcd.print("Cooler: OFF");
        }
        else
        {
          choice = 3;

          //          heaterTemp = sampleTemp(0);
          //          coolerTemp = sampleTemp(1);
          //          mixTemp = sampleTemp(2);

          hotPortion = inputTemp - coolerTemp;
          coldPortion = heaterTemp - inputTemp;
          int gcdr = gcd(hotPortion, coldPortion);
          hotPortion = hotPortion / gcdr;
          coldPortion = coldPortion / gcdr;


          Serial.print("Heater Temp = "); Serial.print(heaterTemp); Serial.print(" Hot Portion = "); Serial.println(hotPortion);
          Serial.print("Cooler Temp = "); Serial.print(coolerTemp); Serial.print(" Cold Portion = "); Serial.println(coldPortion);
          //          Serial.print("Mix Temp = "); Serial.println(mixTemp);

          if (hotPortion == 0 || coldPortion == 0)
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("ERROR CODE: 01");
            lcd.setCursor(1, 0);
            lcd.print("REBOOT...");
            delay(1000);
            return;
          }

          if (hotPortion > coldPortion)
          {
            stopPoint = coldPortion;
            diff = hotPortion - coldPortion;
            c = true;
          }
          else
          {
            stopPoint = hotPortion;
            diff = coldPortion - hotPortion;
            c = false;
          }
        }

        i = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Ready to Dispense");
        lcd.setCursor(0, 1);
        while (io.digitalRead(SX1509_RELEASE_BUTTON))
        {
          yield();
          delay(1000);
          ++i;
          Serial.print(".");
          lcd.print(".");
          if (i == 30)
          {
            Serial.println("Time Out. Enter Idle Mode");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Time out");


            idle = true;
            operate = false;
            sleep = false;

            previousMillis = millis();
            return;
          }
        }

        while (!io.digitalRead(SX1509_RELEASE_BUTTON))
        {
          yield();

          pwm.setPWM(6, 4096, 0);

          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Dispensing Water");
          lcd.setCursor(0, 1);
          lcd.print(".");

          if (choice == 1)
          {
            Serial.println("Open Hot Solenoid");
            io.digitalWrite(SX1509_SOLENOID_HOT, LOW);
          }
          else if (choice == 2)
          {
            Serial.println("Open Cold Solenoid");
            io.digitalWrite(SX1509_SOLENOID_COLD, LOW);
          }
          else if (choice == 3)
          {
//            if (mixTemp > inputTemp)
//            {
//              Serial.println("\nOpen Cold Solenoid");
//              io.digitalWrite(SX1509_SOLENOID_COLD, LOW);
//              delay(stopPoint * 200 / hotPortion);
//            }
//            else
//            {
//              Serial.println("\nOpen Hot Solenoid");
//              io.digitalWrite(SX1509_SOLENOID_HOT, LOW);
//              delay(stopPoint * 200 / coldPortion);
//            }

            Serial.println("\nOpen Hot Solenoid");
            io.digitalWrite(SX1509_SOLENOID_HOT, LOW);
            Serial.println("Open Cold Solenoid");
            io.digitalWrite(SX1509_SOLENOID_COLD, LOW);
            //            delay(stopPoint * 500);
            if (c)
            {
              delay(stopPoint * 1000 / hotPortion);
              Serial.println("Close Cold Solenoid");
              io.digitalWrite(SX1509_SOLENOID_COLD, HIGH);
              delay(diff * 1000 / hotPortion);
            }
            else
            {
              delay(stopPoint * 1000 / coldPortion);
              Serial.println("Close Hot Solenoid");
              io.digitalWrite(SX1509_SOLENOID_HOT, HIGH);
              delay(diff * 1000 / coldPortion);
            }

          }
          else if (choice == 0)
          {
            Serial.println("DEBUG");
          }
        }
        Serial.println("Close Cold Solenoid");
        Serial.println("Close Hot Solenoid");
        io.digitalWrite(SX1509_SOLENOID_HOT, HIGH);
        io.digitalWrite(SX1509_SOLENOID_COLD, HIGH);
        lcd.clear();
      }
    }
  }


  if (idle)
  {
    Serial.println("Standby Mode");


    pwm.setPWM(5, 0, 4096);
    pwm.setPWM(6, 0, 4096);
    pwm.setPWM(7, 4096, 0);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Standby Mode");


    if (detectMotion())
    {
      Serial.println("Motion Detected. Reset1111111111111");

      io.digitalWrite(SX1509_RELAY_HEATER, HIGH);
      io.digitalWrite(SX1509_RELAY_COOLER, HIGH);

      operate = true;
      idle = false;
      sleep = false;

      //      flag = true;
      return;
    }
    else
    {
      Serial.println("HERE");

      delay(1000);

      unsigned long currentMillis = millis();

      Serial.print("Current time: ");
      Serial.println(currentMillis - previousMillis);

      if (currentMillis - previousMillis >= 1800000)
      {
        Serial.println("Exceed 30 minutes. Turn Off");
        Serial.println("TURN OFF EVERYTHING");

        snprintf (msg, 75, "RqstHotOFF", value);
        client.publish("topic/1", msg);

        snprintf (msg, 75, "RqstColdOFF", value);
        client.publish("topic/1", msg);

        sleep = true;
        operate = false;
        idle = false;

        return;
      }
    }

    Serial.print("HEATER: ");
    control(true, SX1509_RELAY_HEATER, 0);
    Serial.print("COOLER: ");
    control(false, SX1509_RELAY_COOLER, 1);
    //    Serial.print("MIXER: ");
    //    control(false, SX1509_RELAY_COOLER, 3);

  }

  if (sleep)
  {

    pwm.setPWM(5, 0, 4096);
    pwm.setPWM(6, 0, 4096);
    pwm.setPWM(7, 0, 4096);


    Serial.println("Sleep Mode");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sleep Mode");

    if (detectMotion())
    {

      Serial.println("Motion Detected. Reset2222222222");

      //      Serial.print("HEATER: ");
      //      control(true, SX1509_RELAY_HEATER, 0);
      //      Serial.print("COOLER: ");
      //      control(false, SX1509_RELAY_COOLER, 1);
      //      Serial.print("MIXER: ");
      //      control(false, SX1509_RELAY_COOLER, 3);

      io.digitalWrite(SX1509_RELAY_HEATER, HIGH);
      io.digitalWrite(SX1509_RELAY_COOLER, HIGH);

      operate = true;
      sleep = false;
      idle = false;

      return;
    }
  }
}
