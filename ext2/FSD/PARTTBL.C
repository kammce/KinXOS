#include "vxd.h"

#include "shared\partwalk.h"
#include "shared\vxddebug.h"
#include "shared\blkdev.h"
#include "shared\fsdioctl.h"
#include "parttbl.h"
#include "Dcb.h"



/**********************************
 *
 * STATIC DATA
 *
 **********************************/


static TPartitionTableEntry		sPartitionTable[PART_TABLE_ENRTIES];


/*********************************
 *
 * STATIC HELPERS
 *
 **********************************/


void PrintPartitionTable()
{
	int						Disk, PartitionNo;
	TPartitionTableEntry	*Partition;


	VxdDebugPrint(D_PARTITION, "****** PARTITION TABLE ******");

	for (Disk=0 ; Disk < MAX_IDE_DISK + MAX_SCSI_DISK ; Disk++)
	{
		for (PartitionNo=0 ; PartitionNo<MAX_PARTITION_PER_DISK ; PartitionNo++)
		{
			Partition = &sPartitionTable[Disk*MAX_PARTITION_PER_DISK + PartitionNo];
			if (Partition->nrSectors && Partition->StartSector)
			{
				VxdDebugPrint(D_PARTITION, "/dev/%s%c%i, %4lu MB, type=%xh   %s",
					Disk < MAX_IDE_DISK ? "hd" : "sd",
					Disk < MAX_IDE_DISK ? Disk + 'a' : Disk-MAX_IDE_DISK + 'a',
					PartitionNo,
					(ULONG) Partition->nrSectors >> 11,
					(int) Partition->PartitionType,
					(!(Partition->Dcb) && Partition->PartitionType == TYPE_LINUX_PARTITION) ? "(not recognised by fsdext2)" : ""
					);
			}
		}
	}

	VxdDebugPrint(D_PARTITION, "****** PARTITION TABLE ******");
}



/*
 * Given a partitionentry, we scan the physical DCB for a logical
 * dcb that matches the partition
 */
static void SearchForDcb(PDCB PhysicalDcb, TPartitionTableEntry *Entry)
{
	PDCB_COMMON	Dcb = ((PDCB_COMMON) PhysicalDcb)->DCB_next_logical_dcb;

	VxdDebugPrint(D_PARTITION, "SearchForDcb: start");

	while (Dcb)
	{
		if (Dcb->DCB_Partition_Start == Entry->StartSector 
				&& Dcb->DCB_partition_type == Entry->PartitionType)
		{
			VxdDebugPrint(D_PARTITION, "SearchForDcb: done, found DCB");
			Entry->Dcb = Dcb;
			return;
		}

		Dcb = Dcb->DCB_next_logical_dcb;
	}
	VxdDebugPrint(D_PARTITION, "SearchForDcb: done, nothing found");
}




/*
 * This is the callback passed to WalkPartitions. It is called for
 * each partition found on the specified disk.
 * We fill in the corresponding entry in the partition table and
 * scan for a logical DCB that correpsonds the partition entry found
 */
static void HandlePartition(PDCB Dcb, int Disk, TDiskPartition *DiskPartition, int PartitionNo)
{
 	TPartitionTableEntry	*Entry;

	Entry = &sPartitionTable[Disk * MAX_PARTITION_PER_DISK + PartitionNo];

	Entry->PartitionType = DiskPartition->sys_ind;
	Entry->nrSectors = DiskPartition->nr_sec;
	Entry->StartSector = DiskPartition->start_sec;

	VxdDebugPrint(D_PARTITION, "HandlePartition: %s%c%i, %4lu MB, type=%i",
		Disk < MAX_IDE_DISK ? "hd" : "sd",
		Disk < MAX_IDE_DISK ? Disk + 'a' : Disk-MAX_IDE_DISK + 'a',
		PartitionNo,
		(ULONG) DiskPartition->nr_sec >> 11,
		(int) DiskPartition->sys_ind);

	SearchForDcb(Dcb, Entry);
}



/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/

void PartitionGetTable(char **Table, DWORD *Size)
{
	*Table = (char *) sPartitionTable;
	*Size = sizeof(sPartitionTable);
}


BOOL PartitionMount(int Disk, int PartitionNo, int Drive, ULONG *Reply)
{
 	TPartitionTableEntry	*Entry;


	VxdDebugPrint(D_PARTITION, "PartitionMount: Disk=%i, PartitionNo=%i, Drive=%c",
			Disk, PartitionNo, Drive+'A');

	*Reply = 0;


	if (Disk > MAX_IDE_DISK + MAX_SCSI_DISK ||
			PartitionNo > MAX_PARTITION_PER_DISK ||
			Drive > 26)
	{
		VxdDebugPrint(D_PARTITION, "PartitionMount: done, invalid parameter");
		return FALSE;
	}

	Entry = &sPartitionTable[Disk * MAX_PARTITION_PER_DISK + PartitionNo];
	if (Entry->PartitionType != TYPE_LINUX_PARTITION)
	{
		VxdDebugPrint(D_PARTITION, "PartitionMount: done, not a linux partition");
		*Reply = ERROR_MOUNT_DEVICE_NOT_LINUX;
		return TRUE;
	} 

	if (Entry->Drive)
	{
		VxdDebugPrint(D_PARTITION, "PartitionMount: done, partition already mounted");
		*Reply = ERROR_MOUNT_DEVICE_ALREADY_MOUNTED;
		return TRUE;
	}

	if (!IspAssociateDcb((PDCB) Entry->Dcb, (CHAR) Drive, ISP_D_A_FL_NOSHELLMSG))
	{
		VxdDebugPrint(D_PARTITION, "PartitionMount: done, drive already exists");
		*Reply = ERROR_MOUNT_DRIVE_ALREADY_EXISTS;
		return TRUE;
	}
	VxdDebugPrint(D_PARTITION, "PartitionMount: done");

	return TRUE;
}



BOOL PartitionUMount(int Drive)
{
	return FALSE;
}





void PartitionInitialise()
{
	int				nrIde, nrScsi;
	PDCB_COMMON		Dcb;

	VxdDebugPrint(D_PARTITION, "PartitionInitialise: walking partition tables");

	nrIde = nrScsi = 0;
	Dcb = (PDCB_COMMON) IspGetFirstNextDcb(0, DCB_type_disk);
	memset(sPartitionTable, 0, sizeof(sPartitionTable));

	if (!Dcb)
	{
		VxdDebugPrint(D_PARTITION, "PartitionInitialise: no DCB found");
		return;
	}

	while(Dcb)
	{
		if (Dcb->DCB_device_flags & DCB_DEV_PHYSICAL)
		{
			if (((PDCB) Dcb)->DCB_bus_type == DCB_BUS_SCSI)
				WalkPartitions((PDCB) Dcb, MAX_IDE_DISK + nrScsi++, HandlePartition);
			else if (((PDCB) Dcb)->DCB_bus_type == DCB_BUS_ESDI)
				WalkPartitions((PDCB) Dcb, nrIde++, HandlePartition);
		}
		Dcb = (PDCB_COMMON) IspGetFirstNextDcb((PDCB) Dcb, DCB_type_disk);
	}
	VxdDebugPrint(D_PARTITION, "PartitionInitialise: done, nrIde=%i, nrScsi=%i", nrIde, nrScsi);

	PrintPartitionTable();
}


void PartitionGetDevice(void *Dcb, int *Disk, int *Partition)
{
	int						i;
 	TPartitionTableEntry	*Entry;
	
	*Disk = 0;
	*Partition = 0;

	for (i=0, Entry=sPartitionTable ; i<PART_TABLE_ENRTIES ; i++, Entry++)
	{
		if (Entry->Dcb == Dcb)
		{
			*Disk =i / MAX_PARTITION_PER_DISK;
			*Partition =i % MAX_PARTITION_PER_DISK;
		}
	}
}


