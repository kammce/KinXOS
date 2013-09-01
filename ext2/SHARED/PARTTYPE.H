#ifndef __PARTTYPE_H
#define __PARTTYPE_H

#define TYPE_EXTENDED_PARTITION 5
#define TYPE_LINUX_PARTITION 131


#define MAX_IDE_DISK 4
#define MAX_SCSI_DISK 8
#define MAX_PARTITION_PER_DISK	16

#define PART_TABLE_ENRTIES (MAX_PARTITION_PER_DISK * (MAX_IDE_DISK + MAX_SCSI_DISK))

#pragma pack(1)
typedef struct 
{
	void			*Dcb;
	unsigned long	nrSectors;
	unsigned long	StartSector;
	unsigned char	PartitionType;
	unsigned char	Drive;
} TPartitionTableEntry;
#pragma pack()



#endif			/* __PARTTYPE_H */