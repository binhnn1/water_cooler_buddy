#ifndef RTC_DS3231_h
#define RTC_DS3231_h

#include <Wire.h>
#include <inttypes.h>
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

class RTC_DS3231 {
public:
    RTC_DS3231(uint8_t rtc_Addr);
    void init();
    void setTime(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year);
    void readTime(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year);
    void displayTime();


private:
    uint8_t _Addr;
    byte decToBcd(byte val);
    byte bcdToDec(byte val);
};


#endif
