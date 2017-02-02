#include "EMem.h"

EMem::EMem() {}

void EMem::saveData(char* data) {
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

void EMem::loadData() {
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
        case 1: _wifiSsid = pch; break;
        case 2: _wifiPwd = pch; break;
        case 3: _mqttServer = pch; break;
        case 4: _mqttPort = pch; break;
        case 5: _mqttUser = pch; break;
        case 6: _mqttPwd = pch; break;
    }
    pch = strtok(NULL, "#");
  }
}


String EMem::getWifiSsid() {
	return _wifiSsid;
}

String EMem::getWifiPwd() {
	return _wifiPwd;
}

String EMem::getMqttServer() {
	return _mqttServer;
}

String EMem::getMqttPort() {
	return _mqttPort;
}

String EMem::getMqttUser() {
	return _mqttUser;
}

String EMem::getMqttPwd() {
	return _mqttPwd;
}
