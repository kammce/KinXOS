// FSDEXT2.h - include file for VxD FSDEXT2

#include <vtoolsc.h>

#define TSDEXT2_Major		1
#define TSDEXT2_Minor		0
#define TSDEXT2_DeviceID		UNDEFINED_DEVICE_ID
#define TSDEXT2_Init_Order	UNDEFINED_INIT_ORDER


#define TSDEXT2_LGN_MASK DRP_TSD
#define TSDEXT2_LGN_ORD  DRP_TSD_BIT

extern ILB TSDEXT2_Ilb;
#define theILB TSDEXT2_Ilb

#include "shared\iosmgr.h"
