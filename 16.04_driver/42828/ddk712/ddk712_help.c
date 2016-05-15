#include "ddk712_help.h"

volatile unsigned char __iomem * mmio712 = NULL;
/* below offset used for sm712/sm722 */
int io_offset712;
/* DPR and VPR memory mapped registers,treated as 32bit access */
int dpr_offset712;
int vpr_offset712;
int dataPort_offset712;

/* after driver mapped io registers, use this function first */
void ddk712_set_mmio(int devid,volatile unsigned char __iomem * addr)
{
	mmio712 = addr;
	if(devid == 0x712){
		/* sm712 register offset stuffs */
		io_offset712 = MB(3);
		dpr_offset712 = KB(32);
		vpr_offset712 = KB(48);
		dataPort_offset712 = MB(1);
	}else if(devid == 0x720){
		/* sm722 register offset stuffs */
		io_offset712 = KB(768);
		dpr_offset712 = KB(0);
		vpr_offset712 = KB(2);
		dataPort_offset712 = MB(1);
	}
}

