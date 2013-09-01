#ifndef __PARTTBL_H
#define __PARTTBL_H

#include "shared\parttype.h"

/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/

void	PartitionInitialise();
void	PartitionGetTable(char **Table, DWORD *Size);
BOOL	PartitionMount(int Disk, int Partition, int Drive, ULONG *Reply);
BOOL	PartitionUMount(int Drive);
void	PartitionGetDevice(void *Dcb, int *Disk, int *Partition);

#endif			/* __PARTTBL_H */