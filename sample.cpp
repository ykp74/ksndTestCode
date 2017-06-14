#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>   //exit

#include <pthread.h>
#include "curses.h"
#include <linux/uinput.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "TM1637.h"
extern "C" {
	#include "DS3231.h"
}
#include "LiquidCrystal_I2C.h"

#if 1
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#define JOY_DEV "/dev/input/js0"
#endif

TM1637 tm1637(16,12); //CLK: 4  DIO: 2
int8_t TimeDisp[] = {0x00,0x00,0x00,0x00};

// set the LCD address to 0x3F for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define TEST_PIN 	17
#define TEST_PIN_2 	23
#define TEST_PIN_3 	24
#define TEST_PIN_4  25

#define LED_1 ( 1 << 1 )
#define LED_2 ( 1 << 2 )
#define LED_3 ( 1 << 3 )

// RTC DS3231
#define ADDR_RTCDS3231 0x68
#define NUM_THREADS	3

int joy_fd, num_of_axis=0, num_of_buttons=0, x;
int axis[29];
char button[17];
char name_of_joystick[80];
struct js_event js;

int led_ctl = 0;

typedef enum _LED {
	led_ctl_1 = 1,
	led_ctl_2 = 2,
	led_ctl_3 = 3,
	led_ctl_4 = 4
}_LED;

int set_led_ctrl( int led )
{
	if( led & (1 << 12) ){
		digitalWrite(TEST_PIN,HIGH);
	} else {
		digitalWrite(TEST_PIN,LOW);
	}

	if( led & (1 << 13) ){
		digitalWrite(TEST_PIN_2,HIGH);
	} else {
		digitalWrite(TEST_PIN_2,LOW);
	}

	if( led & (1 << 14) ){
		digitalWrite(TEST_PIN_3,HIGH);
	} else {
		digitalWrite(TEST_PIN_3,LOW);
	}
	if( led & (1 << 15) ){
		digitalWrite(TEST_PIN_4,HIGH);
	} else {
		digitalWrite(TEST_PIN_4,LOW);
	}
	return 0;
}

void init()
{
	initDs3231( ADDR_RTCDS3231 );

	tm1637.set(BRIGHTEST);
	tm1637.init();

	lcd.init();
	lcd.backlight();
	lcd.clear();

	//lcd.typeln("Hello!");
	//lcd.blink();
}

int init_joypad_device()
{
	if( ( joy_fd = open( JOY_DEV , O_RDONLY)) == -1 ){
		printf( "Couldn't open joystick\n" );
		return -1;
	}

	ioctl( joy_fd, JSIOCGAXES, &num_of_axis );
	ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );
	ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick );

	//axis = (int *) malloc( num_of_axis, sizeof( int ) );
	//button = (char *) malloc( num_of_buttons, sizeof( char ) );

	printf("Joystick detected: %s\n\t%d axis\n\t%d buttons\n\n", name_of_joystick, num_of_axis, num_of_buttons );
	fcntl( joy_fd, F_SETFL, O_NONBLOCK );   /* use non-blocking mode */
	return 0;
}

void get_joystic_update()
{
	while(1){
		usleep(3000);
        	/* read the joystick state */
        	read(joy_fd, &js, sizeof(struct js_event));

        	/* see what to do with the event */
        	switch (js.type & ~JS_EVENT_INIT){
			case JS_EVENT_AXIS:
				axis[ js.number ] = js.value;
				break;
			case JS_EVENT_BUTTON:
				button[ js.number ] = js.value;
				break;
		}

		/* print the results */
		printf( "X: %6d  Y: %6d  ", axis[0], axis[1] );

		if( num_of_axis > 2 )
			printf("Z: %6d  ", axis[2] );

		if( num_of_axis > 3 )
			printf("R: %6d  ", axis[3] );

		if( num_of_axis > 4 )
			printf("Z: %6d  ", axis[4] );

		if( num_of_axis > 5 )
			printf("R: %6d  ", axis[5] );

		for( x=0 ; x<num_of_buttons ; ++x ){
			if( button[x] ){
				led_ctl =  led_ctl | (1 << x);
			} else {
				led_ctl =  led_ctl & ~(1 << x);
			}
		//	printf("B%d: %d  ", x, button[x] );
		}

		printf("  %d  ",led_ctl );
		set_led_ctrl( led_ctl );

		printf("  \r");
		fflush(stdout);
	}
	close( joy_fd );        /* too bad we never get here */
}

void get_time_update()
{
	char sbuf2[100];
	char sbuf1[50];
	static boolean clock_point = true;
	float tempData;
	_ds3231Data timeData;

	while(1){
		clock_point = !clock_point;

		get3231Time(&timeData);

		TimeDisp[0] = timeData.hour / 10;
		TimeDisp[1] = timeData.hour % 10;

		TimeDisp[2] = timeData.minute / 10;
		TimeDisp[3] = timeData.minute % 10;

		tm1637.point(clock_point);
		tm1637.display(TimeDisp);
	
		//tempData = get3231Temp();
		//lcd.setCursor(7,0);
		//lcd.typeFloat(tempData);

		lcd.setCursor(0,0);
		sprintf(sbuf1," %6d  %6d",axis[4],axis[5]);
		lcd.typeln(sbuf1);
		
		//sprintf(sbuf, "20%02d/%02d/%02d %02d:%02d:%02d %02f", year, month, dayOfMonth, hour, minute, second,temp3231);
		sprintf(sbuf2, "%02d/%02d %02d:%02d:%02d", timeData.month, timeData.dayOfMonth, timeData.hour, timeData.minute, timeData.second);
		//printf(sbuf);
		lcd.setCursor(0,1);
		lcd.typeln(sbuf2);

		delay(200);
	}
}

/*********************************************************************/
/* ThreadProcs */
/*********************************************************************/
void *ThreadProcs(void *threadid)
{
	int thread_id = (int)threadid;

	if(thread_id == 0){
		get_joystic_update();
	}

	if(thread_id == 1){
		//THIS THREAD WILL MAKE THE PROGRAM EXIT
        	wmove(stdscr, 1, 0);
        	addstr("Type \"q\" to quit.\n");

		int ch;
		nodelay(stdscr, TRUE);			//	SETUP NON BLOCKING INPUT
		while(1) {
			if ((ch = getch()) == ERR)
				usleep(16666);		//	USER HASN'T RESPONDED
			else if( ch == 'q'){
				endwin();
				exit(0);		//	QUIT ALL THREADS
			}
		}
	}

	if(thread_id == 2){
		get_time_update();
	}
}

/*********************************************************************/
/* main() */
/*********************************************************************/
int main(void)
{
	pthread_t threads[NUM_THREADS];
	int rc, t;
	int cnt = 0;

	//wiringPiSetup();
	//wiringPiSetupSys();
	wiringPiSetupGpio();
	pinMode(TEST_PIN, OUTPUT);
	pinMode(TEST_PIN_2, OUTPUT);
	pinMode(TEST_PIN_3, OUTPUT);
	pinMode(TEST_PIN_4, OUTPUT);

	init();
	init_joypad_device();
	//set3231Time( 17,06,14, 0,20,0,3 );

	for(t = 0; t < NUM_THREADS; t++){	//	MAKE 2 NEW THREADS
		rc = pthread_create(&threads[t], NULL, ThreadProcs, (void *)t);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
			pthread_exit(NULL);
		}
	}

	for(t = 0; t < NUM_THREADS; t++){
		pthread_join(threads[t], NULL);		//	WAIT FOR THREADS TO EXIT OR IT WILL RACE TO HERE.
	}
	endwin();

	return 0;
}
