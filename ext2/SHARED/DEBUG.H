#ifndef __DEBUG_H
#define __DEBUG_H

#define DOUT_CONSOLE    1		/* dumps to console */
#define DOUT_FILE		2		/* dumps to console */
#define DOUT_NO_EOL 	0x8000	/* don't appends an eol */

/*
 * If you add one, add a string in debug_prefix as well
 */
#ifdef FSD
#define D_ALWAYS		0x0000
#define D_ERROR			0x0001
#define D_SYSTEM		0x0002
#define D_BLOCK			0x0004
#define D_BLKDEV		0x0008
#define D_PARTITION		0x0010
#define D_CACHE			0x0020
#define D_SUPER			0x0040
#define D_FILE			0x0080
#define D_VOLUME		0x0100
#define D_INODE			0x0200
#define D_DIR			0x0400
#define D_VXD			0x0800
#define D_MOUNT			0x1000
#define D_EXT2			0x2000
#define D_END			0x2000
#define D_ALL		 (D_END * 2 - 1)
#else
#define D_ALWAYS		0x0000
#define D_ERROR			0x0001
#define D_SYSTEM		0x0002
#define D_BLKDEV		0x0004
#define D_PARTITION		0x0008
#define D_VXD			0x0010
#define D_TSD			0x0020
#define D_END			0x0020
#define D_ALL		 (D_END * 2 - 1)
#endif


#endif			/* __DEBUG_H */