#ifndef __VOLFUNCS_H
#define __VOLFUNCS_H

/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/


int __cdecl FsdVolOpen(pioreq Ioreq);
int __cdecl FsdVolIoctl16(pioreq Ioreq);
int __cdecl FsdVolDelete(pioreq Ioreq);
int __cdecl FsdVolDir(pioreq Ioreq);
int __cdecl FsdVolAttrib(pioreq Ioreq);
int __cdecl FsdVolFlush(pioreq Ioreq);
int __cdecl FsdVolInfo(pioreq Ioreq);
int __cdecl FsdVolRename(pioreq Ioreq);
int __cdecl FsdVolSearch(pioreq Ioreq);
int __cdecl FsdVolQuery(pioreq Ioreq);
int __cdecl FsdVolDisconnect(pioreq Ioreq);
int __cdecl FsdVolUNCPipe(pioreq Ioreq);
int __cdecl FsdVolParams(pioreq Ioreq);
int __cdecl FsdVolFindOpen(pioreq Ioreq);
int __cdecl FsdVolDasdio(pioreq Ioreq);

#endif			/* #ifndef __VOLFUNCS_H */