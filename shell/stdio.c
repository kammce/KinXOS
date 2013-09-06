/*  Desc: I/O Console */

#include <core.h>
#include <shell.h>
#include <stdio.h>
#include <math.h>

int strlen(const char *str)
{
	int retval;
    for(retval = 0; *str != '\0'; str++) { retval++; }
    return retval;
}

boolean strcompare (unsigned char * c1, unsigned char * c2) {
	int i;
	for (i = 0; i < strlen(c1); i++)
    {
		if(!cmp(c1[i],c2[i])) { return FALSE; }
    }
	return TRUE;
}

void itoa(char *_str, int num ) {
	boolean negFlag = FALSE; //if number is negative
	int temp_num = num; //temperary numbers
	short factors = intlength(num);  //holds the length of the number
	counter_0 = 0; //clear counter_0
	counter_1 = 0; //clear counter_1
	//if(factors > strlen(_str)) { return; }
	temp_num = 0;
	for(counter_0 = factors; counter_0 > 0; counter_0--) {
		if(negFlag) {
			num *= -1;
			_str[counter_1++] = '-';
			counter_0++;
			negFlag = FALSE;
		} else {
			temp_num = (int)(num/pow(10,counter_0));
			if(temp_num == 0) { temp_num = num; }
			_str[counter_1++] = temp_num + '0';
			num -= temp_num * pow(10,counter_0);
		}
	}
	_str[counter_1++] = '\0';
	counter_0 = 0; //clear counter_0
	counter_1 = 0; //clear counter_1
}
/* Convert the integer D to a string and save the string in BUF. If
        BASE is equal to 'd', interpret that D is decimal, and if BASE is
        equal to 'x', interpret that D is hexadecimal. */
void itoa2 (char *buf, int base, int d)
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

int atoi( char * c ) {
	int num_place = 0;
	int ru = 0;
	int rd = 0;
	int num = 0;
	int negative = 0;
	if(c[0] == '-') { negative = -1; }
	for (ru = strlen(c); ru > -1; ru--) {
		if(toascii(c[ru]) == 11) { break; }
		num += (toascii(c[ru]) * pow(10,rd));
		rd++;
	}
	if(negative == -1) { num *= -1; }
	return num;
}
/*unsigned char clear_char(unsigned char *clear_var) {
	int i;
	for (i = 0; i < strlen(clear_var); i++)
    { clear_var[i] = '\0'; }
	return clear_var;
} */
/*
void mem_str_cpy(char *dest[], unsigned char mov, int start, int end) {
    int i_s;
    int i_0 = 0;
    for (i_s = start; i_s < end; i_s++) {
        dest[i_s] = mov[i_0];
        i_0++;
        if(mov[i_0] == '\0') { break; }
    }
}
*/
/*
char *clear_char(char * c1, int sum) {
	int i;
	for (i = 0; i < sum; i++) { c1[i] = NULL; }
	return c1;
}*/
/*unsigned char command_compare (unsigned char * c1, unsigned char * c2) {
	int i;
	for (i = '\0'; i < strlen(c1); i++)
    {
		if(c1[i] != '\0' && c2[i] != '\0') {
        	if (c1[i] != c2[i]) { return FALSE; }
		} else if(c1[i] == ' ' && c2[i] == '\0') {
        	return TRUE;
		} else {
			return FALSE;
		}
    }
	return TRUE;
}*/