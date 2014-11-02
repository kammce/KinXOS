#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define assert(expr) \
{ \
    if (!(expr)) \
    { \
        printf("Assertion failed: %s, file %s, line %d\n", #expr, __FILE__, __LINE__); \
        fflush(stdout); \
        exit(-1); \
    } \
}

typedef struct {
	char sig[15];
	void (* method)(char[]);
} kernel_cmds_t;

kernel_cmds_t kernel_cmds[15];

void strcat2(char dest[], char src[]) {
	while(*dest != 0) { *dest++; }
	while(*src  != 0) { *dest++ = *src++; }
	*dest = 0;
}
void printthem(char *test[]){
	int i = 0;
	for(; i < 5; i++) {
		printf("%s\n", test[i]);
	}
}
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
/*char * sprintf_new (char buffer[], const char *format, ...)
{
	char * start = buffer;
	char **arg = (char **) &format; //is the pointer to all of the arguments
	arg++;
	for(int i = 0; i < 5;i++) {
		printf("::%d\n", *((int *) arg++));
	}
	int c;
	char p[20];
	arg++;
	while (*format++ != 0) { //c will point to the first string argument
		c = format;
		if (c != '%') {
			*buffer++ = c;
		} else {
			c = *format++;
			switch (c)
			{
			case 'd':
			case 'u':
			case 'x':
			  itoa2 (p, *((int *) arg++), c); //point to the next item on the list, after, inc
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
				 *buffer++ = *((char *)arg++);
			  break;
			}
		}
	}
	return buffer;
}*/

typedef struct
{
	char * start;
	char * end;
	char * list[50];
	uint8_t pos;
	char data[512];
} omni_param_t;

typedef struct
{
	char type;
	char * strptr;
	char data[32];
} single_param_t;

omni_param_t oparam;

omni_param_t * paramInit(omni_param_t * omni) {
	memset(omni->data, 0, 512);
	memset(omni->list, omni->data, 50);
	omni->start = omni->data;
	omni->end = omni->data;
	omni->pos = 0;
}
omni_param_t * paramClr(omni_param_t * omni) {
	paramInit(omni);
}
omni_param_t * paramAppend(omni_param_t * omni, void * src, uint8_t size) {
	if(size <= 0) { return omni; }
	char * sourcer = (char *)src;
	++omni->pos;
	omni->list[omni->pos] = omni->data; 
	for(; size != 0; --size) {
		omni->data[omni->end++] =  *sourcer++;
	}
}
int getParamSize(omni_param_t * omni) {
	return omni->end - omni->start;
}
omni_param_t * resetParamPos(omni_param_t * omni) {
	omni->pos = 0;
}
void * getNextParam(omni_param_t * omni, single_param_t * sing) {
	// If the next item in the list points to the beginning of the
	// the data array, then that position is the last one and
	// the true end is the omni->end.
	int distance;
	if(omni->list[omni->pos+1] == omni->data) {

	} else {
		distance = omni->list[omni->pos-1] - omni->list[omni->pos]
	}
}

void multiparam(omni_param_t * omni) {
	for(int i = 0; i < 50; i++) {
		printf("%c::%p\n", *(&data0-4*i), &data0+4*i);
	}
}
int main() {
	char * command_sig[] = {
		"default",
		"help",
		"hello",
		"echo",
		"date",
		"clear",
		"cursor",
		"ram",
		"stats",
		"reboot",
		"shutdown",
		"\0"
	};
	printf("hex - = %ld\n", (0x7fff9833beac-0x7fff9833beb0));
	char buffer[200] = {0};
	multiparam('5', '4', '3', '2', '1', '0');
	printf("%s\n", buffer);
	buffer[0] = 0;
	strcat2(buffer,"My ");
	strcat2(buffer,"Ass ");
	strcat2(buffer,"!!! ");
	printf("%s\n", buffer);

	printthem(command_sig);
	/*int _ecx0;
	for(_ecx0 = 0; command_sig[_ecx0][0] != 0; _ecx0++) {
		strcpy(kernel_cmds[_ecx0].sig, command_sig[_ecx0]);
	}
	for(_ecx0 = 0; command_sig[_ecx0][0] != 0; _ecx0++) {
		printf("%d) %s\n", _ecx0, command_sig[_ecx0]);
		printf("%d) %s\n", _ecx0, kernel_cmds[_ecx0].sig);
	}
	int count = 5;
	for(count = 0; count < 5; count++) {
		printf("%s\n",command_sig[count]);
	}*/
}
