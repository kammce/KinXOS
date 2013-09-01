#define   DEVICE_MAIN
#include  "vxd.h"
#undef    DEVICE_MAIN

#include "shared\e2version.h"
#include "shared\vxddebug.h"
#include "shared\blkdev.h"
#include "shared\fsdioctl.h"

#include "ext2fs.h"
#include "error.h"

#include "fsdmount.h"
#include "fsdvol.h"
#include "super.h"
#include "inode.h"
#include "block.h"
#include "dir.h"
#include "parttbl.h"


#ifdef OLD_BEHAVIOUR
Declare_Virtual_Device(FSDEXT2)
#else
Declare_Layered_Driver(FSDEXT2, FSDEXT2_LGN_MASK, "123456789abcdef", 1, 0, 0)
#endif


DefineControlHandler(SYS_DYNAMIC_DEVICE_INIT, OnSysDynamicDeviceInit);
DefineControlHandler(SYS_DYNAMIC_DEVICE_EXIT, OnSysDynamicDeviceExit);
DefineControlHandler(W32_DEVICEIOCONTROL, OnW32Deviceiocontrol);
DefineControlHandler(DEVICE_REBOOT_NOTIFY, OnDeviceRebootNotify);




/**********************************
 *
 * GLOBALS USED BY ALL MODULES
 *
 **********************************/

//ILB			theILB;

/**********************************
 *
 * STATIC DATA
 *
 **********************************/

static int			sProviderId;


/**********************************
 *
 * VXD STUFF
 *
 **********************************/


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
		ON_SYS_DYNAMIC_DEVICE_EXIT(OnSysDynamicDeviceExit);
		ON_W32_DEVICEIOCONTROL(OnW32Deviceiocontrol);
		ON_DEVICE_REBOOT_NOTIFY(OnDeviceRebootNotify);

	END_CONTROL_DISPATCH

	return TRUE;
}


BOOL OnSysDynamicDeviceInit()
{
	DRP		theDRP;
	ULONG	Reply;

		/*
		 * Initialise all modules containing state
		 */
#ifdef FULL_DEBUG
	VxdDebugInitialise(D_ALL, DOUT_FILE, "C:\\FSDEXT2.LOG");
#else
	VxdDebugInitialise(	D_ERROR | D_SYSTEM | D_VXD | D_PARTITION | 
				D_SUPER | D_DIR | D_VOLUME | 
				D_FILE, DOUT_CONSOLE, 0);
#endif
	VxdDebugPrint(D_ALWAYS, "Ext2 FSD for Windows 95");
	VxdDebugPrint(D_ALWAYS, "Version %s, Peter van Sebille", VERSION_STRING);

	VxdDebugPrint(D_ALWAYS, "OnSysDynamicDeviceInit: ILB=%x", &theILB);

	if (!BlockInitialise(128, 512))
		goto sysinit_err_1;
	if (!InodeInitialise(128))
		goto sysinit_err_2;
	if (!SuperInitialise())
		goto sysinit_err_3;
	if (!DirInitialise(128))
		goto sysinit_err_4;


		/*
		 * Ok, first register at the IOS
		 */
#ifdef OLD_BEHAVIOUR
	memset(&theDRP, 0, sizeof(theDRP));
	theDRP.DRP_ilb = &theILB;
	IOS_Register(&theDRP);
#else
	IOS_Register(&FSDEXT2_Drp);
#endif

	VxdDebugPrint(D_VXD, "OnSysDynamicDeviceInit: registered at IOS");

	PartitionInitialise();


		/*
		 * Next, we register at the IFSMgr
		 */
	sProviderId = IFSMgr_RegisterMount(FsdMount, IFSMGRVERSION, 0);
	VxdDebugPrint(D_VXD, "OnSysDynamicDeviceInit: registered at IFSMgr, mount=%x", (ULONG) FsdMount);

	VxdDebugPrint(D_ALWAYS, "OnSysDynamicDeviceInit: done");
	//VxdDebugSetOut(DOUT_FILE, "D:\\FSDEXT2.LOG");
	return TRUE;


		/*
		 * Cleanup in reverse order
		 */
sysinit_err_4:
	SuperCleanup();
sysinit_err_3:
	InodeCleanup();
sysinit_err_2:
	BlockCleanup();
sysinit_err_1:
	VxdDebugPrint(D_ALWAYS, "OnSysDynamicDeviceInit: failed");

	return FALSE;
}

BOOL OnSysDynamicDeviceExit()
{
		/*
		 * When we are here, we are not allowed to touch pageable
		 * code/data. Switch to console debugging as debugging to file
		 * uses a pageable buffer.
		 */
	VxdDebugSetOut(DOUT_CONSOLE, 0);
	VxdDebugPrint(D_ALWAYS, "OnSysDynamicDeviceExit: sorry, this is a no-no");
	return FALSE;
}


DWORD OnW32Deviceiocontrol(PIOCTLPARAMS p)
{
	char			*Table;
	DWORD			Size;
	TMountRequest	*MountRequest;
	ULONG			Reply;
	

	switch (p->dioc_IOCtlCode)
	{
		case DIOC_OPEN:
			VxdDebugPrint(D_VXD, "DEVIOCTL, ioctl=open");
			return 0;
		case DIOC_CLOSEHANDLE:
			VxdDebugPrint(D_VXD, "DEVIOCTL, ioctl=close");
			return 0;

		case FSDIOCTL_GETPARTITIONTABLE:
			VxdDebugPrint(D_VXD, "FSDIOCTL_GETPARTITIONTABLE");

			PartitionGetTable(&Table, &Size);
			if (Size > p->dioc_cbOutBuf)
			{
				VxdDebugPrint(D_ERROR, "FSDIOCTL_GETPARTITIONTABLE done, outputbuffer not large enough");
				return ERROR_INVALID_PARAMETER;
			}

			memcpy(p->dioc_OutBuf, Table, Size);
			*p->dioc_bytesret = Size;
			VxdDebugPrint(D_VXD, "FSDIOCTL_GETPARTITIONTABLE done");
			return 0;

		case FSDIOCTL_MOUNT:
			if (p->dioc_cbInBuf != sizeof(TMountRequest))
			{
				VxdDebugPrint(D_ERROR, "DEVIOCTL, FSDIOCTL_MOUNT done, invalid input buffer size");
				return ERROR_INVALID_PARAMETER;
			}
			if (p->dioc_cbOutBuf != sizeof(Reply))
			{
				VxdDebugPrint(D_ERROR, "DEVIOCTL, FSDIOCTL_MOUNT done, invalid output buffer size");
				return ERROR_INVALID_PARAMETER;
			}

			MountRequest = (TMountRequest *) p->dioc_InBuf;
			if (!PartitionMount(MountRequest->Disk, MountRequest->Partition,
					MountRequest->Drive, &Reply))
			{
				VxdDebugPrint(D_ERROR, "DEVIOCTL, FSDIOCTL_MOUNT done, parameter mismatch");
				return ERROR_INVALID_PARAMETER;
			}

			*p->dioc_bytesret = sizeof(ULONG);
			*((ULONG *) p->dioc_OutBuf) = Reply;
			VxdDebugPrint(D_ERROR, "DEVIOCTL, FSDIOCTL_MOUNT done");

			return 0;

		case FSDIOCTL_CACHE_INFO:
			DirCacheInfo();
			BlockCacheInfo();
			return 0;

		case FSDIOCTL_DEGBUG_INFO:
			{
				int		Value;

				if (p->dioc_cbInBuf != sizeof(int))
				{
					VxdDebugPrint(D_ERROR, "DEVIOCTL, FSDIOCTL_DEGBUG_INFO done, invalid input buffer size");
					return ERROR_INVALID_PARAMETER;
				}
				if (p->dioc_cbOutBuf != sizeof(int))
				{
					VxdDebugPrint(D_ERROR, "DEVIOCTL, FSDIOCTL_DEGBUG_INFO done, invalid output buffer size");
					return ERROR_INVALID_PARAMETER;
				}

					/*
					 * The non-debug version return an int of -1
					 * to indicate a non-debug version
					 */
#ifdef DEBUG
					/*
					 * The debug version understands that if
					 * < 0 is passed, the user app. requests
					 * a GetDebugLevel, otherwise it's a 
					 * DebugSetLevel request.
					 * 
					 */
				Value = ((int *) p->dioc_InBuf)[0];

				VxdDebugPrint(D_ALWAYS, "DEVIOCTL, FSDIOCTL_DEGBUG_INFO, request to set to=%i", Value); 

				if (Value < 0)
					Value = VxdDebugGetLevel();
				else
				{
					VxdDebugSetLevel(Value);
					Value = VxdDebugGetLevel();
				}
				VxdDebugPrint(D_ALWAYS, "DEVIOCTL, FSDIOCTL_DEGBUG_INFO, set to=%i", Value); 
#else
				Value = -1;
#endif
				((int *) p->dioc_OutBuf)[0] = Value;
				*p->dioc_bytesret = sizeof(int);
			}
			return 0;

		default:
			VxdDebugPrint(D_VXD, "DEVIOCTL, ioctl=%x", p->dioc_IOCtlCode);
			return ERROR_INVALID_FUNCTION;
	}
}


VOID __cdecl FSDEXT2_Aer(AEP* pAep)
{
}

VOID OnDeviceRebootNotify()
{
		/*
		 * When we are here, we are not allowed to touch pageable
		 * code/data. Switch to console debugging as debugging to file
		 * uses a pageable buffer.
		 */
	VxdDebugPrint(D_ALWAYS, "OnDeviceRebootNotify: about to restart, switching to console logging");
	VxdDebugSetOut(DOUT_CONSOLE, 0);
}
