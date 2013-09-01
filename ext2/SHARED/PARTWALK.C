#include "vxd.h"

#include "shared\parttype.h"
#include "shared\partwalk.h"
#include "shared\vxddebug.h"
#include "shared\blkdev.h"
#include "dcb.h"



/*********************************
 *
 * STATIC HELPERS
 *
 **********************************/


static int WalkExtendedPartitions(PDCB Dcb, int Disk, TDiskPartition *Partition, int ExtPartitionNo, TPartitionFunc CallBack)
{
	TDiskPartition	*LogicalPartition;
	ULONG			StartSector;
	ULONG			ThisSector;
	char			Sector[512];

	StartSector = Partition->start_sec;
	ThisSector = StartSector;

	while (ExtPartitionNo < MAX_PARTITION_PER_DISK && DevReadSector(Dcb, ThisSector, 1, Sector))
	{
	
		if (*((unsigned short *) (Sector + 510)) != 0xAA55)
			break;				

		LogicalPartition = (TDiskPartition *) (0x1BE + Sector);
		if (LogicalPartition->sys_ind == TYPE_EXTENDED_PARTITION ||	!LogicalPartition->nr_sec)
			break;  /* shouldn't happen */

			/*
			 * Ok, we fiddle with the parition start here.
			 * If we don't, the callback has no clue where
			 * the logical partition realy starts because
			 * LogicalPartition->start_sec is relative
			 * to the start of the extended partition.
			 */
		//LogicalPartition->start_sec += StartSector;
		LogicalPartition->start_sec += ThisSector;
		CallBack(Dcb, Disk, LogicalPartition, ExtPartitionNo);

		ExtPartitionNo++;
		LogicalPartition++;

		if (LogicalPartition->sys_ind != TYPE_EXTENDED_PARTITION ||	!LogicalPartition->nr_sec)
			break;  /* no more logicals in this Partition */

		ThisSector = StartSector + LogicalPartition->start_sec;
	}
	return ExtPartitionNo;

}



/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/


void WalkPartitions(PDCB Dcb, int Disk, TPartitionFunc CallBack)
{
	TDiskPartition	*Partition;
	char			Sector[512];
	int				ExtPartitionNr = 5;
	int				i;

	if (!DevReadSector(Dcb, 0, 1, Sector))
	{
		VxdDebugPrint(D_PARTITION, "WalkPartitions, done, could not read mbr record");
		return;
	}

		/*
		 * check for signature
		 */
	if (* ((unsigned short *) (Sector+510)) != 0xAA55)
	{
		VxdDebugPrint(D_PARTITION, "WalkPartitions: done, MBR does not contain a valid signature");
		return;
	}

	VxdDebugPrint(D_PARTITION, "WalkPartitions: MBR has valid signature");

	Partition = (TDiskPartition *) (0x1BE + Sector);
	for (i=1 ; i<=4 ; i++, Partition++)
	{
		if (!Partition->nr_sec)
			continue;

		CallBack(Dcb, Disk, Partition, i);

		if (Partition->sys_ind == TYPE_EXTENDED_PARTITION)
		{
			ExtPartitionNr = WalkExtendedPartitions(Dcb, Disk, Partition, ExtPartitionNr, CallBack);
		}
	}
	
	VxdDebugPrint(D_PARTITION, "WalkPartitions: done");
}



