#include <core.h>
#include <shell.h>

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
void initKernelCmds();

//const kernel_cmds_t empty_kernel_cmd = { "hello", &hello };
// Array of Kernel commands
kernel_cmds_t kernel_cmds[] = {
	{"default", cmd_null},
	{"help", help},
	{"hello", hello},
	{"echo", echoCMD},
	{"date", displayTime},
	{"clear", clearShell},
	{"cursor", displayCursorLoc},
	{"ram", outputRam},
	//{"stats", outputStats},
	{"reboot", restart},
	{"shutdown", shutdown},
	{0,0}
};
// =================== COMMANDS GO HERE =================== //

void cmd_null(uint8_t argc, char * argv[]) {
	settextcolor(0x40); puts("That command does not exist.");
}
void help(uint8_t argc, char * argv[]) {
	puts("Available Commands: ");
	puts("\nhelp  - Lists the available commands and this prompt.");
	puts("\nhello - Welcome Program.");
	puts("\necho  - display a line of text");
	puts("\ndate  - display date from bios");
	//puts("\nmath  - do simple arithmetic");
	puts("\ncursor - display cursor location");
	puts("\nclear  - clear screen");
	puts("\nram    - print out ram information and usage [BROKEN]");
	//puts("\nstats  - print out computer stats [BROKEN]");
	puts("\nreboot - reboot computer");
}
void hello(uint8_t argc, char * argv[]) {
	printf("Hello! This is KinX's First Command. Thank you for using it!");
}
void echoCMD(uint8_t argc, char * argv[]) {
	echo(argc, argv);
}
void displayTime(uint8_t argc, char * argv[]) {
	display_time();
}
void clearShell(uint8_t argc, char * argv[]) {
	ClearShell();
}
void displayCursorLoc(uint8_t argc, char * argv[]) {
	display_cursor_loc();
}
void outputRam(uint8_t argc, char * argv[]) {
	output_ram();
}
void outputStats(uint8_t argc, char * argv[]) {
	output_stats();
}
void restart(uint8_t argc, char * argv[]) {
	reboot();
}
void shutdown(uint8_t argc, char * argv[]) {
	reboot();
}
// =================== COMMANDS END HERE ================== //
void initKernelCmds() {
	printf("Kernel Command List initialized!!!!");
}