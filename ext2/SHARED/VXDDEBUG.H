#ifndef __VXDDEBUG_H
#define __VXDDEBUG_H

#include "debug.h"

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
