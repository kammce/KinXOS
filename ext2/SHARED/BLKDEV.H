#ifndef __BLKDEV_H
#define __BLKDEV_H


/*
 * Hide the blockdevice type from other modules
 * You and I know it's a pointer to a DCB.
 */
typedef void *	TDevice;



/*********************************
 *
 * BLOCK BASED INTERFACE ROUNTINES
 *
 **********************************/


void	DevSetLogBlockSize(TDevice Device, ULONG LogBlockSize);
USHORT	DevGetLogBlockSize(TDevice Device);
UCHAR	DevGetPartitionType(TDevice Device);
BOOL	DevReadBlock(TDevice Device, ULONG BlockNo, UCHAR *BlockData);
void	DevGenerateName(TDevice Device, char *DevName);


__inline UINT DevGetBlockSize(TDevice Device)
{
	return (512 << ((PDCB)Device)->DCB_cmn.DCB_apparent_blk_shift);
}


/*********************************
 *
 * SECTOR BASED INTERFACE ROUNTINES
 *
 **********************************/

BOOL DevReadSector(TDevice Device, ULONG StartSector, ULONG NrSectors, char *Data);


#endif			/* __BLKDEV_H */
