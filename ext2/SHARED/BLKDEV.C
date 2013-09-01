#include "vxd.h"
#include "iosmgr.h"
#include "iop.h"
#include "ddb.h"

#include "shared\vxddebug.h"
#include "shared\blkdev.h"

/*
 * This module implements the interface to blockdevices.
 * A device is represented by a TDevice object (which is
 * a DCB).
 */

/*********************************
 *
 * STATIC HELPERS
 *
 **********************************/


/*********************************
 *
 * SECTOR BASED INTERFACE ROUNTINES
 *
 **********************************/

/*
 * Read one or more consecutive sectors from a blockdevice
 *
 * PRE CONDITIONS:
 * Device is a valid blockdevice. Data is large enough
 * to store the sector(s) in.
 * 
 * POST CONDITIONS:
 * If successfull, Data contains the read sector(s).
 *
 * IN:
 *	Device:		Valid reference to a block device
 *	StartSector:Starting sector on the block device
 *	NrSector:	Number of sectors to read
 *
 * OUT:
 *	Data:		Buffer to store the read sectors in
 *
 * RETURNS:
 * On success TRUE, FALSE otherwise
 *
 */
BOOL DevReadSectorObsolete(TDevice Device, ULONG StartSector, ULONG NrSectors, char *Data)
{
	USHORT	Rval;
	PDCB	Dcb		= (PDCB) Device;
	USHORT	Offset	= (USHORT) (Dcb->DCB_cmn.DCB_expansion_length + FIELDOFFSET(IOP, IOP_ior));
	USHORT	Size	= Offset + sizeof(IOR) + Dcb->DCB_max_sg_elements * sizeof(SGD);
	PIOP	Iop		= IspCreateIop(Size, Offset, ISP_M_FL_MUST_SUCCEED);
	PIOR	Ior		= &Iop->IOP_ior;

	VxdDebugPrint(D_BLKDEV, "DevReadSector: device=%x, sector=%lu, nr=%lu", ((ULONG) Device), StartSector, NrSectors);

	Iop->IOP_original_dcb = (struct _DCB_COMMON *) Dcb;
	Iop->IOP_physical_dcb = (struct _DCB_COMMON *) Dcb->DCB_cmn.DCB_physical_dcb;

	Ior->IOR_next = 0;					// used for request queueing by lots of downstream people
	Ior->IOR_start_addr[1] = 0;			// assume only 32-bits worth of sector number
	Ior->IOR_flags = IORF_VERSION_002;	// this is how IOS knows we're giving it a new-style IOR
	Ior->IOR_private_client = Offset;	// a common use for this field
	Ior->IOR_req_vol_handle = Dcb->DCB_cmn.DCB_vrp_ptr;
	Ior->IOR_sgd_lin_phys = (ULONG) (Ior + 1); // where scatter/gather descriptors can be built
	Ior->IOR_num_sgds = 0;				// incremented by criteria routine, so must start at zero
	Ior->IOR_vol_designtr = Dcb->DCB_cmn.DCB_unit_number;

	Ior->IOR_func = IOR_READ;			// flag as read request
	Ior->IOR_flags |= IORF_SYNC_COMMAND // don't return until complete
		| IORF_HIGH_PRIORITY			// used for adminstrative-type requests, but optional
		| IORF_BYPASS_VOLTRK;			// we want to bypass volume tracking and just read the MBR
	Ior->IOR_buffer_ptr = (ULONG) Data;	// buffer to read into

	Ior->IOR_start_addr[0] = StartSector;
	Ior->IOR_xfer_count = NrSectors;

	IlbIntIoCriteria(Iop);
	IlbInternalRequest(Iop, Dcb, NULL);

	Rval = Ior->IOR_status;

	IspDeallocMem((PVOID) ((DWORD) Ior - Ior->IOR_private_client));

	if (Rval)
	{
		VxdDebugPrint(D_BLKDEV, "DevReadSector: device=%x, sector=%lu, nr=%lu", ((ULONG) Device), StartSector, NrSectors);
		return FALSE;
	}

	VxdDebugPrint(D_BLKDEV, "DevReadSector: done, device=%x, sector=%lu, nr=%lu", ((ULONG) Device), StartSector, NrSectors);

	return TRUE;
}



/*
 * Read one or more consecutive sectors from a blockdevice
 *
 * PRE CONDITIONS:
 * Device is a valid blockdevice. Data is large enough
 * to store the sector(s) in.
 * 
 * POST CONDITIONS:
 * If successfull, Data contains the read sector(s).
 *
 * IN:
 *	Device:		Valid reference to a block device
 *	StartSector:Starting sector on the block device
 *	NrSector:	Number of sectors to read
 *
 * OUT:
 *	Data:		Buffer to store the read sectors in
 *
 * RETURNS:
 * On success TRUE, FALSE otherwise
 *
 */
BOOL DevReadSector(TDevice Device, ULONG StartSector, ULONG NrSectors, char *Data)
{
	USHORT	Rval;
	PDCB	Dcb		= (PDCB) Device;
	USHORT	Offset	= (USHORT) (Dcb->DCB_cmn.DCB_expansion_length + FIELDOFFSET(IOP, IOP_ior));
	USHORT	Size	= Offset + sizeof(IOR) + Dcb->DCB_max_sg_elements * sizeof(SGD);
	PIOP	Iop		= IspCreateIop(Size, Offset, ISP_M_FL_MUST_SUCCEED);
	PIOR	Ior		= &Iop->IOP_ior;

	VxdDebugPrint(D_BLKDEV, "DevReadSector: device=%x, sector=%lu, nr=%lu", ((ULONG) Device), StartSector, NrSectors);

	Iop->IOP_original_dcb = (struct _DCB_COMMON *) Dcb;
	Iop->IOP_physical_dcb = (struct _DCB_COMMON *) Dcb->DCB_cmn.DCB_physical_dcb;

	Ior->IOR_next = 0;					// used for request queueing by lots of downstream people
	Ior->IOR_start_addr[1] = 0;			// assume only 32-bits worth of sector number
	Ior->IOR_flags = IORF_VERSION_002;	// this is how IOS knows we're giving it a new-style IOR
	Ior->IOR_private_client = Offset;	// a common use for this field
	Ior->IOR_req_vol_handle = Dcb->DCB_cmn.DCB_vrp_ptr;
	Ior->IOR_sgd_lin_phys = (ULONG) (Ior + 1); // where scatter/gather descriptors can be built
	Ior->IOR_num_sgds = 0;				// incremented by criteria routine, so must start at zero
	Ior->IOR_vol_designtr = Dcb->DCB_cmn.DCB_unit_number;

	Ior->IOR_func = IOR_READ;			// flag as read request
	Ior->IOR_flags |= IORF_SYNC_COMMAND // don't return until complete
		| IORF_HIGH_PRIORITY			// used for adminstrative-type requests, but optional
		| IORF_BYPASS_VOLTRK;			// we want to bypass volume tracking and just read the MBR
	Ior->IOR_buffer_ptr = (ULONG) Data;	// buffer to read into

	Ior->IOR_start_addr[0] = StartSector;
	Ior->IOR_start_addr[1] = 0;
	Ior->IOR_xfer_count = NrSectors;
	if (Iop->IOP_original_dcb->DCB_device_flags & DCB_DEV_LOGICAL)
		Ior->IOR_flags |= IORF_LOGICAL_START_SECTOR;

	IlbIntIoCriteria(Iop);
	IlbInternalRequest(Iop, Dcb, NULL);

	Rval = Ior->IOR_status;

	IspDeallocMem((PVOID) ((DWORD) Ior - Ior->IOR_private_client));

	if (Rval)
	{
		VxdDebugPrint(D_BLKDEV, "DevReadSector: error, IOR_status=%lu", (ULONG) Rval);
		return FALSE;
	}

	VxdDebugPrint(D_BLKDEV, "DevReadSector: done");

	return TRUE;
}




/*********************************
 *
 * BLOCK BASED INTERFACE ROUNTINES
 *
 **********************************/

/*
 * Set the logical blocksize for a blockdevice
 *
 * PRE CONDITIONS:
 * Device is a valid blockdevice.
 *
 * POST CONDITIONS:
 * Device blocksize is set to 2 to the power LogBlockSize
 * 
 * IN:
 *	Device:			Valid reference to a block device
 *	LogBlockSize:	log of the block size (blocksize = 1 << log)
 *
 */
void DevSetLogBlockSize(TDevice Device, ULONG LogBlockSize)
{
	PDCB Dcb;

	Dcb = (PDCB) Device;

	Dcb->DCB_cmn.DCB_apparent_blk_shift = (UCHAR) LogBlockSize;

	VxdDebugPrint(D_BLKDEV, "DevSetLogBlockSize: log=%lu, size=%lu", LogBlockSize, (ULONG) (512 << LogBlockSize));
}


/*
 * Get the logical blocksize for a blockdevice
 *
 * PRE CONDITIONS:
 * Device is a valid blockdevice.
 *
 * POST CONDITIONS:
 * <none>
 * 
 * IN:
 *	Device:			Valid reference to a block device
 *
 * RETURNS:
 * The logical blocksize of the block device
 *
 */
USHORT DevGetLogBlockSize(TDevice Device)
{
	PDCB Dcb;

	Dcb = (PDCB) Device;

	return Dcb->DCB_cmn.DCB_apparent_blk_shift;
}

/*
 * Get the partition type of a blockdevice
 *
 * PRE CONDITIONS:
 * Device is a valid blockdevice.
 *
 * POST CONDITIONS:
 * <none>
 *
 * IN:
 *	Device:			Valid reference to a block device
 *
 * RETURNS:
 * The partition type
 *
 */
UCHAR DevGetPartitionType(TDevice Device)
{
	PDCB Dcb;

	Dcb = (PDCB) Device;

	VxdDebugPrint(D_BLKDEV, "DevGetPartitionType: device=%lu, type=0x%x", ((ULONG) Device), (ULONG) Dcb->DCB_cmn.DCB_partition_type);

	return Dcb->DCB_cmn.DCB_partition_type;
}



void DevGenerateName(TDevice Device, char *DevName)
{
	sprintf(DevName, "%s", "/dev/hdtest");
}



/*
 * Read a block from a blockdevice
 *
 * PRE CONDITIONS:
 * Device is a valid blockdevice. BlockData is large enough
 * to store the block in.
 * 
 * POST CONDITIONS:
 * If successfull, BlockData contains the read block.
 *
 * IN:
 *	Device:		Valid reference to a block device
 *	BlockNo:	Logical block number of the block to read
 *
 * OUT:
 *	BlockData:	Buffer to store the read block in
 *
 * RETURNS:
 * On success TRUE, FALSE otherwise
 *
 */
BOOL DevReadBlockObsolete(TDevice Device, ULONG BlockNo, UCHAR *BlockData)
{
	BOOL	Succes;
	PDCB	Dcb;
	PDCB	DcbPhysical;
	ULONG	StartSector;
	ULONG	NrSectors;

	VxdDebugPrint(D_BLKDEV, "DevReadBlock: device=%x, blockno=%lu", (ULONG) Device, BlockNo);

	Dcb = (PDCB) Device;
	DcbPhysical = (PDCB) ((PDCB_COMMON) Dcb)->DCB_physical_dcb;

	StartSector = (BlockNo << Dcb->DCB_cmn.DCB_apparent_blk_shift);
	NrSectors = (1 << Dcb->DCB_cmn.DCB_apparent_blk_shift);

		/*
		 * Account for the fact we read from the physical disk directly
		 */
	StartSector += Dcb->DCB_cmn.DCB_Partition_Start;

//	Succes = DevReadSector(DcbPhysical, StartSectorHigh, StartSectorLow, NrSectors, BlockData);
	if (!Succes)
		VxdDebugPrint(D_ERROR, "DevReadBlock: device=%x, blockno=%lu", (ULONG) Device, BlockNo);
	else		
		VxdDebugPrint(D_BLKDEV, "DevReadBlock: done, device=%x, blockno=%lu", (ULONG) Device, BlockNo);

	return Succes;	
}





/*
 * Read a block from a blockdevice
 *
 * PRE CONDITIONS:
 * Device is a valid blockdevice. BlockData is large enough
 * to store the block in.
 * 
 * POST CONDITIONS:
 * If successfull, BlockData contains the read block.
 *
 * IN:
 *	Device:		Valid reference to a block device
 *	BlockNo:	Logical block number of the block to read
 *
 * OUT:
 *	BlockData:	Buffer to store the read block in
 *
 * RETURNS:
 * On success TRUE, FALSE otherwise
 *
 */
BOOL DevReadBlock(TDevice Device, ULONG BlockNo, UCHAR *BlockData)
{
	BOOL	Succes;
	PDCB	Dcb;
	PDCB	DcbPhysical;
	ULONG	StartSector;
	ULONG	NrSectors;

	VxdDebugPrint(D_BLKDEV, "DevReadBlock: device=%x, blockno=%lu", (ULONG) Device, BlockNo);

	Dcb = (PDCB) Device;
	DcbPhysical = (PDCB) ((PDCB_COMMON) Dcb)->DCB_physical_dcb;

	StartSector = (BlockNo << Dcb->DCB_cmn.DCB_apparent_blk_shift);
	NrSectors = (1 << Dcb->DCB_cmn.DCB_apparent_blk_shift);

		/*
		 * Account for the fact we read from the physical disk directly
		 */
	StartSector += Dcb->DCB_cmn.DCB_Partition_Start;

	Succes = DevReadSector(DcbPhysical,  StartSector, NrSectors, BlockData);
	if (!Succes)
		VxdDebugPrint(D_ERROR, "DevReadBlock: device=%x, blockno=%lu", (ULONG) Device, BlockNo);
	else		
		VxdDebugPrint(D_BLKDEV, "DevReadBlock: done, device=%x, blockno=%lu", (ULONG) Device, BlockNo);

	return Succes;	
}



