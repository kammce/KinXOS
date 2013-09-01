#ifndef __INODE_H
#define __INODE_H


/*
 * Wrapper for an ext2 inode, used to
 * synchronise access to it
 */
typedef struct _Inode
{
		/*
		 * InodeNo and Super->Device uniquely identify
		 * an inode
		 */
	ULONG				InodeNo;
	struct _SuperBlock	*Super;

	struct _Block		*Block;
	struct ext2_inode	*Ext2Inode;
	struct _Inode		*ParentInode;
	ULONG				nrClients;
	ULONG				DosAttrib;
	USHORT				Mode;

	//struct _Inode	*InodeMount;

	struct _Inode		*InodeNext;
	struct _Inode		*InodePrev;
} TInode;


/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/


BOOL	InodeInitialise(UINT EstimatedInodesResident);
void	InodeCleanup();

TInode*	InodeGetRootInode(struct _SuperBlock *Super);
TInode* InodeGetInode(TInode *ParentInode, ULONG InodeNo, char *FileName, int NameLen);
void	InodeRelease(TInode *Inode);

TInode* InodeCloneInode(TInode *Inode);

void*	InodeLock(TInode *Inode);
void	InodeUnlock(TInode *Inode);

struct _Block*	InodeGetBlock(TInode *Inode, ULONG LogicalBlockNo);



#endif			/* __INODE_H */