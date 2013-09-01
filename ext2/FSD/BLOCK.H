#ifndef __BLOCK_H
#define __BLOCK_H

#include "shared\blkdev.h"

typedef struct _Block
{
		/*
		 * Synchronised access by _Block.Mutex
		 */
	void			*BlockData;
	BOOL			isDirty;
	MUTEXHANDLE		Mutex;

		/*
		 * Synchronised access by the module-global
		 * mutex
		 */
	unsigned		nrClients;
	struct _Block	*BlockNext;
	struct _Block	*BlockPrev;

		/*
		 * No synchronised access needed (read-only)
		 */
	ULONG			BlockNo;
	TDevice			Device;
	//TInode			*Inode;
} TBlock;


/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/


BOOL	BlockInitialise(UINT EstimatedBlocksResident, UINT BlockCacheSize);
void	BlockCleanup();

TBlock*	BlockGetBlock(TDevice Device, ULONG StartBlock);
void	BlockRelease(TBlock *Block);

void*	BlockLock(TBlock *Block);
void	BlockUnlock(TBlock *Block);

void BlockCacheInfo();


#endif			/* __BLOCK_H */