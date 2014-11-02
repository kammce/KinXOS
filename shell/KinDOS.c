#include <core.h>
#include <stdio.h>
#include <shell.h>
#include <video.h>

#define STARTLEVEL   1
#define KERNELLEVEL  2
#define USERLEVEL    3
#define PROGRAMLEVEL 4

uint8_t COMPUTER_STATUS = 0;

shell_t shell;

boolean activate = false;

void bufferStack(unsigned char add);
void updatePrompt();
void promptLine();
void startKinDOS();
/* Installs the KinDOS handler into IRQ2 */
void kinDOSInstall()
{
	strcpy(shell.prompt.user, "guest");
	strcpy(shell.prompt.host, "KinX");
	strcpy(shell.prompt.dir, "/");
	strcpy(shell.prompt.home, "/home/guest");
	strcpy(shell.prompt.path, "/bin/");
	initKernelCmds();
	waits(2);
	ClearScreen();
	alter_xy(0,0);
	move_csr();
	puts("\n\n\n\n\n\n\n\n");
	puts("                  [*]--{---------}---------|| |------------[*] \n");
	puts("                  [|]    || /  +           || |     _   _  [|]  \n");
	puts("                  [|]    ||/   ||  ||__     \\/     | | |_  [|]  \n");
	puts("                  [|]    |*    ||  ||  |    ||     |_|  _| [|]  \n");
	puts("                  [|]    ||\\   ||  ||  |    /\\             [|]   \n");
	puts("                  [|]    || \\  ||  ||  |   || |            [|]   \n");
	puts("                  [*]____________{_________|| |____________[*]  \n");
	puts("\n");
	puts("                          [*] ... loading ... [*] \n");
	alter_xy(30,10);
	move_csr();
	waits(2);
	ClearScreen();
	create_titlebar (0x20);
	create_actionbar(0x40);
	write_titlebar("                             [Welcome to KinX OS]", 0x20);
	write_actionbar("                                                ", 0x20);
	settextcolor(ATTRIBFONT(WHITE,BLACK));
	ClearShell();
	puts("Khalil's Website @ http://www.kammce.io \n");
	puts("KinX Version: 0.01 CONTRUCTION\n\n");
	startKinDOS();
}

void bufferStack(uint8_t add) {
	if(COMPUTER_STATUS < USERLEVEL) { return; }
	if(add == '\n') { activateCMD(); return; }
	//if backspace '0x08' is pressed and buffer count is greater than 0
	if(add == 0x08 && shell.cmdpos > 0) {
		--shell.cmdpos;
		shell.cmdbuffer[shell.cmdpos] = '\0';
		putch(0x08);
	} else if (add == 0x08 && (shell.cmdpos <= 0)) {
		shell.cmdpos = 0;
		shell.cmdbuffer[0] = '\0';
	} else {
		shell.cmdbuffer[shell.cmdpos] = add;
		putch(global_keyboard_char);
		shell.cmdpos++;
	}
}
void activateCMD() {
	bufferStack('\0'); // make sure the last character in the command_buffer is
	_ecx0 = 0;
	_ecx1 = 0;
	//set argument count to 0
	shell.argc = 0;
	//set all arguments found in argv to location 0
	memset(shell.argv, blank_arg, sizeof(shell.argv));
	_ecx2 = strlen(shell.cmdbuffer);

	if(isalpha(shell.cmdbuffer[_ecx0])) {
		shell.argc++;
		shell.argv[_ecx1++] = shell.cmdbuffer;
	}
	for (_ecx0 = 0; shell.cmdbuffer[_ecx0] != 0; _ecx0++)
	{
		if(shell.cmdbuffer[_ecx0] == ' ' && shell.cmdbuffer[_ecx0+1] != ' ') {
			shell.argc++;
			shell.argv[_ecx1++] = &shell.cmdbuffer[_ecx0+1];
			shell.cmdbuffer[_ecx0] = 0;
		}
	}
	char buff[80] = "STATUS: ";
	char num[12] = {0};
	itoa(num, shell.argc);
	strcat(buff, "[");
	strcat(buff, num);
	strcat(buff, "]");
	strcat(buff, shell.argv[0]);
	write_actionbar(buff, 0x40);
	boolean exec_flag = false;
	if(shell.argc != 0) {
		for(_ecx0 = 0; kernel_cmds[_ecx0].sig[0] != 0; _ecx0++) {
			if(strcmp(shell.argv[0], kernel_cmds[_ecx0].sig)) {
				kernel_cmds[_ecx0].method(shell.argc, shell.argv);
				exec_flag = true;
				break;
			}
		}
	}
	if(!exec_flag) {
		kernel_cmds[0].method(shell.argc, shell.argv);
	}
	shell.cmdpos = 0;
	shell.cmdbuffer[0] = 0;
	putch('\n');
	promptLine();
}
void updatePrompt(char * buff) {
	/*
	typedef struct {
		char user[20];
		char host[20];
		char dir[200];
		char home[100];
		char path[200];
		char date[40];
	} shell_t;
	*/
	//"[ Omega@KinX Kin -/home/omega ]: "
	buff[0] = 0; // start concatenation @ 0
	buff[1] = 0; // start concatenation @ 0
	settextcolor(ATTRIBFONT(WHITE,BLACK));
	puts("[ ");
	settextcolor(ATTRIBFONT(MAGENTA,BLACK));
	puts(shell.prompt.host);
	putch('@');
	settextcolor(ATTRIBFONT(LIGHTGREEN,BLACK));
	puts(shell.prompt.user);
	putch(' ');
	settextcolor(ATTRIBFONT(WHITE,BLACK));
	puts(shell.prompt.dir);
	puts(" ]: ");
}
void promptLine() {
	/*if(activate == 1) {
		activate_cmd();
		return;
	}*/
	settextcolor(ATTRIBFONT(WHITE,BLACK));
	updatePrompt(default_prompt);
	printf(default_prompt);
	//alter_x(strlen(default_prompt));
	//alter_pl(strlen(default_prompt));
	//move_csr();
}
void startKinDOS() {
	promptLine();
	COMPUTER_STATUS = USERLEVEL;
}