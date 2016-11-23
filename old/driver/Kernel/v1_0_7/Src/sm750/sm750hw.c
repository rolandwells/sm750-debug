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
#include<linux/screen_info.h>

#include<asm/io.h>
#include<asm/uaccess.h>

#include"sm750drv.h"
#include"sm750hw.h"

#include"ddk/ddk750_regs.h"


#include"ddk/ddk750_type.h"
#include"ddk/ddk750_func.h"
#include"ddk/ddk750_intern.h"

unsigned char __iomem * pVMEM ;
unsigned char __iomem * pMMIO ;
/* hardware cursor with and height*/
int hwc_w,hwc_h;

typedef unsigned long ulseg[2];
ulseg segs[] = {
#ifdef VALIDATION_CHIP		
	{SYSTEM_CTRL,SCRATCH_DATA},
#else
	{SYSTEM_CTRL,VGA_CONFIGURATION},
#endif
	{PANEL_DISPLAY_CTRL,PANEL_VERTICAL_SYNC},
	{CRT_DISPLAY_CTRL,CRT_SCALE},
	{PANEL_HWC_ADDRESS,PANEL_HWC_COLOR_3},
	{CRT_HWC_ADDRESS,CRT_HWC_COLOR_3},
	{ZV0_CAPTURE_CTRL,ZV1_CAPTURE_YRGB_CONST},		
};


#define  peek32(ADD)			readl(ADD + pMMIO)
#define  poke32(ADD,DATA)		writel(DATA,ADD + pMMIO)
#define  pokeMem(ADD,DATA)		writel(DATA,ADD + pVMEM);


unsigned char smread8(unsigned long offset)
{
	return readb(offset + pMMIO);
}
unsigned int smread32(unsigned long offset)
{	
	return peek32(offset);
}

void smwrite32(unsigned long offset,unsigned int value)
{
	poke32(offset,value);
}

void smwrite8(unsigned long offset,unsigned char value)
{
	writeb(value,offset + pMMIO);
}

static unsigned long sm750fb_ps_to_hz(unsigned long psvalue)
{
	unsigned long long numerator=1000*1000*1000*1000ULL;
	/* 10^12 / picosecond period gives frequency in Hz */
	do_div(numerator, psvalue);
	return (unsigned long)numerator;
}

void hw_release()
{

}

/*
 * Return memory size of sm750 in bytes
 */
unsigned long hw_get_memsize()
{	
	ulong ulreg;	
	SMENTER;

	ulreg = peek32(MODE0_GATE);
	ulreg = FIELD_SET(ulreg,MODE0_GATE,GPIO,ON);
	poke32(MODE0_GATE,ulreg);
	
	SMEXIT;
	return ddk750_getFrameBufSize();
}

int hw_init_chip(struct sm750fb_state * state)
{
	int ret;
	initchip_param_t parm;
	SMENTER;	
	
	parm.powerMode = 0;
	parm.chipClock = state->chip_clk;
	parm.memClock = state->mem_clk;
	parm.masterClock = state->master_clk;
	parm.setAllEngOff = state->de_off;
	parm.resetMemory = 1;
	
	ret = ddk750_initChipParm(&parm);	
#ifdef USE_DVICHIP	
	/* do not let ret be below's return value, driver should not stop even dvi init failed */
	if(ddk750_initDisplay())
		err_msg("Init Display failed\n");
#endif	
	
	SMEXIT;
	return ret;
}

int hw_alloc_mem(struct sm750fb_par *par,int size,enum sm750_allocmem_reason reason)
{
	struct sm750fb_mem * pmem;
	ulong offset;
	ulong memsize;
	
	memsize = par->share->mem_size;
	switch (reason)
	{
		case REASON_PRI_SURFACE:
			offset = 0;
			pmem = &par->screen;
			break;
		case REASON_SEC_SURFACE:
			offset = memsize / 2;
			pmem = &par->screen;
			break;
		case REASON_PRI_CURSOR:
			offset = memsize / 2 - MAX_CURS_SIZE;
			pmem = &par->cursor;
			break;
		case REASON_SEC_CURSOR:
			offset = memsize - MAX_CURS_SIZE;
			pmem = &par->cursor;
			break;
		default:
			return -EINVAL;
	}
	pmem->offset = offset;
	pmem->size = size;
	pmem->pvAdd = (char *)par->share->pvMem + offset;
	return 0;
}

/*
	1,Translate var information into struct known by ddk
	2,Call ddk set mode routines
*/
int hw_set_mode(const struct sm750fb_par * par,const struct fb_var_screeninfo *var,const struct fb_fix_screeninfo * fix)
{
	int ret;
	ulong pixelclock_hz;
	logicalMode_t logic;
	mode_parameter_t modparm;
	disp_output_t output;
	struct sm750fb_state* state;

	SMENTER;		
	
	logic.x = var->xres;
	logic.y = var->yres;
	
	/* panel expansion size */

	logic.xLCD = par->share->state.xLCD > var->xres ? par->share->state.xLCD : 0;
	logic.yLCD = par->share->state.yLCD > var->yres ? par->share->state.yLCD : 0;

	/*
		1,logic.hz is only used by expansion mode which will only be 60hz
		2,logic.hz can not be got from var ... if you want get it , 
		   write an simple text-anylise routine yourself.
	*/
	logic.hz = 60;
	logic.baseAddress = par->screen.offset;
	logic.pitch = fix->line_length;
	logic.bpp = var->bits_per_pixel;
	logic.dispCtrl = (par->controller == CTRL_PRI)?PANEL_CTRL:CRT_CTRL;
	
	/* 	set Mode 	*/	
	//ddk750_setLogicalDispOutput(NO_DISPLAY,0);
	
	if(logic.xLCD != 0 && logic.yLCD != 0 )
	{
		ret = ddk750_setModeEx(&logic);
	}
	else
	{
		/* 	caculate  pixel clock in HZ 	*/			
		pixelclock_hz = sm750fb_ps_to_hz(var->pixclock);
		/*	set mode parameter	*/
		modparm.pixel_clock = pixelclock_hz;	
		modparm.vertical_sync_polarity = (var->sync & FB_SYNC_HOR_HIGH_ACT) ? POS:NEG;
		modparm.horizontal_sync_polarity = (var->sync & FB_SYNC_VERT_HIGH_ACT) ? POS:NEG;
		modparm.clock_phase_polarity = (var->sync& FB_SYNC_COMP_HIGH_ACT) ? POS:NEG;			
		modparm.horizontal_display_end = var->xres;
		modparm.horizontal_sync_width = var->hsync_len;
		modparm.horizontal_sync_start = var->xres + var->right_margin;
		modparm.horizontal_total = var->xres + var->left_margin + var->right_margin + var->hsync_len;
		modparm.vertical_display_end = var->yres;
		modparm.vertical_sync_height = var->vsync_len;
		modparm.vertical_sync_start = var->yres + var->lower_margin;
		modparm.vertical_total = var->yres + var->upper_margin + var->lower_margin + var->vsync_len;	
		ret = ddk750_setCustomMode(&logic,&modparm);
	}

	if(ret){
		err_msg("error in set mode !\n");
		goto ERR_SETMODE;		
	}

	/*	set logic display output	*/
	state = &par->share->state;	

	switch (state->output)
	{
		case OP_SIMUL_TFT_CRT:
			if(logic.xLCD !=0 && logic.yLCD !=0)
				output = PANEL_CRT_SIMUL_SEC;
			else
				output = PANEL_CRT_SIMUL;
			break;
		case OP_SIMUL_TFT1_TFT2:
			output = PANEL1_PANEL2_SIMUL;
			break;
		case OP_SIMUL_CRT1_CRT2:
			output = CRT1_CRT2_SIMUL;
			break;
		default:
			output = CRT1_CRT2_SIMUL;
			break;
	}	
	
	ret = ddk750_setLogicalDispOutput(output,0);
	if(ret){
		err_msg("error in set logic output !\n");
		goto ERR_SETOUTPUT;
	}

	/*	set 2d engine
			
		2d_init routine must be called after setmode
		because bpp will be altered by setmode
		so deInit will set 2d engine bpp field through checking 0x80000 [1:0]

		when dual mode support in future , each time 2d engine work should
		use bpp passed in but not by check 0x80000 [1:0]
		Modify the code in that time.
	*/
	if(!state->de_off)
	{		
		ddk750_deInit(logic.dispCtrl);
	}
	
	SMEXIT;
	return ret;
ERR_SETOUTPUT:	
ERR_SETMODE:	
	return -1;
}


int hw_set_blank(const struct sm750fb_par *par,int flag)
{
	uint dpms,pps,crtdb;
	int retv;
	
	SMENTER;		
	retv = 0;
	switch (flag)
	{		
		case FB_BLANK_UNBLANK:	
			inf_msg("flag = FB_BLANK_UNBLANK \n");
			dpms = SYSTEM_CTRL_DPMS_VPHP;
			pps = PANEL_DISPLAY_CTRL_DATA_ENABLE;
			crtdb = CRT_DISPLAY_CTRL_BLANK_OFF;			
			break;
		case FB_BLANK_NORMAL:
			inf_msg("flag = FB_BLANK_NORMAL \n");
			dpms = SYSTEM_CTRL_DPMS_VPHP;
			pps = PANEL_DISPLAY_CTRL_DATA_DISABLE;
			crtdb = CRT_DISPLAY_CTRL_BLANK_ON;			
			break;
		case FB_BLANK_VSYNC_SUSPEND:
			dpms = SYSTEM_CTRL_DPMS_VNHP;
			pps = PANEL_DISPLAY_CTRL_DATA_DISABLE;
			crtdb = CRT_DISPLAY_CTRL_BLANK_ON;			
			break;
		case FB_BLANK_HSYNC_SUSPEND:
			dpms = SYSTEM_CTRL_DPMS_VPHN;
			pps = PANEL_DISPLAY_CTRL_DATA_DISABLE;
			crtdb = CRT_DISPLAY_CTRL_BLANK_ON;
			break;
		case FB_BLANK_POWERDOWN:
			dpms = SYSTEM_CTRL_DPMS_VNHN;
			pps = PANEL_DISPLAY_CTRL_DATA_DISABLE;
			crtdb = CRT_DISPLAY_CTRL_BLANK_ON;				
			break;		
	}
	poke32(SYSTEM_CTRL,FIELD_VALUE(peek32(SYSTEM_CTRL),SYSTEM_CTRL,DPMS,dpms));
	poke32(PANEL_DISPLAY_CTRL,FIELD_VALUE(peek32(PANEL_DISPLAY_CTRL),PANEL_DISPLAY_CTRL,DATA,pps));
	poke32(CRT_DISPLAY_CTRL,FIELD_VALUE(peek32(CRT_DISPLAY_CTRL),CRT_DISPLAY_CTRL,BLANK,crtdb));
	SMEXIT;
	return retv;
}

int hw_2d_wait_idle(const struct sm750fb_par * par)
{
	return ddk750_2d_waitIdle();
}

int hw_set_colreg(const struct sm750fb_par *par,int index,ushort r,ushort g,ushort b)
{
	static ulong add[]={PANEL_PALETTE_RAM,CRT_PALETTE_RAM};
	poke32(add[par->controller] + index*4 ,(r<<16)|(g<<8)|b);
	return 0;
}

int hw_dump_reg(const struct sm750fb_par * par,ushort flag)
{
#define BUFSIZE	1024

	int err,index;
	char * buf;
	
	SMENTER;
	err = 0;
	buf = kzalloc(BUFSIZE,GFP_KERNEL);

	if(!buf){
		return -ENOMEM;
	}

	for(index=0;index < sizeof(segs)/sizeof(segs[0]);index++)
	{
		if( (1 << index) & flag)
		{			
			err = ddk750_dumpRegisters(segs[index][0],segs[index][1],sprintf,buf);
			if(err)	
				goto err;
			printk("%s\n",buf);
			memset(buf,0,BUFSIZE);
		}		
	}	
	goto exit;
err:
	err_msg("Error [%d] occured when dump registers. \n",err);
exit:
	kfree(buf);
	SMEXIT;
	return err;
}

int hw_pan_display(const struct sm750fb_par *par,const struct fb_var_screeninfo*var)
{	
	/* panning can be implemented by two methods:
		#1,adjust panel base address
		#2,use Panel Panning Control register or Panel Window Width
		     for PANEL (though I've not used it yetl)
		we use #1 for safe
	*/	
	struct fb_info* info;
	static ulong address[2]={PANEL_FB_ADDRESS,CRT_FB_ADDRESS};	
	if((var->xoffset + var->xres > var->xres_virtual)||
		(var->yoffset + var->yres > var->yres_virtual))
	{
		return -EINVAL;
	}
	info = par->info;	
#if 0	
	if(par->controller == CTRL_PRI)
	{
		data = FIELD_SET(0, PANEL_PAN_CTRL,VERTICAL_PAN, var->yoffset)|
			FIELD_SET(0,PANEL_PAN_CTRL,VERTICAL_VSYNC,4)|
			FIELD_SET(0,PANEL_PAN_CTRL,HORIZONTAL_PAN,var->xoffset)|
			FIELD_SET(0,PANEL_PAN_CTRL,HORIZONTAL_VSYNC,4);
		poke32(PANEL_PAN_CTRL,data);
	}
	else
#endif		
	{
		/* use #1*/
		int total = var->yoffset * info->fix.line_length + ((var->xoffset * var->bits_per_pixel) >> 3);
		total += par->screen.offset;
		poke32(address[par->controller],
			FIELD_VALUE(peek32(address[par->controller]),PANEL_FB_ADDRESS,ADDRESS,total));		
		//inf_msg("CRT_FB_ADDRESS = %08x \n",peek32(CRT_FB_ADDRESS));
	}
	return 0;
}
void hw_cursor_enable(const struct sm750fb_par * par,int on)
{
	ulong value;
	static ulong add[2]={PANEL_HWC_ADDRESS,CRT_HWC_ADDRESS};	
	value = 0;	
	/* 
		seems each time write bit 31 of hwc_address will set bit 0~25
		to an unknown value
	*/
	if(on!=0)	{
		value = FIELD_SET(0,PANEL_HWC_ADDRESS,ENABLE,ENABLE)
				|FIELD_SET(0,PANEL_HWC_ADDRESS,EXT,LOCAL)
				|FIELD_VALUE(0,PANEL_HWC_ADDRESS,ADDRESS,par->cursor.offset);
	}else{
		value = FIELD_SET(0,PANEL_HWC_ADDRESS,ENABLE,DISABLE);
	}
	
	poke32(add[par->controller],value);
}

void hw_cursor_clear(const struct sm750fb_par * par)
{
	if(par->cursor.pvAdd!=NULL)
		memset(par->cursor.pvAdd,0,MAX_CURS_W*MAX_CURS_H*2/8);
}

void hw_cursor_setsize(const int w,const int h)
{
	hwc_w = w;
	hwc_h = h;
}

int hw_cursor_setpos(const struct sm750fb_par * par,ulong dx,ulong dy)
{
	static ulong add[2]={PANEL_HWC_LOCATION,CRT_HWC_LOCATION};
	ulong data;
	if(dx>2047 || dy>2047)
		return -1;	
	
	data = FIELD_VALUE(0,PANEL_HWC_LOCATION,Y,dy)|
		FIELD_VALUE(0,PANEL_HWC_LOCATION,X,dx);
	poke32(add[par->controller],data);
	return 0;
}

int hw_cursor_setcolor(const struct sm750fb_par * par,u32 bg,u32 fg)
{
	/*
		fb only use mono color for cursor
		we put bg into color_1 and fg into color_2
	*/
	static ulong add[] = {PANEL_HWC_COLOR_12,CRT_HWC_COLOR_12};
	poke32(add[par->controller],(fg<<16) |(bg & 0xffff));
	return 0;
}
#if 0
int hw_cursor_setdata(const struct sm750fb_par * par,
						const struct fb_cursor * cur,
						const u8 *pcol,const u8 *pmsk)
{
	int i,j,count,pitch,offset;
	u8 color,mask;
	ushort data;
	ushort * pbuffer,*pstart;
	
	
	/*  in byte*/
	pitch = hwc_w >> 3;
	/* in byte	*/
	count = (hwc_w * hwc_h) >> 3;
	/* in ushort */
	offset = MAX_CURS_W * 2 / 8 / 2;
	
	data = 0;	
	pstart = (ushort *)par->cursor.pvAdd - offset;
	
	for(i=0;i< count;i+=1)
	{
			color = *pcol++;
			mask = *pmsk++;
			for(j=0;j<8;j++)
			{
				if(mask & (1<<j)){
					/*	 1==>1 and 0==>2 	*/
					data |=  ((color & (1<<j))?1:2) << (j*2);
				}
			}

			if( count % pitch ==0 ){
				/* need a return*/
				pstart +=offset;
				pbuffer = pstart;
			}
			*pbuffer = data;
			pbuffer++;			
	}
	return 0;	
}
#else
int hw_cursor_setdata(const struct sm750fb_par * par,
						const struct fb_cursor * cur,
						const u8 *pcol,const u8 *pmsk)
{
	int i,j,count,pitch,offset;
	u8 color,mask,opr;
	ushort data;
	ushort * pbuffer,*pstart;
	
	
	/*  in byte*/
	pitch = hwc_w >> 3;
	/* in byte	*/
	count = (hwc_w * hwc_h) >> 3;
	/* in ushort */
	offset = MAX_CURS_W * 2 / 8 / 2;
	
	data = 0;	
	pstart = (ushort *)par->cursor.pvAdd - offset;
	
	for(i=0;i< count;i+=1)
	{
			color = *pcol++;
			mask = *pmsk++;
			data = 0;
			for(j=0;j<8;j++)
			{
				if(cur->rop == ROP_XOR)
					opr = mask ^ color;
				else
					opr = mask & color;//if not XOR ,treat it as COPY
				
				if(opr & (0x80 >> j))
				{	//use fg color,id = 2
					data|= 2 << (j*2);
				}else{
					//use bg color ,id = 1
					data |= 1 << (j*2);
				}
			}

			if( count % pitch ==0 ){
				/* need a return*/
				pstart +=offset;
				pbuffer = pstart;
			}
			*pbuffer = data;
			pbuffer++;			
	}
	return 0;	
}

#endif
void hw_2d_fillrect(const struct sm750fb_par * par,
			ulong base,ulong pitch,ulong bpp,
			ulong dx,ulong dy,ulong width,ulong height,
			ulong color,ulong rop2)
{	
	ddk750_deRectFill(base,pitch,bpp,dx,dy,width,height,color,rop2);
}

void hw_2d_copyarea(const struct sm750fb_par * par,
			ulong base,ulong pitch,ulong bpp,
			ulong dx,ulong dy,ulong sx,ulong sy,
			ulong width,ulong height)
{
	ddk750_deVideoMem2VideoMemBlt(base,pitch,sx,sy,
									base,pitch,bpp,dx,dy,
									width,height,HW_ROP2_COPY);
}

void hw_2d_imageblit_mono(const struct sm750fb_par * par,
			ulong base,ulong pitch,ulong bpp,
			ulong dx,ulong dy,
			ulong width,ulong height,
			const unsigned char * data,
			ulong fgcol,ulong bgcol)
{
		ddk750_deSystemMem2VideoMemMonoBlt(data,width>>3,0,
											base,pitch,bpp,dx,dy,
											width,height,fgcol,bgcol,HW_ROP2_COPY);
}

#ifdef CONFIG_PM
void hw_suspend(const struct sm750fb_par * par)
{
	setPowerMode(POWER_MODE_CTRL_MODE_SLEEP);
	//setPowerMode(POWER_MODE_CTRL_MODE_MODE1);
	
}
#endif



