#ifndef __PARTWALK_H
#define __PARTWALK_H


typedef struct 
{
	unsigned char 	boot_ind;		/* 0x80 - active */
	unsigned char 	head;			/* starting head */
	unsigned char 	sector;			/* starting sector */
	unsigned char 	cyl;			/* starting cylinder */
	unsigned char 	sys_ind;		/* What partition type */
	unsigned char 	end_head;		/* end head */
	unsigned char 	end_sector;		/* end sector */
	unsigned char 	end_cyl;		/* end cylinder */
	unsigned long 	start_sec;		/* starting sector counting from 0 */
	unsigned long 	nr_sec;			/* nr of sectors in partition */
} TDiskPartition;

typedef void (*TPartitionFunc)(PDCB Dcb, int Disk, TDiskPartition *Partition, int PartitionNo);


/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/

void WalkPartitions(PDCB Dcb, int Disk, TPartitionFunc CallBack);


#endif			/* __PARTWALK_H */
