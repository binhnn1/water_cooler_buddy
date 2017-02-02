#ifndef EMem_h
#define EMem_h

#include <EEPROM.h>
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

class EMem {
public:
	EMem();
	void saveData(char* data);
	void loadData();
	void dataSetup(char* data);
	String getWifiSsid();
	String getWifiPwd();
	String getMqttServer();
	String getMqttPort();
	String getMqttUser();
	String getMqttPwd();

private:
	String _wifiSsid;
	String _wifiPwd;
	String _mqttServer;
	String _mqttPort;
	String _mqttUser;
	String _mqttPwd;
};


#endif
