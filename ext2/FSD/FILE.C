#include "vxd.h"

#include "shared\vxddebug.h"
#include "file.h"
#include "error.h"

#include "ext2fs.h"
#include "super.h"
#include "inode.h"
#include "unixstat.h"
#include "util.h"


BOOL FileRead(TInode *Inode, ULONG Pos, ULONG Length, char *Data)
{
	BOOL		succes;
	char		*blockdata;
	TBlock		*block;
	ULONG		blockno, offset, blocksize, transfersize;

	blocksize = DevGetBlockSize(Inode->Super->Device);
	blockno = Pos / blocksize;
	offset = Pos % blocksize;

	succes = TRUE;
	while (Length)
	{
		if (!(block = InodeGetBlock(Inode, blockno)))
		{
			succes = FALSE;
			break;
		}

		blockdata = BlockLock(block);

			/*
			 * How much to transfer for this block
			 */
		transfersize = blocksize - offset;
		if (transfersize > Length)
			transfersize = Length;

			/*
			 * transfer it
			 */
		memcpy(Data, blockdata + offset, transfersize);
		BlockUnlock(block);
		BlockRelease(block);

			/*
			 * Prepare next block transfer
			 */
		Data += transfersize;
		Length -= transfersize;
		offset = 0;
		blockno++;
	}

	return succes;
}


TFileContext* FileCreateFileContext(TInode *Inode)
{
	TFileContext	*filecontext;
	ext2_inode		*ext2inode;

	if (!(filecontext = (TFileContext *) calloc(1, sizeof(TFileContext))))
		return 0;
	filecontext->Inode = Inode;

	ext2inode = InodeLock(Inode);
	filecontext->FileSize = ext2inode->i_size;
	InodeUnlock(Inode);

	return filecontext;

}


void FileDestroyFileContext(TFileContext *FileContext)
{
	InodeRelease(FileContext->Inode);
	free(FileContext);
}
