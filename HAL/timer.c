/* Desc: Timer driver */
#include <core.h>
#include <cpu.h>

/* This will keep track of how many ticks that the system
*  has been running for */

struct RTC current_time;

unsigned long timer_ticks = 0;
unsigned long second_epoc = 0;
unsigned long minute_epoc = 0;
char nop = ' ';

/* Handles the timer. In this case, it's very simple: We
*  increment the 'timer_ticks' variable every time the
*  timer fires. By default, the timer fires 18.222 times
*  per second. Why 18.222Hz? Some engineer at IBM must've
*  been smoking something funky */
void timer_handler(struct regs *r)
{
	/* Increment our 'tick count' */
	timer_ticks++;

	/* Every 18 clocks (approximately 1 second), we will
	*  display a message on the screen */
	if (timer_ticks % 18 == 0) { second_epoc++; }
	if (second_epoc % 60 == 0) { minute_epoc++; }
	/* get the current time */
	get_time();
}

unsigned long returnTimerTicks() {
	return timer_ticks;
}
unsigned long returnSecondEpoc() {
	return second_epoc;
}
unsigned long returnMinuteEpoc() {
	return minute_epoc;
} 
/* This will continuously loop until the given time has
*  been reached */
void timer_wait(int ticks)
{
	unsigned long eticks;
	eticks = timer_ticks + ticks;
	while(returnTimerTicks() < eticks) {
		hlt();
	}
}/* This will continuously loop until the given time has
*  been reached in seconds */
void waits(int seconds)
{
	unsigned long esecs;
	esecs = second_epoc + seconds;
	while(returnSecondEpoc() < esecs) {
		hlt();
	}
}/* This will continuously loop until the given time has
*  been reached in minutes */
void waitm(int minutes)
{
	unsigned long esecs;
	esecs = second_epoc + minutes*60;
	while(returnMinuteEpoc() < esecs) {
		hlt();
	}
}

/* Sets up the system clock by installing the timer handler
*  into IRQ0 */
void timer_install()
{
	/* Installs 'timer_handler' to IRQ0 */
	irq_install_handler(I86_PIC_IRQ_TIMER, timer_handler);
}

int get_update_in_progress_flag() {
	outportb(cmos_address, 0x0A);
	return (inportb(cmos_data) & 0x80);
}

unsigned char get_RTC_register(int reg) {
	outportb(cmos_address, reg);
	return inportb(cmos_data);
}
/*
Getting Current Date and Time from RTC
To get each of the following date/time values from the RTC, you should first ensure that you won't be effected by an update (see below). Then select the associated "CMOS register" in the usual way, and read the value from Port 0x71.

	+-Register-|-Contents-----+
	|__________|______________|
	| 0x00     | Seconds      |
	| 0x02     | Minutes      |
	| 0x04     | Hours        |
	| 0x06     | Weekday      |
	| 0x07     | Day of Month |
	| 0x08     | Month        |
	| 0x09     | Year         |
	+-------------------------+
*/
void get_time() {
	uint8_t century;
	uint8_t last_second;
	uint8_t last_minute;
	uint8_t last_hour;
	uint8_t last_day;
	uint8_t last_month;
	uint8_t last_year;
	//uint8_t last_century;
	uint8_t registerB;

	uint8_t *second  = &current_time.second;
	uint8_t *minute  = &current_time.minute;
	uint8_t *hour    = &current_time.hour;
	uint8_t *day     = &current_time.day;
	uint8_t *month   = &current_time.month;
	uint8_t *year    = &current_time.year;
	//uint8_t *century = &current_time.century;
 
	// Note: This uses the "read registers until you get the same values twice in a row" technique
	//       to avoid getting dodgy/inconsistent values due to RTC updates
 
	while (get_update_in_progress_flag());                // Make sure an update isn't in progress
	*second = get_RTC_register(0x00);
	*minute = get_RTC_register(0x02);
	*hour = get_RTC_register(0x04);
	*day = get_RTC_register(0x07);
	*month = get_RTC_register(0x08);
	*year = get_RTC_register(0x09);
	/*if(century_register != 0) {
		century = get_RTC_register(century_register);
	}*/
 
	while( (last_second == *second) && (last_minute == *minute) && (last_hour == *hour) &&
		(last_day == *day) && (last_month == *month) && (last_year == *year)) 
	{
		last_second = *second;
		last_minute = *minute;
		last_hour = *hour;
		last_day = *day;
		last_month = *month;
		last_year = *year;
		//last_century = century;
 
		while (get_update_in_progress_flag());           // Make sure an update isn't in progress
		*second = get_RTC_register(0x00);
		*minute = get_RTC_register(0x02);
		*hour = get_RTC_register(0x04);
		*day = get_RTC_register(0x07);
		*month = get_RTC_register(0x08);
		*year = get_RTC_register(0x09);
		/*if(century_register != 0) {
			century = get_RTC_register(century_register);
		}*/
	}
		/*&& (last_century == century) */
 
	registerB = get_RTC_register(0x0B);
 
	// Convert BCD to binary values if necessary
 
	if (!(registerB & 0x04)) {
		*second = (*second & 0x0F) + ((*second / 16) * 10);
		*minute = (*minute & 0x0F) + ((*minute / 16) * 10);
		*hour = ( (*hour & 0x0F) + (((*hour & 0x70) / 16) * 10) ) | (*hour & 0x80);
		*day = (*day & 0x0F) + ((*day / 16) * 10);
		*month = (*month & 0x0F) + ((*month / 16) * 10);
		*year = (*year & 0x0F) + ((*year / 16) * 10);
		/*if(century_register != 0) {
			century = (century & 0x0F) + ((century / 16) * 10);
		}*/
	}
	// Convert 12 hour clock to 24 hour clock if necessary
	/*if (!(registerB & 0x02) && (time[2] & 0x80)) {
				time[2] = ((time[2] & 0x7F) + 12) % 24;
	}*/
}
