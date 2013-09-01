#include  "vxd.h"

#include "shared\vxddebug.h"
#include "error.h"
#include "fsdfile.h"
#include "file.h"
#include "ext2fs.h"
#include "dir.h"
#include "inode.h"
#include "util.h"

#ifdef DEBUG
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

static char*	sEnumhRequest[] =
	{
		"ENUMH_GETFILEINFO",
		"ENUMH_GETFILENAME",
		"ENUMH_GETFINDINFO",
		"ENUMH_RESUMEFIND",
		"ENUMH_RESYNCFILEDIR"
	};
#endif


int __cdecl FsdFileRead(pioreq Ioreq)
{
	TFileContext	*filecontext;

	VxdDebugPrint(D_FILE, "Read, sfn=%lu, pos=%lu, size=%lu", (ULONG) Ioreq->ir_sfn, Ioreq->ir_pos, Ioreq->ir_length);

	filecontext = (TFileContext *) Ioreq->ir_fh;

	if (Ioreq->ir_pos + Ioreq->ir_length > filecontext->FileSize)
		Ioreq->ir_length = (filecontext->FileSize > Ioreq->ir_pos) ?
				filecontext->FileSize - Ioreq->ir_pos
				:
				0;

	Ioreq->ir_error = 0;
	if (!FileRead(filecontext->Inode, Ioreq->ir_pos, Ioreq->ir_length, Ioreq->ir_data))
		Ioreq->ir_error = ERROR_READ_FAULT;	
	else
		Ioreq->ir_pos += Ioreq->ir_length;

	VxdDebugPrint(D_FILE, "Read, done, sfn=%lu, error=%lu, size=%lu, pos=%lu", (ULONG) Ioreq->ir_sfn, (ULONG) Ioreq->ir_error, Ioreq->ir_length, Ioreq->ir_pos);

	return Ioreq->ir_error;
}

int __cdecl FsdFileWrite(pioreq Ioreq)
{

	VxdDebugPrint(D_FILE, "Write, sfn=%lu", (ULONG) Ioreq->ir_sfn);

	Ioreq->ir_error = ERROR_ACCESS_DENIED;

	return Ioreq->ir_error;
}

int __cdecl FsdFileSeek(pioreq Ioreq)
{
	TFileContext	*filecontext;
	int				SignedPosition = (int) Ioreq->ir_pos;
	unsigned int	UnSignedPosition;

	VxdDebugPrint(D_FILE, "Seek, sfn=%lu, pos=%i", (ULONG) Ioreq->ir_sfn, SignedPosition);

	filecontext = (TFileContext *) Ioreq->ir_fh;

	if (Ioreq->ir_flags == FILE_BEGIN)
	{
		if (Ioreq->ir_pos > filecontext->FileSize)
			Ioreq->ir_pos = filecontext->FileSize;
	}
	else
	{
		UnSignedPosition = (unsigned int) (SignedPosition * -1);
		Ioreq->ir_pos = (UnSignedPosition > filecontext->FileSize) ?
			0 : filecontext->FileSize - UnSignedPosition ;
	}

	return (Ioreq->ir_error=0);
}


int __cdecl FsdFileClose(pioreq Ioreq)
{
	TFileContext	*filecontext;

	VxdDebugPrint(D_FILE, "Close, sfn=%lu, option=0x%x, flags=0x%x", (ULONG) Ioreq->ir_sfn, 
			(ULONG) Ioreq->ir_options, (ULONG) Ioreq->ir_flags);

	if (Ioreq->ir_flags == 2 && Ioreq->ir_options == CLOSE_HANDLE)
	{
		filecontext = (TFileContext *) Ioreq->ir_fh;
		FileDestroyFileContext(filecontext);
		VxdDebugPrint(D_FILE, "Close, done, sfn=%lu", (ULONG) Ioreq->ir_sfn);
	}
	else
		VxdDebugPrint(D_FILE, "Close, done (closed nothing though), sfn=%lu", (ULONG) Ioreq->ir_sfn);

	Ioreq->ir_pos = 0;

	return (Ioreq->ir_error=0);
}


int __cdecl FsdFileCommit(pioreq Ioreq)
{
	VxdDebugPrint(D_FILE, "Commit, sfn=%lu", (ULONG) Ioreq->ir_sfn);

	return (Ioreq->ir_error=0);
}

int __cdecl FsdFileFilelocks(pioreq Ioreq)
{
	VxdDebugPrint(D_FILE, "Filelocks, sfn=%lu", (ULONG) Ioreq->ir_sfn);
	return (Ioreq->ir_error=0);
}


int __cdecl FsdFileFiletimes(pioreq Ioreq)
{
	TFileContext	*filecontext;
	ext2_inode		*ext2inode;

	VxdDebugPrint(D_FILE, "Filetimes(%s), sfn=%lu", sAttribActions[Ioreq->ir_flags], (ULONG) Ioreq->ir_sfn);
	filecontext = (TFileContext *) Ioreq->ir_fh;
	ext2inode = InodeLock(filecontext->Inode);

	switch(Ioreq->ir_flags)
	{

		case GET_MODIFY_DATETIME:
			Ioreq->ir_dostime = dateUnix2Dos(ext2inode->i_mtime);
			Ioreq->ir_error = 0;
			break;

		case GET_LAST_ACCESS_DATETIME:
			Ioreq->ir_dostime = dateUnix2Dos(ext2inode->i_atime);
			Ioreq->ir_error = 0;
			break;

		case GET_CREATION_DATETIME:
			Ioreq->ir_dostime = dateUnix2Dos(ext2inode->i_ctime);
			Ioreq->ir_error = 0;
			break;

		case SET_CREATION_DATETIME:
		case SET_MODIFY_DATETIME:
		case SET_LAST_ACCESS_DATETIME:
		default:
			Ioreq->ir_error = ERROR_ACCESS_DENIED;
			break;
	}
	
	InodeUnlock(filecontext->Inode);	

	VxdDebugPrint(D_FILE, "Filetimes, done, sfn=%lu, error=%lu", (ULONG) Ioreq->ir_sfn, (ULONG) Ioreq->ir_error);

	return Ioreq->ir_error;
}


int __cdecl FsdFilePiperequest(pioreq Ioreq)
{
	VxdDebugPrint(D_FILE, "Piperequest");
	return (Ioreq->ir_error=ERROR_INVALID_FUNCTION);
}

int __cdecl FsdFileHandleInfo(pioreq Ioreq)
{
	VxdDebugPrint(D_FILE, "Fileinfo");
	return (Ioreq->ir_error=ERROR_INVALID_FUNCTION);
}

int __cdecl FsdFileEnumHandle(pioreq Ioreq)
{
	TFileContext	*filecontext;
	ext2_inode		*ext2inode;

	_BY_HANDLE_FILE_INFORMATION	*FileInfo;
	VxdDebugPrint(D_FILE, "EnumFile (%s)", sEnumhRequest[Ioreq->ir_flags]);

		/*
		 * by default we do not support most functions
		 */
	Ioreq->ir_error = ERROR_INVALID_FUNCTION;

	switch (Ioreq->ir_flags)
	{
		case ENUMH_GETFILEINFO:
			FileInfo = (_BY_HANDLE_FILE_INFORMATION *) Ioreq->ir_data;
			filecontext = (TFileContext *) Ioreq->ir_fh;
			ext2inode = InodeLock(filecontext->Inode);

			FileInfo->bhfi_dwFileAttributes = filecontext->Inode->DosAttrib;
			IFSMgr_DosToWin32Time(dateUnix2Dos(ext2inode->i_ctime), &FileInfo->bhfi_ftCreationTime);
			IFSMgr_DosToWin32Time(dateUnix2Dos(ext2inode->i_atime), &FileInfo->bhfi_ftLastAccessTime);
			IFSMgr_DosToWin32Time(dateUnix2Dos(ext2inode->i_mtime), &FileInfo->bhfi_ftLastWriteTime);
			
			FileInfo->bhfi_dwVolumeSerialNumber = 0;
			FileInfo->bhfi_nFileSizeHigh = 0;
			FileInfo->bhfi_nFileSizeLow = ext2inode->i_size;
			FileInfo->bhfi_nNumberOfLinks = ext2inode->i_links_count;
			FileInfo->bhfi_nFileIndexHigh = 0;
			FileInfo->bhfi_nFileIndexLow = filecontext->Inode->InodeNo;
		
			InodeUnlock(filecontext->Inode);	
			Ioreq->ir_error = 0;
			break;

		case ENUMH_GETFILENAME:
			break;

		case ENUMH_GETFINDINFO:
			break;

		case ENUMH_RESUMEFIND:
			break;

		case ENUMH_RESYNCFILEDIR:
			break;

		default:
			break;
	}

	return Ioreq->ir_error;
}

int __cdecl FsdFileFindClose(pioreq Ioreq)
{
	TFindContext	*findcontext;

	findcontext = (TFindContext *) Ioreq->ir_fh;
	
	VxdDebugPrint(D_FILE, "FINDCLOSE for ino=%lu", findcontext ? findcontext->Inode->InodeNo : 0);

	if (findcontext)
		DirDestroyFindContext(findcontext);

	VxdDebugFlush();

	return (Ioreq->ir_error=0);
}


int __cdecl FsdFileEmptyFunc(pioreq Ioreq)
{
	VxdDebugPrint(D_FILE, "FileEmptyFunc");
	return (Ioreq->ir_error= ERROR_INVALID_FUNCTION);
}


