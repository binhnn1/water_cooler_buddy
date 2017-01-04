#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>

////The following include is for led FSMs
//#include <FiniteStateMachine.h>
//#include <LED.h>

// The following include is for storing data to EEPROM
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);
#define DS3231_I2C_ADDRESS 0x68


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
char* wifi_ssid = "Bears' Cave";
char* wifi_pwd = "06152009usa";

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
// Real Time Clock
/***********************************************************************************/
byte decToBcd(byte val) {
  return( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val) {
  return( (val/16*10) + (val%16) );
}

void setDS3231time(byte second, byte minute, byte hour)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
 /* Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  */
  Wire.endTransmission();
}

void readDS3231time(byte *second, byte *minute, byte *hour) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
}

void displayTime() {
  byte second, minute, hour;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour);
  // send it to the serial monitor
  Serial.print(hour, DEC);
  // convert the byte variable to a decimal number when displayed
  Serial.print(":");
  if (minute<10)
  {
    Serial.print("0");
  }
  Serial.print(minute, DEC);
  Serial.print(":");
  if (second<10)
  {
    Serial.print("0");
  }
  Serial.print(second, DEC);
  Serial.println(" ");
}
/***********************************************************************************/

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
  Serial.println("Read data from EEPROM");
  EEPROM.begin(512);
  char data[100] = "";
  int count = 0;
  int address = 0;
  while (count < 6)
  {
    char read_char = (char)EEPROM.read(address);
    delay(1);
    strncat(data, &read_char, 1);
    if (read_char == '#')
      ++count;
    ++address;
  }
  Serial.println("Read data complete");
  Serial.println(data);
  delay(100);


  char* pch;
  count = 0;
  pch = strtok(data, "#");
  while (pch != NULL) {
    ++count;
    switch(count) {
        case 1: strcpy(wifi_ssid, pch); break;
        case 2: strcpy(wifi_pwd, pch); break;
        case 3: strcpy(mqtt_server, pch); break;
        case 4: strcpy(mqtt_port, pch); break;
        case 5: strcpy(mqtt_user, pch); break;
        case 6: strcpy(mqtt_pwd, pch); break;
    }
    pch = strtok(NULL, "#");
  }
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

double x;
int dis;
int last;

/***********************************************************************************/

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

  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print(wifi_ssid);
  
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  
  WiFi.begin(wifi_ssid, wifi_pwd);

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
      save_data(data);
      delay(1000);
      ESP.reset();
    }
  }


  delay(1000);
  lcd.clear();
  
  Serial.println("connected");

  lcd.setCursor(0, 0);
  lcd.print("connected");
  
  client.setServer(mqtt_server, atoi(mqtt_port));
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
  displayTime();
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

    snprintf (msg, 75, "StatusOFF", value);
    client.publish("topic/1", msg);
    snprintf (msg, 75, "RqstON", value);
    client.publish("topic/1", msg);

    
    setDS3231time(0,0,0);
  } else {
    Serial.println("NO MOTION");
    Serial.println("RESET CLOCK");
    setDS3231time(0,0,0);
    while(1) {
      byte second, minute, hour;
      readDS3231time(&second, &minute, &hour);
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
        break;
      } else {
        if (t >= 20) {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Exceed 30 minutes");
          lcd.setCursor(0, 1);
          lcd.print("Turn OFF");


//          digitalWrite(relayMain_pin, LOW);

          snprintf (msg, 75, "StatusON", value);
          client.publish("topic/1", msg);
          snprintf (msg, 75, "RqstOFF", value);
          client.publish("topic/1", msg);
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
