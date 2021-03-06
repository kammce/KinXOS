/* Desc: Global Declaration of functions used in stdio terminal */

#ifndef __STDIO_H
#define __STDIO_H

// Character String set of MACROs
extern char _ctype[];

#define CT_UP	0x01	/* upper case */
#define CT_LOW	0x02	/* lower case */
#define CT_DIG	0x04	/* digit */
#define CT_CTL	0x08	/* control */
#define CT_PUN	0x10	/* punctuation */
#define CT_WHT	0x20	/* white space (space/cr/lf/tab) */
#define CT_HEX	0x40	/* hex digit */
#define CT_SP	0x80	/* hard space (0x20) */

#define isalnum(c)	((_ctype + 1)[(unsigned)(c)] & (CT_UP | CT_LOW | CT_DIG))
//#define isalpha(c)	((_ctype + 1)[(unsigned)(c)] & (CT_UP | CT_LOW))
#define isalpha(c)	((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
#define iscntrl(c)	((_ctype + 1)[(unsigned)(c)] & (CT_CTL))
#define isdigit(c)	((_ctype + 1)[(unsigned)(c)] & (CT_DIG))
#define isgraph(c)	((_ctype + 1)[(unsigned)(c)] & (CT_PUN | CT_UP | CT_LOW | CT_DIG))
#define islower(c)	((_ctype + 1)[(unsigned)(c)] & (CT_LOW))
#define isprint(c)	((_ctype + 1)[(unsigned)(c)] & (CT_PUN | CT_UP | CT_LOW | CT_DIG | CT_SP))
#define ispunct(c)	((_ctype + 1)[(unsigned)(c)] & (CT_PUN))
#define isspace(c)	((_ctype + 1)[(unsigned)(c)] & (CT_WHT))
#define isupper(c)	((_ctype + 1)[(unsigned)(c)] & (CT_UP))
#define isxdigit(c)	((_ctype + 1)[(unsigned)(c)] & (CT_DIG | CT_HEX))
#define isascii(c)	((unsigned)(c) <= 0x7F)
#define toascii(c)	((unsigned)(c) & 0x7F)
#define tolower(c)	(isupper(c) ? c + 'a' - 'A' : c)
#define toupper(c)	(islower(c) ? c + 'A' - 'a' : c)
#define cmp(c1, c2)	((c1 == c2) ? TRUE : FALSE)

int strlen(const char *str);
boolean strcmp(char c1[], char c2[]);
char * substr(char dest[], char src[], int start, int len);
char * strcpy(char dest[], char src[]);
char * strcat(char dest[], char src[]);

#endif
