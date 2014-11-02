#include <core.h>
#include <shell.h>

char num_buf0[50];
unsigned char op_buffer[10];
char num_buf1[50] = "125";
//unsigned short int new_line = 0;
int num1, num2, result;
char op;

void arithmetic(int test1, char funct, int test2) {
	if(funct == '+') {
		result = (test1 + test2);
	}
	else if(funct == '-') {
		result = (test1 - test2);
	}
	else if(funct == '*') {
		result = (test1 * test2);
	}
	else if(funct == '/') { puts("not supported at the moment."); return; }
	else { puts("That Operation is either unknown or not supported!"); return; }
	puti(result);
}

void do_math (unsigned char * expr) {
	int i;
	int o = 0;
	int va = 0;
	int end = 0;
	for (i = 0; i < 100; i++)
    {
		switch(va) {
			case 0: 
				if(expr[i] != ' ') {} else {va=1; o=0;}
				break;
			case 1: 
				if(expr[i] != ' ') {
					num_buf0[o] = expr[i];
				} else {
					num_buf0[o++]='\0';  va=2; o=0;
				}
			break;
			case 2: 
				if(expr[i] != ' ') {
					op_buffer[0] = expr[i];
				} else {
					va=3; o=0;
				}
			break;
			case 3: 
				if(expr[i] != ' ' || expr[i] != '\0') {
					num_buf1[o] = expr[i];
				} else {
					num_buf1[o++]='\0'; va=4;
				}
			case 4: 
				end = 1;
			break;
			default: 
				end = 1;
			break;
		}
		if(end == 1) { break; }
		o++;
	}
		/*if(expression[i] != ' ' || expression[i] != '+' || expression[i] != '-' || expression[i] != '*' || expression[i] != '/') {
				num_buf0[o] = expression[i];
				o++;
		} else { break; }
		if(expression[i] == '\0') { break; }
    }
	num_buf0[o++] = '\0';
	if(num_buf0[0] == '\0') { fail=1; }
	o=0;
	for (i; i < 100; i++)
    {
		if(expression[i] == '+' || expression[i] == '-' || expression[i] == '*' || expression[i] == '/') {
			op_buffer = expression[i];
			break;
		}
		if(expression[i] == '\0') { break; }
    }
	//if(op_buffer[0] == '\0') { fail=1; }
	o=0;
	for (i; i < 100; i++)
    {
		if(expression[i] != ' ') {
				num_buf1[o] = expression[i];
				o++;
		} else { break; }
		if(expression[i] == '\0') { break; }*/
    //}
	//num_buf1[o++] = '\0';
	//if(num_buf1[0] == '\0') { fail=1; }
	if(op_buffer[0] != '+' && op_buffer[0] != '-' && op_buffer[0] != '*' && op_buffer[0] != '/') { va = 5; }
	if(va != 4) {
		puts("Incorrect Number of Arguments.");
		puts("\nYou Need 3! [number] [operation {+} {-} {*} {/} ] [number]\n");
		putch(op_buffer[0]);
		puti(va);
	}
	if(va == 4) {
		puts("correct Number of Arguments! yay!");
		num1 = atoi(num_buf0);
		puts(num_buf0);
		op = op_buffer[0];
		num2 = atoi(num_buf1);
		puts(num_buf1);
		//arithmetic(num1, op, num2);
	}
}
