/* Desc: Global Declaration of functions and values needed for the KinDOS terminal */

#ifndef __SHELL_H
#define __SHELL_H

unsigned char default_prompt[100];
/* CONSOLE.C */
void alter_pl(unsigned int pl);
void alter_x(unsigned int new_x);
void alter_y(unsigned int new_y);
void alter_xy(unsigned int new_x, unsigned int new_y);
void scroll(void);
void create_titlebar(unsigned char colour);
void write_titlebar(unsigned char *str, unsigned char colour);
void create_actionbar(unsigned char colour);
void write_actionbar(unsigned char *str, unsigned char colour);
void move_csr(void);
void clear();
void ClearScreen();
void putch(unsigned char c);
void puts(unsigned char *text);
void puti( int num );
void settextcolor(unsigned char color);
void display_cursor_loc();
void init_video(void);

/* KinDOS.c (shell) */
extern void change_prompt(unsigned char *ptext);
extern void prompt_line(unsigned short int activate);
extern void start_kin_dos();
extern void kin_dos_install();
extern void activate_cmd();
extern void buffer_stack(unsigned char add);

/* ECHO */
extern void echo (unsigned char * c1);
extern void output_ram();

/* DATE.C */
extern void display_time ();

/* MATH.C */
extern void do_math (unsigned char * expression);
extern void puti(int c);

#endif
