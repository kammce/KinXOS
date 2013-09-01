#include "vxd.h"

#include "iop.h"
#include "ddb.h"

#include "shared\vxddebug.h"
#include "shared\partwalk.h"
#include "shared\parttype.h"


/********************************
 *
 * STATIC HELPERS + INLINERS
 *
 *********************************/

static void InsertPortDriverReqHandler(DCB_COMMON *DcbLinux)
{
	int		i;
	struct	_DCB_cd_entry	*CallDownEntry;

			/*
			 * Search for the last entry
			 */
	CallDownEntry = ((DCB_COMMON *) DcbLinux->DCB_physical_dcb)->DCB_ptr_cd;
	do
	{
			/*
			 * Yep, this is the port driver's entry!
			 * Use it to hang th port driver in our DCB
			 */
		IspInsertCalldown
			(
				(DCB *) DcbLinux, 
				CallDownEntry->DCB_cd_io_address, 
				CallDownEntry->DCB_cd_ddb,
				//((PDCB_COMMON) dcb)->DCB_ptr_cd->DCB_cd_ddb,
				0,
				//((PDCB_COMMON) dcb->DCB_physical_dcb)->DCB_dmd_flags,
				//((PDCB_COMMON) dcb)->DCB_dmd_flags,
				CallDownEntry->DCB_cd_flags,
				CallDownEntry->DCB_cd_lgn
			);
		CallDownEntry = CallDownEntry->DCB_cd_next;

	} while(CallDownEntry->DCB_cd_next);

}


typedef struct _name_assoc {
	BYTE _func;
	const char* _name;
} NAMETABLE;


const char* GetName(NAMETABLE nameTable[], int n, ULONG func)
{

	int i;

	for (i = 0; i < n; i++)
		if (func == nameTable[i]._func)
			return nameTable[i]._name;

	return "<unknown>";
}

const CHAR* GetIOFunctionName(ULONG func)
{
	
	static NAMETABLE nameTable[] = {
		{IOR_READ, "IOR_READ"},
		{IOR_WRITE, "IOR_WRITE"},
		{IOR_VERIFY, "IOR_VERIFY"},
		{IOR_CANCEL, "IOR_CANCEL"},
		{IOR_WRITEV, "IOR_WRITEV"},
		{IOR_MEDIA_CHECK, "IOR_MEDIA_CHECK"},
		{IOR_MEDIA_CHECK_RESET, "IOR_MEDIA_CHECK_RESET"},
		{IOR_LOAD_MEDIA, "{IOR_IOR_LOAD_MEDIA"},
		{IOR_EJECT_MEDIA, "IOR_EJECT_MEDIA"},
		{IOR_LOCK_MEDIA, "IOR_LOCK_MEDIA"},
		{IOR_UNLOCK_MEDIA, "IOR_UNLOCK_MEDIA"},
		{IOR_REQUEST_SENSE, "IOR_REQUEST_SENSE"},
		{IOR_COMPUTE_GEOM, "IOR_COMPUTE_GEOM"},
		{IOR_GEN_IOCTL, "IOR_GEN_IOCTL"},
		{IOR_FORMAT, "IOR_FORMAT"},
		{IOR_SCSI_PASS_THROUGH, "IOR_SCSI_PASS_THROUGH"},
		{IOR_CLEAR_QUEUE, "IOR_CLEAR_QUEUE"},
		{IOR_DOS_RESET, "IOR_DOS_RESET"},
		{IOR_SCSI_REQUEST, "IOR_SCSI_REQUEST"},
		{IOR_SET_WRITE_STATUS, "IOR_SET_WRITE_STATUS"},
		{IOR_RESTART_QUEUE, "IOR_RESTART_QUEUE"},
		{IOR_ABORT_QUEUE, "IOR_ABORT_QUEUE"},
		{IOR_SPIN_DOWN, "IOR_SPIN_DOWN"},
		{IOR_SPIN_UP, "IOR_SPIN_UP"},
		{IOR_FLUSH_DRIVE, "IOR_FLUSH_DRIVE"},
		{IOR_FLUSH_DRIVE_AND_DISCARD, "IOR_FLUSH_DRIVE_AND_DISCARD"},
		{IOR_FSD_EXT, "IOR_FSD_EXT"}
	};

#define NIOCOMMANDS (sizeof(nameTable)/sizeof(NAMETABLE))

	return GetName(nameTable, NIOCOMMANDS, func);
}













/*
 * DoCallDown, copied from Walter Oney's System Programmer Ramdisk
 * example
 * DoCallDown passes a request to the next lower layer. Note that the
 * documentation about how to do this is totally wrong: you don't just
 * add sizeof(DCB_cd_entry) to the calldown pointer, you follow a
 * linked list from one calldown entry to the next.
 */

static void __declspec(naked) DoCallDown(PIOP Iop)
{
}


/*
 *
 */
static void __cdecl TSDEXT2_RequestHandler(PIOP Iop)
{
	DCB_COMMON	*Dcb = (DCB_COMMON *) Iop->IOP_original_dcb;
	ULONG		StartSector;	


	VxdDebugPrint(D_TSD, "TSDEXT2_RequestHandler: func=%s", GetIOFunctionName(Iop->IOP_ior.IOR_func));
	Iop->IOP_ior.IOR_status = IORS_INVALID_COMMAND;
	return;
			/*
			 * Ok, if our logical DCB is called with a start
			 * sector relative to the start of the logical
			 * partition, we translate it into a absolute disk
			 * sector.
			 */
	switch(Iop->IOP_ior.IOR_func)
	{
		case IOR_READ:
		case IOR_WRITE:
		case IOR_VERIFY:
		case IOR_WRITEV:
			if (	(Dcb->DCB_partition_type == TYPE_LINUX_PARTITION) &&
					(Iop->IOP_ior.IOR_flags & IORF_LOGICAL_START_SECTOR) &&
					!(Iop->IOP_ior.IOR_flags & IORF_PARTITION_BIAS_ADDED))
			{
				Iop->IOP_ior.IOR_start_addr[0] += Dcb->DCB_Partition_Start;
				Iop->IOP_ior.IOR_flags |= IORF_PARTITION_BIAS_ADDED;
			}
			break;
	}


	//Iop->IOP_calldown_ptr = Iop->IOP_calldown_ptr->DCB_cd_next;
	//VxdDebugPrint(D_TSD, "TSDEXT2_RequestHandler: nextfunc=%x", (ULONG) Iop->IOP_calldown_ptr->DCB_cd_io_address);
	//((PFNIOP)Iop->IOP_calldown_ptr->DCB_cd_io_address)(Iop);
//	DoCallDown(Iop);
	_asm
	{
		mov	ecx, [esp+4]
		mov	eax, [ecx]IOP.IOP_calldown_ptr
		mov eax, [eax]DCB_cd_entry.DCB_cd_next
		mov [ecx]IOP.IOP_calldown_ptr, eax
		jmp [eax]DCB_cd_entry.DCB_cd_io_address
	}
}


#ifdef AAP
/*
 * Do not allow a client to call us directly
 */
static VOID __cdecl TSDEXT2_RequestHandler(IOP* pIop)
{
	IOR* pIor = &pIop->IOP_ior;
	pIor->IOR_status = IORS_INVALID_COMMAND;	// assume error

		/*
		 * Invoke the completion handler. First pop off the empty 
		 * entry at the top of the stack. Note: IOP_callback_ptr is 
		 * type as IOP_callback_entry*, not ULONG when using VtoolsD.
		 */
	--pIop->IOP_callback_ptr;	
	pIop->IOP_callback_ptr->IOP_CB_address(pIop);
}
#endif


#define TAKE_FROM_PARENT(me,parent,value) me->value = parent->value
/*
 * This is the callback passed to WalkPartitions. It is called for
 * each partition found on the specified disk.
 */
static void HandlePartition(PDCB PhysDcb, int Disk, TDiskPartition *DiskPartition, int PartitionNo)
{
	DCB_COMMON *DcbLinux;
	DCB_COMMON *DcbPhysical = (DCB_COMMON *) PhysDcb;
	

	if (DiskPartition->sys_ind == TYPE_LINUX_PARTITION)
	{

		VxdDebugPrint(D_PARTITION, "HandlePartition, found linux partition, startsec=%lu, nrsec=%lu, parentdcb= %x", (ULONG) DiskPartition->start_sec, (ULONG) DiskPartition->nr_sec, DcbPhysical);
	
			/*
			 * Create the DCB
			 */
		DcbLinux = (DCB_COMMON *) IspCreateDcb(sizeof(DCB_COMMON));

		DcbLinux->DCB_physical_dcb = (ULONG) PhysDcb;
		TAKE_FROM_PARENT(DcbLinux, DcbPhysical, DCB_next_logical_dcb);
		TAKE_FROM_PARENT(DcbLinux, DcbPhysical, DCB_device_type);
		//DcbLinux->DCB_device_flags = (DcbPhysical->DCB_device_flags & ~(DCB_DEV_PHYSICAL | DCB_DEV_WRITEABLE) ) | DCB_DEV_LOGICAL | DCB_DEV_MUST_CONFIGURE;
		DcbPhysical->DCB_next_logical_dcb = DcbLinux;
		DcbLinux ->DCB_Partition_Start = DiskPartition->start_sec;
		DcbLinux ->DCB_partition_type = 131;
		DcbLinux->DCB_device_flags = (DcbPhysical->DCB_device_flags & ~(DCB_DEV_PHYSICAL | DCB_DEV_WRITEABLE) ) | DCB_DEV_MUST_CONFIGURE | DCB_DEV_LOGICAL;		
		//DcbLinux->DCB_device_flags = (DcbPhysical->DCB_device_flags & ~(DCB_DEV_PHYSICAL | DCB_DEV_WRITEABLE) ) | DCB_DEV_LOGICAL;		
		VxdDebugPrint(D_PARTITION, "HandlePartition, done, newdcb=%x", DcbLinux);
	}
}


static void HandlePartitionObsolete(PDCB PhysDcb, int Disk, TDiskPartition *DiskPartition, int PartitionNo)
{
	DCB_COMMON *DcbLinux;
	DCB_COMMON *DcbPhysical = (DCB_COMMON *) PhysDcb;
	

	if (DiskPartition->sys_ind == TYPE_LINUX_PARTITION)
	{

		VxdDebugPrint(D_PARTITION, "HandlePartition, found linux partition, startsec=%lu, nrsec=%lu, parentdcb= %x", (ULONG) DiskPartition->start_sec, (ULONG) DiskPartition->nr_sec, DcbPhysical);
	
			/*
			 * Create the DCB
			 */
		DcbLinux = (DCB_COMMON *) IspCreateDcb(sizeof(DCB_COMMON));

			/*
			 * Hang into parent and set the fields I'm convinced
			 * we _have_ to set
			 */
		DcbLinux->DCB_physical_dcb = (ULONG) PhysDcb;
		TAKE_FROM_PARENT(DcbLinux, DcbPhysical, DCB_next_logical_dcb);
		TAKE_FROM_PARENT(DcbLinux, DcbPhysical, DCB_device_type);
		//DcbLinux->DCB_device_flags = (DcbPhysical->DCB_device_flags & ~(DCB_DEV_PHYSICAL | DCB_DEV_WRITEABLE) ) | DCB_DEV_LOGICAL | DCB_DEV_MUST_CONFIGURE;
		DcbPhysical->DCB_next_logical_dcb = DcbLinux;
		DcbLinux ->DCB_Partition_Start = DiskPartition->start_sec;
		DcbLinux ->DCB_partition_type = 131;
		//DcbLinux->DCB_device_flags = (DcbPhysical->DCB_device_flags & ~(DCB_DEV_PHYSICAL | DCB_DEV_WRITEABLE) ) | DCB_DEV_MUST_CONFIGURE | DCB_DEV_LOGICAL;		
		DcbLinux->DCB_device_flags = (DcbPhysical->DCB_device_flags & ~(DCB_DEV_PHYSICAL | DCB_DEV_WRITEABLE) ) | DCB_DEV_LOGICAL;		
		DcbLinux->DCB_ptr_cd=0;

			/*
			 * This is the story of my life:
			 * We have to insert the port driver's request handler itself
			 * here.
			 */
		InsertPortDriverReqHandler(DcbLinux);


	//DcbLinux->DCB_dmd_flags = (DcbPhysical->DCB_dmd_flags | DCB_dmd_logical) & ~DCB_dmd_physical;
	//DcbLinux->DCB_dmd_flags = DCB_dmd_logical;


	DcbLinux->DCB_unit_number = 0;
	DcbLinux->DCB_TSD_Flags = 0;
	DcbLinux->DCB_device_flags2 = 0;
	DcbLinux->DCB_user_drvlet = 0xffff;
	DcbLinux->DCB_track_table_ptr = 0;
	//DcbLinux->DCB_disk_bpb_flags = DCBF_DISK_BPB_USEFAKE;
	//DcbLinux->DCB_disk_bpb_flags = 0;
	//DcbLinux->DCB_bds_ptr = 0;
	DcbLinux->DCB_cAssoc = 0xFF;
	//DcbLinux->DCB_cAssoc = 0;

			/*
			 * Set some other fields....
			 */
		//DcbLinux->DCB_expansion_length = 0;
		//DcbLinux->DCB_unit_number = 0;
		//DcbLinux->DCB_vrp_ptr = 0;
		//DcbLinux ->DCB_track_table_ptr = 0;
		//DcbLinux->DCB_user_drvlet = 0xffff;
		//DcbLinux->DCB_bds_ptr=0;
		//DcbLinux->DCB_cAssoc = 0xFF;
		
		//TAKE_FROM_PARENT(DcbLinux, DcbPhysical, DCB_unit_number);
		//TAKE_FROM_PARENT(DcbLinux, DcbPhysical, DCB_TSD_Flags);
		
		//TAKE_FROM_PARENT(DcbLinux, DcbPhysical, DCB_device_flags2);
		//TAKE_FROM_PARENT(DcbLinux, DcbPhysical, DCB_disk_bpb_flags);




		VxdDebugPrint(D_PARTITION, "HandlePartition, done, newdcb=%x", DcbLinux);
	}
}

#define TAKE_FROM_PARENT(me,parent,value) me->value = parent->value
/*
 * This is the callback passed to WalkPartitions. It is called for
 * each partition found on the specified disk.
 */
static void HandleAssociateDcb(PDCB PhysDcb)
{
	DCB_COMMON *DcbLinux;
	DCB_COMMON *DcbPhysical = (DCB_COMMON *) PhysDcb;
	

	VxdDebugPrint(D_PARTITION, "HandleAssociateDcb, start");
	
			/*
			 * Create the DCB
			 */
	DcbLinux = (DCB_COMMON *) IspCreateDcb(sizeof(DCB_COMMON));

		DcbLinux->DCB_physical_dcb = (ULONG) PhysDcb;
		TAKE_FROM_PARENT(DcbLinux, DcbPhysical, DCB_next_logical_dcb);
		TAKE_FROM_PARENT(DcbLinux, DcbPhysical, DCB_device_type);
		//DcbLinux->DCB_device_flags = (DcbPhysical->DCB_device_flags & ~(DCB_DEV_PHYSICAL | DCB_DEV_WRITEABLE) ) | DCB_DEV_LOGICAL | DCB_DEV_MUST_CONFIGURE;
		DcbPhysical->DCB_next_logical_dcb = DcbLinux;
		DcbLinux ->DCB_Partition_Start = 130;
		DcbLinux ->DCB_partition_type = 131;
		DcbLinux->DCB_device_flags = (DcbPhysical->DCB_device_flags & ~(DCB_DEV_PHYSICAL | DCB_DEV_WRITEABLE) ) | DCB_DEV_MUST_CONFIGURE | DCB_DEV_LOGICAL;		
		//DcbLinux->DCB_device_flags = (DcbPhysical->DCB_device_flags & ~(DCB_DEV_PHYSICAL | DCB_DEV_WRITEABLE) ) | DCB_DEV_LOGICAL;		
	VxdDebugPrint(D_PARTITION, "HandleAssociateDcb, end");
}

/*********************************
 *
 * INTERFACE ROUNTINES
 *
 **********************************/

VOID __cdecl TSDEXT2_Aer(AEP* pAep)
{
	DCB_COMMON			*dcb;

	switch (pAep->AEP_func)
	{
		case AEP_INITIALIZE:
			VxdDebugPrint(D_TSD, "AER: AEP_INITIALIZE, ILB_service_rtn=0x%x", (ULONG) theILB.ILB_service_rtn);
			SETDEBUGLEVEL(DBG_HINT, DBG_HINT);
			pAep->AEP_result = AEP_SUCCESS;
			break;

		case AEP_CONFIG_DCB:
			dcb = ((AEP_dcb_config*)pAep)->AEP_d_c_dcb;
			VxdDebugPrint(D_TSD, "AER: AEP_CONFIG_DCB for %s DCB %x", 
					dcb->DCB_device_flags & DCB_DEV_PHYSICAL ? "physical" : "logical",
					dcb);

			if ((dcb->DCB_partition_type == TYPE_LINUX_PARTITION))
			{
				VxdDebugPrint(D_TSD, "AER: AEP_CONFIG_DCB inserting ourselves in Calldown chain"); 

				IspInsertCalldown
					(
						(DCB *) dcb, 
						TSDEXT2_RequestHandler, 
						((PDCB_COMMON) dcb->DCB_physical_dcb)->DCB_ptr_cd->DCB_cd_ddb,
						//((PDCB_COMMON) dcb)->DCB_ptr_cd->DCB_cd_ddb,
						0,
						//((PDCB_COMMON) dcb->DCB_physical_dcb)->DCB_dmd_flags,
						//((PDCB_COMMON) dcb)->DCB_dmd_flags,
						//((PDCB_COMMON) dcb)->DCB_ptr_cd->DCB_cd_flags,
						0,
						((AEP_dcb_config*)pAep)->AEP_d_c_hdr.AEP_lgn
					);

			}

			pAep->AEP_result = AEP_SUCCESS;
			break;

		case AEP_BOOT_COMPLETE:
			VxdDebugPrint(D_TSD, "AER: AEP_BOOT_COMPLETE");
			pAep->AEP_result = AEP_SUCCESS ;
			break;

		case AEP_ASSOCIATE_DCB:
			dcb = ((AEP_assoc_dcb*)pAep)->AEP_a_d_pdcb;
			VxdDebugPrint(D_TSD, "AER: AEP_ASSOCIATE_DCB for DCB %x", dcb);
			
			WalkPartitions((PDCB) dcb, 0, HandlePartition);

			VxdDebugPrint(D_TSD, "AER: AEP_ASSOCIATE_DCB done");
			pAep->AEP_result = AEP_SUCCESS ;
			break;

		default:
			VxdDebugPrint(D_TSD, "AER: func=%lu", (ULONG) pAep->AEP_func);
			break;
	}
}


