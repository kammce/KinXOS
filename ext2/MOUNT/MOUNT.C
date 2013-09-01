#include <windows.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>

#include "fsdif.h"
#include "shared\parttype.h"
#include "shared\e2version.h"

#define FSD
#include "shared\debug.h"
#undef FSD

#define HEX2INT(c)	( isdigit((c)) ? (c) - '0' : (c) - 'a' + 10 )


#define USAGE	"Usage: MOUNT [[devicename] driverletter] [options]\n"\
				"options:\n"\
				"\t/d:           get current debug level\n"\
				"\t/d=value:     set current debug level\n"\
				"\t/?:           show usage\n"

	/*
	 * These come from vxddebug.c Keep them inline please!
	 */
static char		*sDebugLevelString[] = 
		{
			"",
			"Error:    error messages",
			"System:   informational messages",
			"Block:    diskblock management messages",
			"Blkdev:   messages related to reading/writing sectors/blocks from disk",
			"Part:     partition handling messages",
			"Cache:    cache handling messages",
			"Super:    superblock management messages",
			"File:     filehandle based systemcall messages",
			"Volume:   volume based systemcall messages",
			"Inode:    inode management messages",
			"Dir:      directory entry messages",
			"Vxd:      messages related to VxD stuff", 
			"Mount:    mount requests",
			"Ext2:     ext2fs specific messages"
		};



/**********************************
 *
 * STATIC DATA
 *
 **********************************/

static TPartitionTableEntry		sPartitionTable[PART_TABLE_ENRTIES];
static ULONG					sMaxMountErrNo = 3; 
static char						*sMountErrStr[] = 
		{
			"unknown and invalid error code!",
			"drive letter already in use",
			"device already mounted",
			"device is not a linux partition"
		};


static	int sFileSysTypeList[] = 
		{
			1, 4, 5, 6, 7, 0x0A, 0x51, 0x64, 
			0x80, 0x81, 0x82, 0x83, 0x93, 0x94, 0
		};

static char	*sFileSysNameList[] = 
		{
			"DOS 12-bit FAT", "DOS 16-bit <32M", "Extended",
			"DOS 16-bit >=32", "OS/2 HPFS", "OS/2 Boot Manag",
			"Novell?", "Novell", "Old MINIX", "Linux/MINIX",
			"Linux swap", "Linux native", "Amoeba", "Amoeba BBT",
			"Unknown"
		};



/**********************************
 *
 * STATIC HELPERS
 *
 **********************************/

static char *FindFileSysType(int Type)
{
	int i;

	for(i=0; sFileSysTypeList[i] && sFileSysTypeList[i]!=Type ; i++)
		;
	return sFileSysNameList[i];
}

static void PrintDebugLevels(int DebugLevel)
{
	int		i, j;

	if (DebugLevel == -1)
	{
		printf("This is not the debug version of fsdext2\n");
		return;
	}
	else
	{
		for (i=1,j=1 ; i<=D_END ; i<<=1, j++)
		{
			printf("%c %6i %s\n", 
					(DebugLevel & i) ? '*' : ' ',
					i,
					sDebugLevelString[j]);
		}
	}
}


/*
 * Parse a string to integer, allow
 * s = "all"	--> D_ALL
 * s = "0x12f"	--> hex value
 * s = "123"	--> decimal value
 * 
 */
static int String2Int(char *s)
{
	int		value = 0;

	if (strcmp(s, "all") == 0)
		value = D_ALL;
	else if (strcmp(s, "def") == 0)
		value = D_FILE | D_VOLUME | D_ERROR | D_DIR;
	else if (strncmp(s, "0x", 2) == 0)
	{
		s += 2;
		while (*s)
		{
			if (isxdigit(*s))
			{
				value <<= 4;
				value |= HEX2INT( tolower(*s) );
				s++;
			}
			else
				return -1;
		}
	} else
		value = atoi(s);

	return value;
}

static void ExitInvalidParam(char * param)
{
	fprintf(stderr, "Error, invalid/unknown parameter:%s\n\n", param);

	printf("%s", USAGE);

	exit(-1);
}


static void MountDevice(char *DeviceName, char DriveLetter)
{
	int		Disk, PartitionNo, DiskOffset, CurDrive;
	ULONG	Error;
	
		/*
		 * First translate devicename into
		 * Disk + PartitioNo
		 */
	Disk = 0;
	strupr(DeviceName);
	if (strncmp(DeviceName, "/DEV/", 5) != 0)
	{
		fprintf(stderr, "invalid partition name\n");
		return;
	}
	if (DeviceName[5] == 'S')
		Disk += MAX_IDE_DISK;
	else if (DeviceName[5] != 'H')
	{
		fprintf(stderr, "invalid partition name\n");
		return;
	}

	if (DeviceName[6] != 'D')
	{
		fprintf(stderr, "invalid partition name\n");
		return;
	}
	DiskOffset = DeviceName[7] - 'A';
	if (DiskOffset < 0 || 
		(Disk == 0 && DiskOffset > MAX_IDE_DISK) ||
		(DiskOffset > MAX_SCSI_DISK))
	{
		fprintf(stderr, "invalid partition name\n");
		return;
	}
	Disk += DiskOffset;
			
	PartitionNo = atoi(&DeviceName[8]);
	if (PartitionNo <= 0 || PartitionNo > MAX_PARTITION_PER_DISK)
	{
		fprintf(stderr, "invalid partition name\n");
		return;
	}

		/*
		 * Determine the disk to mount on
		 */
	DriveLetter = toupper(DriveLetter) - 'A';
	if (DriveLetter < 0 || DriveLetter > 25)
	{
		fprintf(stderr, "invalid driveletter\n");
		return;
	}

		/*
		 * Hmm, seems that our FSD cannot properly detect 
		 * that a drive letter is in use, so let's do it here
		 */
	CurDrive = _getdrive();
	if (_chdrive(DriveLetter + 'A' + 1) == 0)
	{
		_chdrive(CurDrive);
		fprintf(stderr, "drive already exists\n");
		return;
	}
	_chdrive(CurDrive);


		/*
		 * Mount it....
		 */
	if ((Error = FsdMount(Disk, PartitionNo, DriveLetter)))
	{
		if (Error > sMaxMountErrNo)
			fprintf(stderr, "%s (Error=%i)\n", sMountErrStr[0], Error);
		else
			fprintf(stderr, "Error, %s\n", sMountErrStr[Error]);
	}
}

static void SetDebug(int DebugLevel)
{
	int		newLevel;

	newLevel = FsdGetOrSetDebug(DebugLevel);
	if (newLevel != DebugLevel)
		PrintDebugLevels(newLevel);
}

static void GetDebug()
{
	int		DebugLevel;

	DebugLevel = FsdGetOrSetDebug(-1);
		PrintDebugLevels(DebugLevel);
}



static void PrintPartitionTable()
{
	int						Disk, PartitionNo;
	TPartitionTableEntry	*Partition;

	if (FsdGetPartitionTable(sPartitionTable, sizeof(sPartitionTable)))
	{
		printf("****** PARTITION TABLE ******\n");

		for (Disk=0 ; Disk < MAX_IDE_DISK + MAX_SCSI_DISK ; Disk++)
		{
			for (PartitionNo=0 ; PartitionNo<MAX_PARTITION_PER_DISK ; PartitionNo++)
			{
				Partition = &sPartitionTable[Disk*MAX_PARTITION_PER_DISK + PartitionNo];
				if (Partition->nrSectors && Partition->StartSector)
				{
					printf("/dev/%s%c%i, %4lu MB, type=%2Xh   (%s) %s\n",
						Disk < MAX_IDE_DISK ? "hd" : "sd",
						Disk < MAX_IDE_DISK ? Disk + 'a' : Disk-MAX_IDE_DISK + 'a',
						PartitionNo,
						(ULONG) Partition->nrSectors >> 11,
						(int) Partition->PartitionType,
						FindFileSysType((int) Partition->PartitionType),
						(!(Partition->Dcb) && Partition->PartitionType == TYPE_LINUX_PARTITION) ? "(not recognised by fsdext2)" : ""
						);
				}
			}
		}
		printf("****** PARTITION TABLE ******\n");
	}
	else
		fprintf(stderr, "Could not print partition table");
}


static void PrintBanner()
{
	printf("Ext2 file system mount for Windows 95\n");
	printf("Version %s by Peter van Sebille\n\n", VERSION_STRING);
}


/**********************************
 *
 * MAIN
 *
 **********************************/


/*
 * MOUNT [[devicename] driverletter] [options]
 *	options:
 *		/sp				: show partition tables
 *		/?				: show usage
 *
 * f.e. mount hda1 w
 */

int main(int argc, char *argv[])
{
	int			i;
	int			showusage = 0;
	int			showdebug = 0;
	int			setdebug = -1;
	char		*param;
	char		*devicename = 0;
	char		driveletter = 0;

	PrintBanner();

/******************************
 *
 * No options, only print partition table
 *
 ******************************/
	if (argc == 1)
	{
		PrintPartitionTable();
		return 0;
	}



/******************************
 *
 * Scan optionlist and set some locals
 *
 ******************************/
	for (i=1 ; i<argc ; i++)
	{
		param = argv[i];
		switch(param[0])
		{
			case '/':
				{
					switch(param[1])
					{
						case '?':
							showusage = 1;
							break;

						case 'd':
							if (param[2] == 0)
								showdebug = 1;
							else
							{
								if (param[2] != '=')
									devicename = param;
								else
								{
									setdebug = String2Int(param + 3);
									if (setdebug == -1)
										ExitInvalidParam(param);
								}
							}
							break;
							
						default:
							ExitInvalidParam(param);
					}
				}
				break;
			
			default:
					/*
					 * if it's not an option, it is either
					 * the device name, or the drive letter
					 */
				if (param[1] == 0)
					driveletter = param[0];
				else
					ExitInvalidParam(param);
		}
	}

/******************************
 *
 * Based on the locals, do it
 *
 ******************************/
	if (showusage)
	{
			/*
			 * print usage, terminate
			 */
		printf("%s", USAGE);
		exit(0);
	}

	if (showdebug)
	{
			/*
			 * show debug level, terminate
			 */
		GetDebug();
		exit(0);
	}

	if (setdebug != -1)
	{
			/*
			 * set debug level, don't terminate to see if we need
			 * to mount as well
			 */
		SetDebug(setdebug);
	}
			
		/*
		 * only mount if both the name and a driver letter
		 * are specified
		 */
	if (devicename && driveletter)
	{
		MountDevice(devicename, driveletter);
		return 0;
	}
	
	if ((devicename && !driveletter) || (driveletter && !devicename))
	{
		fprintf(stderr, "Error, %s\n\n", devicename ? "no drive letter" : "no devicename");
		printf("%s", USAGE);
	}
}
