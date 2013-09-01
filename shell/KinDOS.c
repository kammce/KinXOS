#include <core.h>
#include <shell.h>
#include <video.h>

#define STARTLEVEL   1
#define KERNELLEVEL  2
#define USERLEVEL    3
#define PROGRAMLEVEL 4

uint8_t command_buffer[100];
uint8_t command_buffer_count = 0;
uint8_t shell_command[100];
uint8_t shell_args[100];

uint8_t help_cmd  [] = "help";
uint8_t hello_cmd [] = "hello";
uint8_t echo_cmd  [] = "echo";
uint8_t date_cmd  [] = "date";
uint8_t clear_cmd [] = "clear";
uint8_t cursor_cmd[] = "cursor";
uint8_t ram_cmd[] = "ram";
uint8_t reboot_cmd[] = "reboot";
uint8_t shutdown_cmd[] = "shutdown";

uint8_t COMPUTER_STATUS = 0;

int pstring = 0;
struct system_environment {
	char host[20];
	char dir[200];
	char home[100];
	char path[200];
	char date[40];
};

struct system_environment environment;


void change_prompt(unsigned char *ptext) {
	int col = 0;
	for (counter_0 = 0; counter_0 < strlen(ptext); counter_0++)
    {
		if(ptext[counter_0] == ' ' || ptext[counter_0] == '@') {
			col++;
			switch (col) {
			case 0:
				settextcolor(ATTRIBFONT(WHITE,BLACK));
				break;
			case 1:
				settextcolor(ATTRIBFONT(MAGENTA,BLACK));
				break;
			case 2:
				settextcolor(ATTRIBFONT(LIGHTGREEN,BLACK));
				break;
			case 3:
				settextcolor(ATTRIBFONT(CYAN,BLACK));
				break;
			case 4:
				settextcolor(ATTRIBFONT(WHITE,BLACK));
				break;
			default:
				settextcolor(ATTRIBFONT(WHITE,BLACK));
			  break;
			}
		}
		if(ptext[counter_0] != '\0') {
        default_prompt[counter_0] = ptext[counter_0];
		putch(ptext[counter_0]);
		} else { break; }
    }
}

void prompt_line(unsigned short int activate) {
	if(activate == 1) {
		activate_cmd();
		return;
	}
	settextcolor(ATTRIBFONT(WHITE,BLACK));
	change_prompt("[ Omega@KinX Kin -/home/omega ]: ");
	pstring = strlen(default_prompt);
	alter_x(pstring);
	alter_pl(pstring);
    move_csr();
}

void start_kin_dos() {
	//memcpy(environment.dir ,"/home/omega", sizeof("/home/omega"));
	//memcpy(environment.user,"Omega"      , sizeof("Omega"));
	//memcpy(environment.host,"KinX"       , sizeof("...."));
	prompt_line(0);
	COMPUTER_STATUS = USERLEVEL;
}
/* Installs the KinDOS handler into IRQ2 */
void kin_dos_install()
{
	ClearScreen();
	puts("\n");
	puts("\n");
	puts("\n");
	puts("\n");
	puts("\n");
	puts("\n");
	puts("\n");
	puts("\n");
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
    create_titlebar (0x20);
    create_actionbar(0x40);
    write_titlebar  ("                             [Welcome to KinX OS]", 0x20);
    write_actionbar  ("", 0x20);
    settextcolor(ATTRIBFONT(WHITE,BLACK));
	ClearShell();
	puts(" ____________________________________________  \n");
	puts("|    \\____Welcome to                        | \n");
	puts("|___________________\\KinX Operating System__| \n");
	puts("\n");
	puts("KinX Documentation @ http://www.kammcecorp.net/#body/projects.html \n");
	puts("KinX Version: 0.01 CONTRUCTION\n\n");
	start_kin_dos();
}
void buffer_stack(unsigned char add) {
	if(COMPUTER_STATUS < USERLEVEL) { return; }
	if(add == '\n') { activate_cmd(); return; }
	//if backspace '0x08' is pressed and buffer count is greater than 0
	if(add == 0x08 && (command_buffer_count > 0)) {
		command_buffer_count--;
		command_buffer[command_buffer_count] = '\0';
		putch(0x08);
	}
	else if (add == 0x08 && (command_buffer_count <= 0)) {
		command_buffer_count = 0;
		command_buffer[0] = ' '; 
		command_buffer[1] = '\0';
	} else {
		command_buffer[command_buffer_count] = add;
		putch(global_keyboard_char);
		command_buffer_count++;
	}
}
void activate_cmd() {
	buffer_stack('\0'); // make sure the last character in the command_buffer is 
	// 0 =  surpassing leading spaces
	// 1 =  getting process name
	// 2 >= getting arguments
	int shellStage = 0;
	counter_0 = 0;
	counter_1 = 0;
	counter_2 = 0;
	counter_3 = 0;
	for (counter_0 = 0; counter_0 < strlen(command_buffer); counter_0++)
    {
    	if(command_buffer[counter_0] != ' ' && shellStage == 0) {
    		shellStage++;
    		counter_0--;
    	} else if(command_buffer[counter_0] != ' ' && shellStage == 1) {
			shell_command[counter_1++] = command_buffer[counter_0];
		} else if(command_buffer[counter_0] == ' ' && shellStage == 1) {
			shellStage++;
		} else if(shellStage == 2) {
			shell_args[counter_2++] = command_buffer[counter_0];
			if(command_buffer[counter_0] == '\0') {
				break;
			}
		}
    }
	write_actionbar(shell_args, 0x40);
	if(shell_command[0] != '\0') {
		if(strcompare(shell_command, help_cmd) == TRUE) {
			puts("Available Commands: ");
			puts("\nhelp  - Lists the available commands and this prompt.");
			puts("\nhello - Welcome Program.");
			puts("\necho  - display a line of text");
			puts("\ndate  - display date from bios");
			//puts("\nmath  - do simple arithmetic");
			puts("\ncursor - display cursor location");
			puts("\nclear  - clear screen");
			puts("\nram    - ????????????");
			puts("\nreboot - reboot computer");
		}
		else if(strcompare(shell_command,hello_cmd) == TRUE) { puts("Hello! This is KinX's First Command. Thank you for using it!"); }
		else if(strcompare(shell_command,echo_cmd) == TRUE)  { echo(shell_args); }
		else if(strcompare(shell_command,date_cmd) == TRUE)  { display_time(); }
		else if(strcompare(shell_command,clear_cmd) == TRUE) { ClearShell(); }
		else if(strcompare(shell_command,cursor_cmd) == TRUE){ display_cursor_loc(); }
		else if(strcompare(shell_command,ram_cmd) == TRUE)   { output_ram(); }
		else if(strcompare(shell_command,reboot_cmd) == TRUE){ reboot(); }
		//else if(strcompare(shell_command,shutdown_cmd) == TRUE){ acpiPowerOff(); }
		else { settextcolor(0x40); puts("That command does not exist."); }
		if(!strcompare(shell_command,clear_cmd)) { putch('\n'); }
	}
	int i;
	//reset command and shell commands and re-state prompt
	for (i = 0; i < command_buffer_count; i++) { command_buffer[i] = NULLCHAR; }
	for (i = 0; i < command_buffer_count; i++) { shell_command[i] = NULLCHAR; }
	for (i = 0; i < command_buffer_count; i++) { shell_args[i] = NULLCHAR; }
	command_buffer_count = 0;
	prompt_line(0);
}

