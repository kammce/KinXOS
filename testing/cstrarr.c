#include <stdio.h>
#include <string.h>

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
	char buffer[200] = {0};
	strcat2(buffer,"Hello ");
	strcat2(buffer,"World ");
	strcat2(buffer,"MothaFucka ");
	strcat2(buffer,"!!! ");
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
