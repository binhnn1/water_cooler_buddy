#include "RTC_DS3231.h"

#include <inttypes.h>



RTC_DS3231::RTC_DS3231(uint8_t rtc_Addr) {
	_Addr = rtc_Addr;
}

void RTC_DS3231::init() {
	Wire.begin();
}

// Convert normal decimal numbers to binary coded decimal
byte RTC_DS3231::decToBcd(byte val) {
	return( (val/10*16) + (val%10) );
}

// Convert binary coded decimal to normal decimal numbers
byte RTC_DS3231::bcdToDec(byte val) {
	return( (val/16*10) + (val%16) );
}

void RTC_DS3231::setTime(byte second, byte minute, byte hour, byte dayOfWeek, byte      dayOfMonth, byte month, byte year) {
	Wire.beginTransmission(_Addr);
	Wire.write(0); // set next input to start at the seconds register
	Wire.write(decToBcd(second)); // set seconds
	Wire.write(decToBcd(minute)); // set minutes
	Wire.write(decToBcd(hour)); // set hours
	Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
	Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
	Wire.write(decToBcd(month)); // set month
	Wire.write(decToBcd(year)); // set year (0 to 99)
	Wire.endTransmission();
}
     

void RTC_DS3231::readTime(byte *second, byte *minute, byte *hour, byte *dayOfWeek,      byte *dayOfMonth, byte *month, byte *year) {
	Wire.beginTransmission(_Addr);
	Wire.write(0); // set DS3231 register pointer to 00h
	Wire.endTransmission();
	Wire.requestFrom(_Addr, (uint8_t)7);
	// request seven bytes of data from DS3231 starting from register 00h
	*second = bcdToDec(Wire.read() & 0x7f);
	*minute = bcdToDec(Wire.read());
	*hour = bcdToDec(Wire.read() & 0x3f);
	*dayOfWeek = bcdToDec(Wire.read());
	*dayOfMonth = bcdToDec(Wire.read());
	*month = bcdToDec(Wire.read());
	*year = bcdToDec(Wire.read());
}
     
void RTC_DS3231::displayTime() {
 byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  
	// retrieve data from DS3231
	readTime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
	// send it to the serial monitor
	Serial.print(hour, DEC);
	// convert the byte variable to a decimal number when displayed
	Serial.print(":");
	if (minute<10) {
		Serial.print("0");
	}
	Serial.print(minute, DEC);
	Serial.print(":");
	if (second<10) {
		Serial.print("0");
	}
	Serial.print(second, DEC);
	Serial.print(" ");
	Serial.print(dayOfMonth, DEC);
	Serial.print("/");
	Serial.print(month, DEC);
	Serial.print("/");
	Serial.print(year, DEC);
	Serial.print(" Day of week: ");
	switch(dayOfWeek) {
	case 1:
		Serial.println("Sunday");
		break;
	case 2:
		Serial.println("Monday");
		break;
	case 3:
		Serial.println("Tuesday");
		break;
	case 4:
		Serial.println("Wednesday");
		break;
	case 5:
		Serial.println("Thursday");
		break;
	case 6:
		Serial.println("Friday");
		break;
	case 7:
		Serial.println("Saturday");
		break;
	}
}