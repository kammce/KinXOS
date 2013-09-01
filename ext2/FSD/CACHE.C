#include "vxd.h"

#include "shared\vxddebug.h"
#include "cache.h"



/*
 * This module implements a cache mechansism used for directory
 * entries and diskblocks.
 *
 * There are two flavours of the Cache:
 *	1.	dir. entry caches: Stores the inode number of dir. entries
 *		It is a two-level cache to prevent flushing when reading
 *		"uninterresting" items. An item is lifted from level 1 to 2
 *		after a cache-hit. The Cache allocates the space to store
 *		the cached inode numbers itself.
 *		Also, each object is looked by Id (actually two Ids: the 
 *		MediumId and the ObjectId) and by name. 
 *
 *	2.	diskblock caches: The cache will not store copies of
 *		disk blocks but, instead, it stores the pointers only.
 *		This implies that a client of such a TCache object is
 *		responsible for allocating space for the object.
 *		When a disk block is successfully looked up, the 
 *		pointer to the disk block's buffer is returned; also
 *		the cache object itself is removed from the cache.
 *		This is a _must_ because when other cache objects are
 *		added, stored cache objects are removed (last recently 
 *		used first). If so, the data (i.e., the disk block's 
 *		buffer) is freed. 
 *		Diskblock caches only have one level and they lookup
 *		a block by Id only (again, actually two IDs)
 *
 * TCache objects are thread-safe as they use a Mutex to
 * synchronise update to internal data structures.
 *
 */

#define HASH_QUEUE_SIZE 4
#define LEVEL1 '\1'
#define LEVEL2 '\2'

#define MAX_CACHE_NAME		16
#define MAX_OBJECT_NAME		16

/**********************************
 *
 * PRIVATE TYPEDEFS
 *
 **********************************/


typedef struct _CacheObject
{
	unsigned long		MediumId;
	unsigned long		ObjectId;
	char				ObjectName[MAX_OBJECT_NAME];
	unsigned			NameLen;

	void				*ObjectData;

	unsigned short		HashIndex;
	unsigned char		Level;

	struct _CacheObject *LruNext;
	struct _CacheObject *LruPrev;
	struct _CacheObject	*HashNext;
	struct _CacheObject	*HashPrev;
} TCacheObject;


typedef struct _Cache
{
	TCacheObject		**HashQueue;
	TCacheObject		*LruHead[2];
	TCacheObject		*CacheObjectList;
	
	void				*ObjectDataSpace;
	
	UINT				HashSize;
	UINT				nrObjects;
	UINT				ObjectDataSize;

	char				Name[MAX_CACHE_NAME];
	ULONG				nrLookups;
	ULONG				nrHits;

	MUTEXHANDLE			Mutex;
} TCache;


/**********************************
 *
 * GLOBAL DATA
 *
 **********************************/

// none


/**********************************
 *
 * STATIC DATA
 *
 **********************************/

// none


/************************************************
 *
 * STATIC HELPERS
 *
 * All static helpers assume that access to the
 * Cache's internal datastructures is protected
 * by a synchronisation object
 *
 *************************************************/


/*
 * Simple hash function
 */
static __inline unsigned short HashFunction(unsigned MediumId, unsigned long ObjectId, const char *ObjectName, unsigned NameLen, unsigned HashSize)
{
	return 	(ObjectName) ?
		//(unsigned short) (MediumId ^ ObjectId ^ ( ((unsigned long) NameLen) * *((unsigned char *) ObjectName))
		(unsigned short) ((MediumId ^ ObjectId ^ ((unsigned long) NameLen))
			% (unsigned long) HashSize)
		:
		(unsigned short) ((MediumId ^ ObjectId) % (unsigned long) HashSize);
}



static void LruRemove(TCache *Cache, TCacheObject *Object)
{
	TCacheObject **Lru = &(Cache->LruHead[Object->Level-1]);
	
		/*
		 *  remove object, special care if were are the LruHead
		 */
	if (Object == (*Lru))
		(*Lru) = (*Lru)->LruNext;
	Object->LruPrev->LruNext = Object->LruNext;
	Object->LruNext->LruPrev = Object->LruPrev;
}


static void LruPutBefore(TCacheObject *Object1, TCacheObject *Object2)
{

		/*
		 * insert Object1 before Object2
		 */
	Object1->LruNext = Object2;
	Object1->LruPrev = Object2->LruPrev;
	Object2->LruPrev->LruNext = Object1;
	Object2->LruPrev = Object1;
}


static void HashRemove(TCacheObject **HashQueue, TCacheObject *Object)
{
		/*
		 * First see if we are hashed
		 */
	if (Object->HashIndex == 0xFFFFu)
		return;

	if (Object->HashNext == Object)
	{

			/* 
			 * removal of last object 
			 */
		HashQueue[Object->HashIndex] = 0;
	}
	else
	{
		Object->HashNext->HashPrev = Object->HashPrev;
		Object->HashPrev->HashNext = Object->HashNext;

			/* 
			 * ensure that that queue points to something 
			 */
		HashQueue[Object->HashIndex] = Object->HashNext;
	}
	Object->HashNext = 0;
	Object->HashPrev = 0;
	Object->HashIndex = 0xFFFFu;
}


static void HashAdd(TCacheObject **HashQueue, TCacheObject *Object)
{
	TCacheObject	*HashFirst;
	

		/*
		 * Get beginning of hash queue
		 */
	HashFirst = HashQueue[Object->HashIndex];

	if (HashFirst)
	{
			/*
			 * For non-empty list position us as the first item
			 */
		Object->HashNext = HashFirst;
		Object->HashPrev = HashFirst->HashPrev;
		HashFirst->HashPrev->HashNext = Object;
		HashFirst->HashPrev = Object;
	}
	else
	{
			/*
			 * We are the only item
			 */
		Object->HashNext = Object;
		Object->HashPrev = Object;
	}

	HashQueue[Object->HashIndex] = Object;
}



static void CacheUpdate(TCache *Cache, TCacheObject *Object)
{

		/*
		 * We have an object, put it last in the list (ie the most recent one)
		 */
	LruRemove(Cache, Object);
	LruPutBefore(Object, Cache->LruHead[Object->Level-1]);
}


static void CacheLiftLevel(TCache *Cache, TCacheObject *Object)
{
	TCacheObject 	*LruHead2 = Cache->LruHead[1];

		/*
		 * We replace 'object' from level 1 with the lastest used object from level2
		 * because we made 'object' the most recent in level 1, the level2_object
		 * will now be the most recent object in level 1 (seems fair enough)
		 *
		 * Finally we mark 'object' in level 2 as the most recent one
		 */

		/*
		 * first remove them
		 */
	LruRemove(Cache, LruHead2);
	LruRemove(Cache, Object);

		/*
		 * switch level
		 */
	LruHead2->Level = LEVEL1;
	Object->Level = LEVEL2;

		/*
		 * put them in the right position, that is, most recently used
		 */
	LruPutBefore(LruHead2, Cache->LruHead[0]);
	LruPutBefore(Object, Cache->LruHead[1]);
}



/*
 * Lookup item by Id and possibly by name
 *
 */
static BOOL CacheLookupByNameOrId(TCache *Cache, unsigned long MediumId, unsigned long ObjectId, const char *ObjectName, unsigned NameLen, void *ObjectData)
{
	TCacheObject		*CacheObject, *HashFirst;
	unsigned short		HashIndex;

	VxdDebugPrint(D_CACHE, "CacheLookup: cache=%s, medium=%lu, id=%lu, len=%u", 
			Cache->Name, 
			MediumId, 
			ObjectId,
			NameLen);

	Cache->nrLookups++;

		/*
		 * Find the object in the hash_queue, if the list is
		 * empty, the item is not there
		 */
	HashIndex = HashFunction(MediumId, ObjectId, ObjectName, NameLen, Cache->HashSize);

	HashFirst = Cache->HashQueue[HashIndex];
	if (!HashFirst)
	{
		VxdDebugPrint(D_CACHE, "CacheLookup: miss, hash chain empty");
		return FALSE;
	}

		/*
		 * itterate over the list to see if it's there
		 */
	CacheObject = HashFirst;
	do
	{
			/*
			 * Let's see if we match.
			 * first, MediumId and ObjectId must match
			 * next, if we looking on ObjectName as well, compare the object's name
			 */
		if (! ((CacheObject->MediumId == MediumId) && (CacheObject->ObjectId == ObjectId)))
			continue;
		if (ObjectName)
			if (! ((CacheObject->NameLen==NameLen) && (memcmp(ObjectName, CacheObject->ObjectName, CacheObject->NameLen)==0))) 
				continue;
			
			/*
			 * Match found !
			 */
		VxdDebugPrint(D_CACHE, "CacheLookup: done, lookup hit");
		Cache->nrHits++;
		
			/*
			 * For the block cache we remove the item.
			 * For the dir. entry cache we levellift id needed
			 * and mark the item the most recent one.
			 */
		if (Cache->ObjectDataSize)
		{
				/*
				 * if the item is in level 1 cache, lift it to level 2
				 * otherwise, make it the most recent one
				 */
			if (CacheObject->Level == LEVEL1)
				CacheLiftLevel(Cache, CacheObject);
			else
				CacheUpdate(Cache, CacheObject);
			memcpy(ObjectData, CacheObject->ObjectData, Cache->ObjectDataSize);
		}
		else
		{
				/*
				 * Remove the item from the hash list (it will
				 * never be found again)
				 * Also, make this the least recently used (it
				 * will be the first one reused when a new item 
				 * is added to the cache is full)
				 */
			HashRemove(Cache->HashQueue, CacheObject);
			CacheUpdate(Cache, CacheObject);		// at the end
			Cache->LruHead[0] = CacheObject;		// at the beginning
			* ((char **) ObjectData) = (char *) CacheObject->ObjectData;
			CacheObject->ObjectData = 0;
		}

		return TRUE;

	} while ((CacheObject = CacheObject->HashNext) != HashFirst);

	VxdDebugPrint(D_CACHE, "CacheLookup: done, lookup miss");

	return FALSE;
}


/*
 * Add an item to the cache, replacing the least recently used.
 * It is made the most recent in level 1.
 *
 * PRE CONDITION:
 *	NameLen <= MAX_OBJECT_NAME
 */
static void CacheAddByNameOrId(TCache *Cache, unsigned long MediumId, unsigned long ObjectId, char *ObjectName, unsigned NameLen, void *ObjectData)
{
	unsigned short		NewHashIndex;
	TCacheObject*		CacheObject;

	VxdDebugPrint(D_CACHE, "CacheAdd: cache=%s, medium=%u, id=%u, namelen=%u", 
		Cache->Name, 
		MediumId, 
		ObjectId, 
		NameLen);

		/*
		 * Get the least recently used
		 */
	CacheObject = Cache->LruHead[0];
	
		/*
		 * Set the new memebers
		 */
	CacheObject->MediumId = MediumId;
	CacheObject->ObjectId = ObjectId;
	if (ObjectName)
	{
		memcpy(CacheObject->ObjectName, ObjectName, NameLen);
		CacheObject->NameLen = NameLen;
	}

		/*
		 * If the new object has a different hash value than the object
		 * we are replacing, update the hash links
		 */
	NewHashIndex = 	HashFunction(MediumId, ObjectId, ObjectName, NameLen, Cache->HashSize);
	if (NewHashIndex != CacheObject->HashIndex)
	{
		HashRemove(Cache->HashQueue, CacheObject);
		CacheObject->HashIndex = NewHashIndex;
		HashAdd(Cache->HashQueue, CacheObject);
	}

		/*
		 * Make the new object the most recent one, by pointing
		 * the head of the list to the next item (so the object
		 * is now at the end of the list.
		 */
	Cache->LruHead[0] = CacheObject->LruNext;
	
		/*
		 * If we are a diskblock cache, check if
		 * the object we are removing must be deleted.
		 */
	if (Cache->ObjectDataSize == 0)
	{
		if (CacheObject->ObjectData)
		{
			VxdDebugPrint(D_CACHE, "CacheAddByNameOrId: flushing %X", CacheObject->ObjectData);
			free(CacheObject->ObjectData);
		}
		CacheObject->ObjectData = ObjectData;
		VxdDebugPrint(D_CACHE, "CacheAddByNameOrId: adding %X", CacheObject->ObjectData);
	}
	else
		memcpy(CacheObject->ObjectData, ObjectData, Cache->ObjectDataSize);

	VxdDebugPrint(D_CACHE, "CacheAdd: done");
}





/**********************************
 *
 * INTERFACE ROUTINES
 *
 **********************************/



/* 
 * Create a TCache object. The Cache can either be a dir. entry
 * cache or a diskblock cache. Cache object can be stored 
 * by either object Id or by object Id and Object Name. 
 * In both ways, a Medium id on which the object resides can be 
 * specified. For dir. entry caches, the cache allocates the 
 * memory to store the copies of the objects. For diskblock
 * caches, the client is responsible for the object's data memory.
 *
 * PRE CONDITIONS
 *	<none>
 * POST CONDITIONS
 *	Cache object created and initialised. If cache is a dir. entry
 *	cache, space for these objects is allocated.
 *
 * IN
 *	CacheName:	A descriptive name, used in debug messages (only 
 *				first 16 charecters are used)
 *	NrObjects:	Number of Objects to store in the cache
 *	DataSize:	If the cache should only conain dir. entrys,
 *				specify the size, the cache will allocate the space
 *				for all objects at once. Specify 0 if the cache
 *				contains diskblocks.
 * OUT:
 *	<none>
 *
 * RETURN VALUE:
 *	On success a valid pointer to an TCache object, otherwise 0
 */ 
TCache* CacheCreate(const char *CacheName, unsigned NrObjects, unsigned ObjectDataSize)
{
	TCache				*Cache;
	TCacheObject		*CacheObject;
	unsigned			i, HalfWay;


	VxdDebugPrint(D_SYSTEM, "CacheCreate: cache=%s, nr=%u, size=%u", CacheName, NrObjects, ObjectDataSize);

		/* 
		 * Alloc cache object
		 */
	if (! (Cache=(TCache *) calloc(1, sizeof(TCache))))
	{
		VxdDebugPrint(D_ERROR, "CacheCreate: could not alloc cache");
		goto error_init_cache_1;
	}

		/*
		 * Initialise field members
		 */
	Cache->nrObjects = NrObjects;
	Cache->ObjectDataSize = ObjectDataSize;
	strncpy(Cache->Name, CacheName, MAX_CACHE_NAME-1);
	Cache->Name[MAX_CACHE_NAME-1] = 0;

		/*
		 * For dir. entry caches, allocate space 
		 * to store the data belonging to the objects
		 */
	if (ObjectDataSize)
	{
		if ( !(Cache->ObjectDataSpace = malloc(ObjectDataSize * NrObjects)))
		{
			VxdDebugPrint(D_ERROR, "CacheCreate: could not alloc space for dir. entry cache ");
			goto error_init_cache_2;
		}
	}

		/*
		 * Allocate the cache object list
		 */
	if ( !(Cache->CacheObjectList = (TCacheObject*) calloc(NrObjects, sizeof(TCacheObject))))
	{
		VxdDebugPrint(D_ERROR, "CacheCreate: could not alloc cache entries");
		goto error_init_cache_3;
	}

		/*
		 * For dir. entry cache we have two levels.
		 * For disk block reference we only have one level
		 */
	if (ObjectDataSize)
	{
			/*
			 * Setup the two LRU heads
			 */
		Cache->LruHead[0] = Cache->CacheObjectList;
		HalfWay = NrObjects / 2;
		Cache->LruHead[1] = Cache->LruHead[0] + HalfWay;

			/*
			 * Setup the level 1 cache
			 */
		CacheObject = Cache->LruHead[0];
		for (i=0; i<HalfWay ; i++, CacheObject++)
		{
			CacheObject->HashNext = CacheObject->HashPrev = 0;
			CacheObject->LruNext = CacheObject+1;
			CacheObject->LruPrev = CacheObject-1;
			CacheObject->Level = LEVEL1;
			CacheObject->HashIndex = 0xFFFFu;
			CacheObject->ObjectData = ((char *) Cache->ObjectDataSpace) + ObjectDataSize * i;
		}
			/*
			 * Fix first+last entries to create circular list
			 */
		Cache->LruHead[0]->LruPrev = Cache->LruHead[1]-1;
		Cache->CacheObjectList[HalfWay - 1].LruNext = Cache->LruHead[0];


			/*
			 * Setup the level 2 cache (dir. entry cache only)
			 */
		CacheObject = Cache->LruHead[1];
		for(i=HalfWay ; i<NrObjects ; i++, CacheObject++)
		{
			CacheObject->HashNext = CacheObject->HashPrev = 0;
			CacheObject->LruNext = CacheObject+1;
			CacheObject->LruPrev = CacheObject-1;
			CacheObject->Level = LEVEL2;
			CacheObject->HashIndex = 0xFFFFu;
			CacheObject->ObjectData = ((char *) Cache->ObjectDataSpace) + ObjectDataSize * i;
		}
			/*
			 * Fix first+last entries to create circular list
			 */
		Cache->LruHead[1]->LruPrev = &Cache->CacheObjectList[NrObjects -1];
		Cache->CacheObjectList[NrObjects -1].LruNext = Cache->LruHead[1];
	}
	else
	{
			/*
			 * Setup the level 1 cache only
			 */

		Cache->LruHead[0] = Cache->CacheObjectList;
		Cache->LruHead[1] = 0;
		CacheObject = Cache->LruHead[0];
		for (i=0; i<NrObjects ; i++,CacheObject++)
		{
			CacheObject->HashNext = CacheObject->HashPrev = 0;
			CacheObject->LruNext = CacheObject+1;
			CacheObject->LruPrev = CacheObject-1;
			CacheObject->Level = LEVEL1;
			CacheObject->HashIndex = 0xFFFFu;
		}
			/*
			 * Fix first+last entries to create circular list
			 */
		Cache->LruHead[0]->LruPrev =  &Cache->CacheObjectList[NrObjects -1];
		Cache->CacheObjectList[NrObjects -1].LruNext = Cache->LruHead[0];
	}

		/*
		 * alloc HashQueue, initialised with 0 pointers
		 */
	Cache->HashSize = NrObjects/HASH_QUEUE_SIZE;
	if ( !(Cache->HashQueue = (TCacheObject **) calloc(Cache->HashSize, sizeof(TCacheObject*))))
	{
		VxdDebugPrint(D_ERROR, "CacheCreate: could not alloc hash_queue");
		goto error_init_cache_4;
	}

		/*
		 * Create the Mutex, needed to provide for exclusive access
		 * when updating the cache internal datastructures
		 * There is no boost needed as we specify the MUTEX_MUST_COMPLETE
		 * flag. Since we typically run for short moments, it is
		 * not smart to let the thread preempted when the Mutex is 
		 * claimed
		 */
	Cache->Mutex = CreateMutex(0, MUTEX_MUST_COMPLETE);

	VxdDebugPrint(D_SYSTEM, "CacheCreate: done");
	
	return Cache;

error_init_cache_4:
	free(Cache->CacheObjectList);

error_init_cache_3:
	if (Cache->ObjectDataSpace)
		free(Cache->ObjectDataSpace);

error_init_cache_2:
	free(Cache);

error_init_cache_1:

	VxdDebugPrint(D_SYSTEM, "CacheCreate: done");

	return (TCache*) 0;
}


/*
 * Destroys a TCache object
 *
 * PRE CONDITIONS:
 *	There must be no more clients that use the TCache object
 *
 * POST CONDITIONS:
 *	Cache object and internal data structures are destroyed
 *
 * IN:
 *	Cache	: pointer to a previously created TCache object
 *
 * OUT:
 *	<none>
 *
 * RETURNS
 *	<none>
 */
void CacheDestroy(TCache *Cache)
{
	TCacheObject		*CacheObject;
	unsigned			i;

	VxdDebugPrint(D_SYSTEM, "CacheDestroy: cache=%s, lookups: %lu, hits:%lu",Cache->Name, Cache->nrLookups, Cache->nrHits);

	EnterMutex(Cache->Mutex, BLOCK_THREAD_IDLE);

		/*
		 * Free the data belonging to each cache object.
		 * For dir. entry caches, the cache holds the 
		 * buffer space for all these objects itself.
		 * For diskblock caches, the DestroyOnFlush
		 * flag specifies if the cache should delete the
		 * objects or not. If so, we traverse the objectlist 
		 * and delete each dataobject individually
		 */
	if (Cache->ObjectDataSpace)
		free (Cache->ObjectDataSpace);
	else
	{
		for (i=0, CacheObject=Cache->CacheObjectList; i<Cache->nrObjects ; i++,CacheObject++)
		{
			if (CacheObject->ObjectData)
				free (CacheObject->ObjectData);
		}
	}

	free(Cache->CacheObjectList);
	free(Cache->HashQueue);

		/*
		 * First, leave the Mutex (and destroy it) before
		 * deleting the TCache object itself
		 */
	LeaveMutex(Cache->Mutex);
	DestroyMutex(Cache->Mutex);

	free(Cache);
	
	VxdDebugPrint(D_SYSTEM, "CacheDestroy: done");
}



/*
 * Lookup an cache object by Id
 * 
 * PRE CONDITIONS:
 *	Cache object should be valid.
 *	ObjectData should be point to a buffer large enough to hold
 *	either the the data belonging to the cache object 
 *	(dir. entry cache) or the pointer to the data
 *	(diskblock cache)
 *
 * POST CONDITIONS:
 *	Dir entry cache:
 *	If the object being looked up for is in the cache, it is
 *	lifted to level 2 (if not already there). The found cache
 *	object is marked as the most recently used object. The data 
 *	belonging to the found cache item is copied to the caller's 
 *	buffer space pointed to by Data. 
 *	Diskblock caches:
 *	If the object being lookup for is in the cache, it is removed
 *	from the cache and the _pointer_ to the object's data is 
 *	copied to the buffer Data is pointing to.
 *
 * IN:
 *	Cache:		pointer to valid TCache object
 *	MediumId:	Id of the medium on which the objects reside
 *	ObjectId:	id of the object
 * 
 * OUT:
 *	ObjectData:	pointer to caller's buffer space
 *
 * RETURNS:
 *	TRUE:		when the object is found
 *	FALSE:		otherwise
 *
 */
BOOL CacheLookupById(TCache *Cache, unsigned long MediumId, unsigned long ObjectId, void *ObjectData)
{
	BOOL	Rval;

	EnterMutex(Cache->Mutex, BLOCK_THREAD_IDLE);

	Rval = CacheLookupByNameOrId(Cache, MediumId, ObjectId, 0, 0, ObjectData);

	LeaveMutex(Cache->Mutex);

	return Rval;
}



/*
 * Lookup an cache object by Id and by Name
 * 
 * PRE CONDITIONS:
 *	Cache object should be valid.
 *	ObjectData should be point to a buffer large enough to hold
 *	either the the data belonging to the cache object 
 *	(dir. entry cache) or the pointer to the data
 *	(diskblock cache)
 *
 * POST CONDITIONS:
 *	Dir entry cache:
 *	If the object being looked up for is in the cache, it is
 *	lifted to level 2 (if not already there). The found cache
 *	object is marked as the most recently used object. The data 
 *	belonging to the found cache item is copied to the caller's 
 *	buffer space pointed to by Data. 
 *	Diskblock caches:
 *	If the object being lookup for is in the cache, it is removed
 *	from the cache and the _pointer_ to the object's data is 
 *	copied to the buffer Data is pointing to.
 *
 *	If NameLen > 16, CacheLookupByName returns FALSE
 *
 * IN:
 *	Cache:		pointer to valid TCache object
 *	MediumId:	Id of the medium on which the objects reside
 *	ObjectId:	id of the object
 *	ObjectName:	Name of the object (need not be zero-terminated)
 *	NameLen:	Length of the object's name
 * 
 * OUT:
 *	ObjectData:	pointer to caller's buffer space
 *
 * RETURNS:
 *	TRUE:		when the object is found
 *	FALSE:		otherwise
 *
 */
BOOL CacheLookupByName(TCache *Cache, unsigned long MediumId, unsigned long ObjectId, const char *ObjectName, unsigned NameLen, void *ObjectData)
{
	BOOL	Rval;

		/*
		 * Long ObjectNames were just not cached :-)
		 */
	if (NameLen > MAX_OBJECT_NAME)
		return FALSE;

		/*
		 * Synchronised access to the Cache
		 */
	EnterMutex(Cache->Mutex, BLOCK_THREAD_IDLE);

	Rval = CacheLookupByNameOrId(Cache, MediumId, ObjectId, ObjectName, NameLen, ObjectData);

	LeaveMutex(Cache->Mutex);

	return Rval;
}


/*
 * Add an object to the cache
 * 
 * PRE CONDITIONS:
 *	Cache object should be valid. 
 *	dir. entry caches:
 *	ObjectData should point to a buffer with at least the same
 *	size of the Cache's objectsize
 *	diskblock caches:
 *	ObjectData should point to a valid block of memory that can
 *	be freed by the Cache if the object need to be flushed
 *
 * POST CONDITIONS:
 *	dir. entry caches:
 *	The object is copied to the cache and marked most recently
 *	used. It does not matter if a previous copy of the object is 
 *	still in the cache. Eventhough a previous copy will not be 
 *	overwritten by this copy, it is garantueed that subsequent 
 *	lookups on this object will return the last added copy.
 *	If the cache is full, the least recently used object is 
 *	released from the cache.
 *	Diskblock caches:
 *	The pointer is copied in the cache and marked as most recently
 *	used. If the cache was full, the least recently used object
 *	is destroyed, that is, the memory is freed.
 *	It _does_ matter if a previously copy is still in the
 *	cache, as both point to the same memory. The oldest pointer
 *	may be flushed, resulting in the second pointer pointing
 *	to invalid memeory.
 *
 *	If NameLen > 16, CacheAddById ignored the object and don't
 *	caches it.
 *
 * IN:
 *	Cache:		pointer to valid TCache object
 *	MediumId:	Id of the medium on which the objects reside
 *	ObjectId:	id of the object
 *	ObjectData:	pointer to caller's buffer space
 * 
 * OUT:
 *	<none>
 *
 * RETURNS:
 *	<none>
 *
 */
void CacheAddById(TCache *Cache, unsigned long MediumId, unsigned long ObjectId, void *ObjectData)
{
		/*
		 * Synchronised access to the Cache
		 */
	EnterMutex(Cache->Mutex, BLOCK_THREAD_IDLE);

	CacheAddByNameOrId(Cache, MediumId, ObjectId, 0, 0, ObjectData);

	LeaveMutex(Cache->Mutex);
}



/*
 * Add an object to the cache
 * 
 * PRE CONDITIONS:
 *	Cache object should be valid. 
 *	dir. entry caches:
 *	ObjectData should point to a buffer with at least the same
 *	size of the Cache's objectsize
 *	diskblock caches:
 *	ObjectData should point to a valid block of memory that can
 *	be freed by the Cache if the object need to be flushed
 *
 * POST CONDITIONS:
 *	dir. entry caches:
 *	The object is copied to the cache and marked most recently
 *	used. It does not matter if a previous copy of the object is 
 *	still in the cache. Eventhough a previous copy will not be 
 *	overwritten by this copy, it is garantueed that subsequent 
 *	lookups on this object will return the last added copy.
 *	If the cache is full, the least recently used object is 
 *	released from the cache.
 *	Diskblock caches:
 *	The pointer is copied in the cache and marked as most recently
 *	used. If the cache was full, the least recently used object
 *	is destroyed, that is, the memory is freed.
 *	It _does_ matter if a previously copy is still in the
 *	cache, as both point to the same memory. The oldest pointer
 *	may be flushed, resulting in the second pointer pointing
 *	to invalid memeory.
 *
 *	If NameLen > 16, CacheAddByName ignored the object and don't
 *	caches it.
 *
 * IN:
 *	Cache:		pointer to valid TCache object
 *	MediumId:	Id of the medium on which the objects reside
 *	ObjectId:	id of the object
 *	ObjectData:	pointer to caller's buffer space
 *	ObjectName:	Name of the object
 *	ObjectLen:	Length of the object's name
 * 
 * OUT:
 *	<none>
 *
 * RETURNS:
 *	<none>
 *
 */
void CacheAddByName(TCache *Cache, unsigned long MediumId, unsigned long ObjectId, char *ObjectName, unsigned NameLen, void *ObjectData)
{
		/*
		 * Synchronised access to the Cache
		 */

		/*
		 * Long ObjectNames were just not cached :-)
		 */
	if (NameLen > MAX_OBJECT_NAME)
		return;

	EnterMutex(Cache->Mutex, BLOCK_THREAD_IDLE);

	CacheAddByNameOrId(Cache, MediumId, ObjectId, ObjectName, NameLen, ObjectData);

	LeaveMutex(Cache->Mutex);
}


/*
 * Returns the statistics for a cache object
 * 
 * PRE CONDITIONS:
 *	Cache object should be valid. Both pointers should point
 *	to memory large enough to store a long
 *
 * POST CONDITIONS:
 *	The number of lookups and hits on the cache are returned
 *
 * IN:
 *	Cache:		pointer to valid TCache object
 * 
 * OUT:
 *	Lookup:	Pointer to unsigned long in which the number of 
 *			lookups on the cache is returned
 *	Hits:	Pointer to unsigned long in which the number of 
 *			hits on the cache is returned
 *
 * RETURNS:
 *	<none>
 *
 */
void CacheGetStats(TCache *Cache, unsigned long *Lookups, unsigned long *Hits)
{
	*Lookups = Cache->nrLookups;
	*Hits = Cache->nrHits;	
}
