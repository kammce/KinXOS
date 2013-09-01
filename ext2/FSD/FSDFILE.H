#ifndef __FSDFILE_H
#define __FSDFILE_H

/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/


int __cdecl FsdFileRead(pioreq Ioreq);
int __cdecl FsdFileWrite(pioreq Ioreq);
int __cdecl FsdFileSeek(pioreq Ioreq);
int __cdecl FsdFileClose(pioreq Ioreq);
int __cdecl FsdFileCommit(pioreq Ioreq);
int __cdecl FsdFileFilelocks(pioreq Ioreq);
int __cdecl FsdFileFiletimes(pioreq Ioreq);
int __cdecl FsdFilePiperequest(pioreq Ioreq);
int __cdecl FsdFileHandleInfo(pioreq Ioreq);
int __cdecl FsdFileEnumHandle(pioreq Ioreq);
int __cdecl FsdFileFindClose(pioreq Ioreq);
int __cdecl FsdFileEmptyFunc(pioreq Ioreq);


#endif			/* __FSDFILE_H */