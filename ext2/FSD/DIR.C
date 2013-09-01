#include "vxd.h"

#include "dir.h"
#include "cache.h"
#include "super.h"
#include "inode.h"
#include "block.h"
#include "util.h"
#include "unixstat.h"
#include "shared\vxddebug.h"

	/*
	 * First time I ever wrote two functions with 
	 * cross-dependency :-)
	 */
static TInode* FollowLink(TInode *DirInode, TInode *LinkInode);
static TInode* doDirName2Inode(TInode *DirInode, char *DirName, int DirNameLength);



#define MUSTMATCH(ir_attr,attr)		((((ir_attr & (attr)<<8)	\
										^ ir_attr)				\
										& FILE_ATTRIBUTE_MUSTMATCH) == 0)



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

static TCache			*sDirCache;
static char				sDirCacheName[] = "DirCache";



/********************************
 *
 * STATIC HELPERS
 *
 *********************************/

static BOOL DirIsEntryNameMatch(TFindContext *FindContext, BOOL isShortName)
{
	int			ShortNameLen;
	BOOL		Succes;

	if (isShortName)
	{
			/*
			 * Search on short Filenames
			 */
		LongToShort(FindContext->FoundDirEntry->name, FindContext->FoundDirEntry->name_len, FindContext->ShortName, FindContext->FoundDirEntry->inode);
		VxdDebugPrint(D_DIR, "shortfilename=%s", FindContext->ShortName);
		ShortNameLen = strlen(FindContext->ShortName);
		Succes = isEntryMatch(FindContext->ShortName, FindContext->SearchName, FindContext->SearchFlags, ShortNameLen);

	}
	else
	{
			/*
			 * Search on LFN Filenames
			 */
		Succes = isEntryMatch(FindContext->FoundDirEntry->name, FindContext->SearchName, FindContext->SearchFlags, FindContext->FoundDirEntry->name_len);
	}

	return Succes;
}


static BOOL DirIsEntryAttribMatch(TFindContext *FindContext, BOOL isShortName)
{
	BOOLEAN			Success;
	ULONG			Attrib;
	BYTE			ExcludeMask;
	Attrib = FindContext->FoundInode->DosAttrib;
		
	if (isShortName)
	{
		if (Attrib)
			Success = (FindContext->SearchFlags & Attrib);
		else
			Success = TRUE;
	}
	else
	{
		ExcludeMask = (~FindContext->SearchFlags) & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_SYSTEM);
		Success = (!(Attrib & ExcludeMask) && MUSTMATCH(FindContext->SearchFlags, Attrib));
	}

	return Success;
}






/*
 * Lookup an directory entry and return the object's inode nr
 *
 * PRE:
 * DirInode is indeed an directory inode. EntryName is zero-terminated
 *
 * POST:
 * <none>
 *
 * IN:
 *	DirInode	: pointer to a fully initialised directory inode 
 *	EntryName	: zero-terminated entry name which is to be looked up
 *
 * OUT:
 *	<none>
 *
 * RETURNS
 *	Succes		: inode number of the entry name
 *	otherwise	: 0
 * 
 */
static ULONG DirLookupEntry(TInode *DirInode, char *EntryName, int NameLen, BOOL isShortName)
{
	char			ShortName[13];
	char			zEntryName[13];
	ULONG			InodeNo, nrBlocks, BlockSize, BlockNo;
	USHORT			RecLen, ShortNameLen;
	TBlock			*Block;
	ext2_dir_entry	*DirEntry;
	ext2_inode		*Ext2Inode;
	char			*DirData;

#ifdef DEBUG
	char			zName[256];

	memcpy(zName, EntryName, NameLen);
	zName[NameLen] = 0;
#endif
	VxdDebugPrint(D_DIR, "DirLookupEntry(%s): inode=(dev=%s, ino=%lu), name=%s", 
			isShortName ? "short" : "lfn",
			DirInode->Super->DeviceName, 
			DirInode->InodeNo,
			zName);

		/*
		 * The EntryName is not zero-terminated
		 */
	if (isShortName)
	{
		memcpy(zEntryName, EntryName, NameLen);
		zEntryName[NameLen] = 0;
	}

	Ext2Inode = InodeLock(DirInode);

		/*
		 * init some locals
		 */
	InodeNo = 0;
	BlockSize = DevGetBlockSize(DirInode->Super->Device);
	nrBlocks = Ext2Inode->i_size / BlockSize;
	
		/*
		 * Sanity check
		 */
	if (nrBlocks * BlockSize != Ext2Inode->i_size)
	{
		VxdDebugPrint(D_ERROR, "DirLookupEntry: i_size=%lu not multiple of BlockSize, done", Ext2Inode->i_size);
		goto lookup_done;
	}


		/*
		 * itterate over directory blocks
		 */
	for (BlockNo=0 ; BlockNo<nrBlocks ; BlockNo++)
	{
		VxdDebugPrint(D_DIR, "DirLookupEntry: looking in block=%lu of blocks=%lu", BlockNo, nrBlocks);

		if (!(Block = InodeGetBlock(DirInode, BlockNo)))
		{
			VxdDebugPrint(D_ERROR, "DirLookupEntry: could not read block %i, continuing on next block", BlockNo);
			continue;
		}
		
			/*
			 * For each block we loop over the contained
			 * directory entries and look for a match.
			 */
		DirData = BlockLock(Block);
		RecLen = 0;

		do
		{
			DirEntry = (ext2_dir_entry *) (DirData + RecLen);

				/*
				 * Add each entry to the cache
				 */
			CacheAddByName(sDirCache, (ULONG) DirInode->Super->Device, DirInode->InodeNo, DirEntry->name, DirEntry->name_len, &DirEntry->inode);

				/*
				 * The name we are looking for is short, so
				 * convert the entryname to 8.3
				 * Then do a case-insensitive search
				 */
			if (isShortName)
			{
				LongToShort(DirEntry->name, DirEntry->name_len, ShortName, DirEntry->inode);
				ShortNameLen = strlen(ShortName);
				if (isEntryMatch(ShortName, zEntryName, 0, ShortNameLen))
				{

						/*
						 * Ok, this is the one we need, break
						 * to the exit code
						 */
					InodeNo = DirEntry->inode;
					BlockUnlock(Block);
					goto lookup_done;
				}
				else
				{
					VxdDebugPrint(D_DIR, "DirLookupEntry(short): name=%s, no match", ShortName);
				}
			}
			else
			{
				if (DirEntry->name_len == NameLen)
				{

						/*
						 * We must have an exact match
						 */
					if (strncmp(DirEntry->name, EntryName, NameLen) == 0)
					{
							/*
							 * Ok, this is the one we need, break
							 * to the exit code
							 */
						InodeNo = DirEntry->inode;
						BlockUnlock(Block);
						goto lookup_done;
					}
					else
					{
#ifdef DEBUG
						memcpy(zName, DirEntry->name, DirEntry->name_len);
						zName[DirEntry->name_len] = 0;
						VxdDebugPrint(D_DIR, "DirLookupEntry(lfn): name=%s, no match", zName);
#endif
					}
				}
				else
				{
#ifdef DEBUG
					memcpy(zName, DirEntry->name, DirEntry->name_len);
					zName[DirEntry->name_len] = 0;
					VxdDebugPrint(D_DIR, "DirLookupEntry(lfn): name=%s, len=%i, no len match", zName, DirEntry->name_len);
#endif
				}
			}
			RecLen += DirEntry->rec_len;
		} while (RecLen < BlockSize);

		BlockUnlock(Block);
	}
	
lookup_done:
	VxdDebugPrint(D_DIR, "DirLookupEntry done, ino=%lu", InodeNo);
	InodeUnlock(DirInode);

	return InodeNo;
}


static ULONG DirName2InodeNo(TInode *Inode, char *PathName, int PathLen, BOOL isShortName)
{
	ULONG	InodeNo;

		/*
		 * First, see if it's in the cache
		 */
	InodeNo = 0;
	if (!CacheLookupByName(sDirCache, (ULONG) Inode->Super->Device, Inode->InodeNo, PathName, PathLen, &InodeNo))
	{
			
			/*
			 * No, not in the cache, read the directory
			 */
		InodeNo = DirLookupEntry(Inode, PathName, PathLen, isShortName);
	}

	return InodeNo;
}


/*
 * Get Inode identified by the PathName
 *
 * IN:
 *
 * OUT:
 *
 * RETURNS:
 */
static TInode* doDirName2Inode(TInode *DirInode, char *DirName, int DirNameLength)
{
	char			ShortName[13];
	char			*PathStart, *PathEnd;
	int				PathLen;
	ULONG			InodeNo;
	TInode			*ParentInode;
	TInode			*ChildInode;
	TInode			*FollowedInode;

	char			Name[65];
	
		/*
		 * DirName2Inode explicitly requests for a BCS pathname
		 * and not for a Unicode pathname. This has the disadvantage
		 * that all FSD requests (dir/open/delete etc) have to
		 * do the unicode->bcs conversion themselves. However,
		 * in order to following links, it is easiest to recursively
		 * call DirName2Inode (for absolute links only!). In that
		 * case we simply cannot have a MAX_PATH array on the stack.
		 */

		/*
		 * If path is empty we return the directory inode
		 */
	if (!DirName || !DirNameLength)
		return InodeCloneInode(DirInode);

		/*
		 * If Dir name is absolute, get the root inode
		 */
	if (*DirName == '/')
	{
		DirName++;
		DirNameLength--;
		ParentInode = InodeGetRootInode(DirInode->Super);
	}
	else
		ParentInode = InodeCloneInode(DirInode);

	PathEnd = PathStart = DirName;

	while(DirNameLength)
	{

			/*
			 * Make sure we the inode is a directory
			 */
		if (!S_ISDIR(ParentInode->Mode))
		{
			VxdDebugPrint(D_ERROR, "DirName2Inode: inode(dev=%s, ino=%lu) is not directory", ParentInode->Super->DeviceName, ParentInode->InodeNo);
			ChildInode = 0;
			break;
		}

			/*
			 * Stop criterium is reached iff PathEnd == 0,
			 * that is, there are no more path separators
			 */
		if ((PathEnd = strnchr(PathStart, '/', DirNameLength)))
		{
			PathLen = PathEnd - PathStart;
		}
		else
		{
			PathLen = DirNameLength;
		}

	
			/*
			 * Now, do some serious name 2 inode resolving.
			 */
		if (PathLen > 12)
		{

				/*
				 * This one is easy, it is a pure LFN filename, so
				 * we have to do an exact match 
				 */
			if (!(InodeNo = DirName2InodeNo(ParentInode, PathStart, PathLen, FALSE)))
			{
				ChildInode = 0;
				break;
			}
		}
		else
		{

				/*
				 * More difficult.
				 * The path may be:
				 * a) a name issued by a LFN filename aware app.
				 * b) a converted LFN name (containing a ~)
				 * c) a name issued by a non-LFN aware application
				 *
				 * Case b is simple and can be detected. Case
				 * a and b are indistinguishable.
				 * Consider open "FTP", this could issued from a
				 * dos application that is returned "FTP" (remember
				 * that we return "FTP" in ffirst/fnext calls when
				 * the entryname is "ftp"!!). It could also be
				 * issued by a win32 application and the filename
				 * is indeed "FTP" (its real name)
				 */				
			if (strnchr(PathStart, '~', PathLen))
			{
				if (!(InodeNo = DirName2InodeNo(ParentInode, PathStart, PathLen, TRUE)))
				{
					ChildInode = 0;
					break;
				}
			}
			else
			{
					/*
					 * First try an exact match
					 */
				if (!(InodeNo = DirName2InodeNo(ParentInode, PathStart, PathLen, FALSE)))
				{

						/*
						 * Now, iff the path is _all_ uppercased
						 * we try an exact match with the lower
						 * cased filename
						 */
//					if (StrConvAndTestToLower(ShortName, PathStart, PathLen))
//					{
//						if (!(InodeNo = DirName2InodeNo(ParentInode, ShortName, PathLen, FALSE)))
//						{
//							ChildInode = 0;
//							break;
//						}
//					}
//					else
					{
							/*
							 * Try matching names using short file
							 * names
							 */
						if (!(InodeNo = DirName2InodeNo(ParentInode, PathStart, PathLen, TRUE)))
						{
							ChildInode = 0;
							break;
						}
					}
				}
			}
		}


			/*
			 * Next, Get the TInode of the just found inode
			 */
		if (!(ChildInode = InodeGetInode(ParentInode, InodeNo, PathStart, PathLen)))
		{
			ChildInode = 0;
			break;
		}

			/*
			 * If ChildInode is a symbolic link then
			 * follow the link
			 */
		if (S_ISLNK(ChildInode->Mode))
		{
			memcpy(Name, PathStart, PathLen);
			Name[PathLen] = 0;
			VxdDebugPrint(D_DIR, "doDirName2Inode: Found symlink name=%s, mode=0x%x", Name, (int) ChildInode->Mode);

			if ((FollowedInode = FollowLink(ParentInode, ChildInode)))
			{
				InodeRelease(ChildInode);
				ChildInode = FollowedInode;
			}
		}

			/*
			 * Whether or not we followed a link (successful
			 * or not), ChildInode points to the inode we need
			 */

			/*
			 * Continue search
			 */
		PathStart = PathEnd + 1;
		if (DirNameLength == PathLen)
			DirNameLength = 0;
		else
			DirNameLength -= (PathLen + 1);
		InodeRelease(ParentInode);
		ParentInode = InodeCloneInode(ChildInode);
	}

	InodeRelease(ParentInode);
		
	return ChildInode;
}



static TInode* FollowLink(TInode *DirInode, TInode *SymLinkInode)
{
	char		*TargetName;
	TInode		*FollowedInode;
	ext2_inode	*Ext2Inode;
	TBlock		*FirstBlock;
	char		Name[65];

	VxdDebugPrint(D_DIR, "FollowLink: start");
	
	Ext2Inode = InodeLock(SymLinkInode);
	

		/*
		 * First get the filename of the file object the
		 * link is pointing to
		 */
	TargetName = 0;
	FirstBlock = 0;
	if (Ext2Inode->i_size <= 64)
	{
		TargetName = (char *) Ext2Inode->i_block;
		memcpy(Name, TargetName, Ext2Inode->i_size);
		Name[Ext2Inode->i_size] = 0;
		VxdDebugPrint(D_DIR, "Direct link name=%s", Name);
	}
	else
	{
		if ((FirstBlock = InodeGetBlock(SymLinkInode, 0)))
		{
			TargetName = (char *) BlockLock(FirstBlock);
		}
	}

		/*
		 * If we have a name, resolve it
		 */
	
	if (TargetName)
	{
		if (!(FollowedInode = doDirName2Inode(DirInode, TargetName, (int) Ext2Inode->i_size)))
		{
			VxdDebugPrint(D_DIR, "FollowLink: target not found");
		}
		else
			VxdDebugPrint(D_DIR, "FollowLink: target found");
	}
	else
	{
		VxdDebugPrint(D_DIR, "FollowLink: no targetname found");
		FollowedInode = 0;
	}

	
		/*
		 * Clean up if necessary
		 */
	if (FirstBlock)
	{
		BlockUnlock(FirstBlock);
		BlockRelease(FirstBlock);
	}
	InodeUnlock(SymLinkInode);

	VxdDebugPrint(D_DIR, "FollowLink: done");

	return FollowedInode;
}




/**********************************
 *
 * INTERFACE ROUTINES
 *
 **********************************/



/*
 * Get Inode identified by the PathName
 *
 * IN:
 *
 * OUT:
 *
 * RETURNS:
 */
TInode* DirName2Inode(TSuperBlock *Super, char *DirName)
{
	TInode			*RootInode;
	TInode			*TargetInode;
	
	VxdDebugPrint(D_DIR, "DirName2Inode: device=%s, dir=%s", Super->DeviceName, DirName);

	RootInode = InodeGetRootInode(Super);
	TargetInode = doDirName2Inode(RootInode, DirName, DirName ? strlen(DirName) : 0);
	InodeRelease(RootInode);
		
	VxdDebugPrint(D_DIR, "DirName2Inode: done, ino=%lu", 
		TargetInode ? TargetInode->InodeNo : 0);

	return TargetInode;
}



/*
 * DirFindEntry searches for a match in a directory
 *
 * PRE:
 * If FindContext->Block is valid it must _not_ be locked.
 * FindContext->FoundInode must _not_ be valid, that is, a client
 * must have released it the previous time.
 *
 * POST:
 * If an entry is found:
 * -	FindContext->Block is locked and must be unlocked _before_
 *		a new DirFindEntry is issued.
 * -	FindContext->FoundEntry points in FindContext->Block->BlockData
 * -	FindContext->FoundInode points to the inode of the found entry,
 *		it must be released _before_ a new DirFindEntry is issued.
 *
 * IN:
 *
 * OUT:
 *
 * RETURNS
 */
BOOL DirFindEntry(TFindContext *FindContext, BOOL isShortName)
{
	TInode			*FollowedInode;
	ULONG			BlockSize;
	char			*DirData;
	BOOL			Succes;
	char			Name[65];
	char			TempName[128];

	VxdDebugPrint(D_DIR, "DirFindEntry, inode=(dev=%s, ino=%lu), search=%s",
			FindContext->Inode->Super->DeviceName,
			FindContext->Inode->InodeNo,
			FindContext->SearchName);

		/*
		 * Loop over all directory entries, starting where we left
		 * off the previous time
		 */
	BlockSize = DevGetBlockSize(FindContext->Inode->Super->Device);

		/*
		 * itterate over directory blocks
		 * Note that FindContext->LogicalBlockNo and FindContext->RecLen
		 * preserve the state of the search
		 */
	for (; FindContext->LogicalBlockNo<FindContext->nrBlocks ; FindContext->LogicalBlockNo++)
	{
		VxdDebugPrint(D_DIR, "DirFindEntry: looking in block=%lu of blocks=%lu", FindContext->LogicalBlockNo, FindContext->nrBlocks);

			/*
			 * See if the current block is present already
			 */
		if (!FindContext->Block)
		{
			if (!(FindContext->Block = InodeGetBlock(FindContext->Inode, FindContext->LogicalBlockNo)))
			{
				VxdDebugPrint(D_ERROR, "DirFindEntry: InodeGetBlock failed BlockNo=%lu, continuing on next block", FindContext->LogicalBlockNo);
				continue;
			}
		}
		
			/*
			 * For each block we loop over the contained
			 * directory entries and look for a match.
			 */
		DirData = BlockLock(FindContext->Block);

		while (FindContext->RecLen < BlockSize)
		{
				/*
				 * Get the next directory entry, mark it as found, and
				 * set RecLen 
				 */
			FindContext->FoundDirEntry = (ext2_dir_entry *) (DirData + FindContext->RecLen);
			FindContext->RecLen += FindContext->FoundDirEntry->rec_len;

				/*
				 * Add each entry to the cache
				 */
			CacheAddByName(sDirCache, (ULONG) FindContext->Inode->Super->Device, FindContext->Inode->InodeNo, FindContext->FoundDirEntry->name, FindContext->FoundDirEntry->name_len, &FindContext->FoundDirEntry->inode);

#ifdef DEBUG
				/*
				 * First see if we have a name match
				 */
			memcpy(TempName, FindContext->FoundDirEntry->name, FindContext->FoundDirEntry->name_len);
			TempName[FindContext->FoundDirEntry->name_len] = 0;
#endif			
			if (DirIsEntryNameMatch(FindContext, isShortName))
			{
				
					/*
					 * Now, get the inode of the found entry and see
					 * if the attributes match
					 */
				if ((FindContext->FoundInode = InodeGetInode(FindContext->Inode, FindContext->FoundDirEntry->inode, FindContext->FoundDirEntry->name, FindContext->FoundDirEntry->name_len)))
				{
				
						/*
						 * If the found inode is a symbolic link then
						 * follow the link first
						 */
					if (S_ISLNK(FindContext->FoundInode->Mode))
					{
						memcpy(Name, FindContext->FoundDirEntry->name, FindContext->FoundDirEntry->name_len);
						Name[FindContext->FoundDirEntry->name_len] = 0;
						VxdDebugPrint(D_DIR, "DirFindEntry: Found symlink name=%s, mode=0x%x", Name, (int) FindContext->FoundInode->Mode);

							/*
							 * If the followed link is a directory, don't follow it
							 * or we have a possible recursion.
							 * For example, on most machines /etc/inet is a sym link to /etc
							 */
						if ((FollowedInode = FollowLink(FindContext->Inode, FindContext->FoundInode)))
						{
							if (!(FollowedInode->DosAttrib & FILE_ATTRIBUTE_DIRECTORY))
							{
								InodeRelease(FindContext->FoundInode);
								FindContext->FoundInode = FollowedInode;
							}
							else
								InodeRelease(FollowedInode);
						}
					}

				
					if (DirIsEntryAttribMatch(FindContext, isShortName))
					{
						Succes = TRUE;

							/*
							 * !!! We leave the FindContext->Block locked !!!
							 */
						goto find_done;
					}
					else
					{
						VxdDebugPrint(D_DIR, "DirFindEntry: named macthed, attrib didn't name=%s, attrib=0x%x", TempName, (int) FindContext->FoundInode->DosAttrib);
						InodeRelease(FindContext->FoundInode);
					}
				}
			}
			else
			{
				VxdDebugPrint(D_DIR, "DirFindEntry: name nor attrib matched, name=%s", TempName);
			}
		}

			/*
			 * unlock and release the block
			 */
		if (FindContext->Block)
		{
			BlockUnlock(FindContext->Block);
			BlockRelease(FindContext->Block);
		}

			/*
			 * Next time, we start at offset 0 in the next block
			 */
		FindContext->Block = 0;
		FindContext->RecLen = 0;
	}
	Succes = FALSE;
	
find_done:
	VxdDebugPrint(D_DIR, "DirFindEntry: done, ino=%lu", 
			Succes ? FindContext->FoundDirEntry->inode : 0);

	return Succes;
}


/*
 * Create a FindContext
 *
 * PRE:
 * DirInode is an directory inode.
 *
 * POST:
 * DirInode is locked. Created FindContext->Block is initialised
 * with logical block 0 of the DirInode. The context variants and
 * invariants are initialised (excluding the "return values")
 *
 * IN:
 *
 * RETURNS:
 */
TFindContext* DirCreateFindContext(TInode *DirInode, char *SearchName, int SearchFlags)
{
	TFindContext	*FindContext = 0;
	ext2_inode		*Ext2Inode;


	VxdDebugPrint(D_DIR, "DirCreateFindContext, inode=(dev=%s, ino=%lu), search=%s",
			DirInode->Super->DeviceName,
			DirInode->InodeNo,
			SearchName);

		/*
		 * Create the find context
		 */
	if (!(FindContext = (TFindContext	*) calloc (1, sizeof(TFindContext))))
	{
		VxdDebugPrint(D_ERROR, "DirCreateFindContext: done, could not malloc TFindContext");
		return 0;
	}

		/*
		 * Read the first logical block beloning to the inode
		 */
	if (!(FindContext->Block = InodeGetBlock(DirInode, 0)))
	{
		VxdDebugPrint(D_ERROR, "DirCreateFindContext: done, could not get block 0");
		free(FindContext);
		return 0;
	}

		/*
		 * Lock the inode
		 */
	Ext2Inode = InodeLock(DirInode);
		
		/*
		 * Setup the find context
		 */
	strcpy(FindContext->SearchName, SearchName);
	FindContext->SearchFlags = SearchFlags;
	FindContext->Inode = DirInode;
	FindContext->nrBlocks = Ext2Inode->i_size / DevGetBlockSize(DirInode->Super->Device);

	VxdDebugPrint(D_DIR, "DirCreateFindContext: done");

	return FindContext;
}


/*
 * Destroy a FindContext (created by Ext2_CreateFindContext)
 * 
 * PRE:
 * FindContext->Inode is locked. If FindContext->Block exists,
 * it is unlocked.
 *
 * POST:
 * FindContext->Inode is unlocked and releases. If FindContext exists
 * it is released. The FindContext is freed.
 *
 * IN:
 * 
 *
 * OUT:
 * 
 * RETURNS
 */
void DirDestroyFindContext(TFindContext *FindContext)
{
	
	VxdDebugPrint(D_DIR, "DirDestroyFindContext, inode=(dev=%s, ino=%lu), search=%s",
			FindContext->Inode->Super->DeviceName,
			FindContext->Inode->InodeNo,
			FindContext->SearchName);
	
		/*
		 * Important!! Release the the block and the inode
		 */
	if (FindContext->Block)
		BlockRelease(FindContext->Block);
	InodeUnlock(FindContext->Inode);
	InodeRelease(FindContext->Inode);
	free(FindContext);

	VxdDebugPrint(D_DIR, "DirDestroyFindContext: done");
}



BOOL DirInitialise(UINT DirCacheSize)
{
	VxdDebugPrint(D_SYSTEM, "DirInitialise: cachesize=%u", DirCacheSize);

		/*
		 * Create the Directory Entry Cache.
		 */
	if (! (sDirCache = CacheCreate(sDirCacheName, DirCacheSize, sizeof(ULONG))))
	{
		VxdDebugPrint(D_ERROR, "DirInitialise: could not create dir cache");
		goto dir_init_1;
	}

	VxdDebugPrint(D_SYSTEM, "DirInitialise: done");

	return TRUE;

dir_init_1:
	VxdDebugPrint(D_SYSTEM, "DirInitialise: done");

	return FALSE;
}




void DirCleanup()
{
	VxdDebugPrint(D_SYSTEM, "DirCleanup");
	
	CacheDestroy(sDirCache);

	VxdDebugPrint(D_SYSTEM, "DirCleanup: done");
}

void DirCacheInfo()
{
	unsigned long	Lookups;
	unsigned long	Hits;

	CacheGetStats(sDirCache, &Lookups, &Hits);
	VxdDebugPrint(D_SYSTEM, "DirCacheInfo: lookup=%lu, hit=%lu, ratio=%lu",
			Lookups, Hits, 
			Lookups? Hits*100/ Lookups: 0);
}

