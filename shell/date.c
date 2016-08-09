/* Requests Time and Date from CMOS/RTC*/
#include <core.h>
#include <shell.h>

unsigned char *months[] = { 0, "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec",0 };

void display_time()
{
	//This need to be changed to match current date data structure.
	puts(months[current_time.month]);
	puts(" ");
	puti(current_time.day);
	puts(", ");
	//puti(2013);
	//puts(", ");
	puti(current_time.hour);
	puts(":");
	if(current_time.minute < 10) {
		puts("0");
	}
	puti(current_time.minute);
	puts(":");
	if(current_time.second < 10) {
		puts("0");
	}
	puti(current_time.second);
	//puts("Apr 29 15:32:33 2012");
	return;
}
