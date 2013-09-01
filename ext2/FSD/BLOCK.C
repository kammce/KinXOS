#include "vxd.h"

#include "shared\vxddebug.h"
#include "shared\blkdev.h"
#include "cache.h"
#include "block.h"


/*
 * This module administrates all blocks loaded in memory.
 * For each block a TBlock is created; these are looked up
 * via the BlockTable. This is a hashtable of TBlock pointers.
 * Each pointer is a double-linked list of TBlock's with the 
 * same hashvalue.
 *
 * We also maintain a Block cache. When a block is removed
 * from the BlockTable, we add the _pointer_ to the block's
 * data to the cache (no copies are made!).
 *
 * Whenever a client wants a block, we first look if it's already
 * loaded (via the BlockTable). If so, we increase nrClients to 
 * reference count it. If the block is not in the table, we create 
 * a new TBlock structure and add to the BlockTable. Then we get 
 * the block from either the cache or from disk.
 * When getting it from the cache, the cache also removes the entry.
 */
 

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
		 * Mutex is needed for synchronised access to the
		 * sBlockTable and to some fields of a TBlock
		 * object.
		 *
		 * Each TBlock object has a Mutex of it's own.
		 * That Mutex is used to lock/unlock a block
		 */
static MUTEXHANDLE		sMutex = 0;
static TBlock			**sBlockTable = 0;
static UINT				sNrsBlockTable;
static TCache			*sBlockCache;
static char				sBlockCacheName[] = "BlockCache";



/**********************************************
 *
 * STATIC HELPERS
 *
 * For all static function the following PRE CONDITION
 * holds: sMutex is entered, that is, it is safe to
 * alter the sInodeTable
 *
 *********************************************/

static __inline UINT HashFunction(TDevice Device, ULONG BlockNo)
{
	return (UINT) ((((ULONG) Device) ^ BlockNo) % sNrsBlockTable);
}


static TBlock* BlockGetFromTable(TDevice Device, ULONG BlockNo, UINT Hash)
{
	TBlock	*Block, *HashFirst;
	
	VxdDebugPrint(D_BLOCK, "BlockGetFromTable: device=%x, block=%lu, hash=%u", (ULONG) Device, BlockNo, Hash);

	Block = HashFirst = sBlockTable[Hash];
	if(!Block)
	{
		VxdDebugPrint(D_BLOCK, "BlockGetFromTable: done, hash chain empty");
		return 0;
	}

		/*
		 * loop over the (circular) list to see if it's
		 * there
		 */
	do
	{
		if (Block->Device == Device && Block->BlockNo == BlockNo)
			goto getfromtable_done;
	} while ( (Block = Block->BlockNext) != HashFirst);
	Block = 0;

getfromtable_done:
	VxdDebugPrint(D_BLOCK, "BlockGetFromTable: done, block (%s)", Block?"found":"not found");
	
	return Block;
}



static TBlock* BlockCreate(TDevice Device, ULONG BlockNo, ULONG Hash)
{
	TBlock		*Block, *HashFirst;

	VxdDebugPrint(D_BLOCK, "BlockCreate: device=%x, block=%lu", (ULONG) Device, BlockNo);

	Block = (TBlock *) calloc(1, sizeof(TBlock));
	if (!Block)
	{
		VxdDebugPrint(D_ERROR, "BlockCreate: could not alloc TBlock");
		goto blkcreate_err_1;
	}

	Block->Device = Device;
	Block->BlockNo = BlockNo;
	Block->nrClients = 0;

		/*
		 * Link block in BlockTable, put it first in list.
		 * Special treatment for empty linked list
		 */
	if ((HashFirst = sBlockTable[Hash]))
	{
		HashFirst->BlockPrev->BlockNext = Block;
		Block->BlockPrev = HashFirst->BlockPrev;
		Block->BlockNext = HashFirst;
		HashFirst->BlockPrev = Block;
		sBlockTable[Hash] = Block;
	}
	else
	{
		Block->BlockPrev = Block->BlockNext = Block;
		sBlockTable[Hash] = Block;
	}
	
blkcreate_err_1:
	VxdDebugPrint(D_BLOCK, "BlockCreate: done");
	
	return Block;
}



/*
 * Remove a TBlock object.
 *
 * PRE_CONDITION
 *	Block->nrClient == 0, that is, there are no more
 *	clients holding the block
 */
static void BlockDestroy(TBlock *Block)
{
	UINT		Hash;

	VxdDebugPrint(D_BLOCK, "BlockDestroy: device=%x, block=%lu", (ULONG) Block->Device, Block->BlockNo);

		/*
		 * Remove block from HashTable.
		 * Special treatment if we're last item in list
		 */
	Hash = HashFunction(Block->Device, Block->BlockNo);
	if (Block == Block->BlockNext)
	{
		sBlockTable[Hash] = 0;
	}
	else
	{
			/*
			 * If we're head of the chain, let our
			 * neighbour be the head of the chain
			 */
		if (sBlockTable[Hash] == Block)
			sBlockTable[Hash] = Block->BlockNext;

			/*
			 * Remove us from the list
			 */
		Block->BlockPrev->BlockNext = Block->BlockNext;
		Block->BlockNext->BlockPrev = Block->BlockPrev;
	}
		
		/*
		 * Get rid of the TBlock itself
		 */
	free(Block);

	VxdDebugPrint(D_BLOCK, "BlockDestroy: done");
}





/**************************************
 *
 * INTERFACE ROUNTINES
 *
 **************************************/

/*
 * Get a block identified by its number from a device 
 *
 * PRE CONDITIONS:
 *	Device is a valid reference to a  blockdevice
 *
 * POST CONDITIONS:
 *	If the block is in the blocktable it can be returned
 *	immediately and its reference counted is increased by one.
 *	If not in the blocktable, the block is looked up in the block 
 *	cache, if it in the block cache it is read from disk.
 *	In the last two cases a new TBlock object is created,
 *	initialised with reference count=1 and added to the BlockTable.
 *	If the block is read from disk, it is _not_ added to the 
 *	cache.
 *	
 * IN:
 *	Device:		Valid refenece to a block device
 *	BlockNo:	logical block number
 *
 * OUT:
 *	<none>
 *
 * RETURNS:
 *	success: valid pointer to TBlock object
 *	otherwise: 0
 */
TBlock* BlockGetBlock(TDevice Device, ULONG BlockNo)
{
	TBlock		*Block;
	UINT		Hash;

	EnterMutex(sMutex, BLOCK_THREAD_IDLE);

	VxdDebugPrint(D_BLOCK, "BlockGetBlock: device=%x, block=%lu", (ULONG) Device, BlockNo);

		/*
		 * First, see if it's in the the Table, if not
		 * create a TBlock and get it from either the cache
		 * or from disk
		 */
	Hash = HashFunction(Device, BlockNo);
	if (!(Block = BlockGetFromTable(Device, BlockNo, Hash)))
	{
			/*
			 * First, create a TBlock object
			 */
		if (!(Block = BlockCreate(Device, BlockNo, Hash)))
		{
			VxdDebugPrint(D_ERROR, "BlockGetBlock: could not create a TBlock, device=%lu, block=%lu", (ULONG) Device, BlockNo);
			goto getblock_done;
		}

			/*
			 * Is it in the block cache ?
			 */
		if (CacheLookupById(sBlockCache, (ULONG) Device, BlockNo, &Block->BlockData))
		{
				/*
				 * Yep, blockdata now points to the buffer
				 */
			;
		}
		else
		{

			if (!(Block->BlockData = malloc(DevGetBlockSize(Device))))
			{
				VxdDebugPrint(D_ERROR, "BlockGetBlock: could not malloc blockdata, device=%lu, block=%lu", (ULONG) Device, BlockNo);
				goto getblock_done;
			}

				/*
				 * Get it from disk
				 */
			//LeaveMutex(sMutex);
			
			if (!DevReadBlock(Device, BlockNo, Block->BlockData))
			{
				VxdDebugPrint(D_ERROR, "BlockGetBlock: could not readblock from disk, device=%lu, block=%lu", (ULONG) Device, BlockNo);

				free(Block->BlockData);
			
				BlockDestroy(Block);
				Block = (TBlock *) 0;
				goto getblock_done;
			}
		}

	}

	Block->nrClients++;		// increase reference count for this block

getblock_done:
	VxdDebugPrint(D_BLOCK, "BlockGetBlock: done, Block=%X", Block);
	LeaveMutex(sMutex);

	return Block;
}


/*
 * Release a TBlock object
 *
 * PRE CONDITIONS:
 *	Block is a valid TBlock object
 *
 * POST CONDITIONS:
 *	First, the the TBlock's object reference count is decreased
 *	by one. If the reference count reaches 0, the Tblock is
 *	removed from the BlockTable and the actual block is added
 *	to the cache.
 *	
 * IN:
 *	Block:	valid reference to a TBlock object
 *
 * OUT:
 *	<none>
 *
 * RETURNS:
 *	<none>
 */
void BlockRelease(TBlock *Block)
{
	EnterMutex(sMutex, BLOCK_THREAD_IDLE);

	VxdDebugPrint(D_BLOCK, "BlockReleaseBlock: device=%x, block=%lu, block=%X", 
		(ULONG) Block->Device, 
		Block->BlockNo,
		Block);

	if (--(Block->nrClients) == 0)
	{
			/*
			 * Don't free the actual diskblock but add it to the cache.
			 * Also note that we don't actually copy the block but rahther
			 * the pointer to the block.
			 */
		CacheAddById(sBlockCache, (ULONG) Block->Device, Block->BlockNo, Block->BlockData);

			/*
			 * Remove the TBlock object from the BlockTable
			 */
		BlockDestroy(Block);
	}
	
	VxdDebugPrint(D_BLOCK, "BlockReleaseBlock: done");
	
	LeaveMutex(sMutex);
}


/*
 * Locks a TBlock object
 */
VOID* BlockLock(TBlock *Block)
{
		/*
		 * In the future we should use a Mutex to lock
		 * the block
		 */
	return Block->BlockData;
}

/*
 * Unlocks a TBlock object
 */
void BlockUnlock(TBlock *Block)
{
}


/*
 * Initialises the block module
 *
 * PRE CONDITIONS:
 *	<none>
 *
 * POST CONDITIONS:
 *	The block table and the block cache are created and
 *	initialised.
 *
 * IN:
 *	EstimatedBlocksResident:	Estimate number of blocks that should
 *								be resident at any given moment.
 *	BlockCacheSize:				Number of objects that should fit
 *								in the cache
 */
 BOOL BlockInitialise(UINT EstimatedBlocksResident, UINT BlockCacheSize)
{
	sNrsBlockTable = EstimatedBlocksResident / HASH_QUEUE_SIZE;

	VxdDebugPrint(D_SYSTEM, "BlockInitialise: resident=%u, cache=%u", EstimatedBlocksResident, BlockCacheSize);

		/*
		 * Create sBlockTable
		 */
	if (!(sBlockTable = (TBlock **) calloc(sNrsBlockTable, sizeof(TBlock *))))
	{
		VxdDebugPrint(D_ERROR, "BlockInitialse: could not alloc sBlockTable");
		goto blk_init_1;
	}

		/*
		 * Create a diskblock cache, this means the cache only 
		 * stores pointers and we are responsible for the block's 
		 * memory space.
		 */
	if (! (sBlockCache = CacheCreate(sBlockCacheName, BlockCacheSize, 0)))
	{
		VxdDebugPrint(D_ERROR, "BlockInitialse: could not create block cache");
		goto blk_init_2;
	}

	sMutex = CreateMutex(0, MUTEX_MUST_COMPLETE);

	VxdDebugPrint(D_SYSTEM, "BlockInitialise: done");

	return TRUE;

blk_init_2:
	free(sBlockTable);

blk_init_1:

	VxdDebugPrint(D_SYSTEM, "BlockInitialise: done");

	return FALSE;
}


void BlockCleanup()
{
	VxdDebugPrint(D_SYSTEM, "BlockCleanUp");
	
	CacheDestroy(sBlockCache);

	EnterMutex(sMutex, BLOCK_THREAD_IDLE);

	free(sBlockTable);

	LeaveMutex(sMutex);

	DestroyMutex(sMutex);

	VxdDebugPrint(D_SYSTEM, "BlockCleanUp: done");
}

void BlockCacheInfo()
{
	unsigned long	Lookups;
	unsigned long	Hits;

	CacheGetStats(sBlockCache, &Lookups, &Hits);
	VxdDebugPrint(D_SYSTEM, "BlockCacheInfo: lookup=%lu, hit=%lu, ratio=%lu",
			Lookups, Hits, 
			Lookups? Hits*100/ Lookups: 0);
}
