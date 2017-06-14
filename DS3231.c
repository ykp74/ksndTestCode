#include <wiringPiI2C.h>
#include "DS3231.h"

int fd = 0;
_ds3231Data dt;

// 10진수�?2진화 10진수??BCD �?변??(Binary Coded Decimal)
static byte decToBcd(byte val)
{
	return ( (val/10*16) + (val%10) );
}

static byte bcdToDec(byte val) 
{ 
	return( (val/16*10) + (val%16) ); 
}

void initDs3231( int address )
{
	fd = wiringPiI2CSetup( address ); 
}

float get3231Temp(void)
{
	byte tMSB,tLSB;
    float temp3231;

	//read the data 0x11 0x12 address
	tMSB = wiringPiI2CReadReg8 (fd,0x11); //2's complement int portion
	tLSB = wiringPiI2CReadReg8 (fd,0x12); //fraction portion

	temp3231 = (tMSB & 0B01111111); //do 2's math on Tmsb
	temp3231 += ( (tLSB >> 6) * 0.25 ); //only care about bits 7 & 8

	return temp3231;
}

void set3231Time(byte year, byte month, byte dayOfMonth, byte hour, byte minute, byte second, byte dayOfWeek)
{
	// sets time and date data to DS3231 
	wiringPiI2CWriteReg8(fd, 0x00, decToBcd(second));		// set seconds
	wiringPiI2CWriteReg8(fd, 0x01, decToBcd(minute)); 		// set minutes
	wiringPiI2CWriteReg8(fd, 0x02, decToBcd(hour)); 		// set hours
	wiringPiI2CWriteReg8(fd, 0x03, decToBcd(dayOfWeek)); 	// ( 1=Sunday, 7=Saturday)
	wiringPiI2CWriteReg8(fd, 0x04, decToBcd(dayOfMonth)); 	// set dayOfMonth (1 to 31)
	wiringPiI2CWriteReg8(fd, 0x05, decToBcd(month)); 		// set month
	wiringPiI2CWriteReg8(fd, 0x06, decToBcd(year)); 		// set year (0 to 99)
}

int get3231Time( _ds3231Data * input )
{
	input->second    = bcdToDec(wiringPiI2CReadReg8 (fd, 0x00) & 0x7f );
	input->minute    = bcdToDec(wiringPiI2CReadReg8 (fd, 0x01) );
	input->hour      = bcdToDec(wiringPiI2CReadReg8 (fd, 0x02) & 0x3f );
	input->dayOfWeek = bcdToDec(wiringPiI2CReadReg8 (fd, 0x03) );
	input->dayOfMonth = bcdToDec(wiringPiI2CReadReg8 (fd, 0x04) );
	input->month     = bcdToDec(wiringPiI2CReadReg8 (fd, 0x05) );
	input->year      = bcdToDec(wiringPiI2CReadReg8 (fd, 0x06) );
	
	return 0;
}