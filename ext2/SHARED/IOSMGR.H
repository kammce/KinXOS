#ifndef __IOSMGR_H
#define __IOSMGR_H

#define VXDINLINE static __inline
#define NAKED __declspec(naked)

#pragma warning(disable:4047 4035)	


/*
 * This module assumes that theIlb is defined
 */

typedef pIOP PIOP;
//typedef pDDB PDDB;




VXDINLINE DWORD NAKED IlbService(PVOID isp)	// really has (PISP) arg
{
	_asm jmp [theILB.ILB_service_rtn]
}




#ifndef FIELDOFFSET
	#define FIELDOFFSET(type, field) ((DWORD)(&((type *)0)->field))
#endif



VXDINLINE DWORD NAKED IlbIntIoCriteria(PIOP iop)
{
	_asm jmp [theILB.ILB_int_io_criteria_rtn]
}
VXDINLINE VOID IlbInternalRequest(PIOP iop, PDCB dcb, PVOID calldown)
{
	_asm mov edi, iop
	_asm mov ebx, dcb
	_asm mov edx, calldown
	(* (VOID (*)()) theILB.ILB_internal_request)();
}


VXDINLINE DWORD NAKED IlbIoCriteria(PIOR ior)
{
	_asm jmp [theILB.ILB_io_criteria_rtn]
}





VXDINLINE PVOID IspAllocMem(USHORT size, USHORT type, UCHAR flags)
{
	ISP_mem_alloc isp = {{ISP_ALLOC_MEM, 0}, size, 0, type, flags};
	IlbService(&isp);
	if (isp.ISP_mem_a_hdr.ISP_result == 0)
		return (PVOID) isp.ISP_mem_ptr;
	return NULL;
}

VXDINLINE BOOL IspAssociateDcb(PDCB dcb, CHAR drive, UCHAR flags)
{
	ISP_dcb_associate isp = {{ISP_ASSOCIATE_DCB, 0}, (ULONG) dcb, drive, flags};
	IlbService(&isp);
	return isp.ISP_d_a_hdr.ISP_result == 0;
}

VXDINLINE BOOL IspBroadcastAep(PAEP aep)
{
	ISP_AEP_BROADCAST isp = {{ISP_BROADCAST_AEP, 0}, aep};
	IlbService(&isp);
	return isp.ISP_baep_hdr.ISP_result == 0;
}

VXDINLINE PDCB IspCreateDcb(USHORT size)
{
	ISP_dcb_create isp = {{ISP_CREATE_DCB, 0}, size};
	IlbService(&isp);
	if (isp.ISP_dcb_hdr.ISP_result == 0)
		return (PDCB) isp.ISP_dcb_ptr;
	return NULL;
}

VXDINLINE PDDB IspCreateDdb(USHORT size, UCHAR flags)
{
	ISP_ddb_create isp = {{ISP_CREATE_DDB, 0}, size, 0, flags};
	IlbService(&isp);
	if (isp.ISP_ddb_hdr.ISP_result == 0)
		return (PDDB) isp.ISP_ddb_ptr;
	return NULL;
}

VXDINLINE PIOP IspCreateIop(USHORT size, ULONG delta, UCHAR flags)
{
	ISP_IOP_alloc isp = {{ISP_CREATE_IOP, 0}, size, delta, 0, flags};
	IlbService(&isp);
	if (isp.ISP_i_c_hdr.ISP_result == 0)
		return (PIOP) (isp.ISP_IOP_ptr + delta - FIELDOFFSET(IOP, IOP_ior)); // ptr is to memory block, NOT the IOP (?!)
	return NULL;
}

VXDINLINE PIOR IspCreateIor(USHORT size, ULONG delta, UCHAR flags)
{
	ISP_IOP_alloc isp = {{ISP_CREATE_IOP, 0}, size, delta, 0, flags};
	IlbService(&isp);
	if (isp.ISP_i_c_hdr.ISP_result == 0)
		return (PIOR) (isp.ISP_IOP_ptr + delta); // ptr is to memory block, NOT the IOP (?!)
	return NULL;
}

VXDINLINE BOOL IspDeallocDdb(PDDB ddb)
{
	ISP_DDB_DEALLOC isp = {{ISP_DEALLOC_DDB, 0}, (ULONG) ddb, theILB.ILB_dvt};
	IlbService(&isp);
	return isp.ISP_ddb_d_hdr.ISP_result == 0;
}

VXDINLINE BOOL IspDeallocMem(PVOID p)
{
	ISP_mem_dealloc isp = {{ISP_DEALLOC_MEM, 0}, (ULONG) p};
	IlbService(&isp);
	return isp.ISP_mem_da_hdr.ISP_result == 0;
}

VXDINLINE BOOL IspDeleteLdmEntry(ULONG drive)
{
	ISP_FIND_LDM isp = {{ISP_DELETE_LDM_ENTRY, 0}, drive};
	IlbService(&isp);
	return isp.ISP_fldm_hdr.ISP_result == 0;
}

VXDINLINE BOOL IspDestroyDcb(PDCB dcb)
{
	ISP_DCB_DESTROY isp = {{ISP_DESTROY_DCB, 0}, (ULONG) dcb};
	IlbService(&isp);
	return isp.ISP_ddb_d_hdr.ISP_result == 0; // not a typo! it's really spelled "ddb_d_hdr" instead of "dcb_d_hdr"
}

VXDINLINE BOOL IspDeviceArrived(PDCB dcb, ULONG flags)
{
	ISP_DEV_ARRIVED isp = {{ISP_DEVICE_ARRIVED, 0}, dcb, flags};
	IlbService(&isp);
	return isp.ISP_d_arrived_hdr.ISP_result == 0;
}

VXDINLINE BOOL IspDeviceRemoved(PDCB dcb, ULONG flags)
{
	ISP_DEV_REMOVED isp = {{ISP_DEVICE_REMOVED, 0}, dcb, flags};
	IlbService(&isp);
	return isp.ISP_d_removed_hdr.ISP_result == 0;
}

VXDINLINE BOOL IspDisassociateDcb(ULONG drive)
{
	ISP_DISASSOC_DCB isp = {{ISP_DISASSOCIATE_DCB, 0}, drive};
	IlbService(&isp);
	return isp.ISP_dis_dcb_hdr.ISP_result == 0;
}

VXDINLINE UCHAR IspDriveLetterPick(PDCB dcb, UCHAR flags)
{
	ISP_pick_drive_letter isp = {{ISP_DRIVE_LETTER_PICK, 0}, dcb, {0, 0}, flags};
	IlbService(&isp);
	if (isp.ISP_p_d_l_hdr.ISP_result == 0)
		return isp.ISP_p_d_l_letter[0];
	return 0;
}

VXDINLINE PVOID IspFindLdmEntry(ULONG drive)
{
	ISP_FIND_LDM isp = {{ISP_DELETE_LDM_ENTRY, 0}, drive};
	if (isp.ISP_fldm_hdr.ISP_result == 0)
		return isp.ISP_fldm_pldm;
	return NULL;
}

VXDINLINE PDCB IspGetDcb(ULONG drive)
{
	ISP_dcb_get isp = {{ISP_GET_DCB, 0}, 0, drive};
	IlbService(&isp);
	if (isp.ISP_g_d_hdr.ISP_result == 0)
		return (PDCB) isp.ISP_g_d_dcb;
	return NULL;
}

VXDINLINE PDCB IspGetFirstNextDcb(PDCB dcb, BYTE type)
{
	ISP_GET_FRST_NXT_DCB isp = {{ISP_GET_FIRST_NEXT_DCB, 0}, (ULONG) dcb, 0, type};
	IlbService(&isp);
	if (isp.ISP_gfnd_hdr.ISP_result == 0)
		return (PDCB) isp.ISP_gfnd_found_dcb;
	return NULL;
}

VXDINLINE BOOL IspInsertCalldown(PDCB dcb, VOID (*calldown)(PIOP), PDDB ddb, USHORT expand, DWORD demand, UCHAR loadgroup)
{
	ISP_calldown_insert isp = {{ISP_INSERT_CALLDOWN, 0}, (ULONG) dcb, (PVOID) calldown, (ULONG) ddb, expand, demand, loadgroup};
	IlbService(&isp);
	return isp.ISP_i_cd_hdr.ISP_result == 0;
}

VXDINLINE ULONG IspQueryMatchingDcbs(PDCB dcb)
{
	ISP_QUERY_MATCH isp = {{ISP_QUERY_MATCHING_DCBS, 0}, dcb};
	IlbService(&isp);
	if (isp.ISP_q_match_hdr.ISP_result == 0)
		return isp.ISP_q_match_drives;
	return 0;
}

VXDINLINE BOOL IspRegistryRead(PDCB dcb, char* keyname, PDWORD ptype, PDWORD psize, PVOID buffer)
{
	ISP_read_registry isp = {{ISP_REGISTRY_READ, 0}, dcb, keyname, ptype, psize, buffer};
	IlbService(&isp);
	return isp.ISP_r_r_hdr.ISP_result == 0;
}

VXDINLINE BOOL IspQueryRemoveDcb(PDCB dcb, ULONG flags)
{
	ISP_QUERY_REMOVE isp = {{ISP_QUERY_REMOVE_DCB, 0}, dcb, flags};
	IlbService(&isp);
	return isp.ISP_q_remove_hdr.ISP_result == 0;
}

#pragma warning(default:4047 4035)

#endif				/* __IOSMGR_H */

