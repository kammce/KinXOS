#ifndef __FSD_H
#define __FSD_H


BOOL	FsdGetPartitionTable(void *Table, int Size);
ULONG	FsdMount(int Disk, int Partition, int Drive);
ULONG	FsdUMount(int Drive);
void	FsdCacheInfo();
int		FsdGetOrSetDebug(int DebugLevel);

#endif