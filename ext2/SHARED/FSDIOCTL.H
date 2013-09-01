#ifndef __FSDIOCTL_H
#define __FSDIOCTL_H

#define FSDIOCTL_GETPARTITIONTABLE	0x8001
#define FSDIOCTL_MOUNT				0x8002
#define FSDIOCTL_UMOUNT				0x8003
#define FSDIOCTL_CACHE_INFO			0x8004
#define FSDIOCTL_DEGBUG_INFO		0x8010

#pragma pack(1)
typedef struct
{
	int		Disk;
	int		Partition;
	int		Drive;
} TMountRequest;
#pragma pack()


#define	ERROR_MOUNT_DRIVE_ALREADY_EXISTS		0x01
#define ERROR_MOUNT_DEVICE_ALREADY_MOUNTED		0x02
#define ERROR_MOUNT_DEVICE_NOT_LINUX			0x03


#endif			/* __FSDIOCTL_H */