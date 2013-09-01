#include <stdio.h>
#include <string.h>
#include <ctype.h>

//#include "vtoolsd.h"
//#include "dcb.h"

typedef unsigned long	DWORD,	*PDWORD;
typedef unsigned long	ULONG,	*PULONG;
typedef unsigned char	BYTE,	*PBYTE;
typedef unsigned char	UCHAR,	*PUCHAR;
typedef unsigned short	WORD,	*PWORD;
typedef unsigned short	USHORT,	*PUSHORT;
typedef unsigned int	UINT,	*PUINT;
typedef	void		VOID,	*PVOID,	**PPVOID;
typedef int		INT,	*PINT;
typedef int		BOOL,	*PBOOL;
typedef	long		LONG,	*PLONG;
typedef	char		CHAR,	*PCHAR, **PPCHAR;
typedef	unsigned int	size_t;
typedef int ptrdiff_t;

#include "dcb.h"

int isEntryMatch(char *EntryName, char *ToBeMatchedName, int MatchFlags, int EntryLen)
{
	char	*EntryNameEnd = EntryName + EntryLen;
	char	c1, c2;
		
	while (EntryName<EntryNameEnd && *ToBeMatchedName)
	{
		switch (*ToBeMatchedName)
		{
			case '*':
					/*
					 * Wildcard match
					 *
					 * First, look for the first non wildcard
					 */
				while (*ToBeMatchedName == '*' || *ToBeMatchedName == '?')
					ToBeMatchedName++;

					/*
					 * If we end with wildcards, we have a match
					 */
				if (*ToBeMatchedName == 0)
					return 1;

					/*
					 * Next, scan the entryname for the non-wild
					 * card character
					 */
				EntryName = memchr(EntryName, *ToBeMatchedName, EntryNameEnd-EntryName);
				if (!EntryName)
					return 0;

					/*
					 * Continue search at next character
					 */
				EntryName++;
				ToBeMatchedName++;
				break;

			case '?':
				ToBeMatchedName++;
				EntryName++;
				break;
			default:
					/*
					 * Simple case-insensitive char comparison
					 * toupper has side-effects :-)
					 */
				c1 = *EntryName++;
				c2 = *ToBeMatchedName++;
				//if (toupper(*EntryName++) != toupper(*ToBeMatchedName++))
				if (toupper(c1) != toupper(c2))
					return 0;
		}
	}

		/*
		 * We ended the while loop, this means 
		 * 1. Only ToBeMatchedName is empty
		 * 2. Only EntryName is empty
		 * 3. They are both empty
		 * 
		 */
	if (EntryName<EntryNameEnd)
	{

			/*
			 * 1. Only ToBeMatchedName is empty
			 */
		return 0;
	}
	else
	{

				/*
				 * 2. Only EntryName is empty
				 * 3. They are both empty
				 *
				 * We only have a match if only wildcards are
				 * left in ToBeMatchedName (or it is empty)
				 */
		while (*ToBeMatchedName)
		{
			if (*ToBeMatchedName == '*' || *ToBeMatchedName == '?')
				ToBeMatchedName++;
			else
				return 0;
		}
		return 1;
	}

		/*
		 * Cannot reach here
		 */
	return 0;

}


void main()
{
	int				Loop = 1;
	PDCB_COMMON		pDriveC = 0xc16434b0;
	//PDCB_COMMON		pDriveD = 0xc167bee4;
	//PDCB_COMMON		pDriveE = 0xc163a0c0;
	//PDCB_COMMON		pDriveF = 0xc167be18;
	//PDCB_COMMON		pDriveQ = 0xc1660e04;

	PDCB_COMMON		pDrive1 = (PDCB_COMMON) pDriveC->DCB_physical_dcb;

	DCB_cd_entry	*Cd = pDriveC->DCB_ptr_cd;

	isEntryMatch("NETSCAPE", "NETSCAPE", 0, 8);

	while(Loop)
		;
/*
	char	**pc;
	char	ShortName[32];
	int		offset;

	pc = FileList;
	offset = 4;

	for(; *pc ; pc++)
	{
		
		LongToShort(*pc, strlen(*pc), ShortName, offset++);
		printf("Long=%s, short=%s (len=%i)\n", *pc, ShortName, strlen(ShortName));
	}
	printf("Done\n");
*/
}
