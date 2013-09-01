#ifndef __SUPERBLK_H
#define __SUPERBLK_H

#include "block.h"
#include "ext2fs.h"


#define	MAX_DEV_NAME 16



/*
 * Superblock in memory. Basically, it's the ext2_superblock in
 * memory, with some additional fields
 */
typedef struct _SuperBlock
{
		/*
		 * Info from ext2_superblock
		 */
	ULONG s_frag_size;		 /* Size of a fragment in bytes */
	ULONG s_frags_per_block; /* Number of fragments per block */
	ULONG s_inodes_per_block;/* Number of inodes per block */
	ULONG s_frags_per_group; /* Number of fragments in a group */
	ULONG s_blocks_per_group;/* Number of blocks in a group */
	ULONG s_inodes_per_group;/* Number of inodes in a group */
	ULONG s_itb_per_group;	 /* Number of inode table blocks per group */
	ULONG s_db_per_group;	 /* Number of descriptor blocks per group */
	ULONG s_desc_per_block;	 /* Number of group descriptors per block */
	ULONG s_groups_count;	 /* Number of groups in the fs */

	//TExt2GroupDesc **s_group_desc;
	int				s_db_count;
	TBlock			**s_group_desc;

	USHORT	s_loaded_inode_bitmaps;
	USHORT	s_loaded_block_bitmaps;
	ULONG	s_inode_bitmap_number[EXT2_MAX_GROUP_LOADED];

	char	*s_inode_bitmap[EXT2_MAX_GROUP_LOADED];
	char	*s_block_bitmap[EXT2_MAX_GROUP_LOADED];
	ULONG	s_block_bitmap_number[EXT2_MAX_GROUP_LOADED];


		/*
		 * Additional info, needed to administrate this/all superblock(s)
		 */
	struct _SuperBlock	*SuperNext;
	TBlock				*SuperBlock;
	TDevice				Device;
	struct _Inode		*RootInode;	/* Pointer to the root directory's inode  */

	char				DeviceName[MAX_DEV_NAME];
	char				DosDrive;
	void				*Vrp;
	MUTEXHANDLE			Mutex;
} TSuperBlock;


/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/


void SuperCleanup();
BOOL SuperInitialise();

TSuperBlock*	SuperMountDevice(TDevice Device, struct _Inode *InodeRoot);
BOOL			SuperUmount(TDevice Device);
TSuperBlock*	SuperMountRoot(TDevice Device);


struct ext2_super_block* SuperLock(TSuperBlock *Super);
void SuperUnlock(TSuperBlock *Super);

/*
 * For now, we must mount each partition on a separate
 * drive letter. In the future we will allow:
 * mount /dev/xxx q:		(root device)
 * mount /dev/yyy q:/usr	
 * mount /dev/zzz q:/...
 */
//TSuperBlock*	SuperGetSuperRoot();
TSuperBlock* SuperMountDeviceStandAlone(TDevice Device);


#endif			/* __SUPERBLK_H */