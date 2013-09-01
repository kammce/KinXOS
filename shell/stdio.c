/*  Desc: I/O Console */

#include <core.h>
#include <cpu.h>
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