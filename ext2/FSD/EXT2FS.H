#ifndef __EXT2FS_H
#define __EXT2FS_H


/*********************************
 *
 * STRUCTS, TYPEDEFS AND DEFINES
 *
 **********************************/



/*
 * Most of these typedefs have shamelessly been copied from the
 * linux source
 */

#define EXT2_SUPER_MAGIC	0xEF53
#define EXT2_SIZE_SB		1024			/* size of superblock on disk */
/*
 * Constants relative to the data blocks
 */
#define	EXT2_NDIR_BLOCKS		12l
#define	EXT2_IND_BLOCK			EXT2_NDIR_BLOCKS
#define	EXT2_DIND_BLOCK			(EXT2_IND_BLOCK + 1l)
#define	EXT2_TIND_BLOCK			(EXT2_DIND_BLOCK + 1l)
#define	EXT2_N_BLOCKS			(EXT2_TIND_BLOCK + 1l)

/*
 * Inode flags
 */
#define	EXT2_SECRM_FL			0x00000001l /* Secure deletion */
#define	EXT2_UNRM_FL			0x00000002l /* Undelete */
#define	EXT2_COMPR_FL			0x00000004l /* Compress file */
#define EXT2_SYNC_FL			0x00000008l /* Synchronous updates */
#define EXT2_IMMUTABLE_FL		0x00000010l /* Immutable file */
#define EXT2_APPEND_FL			0x00000020l /* writes to file may only append */
#define EXT2_NODUMP_FL			0x00000040l /* do not dump file */


/*
 * Macro-instructions used to manage several block sizes
 */

#define EXT2_MIN_BLOCK_SIZE		1024l
#define	EXT2_MAX_BLOCK_SIZE		4096l
#define EXT2_MIN_BLOCK_LOG_SIZE		  10l
#define EXT2_BLOCK_SIZE(s)	( EXT2_MIN_BLOCK_SIZE << (s)->s_log_block_size)
#define	EXT2_INODES_PER_BLOCK(s)	(EXT2_BLOCK_SIZE(s) / sizeof (ext2_inode))
#define EXT2_ACLE_PER_BLOCK(s)		(EXT2_BLOCK_SIZE(s) / sizeof (ext2_acl_entry))
#define	EXT2_ADDR_PER_BLOCK(s)		(EXT2_BLOCK_SIZE(s) / sizeof (ULONG))

/*
 * Macro-instructions used to manage fragments
 */
#define EXT2_MIN_FRAG_SIZE		1024l
#define	EXT2_MAX_FRAG_SIZE		4096l
#define EXT2_MIN_FRAG_LOG_SIZE		  10l


#define EXT2_FRAG_SIZE(s)			( (s)->s_frag_size)
#define EXT2_FRAGS_PER_BLOCK(s)	( (s)->s_frags_per_block)




/*
 * Structure of the super block on the disk
 */

typedef struct ext2_super_block
{
	ULONG  	s_inodes_count;		/* Inodes count */
	ULONG  	s_blocks_count;		/* Blocks count */
	ULONG  	s_r_blocks_count;	/* Reserved blocks count */
	ULONG  	s_free_blocks_count;/* Free blocks count */
	ULONG  	s_free_inodes_count;/* Free inodes count */
	ULONG  	s_first_data_block;	/* First Data Block */
	ULONG  	s_log_block_size;	/* Block size */
	LONG    s_log_frag_size;	/* Fragment size */
	ULONG  	s_blocks_per_group;	/* # Blocks per group */
	ULONG  	s_frags_per_group;	/* # Fragments per group */
	ULONG  	s_inodes_per_group;	/* # Inodes per group */
	ULONG  	s_mtime;			/* Mount time */
	ULONG  	s_wtime;			/* Write time */
	USHORT 	s_mnt_count;		/* Mount count */
	SHORT   s_max_mnt_count;	/* Maximal mount count */
	USHORT 	s_magic;			/* Magic signature */
	USHORT 	s_state;			/* File system state */
	USHORT 	s_errors;			/* Behaviour when detecting errors */
	USHORT 	s_pad;
	ULONG  	s_lastcheck;		/* time of last check */
	ULONG  	s_checkinterval;	/* max. time between checks */
	ULONG  	s_creator_os;		/* OS */
	ULONG  	s_rev_level;		/* Revision level */
	ULONG  	s_reserved[236];	/* Padding to the end of the block */
} ext2_super_block;


/*
 * Special inodes numbers
 */
#define	EXT2_BAD_INO		 1l	/* Bad blocks inode */
#define EXT2_ROOT_INO		 2l	/* Root inode */
#define EXT2_ACL_IDX_INO	 3l	/* ACL inode */
#define EXT2_ACL_DATA_INO	 4l	/* ACL inode */
#define EXT2_BOOT_LOADER_INO 5l	/* Boot loader inode */
#define EXT2_UNDEL_DIR_INO   6l /* Undelete directory inode */
#define EXT2_FIRST_INO		11l	/* First non reserved inode */


/*
 * Structure of an inode on the disk
 * Please do not remove the union by removing the struct definiftions
 * for other OS's as it affects the size of struct ext2_inode
 */

typedef struct ext2_inode
{
	USHORT 	i_mode;				/* File mode */
	USHORT 	i_uid;				/* Owner Uid */
	ULONG  	i_size;				/* Size in bytes */
	ULONG  	i_atime;			/* Access time */
	ULONG  	i_ctime;			/* Creation ’í’ÿ’ÿ’ÿ’ÿ’ÿ’ÿ’ÿtime */
	ULONG  	i_mtime;			/* Modification time */
	ULONG  	i_dtime;			/* Deletion Time */
	USHORT 	i_gid;				/* Group Id */
	USHORT 	i_links_count;		/* Links count */
	ULONG  	i_blocks;			/* Blocks count */
	ULONG  	i_flags;			/* File flags */
	union
	{
		struct
		{
			ULONG  l_i_reserved1;
		} linux1;
		struct
		{
			ULONG  h_i_translator;
		} hurd1;
		struct
		{
			ULONG  m_i_reserved1;
		} masix1;
	} osd1;							/* OS dependent 1 */

	ULONG  	i_block[EXT2_N_BLOCKS];	/* Pointers to blocks */
	ULONG  	i_version;				/* File version (for NFS) */
	ULONG  	i_file_acl;				/* File ACL */
	ULONG  	i_dir_acl;				/* Directory ACL */
	ULONG  	i_faddr;				/* Fragment address */
	union
	{
		struct
		{
			UCHAR	l_i_frag;		/* Fragment number */
			UCHAR	l_i_fsize;		/* Fragment size */
			USHORT	i_pad1;
			ULONG	l_i_reserved2[2];
		} linux2;
		struct
		{
			UCHAR	h_i_frag;		/* Fragment number */
			UCHAR	h_i_fsize;		/* Fragment size */
			USHORT	h_i_mode_high;
			USHORT	h_i_uid_high;
			USHORT	h_i_gid_high;
			ULONG	h_i_author;
		} hurd2;
		struct
		{
			UCHAR	m_i_frag;		/* Fragment number */
			UCHAR	m_i_fsize;		/* Fragment size */
			USHORT	m_pad1;
			ULONG	m_i_reserved2[2];
		} masix2;
	} osd2;							/* OS dependent 2 */
} ext2_inode;

#define i_reserved1	osd1.linux1.l_i_reserved1
#define i_frag		osd2.linux2.l_i_frag
#define i_fsize		osd2.linux2.l_i_fsize
#define i_reserved2	osd2.linux2.l_i_reserved2


/*
 * Structure of a directory entry
 */
#define EXT2_NAME_LEN 255

typedef struct 
{
	ULONG	inode;			/* Inode number */
	USHORT  rec_len;		/* Directory entry length */
	USHORT  name_len;		/* Name length */
	char    name[EXT2_NAME_LEN];	/* File name */
} ext2_dir_entry;



typedef struct ext2_group_desc
{
	ULONG	bg_block_bitmap;		/* Blocks bitmap block */
	ULONG	bg_inode_bitmap;		/* Inodes bitmap block */
	ULONG	bg_inode_table;			/* Inodes table block */
	USHORT	bg_free_blocks_count;	/* Free blocks count */
	USHORT	bg_free_inodes_count;	/* Free inodes count */
	USHORT  bg_used_dirs_count;		/* Directories count */
	USHORT  bg_pad;
	ULONG	bg_reserved[3];
} ext2_group_desc;



/*
 * ACL structures
 */
typedef struct
{
	ULONG aclh_size;
	ULONG aclh_file_count;
	ULONG aclh_acle_count;
	ULONG aclh_first_acle;
} ext2_acl_header;	/* Header of Access Control Lists */

typedef struct
{
	ULONG  acle_size;
	USHORT acle_perms;	/* Access permissions */
	USHORT acle_type;	/* Type of entry */
	USHORT acle_tag;	/* User or group identity */
	USHORT acle_pad1;
	ULONG  acle_next;	/* Pointer on next entry for the */
						/* same inode or on next free entry */
} ext2_acl_entry;		/* Access Control List Entry */

/*
 * The following is not needed anymore since the descriptors buffer
 * heads are now dynamically allocated
 */
/* #define EXT2_MAX_GROUP_DESC	8 */

#define EXT2_MAX_GROUP_LOADED	8



/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/

ULONG	Ext2BlockMap(struct _Inode *Inode, ULONG BlockNo);
BOOL	Ext2ReadInode(struct _Inode *Inode);
BOOL	Ext2GetSuperBlock(struct _SuperBlock *Super);
void	Ext2ReleaseSuperBlock(struct _SuperBlock *Super);

#endif		// __EXT2FS_H
