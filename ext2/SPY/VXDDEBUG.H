#ifndef __VXDDEBUG_H
#define __VXDDEBUG_H

#define DOUT_CONSOLE    1		/* dumps to console */
#define DOUT_FILE		2		/* dumps to console */
#define DOUT_NO_EOL 	0x8000	/* don't appends an eol */


#define D_ALWAYS		0x00
#define D_ERROR			0x01
#define D_OPENCLOSE		0x02
#define D_READ			0x04
#define D_WRITE			0x08
#define D_SEEK			0x10
#define D_END			0x10
#define D_ALL (D_END * 2 - 1)



/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/


#ifdef DEBUG
int		VxdDebugSetLevel(int NewLevel);
int		VxdDebugGetLevel();
int		VxdDebugSetOut(int NewOut, char *DebugFileName);
void	VxdDebugInitialise(int Level, int Out, char *DebugFileName);
void	VxdDebugCleanup();
void	VxdDebugFlush();
void __cdecl VxdDebugPrint(int Flag, char *Format, ...);
#else
#define VxdDebugSetLevel	//
#define VxdDebugSetOut()	//
#define VxdDebugInitialise	//
#define VxdDebugCleanup()	//
#define VxdDebugPrint()		//
#define VxdDebugFlush()		//
#define VxdDebugGetLevel()	//
#pragma warning(disable:4002)	
#endif

#endif			/* __VXDDEBUG_H */
