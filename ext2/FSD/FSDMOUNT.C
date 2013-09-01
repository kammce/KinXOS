#include "vxd.h"

#include "shared\vxddebug.h"
#include "shared\blkdev.h"

#include "fsdvol.h"
#include "fsdmount.h"

#include "ext2fs.h"
#include "fsdmount.h"
#include "fsdvol.h"
#include "super.h"


/**********************************
 *
 * STATIC DATA
 *
 **********************************/

static volfunc		sFsdVolumeFunctions = 
	{
		IFS_VERSION,
		IFS_REVISION,
		NUM_VOLFUNC,

		FsdVolDelete, 		// VFN_DELETE
		FsdVolDir,			// VFN_DIR
		FsdVolAttrib,		// VFN_FILEATTRIB
		FsdVolFlush,		// VFN_FLUSH (VolFlush: name conflict with IFSMgr service)
		FsdVolInfo,			// VFN_GETDISKINFO
		FsdVolOpen,			// VFN_OPEN
		FsdVolRename,		// VFN_RENAME
		FsdVolSearch,		// VFN_SEARCH
		FsdVolQuery,		// VFN_QUERY
		FsdVolDisconnect,	// VFN_DISCONNECT
		FsdVolUNCPipe,		// VFN_UNCPIPEREQ
		FsdVolIoctl16,		// VFN_IOCTL16DRIVE
		FsdVolParams, 		// VFN_GETDISKPARAMS
		FsdVolFindOpen,		// VFN_FINDOPEN
		FsdVolDasdio		// VFN_DASDIO
	};

/**********************************
 *
 * STATIC FUNCTIONS AND INLINERS
 *
 **********************************/

int _cdecl FsdMount(pioreq Ioreq)
{
	PDCB			dcb;
	TSuperBlock		*Super;

		/*
		 * assume an error
		 */
	Ioreq->ir_error = -1; 

	switch (Ioreq->ir_flags)
	{
		case IR_FSD_MOUNT:
			VxdDebugPrint(D_MOUNT, "FsdMount: flags=%i, drive=%i", Ioreq->ir_flags, Ioreq->ir_mntdrv);

				/*
				 * Let's see if we can read the superblock
				 */
			dcb = IspGetDcb(Ioreq->ir_mntdrv);
			//if ((Super=SuperMountRoot(dcb)))
			if ((Super=SuperMountDeviceStandAlone(dcb)))
			{
				Super->DosDrive = (char) Ioreq->ir_mntdrv;
				Super->Vrp = (PVRP) Ioreq->ir_volh;
				VxdDebugPrint(D_MOUNT, "FsdMount: drive %c succesfully mounted", Ioreq->ir_mntdrv + 'A');
				Ioreq->ir_vfunc = &sFsdVolumeFunctions;	
				Ioreq->ir_error = 0;
				Ioreq->ir_rh = Super;
			}
			else
				VxdDebugPrint(D_MOUNT, "FsdMount: no superblock found");
			break;
		default:
			VxdDebugPrint(D_MOUNT, "FsdMount: other flags=%i, drive=%i", Ioreq->ir_flags, Ioreq->ir_mntdrv);
	}

	VxdDebugFlush();

	return Ioreq->ir_error;
}




