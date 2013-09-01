#include  "vxd.h"

#include "shared\vxddebug.h"
#include "error.h"
#include "fsdfile.h"
#include "fsdvol.h"

#include "ext2fs.h"
#include "super.h"
#include "inode.h"
#include "unixstat.h"
#include "util.h"
#include "dir.h"
#include "file.h"
#include "crs.h"


//extern PVRP gVrp;


#define SUPER(ioreq) ( ((TSuperBlock *) (ioreq)->ir_rh) )

//static int __cdecl FsdShortFindNext(pioreq Ioreq) {return 0;}
//static TFindContext* FsdPrepareFindNext(pioreq Ioreq) {return 0;}
//static ext2_dir_entry* FsdDoFindNext(TFindContext	*findcontext, ext2_inode *Inode, char *ShortName) { return 0;}
//static int __cdecl FsdLfnFindNext(pioreq Ioreq) {return 0;}

/**********************************
 *
 * STATIC DATA
 *
 **********************************/


static hndlmisc sFsdFileFunctions = 
	{
		IFS_VERSION,
		IFS_REVISION,
		NUM_HNDLMISC,

		FsdFileSeek,
		FsdFileClose,
		FsdFileCommit,	 
		FsdFileFilelocks,
		FsdFileFiletimes,
		FsdFilePiperequest,
		FsdFileHandleInfo,
		FsdFileEnumHandle
	};

static hndlmisc gFsdFindFunctions = 
	{
		IFS_VERSION,
		IFS_REVISION,
		NUM_HNDLMISC,
		{
			FsdFileEmptyFunc,
			FsdFileFindClose,
			FsdFileEmptyFunc,
			FsdFileEmptyFunc,
			FsdFileEmptyFunc,
			FsdFileEmptyFunc,
			FsdFileEmptyFunc,
			FsdFileEnumHandle
		}
	};

static char gFileSystem[] = "ext2 fs";
static char gResourceName[] = "linux";

#ifdef DEBUG
static char*	sDirActions[] =
	{
		"create",
		"delete",
		"check",
		"query8.3",
		"querylfn"
	};

static char*	sAttribActions[] =
	{
		"GET_ATTRIBUTES",
		"SET_ATTRIBUTES",
		"GET_ATTRIB_COMP_FILESIZE",
		"SET_ATTRIB_MODIFY_DATETIME",
		"GET_ATTRIB_MODIFY_DATETIME",
		"SET_ATTRIB_LAST_ACCESS_DATETIME",
		"GET_ATTRIB_LAST_ACCESS_DATETIME",
		"SET_ATTRIB_CREATION_DATETIME",
		"GET_ATTRIB_CREATION_DATETIME",
		"GET_ATTRIB_FIRST_CLUST"
	};
#endif




/**********************************
 *
 * STATIC HELPERS
 *
 **********************************/

#ifdef NYI
static PIOR CreateIOR(PVRP Vrp, DWORD opcode, DWORD flags, int Drive)
{							
	DWORD size = Vrp->VRP_max_req_size + Vrp->VRP_max_sgd * sizeof(SGD);
	DWORD offset = Vrp->VRP_delta_to_ior;
	PIOR ior = IspCreateIor((USHORT) size, (USHORT) offset, ISP_M_FL_MUST_SUCCEED);
	if (!ior)
		return NULL;

	ior->IOR_next = 0;					// used for request queueing by lots of downstream people
	ior->IOR_start_addr[1] = 0;			// assume only 32-bits worth of sector number
	ior->IOR_flags = IORF_VERSION_002 | flags;	// this is how IOS knows we're giving it a new-style IOR
	ior->IOR_private_client = offset;	// a common use for this field
	ior->IOR_req_vol_handle = (ULONG) Vrp;
	ior->IOR_sgd_lin_phys = (ULONG) (ior + 1); // where scatter/gather descriptors can be built
	ior->IOR_num_sgds = 0;				// incremented by criteria routine, so must start at zero
	ior->IOR_vol_designtr = Drive;	// drive number
	ior->IOR_func = (USHORT) opcode;

	return ior;
}

static void DestroyIOR(PIOR ior)
{
	IspDeallocMem((PVOID) ((DWORD) ior - ior->IOR_private_client));
}
#endif		/* NYI */

static void FillLfnFindData(TFindContext *FindContext, _WIN32_FIND_DATA *FindData)
{
	_QWORD			Result;
	ext2_inode		*ext2inode;

		/*
		 * We've set the attributes when we tested them for a match
		 */
	ext2inode = InodeLock(FindContext->FoundInode);

	FindData->dwFileAttributes = FindContext->FoundInode->DosAttrib;

	IFSMgr_DosToWin32Time(dateUnix2Dos(ext2inode->i_ctime), &FindData->ftCreationTime);
	IFSMgr_DosToWin32Time(dateUnix2Dos(ext2inode->i_atime), &FindData->ftLastAccessTime);
	IFSMgr_DosToWin32Time(dateUnix2Dos(ext2inode->i_mtime), &FindData->ftLastWriteTime);
	FindData->nFileSizeHigh = 0;
	FindData->nFileSizeLow = ext2inode->i_size;
		
		/*
		 * Convert to Unicode LFN FileName
		 */
	BCSToUni(FindData->cFileName, FindContext->FoundDirEntry->name, FindContext->FoundDirEntry->name_len, BCS_OEM, &Result);
	FindData->cFileName[Result.ddLower >> 1] = 0;

		/*
		 * Convert to Unicode Short FileName, make sure we do not
		 * convert . or ..
		 */
	if (isDotOrDotDot(FindContext->FoundDirEntry->name, FindContext->FoundDirEntry->name_len))
	{
		BCSToUni(FindData->cAlternateFileName, FindContext->FoundDirEntry->name, FindContext->FoundDirEntry->name_len, BCS_OEM, &Result);
		FindData->cAlternateFileName[Result.ddLower >> 1] = 0;
	}
	else
	{
		LongToShort(FindContext->FoundDirEntry->name, FindContext->FoundDirEntry->name_len, FindContext->ShortName, FindContext->FoundDirEntry->inode);
		BCSToUni(FindData->cAlternateFileName, FindContext->ShortName, strlen(FindContext->ShortName), BCS_OEM, &Result);
		FindData->cAlternateFileName[Result.ddLower >> 1] = 0;
	}

	InodeUnlock(FindContext->FoundInode);
}



static void FillShortFindData(TFindContext *FindContext, srch_entry *SearchEntry)
{
	dos_time		d;
	ext2_inode		*ext2inode;

	ext2inode = InodeLock(FindContext->FoundInode);

	SearchEntry->se_attrib = (UCHAR) FindContext->FoundInode->DosAttrib;
	d = dateUnix2Dos(ext2inode->i_mtime);
	SearchEntry->se_time = d.dt_time;
	SearchEntry->se_date = d.dt_date;
	SearchEntry->se_size = ext2inode->i_size;

	strcpy(SearchEntry->se_name, FindContext->ShortName);

	InodeUnlock(FindContext->FoundInode);
}




/*
 * Called by both FsdFindFirst and FsdSearch
 * to setup search info
 *
 */
static TFindContext* FsdPrepareFindNext(pioreq Ioreq)
{
	TFindContext	*findcontext;
	TInode			*inode;
	char			bcspathname[MAX_PATH];
	char			*filename;
	char			*path;


	VxdDebugPrint(D_VOLUME, "FsdPrepareFindNext");

	findcontext = 0;
	Ioreq->ir_error = 0;

		/*
		 * First get the bcs_pathname of the directory in which
		 * the lookup is to be performed
		 */
	if (GetBcsPath(SUPER(Ioreq)->DosDrive, Ioreq->ir_upath, bcspathname))
	{
			/*
			 * Cut off the file name part by looking for the
			 * last dirname separator
			 */
		if ((filename = strrchr(bcspathname, '/')))
		{
			path = bcspathname;
			*filename++ = 0;
		}
		else
		{
			path = "";
			filename = bcspathname;		// we only have a filename
		}

			/*
			 * Get the inode belonging to the directoryname
			 */
		if ( (inode = DirName2Inode(SUPER(Ioreq), path)))
		{

				/*
				 */
			if ((findcontext = DirCreateFindContext(inode, filename, Ioreq->ir_attr)))
			{
					/*
					 * As we peform the matching ourselves, translate 
					 * "*.*" in "*"
					 */

				if (findcontext->SearchFlags & FILE_FLAG_HAS_DOT)
				{
					findcontext->SearchFlags &= ~FILE_FLAG_HAS_DOT;
					SmartConvertDots2QuestionMark(findcontext->SearchName);
				}
			}
			else
			{
				Ioreq->ir_error = ERROR_PATH_NOT_FOUND;
				InodeRelease(inode);
			}
		}
		else
		{
			VxdDebugPrint(D_VOLUME, "FsdPrepareFindNext: path not found");	
			Ioreq->ir_error = ERROR_PATH_NOT_FOUND;
		}
	}
	else
	{
		VxdDebugPrint(D_ERROR, "FsdPrepareFindNext: could not getbcspath on:%U", Ioreq->ir_upath);
		Ioreq->ir_error = ERROR_PATH_NOT_FOUND;
	}

	VxdDebugPrint(D_VOLUME, "FsdPrepareFindNext: done");
	return findcontext;
}


static int __cdecl FsdLfnFindNext(pioreq Ioreq)
{
	TFindContext		*findcontext;

		/*
		 * Search for entry, using lfn matching scheme
		 */
	Ioreq->ir_error = 0;
	findcontext = (TFindContext	*) Ioreq->ir_fh;

	if (findcontext)
	{

		VxdDebugPrint(D_VOLUME, "FNEXT(lfn): inode=(dev=%s, ino=%lu), search=%s",
				findcontext->Inode->Super->DeviceName,
				findcontext->Inode->InodeNo,
				findcontext->SearchName);

			/*
			 * Perform a lookup on long names
			 */
		if ((DirFindEntry(findcontext, FALSE)))
		{
				/*
				/*
				 * Great! found a match...
				 * Fill in the Win32 FindData structure
				 */
			FillLfnFindData(findcontext, Ioreq->ir_data);
			BlockUnlock(findcontext->Block);
			InodeRelease(findcontext->FoundInode);
			VxdDebugPrint(D_VOLUME, "FNEXT(lfn): done, found:%U", 
				((_WIN32_FIND_DATA *) Ioreq->ir_data)->cFileName);
		}
		else
		{
			VxdDebugPrint(D_VOLUME, "FNEXT(lfn): done, no more files");
			Ioreq->ir_error = ERROR_NO_MORE_FILES;
		}
		VxdDebugFlush();
	}
	else
	{
		VxdDebugPrint(D_VOLUME, "FNEXT(lfn): done (no findcontext), no more files");
		Ioreq->ir_error = ERROR_NO_MORE_FILES;
	}

	return Ioreq->ir_error;
}



static int __cdecl FsdShortFindNext(pioreq Ioreq)
{
	TFindContext		*findcontext;
	srch_entry			*search_entry;

		/*
		 * Search for entry, using lfn matching scheme
		 */
	Ioreq->ir_error = 0;
	search_entry = (srch_entry *) Ioreq->ir_data;
	memcpy(&findcontext, search_entry->se_key.sk_localFSD, 4);

	VxdDebugPrint(D_VOLUME, "FNEXT(short): inode=(dev=%s, ino=%lu), search=%s",
			findcontext->Inode->Super->DeviceName,
			findcontext->Inode->InodeNo,
			findcontext->SearchName);

	if ((DirFindEntry(findcontext, TRUE)))
	{
			/*
			/*
			 * Great! found a match...
			 * Fill in the search entry structure
			 */
		FillShortFindData(findcontext, search_entry);
		BlockUnlock(findcontext->Block);
		InodeRelease(findcontext->FoundInode);
		VxdDebugPrint(D_VOLUME, "FNEXT(short): done, found:%s", 
			findcontext->ShortName);
	}
	else
	{
		VxdDebugPrint(D_VOLUME, "FNEXT(short): done, no more files");
		Ioreq->ir_error = ERROR_NO_MORE_FILES;
		DirDestroyFindContext(findcontext);
	}
	VxdDebugFlush();
	return Ioreq->ir_error;


#ifdef NYI
	TFindContext	*findcontext;
	ext2_inode			inode;
	ext2_dir_entry		*dir_entry;
	srch_entry			*search_entry;

		/*
		 * Search for entry, using short filename matching scheme
		 */
	Ioreq->ir_error = 0;
	search_entry = (srch_entry *) Ioreq->ir_data;
	memcpy(&findcontext, search_entry->se_key.sk_localFSD, 4);
	if ((dir_entry=FsdDoFindNext(findcontext, &inode, search_entry->se_name)))
	{
			/*
			/*
			 * Great! found a match...
			 * Fill in the search entry structure
			 */
		FillShortFindData(dir_entry, &inode, search_entry);
	}
	else
		Ioreq->ir_error = ERROR_NO_MORE_FILES;

	return Ioreq->ir_error;
#endif			/* NYI */
}




/**********************************
 *
 * INTERFACE ROUTINES
 *
 **********************************/


int __cdecl FsdVolOpen(pioreq Ioreq)
{
	char			bcsfilename[MAX_PATH];
	TFileContext	*filecontext;
	TInode			*inode;
	ext2_inode		*ext2inode;
	unsigned char	flags;

		
	VxdDebugPrint(D_VOLUME,  "FileOpen, file=%U, sfn=%lu, flags=%lu, options=%lu", 
			Ioreq->ir_upath, (ULONG) Ioreq->ir_sfn, (ULONG) Ioreq->ir_flags, (ULONG) Ioreq->ir_options);

	if (GetBcsPath(SUPER(Ioreq)->DosDrive, Ioreq->ir_upath, bcsfilename))
	{
		if ((inode = DirName2Inode(SUPER(Ioreq), bcsfilename)))
		{
			//if (!S_ISDIR(inode->Mode))
			if (S_ISREG(inode->Mode))
			{

					/*
					 * We are a read-only file system
					 */

				flags = Ioreq->ir_flags & ACCESS_MODE_MASK;
				if (	((flags == ACCESS_READONLY) || (flags != ACCESS_EXECUTE))
						&&
						((Ioreq->ir_options == ACTION_OPENEXISTING) || (Ioreq->ir_options == ACTION_OPENALWAYS)))
				{
					if ((filecontext = FileCreateFileContext(inode)))
					{
						ext2inode = InodeLock(inode);

						Ioreq->ir_hfunc->hf_read = FsdFileRead;	
						Ioreq->ir_hfunc->hf_write = FsdFileWrite;	
						Ioreq->ir_hfunc->hf_misc = &sFsdFileFunctions;
						Ioreq->ir_fh = filecontext;
						Ioreq->ir_options = ACTION_OPENED;
						Ioreq->ir_size = filecontext->FileSize;
						Ioreq->ir_attr = inode->DosAttrib;
						Ioreq->ir_dostime = dateUnix2Dos(ext2inode->i_mtime);
						Ioreq->ir_error = 0;

						InodeUnlock(inode);	
					}
					else
					{
						Ioreq->ir_error = ERROR_NOT_ENOUGH_MEMORY;
						InodeRelease(inode);
					}
				}
				else
				{
					Ioreq->ir_error = ERROR_ACCESS_DENIED;
					InodeRelease(inode);
				}
			}
			else
			{
				Ioreq->ir_error = ERROR_FILE_NOT_FOUND;
				InodeRelease(inode);
			}
		}
		else
			Ioreq->ir_error = ERROR_FILE_NOT_FOUND;
	}
	else
		Ioreq->ir_error = ERROR_FILE_NOT_FOUND;

	VxdDebugPrint(D_VOLUME,  "FileOpen, done, error=%lu", Ioreq->ir_error);
	VxdDebugFlush();

	return Ioreq->ir_error;
}



int __cdecl FsdVolIoctl16(pioreq Ioreq)
{
	CLIENT_STRUCT		*pRegs;
	PIOR				ior;
	USHORT				status;

	pRegs = (CLIENT_STRUCT*) Ioreq->ir_cregptr;	

	VxdDebugPrint(D_VOLUME, "IOCTL16, al=%x, bl=%x, cl=%x", (unsigned) _clientAL, (unsigned) _clientBL, (unsigned) _clientCL);

	Ioreq->ir_error = 5;
	VxdDebugFlush();
	return Ioreq->ir_error;

#ifdef NYI
	ior = CreateIOR((PVRP) SUPER(Ioreq)->Vrp, IOR_GEN_IOCTL, IORF_SYNC_COMMAND, SUPER(Ioreq)->DosDrive);
	if (!ior)
		return Ioreq->ir_error = ERROR_NOT_ENOUGH_MEMORY;

	// macro to simplify grossly long field name access:
	#define ioctl(f) ior->_ureq.sdeffsd_req_usage._IOR_ioctl_##f
	
	pRegs = (CLIENT_STRUCT*) Ioreq->ir_cregptr;	
	ioctl(client_params) = (ULONG) pRegs;
	ioctl(function) = _ClientAX;
	//ASSERT(_ClientBL-1 == m_drive);
	ioctl(drive) = SUPER(Ioreq)->DosDrive;
	ioctl(control_param) = _ClientCX;

	if (Ioreq->ir_options & IOCTL_PKT_LINEAR_ADDRESS)
		ioctl(buffer_ptr) = (ULONG) Ioreq->ir_data; // flat buffer address
	else
		{						// 16-bit IOCTL
		ior->IOR_flags |= IORF_16BIT_IOCTL;
		ioctl(buffer_ptr) = (ULONG) Client_Ptr_Flat(DS, DX);
		}						// 16-bit IOCTL

	//IOS_SendCommand(ior, (PDCB) Vrp->VRP_device_handle);
	IOS_SendCommand(ior, SUPER(Ioreq)->Device);
	
	status = ior->IOR_status;
	_ClientAX = (USHORT) ioctl(return);
	DestroyIOR(ior);

	if (status >= IORS_ERROR_DESIGNTR)	
		status = (USHORT) IOSMapIORSToI21(status);
	else
		status = 0;				// success after error is still success

	Ioreq->ir_error = (int) status;

	VxdDebugPrint(D_VOLUME, "IOCTL16, done, error=%i", Ioreq->ir_error);

	return Ioreq->ir_error;
#endif		/* NYI */
}


int __cdecl FsdVolDelete(pioreq Ioreq)
{
	VxdDebugPrint(D_VOLUME,  "DEL");
	Ioreq->ir_error = 5;
	VxdDebugFlush();
	return Ioreq->ir_error;
}











int __cdecl FsdVolDir(pioreq Ioreq)
{
	char			bcsdirname[MAX_PATH];
	TInode			*inode;

	VxdDebugPrint(D_VOLUME, "DIR (%s), dir=%U", sDirActions[Ioreq->ir_flags], Ioreq->ir_upath);

	switch (Ioreq->ir_flags)
	{
		case CREATE_DIR:
		case DELETE_DIR:
		case QUERY83_DIR:
			Ioreq->ir_error = ERROR_INVALID_FUNCTION;
			break;

		case CHECK_DIR:
			Ioreq->ir_error = ERROR_PATH_NOT_FOUND;
			if (GetBcsPath(SUPER(Ioreq)->DosDrive, Ioreq->ir_upath, bcsdirname))
			{
				if ((inode = DirName2Inode(SUPER(Ioreq), bcsdirname)))
				{
					if (S_ISDIR(inode->Mode))
						Ioreq->ir_error = 0;
					InodeRelease(inode);
				}
			}
			break;

		case QUERYLONG_DIR:
			Ioreq->ir_error = ERROR_PATH_NOT_FOUND;
			if (GetBcsPath(SUPER(Ioreq)->DosDrive, Ioreq->ir_upath, bcsdirname))
			{
				if ((inode = DirName2Inode(SUPER(Ioreq), bcsdirname)))
				{
					if (S_ISDIR(inode->Mode))
					{
						Ioreq->ir_error = 0;
						BcsToUniParsed(bcsdirname, (PUSHORT) Ioreq->ir_ppath2);
					}
					InodeRelease(inode);
				}
			}
			break;

		default:
			Ioreq->ir_error = ERROR_INVALID_FUNCTION;
			break;
	}

	VxdDebugPrint(D_VOLUME, "DIR: done, error=%i", Ioreq->ir_error);
	VxdDebugFlush();

	return Ioreq->ir_error;
}








int __cdecl FsdVolAttrib(pioreq Ioreq)
{
	TInode			*inode;
	ext2_inode		*ext2inode;
	char			bcspathname[MAX_PATH];

	VxdDebugPrint(D_VOLUME, "ATTRIB(%s), path=%U, attrib=%X", sAttribActions[Ioreq->ir_flags], Ioreq->ir_upath, Ioreq->ir_uFName, (Ioreq->ir_attr & 0xFFFF));

		/*
		 * First get the bcs_pathname of the directory in which
		 * the lookup is to be performed
		 */
	if (GetBcsPath(SUPER(Ioreq)->DosDrive, Ioreq->ir_upath, bcspathname))
	{

			/*
			 * Get the inode belonging to the object name
			 */
		if ( (inode = DirName2Inode(SUPER(Ioreq), bcspathname)))
		{
			ext2inode = InodeLock(inode);

			switch(Ioreq->ir_flags)
			{
				case GET_ATTRIBUTES:
					Ioreq->ir_attr = inode->DosAttrib;
					Ioreq->ir_error = 0;
					break;

				case GET_ATTRIB_MODIFY_DATETIME:
					Ioreq->ir_dostime = dateUnix2Dos(ext2inode->i_mtime);
					Ioreq->ir_error = 0;
					break;

				case GET_ATTRIB_LAST_ACCESS_DATETIME:
					Ioreq->ir_dostime = dateUnix2Dos(ext2inode->i_atime);
					Ioreq->ir_error = 0;
					break;

				case GET_ATTRIB_CREATION_DATETIME:
					Ioreq->ir_dostime = dateUnix2Dos(ext2inode->i_ctime);
					Ioreq->ir_error = 0;
					break;

				case GET_ATTRIB_COMP_FILESIZE:
					Ioreq->ir_size = ext2inode->i_size;
					break;

					/*
					 * Don't fail these, just ignore them...
					 */
				case SET_ATTRIB_LAST_ACCESS_DATETIME:
				case SET_ATTRIB_MODIFY_DATETIME:
				case SET_ATTRIB_CREATION_DATETIME:
					Ioreq->ir_error = 0;
					break;

				case SET_ATTRIBUTES:
				default:
					Ioreq->ir_error = ERROR_ACCESS_DENIED;
					break;
			}
			InodeUnlock(inode);	
		}
		else
			Ioreq->ir_error = ERROR_FILE_NOT_FOUND;
	}
	else
		Ioreq->ir_error = ERROR_FILE_NOT_FOUND;

	VxdDebugPrint(D_VOLUME, "ATTRIB: done, error=%lu", (ULONG) Ioreq->ir_error);
	VxdDebugFlush();

	return Ioreq->ir_error;
}








int __cdecl FsdVolFlush(pioreq Ioreq) 
{
	VxdDebugPrint(D_VOLUME,  "flush");
	VxdDebugFlush();
	return (Ioreq->ir_error=0);
}


/*
	IOREQ Structure:
	Entry
		ir_rh -- Supplies handle to disk volume or network resource to get info on.
		ir_flags -- Supplies special flags for this function.
			GetDiskInfo Flags:
			GDF_NO_DISK_HIT -- This flag is a hint flag to the FSD. It specifies that the FSD should, if possible, return a close approximation to the freespace on the disk without accessing the disk in any fashion. This is an optimization that is used by some components in the system. It is not mandatory to implement this except in certain special cases. Look in the notes below for more details.
		ir_user -- User id for this request.
		ir_pid -- Process id for this request.
	Returns
		ir_error -- Returns status of the operation ( 0 if no error, errorcode otherwise ).
		ir_length -- Returns number of bytes per sector.
		ir_size -- Returns total number of allocation units.
		ir_data -- Returns an optional pointer to the file allocation table (FAT) identification byte.  This is only required for local FAT file systems.
		ir_sectors -- Returns number of sectors per allocation unit.
		ir_numfree -- Returns number of free allocation units.
*/

int __cdecl FsdVolInfo(pioreq Ioreq)
{
	ext2_super_block	*ext2_super;

	VxdDebugPrint(D_VOLUME,  "info");

	ext2_super = SuperLock(SUPER(Ioreq));

		/*
		 * Eventhough the ir_size and ir_numfree are 32 bit numbers,
		 * most applications (also the explorer) treat these as
		 * 16 bits numbers.
		 * Workaround: we always return allocation unit of 32KB
		 */
	//Ioreq->ir_data = 0;
	//Ioreq->ir_length = 512;
	//Ioreq->ir_sectors = 1 << DevGetLogBlockSize(SUPER(Ioreq)->Device);
	//Ioreq->ir_size = ext2_super->s_blocks_count; 
	//Ioreq->ir_numfree = ext2_super->s_free_blocks_count;

	Ioreq->ir_data = 0;
	Ioreq->ir_length = 512;
	Ioreq->ir_sectors = 64; // 64 == 1 << 6
	Ioreq->ir_size = ext2_super->s_blocks_count >> (6 - DevGetLogBlockSize(SUPER(Ioreq)->Device)); 
	Ioreq->ir_numfree = ext2_super->s_free_blocks_count >> (6 - DevGetLogBlockSize(SUPER(Ioreq)->Device)); 

	VxdDebugPrint(D_VOLUME,  "sector=%lu, size=%lu, numfree=%lu", (ULONG) Ioreq->ir_sectors, (ULONG) Ioreq->ir_size, (ULONG) Ioreq->ir_numfree);
	
	SuperUnlock(SUPER(Ioreq));

	VxdDebugFlush();

	return (Ioreq->ir_error=0);
}










int __cdecl FsdVolRename(pioreq Ioreq)
{
	VxdDebugPrint(D_VOLUME,  "rename");
	return (Ioreq->ir_error=5);
}







int __cdecl FsdVolSearch(pioreq Ioreq)
{
	TFindContext	*findcontext;
	srch_entry		*search_entry;
	char			*pc;
	int				error;
	int				attr;

	if (Ioreq->ir_flags == SEARCH_FIRST)
	{
		attr = Ioreq->ir_attr & 0xFFFF;
		Ioreq->ir_error = 0;

		VxdDebugPrint(D_VOLUME, "FFIRST(short), path=%U, attrib=%X", Ioreq->ir_upath, attr);

			/*
			 * If we are asked for a dir label, we return
			 * the device name
			 */
		if (attr == FILE_ATTRIBUTE_LABEL)
		{
			search_entry = (srch_entry *) Ioreq->ir_data;
			search_entry->se_attrib = FILE_ATTRIBUTE_LABEL;
			strcpy(search_entry->se_name, SUPER(Ioreq)->DeviceName + 5);
			VxdDebugPrint(D_VOLUME, "FFIRST(short), done, returned label");
			VxdDebugFlush();

			return Ioreq->ir_error;
		}


			/*
			 * Setup a findcontext
			 */
		if ((findcontext = FsdPrepareFindNext(Ioreq)))
		{

				/*
				 * Update the ioreq for future findnext calls
				 */
			search_entry = (srch_entry *) Ioreq->ir_data;
			memcpy(search_entry->se_key.sk_localFSD, &findcontext, 4);
			Ioreq->ir_firstclus = 0;

				/*
				 * 8.3 searches with capitals
				 */
			pc = findcontext->SearchName;
			while(*pc)
				*pc++ = toupper(*pc);


			
				/*
				 * Now simple issue an findnext
				 */
			error = FsdShortFindNext(Ioreq);
				
				/*
				 * Adjust the error code for a findfirst
				 */
			if (error == ERROR_NO_MORE_FILES)
				Ioreq->ir_error = ERROR_FILE_NOT_FOUND;
		}
		VxdDebugPrint(D_VOLUME, "FFIRST(short), done, %s",
			Ioreq->ir_error ? "no more files or error" : "entry found");
	}
	else
		FsdShortFindNext(Ioreq);
		
	VxdDebugFlush();
	return Ioreq->ir_error;
}



int __cdecl FsdVolQuery(pioreq Ioreq)
{

	VxdDebugPrint(D_VOLUME, "QUERY level %i", Ioreq->ir_options);

	switch(Ioreq->ir_options)
	{
		case 0:
		case 1:
			Ioreq->ir_error = ERROR_INVALID_FUNCTION;
			break;
		case 2:
			//BCSToUni(Ioreq->ir_data, gFileSystem, strlen(gFileSystem), BCS_OEM, &qword);
			//((PUSHORT) Ioreq->ir_data)[qword.ddLower >> 1] = 0;
			Ioreq->ir_length = EXT2_NAME_LEN | (EXT2_NAME_LEN << 16);
			Ioreq->ir_options = FS_CASE_IS_PRESERVED | FS_VOL_SUPPORTS_LONG_NAMES;
			//Ioreq->ir_options = FS_VOL_SUPPORTS_LONG_NAMES;
			Ioreq->ir_error = 0;
			break;
	}
	VxdDebugPrint(D_VOLUME, "QUERY: done");
	VxdDebugFlush();

	return Ioreq->ir_error;
}

int __cdecl FsdVolDisconnect(pioreq Ioreq)
{
	VxdDebugPrint(D_VOLUME,  "disconnect");
	VxdDebugFlush();
	return (Ioreq->ir_error=5);
}

int __cdecl FsdVolUNCPipe(pioreq Ioreq)
{
	VxdDebugPrint(D_VOLUME,  "UNCPipe");
	VxdDebugFlush();
	return (Ioreq->ir_error=5);
}

int __cdecl FsdVolParams(pioreq Ioreq)
{
		/*
		 * We do maintain DPDs
		 */
	VxdDebugPrint(D_VOLUME,  "Params");
	VxdDebugFlush();
	return (Ioreq->ir_error = ERROR_INVALID_FUNCTION);
}


int __cdecl FsdVolFindOpen	(pioreq Ioreq)
{
	_QWORD				Result;
	_WIN32_FIND_DATA	*FindData;
	TFindContext		*findcontext;
	char				*devicename;
	int					error;
	int					attr;

	attr = (Ioreq->ir_attr & 0xFFFF);
	Ioreq->ir_error = 0;
	VxdDebugPrint(D_VOLUME, "FFIRST(lfn), path=%U, attrib=%X", Ioreq->ir_upath, attr);

	if (attr == FILE_ATTRIBUTE_LABEL)
	{
			/*
			 * return the device name on a label request
			 */
		devicename = SUPER(Ioreq)->DeviceName + 5;
		FindData = Ioreq->ir_data;
			
			/*
			 * Set long name
			 */
		BCSToUni(FindData->cFileName, devicename, strlen(devicename), BCS_OEM, &Result);
		FindData->cFileName[Result.ddLower >> 1] = 0;

			/*
			 * Set short name
			 */
		BCSToUni(FindData->cAlternateFileName, devicename, strlen(devicename), BCS_OEM, &Result);
		FindData->cAlternateFileName[Result.ddLower >> 1] = 0;

		FindData->dwFileAttributes = FILE_ATTRIBUTE_LABEL;
		Ioreq->ir_pos = 0;
		Ioreq->ir_fh = 0;
		SetHandleFunc(Ioreq, FsdLfnFindNext, FsdFileEmptyFunc, &gFsdFindFunctions);	

		VxdDebugPrint(D_VOLUME, "FFIRST(lfn), done, returned label");
		
		return 0;
	}

		/*
		 * Setup a findcontext
		 */
	if ((findcontext = FsdPrepareFindNext(Ioreq)))
	{

			/*
			 * Update the ioreq for future findnext calls
			 */
		Ioreq->ir_fh = findcontext;
		SetHandleFunc(Ioreq, FsdLfnFindNext, FsdFileEmptyFunc, &gFsdFindFunctions);
		
			/*
			 * Now simply issue an findnext
			 */
		error = FsdLfnFindNext(Ioreq);
			
			/*
			 * Adjust the error code for a findfirst
			 */
		if (error == ERROR_NO_MORE_FILES)
			Ioreq->ir_error = ERROR_FILE_NOT_FOUND;

		if (error)
			DirDestroyFindContext(findcontext);
	}
	VxdDebugPrint(D_VOLUME, "FFIRST(lfn), done, %s",
		Ioreq->ir_error ? "no more files or error" : "entry found");
	
	VxdDebugFlush();

	return Ioreq->ir_error;
}



int __cdecl FsdVolDasdio(pioreq Ioreq)
{
	VxdDebugPrint(D_VOLUME, "DASDIO");

	VxdDebugFlush();

	return (Ioreq->ir_error = ERROR_INVALID_FUNCTION);
}


