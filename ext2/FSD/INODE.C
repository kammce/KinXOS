#include "vxd.h"

#include "shared\vxddebug.h"
#include "inode.h"
#include "super.h"
#include "ext2fs.h"
#include "unixstat.h"


#define HASH_QUEUE_SIZE 4

/**********************************
 *
 * GLOBAL DATA
 *
 **********************************/

// none


/**********************************
 *
 * STATIC DATA
 *
 **********************************/

		/*
		 */
static MUTEXHANDLE		sMutex = 0;
static TInode			**sInodeTable = 0;
static UINT				sNrsInodeTable;


/**********************************************
 *
 * STATIC HELPERS
 *
 * For all static function the following PRE CONDITION
 * holds: sMutex is entered, that is, it is safe to
 * alter the static variables (in particular sInodeTable)
 *
 *********************************************/

static ULONG UnixToWinAttrib(ext2_inode *Ext2Inode, char *FileName, int NameLen)
{
	ULONG attrib;

	attrib = 0;
	if (S_ISDIR(Ext2Inode->i_mode))
		attrib |= FILE_ATTRIBUTE_DIRECTORY;
//	if (Ext2Inode->i_mode & S_IRUSR)
//		attrib |= FILE_ATTRIBUTE_READONLY;

		/*
		 * We have a "hidden" entry, if the name starts with
		 * a dot and it's not . or ..
		 */
	if (FileName)
	{
		if (FileName[0] == '.')
		{
			if ((NameLen > 1) && (FileName[1] != '.'))
				attrib |= FILE_ATTRIBUTE_HIDDEN;
		}
	}
	return attrib;
}

static __inline UINT HashFunction(ULONG DeviceId, ULONG InodeNo)
{
	return (UINT) ((DeviceId ^ InodeNo) % sNrsInodeTable);
}




/*
 * If present, get an Inode from the InodeTable.
 * 
 * PRE:
 * <none>
 *
 * POST:
 * <none>
 *  
 * IN:
 *	Super		: Superblock of the file system the inode resides in
 *	InodeNo		: Inode nr of the Inode
 *
 * OUT
 *	<none>
 *
 * RETURNS
 *	pointer to TInode if found, otherwise 0.
 */
static TInode* InodeGetFromTable(TSuperBlock *Super, ULONG InodeNo)
{
	UINT		hash;
	TInode		*inode, *hashfirst;
	
	hash = HashFunction((ULONG) Super->Device, InodeNo);

	VxdDebugPrint(D_INODE, "InodeGetFromTable: dev=%s inono=%lu, hash=%u", 
		Super->DeviceName,
		InodeNo,
		hash);

	inode = hashfirst = sInodeTable[hash];
	if(!inode)
		return 0;

		/*
		 * loop over the (circular) list to see if it's
		 * there
		 */
	do
	{
		if (inode->Super->Device == Super->Device && inode->InodeNo == InodeNo)
			goto getfromtable_done;
	} while ( (inode = inode->InodeNext) != hashfirst);
	inode = 0;

getfromtable_done:
	VxdDebugPrint(D_INODE, "InodeGetFromTable: inode %s", inode ? "found":"not found");
	
	return inode;
}


/*
 * Create an TInode structure
 * 
 * PRE:
 * The inode is not yet present in the InodeTable, that is,
 * InodeGetFromTable returned 0.
 *
 * POST:
 * New TInode structure is created and (if succesfull) is chained 
 * in InodeTable. TInode fields initialised:
 *	inode->nrClients = 0
 *	inode->InodePrev and InodeNext --> used to chain the inode in Table
 *	inode->Super = Super
 *	inode=>ParentInode = ParentInode
 *  
 * IN:
 *	Super		: Superblock of the file system the inode resides in
 *	ParentInode	: pointer to TInode of parent inode (may be zero)
 *	InodeNo		: Inode nr of the Inode
 *
 * OUT
 *	<none>
 *
 * RETURNS
 *	pointer to TInode if created, otherwise 0.
 */
static TInode* InodeCreate(TSuperBlock *Super, TInode *ParentInode, ULONG InodeNo)
{
	UINT		hash;
	TInode		*inode, *hashfirst;

	hash = HashFunction((ULONG) Super->Device, InodeNo);

	VxdDebugPrint(D_INODE, "InodeCreate: dev=%s, inono=%lu, hash=%u", 
		Super->DeviceName,
		InodeNo,
		hash);

	inode = (TInode *) calloc(1, sizeof(TInode));
	if (!inode)
	{
		VxdDebugPrint(D_ERROR, "InodeCreate: could not alloc TInode");
		goto inocreate_err_1;
	}

		/*
		 * Set fields
		 */
	inode->ParentInode = ParentInode;
	inode->InodeNo = InodeNo;
	inode->Super = Super;
	inode->nrClients = 0;

		/*
		 * Link inode in InodeTable, put it first in list.
		 * Special treatment for empty linked list
		 */
	if ((hashfirst = sInodeTable[hash]))
	{
		hashfirst->InodePrev->InodeNext = inode;
		inode->InodePrev = hashfirst->InodePrev;
		inode->InodeNext = hashfirst;
		hashfirst->InodePrev = inode;
		sInodeTable[hash] = inode;
	}
	else
	{
		inode->InodePrev = inode->InodeNext = inode;
		sInodeTable[hash] = inode;
	}
	
inocreate_err_1:
	VxdDebugPrint(D_INODE, "InodeCreate: done");
	
	return inode;
}



/*
 * Remove a TInode object.
 *
 * PRE_CONDITION
 *	Inode is fully initialised
 *	Inode->nrClient == 0, that is, there are no more
 *	clients holding the inode
 */
static void InodeDestroy(TInode *Inode)
{
	UINT		hash;

	hash = HashFunction((ULONG) Inode->Super->Device, Inode->InodeNo);

	VxdDebugPrint(D_INODE, "InodeDestroy: dev=%s, inono=%lu, hash=%u", 
		Inode->Super->DeviceName,
		Inode->InodeNo,
		hash);

		/*
		 * Remove inode from HashTable.
		 * Special treatment if we're last item in list
		 */
	if (Inode == Inode->InodeNext)
	{
		sInodeTable[hash] = 0;
	}
	else
	{
			/*
			 * If we're head of the chain, let our
			 * neighbour be the head of the chain
			 */
		if (sInodeTable[hash] == Inode)
			sInodeTable[hash] = Inode->InodeNext;

			/*
			 * Remove us from the list
			 */
		Inode->InodePrev->InodeNext = Inode->InodeNext;
		Inode->InodeNext->InodePrev = Inode->InodePrev;
	}
		
		/*
		 * Get rid of the TInode itself
		 */
	free(Inode);

	VxdDebugPrint(D_INODE, "InodeDestroy: done");
}

/*
 * Generic function to deal with the creating/lookup of inodes.
 * 
 */
static TInode* doInodeGetInode(TSuperBlock *Super, TInode *ParentInode, ULONG InodeNo, char *FileName, int NameLen)
{
	TInode		*inode;
	ext2_inode	*ext2inode;

	inode = 0;		// assume error

	VxdDebugPrint(D_INODE, "doInodeGetInode: dev=%s, inono=%lu", 
		Super->DeviceName,
		InodeNo);

		/*
		 * Sometimes directories contain entries with InodeNo=0
		 * Don't consider this an error, just return silently
		 */
	if (!InodeNo)
		return 0;

		/*
		 * First, see if it's in the the Table, if not
		 * create a TInode and get it from disk (actually the block
		 * my be in the cache)
		 */
	if (!(inode = InodeGetFromTable(Super, InodeNo)))
	{
			/*
			 * Not present, so first create a TInode object
			 */
		if (!(inode = InodeCreate(Super, ParentInode, InodeNo)))
		{
			VxdDebugPrint(D_ERROR, "doInodeGetInode: could not create a TInode");
			goto doget_inode_done;
		}

			/*
			 * Get it from the file system
			 */
		if (!Ext2ReadInode(inode))
		{
			VxdDebugPrint(D_ERROR, "doInodeGetInode: could not readinode from disk");

			InodeDestroy(inode);
			inode = (TInode *) 0;
			goto doget_inode_done;
		}

		ext2inode = InodeLock(inode);
		inode->DosAttrib = UnixToWinAttrib(ext2inode, FileName, NameLen);
		inode->Mode = ext2inode->i_mode;
		InodeUnlock(inode);
	}

	inode->nrClients++;		// increase reference count for this inode

doget_inode_done:
	VxdDebugPrint(D_INODE, "doInodeGetInode: done (attrib=%i)", inode ? inode->DosAttrib : -1);

	return inode;
}





/**************************************
 *
 * INTERFACE ROUNTINES
 *
 **************************************/

/*
 * Get an inode (but not the root inode) relative to a directory
 * inode.
 *
 * PRE:
 * Since this function is used to get all _but_ the rootinode, the
 * root inode of a device must succesfully have been read using 
 * InodeGetRootInode.
 *
 * POST:
 * If the inode is in the InodeTable, a reference is returned, that is,
 * the nrClients is increased by one. If it's not present, a new
 * inode structure is created, chanined in the table and nrClients is
 * set to 1, next the inode is read from disk.
 *
 *
 * IN:
 *	ParentInode	: pointer to inode structure of the parent inode
 *					(this may _not_ be zero)
 *	InodeNo		: Inode nr of the Inode
 *
 * OUT:
 *	<none>
 *
 * RETURNS:
 *	pointer to TInode if succesfull, otherwise 0
 *
 */
TInode* InodeGetInode(TInode *ParentInode, ULONG InodeNo, char *FileName, int NameLen)
{
	TInode		*Inode;

		/*
		 * Basically, we are a wrapper for doInodeGetInode
		 * Our Inode resides on the same filesystem (identified by
		 * ParentInode->Super) as our parent inode
		 */
	if (!ParentInode)
	{
		VxdDebugPrint(D_ERROR, "InodeGetInode: ParentInode==0");
		return 0;
	}

	EnterMutex(sMutex, BLOCK_THREAD_IDLE);

		/*
		 * First get the inode
		 */
	Inode = doInodeGetInode(ParentInode->Super, ParentInode, InodeNo, FileName, NameLen);


	VxdDebugPrint(D_INODE, "InodeGetInode: done, Inode=%X", Inode);

	LeaveMutex(sMutex);

	return Inode;
}

TBlock* InodeGetBlock(TInode *Inode, ULONG LogicalBlockNo)
{
	TBlock		*block;
	ULONG		blockno;

	VxdDebugPrint(D_INODE, "InodeGetBlock: inode=(dev=%s, inono=%lu), lblock=%lu",
			Inode->Super->DeviceName,
			Inode->InodeNo,
			LogicalBlockNo);

	if (!(blockno = Ext2BlockMap(Inode, LogicalBlockNo)))
	{
		VxdDebugPrint(D_ERROR, "InodeGetBlock: done, could not Ext2BlockMap");
		return 0;
	}

	if (!(block = BlockGetBlock(Inode->Super->Device, blockno)))
	{
		VxdDebugPrint(D_ERROR, "InodeGetBlock: done, could not get block(dev=%s, blockno=%lu)",
				Inode->Super->DeviceName,
				blockno);
		return 0;
	}

	VxdDebugPrint(D_INODE, "InodeGetBlock: done, blockno=%lu", blockno);
	
	return block;
}



/*
 * Get the root inode
 *
 * PRE:
 * <none>
 *
 * POST:
 *
 *
 * IN:
 *	ParentInode	: pointer to inode structure of the parent inode
 *					(this may _not_ be zero)
 *	InodeNo		: Inode nr of the Inode
 *
 * OUT:
 *	<none>
 *
 * RETURNS:
 *	pointer to TInode if succesfull, otherwise 0
 *
 */
TInode* InodeGetRootInode(TSuperBlock *Super)
{
	TInode	*inode;

		/*
		 * Basically, we are a wrapper for doInodeGetInode
		 * We need the root inode of the file system identified
		 * by Super. Since we are the root inode, we have no parent
		 * inode
		 */
	EnterMutex(sMutex, BLOCK_THREAD_IDLE);

	inode = doInodeGetInode(Super, 0, EXT2_ROOT_INO, "", 0);

	LeaveMutex(sMutex);

	return inode;
}


void InodeRelease(TInode *Inode)
{
	EnterMutex(sMutex, BLOCK_THREAD_IDLE);

	VxdDebugPrint(D_INODE, "InodeReleaseInode: dev=%s, inono=%lu, inode=%X", 
		Inode->Super->DeviceName,
		Inode->InodeNo,
		Inode);

		/*
		 * Inodes are reference counted, only destroy when
		 * we have no more clients referencing it.
		 */
	if (--(Inode->nrClients) == 0)
	{
			/*
			 * Remove the TInode object from the InodeTable
			 */
		BlockRelease(Inode->Block);
		InodeDestroy(Inode);
	}
	
	VxdDebugPrint(D_INODE, "InoReleaseInode: done");
	
	LeaveMutex(sMutex);
}



void* InodeLock(TInode *Inode)
{
	return Inode->Ext2Inode;
}


void InodeUnlock(TInode *Inode)
{
}


BOOL InodeInitialise(UINT EstimatedInodesResident)
{
	sNrsInodeTable = EstimatedInodesResident / HASH_QUEUE_SIZE;

	VxdDebugPrint(D_SYSTEM, "InoInitialise: resident=%u", EstimatedInodesResident);

		/*
		 * Create sInodeTable
		 */
	if (!(sInodeTable = (TInode **) calloc(sNrsInodeTable, sizeof(TInode *))))
	{
		VxdDebugPrint(D_ERROR, "InoInitialise: could not alloc sInodeTable");
		goto ino_init_1;
	}

	sMutex = CreateMutex(0, MUTEX_MUST_COMPLETE);

	VxdDebugPrint(D_SYSTEM, "InoInitialise: done");

	return TRUE;

ino_init_1:

	VxdDebugPrint(D_SYSTEM, "InoInitialise: done");

	return FALSE;
}


void InodeCleanup()
{
	VxdDebugPrint(D_SYSTEM, "InoCleanUp");
	
	EnterMutex(sMutex, BLOCK_THREAD_IDLE);

	free(sInodeTable);

	LeaveMutex(sMutex);

	DestroyMutex(sMutex);

	VxdDebugPrint(D_SYSTEM, "InoCleanUp: done");
}


TInode* InodeCloneInode(TInode *Inode)
{
	Inode->nrClients++;
	return Inode;
}














