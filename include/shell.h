/* Desc: Global Declaration of functions and values needed for the KinDOS terminal */

#ifndef __SHELL_H
#define __SHELL_H

#define STARTLEVEL   1
#define KERNELLEVEL  2
#define USERLEVEL    3
#define PROGRAMLEVEL 4

unsigned char default_prompt[100];

typedef struct {
	char sig[15];
	void (* method)(uint8_t, char *[]);
} kernel_cmds_t;

uint8_t COMPUTER_STATUS;
kernel_cmds_t kernel_cmds[15];

extern char blank_arg[];

typedef struct {
	char user[20];
	char host[20];
	char dir[255];
	char home[255];
	char path[255];
} prompt_t;

typedef struct {
	prompt_t prompt;
	uint8_t argc;
	char * argv[20];
	char cmdbuffer[255];
	uint8_t cmdpos;
} shell_t;

/* CONSOLE.C */
extern void alter_pl(unsigned int pl);
extern void alter_x(unsigned int new_x);
extern void alter_y(unsigned int new_y);
extern void alter_xy(unsigned int new_x, unsigned int new_y);
extern void scroll(void);
extern void create_titlebar(unsigned char colour);
extern void write_titlebar(unsigned char *str, unsigned char colour);
extern void create_actionbar(unsigned char colour);
extern void write_actionbar(char str[], unsigned char colour);
extern void move_csr(void);
extern void clear();
extern void ClearScreen();
extern void putch(unsigned char c);
extern void puts(unsigned char *text);
extern void puti( int num );
extern void settextcolor(unsigned char color);
extern void display_cursor_loc();
extern void init_video(void);

/* commands.c (commands for shell) */

void cmd_null(uint8_t argc, char * argv[]);
void help(uint8_t argc, char * argv[]);
void hello(uint8_t argc, char * argv[]);
void echoCMD(uint8_t argc, char * argv[]);
void displayTime(uint8_t argc, char * argv[]);
void clearShell(uint8_t argc, char * argv[]);
void displayCursorLoc(uint8_t argc, char * argv[]);
void outputRam(uint8_t argc, char * argv[]);
void outputStats(uint8_t argc, char * argv[]);
void restart(uint8_t argc, char * argv[]);
void shutdown(uint8_t argc, char * argv[]);
extern void initKernelCmds();

/* KinDOS.c (shell) */

extern void changePrompt();
extern void promptLine();
extern void startKinDOS();
extern void kinDOSInstall();
extern void activateCMD();
extern void bufferStack(uint8_t add);

/* ECHO */
extern void echo (uint8_t count, char * str[]);
extern void output_ram();
extern void output_stats();

/* DATE.C */
extern void display_time();

/* MEVAL.C */
extern void do_math (unsigned char * expression);

#endif
