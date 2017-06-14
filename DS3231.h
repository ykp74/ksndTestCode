#ifndef DS3231_h
#define DS3231_h

#include <stdio.h>
#include <stdint.h>

#define byte uint8_t

typedef struct _ds3231Data {
	int year;
	int month;
	int dayOfMonth;
	int hour;
	int minute;
    int second;
	int dayOfWeek; 
}_ds3231Data;

extern void initDs3231( int address );
extern float get3231Temp(void);
extern void set3231Time(byte year, byte month, byte dayOfMonth, byte hour, byte minute, byte second, byte dayOfWeek);
extern int get3231Time( _ds3231Data * input );

#endif