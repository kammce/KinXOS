#include "spy.h"
#include "vxddebug.h"


/*
 * Only compile this module for the debug
 * version
 * 
 */
#ifdef DEBUG

#define		DEBUG_STR_MAX			256
#define		MAX_DEBUG_FILE_NAME		64
#define		MAX_DEBUG_IN_MEM		0xffff

/*********************************
 *
 * GLOBALS AND STATIC DATA
 *
 **********************************/

static MUTEXHANDLE		sMutex = 0;
static int 				sCurrentDebugLevel = 0;
static int 				sCurrentDebugOut = DOUT_CONSOLE;

static char				sDebugFileName[MAX_DEBUG_FILE_NAME];
static DWORD			sDebugFileOffset = 0;
static char				*sDebugCachedLog = 0;
static DWORD			sDebugCachedLen = 0;
static char 			*sDebugPrefix[]= 
		{
			"",
			"Error",
			"OpenClose",
			"Read",
			"Write",
			"Seek"
		};

static char				*sModuleName = "SPY";


/**********************************
 *
 * STATIC HELPERS
 *
 **********************************/




/*********************************
 *
 * INTERFACE ROUTINES
 *
 **********************************/

int VxdDebugGetLevel()
{
	return sCurrentDebugLevel;
}


void VxdDebugFlush()
{
	WORD		error;
	DWORD		written;
	BYTE		action;
	HANDLE		filehandle;


	EnterMutex(sMutex, BLOCK_THREAD_IDLE);

		/*
		 * Only flush, if we are logging to file and there is
		 * something to log
		 */
	if (sCurrentDebugOut != DOUT_FILE || !sDebugCachedLen)
		goto flush_done;

	if ((filehandle = R0_OpenCreateFile(TRUE, sDebugFileName, 2, 0, 0x11, 0, &error, &action)))
	{
		written = R0_WriteFile(TRUE, filehandle, sDebugCachedLog, sDebugCachedLen, sDebugFileOffset, &error);
		if (error)
		{
			sCurrentDebugOut = DOUT_CONSOLE;
			VxdDebugPrint(D_ERROR, "VxdDebugPrint: could not writefile, error=%i", (int) error);
		}
		else if (written != sDebugCachedLen)
		{
			sCurrentDebugOut = DOUT_CONSOLE;
			VxdDebugPrint(D_ERROR, "VxdDebugPrint: writefile not atomic: write=%lu, written=&lu", (ULONG) sDebugCachedLen, (ULONG) written);
			sCurrentDebugOut = DOUT_FILE;
		}

		R0_CloseFile(filehandle, &error);
		sDebugFileOffset += sDebugCachedLen;
	}
	else
	{
		sCurrentDebugOut = DOUT_CONSOLE;
		VxdDebugPrint(D_ERROR, "VxdDebugPrint: could not openfile(%s), error=%i", sDebugFileName, (int) error);
	}
	sDebugCachedLen = 0;

flush_done:
	LeaveMutex(sMutex);
}

/*
 * Set new debug level
 *
 * PRE CONDITIONS
 *	<none>
 *
 * POST CONDITIONS
 *	The debug level is set to NewLevel. The old level is returned.
 *	This routine is thread safe by using a mutex;
 *
 * IN:
 *	NewLevel	: New debug Level. Specify individual bits from D_xxx
 *
 * OUT:
 *	<none>
 *
 * RETURN
 *	Returns the previously debug level
 */

int VxdDebugSetLevel(int NewLevel)
{
	int old;
	
	EnterMutex(sMutex, BLOCK_THREAD_IDLE);

	old = sCurrentDebugLevel;
	sCurrentDebugLevel = NewLevel;
	if (sCurrentDebugLevel > D_ALL)
		sCurrentDebugLevel = D_ALL;

	LeaveMutex(sMutex);

	return old;
}



/*
 * Set new debugging medium
 *
 * PRE CONDITIONS
 *	<none>
 *
 * POST CONDITIONS
 *	The debug medium is set to NewOut. The old medium is returned.
 *	This routine is thread safe by using a mutex;
 *
 * IN:
 *	NewOut		: New debug medium (one of the DOUT_xxx values)
 *
 * OUT:
 *	<none>
 *
 * RETURNS:
 *	Returns the previously debug medium.
 */

int VxdDebugSetOut(int NewOut, char *DebugFileName)
{
	int		old;
	WORD	error;
	
	EnterMutex(sMutex, BLOCK_THREAD_IDLE);

	if (DebugFileName)
	{
		strncpy(sDebugFileName, DebugFileName, MAX_DEBUG_FILE_NAME);
		sDebugFileName[MAX_DEBUG_FILE_NAME - 1] = 0;
	}

	old  = sCurrentDebugOut;
	sCurrentDebugOut = NewOut;

	if (sCurrentDebugOut == DOUT_FILE)
	{
		R0_DeleteFile(sDebugFileName, 0, &error);
		sDebugFileOffset = 0;
		
		sDebugCachedLen = 0;
		if (sDebugCachedLog)
			free(sDebugCachedLog);
		if (!(sDebugCachedLog = calloc(1, MAX_DEBUG_IN_MEM)))
		{
			sCurrentDebugOut = DOUT_CONSOLE;
			VxdDebugPrint(D_ERROR, "VxdDebugSetOut: could not calloc sDebugCachedLog");
		}
		sCurrentDebugOut = DOUT_CONSOLE;
		VxdDebugPrint(D_ALWAYS, "VxdDebugSetOut: sDebugCachedLog=%x, sDebugCachedLen=%x", sDebugCachedLog, &sDebugCachedLen);
		sCurrentDebugOut = DOUT_FILE;
	}

	LeaveMutex(sMutex);

	return old;
}



/*
 * Creates a new debug output medium
 *
 * PRE CONDITIONS
 *	<none>
 *
 * POST CONDITIONS
 *	The debug medium is set to Out, the current debug level to
 *	Level.
 *
 * IN:
 *	Out		: Debug medium (one of the DOUT_xxx values)
 *	Level	: Debug Level, specify individual bits from D_xxx
 *
 * OUT:
 *	<none>
 *
 * RETURNS:
 *	<none>
 */

void VxdDebugInitialise(int Level, int Out, char *DebugFileName)
{

	sMutex = CreateMutex(0, MUTEX_MUST_COMPLETE);

	sCurrentDebugLevel = Level;
	sCurrentDebugOut = DOUT_CONSOLE;
	sDebugFileOffset = 0;

	VxdDebugSetOut(Out, DebugFileName);	

	VxdDebugPrint(D_ALWAYS, "VxdDebugInitialise: sCurrentDebugLevel=%x", &sCurrentDebugLevel);
}


/*
 * Destroys debug medium
 *
 * PRE CONDITIONS
 *	VxdDebugCreate should be called exactly once
 *
 * POST CONDITIONS
 *	All information and data associated with the debugging medium
 *	is released
 *
 * IN:
 *	<none>
 *
 * OUT:
 *	<none>
 *
 * RETURNS:
 *	<none>
 */

void VxdDebugCleanup()
{
	DestroyMutex(sMutex);
}


/*
 * VxdDebugPrint writes a debug message to the current outputmedium.
 *
 * PRE CONDITIONS:
 *	The debug medium must be initialised
 *
 * POST CONDITIONS:
 *	The Flag parameter is checked againt the Current Debug Level.
 *	If the first bit set in Flag (scanned from least significant bit
 *	to most significant bit) is set in the Current Debug Level, the
 *	message is printed. If Flag is 0 (equals D_ALWAYS), the message
 *	is always printed. An eol is always appended to the message.
 *	Format of the message:
 *		<FSD> <prefix> <count>: message
 *	In which <prefix> is a textual representation of Flag and <count>
 *	an increasing message counter.
 *	The routine is thread safe.
 *
 * The variable length specifier does not work (e.g.,%*s)
 *
 * IN:
 *	Flag		: Bit spefifying the category of the message (D_xxx)
 *	Format		: Same as for printf 
 *	...			: Same as for printf
 *
 * OUT:
 *	<none>
 *
 * RETURNS:
 *	<none>
 */
void __cdecl VxdDebugPrint(int Flag, char *Format,...)
{
	static int	debug_count=1;
	va_list		argptr;
	int			which;
	unsigned	len;
	char		str_debug[DEBUG_STR_MAX];

	EnterMutex(sMutex, BLOCK_THREAD_IDLE);

		/*
		 * Only debug if wanted
		 */
	if ( !(((sCurrentDebugLevel & Flag) || !Flag) && Flag<=D_END))
		goto debugprint_done;


	va_start(argptr, Format);

	if (!Flag)
		which=0;
	else
	{
		which=1;
		while (!(Flag & 1))
		{
			which++;
			Flag>>=1;
		}
	}

	len = 0;
	if (Flag)
		len = sprintf(str_debug, "%s:%-6s %5u:", sModuleName, sDebugPrefix[which], debug_count++);

	len += vsprintf(str_debug+len, Format, argptr);
	va_end(argptr);

	str_debug[len++] = '\r';
	str_debug[len++] = '\n';


	switch (sCurrentDebugOut)
	{
		case DOUT_FILE:
			if (sDebugCachedLen + len > MAX_DEBUG_IN_MEM)
				VxdDebugFlush();
			memcpy(sDebugCachedLog + sDebugCachedLen, str_debug, len);
			sDebugCachedLen += len;
		break;
			
		case DOUT_CONSOLE:
		default:
			str_debug[len++] = '\0';
			dprintf(str_debug);
			break;
	}

debugprint_done:
	LeaveMutex(sMutex);

#ifdef FULL_DEBUG
			/*
			 * If we are in a full debug, we flush immediately
			 */
		VxdDebugFlush();
#endif

}

#endif		/* DEBUG */
