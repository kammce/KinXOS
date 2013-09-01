#include "vxd.h"

#include "shared\parttype.h"
#include "shared\vxddebug.h"
#include "super.h"
#include "inode.h"
#include "parttbl.h"


/**********************************
 *
 * STATIC DATA
 *
 **********************************/

static MUTEXHANDLE		sMutex = 0;
static TSuperBlock		sSuperRoot;

/**********************************
 *
 * STATIC HELPERS
 *
 **********************************/

static void SetDeviceName(TSuperBlock *Super)
{
	int	Disk, Partition;

	PartitionGetDevice(Super->Device, &Disk, &Partition);

	sprintf(Super->DeviceName, "/dev/%cd%c%i", 
		Disk < MAX_IDE_DISK ? 'h' : 's',
		Disk < MAX_IDE_DISK ? Disk + 'a' : Disk + 'a' - MAX_IDE_DISK,
		Partition);
}

/*
 * Mounts a device on an inode
 * This resembles the mount command:
 *	mount /dev/hda1 /mnt -t ext2
 * 
 * PRE CONDITIONS:
 * sMutex is entered
 * No fields in the Super needs to be initialised. Device
 * refers to a existing device. InodeMount may either be zero
 * (to denote a mount of the root partition) or a pointer to a
 * inode structure (to denote the mount point).
 *
 * POST CONDITIONS:
 * If the mount succeeded, Super is initialised as described
 * below.
 *
 * IN:
 *	Super		: Pointer to a Super structure. No fields need to
 *					be initialised
 *	Device		: Valid reference to a device
 *	InodeMount	: 0 to denote a mount of the root partition, otherwise
 *					a pointer to the inode of the mount point
 *	
 * 
 * OUT:
 *	Super		: _All_ fields have sensible values.
 *
 * RETURNS:
 *	TRUE on succes, FALSE otherwise
 *
 */
BOOL SuperMount(TSuperBlock *Super, TDevice Device, TInode *InodeMount)
{

	DevGenerateName(Device, Super->DeviceName);

	VxdDebugPrint(D_SUPER, "SuperMount: device=%s, inode=(device=%s, ino=%lu)", 
		(ULONG) Super->DeviceName,
		InodeMount ? InodeMount->Super->DeviceName: "dev/root", 
		InodeMount ? (ULONG) InodeMount->InodeNo: 0ul);

		/*
		 * Make sure we are on a hardisk device. We can tell
		 * by the partition type. It was put there by our VSD
		 */
	if (DevGetPartitionType(Device) != TYPE_LINUX_PARTITION)
	{
		VxdDebugPrint(D_ERROR, "SuperMount: device not a hardisk, device");
		goto mnt_dev_err_1;
	}

		/*
		 * Set these fields now, the Ext2 module depends
		 * on Super->Device and Super->Mutex being initialised
		 */
	Super->Device = Device;
	Super->Mutex = CreateMutex(0, MUTEX_MUST_COMPLETE);
		
		/*
		 * Can we read an ext2 superblock on it?
		 */
	if (!Ext2GetSuperBlock(Super))
	{
		VxdDebugPrint(D_ERROR, "SuperMount: failed to read superblock");
		goto mnt_dev_err_2;
	}

		/*
		 * Get the root inode for this file system
		 */
	if (!(Super->RootInode = InodeGetRootInode(Super)))
	{
		VxdDebugPrint(D_ERROR, "SuperMount: failed to read inode");
		goto mnt_dev_err_3;
	}

	SetDeviceName(Super);

	VxdDebugPrint(D_SUPER, "SuperMount: done");

	
	return TRUE;

mnt_dev_err_3:
	Ext2ReleaseSuperBlock(Super);
mnt_dev_err_2:
	DestroyMutex(Super->Mutex);



mnt_dev_err_1:

		/*
		 * If we fail, reset the Super->Device. It is used
		 * by mount root to detect if the root partition
		 * is already mounted.
		 */
	Super->Device = 0;

	return FALSE;
}



/**********************************
 *
 * INTERFACE ROUTINES
 *
 **********************************/



TSuperBlock* SuperMountRoot(TDevice Device)
{
	TSuperBlock*		Super;


	EnterMutex(sMutex, BLOCK_THREAD_IDLE);

	VxdDebugPrint(D_SUPER, "SuperMountRoot: device=%x", (ULONG) Device);

	Super = 0;

		/*
		 * First, see if a root is already loaded
		 */ 
	if (sSuperRoot.Device)
	{
		VxdDebugPrint(D_ERROR, "SuperMountRoot: root already loaded");
		goto mnt_root_done;
	}

		/*
		 * Mount the volume, mountpoint = 0
		 */
	if (!(SuperMount(&sSuperRoot, Device, 0)))
	{
		VxdDebugPrint(D_ERROR, "SuperMountRoot: could not mount volume");
		goto mnt_root_done;
	}

	VxdDebugPrint(D_SUPER, "SuperMountRoot: done");


	Super = &sSuperRoot;

mnt_root_done:
	LeaveMutex(sMutex);

	VxdDebugPrint(D_SUPER, "SuperMountRoot: done");
	
	return Super;
}



/*
 * Mounts a device on a directory
 * This resembles the mount command:
 *	mount /dev/hda1 /mnt -t ext2
 * 
 * PRE CONDITIONS:
 * No fields in the Super needs to be initialised. Device
 * refers to a existing device. InodeMount may either be zero
 * (to denote a mount of the root partition) or a pointer to a
 * inode structure (to denote the mount point).
 *
 * POST CONDITIONS:
 * If the mount succeeded, Super is initialised as described
 * below.
 *
 * IN:
 *	Super	: Pointer to a SuperBlock structure. No fields need to
 *					be initialised
 *	Device		: Valid reference to a device
 *	InodeMount	: 0 to denote a mount of the root partition, otherwise
 *					a pointer to the inode of the mount point
 *	
 * 
 * OUT:
 *	Super	: _All_ fields have sensible values.
 *
 * RETURNS:
 *	TRUE on succes, FALSE otherwise
 *
 */

TSuperBlock* SuperMountDevice(TDevice Device, TInode *InodeRoot)
{
	return FALSE;
}



BOOL SuperUmount(TDevice Device)
{
	return TRUE;
}



BOOL SuperInitialise()
{

	VxdDebugPrint(D_SYSTEM, "SuperInitialise");

		/*
		 * clean our global
		 */
	memset(&sSuperRoot, 0, sizeof(TSuperBlock));
	
	sMutex = CreateMutex(0, MUTEX_MUST_COMPLETE);

	VxdDebugPrint(D_SYSTEM, "SuperInitialise: done");

	return TRUE;
}



void SuperCleanup()
{

	VxdDebugPrint(D_SYSTEM, "SuperCleanup");

	DestroyMutex(sMutex);

	VxdDebugPrint(D_SYSTEM, "SuperCleanup: done");
}


ext2_super_block* SuperLock(TSuperBlock *Super)
{
	return BlockLock(Super->SuperBlock);
}


void SuperUnlock(TSuperBlock *Super)
{
	BlockUnlock(Super->SuperBlock);
}


/*
 * This is for the future
 */
//TSuperBlock* SuperGetSuperRoot()
//{
//	return &sSuperRoot;
//}


TSuperBlock* SuperMountDeviceStandAlone(TDevice Device)
{
	TSuperBlock*		Super;

	VxdDebugPrint(D_SUPER, "SuperMountDeviceStandAlone: device=%x", (ULONG) Device);

	if (!(Super = (TSuperBlock *) calloc(1, sizeof(TSuperBlock))))
	{
		VxdDebugPrint(D_ERROR, "SuperMountDeviceStandAlone: could not malloc super");
		return 0;
	}

		/*
		 * Mount the volume, mountpoint = 0
		 */
	if (!(SuperMount(Super, Device, 0)))
	{
		VxdDebugPrint(D_ERROR, "SuperMountDeviceStandAlone: could not mount volume");
		return 0;
	}

	VxdDebugPrint(D_SUPER, "SuperMountDeviceStandAlone: done");

	return Super;
}

