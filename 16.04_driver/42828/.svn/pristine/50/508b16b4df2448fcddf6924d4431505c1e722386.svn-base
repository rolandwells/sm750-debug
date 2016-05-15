#include<linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
#include<linux/config.h>
#endif
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/errno.h>
#include<linux/string.h>
#include<linux/mm.h>
#include<linux/slab.h>
#include<linux/delay.h>
#include<linux/fb.h>
#include<linux/ioport.h>
#include<linux/init.h>
#include<linux/pci.h>
#include<linux/vmalloc.h>
#include<linux/pagemap.h>
#include <linux/console.h>
#ifdef CONFIG_MTRR
#include <asm/mtrr.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
/* no below two header files in 2.6.9 */
#include<linux/platform_device.h>
#include<linux/screen_info.h>
#else
/* nothing by far */
#endif

#include "lynx_drv.h"
#include "lynx_hw712.h"
#include "ddk712/ddk712.h"
#include "lynx_accel.h"

int hw_sm712_map(struct lynx_share* share,struct pci_dev* pdev)
{
	int ret;
	ENTER();

	ret = 0;
	if(share->devid == 0x712){
		share->vidreg_start = pci_resource_start(pdev,0)+MB(4);
		share->vidreg_size = MB(4);
		share->vidmem_start = pci_resource_start(pdev,0);
	}else if(share->devid == 0x720){
		share->vidreg_start = pci_resource_start(pdev,0);
		share->vidreg_size = MB(2);
		share->vidmem_start = pci_resource_start(pdev,0) + MB(2);
	}else{
		/* do not support other chip currently */
		return -ENODEV;
	}

	/* now map mmio*/
	share->pvReg = ioremap(share->vidreg_start,share->vidreg_size);
	if(!share->pvReg){
		err_msg("mmio failed\n");
		ret = -EFAULT;
		goto exit;
	}

	if(share->devid == 0x720){
		share->accel.dprBase = share->pvReg + DE_BASE_ADDR_TYPE3;
		share->accel.dpPortBase = share->pvReg + DE_PORT_ADDR_TYPE3;
	}else{
		share->accel.dprBase = share->pvReg + DE_BASE_ADDR_TYPE2;
		share->accel.dpPortBase = share->pvReg + DE_PORT_ADDR_TYPE2;
	}

	ddk712_set_mmio(share->devid,share->pvReg);
	share->vidmem_size = hw_sm712_getVMSize(share);
	/* map frame buffer*/
	share->pvMem = ioremap(share->vidmem_start,	share->vidmem_size);
	if(!share->pvMem){
		err_msg("Map video memory failed\n");
		ret = -EFAULT;
		goto exit;
	}

	inf_msg("mapped video memory = %p\n",share->pvMem);

	memset(share->pvMem,0,share->vidmem_size);
exit:
	LEAVE(ret);
}


int hw_sm712_inithw(struct lynx_share* share,struct pci_dev * pdev)
{
	init_parm_712 parm;
	struct sm712_share * spec_share;
	ENTER();
	spec_share = container_of(share,struct sm712_share,share);
	parm.devid = share->devid;
	parm.memClock = spec_share->mclk;
	/* a cruel limitation of sm712,burst read make 712 die in x86_64 bit system */
#if defined(__x86_64__)
	parm.pci_burst = 2;/* no burst read,only burst write*/
#else
	/* enable both read and write */
	parm.pci_burst = 3;
#endif
    parm.lcd = spec_share->lcd;
	parm.lcd_color.tftColor = spec_share->lcd_color.tftColor;
	parm.lcd_color.dstnColor = spec_share->lcd_color.dstnColor;

#if defined(__i386__) || defined( __x86_64__)
	/* enable linear mode and packed pixel format first*/
	outb_p(0x18, 0x3c4);
	outb_p(0x11, 0x3c5);
#else
	/*
	 * don't know how to write IO port in other arch/platform
	 * just use default method as a try
	 * */
	outb_p(0x18, 0x3c4);
	outb_p(0x11, 0x3c5);
#endif

	ddk712_hw_init(&parm);
	if(!share->accel_off){
		hw_sm712_initAccel(&share->accel, parm.devid);
	}
	LEAVE(0);
}

resource_size_t hw_sm712_getVMSize(struct lynx_share * share)
{
	resource_size_t ret;
	ret = 0;
	ENTER();
	if(share->devid == 0x712){
		/* sm712 only support 4mb internal video memory */
		ret = MB(4);
	}else if(share->devid == 0x720){
		/* SM722 only support 8mb internal video memory */
		ret = MB(8);
	}
	LEAVE(ret);
}

int hw_sm712_output_checkMode(struct lynxfb_output* output,
								struct fb_var_screeninfo* var)
{
	ENTER();

	LEAVE(0);
}

int hw_sm712_output_setMode(struct lynxfb_output* output,
							struct fb_var_screeninfo* var,
							struct fb_fix_screeninfo* fix)
{
	struct lynxfb_par * par;
	struct lynx_share * share;
	struct sm712_share * spec_share;
	ENTER();

	par = container_of(output,struct lynxfb_par,output);
	share = par->share;
	spec_share = container_of(share,struct sm712_share,share);

	if(output->paths == 1){
		/* this is 1st CRT head,and it only connected to CRT channel
		 * nothing need to set,leave them be what they are
		 * */
		inf_msg("1st CRT path <== CRT channel\n");
	}else if(output->paths == 2){
		/* this is Panel head,and it can connected to either CRT or Panel channel*/
		if(*output->channel == 0){
			/* Virtual Refresh Clk should be disabled */
			inf_msg("2nd CRT/Panel Path <== CRT channel\n");
			poke_fpr(0x31,peek_fpr(0x31)&0x7f);
		}else{
			/* Virtual Clock should be enabled*/
			poke_fpr(0x31,peek_fpr(0x31)|0x80);
			inf_msg("2nd CRT/Panel Path <== Panel channel\n");
		}
	}else if(output->paths == 3){
		/* these are Panel and Crt heads,
		 * can only connected to CRT channel simutanounsly
		 * need disable Virtual Clock
		 * */
		poke_fpr(0x31,peek_fpr(0x31)&0x7f);
		inf_msg("1st CRT Path/2nd CRT/Panel Path <== CRT channel\n");

	}else{
		err_msg("output->path == %d,not valid.\n",output->paths);
		LEAVE(-EINVAL);
	}

	LEAVE(0);
}

int hw_sm712_crtc_checkMode(struct lynxfb_crtc* crtc,
							struct fb_var_screeninfo* var)
{
	int ret;
	ENTER();
	switch(var->bits_per_pixel){
		/* for crt channel,all pixel format are supported */
		case 8:
		case 16:
			ret = 0;
			break;
		case 24:
		case 32:
		/* only 8/16 bpp format are supported by virtual clock */
			if(crtc->channel != 0)
				ret = - EINVAL;
			else
				ret = 0;
			break;
		default:
			ret = -EINVAL;
			break;

	}
	LEAVE(ret);
}

int hw_sm712_crtc_setMode(struct lynxfb_crtc* crtc,
						struct fb_var_screeninfo* var,
						struct fb_fix_screeninfo* fix)
{
	char tmp;
	int fmt;
	unsigned int base,pitch,width;
	struct lynxfb_par *par;
	struct lynx_share * share;
	ENTER();

	par = container_of(crtc,struct lynxfb_par,crtc);
	share = par->share;
	/* set timing,use mode table to set mode
	 * as @var couldn't tell what v-refresh rate it is
	 * so we assume all modes use 60 hz
	 * we may need to create a new setmode function for sm712
	 * like sm750 style,which can use @var parameter directly
	 * */
	ddk712_setModeTiming(crtc->channel,var->xres,var->yres,60);

	if(crtc->channel == 0){
		/* sm712 CRT channel */
		switch(var->bits_per_pixel){
			case 8:
				tmp = 0;
				break;
			case 16:
				tmp = 2;
				break;
			case 24:
				tmp = 4;
				break;
			case 32:
				tmp = 3;
				break;
			default:
				LEAVE(-EINVAL);
		}

		poke32_vpr(0,
				FIELD_VALUE(peek32_vpr(0x0),
					MISC_GRAPH_VIDEO_CTRL,
					DATA_FORMAT,
					tmp));
	}else{
		/* sm712 panel channel */
		switch(var->bits_per_pixel){
			case 8:
				tmp = 0x40;
			case 16:
				tmp = 0x0;
			break;
			default:
			LEAVE(-EINVAL);
		}

		poke_fpr(0x31,(peek_fpr(0x31)&~0x40)|tmp);
	}

	/* set pitch,width,address stuffs*/
	base = crtc->oScreen;
	pitch = fix->line_length;
	width = var->xres * var->bits_per_pixel / 8;

	base >>= 3;
	pitch >>= 3;
	width >>= 3;

	if(crtc->channel == 0){
		poke32_vpr(0x0c,base);
		poke32_vpr(0x10,(width<<16)|(pitch & 0xffff));
	}else{
		/* fpr40~43: read fifo base address*/
		poke_fpr(0x40,base & 0xff);
		poke_fpr(0x41,(base & 0xff00)>>8);
		poke_fpr(0x42,base & 0xff);
		poke_fpr(0x43,(base & 0xff00)>>8);

		/* fpr44,45:read fifo1 offset */
		poke_fpr(0x44,pitch & 0xff);

		if(share->devid == PCI_DEVID_LYNX_3DM){
			/* not implemented */
		}else{
			tmp = (pitch & 0x300) >> 2;
			tmp |= ((base & 0x70000)>>16)|((base & 0x70000)>>13);
			/* fpr45: fifo1 offset and fifo1 base address*/
			poke_fpr(0x45,tmp);
		}
		/* found nothing changed of lcd video after fpr46~49 modified */

#if 0
		/* lcd write start address low of lcd frame buffer*/
		poke_fpr(0x46,base & 0xff);
		/* lcd write start address high of lcd frame buffer*/
		poke_fpr(0x47,(base>>8)&0xff);

		/* write offset of lcd frame buffer*/
		poke_fpr(0x48,width & 0xff);
		poke_fpr(0x49,(peek_fpr(0x49)&0xfc)|((width&0x300)>>8));
#endif
	}
#if 1
	if(!share->accel_off){
		/* set 2d engine pixel format according to mode bpp */
		switch(var->bits_per_pixel){
			case 8:
				fmt = 0;
				break;
			case 16:
				fmt = 1;
				break;
			case 32:
				fmt = 2;
				break;
			case 24:
				fmt = 3;
				break;
			default:
				fmt = 1;
				break;
		}
		hw_set2dformat(&share->accel,fmt);
	}
#endif
	LEAVE(0);
}

int hw_sm712_setColReg(struct lynxfb_crtc* crtc,ushort index,
							ushort red,ushort green,ushort blue)
{
	int ret;
	ENTER();
	ret = 0;
	if(crtc->channel == 0){
		/* write CRT channel RAM only
		 * use 8bits RAM
		 * */
		poke_ccr(0x66,0xa8);
	}else{
		/* write LCD channel RAM only
		 * use 8bits RAM
		 * */
		poke_ccr(0x66,0x58);
	}

	/* DAC mask set to x0ff*/
	poke8_io(0x3c6,0xff);

	/* write color code */
	poke8_io(0x3c8,index);

	/*write color value order by r,g,b*/
	poke8_io(0x3c9,red);
	poke8_io(0x3c9,green);
	poke8_io(0x3c9,blue);
	LEAVE(ret);
}

int hw_sm712_setBLANK(struct lynxfb_output* output,int blank)
{
	ENTER();

	switch (blank)
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
		case FB_BLANK_UNBLANK:
#else
		case VESA_NO_BLANKING:
#endif
			if(output->paths & 1){
				/* HPVP */
				poke_fpr(0x22,peek_fpr(0x22) & 0xcf);
				/* experiment found fpr31 bit 2 did not affect
				 * CRT display
				 * */
				poke_fpr(0x31,peek_fpr(0x31)|2);
			}

			if(output->paths & 2){
				/* unblank for LCD head */
				poke_fpr(0x31,peek_fpr(0x31)|1);
			}
			break;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
		case FB_BLANK_NORMAL:
			if(output->paths & 1){
				poke_fpr(0x22,peek_fpr(0x22) & 0xcf);
				poke_fpr(0x31,peek_fpr(0x31) & ~2);
			}

			if(output->paths & 2){
				/* blank for LCD head */
				poke_fpr(0x31,peek_fpr(0x31)&~1);
			}
			break;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
		case FB_BLANK_VSYNC_SUSPEND:
#else
		case VESA_VSYNC_SUSPEND:
#endif
			if(output->paths & 1){
				poke_fpr(0x22,(peek_fpr(0x22)&0xcf)|0x20);
				poke_fpr(0x31,peek_fpr(0x31) & ~2);
			}
			if(output->paths & 2){
				poke_fpr(0x31,peek_fpr(0x31)&~1);
			}
			break;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
		case FB_BLANK_HSYNC_SUSPEND:
#else
		case VESA_HSYNC_SUSPEND:
#endif
			if(output->paths & 1){
				poke_fpr(0x22,(peek_fpr(0x22)&0xcf)|0x10);
				poke_fpr(0x31,peek_fpr(0x31) & ~2);
			}
			if(output->paths & 2){
				poke_fpr(0x31,peek_fpr(0x31)&~1);
			}
			break;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
		case FB_BLANK_POWERDOWN:
#else
		case VESA_POWERDOWN:
#endif
			if(output->paths & 1){
				poke_fpr(0x22,(peek_fpr(0x22)&0xcf)|0x30);
				poke_fpr(0x31,peek_fpr(0x31) & ~2);
			}
			if(output->paths & 2){
				poke_fpr(0x31,peek_fpr(0x31)&~1);
			}
			break;
	}
	LEAVE(0);
}

int hw_sm712_deWait()
{
	unsigned int loop = 0x100000;
	/* wait queue ,assume PCIRetry is not enabled */
	while(loop--){
		/* see if fifo empty and engine idle */
		if((peek_scr(0x16) & 0x08) == 0 &&
			(peek_scr(0x16) & 0x10) != 0)
			break;
	}
	return (!loop);
}

void hw_sm712_initAccel(struct lynx_accel * accel, int devid)
{
	u8 reg;
	ENTER();
	/* enable drawing engine */
	poke_scr(0x21,peek_scr(0x21)&0xf8);

	if(devid == 0x712)
	{
	/*by ilena, for bug: system hang
	 after repeatly unload and load driver on sm722*/
	/* reset drawing engine */
		reg = peek_scr(0x15);
		poke_scr(0x15,reg|0x30);
		poke_scr(0x15,reg);
	}

	/* init de register */
	accel->de_init(accel);
	LEAVE();
}

void hw_sm712_crtc_clear(struct lynxfb_crtc* crtc)
{
	ENTER();
	LEAVE();
}

void hw_sm712_output_clear(struct lynxfb_output* output)
{

	ENTER();
	LEAVE();
}
