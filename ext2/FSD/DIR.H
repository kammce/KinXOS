#ifndef __DIR_H
#define __DIR_H

#include "ext2fs.h"
#include "inode.h"

/*
 * This is the find structure used in FindFirst/FindNext
 * (on LFN names) and SearchFile (on 8.3 names)
 * calls
 */
typedef struct _FindContext
{
		/*
		 * Invariants, set upon DirCreateFindContext
		 */
	char			SearchName[MAX_PATH];	// search string
	int				SearchFlags;			// search attributes flag
	struct _Inode	*Inode;					// DirInode in which lookup
											// is performed
	ULONG			nrBlocks;				// nr of block for this Inode


		/*
		 * Variants, that is, the "search variables"
		 */
	struct _Block	*Block;					// Current block
	ULONG			LogicalBlockNo;
	ULONG			RecLen;

		/*
		 * "Return values", these have sensible values iff
		 * DirFindEntry returns TRUE
		 * FoundDirEntry points to the data encapsulated by Block,
		 * also, Block is locked. 
		 */
	char			ShortName[13];
	ext2_dir_entry	*FoundDirEntry;
	TInode			*FoundInode;
} TFindContext;



/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/


	/*
	 * Module initialise and cleanup functions
	 */
BOOL	DirInitialise(UINT DirCacheSize);
void	DirCleanup();

	/*
	 * Functions for findcontext manipulations
	 */
void			DirDestroyFindContext(TFindContext *FindContext);
TFindContext*	DirCreateFindContext(TInode *DirInode, char *SearchName, int SearchFlags);
BOOL			DirFindEntry(TFindContext *FindContext, BOOL isShortName);

	/*
	 * Pathname to TInode resolver
	 */
TInode*	DirName2Inode(struct _SuperBlock *Super, char *DirName);

void DirCacheInfo();

#endif		/* __DIR_H */
