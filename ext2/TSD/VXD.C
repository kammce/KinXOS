// VXD.c - main module for VxD TSDEXT2

#define   DEVICE_MAIN
#include  "vxd.h"
#undef    DEVICE_MAIN

#include "shared\vxddebug.h"
#include "shared\e2version.h"


Declare_Layered_Driver(TSDEXT2, TSDEXT2_LGN_MASK, "123456789abcdef", 1, 0, 0)

DefineControlHandler(SYS_DYNAMIC_DEVICE_INIT, OnSysDynamicDeviceInit);

BOOL __cdecl ControlDispatcher(
	DWORD dwControlMessage,
	DWORD EBX,
	DWORD EDX,
	DWORD ESI,
	DWORD EDI,
	DWORD ECX)
{
	START_CONTROL_DISPATCH

		ON_SYS_DYNAMIC_DEVICE_INIT(OnSysDynamicDeviceInit);

	END_CONTROL_DISPATCH

	return TRUE;
}


BOOL OnSysDynamicDeviceInit()
{
	VxdDebugInitialise(D_ALL, DOUT_CONSOLE, 0);
	VxdDebugPrint(D_ALWAYS, "ext2 TSD for Windows 95");
	VxdDebugPrint(D_ALWAYS, "Version %s, Peter van Sebille", VERSION_STRING);

	VxdDebugPrint(D_VXD, "OnSysDynamicDeviceInit: ILB=%x", &theILB);

	IOS_Register(&TSDEXT2_Drp);
	
	VxdDebugPrint(D_VXD, "OnSysDynamicDeviceInit: registration result=%04x", TSDEXT2_Drp.DRP_reg_result);

	return TRUE;
}


