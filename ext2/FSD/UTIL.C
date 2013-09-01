#include  "vxd.h"

#include "shared\vxddebug.h"
#include "util.h"
#include "dir.h"
#include "unixstat.h"


#define IS_WILDCHAR(c) ( (c)=='?' || (c)=='*' )

/**********************************
 *
 * STATIC DATA
 *
 **********************************/


	/* 
	 * Linear day numbers of the respective 1sts in non-leap years. 
	 * JanFebMarApr May Jun Jul Aug Sep Oct Nov Dec 
	 */
static long sDay_N[] = { 0,31,59,90,120,151,181,212,243,273,304,334,0,0,0,0 };
char 		sBadChars[] = "*?<>\\$+=;";
		  

/**********************************
 *
 * STATIC HELPERS
 *
 **********************************/

/* Cool Long to Short file name conversion routine.
 *
 * PRE:
 * If isFCB is true --> ShortName must be at least 11 bytes long
 * otherwise is must be at least 13 bytes long
 *
 * POST:
 * When converting LFN to 8.3 names, we will prevent name clashes due
 * to case sensitivity, that is, filename.c and Filename.c cannot both
 * convert to FILENAME.C. Either one of them has to be converted to
 * FILENA~1.C (or something like that). Since in UNIX file systems,
 * lower cased file names are more natural, we apply the following
 * conversion rule: if a filename contains an _uppercased_ character,
 * it will have a numerical tail appended (the ~1). Of course, filenames
 * that have too long file name (>8) or extention lenghts (>3) or
 * contain invalid characters will have a numerical tail appended as
 * well.
 *
 * IN:
 *	LfnName		: pointer to LfnName, it not be zero terminated
 *	LfnNameLen	: lenght of LfnName (excluding a possible '\0')
 *	isFCB		: specifies whether or not the converted shortname
 *					will be in FCB format
 * OUT:
 *	ShortName	: pointer to a buffer for the converted shortname
 */
static BOOL LongToUpperShort(char *LfnName, int LfnNameLen, char *ShortName)
{
	char		*pc;
	char		c;
	int			i;
	BOOL		isconverted;

	isconverted = FALSE;
	pc = ShortName;

		/*
		 * . and .. need no conversion
		 */
	if (isDotOrDotDot(LfnName, LfnNameLen))
	{
		strncpy(ShortName, LfnName, LfnNameLen);
		ShortName[LfnNameLen] = 0;
		return FALSE;
	}

		/*
		 * convert hidden files (starting with a dot) to underscore
		 */
	if (LfnName[0]=='.')
	{
		LfnName++;
		LfnNameLen--;
		*pc++ = '_';
	}

		/*
		 * copy filename part
		 */
	for (i=0 ; i<8 && LfnNameLen ; i++)
	{
		c = *LfnName++;
		LfnNameLen--;

			/*
			 * if '.' we found the extention
			 */
		if (c=='.')
			break;

			/*
			 * if c is present in the bad character list, we
			 * convert it to an underscore
			 */
		if (strchr(sBadChars, c))
		{
			isconverted = TRUE;
			c = '_';
		}
		else if (isalpha(c))
			if (isupper(c))
				isconverted = TRUE;
			else
				c = toupper(c);

		*pc++ = c;
	}
		
		/*
		 * If we scanned 8 characters, the filename still
		 * contains characters and we do not see a '.'
		 * the filename part is too long
		 */
	if (i==8 && LfnNameLen && (*LfnName != '.'))
		isconverted = TRUE;

		/*
		 * search dot for extension of the filename
		 */
	for(; LfnNameLen && c!='.' ; LfnNameLen--, c=*LfnName++)
		;

		/*
		 * copy extension
		 */
	if (c == '.')
		*pc++ = '.';

		/*
		 * copy the extension part
		 */
	for (i=0 ; i<3 && LfnNameLen ; i++)
	{
		c = *LfnName++;
		LfnNameLen--;

			/*
			 * if c is present in the bad character list or is a
			 * '.' (a second '.'), we convert it to 
			 * an underscore
			 */
		if (c == '.' || strchr(sBadChars, c))
		{
			isconverted = TRUE;
			c = '_';
		}
		else if (isalpha(c))
			if (isupper(c))
				isconverted = TRUE;
			else
				c = toupper(c);

		*pc++ = c;
	}

		/*
		 * If, at this point, the filename still contains characters
		 * the file extension part is too long
		 */
	if (LfnNameLen)
		isconverted = TRUE;

	*pc = 0;

	return isconverted;
}

/*
 * Append a numerical tail to a filename. This mat cause the loss
 * of some characters.
 *
 * PRE:
 *
 * POST
 * IN:
 *	ShortName	: pointer to a shortname
 *	Offset		: numerical tail
 *	isFCB		: whether or not ShortName is in FCB format
 * OUT:
 *	ShortName	: the converted shortname (that is, with the numerical
 *					tail appended)
 */
static void AppendNumericalTail(char *ShortName, ULONG Tail)
{
	char		*pc;
	int			tailstart, taillength, shift, extlength, filenamelength;


		/*
		 * tail is ~x where x = Offset mod 100, this means that
		 * the tail is ~1 until ~99. It's length is 2 or 3
		 */
	taillength = Tail < 10 ? 2 : 3;

		/*
		 * Search . or eos (it's the end od the num tail)
		 */
	if (!(pc = strchr(ShortName, '.')))
		pc = memchr(ShortName, 0, 14);

		/*
		 * determine where to start the tail and if the
		 * we need to overwrite or shift some char
		 */
	filenamelength = pc - ShortName;
	shift = 8 - filenamelength;
	if (shift > taillength)
		shift = taillength;
	tailstart = filenamelength + taillength;
	tailstart = (filenamelength + taillength > 8) ?
		8 - taillength:
		filenamelength;
		

		/*
		 * See if we need to shift the extention
		 */
	if (shift)
	{
		extlength = strlen(pc) + 1;	// copy eos as well
		memmove(pc + shift, pc , extlength);
	}

		/*
		 * Append the tail
		 */
	pc = ShortName + tailstart;
	*pc++ = '~';
	if (Tail > 9)
		*pc++ = (char) ((Tail / 10) + '0');
	*pc = (char) ((Tail % 10) + '0');
}




/**********************************
 *
 * INTERFACE ROUTINES
 *
 **********************************/


/* 
 * This comes strait from the Linux kernel source. I took it from
 * the msdos filesystem. Nasty piece of code, but it works :-)
 */
dos_time dateUnix2Dos(ULONG UnixDate)
{
	long		day, year, nl_day, month;
	ULONG		time, date;
	dos_time	dt;

	time = (UnixDate % 60)/2+(((UnixDate/60) % 60) << 5)+
		(((UnixDate/3600) % 24) << 11);
	day = UnixDate/86400l-3652;
	year = day/365;
	if ((year+3)/4+365*year > day) 
		year--;
	day -= (year+3)/4+365*year;
	if (day == 59 && !(year & 3)) 
	{
		nl_day = day;
		month = 2;
	}
	else 
	{
		nl_day = (year & 3) || day <= 59 ? day : day-1;
		for (month=0; month < 12; month++)
			if (sDay_N[month] > nl_day) 
				break;
	}
	date = nl_day-sDay_N[month-1]+1+(month << 5)+(year << 9);

	dt.dt_time = (USHORT) time;
	dt.dt_date = (USHORT) date;

	return dt;
}



/*
 * Convert A LFN Filename to a 8.3 filename
 *
 * IN:
 * pLfnName			: Name of an ext2 directory entry
 * EntryLen			: Length of bytes of pLfnName
 *
 * OUT:
 * ShortName		: ASCIIZ 8.3 filename
 *
 */
void LfnToShortName(char *EntryName, int EntryLen, char *ShortName, int Offset)
{
	USHORT		uni_lfn[MAX_PATH];
	USHORT		uni_short[14];
	_QWORD		qword;
	DWORD		rval;

		/*
		 * Convert to Unicode Short FileName, make sure we do not
		 * convert . or ..
		 */
	if (isDotOrDotDot(EntryName, EntryLen))
	{
		strncpy(ShortName, EntryName, EntryLen);
		ShortName[EntryLen] = 0;
	}
	else
	{
			/*
			 * Convert BCS LFN to Unicode LFN FileName
			 */
		BCSToUni(uni_lfn, EntryName, EntryLen, BCS_OEM, &qword);
		uni_lfn[qword.ddLower >> 1] = 0;

			/*
			 * Create the Unicode 8.3 name
			 */
		rval = CreateBasis(uni_short, uni_lfn, qword.ddLower);
		if (rval & (BASIS_LOSS | BASIS_UPCASE | BASIS_TRUNC))
			AppendBasisTail(uni_short, (INT) Offset%100);

			/*
			 * Convert to BCS
			 */
		UniToBCS(ShortName, uni_short, 2 * ustrlen(uni_short), 12, BCS_OEM, &qword);
		ShortName[qword.ddLower] = 0;
	}
}


/*
 * Compare an EntryName with a string that contains wildcards (* and ?)
 *
 * IN:
 * EntryName		: Name of an ext2 directory entry
 * ToBeMatchedName	: Match string (possibly containing wildcards)
 * MatchFlags		: Flags that specify the match scheme
 *
 * OUT:
 *
 * RETURN
 * success			: non - 0
 * failure			: 0
 */
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


/*
 * Convert an Unicode Path name to a BcsPathName, also, 
 * The converted directory is absolute (starting with root)
 * even if the Unicode pathname is relative to the current 
 * directory. It accepts \ and / as directory separators.
 * The pathnames may be separated by both \ and / but are
 * all converted to / (to facilitate the following of symbolic
 * links)
 * The converted pathname does not start with the root /
 * So, "Q:\usr\local\bin" results in "usr/local/bin"
 *
 * IN:
 * UniPath	: pointer to unicode pathname, it may be
 *			  relative to the current directory
 *
 * OUT:
 * BcsPath	: pointer to bcs pathname (absolute to root)
 * 
 * RETURNS
 * BcsPath	: pointer to bcs pathname (absolute to root)
 */
char* GetBcsPath(int Drive, USHORT *UniPath, char *BcsPath)
{
	char			cur_dir[MAX_PATH];
	char			*pc;
	_QWORD			qword;
	
		/*
		 * Skip drive letter
		 */
	if ( ((UCHAR) UniCharToOEM(UniPath[1])) == ':' )
		UniPath+=2;

		/*
		 * Absolute path?
		 */
	if ( ((UCHAR) *UniPath) == '\\' || ((UCHAR) *UniPath) == '/')
	{
			/*
			 * Skip root character
			 */
		UniPath++;
		UniToBCS(BcsPath, UniPath, 2 * ustrlen(UniPath), MAX_PATH-1, BCS_OEM, &qword);
		BcsPath[qword.ddLower] = 0;
	}
	else
	{
			/*
			 * We translate the path relative to the current directory
			 */
		cur_dir[0] = 0;
		if (IFSMgr_Win32GetVMCurdir(Drive, cur_dir) != 0)
		{
			VxdDebugPrint(D_ERROR, "GetBcsPath could not get current dir from IFS");
			return 0;
		}
			/*
			 * append a directory separator, unless we are the root
			 * directory
			 */
		if (*cur_dir)
			strcat(cur_dir, "/");

		UniToBCS(BcsPath, UniPath, 2 * ustrlen(UniPath), MAX_PATH-1, BCS_OEM, &qword);
		BcsPath[qword.ddLower] = 0;

		strcat(cur_dir, BcsPath);
		strcpy(BcsPath, cur_dir);
	}

		/*
		 * fixup separators
		 */
	pc = BcsPath;
	while (*pc)
	{
		if (*pc == '\\')
			*pc = '/';
		pc++;
	}

	VxdDebugPrint(D_DIR, "GetBcsPath=%s", BcsPath);
	return BcsPath;
}


/*
 * Translate Unix file attributes to Win 95 attributes
 *
 * IN:
 *	Ext2Inode	: pointer to ext2_inode structure
 *	FileName	: pointer to Filename, need not be zero terminated and
 *					 may be zero
 *	NameLen		: lenght of the filename (if Filename not zero)
 *
 * OUT:
 *	<none>
 *
 * RETURNS
 * Returns the Win 95 for this inode. If the filename starts with a
 * dot and it is not . or .. we set the hidden attribute
 */







void BcsToUniParsed(char *BcsDirName, PUSHORT UniParsed)
{
	USHORT		len, fileoffset, pathlen;
	PUSHORT		unipath;
	char		*pathstart, *pathend;
	_QWORD		result;

	pathstart = pathend = BcsDirName;
	unipath = UniParsed+2;
	fileoffset = 4;
	len = 0;
	if (*pathstart)
	{	
		while(pathend)
		{
				/*
				 * Stop criterium is reached iff pathend == 0,
				 * that is, there are no more path separators
				 */
			if ((pathend = strchr(pathstart, '/')))
			{
				pathlen = pathend - pathstart;
			}
			else
			{
				pathlen = strlen(pathstart);
				fileoffset = unipath - UniParsed;
			}
			BCSToUni(unipath+1, pathstart, pathlen, BCS_OEM, &result);
			*unipath = (USHORT) result.ddLower + 2;
			len += *unipath;
			unipath += (*unipath) >> 1;

				/*
				 * Continue search
				 */
			pathstart = pathend + 1;
		}
	}
	
	(*unipath) = 0;
	UniParsed[0] = len;
	UniParsed[1] = fileoffset;

}


void LongToShort(char *LfnName, int LfnNameLen, char *ShortName, ULONG Tail)
{
	char	c;
	int		len;

	*ShortName = 0;
	if (LongToUpperShort(LfnName, LfnNameLen, ShortName))
		AppendNumericalTail(ShortName, Tail%100);

	if ( (len = strlen(ShortName) > 12) )
	{
			/*
			 * panic mode to see if conversion went well.
			 * Also, use a cludge a pass missing feature in
			 * VtoolsD printf (%*s) for non-zero terminated trings
			 */
		c = LfnName[LfnNameLen-1];
		LfnName[LfnNameLen-1] = 0;
		VxdDebugPrint(D_ERROR, "PANIC, LongToShort: %s%c --> %s, len=%i",
			LfnName, (int) c, ShortName, len);
		LfnName[LfnNameLen-1] = c;
		VxdDebugFlush();
	}
}


char* strnchr(char *str, char c, int len)
{
	while(*str && len)
	{
		if (*str == c)
			return str;
		str++;
		len--;
	}

	return 0;
}

/*
 * Convert an input string to a lowercase character string.
 * Return TRUE iff the input string does _not_ contain
 * lower cased characters.
 * Output string is not zero-terminated
 */
BOOL StrConvAndTestToLower(char *Dst, char *Src, int Len)
{
	char	c;

	while (Len)
	{
		c = *Src++;
		if (isalpha(c))
		{
			if (islower(c))
				return FALSE;
			*Dst++ = tolower(c);
		}
		else
			*Dst++ = c;
		Len--;
	}
	return TRUE;
}


void StrToUpper(char *Src, int Len)
{
	char	c;

	while (Len--)
	{
		c = *Src;
		if (isalpha(c))
			*Src++ = toupper(c);
		else
			Src++;
	}
}


BOOL isDotOrDotDot(char *FileName, int Len)
{
	if (FileName[0] == '.')
	{
		if (Len == 1)
			return TRUE;
		else if (FileName[1] == '.' && Len == 2)
			return TRUE;
	}

	return FALSE;
}

void SmartConvertDots2QuestionMark(char *FileName)
{
	char	*pc;

	pc = strchr(FileName + 1, '.');
	while(pc)
	{
		if ( IS_WILDCHAR( *(pc-1)) || IS_WILDCHAR(*(pc+1)) )
			*pc = '?';
		pc = strchr(pc + 1, '.');
	}
}
