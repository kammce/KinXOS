#include <core.h>
#include <shell.h>

/*unsigned char echo_buffer[100];
unsigned short int new_line = 0;*/

void echo (unsigned char *c1) {
/*	int i;
	int o = 0;
	for (i = 5; i < 100; i++)
    {
		if(c1[i] == '-' && c1[i+1] == 'n') {  new_line = 1; i=i+2; }
		else {
			echo_buffer[o] = c1[i];
			o++;
		}
		if(c1[i] == '\0' && new_line == 0) { puts(echo_buffer); return; }
		if(c1[i] == '\0' && new_line == 1) { puts(echo_buffer); putch('\n'); return; }
    }*/
	puts(c1);
}
void output_ram() {
	puts("Memory Size = ");
	memory_size = 253;
	puti(memory_size);
	puti(46549879);
	puts(" :: ");
	puts("\n");
}
//179488