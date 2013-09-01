// SPY.c - main module for SPY 

// This dynamic VxD outputs information to the debug console for each
// IFS call.

#define   DEVICE_MAIN
#include  "spy.h"
#undef    DEVICE_MAIN

#include "vxddebug.h"

Declare_Virtual_Device(SPY)

ppIFSFileHookFunc PrevHook;

DefineControlHandler(SYS_DYNAMIC_DEVICE_INIT, OnSysDynamicDeviceInit);
DefineControlHandler(SYS_DYNAMIC_DEVICE_EXIT, OnSysDynamicDeviceExit);

struct IFSFunctionNameID_t
{
	int	ifs_fcnid;
	char*	ifs_fcnname;
};

struct IFSFunctionNameID_t FuncTable[] = {
	{IFSFN_READ, "Read"},
	{IFSFN_WRITE, "Write"},
	{IFSFN_FINDNEXT, "Findnext"},
	{IFSFN_FCNNEXT, "Fcnnext"},
	{IFSFN_SEEK, "Seek"},
	{IFSFN_CLOSE, "Close"},
	{IFSFN_FINDCLOSE, "Findclose"},
	{IFSFN_FCNCLOSE, "Fcnclose"},
	{IFSFN_COMMIT, "Commit"},
	{IFSFN_FILELOCKS, "Filelocks"},
	{IFSFN_FILETIMES, "Filetimes"},
	{IFSFN_PIPEREQUEST, "Piperequest"},
	{IFSFN_HANDLEINFO, "Handleinfo"},
	{IFSFN_ENUMHANDLE, "Enumhandle"},
	{IFSFN_CONNECT, "Connect"},
	{IFSFN_DELETE, "Delete"},
	{IFSFN_DIR, "Dir"},
	{IFSFN_FILEATTRIB, "Fileattrib"},
	{IFSFN_FLUSH, "Flush"},
	{IFSFN_GETDISKINFO, "Getdiskinfo"},
	{IFSFN_OPEN, "Open"},
	{IFSFN_RENAME, "Rename"},
	{IFSFN_SEARCH, "Search"},
	{IFSFN_QUERY, "Query"},
	{IFSFN_DISCONNECT, "Disconnect"},
	{IFSFN_UNCPIPEREQ, "Uncpipereq"},
	{IFSFN_IOCTL16DRIVE, "Ioctl16drive"},
	{IFSFN_GETDISKPARMS, "Getdiskparms"},
	{IFSFN_FINDOPEN, "Findopen"},
	{IFSFN_DASDIO, "Dasdio"},
};

BOOL ControlDispatcher(
	DWORD dwControlMessage,
	DWORD EBX,
	DWORD EDX,
	DWORD ESI,
	DWORD EDI,
	DWORD ECX)
{
	START_CONTROL_DISPATCH

		ON_SYS_DYNAMIC_DEVICE_INIT(OnSysDynamicDeviceInit);
		ON_SYS_DYNAMIC_DEVICE_EXIT(OnSysDynamicDeviceExit);

	END_CONTROL_DISPATCH

	return TRUE;
}

char* GetFunctionName(int ID)
{
	int i;

	for (i=0; i < sizeof(FuncTable)/sizeof(struct IFSFunctionNameID_t); i++)
		if (FuncTable[i].ifs_fcnid == ID)
			return FuncTable[i].ifs_fcnname;

	return "Unknown";

}

char* GetResTypeString(int restype)
{
	switch (restype)
	{
	case IFSFH_RES_UNC:
		return "IFSFH_RES_UNC";
	case IFSFH_RES_NETWORK:
		return "IFSFH_RES_NETWORK";
	case IFSFH_RES_LOCAL:
		return "IFSFH_RES_LOCAL";
	case IFSFH_RES_CFSD:
		return "IFSFH_RES_CFSD";
	case IFSFH_RES_NETWORK + IFSFH_RES_UNC:
		return "IFSFH_RES_NETWORK (UNC)";
	case IFSFH_RES_LOCAL + IFSFH_RES_UNC:
		return "IFSFH_RES_LOCAL (UNC)";
	case IFSFH_RES_CFSD + IFSFH_RES_UNC:
		return "IFSFH_RES_CFSD (UNC)";
	default:
		return "Unknown";
	}
}

int _cdecl MyIfsHook(pIFSFunc pfn, int fn, int Drive, int ResType,
		int CodePage, pioreq pir)
{
	int	rval;
	static int		Count = 0;

		/*
		 * Only log drive D
		 */
	if (Drive -1 != 'D' - 'A')
		return (*PrevHook)(pfn, fn, Drive, ResType, CodePage, pir);

	switch(fn)
	{
		case IFSFN_SEEK: 
			if (pir->ir_flags == FILE_BEGIN)
				VxdDebugPrint(D_SEEK, "sfn=%lu, pos=%lu, from BEGIN", (ULONG)pir->ir_sfn, (ULONG) pir->ir_pos, (ULONG) pir->ir_flags);
			else
				VxdDebugPrint(D_SEEK, "sfn=%lu, pos=%i, from END", (ULONG)pir->ir_sfn, (int) pir->ir_pos, (ULONG) pir->ir_flags);
			break;

		case IFSFN_WRITE:
			VxdDebugPrint(D_WRITE, "sfn=%lu, len=%lu, pos=%lu, options=0x%x", (ULONG) pir->ir_sfn, (ULONG) pir->ir_length, (ULONG) pir->ir_pos, (ULONG) pir->ir_options);
			break;

		case IFSFN_OPEN:
			VxdDebugPrint(D_OPENCLOSE, "sfn=%lu, flags=0x%x, options=0x%x, attr=0x%x", (ULONG) pir->ir_sfn, (ULONG) pir->ir_flags, (ULONG) pir->ir_options, (ULONG) pir->ir_attr);
			break;

	
		case IFSFN_READ:
			VxdDebugPrint(D_READ, "sfn=%lu, len=%lu, pos=%lu, options=0x%x", (ULONG) pir->ir_sfn, (ULONG) pir->ir_length, (ULONG) pir->ir_pos, (ULONG) pir->ir_options);
			break;

		case IFSFN_CLOSE:
			VxdDebugPrint(D_OPENCLOSE, "sfn=%lu, options=0x%x, flags=0x%x", (ULONG) pir->ir_sfn, (ULONG) pir->ir_options, (ULONG) pir->ir_flags);
			break;

		default:
			;
/*
			VxdDebugPrint(D_ALWAYS, "IFS(%4lu): Call %d(%s)\tDrive: %c:  Res=%04x(%s)\tCP: %s",
				fn, GetFunctionName(fn),
				'A'-1+Drive,
				ResType, GetResTypeString(ResType),
				(CodePage==BCS_WANSI) ? "ANSI":"OEM");
*/
	}

	VxdDebugFlush();

	rval = (*PrevHook)(pfn, fn, Drive, ResType, CodePage, pir);

	switch(fn)
	{
		case IFSFN_SEEK: 
			VxdDebugPrint(D_SEEK, "sfn=%lu, error=%lu, pos=%lu", (ULONG) pir->ir_sfn, (ULONG) pir->ir_error, (ULONG) pir->ir_pos);
			break;

		case IFSFN_WRITE:
			VxdDebugPrint(D_WRITE, "sfn=%lu, error=%lu, len=%lu, pos=%lu", (ULONG) pir->ir_sfn, (ULONG) pir->ir_error, (ULONG) pir->ir_length, (ULONG) pir->ir_pos);
			break;

		case IFSFN_OPEN:
			VxdDebugPrint(D_OPENCLOSE, "file=%U, sfn=%lu, error=%lu, size=%lu, options=0x%x, attr=0x%x", pir->ir_upath, (ULONG) pir->ir_sfn, (ULONG) pir->ir_error, (ULONG) pir->ir_size, (ULONG) pir->ir_options, (ULONG) pir->ir_attr);
			break;

	
		case IFSFN_READ:
			VxdDebugPrint(D_READ, "sfn=%lu, error=%lu, len=%lu, pos=%lu", (ULONG) pir->ir_sfn, (ULONG) pir->ir_error, (ULONG) pir->ir_length, (ULONG) pir->ir_pos);
			break;

		case IFSFN_CLOSE:
			VxdDebugPrint(D_OPENCLOSE, "sfn=%lu, error=%lu, pos=%lu", (ULONG) pir->ir_sfn, (ULONG) pir->ir_error, (ULONG) pir->ir_pos);
			break;

		default:
/*
			VxdDebugPrint(D_ALWAYS, "IFS: Call %d(%s) done",
				fn, GetFunctionName(fn));
*/
			;
	}

	VxdDebugFlush();

	return rval;
}

BOOL OnSysDynamicDeviceInit()
{
	VxdDebugInitialise(D_ALL, DOUT_FILE, "e:\\spy");

	VxdDebugPrint(D_ALWAYS, "Spy, starting up");

	PrevHook = IFSMgr_InstallFileSystemApiHook(MyIfsHook);

	return TRUE;
}

BOOL OnSysDynamicDeviceExit()
{
	IFSMgr_RemoveFileSystemApiHook(MyIfsHook);

	VxdDebugPrint(D_ALWAYS, "Spy, bye bye!");

	return TRUE;
}
