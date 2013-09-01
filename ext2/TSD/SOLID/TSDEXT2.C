// FSDEXT2.c - main module for VxD FSDEXT2

#define   DEVICE_MAIN
#include  "tsdext2.h"
#undef    DEVICE_MAIN


Declare_Layered_Driver(TSDEXT2, TSDEXT2_LGN_MASK, "123456789abcdef", 1, 0, 0)

DefineControlHandler(SYS_DYNAMIC_DEVICE_INIT, OnSysDynamicDeviceInit);

BOOL __cdecl ControlDispatcher(
	DWORD dwControlMessage,
	DWORD EBX,
	DWORD EDX,
	DWORD ESI,
	DWORD EDI,
	DWORD ECX)
{
	START_CONTROL_DISPATCH

		ON_SYS_DYNAMIC_DEVICE_INIT(OnSysDynamicDeviceInit);

	END_CONTROL_DISPATCH

	return TRUE;
}


BOOL OnSysDynamicDeviceInit()
{
	
	dprintf("TSDEXT2: Here we go, ILB=%x!!!!\r\n", &TSDEXT2_Ilb);
	IOS_Register(&TSDEXT2_Drp);
	dprintf("TSDEXT2: registration result=%04x\r\n", TSDEXT2_Drp.DRP_reg_result);

	return TRUE;
}


/*
 * Do not allow client to call us directly
 */
VOID __cdecl TSDEXT2_RequestHandler(IOP* pIop)
{
	IOR* pIor = &pIop->IOP_ior;
	pIor->IOR_status = IORS_INVALID_COMMAND;	// assume error

// Invoke the completion handler. First pop off the empty entry at the
// top of the stack. Note: IOP_callback_ptr is type as IOP_callback_entry*,
// not ULONG when using VtoolsD.

	--pIop->IOP_callback_ptr;	
	pIop->IOP_callback_ptr->IOP_CB_address(pIop);
}

// Async Event Routine


VOID __cdecl TSDEXT2_Aer(AEP* pAep)
{
	DCB_COMMON* pDcb;
	ISP_calldown_insert InsertISP;

	switch (pAep->AEP_func)
	{
		case AEP_INITIALIZE:
			dprintf("TSDEXT2: AEP_Initialise\r\n");
			pAep->AEP_result = AEP_SUCCESS;
			break;

		case AEP_CONFIG_DCB:
			pDcb = ((AEP_dcb_config*)pAep)->AEP_d_c_dcb;

			if ( (pDcb->DCB_device_flags & DCB_DEV_PHYSICAL))
			{
				dprintf("TSDEXT2: AEP_CONFIG_DCB for physical DCB %x\r\n", pDcb);
			}
			else
			{
				dprintf("TSDEXT2: AEP_CONFIG_DCB for logical DCB %x, drive=%c (type=%i)\r\n", pDcb, pDcb->DCB_drive_lttr_equiv+'A', pDcb->DCB_partition_type);
				
				if ((pDcb->DCB_partition_type == 131))
				{
					SetupLinuxDcb(pDcb);
					IspInsertCalldown
						(
							pDcb, 
							TSDEXT2_RequestHandler, 
							((PDCB_COMMON) pDcb->DCB_physical_dcb)->DCB_ptr_cd->DCB_cd_ddb,
							0,
							((PDCB_COMMON) pDcb->DCB_physical_dcb)->DCB_dmd_flags,
							((AEP_dcb_config*)pAep)->AEP_d_c_hdr.AEP_lgn
						);
				}
				
			}
			pAep->AEP_result = AEP_SUCCESS;
			break;

		case AEP_BOOT_COMPLETE:
			pAep->AEP_result = AEP_SUCCESS ;
			break;

		case AEP_ASSOCIATE_DCB:
			pDcb = ((AEP_assoc_dcb*)pAep)->AEP_a_d_pdcb;
			dprintf("TSDEXT2: AEP_ASSOCIATE_DCB for DCB %x\r\n", pDcb);
			dprintf("TSDEXT2: walking partition table\r\n", pDcb);
			DCB_WalkPartitions(pDcb);
			dprintf("TSDEXT2: done walking partition table\r\n", pDcb);
			pAep->AEP_result = AEP_SUCCESS ;
			break;

		default:
			break;
	}
}



