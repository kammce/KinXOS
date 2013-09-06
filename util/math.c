#include <core.h>
#include <math.h>

/* math.c
 * Mathematical Functions.
 *
 */
int pow(int base, short power) {
	int num = base;
	int i = 0;
	for(i = 2; i < power; i++) {
		num *= base;
	}
	return num;
}
int intlength(int num) {
	boolean negFlag = FALSE;
	int factors = 1;
	if(num < 0) { negFlag = TRUE; }
	if(negFlag) {
		while(num <= -10) {
			num /= 10;
			factors++;
		}
	} else {
		while(num >= 10) {
			num /= 10;
			factors++;
		}
	}
	return factors;
}