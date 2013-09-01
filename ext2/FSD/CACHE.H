#ifndef __CACHE_H
#define __CACHE_H

typedef struct _Cache TCache;

/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/


TCache*	CacheCreate(const char *CacheName, unsigned NrObjects, unsigned ObjectDataSize);
void	CacheDestroy(TCache *Cache);
BOOL	CacheLookupById(TCache *Cache, unsigned long MediumId, unsigned long ObjectId, void *ObjectData);
BOOL	CacheLookupByName(TCache *Cache, unsigned long MediumId, unsigned long ObjectId, const char *ObjectName, unsigned NameLen, void *ObjectData);
void	CacheAddById(TCache *Cache, unsigned long MediumId, unsigned long ObjectId, void *ObjectData);
void	CacheAddByName(TCache *Cache, unsigned long MediumId, unsigned long ObjectId, char *ObjectName, unsigned ObjectNameSize, void *ObjectData);

void	CacheGetStats(TCache *Cache, unsigned long *Lookups, unsigned long *Hits);

#endif			/* __CACHE_H */
