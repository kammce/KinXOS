/*  Desc: I/O Console */

#include <core.h>
#include <shell.h>
#include <stdio.h>
#include <math.h>

void itoa(char *buf, int num ) {
	boolean negFlag = FALSE; //if number is negative
	int temp_num = num; //temperary numbers
	short factors = intlength(num);  //holds the length of the number
	counter_0 = 0; //clear counter_0
	counter_1 = 0; //clear counter_1
	//if(factors > strlen(buf)) { return; }
	temp_num = 0;
	for(counter_0 = factors; counter_0 > 0; counter_0--) {
		if(negFlag) {
			num *= -1;
			buf[counter_1++] = '-';
			counter_0++;
			negFlag = FALSE;
		} else {
			temp_num = (int)(num/pow(10,counter_0));
			if(temp_num == 0) { temp_num = num; }
			buf[counter_1++] = temp_num + '0';
			num -= temp_num * pow(10,counter_0);
		}
	}
	buf[counter_1++] = '\0';
	counter_0 = 0; //clear counter_0
	counter_1 = 0; //clear counter_1
}
/* Convert the integer D to a string and save the string in BUF. If
		BASE is equal to 'd', interpret that D is decimal, and if BASE is
		equal to 'x', interpret that D is hexadecimal. */
void itoa2(char *buf, int base, int d)
{
  char *p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;

  /* If %d is specified and D is minus, put `-' in the head. */
  if (base == 'd' && d < 0)
	{
	  *p++ = '-';
	  buf++;
	  ud = -d;
	}
  else if (base == 'x')
	divisor = 16;

  /* Divide UD by DIVISOR until UD == 0. */
  do
	{
	  int remainder = ud % divisor;

	  *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
	}
  while (ud /= divisor);

  /* Terminate BUF. */
  *p = 0;

  /* Reverse BUF. */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2)
	{
	  char tmp = *p1;
	  *p1 = *p2;
	  *p2 = tmp;
	  p1++;
	  p2--;
	}
}

int atoi( char * buf ) {
	int num_place = 0;
	int ru = 0;
	int rd = 0;
	int num = 0;
	int negative = 0;
	if(buf[0] == '-') { negative = -1; }
	for (ru = strlen(buf); ru > -1; ru--) {
		if(toascii(buf[ru]) == 11) { break; }
		num += (toascii(buf[ru]) * pow(10,rd));
		rd++;
	}
	if(negative == -1) { num *= -1; }
	return num;
}
int strlen(const char *str)
{
	int retval;
	for(retval = 0; *str != '\0'; str++) { retval++; }
	return retval;
}

boolean strcmp(char c1[], char c2[]) {
	while(*c1 != 0) {
		if(*c1++ != *c2++) {
			return FALSE;
		}
	}
	return TRUE;
}

char * substr(char dest[], char src[], int start, int len) {
	src = src+start;
	for (; len != 0; --len) { *dest++ = *src++; }
	return dest;
}

char * strcpy(char dest[], char src[]) {
	while(*src != 0) { *dest++ = *src++; }
	return dest;
}

char * strcat(char dest[], char src[]) {
	while(*dest != 0) { *dest++; }
	while(*src  != 0) { *dest++ = *src++; }
	*dest = 0;
	return dest;
}
/* Format a string and copy it to a character array. */
char * sprintf (char buffer[], const char *format, ...)
{
	char start = buffer;
	char **arg = (char **) &format; //is the pointer to all of the arguments
	int c;
	char buf[20];
	arg++;
	while ((c = *format++) != 0) { //c will point to the first string argument
		if (c != '%') {
			*buffer++ = c;
		} else {
			char *p;
			c = *format++;
			switch (c)
			{
			case 'd':
			case 'u':
			case 'x':
			  itoa2 (buf, *((int *) arg++), c); //point to the next item on the list, after, inc
			  p = buf;
			  goto string;
			  break;
			case 's':
			  p = *arg++;
			  if (! p)
				 p = "(null)";

			string:
			  while (*p)
				 *buffer++ = *p++;
			  break;

			default:
				 *buffer++ = *arg++;
			  break;
			}
		}
	}
	return buffer;
}