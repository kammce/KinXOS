// FSDEXT2.h - include file for VxD FSDEXT2

#include <vtoolsc.h>

#ifdef OLD_BEHAVIOR





#define FSDEXT2_Major		1
#define FSDEXT2_Minor		0
#define FSDEXT2_DeviceID		UNDEFINED_DEVICE_ID
//#define FSDEXT2_Init_Order	UNDEFINED_INIT_ORDER
#define FSDEXT2_Init_Order	0xb0000000

extern ILB theILB;
#include "shared\iosmgr.h"





#else




#define FSDEXT2_Major		1
#define FSDEXT2_Minor		0
#define FSDEXT2_DeviceID		UNDEFINED_DEVICE_ID
#define FSDEXT2_Init_Order	UNDEFINED_INIT_ORDER

#define FSDEXT2_LGN_MASK DRP_VSD_1	// define IOS group number (layer) for this VSD
#define FSDEXT2_LGN_ORD  DRP_VSD_1_BIT	// define IOS group number (layer) for this VSD

extern ILB FSDEXT2_Ilb;
#define theILB FSDEXT2_Ilb
#include "shared\iosmgr.h"


#endif
