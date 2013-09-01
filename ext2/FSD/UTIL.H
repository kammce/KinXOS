#ifndef __FSDUTIL_H
#define __FSDUTIL_H

#include "ext2fs.h"


/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/

dos_time dateUnix2Dos(ULONG UnixDate);

int		toDos8_3(char *LongFileName, char *ShortFileName, int Offset);
int		isEntryMatch(char *EntryName, char *ToBeMatchedName, int MatchFlags, int EntryLen);
char*	GetBcsPath(int Drive, USHORT *UPath, char *BcsPath);
void	LfnToShortName(char *EntryName, int EntryLen, char *ShortName, int Offset);
void	BcsToUniParsed(char *BcsDirName, PUSHORT UniParsed);
void	LongToShort(char *LfnName, int LfnNameLen, char *ShortName, ULONG Tail);
BOOL	isDotOrDotDot(char *FileName, int Len);
char*	strnchr(char *Str, char C, int Len);
void	StrToUpper(char *Src, int Len);
BOOL	StrConvAndTestToLower(char *Dst, char *Src, int Len);
void	SmartConvertDots2QuestionMark(char *FileName);

#endif			/* __FSDUTIL_H */