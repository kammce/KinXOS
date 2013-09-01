#ifndef __FILE_H
#define __FILE_H


typedef struct _FileContext
{
	struct _Inode	*Inode;
	ULONG			FileSize;
} TFileContext;



/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/

TFileContext*	FileCreateFileContext(struct _Inode *Inode);
void			FileDestroyFileContext(TFileContext *Context);

BOOL	FileRead(struct _Inode *Inode, ULONG Pos, ULONG Length, char *Data);


#endif			/* __FILE_H */