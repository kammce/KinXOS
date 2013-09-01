#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "shared\fsdioctl.h"
#include "fsdif.h"

/**********************************
 *
 * STATIC DATA
 *
 **********************************/

static char		*sVxdName = "\\\\.\\VEXT2D.VXD";

/**********************************
 *
 * STATIC HELPERS
 *
 **********************************/



/**********************************
 *
 * INTERFACE ROUTINES
 *
 **********************************/

BOOL FsdGetPartitionTable(void *Table, int Size)
{
	DWORD		BytesReturned;
	HANDLE		hVxd;
			
	hVxd = CreateFile(sVxdName, 0, 0, 0, 0, OPEN_EXISTING, 0);
	if (hVxd == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Error: cannot load %s, error=%08lx\n",
				sVxdName, GetLastError());
	 	return FALSE;
	}

	if (!DeviceIoControl(hVxd, FSDIOCTL_GETPARTITIONTABLE,
               0, 0, Table, Size,
               &BytesReturned, NULL))
	{
		fprintf(stderr, "Error: cannot ioctl to %s, error=%08lx\n",
				sVxdName, GetLastError());
	 	return FALSE;
	}

	CloseHandle(hVxd);

	return TRUE;
}


ULONG FsdMount(int Disk, int Partition, int Drive)
{
	DWORD			BytesReturned;
	TMountRequest	MountRequest;
	ULONG			Reply;
	HANDLE			hVxd;
			
	hVxd = CreateFile(sVxdName, 0, 0, 0, 0, 0, 0);
	if (hVxd == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Error: cannot load %s, error=%08lx\n",
				sVxdName, GetLastError());
	 	return FALSE;
	}

	MountRequest.Disk = Disk;
	MountRequest.Partition = Partition;
	MountRequest.Drive = Drive;
	
	if (!DeviceIoControl(hVxd, FSDIOCTL_MOUNT,
               &MountRequest, sizeof(MountRequest), &Reply, 
			   sizeof(Reply), &BytesReturned, NULL))
	{
		fprintf(stderr, "Error: cannot ioctl to %s, error=%08lx\n",
				sVxdName, GetLastError());
	 	return FALSE;
	}

	CloseHandle(hVxd);

	return Reply;
}


int FsdGetOrSetDebug(int DebugLevel)
{
	DWORD			BytesReturned;
	int				newDebugLevel;
	HANDLE			hVxd;
			
	hVxd = CreateFile(sVxdName, 0, 0, 0, 0, 0, 0);
	if (hVxd == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Error: cannot load %s, error=%08lx\n",
				sVxdName, GetLastError());
	 	return FALSE;
	}

	if (!DeviceIoControl(hVxd, FSDIOCTL_DEGBUG_INFO,
               &DebugLevel, sizeof(int), &newDebugLevel, 
			   sizeof(int), &BytesReturned, NULL))
	{
		fprintf(stderr, "Error: cannot ioctl to %s, error=%08lx\n",
				sVxdName, GetLastError());
	 	return FALSE;
	}

	CloseHandle(hVxd);

	return newDebugLevel;
}


ULONG FsdUMount(int Drive)
{
	DWORD			BytesReturned;
	ULONG			Reply;
	HANDLE			hVxd;
			
	hVxd = CreateFile(sVxdName, 0, 0, 0, 0, 0, 0);
	if (hVxd == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Error: cannot load %s, error=%08lx\n",
				sVxdName, GetLastError());
	 	return FALSE;
	}

	if (!DeviceIoControl(hVxd, FSDIOCTL_MOUNT,
               &Drive, sizeof(Drive), &Reply, 
			   sizeof(Reply), &BytesReturned, NULL))
	{
		fprintf(stderr, "Error: cannot ioctl to %s, error=%08lx\n",
				sVxdName, GetLastError());
	 	return FALSE;
	}

	CloseHandle(hVxd);

	return Reply;
}


void FsdCacheInfo()
{
	DWORD			BytesReturned;
	ULONG			Reply;
	HANDLE			hVxd;
			
	hVxd = CreateFile(sVxdName, 0, 0, 0, 0, 0, 0);
	if (hVxd == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Error: cannot load %s, error=%08lx\n",
				sVxdName, GetLastError());
	 	return;
	}

	if (!DeviceIoControl(hVxd, FSDIOCTL_CACHE_INFO,
               0, 0, &Reply, 
			   sizeof(Reply), &BytesReturned, NULL))
	{
		fprintf(stderr, "Error: cannot ioctl to %s, error=%08lx\n",
				sVxdName, GetLastError());
	 	return;
	}

	CloseHandle(hVxd);
}
