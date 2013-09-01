#include "vxd.h"

#include "shared\vxddebug.h"
#include "ext2fs.h"
#include "inode.h"
#include "super.h"

//#include "util.h"

/*
 * Macro-instructions used to manage group descriptors
 */
#define EXT2_BLOCKS_PER_GROUP(s)	( (s)->s_blocks_per_group)
#define EXT2_DESC_PER_BLOCK(s)		( (s)->s_desc_per_block)
#define EXT2_INODES_PER_GROUP(s)	( (s)->s_inodes_per_group)


#define INODE_BMAP(inode,nr)	((inode)->i_block[nr])
#define BLOCK_BMAP(block,nr)	( ((ULONG *) (block))[nr])



/********************************
 *
 * STATIC Data
 *
 *********************************/

//none

/********************************
 *
 * STATIC HELPERS
 *
 *********************************/

static void Ext2_DumpInode(ext2_inode *Inode)
{
	VxdDebugPrint(D_EXT2, "dumInode, size=%lu, mode=%lu, user=%lu, blocks=%lu",
		Inode->i_size, Inode->i_mode, Inode->i_uid, Inode->i_blocks);
}







/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/



/*
 * Translates a logical block nr for this object to a physical
 * block nr
 *
 * IN
 * Drive
 * 
 */
ULONG Ext2BlockMap(TInode *Inode, ULONG BlockNo)
{
	TBlock				*block;
	ULONG 				blockno;
	TSuperBlock			*Super = Inode->Super;
	ext2_super_block	*ext2sb = (ext2_super_block *) Super->SuperBlock->BlockData;
	ULONG 				addr_per_block = EXT2_ADDR_PER_BLOCK(ext2sb);
	
	if (BlockNo >= EXT2_NDIR_BLOCKS + addr_per_block +
			 addr_per_block * addr_per_block +
			 addr_per_block * addr_per_block * addr_per_block)
	{
		VxdDebugPrint(D_ERROR, "Ext2BlockMap: blockno too big, blockno=%lu", BlockNo);
		blockno = 0ul;
		goto bmap_done;
	}

		/*
		 * Direct block ?
		 */
	if (BlockNo < EXT2_NDIR_BLOCKS)
	{
		blockno = INODE_BMAP(Inode->Ext2Inode, BlockNo);
		goto bmap_done;
	}

		/*
		 * indirect ?
		 */
	BlockNo -= EXT2_NDIR_BLOCKS;
	if (BlockNo < addr_per_block)
	{
		blockno = INODE_BMAP(Inode->Ext2Inode, EXT2_IND_BLOCK);
		if (!blockno)
		{
			VxdDebugPrint(D_ERROR, "Ext2BlockMap: indirect blockno==0");
			goto bmap_done;
		}

		if (!(block = BlockGetBlock(Super->Device, blockno)))
		{
			VxdDebugPrint(D_ERROR, "Ext2BlockMap: could not read block");
			blockno = 0ul;
			goto bmap_done;
		}
		blockno = BLOCK_BMAP(block->BlockData, BlockNo);
		BlockRelease(block);
		goto bmap_done;
	}

		/*
		 * second indirect ?
		 */
	BlockNo -= addr_per_block;
	if (BlockNo < addr_per_block * addr_per_block)
	{
		blockno = INODE_BMAP(Inode->Ext2Inode, EXT2_DIND_BLOCK);
		if (!blockno)
		{
			VxdDebugPrint(D_ERROR, "2nd indirect blockno==0");
			blockno = 0ul;
			goto bmap_done;
		}
		
		if (!(block = BlockGetBlock(Super->Device, blockno)))
		{
			VxdDebugPrint(D_ERROR, "Ext2BlockMap: could not read block");
			blockno = 0ul;
			goto bmap_done;
		}

		blockno = BLOCK_BMAP(block->BlockData, BlockNo / addr_per_block);
		BlockRelease(block);
		if (!blockno)
		{
			VxdDebugPrint(D_ERROR, "2nd indirect bmap blockno==0");
			blockno = 0ul;
			goto bmap_done;
		}

		if (!(block = BlockGetBlock(Super->Device, blockno)))
		{
			VxdDebugPrint(D_ERROR, "Ext2BlockMap: could not read block");
			blockno = 0ul;
			goto bmap_done;
		}

		blockno = BLOCK_BMAP(block->BlockData, BlockNo & (addr_per_block - 1));
		BlockRelease(block);
		goto bmap_done;
	}

		/*
		 * Third indirect it is then...
		 */
	BlockNo -= addr_per_block * addr_per_block;
	blockno = INODE_BMAP(Inode->Ext2Inode, EXT2_TIND_BLOCK);
	if (!blockno)
	{
		VxdDebugPrint(D_ERROR, "3nd indirect blockno==0");
		blockno = 0ul;
		goto bmap_done;
	}

	if (!(block = BlockGetBlock(Super->Device, blockno)))
	{
		VxdDebugPrint(D_ERROR, "Ext2BlockMap: could not read block");
		blockno = 0ul;
		goto bmap_done;
	}
	blockno = BLOCK_BMAP(block->BlockData ,BlockNo / (addr_per_block * addr_per_block));
	BlockRelease(block);

	if (!blockno)
	{
		VxdDebugPrint(D_ERROR, "3nd indirect bmap blockno==0");
		blockno = 0ul;
		goto bmap_done;
	}

	if (!(block = BlockGetBlock(Super->Device, blockno)))
	{
		VxdDebugPrint(D_ERROR, "Ext2BlockMap: could not read block");
		blockno = 0ul;
		goto bmap_done;
	}
	blockno = BLOCK_BMAP(block->BlockData, (BlockNo / addr_per_block) & (addr_per_block - 1));
	
	BlockRelease(block);

	if (!blockno)
	{
		VxdDebugPrint(D_ERROR, "3nd indirect bmap bmap blockno==0");
		blockno = 0ul;
		goto bmap_done;
	}

	if (!(block = BlockGetBlock(Super->Device, blockno)))
	{
		VxdDebugPrint(D_ERROR, "Ext2BlockMap: could not read block");
		blockno = 0ul;
		goto bmap_done;
	}

	blockno = BLOCK_BMAP(block->BlockData, BlockNo & (addr_per_block - 1));
	BlockRelease(block);

bmap_done:
	VxdDebugPrint(D_EXT2, "Ext2BlockMap: done, block=%lu", blockno);
	return blockno;
}





/*
 * Read an ext2 inode from disk
 *
 * PRE:
 * fields in Inode that needs to initialised:
 *	Inode->Super	: pointer to _fully_ initialised superblock
 *	Inode->InodeNo	: inode nr on the file system administrated by 
 *						Superblock
 *
 *
 * POST:
 * If succesfull, the following fiels are initialised
 *	Inode->Block	: pointer to TBlock in which the inode resides
 *						(note that TBlock's are reference counted)
 *	Inode->Ext2Inode: pointer inside the TBlock->BlockData to point
 *						to the inode. Remember that a diskblock (1 KB,
 *						2 KB or 4 KB) contains	several inodes 
 *						(128 bytes)
 *
 * IN:
 *	Inode		: pointer to inode structure (see PRE for required
 *					initialised fields)
 *
 * OUT:
 *	Inode		: pointer to inode structure (see POST for filled fields)
 *
 * RETURNS:
 *	TRUE on succes, FALSE otherwise
 */

BOOL Ext2ReadInode(TInode *Inode)
{
	BOOL				succes;
	ULONG 				block_group;
	ULONG 				group_desc;
	ULONG 				desc;
	ULONG 				block;
	ULONG 				inodeno;
	ext2_group_desc		*gdp;
	TSuperBlock			*Super = Inode->Super;
	ext2_super_block	*ext2sb = (ext2_super_block *) Inode->Super->SuperBlock->BlockData;
	
	//SuperLock(Inode->Super);
	VxdDebugPrint(D_EXT2, "Ext2ReadInode:  dev=%s, inono=%lu",
		Inode->Super->DeviceName,
		Inode->InodeNo);

	succes = FALSE;
	inodeno = Inode->InodeNo;

	if ((inodeno != EXT2_ROOT_INO && inodeno != EXT2_ACL_IDX_INO &&
		 inodeno != EXT2_ACL_DATA_INO && inodeno < EXT2_FIRST_INO) ||
		 inodeno > ext2sb->s_inodes_count)
	{
		VxdDebugPrint(D_ERROR, "Ext2ReadInode, bad inode number: %lu", inodeno);
		goto ext2_ino_read_err;
	}

	block_group = (inodeno - 1) / EXT2_INODES_PER_GROUP(ext2sb);
	if (block_group >= Inode->Super->s_groups_count)
	{
		VxdDebugPrint(D_ERROR, "Ext2ReadInode, group >= groups count");
		goto ext2_ino_read_err;
	}

	group_desc = block_group / EXT2_DESC_PER_BLOCK(Super);
	desc = block_group % EXT2_DESC_PER_BLOCK(Super);
	gdp = (ext2_group_desc *) Super->s_group_desc[group_desc]->BlockData;
	block = gdp[desc].bg_inode_table +	
			(((inodeno - 1) % EXT2_INODES_PER_GROUP(ext2sb)) 
			/ EXT2_INODES_PER_BLOCK(ext2sb));

	if (!(Inode->Block = BlockGetBlock(Super->Device, block)))
	{
		VxdDebugPrint(D_ERROR, "Ext2ReadInode: could not readblock %lu", block);
		goto ext2_ino_read_err;
	}

	Inode->Ext2Inode = ((ext2_inode *) Inode->Block->BlockData) + (inodeno - 1) % EXT2_INODES_PER_BLOCK(ext2sb);
	//Ext2_DumInode(Inode);
	succes = TRUE;

ext2_ino_read_err:
	
	VxdDebugPrint(D_EXT2, "Ext2ReadInode: done");

	return succes;
}





/*
 * Read an ext2 superblock on the specified device
 * 
 * PRE CONDITIONS:
 * Super->Mutex, Super->Device and Super->DeviceName must exist
 *
 * POST CONDITIONS:
 *	All ext2_superblock fields in Super are initialised
 *	if superblock could be read
 *
 * IN:
 * 
 * OUT:
 *	<none>
 *
 * RETURNS:
 *	<none>
 *
 */
BOOL Ext2GetSuperBlock(TSuperBlock *Super)
{
	ULONG 				logic_sb_block;
	int 				db_count, i;
	USHORT				oldlogsize;

	ext2_super_block		*ext2sb;

	//SuperLock(Super);

	VxdDebugPrint(D_EXT2, "Ext2GetSuperBlock: super=%x, device=%s", (ULONG) Super, Super->DeviceName);

		/*
		 * Set blocksize to 1024 bytes
		 */
	oldlogsize = DevGetLogBlockSize(Super->Device);
	
		/*
		 * Prefered way to read superblock: try blocksizes 1024, 2048 and 4096
		 */
	for (i=0 ; i<3 ; i++)
	{
		VxdDebugPrint(D_EXT2, "Ext2GetSuperBlock: trying blocksize %lu", (ULONG) 1024 << i);
		DevSetLogBlockSize(Super->Device, i+1);

			/*
			 * Superblock is located in the first logical block
			 */
		if (!(Super->SuperBlock = BlockGetBlock(Super->Device, 1)))
		{
			continue;
		}

		ext2sb = BlockLock(Super->SuperBlock);
			
		if (ext2sb->s_magic == EXT2_SUPER_MAGIC)
			break;

			/*
			 * Magic failed, unlock and release block
			 */
		BlockUnlock(Super->SuperBlock);
		BlockRelease(Super->SuperBlock);
	}
		/*
		 * If we reach here, we have either found a 
		 * locked superblock (correct magic) or no superblock
		 * at all
		 */
	if (i == 3)
	{
		VxdDebugPrint(D_ERROR, "Ext2GetSuperBlock: failed, tried all blocksize");
		goto read_sb_err_1;
	}

	VxdDebugPrint(D_SYSTEM, "Ext2GetSuperBlock: magic check succeed, bloksize=%lu", DevGetBlockSize(Super->Device));

	logic_sb_block = ext2sb->s_first_data_block;
	Super->s_frag_size = EXT2_MIN_FRAG_SIZE <<  ext2sb->s_log_frag_size;
	if (Super->s_frag_size)
		Super->s_frags_per_block = EXT2_BLOCK_SIZE(ext2sb) / Super->s_frag_size;
	else
		ext2sb->s_magic = 0;
	Super->s_blocks_per_group = ext2sb->s_blocks_per_group;
	Super->s_frags_per_group = ext2sb->s_frags_per_group;
	Super->s_inodes_per_group = ext2sb->s_inodes_per_group;
	Super->s_inodes_per_block = EXT2_BLOCK_SIZE(ext2sb) / sizeof (ext2_inode);
	Super->s_itb_per_group = Super->s_inodes_per_group / Super->s_inodes_per_block;
	Super->s_desc_per_block = EXT2_BLOCK_SIZE(ext2sb) / sizeof (ext2_group_desc);

	VxdDebugPrint(D_EXT2, "Ext2GetSuperBlock: setup group descriptors");

	Super->s_groups_count = (ext2sb->s_blocks_count -ext2sb->s_first_data_block +
					   EXT2_BLOCKS_PER_GROUP(ext2sb) - 1) /
				       EXT2_BLOCKS_PER_GROUP(ext2sb);
	db_count = (Super->s_groups_count + EXT2_DESC_PER_BLOCK(Super) - 1) /
		   EXT2_DESC_PER_BLOCK(Super);

	Super->s_db_count = db_count;
	Super->s_group_desc = (TBlock **) calloc (db_count, sizeof (TBlock*));
	if (!Super->s_group_desc)
	{
		VxdDebugPrint(D_ERROR, "Ext2GetSuperBlock: could not malloc group desc. table");
		goto read_sb_err_2;
	}

		/*
		 * Read the group descriptors
		 */
	for (i = 0; i < db_count; i++)
	{
		VxdDebugPrint(D_EXT2, "Setup group descriptor %i", i);

		if (!(Super->s_group_desc[i] = BlockGetBlock(Super->Device, logic_sb_block + i + 1)))
		{
			VxdDebugPrint(D_ERROR, "Ext2GetSuperBlock: could not read block for group desc %i", i);
			goto read_sb_err_3;
		}
	}

		/*
		 * We should check the group descriptors here but we leave that
		 * for the future :-)
		 */
	for (i = 0; i < EXT2_MAX_GROUP_LOADED; i++)
	{
		Super->s_inode_bitmap_number[i] = 0;
		Super->s_inode_bitmap[i] = NULL;
		Super->s_block_bitmap_number[i] = 0;
		Super->s_block_bitmap[i] = NULL;
	}
	Super->s_loaded_inode_bitmaps = 0;
	Super->s_loaded_block_bitmaps = 0;
	Super->s_db_per_group = db_count;

		/*
		 * Don't forget to unlock it :-)
		 */
	BlockUnlock(Super->SuperBlock);
	VxdDebugPrint(D_EXT2, "Ext2GetSuperBlock: done");
	SuperUnlock(Super);

	return TRUE;

		/*
		 * Error labels, we clean up in reverse order 
		 */
read_sb_err_3:
	for (i = 0; i < db_count; i++)
	{
		if (Super->s_group_desc[i])
			BlockRelease(Super->s_group_desc[i]);
	}
	free(Super->s_group_desc);

read_sb_err_2:
	BlockUnlock(Super->SuperBlock);

read_sb_err_1:

	VxdDebugPrint(D_EXT2, "Ext2GetSuperBlock: done");
	//SuperUnlock(Super);

	return FALSE;
}


/*
 * Release the ext2 info in a SuperBlock structure 
 * 
 * PRE CONDITIONS:
 * Super->Mutex, Super->Device and Super->DeviceName must be set 
 * and valid
 *
 * POST CONDITIONS:
 * Except Super->Mutex, Super->Device and Super->DeviceName 
 * all fields are invalid
 *
 * IN:
 * 
 * OUT:
 *	<none>
 *
 * RETURNS:
 *	<none>
 *
 */
void Ext2ReleaseSuperBlock(TSuperBlock *Super)
{
	int 				i;

	SuperLock(Super);

	VxdDebugPrint(D_EXT2, "Ext2ReleaseSuperBlock: super=%x, dev=%s", (ULONG) Super, Super->DeviceName);

	for (i = 0; i < Super->s_db_count; i++)
	{
		BlockRelease(Super->s_group_desc[i]);
	}

	free(Super->s_group_desc);
	BlockRelease(Super->SuperBlock);

	VxdDebugPrint(D_EXT2, "Ext2ReleaseSuperBlock: done");

	SuperUnlock(Super);

}

