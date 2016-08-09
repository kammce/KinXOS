/* Desc: Global Declaration of functions used in stdio terminal */

#ifndef __VIDEO_H
#define __VIDEO_H

// Video Colors
#define BLACK         0x0
#define BLUE          0x1
#define GREEN         0x2
#define CYAN          0x3
#define RED           0x4
#define MAGENTA       0x5
#define BROWN         0x6
#define LIGHTGRAY     0x7
#define DARKGRAY      0x8
#define LIGHTBLUE     0x9
#define LIGHTGREEN    0xA
#define LIGHTCYAN     0xB
#define LIGHTRED      0xC
#define LIGHTMAGENTA  0xD
#define LIGHTBROWN    0xE
#define WHITE         0xF

#define ATTRIBFONT(fore,back) (fore) | (back<<4)

#endif