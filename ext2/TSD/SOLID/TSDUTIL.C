#include "tsdext2.h"

#include "iop.h"
#include "ddb.h"

#include "tsdutil.h"


#define TYPE_EXT_PARTITION 5
#define TYPE_NATIVE_LINUX_PARTITION 131

typedef struct 
{
	UCHAR 		boot_ind;		/* 0x80 - active */
	UCHAR 		head;			/* starting head */
	UCHAR 		sector;			/* starting sector */
	UCHAR 		cyl;			/* starting cylinder */
	UCHAR 		sys_ind;		/* What partition type */
	UCHAR 		end_head;		/* end head */
	UCHAR 		end_sector;		/* end sector */
	UCHAR 		end_cyl;		/* end cylinder */
	ULONG 		start_sec;		/* starting sector counting from 0 */
	ULONG 		nr_sec;			/* nr of sectors in partition */
} partition_t;

static char		MBR[512];
static int		fs_type_list[]={1,4,5,6,7,0x0A,0x51,0x64,0x80,0x81,0x82,0x83,0x93,0x94,0};
static char		*fs_type_name[]={"DOS 12-bit FAT","DOS 16-bit <32M", "Extended",
					  "DOS 16-bit >=32", "OS/2 HPFS","OS/2 Boot Manag",
					  "Novell?","Novell", "Old MINIX","Linux/MINIX",
					  "Linux swap","Linux native", "Amoeba","Amoeba BBT",
					  "Unknown"};


/********************************
 *
 * STATIC HELPERS + INLINERS
 *
 *********************************/


static char *find_fstype(int type)
{
	int	i=0;
	for(i=0; fs_type_list[i] && fs_type_list[i]!=type ; i++)
		;
	return fs_type_name[i];
}

static void ReadSector(PDCB dcb, unsigned long start_sector, unsigned long count_sector, char *block)
{
	
	USHORT offset = (USHORT) (dcb->DCB_cmn.DCB_expansion_length + FIELDOFFSET(IOP, IOP_ior));
	USHORT size = offset + sizeof(IOR) + dcb->DCB_max_sg_elements * sizeof(SGD);
	PIOP iop = IspCreateIop(size, offset, ISP_M_FL_MUST_SUCCEED);
	PIOR ior = &iop->IOP_ior;

	// Before submitting an internal request, the two DCB pointer fields
	// of the IOP must be completed. The criteria routine barfs otherwise.

	iop->IOP_original_dcb = (ULONG) dcb;
	iop->IOP_physical_dcb = (ULONG) dcb->DCB_cmn.DCB_physical_dcb;

	// Fill in required fields in the IOR. The earlier remarks about
	// external users apply here too. The main difference is how we
	// fill in the volume handle and "designator"

	ior->IOR_next = 0;					// used for request queueing by lots of downstream people
	ior->IOR_start_addr[1] = 0;			// assume only 32-bits worth of sector number
	ior->IOR_flags = IORF_VERSION_002;	// this is how IOS knows we're giving it a new-style IOR
	ior->IOR_private_client = offset;	// a common use for this field
	ior->IOR_req_vol_handle = dcb->DCB_cmn.DCB_vrp_ptr;
	ior->IOR_sgd_lin_phys = (ULONG) (ior + 1); // where scatter/gather descriptors can be built
	ior->IOR_num_sgds = 0;				// incremented by criteria routine, so must start at zero
	ior->IOR_vol_designtr = dcb->DCB_cmn.DCB_unit_number;

	// Fill in request-specific info. This is just like the external
	// client does it, except that an internal client would get
	// the partition starting sector from the DCB:

	ior->IOR_func = IOR_READ;			// flag as read request
	ior->IOR_flags |= IORF_SYNC_COMMAND // don't return until complete
		| IORF_HIGH_PRIORITY			// used for adminstrative-type requests, but optional
		| IORF_BYPASS_VOLTRK;			// we want to bypass volume tracking and just read the MBR
	ior->IOR_start_addr[0] = start_sector;			// reading sector zero
	ior->IOR_buffer_ptr = (ULONG) block;	// buffer to read into
	ior->IOR_xfer_count = count_sector;			// transfer 1 sector
//	ior->IOR_start_addr[0] += dcb->DCB_cmn.DCB_Partition_Start;

	// Use the internal criteria routine to massage the request:

	IlbIntIoCriteria(iop);

	// Submit the request using the internal interface. The third argument
	// to this interface is either zero (when you want the request to start
	// at the top of the calldown stack) or the address of the request
	// routine ABOVE the one you want called.

	IlbInternalRequest(iop, dcb, NULL);

	IspDeallocMem((PVOID) ((DWORD) ior - ior->IOR_private_client));
	
	return;
}





#define TAKE_FROM_PARENT(me,parent,value) me->value = parent->value
static void MakeLinuxDcb(PDCB pDcbPhys, unsigned long start_sec)
{
	DCB_COMMON *pDCBLinux;
	DCB_COMMON *pDcbPhysical = (DCB_COMMON *) pDcbPhys;
	pDCBLinux = (DCB_COMMON *) IspCreateDcb(sizeof(DCB_COMMON));

	
		/*
		 * Hang into parent
		 */
	dprintf("TSDEXT2, MakeLinuxDCB, me=%x, parent = %x\r\n", pDCBLinux, pDcbPhysical);
	pDCBLinux->DCB_physical_dcb = pDcbPhys;
	//TAKE_FROM_PARENT(pDCBLinux, pDcbPhysical, DCB_expansion_length);
	//TAKE_FROM_PARENT(pDCBLinux, pDcbPhysical, DCB_ptr_cd);
	TAKE_FROM_PARENT(pDCBLinux, pDcbPhysical, DCB_next_logical_dcb);

	pDCBLinux->DCB_device_flags = (pDcbPhysical->DCB_device_flags & ~(DCB_DEV_PHYSICAL | DCB_DEV_WRITEABLE) ) | DCB_DEV_LOGICAL;
	pDcbPhysical->DCB_next_logical_dcb = pDCBLinux;
	pDCBLinux->DCB_drive_lttr_equiv = 'Q' - 'A';
	pDCBLinux ->DCB_Partition_Start = start_sec;
	pDCBLinux ->DCB_partition_type = 131;
	dprintf("TSDEXT2, done MakeLinuxDCB\r\n");
}


static void WalkPartitions(PDCB pDcb, char *data)
{
	partition_t		*p;
	int				i;

	// check for signature
	if (*(uint16 *) (data+510) == 0xAA55)
	{
		dprintf("\r\nTSDEXT2: Yep, this MBR has a partition table\r\n");
		p = (partition_t *) (0x1BE + data);
		for (i=1 ; i<=4 ; i++,p++)
		{
			if (!p->nr_sec)
				continue;

			dprintf("TSDEXT2: partition %i, %4lu MB, %s (%u)\r\n",
				i, p->nr_sec >> 11,
				find_fstype(p->sys_ind),(unsigned) p->sys_ind);

			if (p->sys_ind == TYPE_NATIVE_LINUX_PARTITION)
			{
				dprintf("TSDEXT2: Linux: start=%u\r\n", p->start_sec);
				MakeLinuxDcb(pDcb, p->start_sec);
			}

			if (p->sys_ind == TYPE_EXT_PARTITION)
			{
				/*
		 		 * Now handle the extended partitions. Make a new logical 
				 * sothat we can read the first partition table on it.
		 		 */
					
				//extended=read_extended_partitions(disk,extended);
				dprintf("TSDEXT2: Watch, there is ext partition\r\n");
			}
		}
	}
	else
		dprintf("TSDEXT2: TOO bad, this MBR has no partition table\r\n");

}




/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/


void DCB_WalkPartitions(PDCB pDcb)
{
	ReadSector(pDcb, 0, 1, MBR);
	WalkPartitions(pDcb, MBR);
}


void SetupLinuxDcb(PDCB pDcb)
{
	
	DCB_COMMON *pDCBLinux = pDcb;
	DCB_COMMON *pDcbPhysical = (DCB_COMMON *) pDCBLinux->DCB_physical_dcb;
	
	dprintf("TSDEXT2, SetupDCB, me=%x, parent = %x\r\n", pDcb, pDcbPhysical);
	
	//TAKE_FROM_PARENT(pDCBLinux, pDcbPhysical, DCB_expansion_length);
	//TAKE_FROM_PARENT(pDCBLinux, pDcbPhysical, DCB_ptr_cd);
	pDCBLinux->DCB_expansion_length = 0;
	
	//TAKE_FROM_PARENT(pDCBLinux, pDcbPhysical, DCB_unit_number);
	pDCBLinux->DCB_unit_number = 0;
	TAKE_FROM_PARENT(pDCBLinux, pDcbPhysical, DCB_TSD_Flags);
	pDCBLinux->DCB_vrp_ptr = 0;
	TAKE_FROM_PARENT(pDCBLinux, pDcbPhysical, DCB_dmd_flags);
	pDCBLinux->DCB_device_flags = (pDcbPhysical->DCB_device_flags & ~(DCB_DEV_MUST_CONFIGURE | DCB_DEV_PHYSICAL | DCB_DEV_WRITEABLE) ) | DCB_DEV_LOGICAL;
	TAKE_FROM_PARENT(pDCBLinux, pDcbPhysical, DCB_device_flags2);
	
	pDCBLinux ->DCB_track_table_ptr = 0;
	TAKE_FROM_PARENT(pDCBLinux, pDcbPhysical, DCB_device_type);
	pDCBLinux->DCB_user_drvlet = 0xffff;
	TAKE_FROM_PARENT(pDCBLinux, pDcbPhysical, DCB_disk_bpb_flags);
	pDCBLinux->DCB_bds_ptr=0;
	pDCBLinux->DCB_cAssoc = 0xFF;

	dprintf("TSDEXT2, done SetupDCB\r\n");
}
