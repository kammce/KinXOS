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

#define TAKE_FROM_PARENT(me,parent,value) me->value = parent->value

static void SetupLinuxDcb(PDCB Dcb)
{
	
	DCB_COMMON *DcbLinux = (DCB_COMMON *) Dcb;
	DCB_COMMON *DcbPhysical = (DCB_COMMON *) DcbLinux->DCB_physical_dcb;
	
	VxdDebugPrint(D_TSD, "SetupDCB, me=%x, parent = %x", pDcb, DcbPhysical);
	
	//TAKE_FROM_PARENT(DcbLinux, pDcbPhysical, DCB_expansion_length);
	//TAKE_FROM_PARENT(DcbLinux, pDcbPhysical, DCB_ptr_cd);
	DcbLinux->DCB_expansion_length = 0;
	
	//TAKE_FROM_PARENT(DcbLinux, pDcbPhysical, DCB_unit_number);
	DcbLinux->DCB_unit_number = 0;
	TAKE_FROM_PARENT(DcbLinux, DcbPhysical, DCB_TSD_Flags);
	DcbLinux->DCB_vrp_ptr = 0;
	TAKE_FROM_PARENT(DcbLinux, DcbPhysical, DCB_dmd_flags);
	DcbLinux->DCB_device_flags = (DcbPhysical->DCB_device_flags & ~(DCB_DEV_MUST_CONFIGURE | DCB_DEV_PHYSICAL | DCB_DEV_WRITEABLE) ) | DCB_DEV_LOGICAL;
	TAKE_FROM_PARENT(DcbLinux, DcbPhysical, DCB_device_flags2);
	
	DcbLinux ->DCB_track_table_ptr = 0;
	TAKE_FROM_PARENT(DcbLinux, DcbPhysical, DCB_device_type);
	DcbLinux->DCB_user_drvlet = 0xffff;
	TAKE_FROM_PARENT(DcbLinux, DcbPhysical, DCB_disk_bpb_flags);
	DcbLinux->DCB_bds_ptr=0;
	DcbLinux->DCB_cAssoc = 0xFF;

	VxdDebugPrint(D_TSD, "SetupDCB, done");
}


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

		DcbLinux->DCB_physical_dcb = (unsigned long) PhysDcb;
		//TAKE_FROM_PARENT(DcbLinux, PhysDcb, DCB_expansion_length);
		//TAKE_FROM_PARENT(DcbLinux, PhysDcb, DCB_ptr_cd);
		TAKE_FROM_PARENT(DcbLinux, DcbPhysical, DCB_next_logical_dcb);

		DcbLinux->DCB_device_flags = (DcbPhysical->DCB_device_flags & ~(DCB_DEV_PHYSICAL | DCB_DEV_WRITEABLE) ) | DCB_DEV_LOGICAL;
		DcbPhysical->DCB_next_logical_dcb = DcbLinux;
		//DcbLinux->DCB_drive_lttr_equiv = 'Q' - 'A';
		DcbLinux ->DCB_Partition_Start = DiskPartition->start_sec;
		DcbLinux ->DCB_partition_type = 131;
		VxdDebugPrint(D_PARTITION, "HandlePartition, done, newdcb=%x", DcbLinux);
	}
}



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

		DcbLinux->DCB_sig = 0x4342;
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

				SetupLinuxDcb((PDCB) dcb);
				IspInsertCalldown
					(
						dcb, 
						TSDEXT2_RequestHandler, 
						((PDCB_COMMON) dcb->DCB_physical_dcb)->DCB_ptr_cd->DCB_cd_ddb,
						0,
						((PDCB_COMMON) dcb->DCB_physical_dcb)->DCB_dmd_flags,
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


