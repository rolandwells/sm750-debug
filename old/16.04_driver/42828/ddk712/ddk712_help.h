#ifndef DDK712_HELP_H__
#define DDK712_HELP_H__
#include "ddk712_chip.h"
#ifndef USE_INTERNAL_REGISTER_ACCESS

#include <linux/ioport.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "../lynx_help.h"

/* mmio712 start from framebuffer + 4mega bytes */
extern volatile unsigned  char __iomem * mmio712;
extern int io_offset712;
/* DPR and VPR memory mapped registers,treated as 32bit access */
extern int dpr_offset712;
extern int vpr_offset712;
extern int dataPort_offset712;

#define peek32(addr) readl((addr)+mmio712)
#define poke32(addr,data) writel((data),(addr)+mmio712)

#define peek8(addr) readb((addr)+mmio712)
#define poke8(addr,data) writeb((data),(addr)+mmio712)

#define peek8_io(addr) peek8(addr+io_offset712)
#define poke8_io(addr,data) poke8(addr+io_offset712,data)

static inline char peek_scr(int idx)
{
	poke8_io(0x3c4,idx);
	return peek8_io(0x3c5);
}

static inline void poke_scr(int idx,char val)
{
	poke8_io(0x3c4,idx);
	poke8_io(0x3c5,val);
}

static inline char peek_crt(int idx)
{
	poke8_io(0x3d4,idx);
	return peek8_io(0x3d5);
}

static inline void poke_crt(int idx,char val)
{
	poke8_io(0x3d4,idx);
	poke8_io(0x3d5,val);
}

/* below registers equal to sequence registers */
#define peek_pdr(a) peek_scr(a)
#define peek_fpr(a) peek_scr(a)
#define peek_mcr(a) peek_scr(a)
#define peek_ccr(a) peek_scr(a)
#define peek_gpr(a) peek_scr(a)
#define peek_phr(a) peek_scr(a)
#define peek_pop(a) peek_scr(a)
#define peek_hcr(a) peek_scr(a)

#define poke_pdr(a,b) poke_scr(a,b)
#define poke_fpr(a,b) poke_scr(a,b)
#define poke_mcr(a,b) poke_scr(a,b)
#define poke_ccr(a,b) poke_scr(a,b)
#define poke_gpr(a,b) poke_scr(a,b)
#define poke_phr(a,b) poke_scr(a,b)
#define poke_pop(a,b) poke_scr(a,b)
#define poke_hcr(a,b) poke_scr(a,b)

/* below registers equal to crtc registers*/
#define peek_svr(a) peek_crt(a)
#define poke_svr(a,b) poke_crt(a,b)


static inline unsigned int peek32_dpr(int index)
{
	return peek32(index + dpr_offset712);
}

static inline void poke32_dpr(int index,unsigned int val)
{
	poke32(index + dpr_offset712,val);
}

static inline unsigned int peek32_vpr(int index)
{
	return peek32(index + vpr_offset712);
}

static inline void poke32_vpr(int index,unsigned int val)
{
	poke32(index + vpr_offset712,val);
}
#else
/* implement if you want use it*/
#endif

#endif
