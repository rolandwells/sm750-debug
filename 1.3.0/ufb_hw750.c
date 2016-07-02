#include 	<linux/module.h>
#include 	<linux/kernel.h>
#include 	<linux/errno.h>
#include 	<linux/string.h>
#include 	<linux/mm.h>
#include 	<linux/slab.h>
#include 	<linux/delay.h>
#include 	<linux/fb.h>
#include 	<linux/init.h>
#include 	<linux/vmalloc.h>
#include 	<linux/pagemap.h>
#include 	<linux/screen_info.h>
#include 	<asm/uaccess.h>
#include	<linux/usb.h>
#include	<linux/delay.h>

#include 	"usbfb.h"
#include	"plx.h"
#include 	"ufb_drv.h"
#include 	"ufb_hw750.h"

#include 	"ddk/ddk750_regs.h"
#include 	"ddk/ddk750_type.h"
#include 	"ddk/ddk750_func.h"
#include 	"ddk/ddk750_intern.h"

//static struct csc_regs lastcscEx;
//static struct x_io_csc lastcsc;

/*globle var*/
//static struct ufb_data * g_ufb;

/* hardware cursor width and height*/
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

/*
void hw_set_ufb(struct ufb_data * ufb){
	g_ufb = ufb;
}
*/

void tst_hdmi(void * ufb,int d){
	uint8_t rcc;
	rcc = swI2CReadReg(ufb,0x72,0x1B);
	printk("rcc%d=%02x\n",d,rcc);
}

uint32_t peek32(struct ufb_data * ufb,uint32_t offset){
	if(!ufb){
		error("null pointer on ufb");
		return 0;
	}

	return SMI_READ_MMIO(ufb,offset);
}

void poke32(struct ufb_data * ufb,uint32_t offset,uint32_t value){	
	if(!ufb){
		error("null pointer on ufb");
	}
	
	SMI_WRITE_MMIO(ufb,offset,value);
}

uint32_t peek32_vm(struct ufb_data * ufb,uint32_t offset){
	if(!ufb){
		error("null pointer on ufb");
		return 0;
	}

	return SMI_READ_VMEM(ufb,offset);
}

uint32_t peek32_2d(struct ufb_data * ufb,uint32_t offset){
#if 1
	int idx;
	idx = offset - 0x100000;
	idx >>= 2;
	return ufb->reg2d[idx].lastValue;
#else
	return SMI_READ_MMIO(ufb,offset);
#endif
}

void poke32_2d(struct ufb_data * ufb,uint32_t offset,uint32_t value){
#if 1
	int idx;
	uint32_t val;
	idx = offset - 0x100000;
	idx >>= 2;
	/* just in case ..*/
	val = value & ~ufb->reg2d[idx].ignoreBits;
	if(ufb->reg2d[idx].lastValue != val){
		ufb->reg2d[idx].lastValue = val;
		/* do real write */
		SMI_WRITE_MMIO(ufb,offset,val);
	}
#else
	SMI_WRITE_MMIO(ufb,offset,value);
#endif
}

static const uint32_t g_sm750_reg_mask_2d[REG_CNT_SM750_2D] = 
{
	1<<30,3<<29,7<<29,1<<31,/*0x10000c*/
	(7<<29)|(7<<13),0,0,(0xf<<12)|(1<<22)|(1<<26)|(1<<31),/* 0x10001c*/
	0xff<<24,0xff<<24,0,3<<14,/* 0x10002c */
	7<<13,0,0,(7<<29)|(7<<13),/* 0x10003c */
	0xf<<28,0xf<<28,0xffffff00,0,/* 0x10004c,2d wrap */	
	0,/* 0x100050 2d status,do not read it with cache-method */
};

void hw_fill_2d_cache(struct ufb_data * ufb)
{
	/* need 2d engine already enabled */
	int idx,offset;
	ENTER();
	offset = 0x100000;
	while(offset < 0x100054){
		idx = (offset - 0x100000)>>2;
		ufb->reg2d[idx].ignoreBits = g_sm750_reg_mask_2d[idx];
		ufb->reg2d[idx].lastValue = SMI_READ_MMIO(ufb,offset);
		ufb->reg2d[idx].lastValue &= ~ ufb->reg2d[idx].ignoreBits;
		offset += 4;
	}
	LEAVE();
}

static void sm_fill_fb32(struct ufb_data * ufb,uint32_t offset,void * src,int maxdw,int async){
	uint32_t flag;
	flag = maxdw;
	if(async)
		flag |= WRITE_PCIMM_ASYNC;

	PciMm_WriteUlong_full(ufb,
			offset+SMI_VMEM_BAR0,
			(uint32_t *)src,
			flag);
}

void poke32_dataport(struct ufb_data * ufb,uint32_t value){
	uint32_t flag;
	flag = 1;
	flag |= WRITE_PCIMM_ASYNC;

	PciMm_WriteUlong_full(ufb,
		DE_DATA_PORT+SMI_MMIO_BAR1,
		&value,
		flag);
}

/* 
 * do not let count over 16mb
 */
static int smi_fill_dma(struct ufb_data * ufb,uint32_t offset,void * src,int count)
{
	int err;
	int cnt;
	int sum;
    ENTER();
	err = 0;
	sum = 0;
	if(ufb){
		/*copy data from src to physical continuse area */
		while(sum < count){
			if(BUF_DMA_SIZE > (count - sum)){
				cnt = count - sum;
			}else{
				cnt = BUF_DMA_SIZE;
			}
			memcpy(ufb->buf_dma,src + sum,cnt);
			err = PciMm_WriteDma(ufb,
				SMI_VMEM_BAR0 + offset + sum,
				ufb->buf_dma,
				cnt,0);
			if(err < 0){
				//printk("k");
				return err;
			}
			sum += cnt;
		}
	}
	LEAVE(0);
}

static unsigned long sm750fb_ps_to_hz(unsigned long psvalue)
{
	unsigned long long numerator=1000*1000*1000*1000ULL;
	/* 10^12 / picosecond period gives frequency in Hz */
	do_div(numerator, psvalue);
	return (unsigned long)numerator;
}

static int hw_tst_edid(struct ufb_data * ufb,int scl,int sda)
{
#define PATTERN_TST_CNT 3
	int i;
	uint8_t head[8];
	static const uint8_t pattern[8] = {0,255,255,255,255,255,255,0};
	swI2CInit((void *)ufb,scl,sda);
	i = 0;
	while(i<PATTERN_TST_CNT){
		head[i] = swI2CReadReg(ufb,0xa0,i);
		//printk("head[%d]=%d\n",i,head[i]);
		i++;
	}
	
	i=0;
	while(i<PATTERN_TST_CNT){
		if(head[i]!=pattern[i])
			return 0;
		i++;
	}
	
	return 1;
#undef PATTERN_TST_CNT
}

int hw_output0_connected(struct ufb_data * ufb){
	int ret = 0;
	switch(ufb->output0){
		case output_hdmi:
			ret = sii9022xIsConnected(ufb);
			if(ret)
				inf_msg("hdmi connected\n");
			else
				inf_msg("hdmi disconnected\n");
			break;
		case output_dvid:
			ret = sii164IsConnected(ufb);		
			if(ret)
				inf_msg("dvi connected\n");
			else{
				inf_msg("dvi disconnected,try dvi-i[vga] interface\n");
				ret = hw_tst_edid(ufb,30,31);
				if(ret){
					ufb->output0 = output_analog;
					inf_msg("dvi-i or vga connected\n");
				}else{
					inf_msg("dvi-i or vga disconnected\n");
				}
			}
			break;
		default:
		case output_analog:
			ret = hw_tst_edid(ufb,30,31);
			if(ret)
				inf_msg("dvi-i or vga connected\n");
			else
				inf_msg("dvi-i or vga disconnected\n");
			break;
	}

	return ret;

#if 0
#ifdef USE_HDMICHIP
	/* new layout:use HDMI contact,and check EDID via VGA analog line 
	 * if HDMI connection not present */
	if(sii9022xIsConnected(ufb))
		return 1;

#elif defined (USE_DVICHIP)
	/* formal layout:use DVI-I contact,and check EDID via VGA analog line
	 * if DVI connection not present */
	if(sii164IsConnected(ufb)){
		printk("primary output use dvi\n");
		return 1;
	}
	else{
		printk("primary output use dvi2vga\n");
		/* do not use any kind of digital display controller */
		if(hw_tst_edid(ufb,30,31))
			return 1;		
	}
#else
	return 0;
#endif
#endif
}


int hw_output1_connected(struct ufb_data * ufb){
	return hw_tst_edid(ufb,17,18);
}

void hw_release()
{

}

/*
 * Return memory size of sm750 in bytes
 */
int hw_get_memsize(struct ufb_data * ufb)
{	
	uint32_t ulreg;	
	ENTER();

	ulreg = peek32(ufb,MODE0_GATE);
	ulreg = FIELD_SET(ulreg,MODE0_GATE,GPIO,ON);
	poke32(ufb,MODE0_GATE,ulreg);
	
	ulreg = ddk750_getFrameBufSize(ufb);
	LEAVE ((int)ulreg);
}

int hw_init_chip(struct ufb_data * ufb,struct sm750fb_state * state)
{
	int ret;
	initchip_param_t parm;
	ENTER();	
	
	parm.powerMode = 0;
	parm.chipClock = state->chip_clk;
	parm.memClock = state->mem_clk;
	parm.masterClock = state->master_clk;
	parm.setAllEngOff = state->de_off;
	parm.resetMemory = 1;
	
	ret = ddk750_initChipParm(ufb,&parm);	

	if(!sii9022xInitChip(ufb))
	{	
		/* init sii9022a hdmi controller done */
		ufb->output0 = output_hdmi;
		inf_msg("find sii9022x hdmi controller\n");
	}else if(!ddk750_initDisplay(ufb)){
		/* init sii164 dvi-d controller done */
		ufb->output0 = output_dvid;
		inf_msg("find sii164 dvi controller\n");
	}else{
		ufb->output0 = output_analog;
	}
	
#if 0
#ifdef USE_DVICHIP
	int ct = 1;
	while(ct<4){
		if(ddk750_initDisplay(ufb)){
			error("Init DVI-Display failed:%d",ct);
		}else{
			break;
		}
		ct++;
	}
#elif defined (USE_HDMICHIP)
	if((ret = sii9022xInitChip(ufb)) < 0)
	{
		error("Init HDMI-Tx chip failed!");
		ret = 0;
	}
#endif
#endif
	LEAVE (ret);
}

#ifdef USE_HDMICHIP
int hw_light_hdmi(struct ufb_data * ufb,mode_parameter_t * modeparm)
{
	ENTER();
	sii9022xSetMode(ufb,modeparm);
	LEAVE(0);
}
#endif
/*
	1,Translate var information into struct known by ddk
	2,Call ddk set mode routines
*/
int hw_set_mode(const struct sm750fb_par * par,const struct fb_var_screeninfo *var,const struct fb_fix_screeninfo * fix)
{
	int ret;
	uint32_t pixelclock_hz;
	logicalMode_t logic;
	mode_parameter_t modparm;
	struct ufb_data * ufb;

	ENTER();		
	ufb = par->priv;
	logic.x = var->xres;
	logic.y = var->yres;
	
	logic.xLCD = 0;
	logic.yLCD = 0;
	if(logic.xLCD < logic.x || logic.yLCD < logic.y ||(logic.xLCD * logic.yLCD) <= (logic.x * logic.y))		
	{
		logic.xLCD = logic.yLCD = 0;
	}

	/*
		1,logic.hz is only used by expansion mode which will only be 60hz
		2,logic.hz can not be got from var ... if you want get it , 
		   write an simple text-anylise routine yourself.
	*/
	logic.hz = 60;
	logic.baseAddress = par->screen.offset;
	logic.pitch = fix->line_length;
	logic.bpp = var->bits_per_pixel;
    if(par->convert32to16)
    {
        logic.pitch >>= 1;
        logic.bpp >>= 1;
        dbg_msg("cut the pitch to %ld and bpp to %ld!\n", logic.pitch, logic.bpp);
    }
	logic.dispCtrl = (par->controller == CTRL_PRI)?PANEL_CTRL:CRT_CTRL;
	
	if(logic.xLCD != 0 && logic.yLCD != 0 )
	{
		ret = ddk750_setModeEx(ufb,&logic);
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
		modparm.vertical_frequency = (modparm.pixel_clock / modparm.horizontal_total)/modparm.vertical_total;
		ret = ddk750_setCustomMode(ufb,&logic,&modparm);

	}	
	
	if(ret){
		error("error in set mode !");
		goto ERR_SETMODE;		
	}

	if(par->idx == 1)
	{
		/* handle output2 */
		ret = ddk750_setLogicalDispOutput(ufb,CRT_2_SECONDARY,0);
	}
	else
	{
		/* handle output1 */
		if(ufb->outputs == 1){
			/* do SIMUL mode,all output use Primary Channel */
			ret = ddk750_setLogicalDispOutput(ufb,PANEL_CRT_SIMUL,0);
		}else{
			/* Just take care of primary channel and PANEL output */
			ret = ddk750_setLogicalDispOutput(ufb,PANEL_2_PRIMARY,0);
		}
	}
#ifdef USE_HDMICHIP
	if(ufb->output0 == output_hdmi)
		hw_light_hdmi(ufb,&modparm);
#endif

	if(ret){
		error("error in set logic output !\n");
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
	ddk750_deInit(ufb,logic.dispCtrl);

	//hw_dump_reg(par,0);
	
	LEAVE(ret);
ERR_SETOUTPUT:	
ERR_SETMODE:	
	LEAVE(-1);
}

int hw_set_blank(const struct sm750fb_par *par,int flag)
{
	uint32_t dpms,pps,crtdb;
	int retv;
	struct ufb_data* ufb;
	
	//ENTER();		
	ufb = par->priv;
	
	retv = 0;
	dpms = pps = crtdb = 0;
	switch (flag)
	{		
		case FB_BLANK_UNBLANK:	
			dpms = SYSTEM_CTRL_DPMS_VPHP;
			pps = PANEL_DISPLAY_CTRL_DATA_ENABLE;
			crtdb = CRT_DISPLAY_CTRL_BLANK_OFF;			
			break;
		case FB_BLANK_NORMAL:
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
	poke32(ufb,SYSTEM_CTRL,FIELD_VALUE(peek32(ufb,SYSTEM_CTRL),SYSTEM_CTRL,DPMS,dpms));
	poke32(ufb,PANEL_DISPLAY_CTRL,FIELD_VALUE(peek32(ufb,PANEL_DISPLAY_CTRL),PANEL_DISPLAY_CTRL,DATA,pps));
	poke32(ufb,CRT_DISPLAY_CTRL,FIELD_VALUE(peek32(ufb,CRT_DISPLAY_CTRL),CRT_DISPLAY_CTRL,BLANK,crtdb));
	
	//LEAVE(retv);
}

int hw_2d_wait_idle(const struct sm750fb_par * par)
{
	struct ufb_data * ufb = par->priv;
	return ddk750_2d_waitIdle(ufb);
}

int hw_set_colreg(const struct sm750fb_par *par,int index,ushort r,ushort g,ushort b)
{
	struct ufb_data * ufb = par->priv;
	static uint32_t add[]={PANEL_PALETTE_RAM,CRT_PALETTE_RAM};
	poke32(ufb,add[par->controller] + index*4 ,(r<<16)|(g<<8)|b);
	return 0;
}

int hw_dump_reg(const struct sm750fb_par * par,ushort flag)
{
	struct ufb_data * ufb = par->priv;
	int index;
	for(index=0;index<=0x88;index+=4){
		printk("0x%08x=%08x\n",index,peek32(ufb,index));
	}

	for(index=0x80000;index<=0x80034;index+=4){
		printk("0x%08x=%08x\n",index,peek32(ufb,index));
	}
	for(index=0x80040;index<=0x80068;index+=4){
		printk("0x%08x=%08x\n",index,peek32(ufb,index));
	}
	return 0;
}

int hw_pan_display(const struct sm750fb_par *par,const struct fb_var_screeninfo*var)
{	
	struct ufb_data * ufb;
	/* panning can be implemented by two methods:
		#1,adjust panel base address
		#2,use Panel Panning Control register or Panel Window Width
		     for PANEL (though I've not used it yetl)
		we use #1 for safe
	*/	
	struct fb_info* info;
	static uint32_t address[2]={PANEL_FB_ADDRESS,CRT_FB_ADDRESS};	
	if((var->xoffset + var->xres > var->xres_virtual)||
		(var->yoffset + var->yres > var->yres_virtual))
	{
		LEAVE(-EINVAL);
	}
	ufb = par->priv;
	info = par->info;
#if 0	
	if(par->controller == CTRL_PRI)
	{
		data = FIELD_SET(0, PANEL_PAN_CTRL,VERTICAL_PAN, var->yoffset)|
			FIELD_SET(0,PANEL_PAN_CTRL,VERTICAL_VSYNC,4)|
			FIELD_SET(0,PANEL_PAN_CTRL,HORIZONTAL_PAN,var->xoffset)|
			FIELD_SET(0,PANEL_PAN_CTRL,HORIZONTAL_VSYNC,4);
		poke32(ufb,PANEL_PAN_CTRL,data);
	}
	else
#endif		
	{
		/* use #1*/
		int total = var->yoffset * info->fix.line_length + ((var->xoffset * var->bits_per_pixel) >> 3);
		total += par->screen.offset;
		poke32(ufb,address[par->controller],
			FIELD_VALUE(peek32(ufb,address[par->controller]),PANEL_FB_ADDRESS,ADDRESS,total));		
		//inf_msg("CRT_FB_ADDRESS = %08x \n",peek32(ufb,CRT_FB_ADDRESS));
	}
	LEAVE(0);
}
void hw_cursor_enable(const struct sm750fb_par * par,int on)
{
	uint32_t value;
	static uint32_t add[2]={PANEL_HWC_ADDRESS,CRT_HWC_ADDRESS};	
	struct ufb_data * ufb;
	ufb = par->priv;
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
	
	poke32(ufb,add[par->controller],value);
}

void hw_cursor_clear(const struct sm750fb_par * par)
{
/*	
	if(par->cursor.pvAdd!=NULL)
		memset(par->cursor.pvAdd,0x0,MAX_CURS_W*MAX_CURS_H*2/8);
		*/
}

void hw_cursor_setsize(const int w,const int h)
{
	hwc_w = w;
	hwc_h = h;
}

int hw_cursor_setpos(const struct sm750fb_par * par,uint32_t dx,uint32_t dy)
{
	static uint32_t add[2]={PANEL_HWC_LOCATION,CRT_HWC_LOCATION};
	struct ufb_data * ufb;
	uint32_t data;
	if(dx>2047 || dy>2047)
		return -1;	
	

	ufb = par->priv;
	data = FIELD_VALUE(0,PANEL_HWC_LOCATION,Y,dy)|
		FIELD_VALUE(0,PANEL_HWC_LOCATION,X,dx);
	poke32(ufb,add[par->controller],data);
	return 0;
}

int hw_cursor_setcolor(const struct sm750fb_par * par,u32 bg,u32 fg)
{
	struct ufb_data * ufb;
	ufb = par->priv;
	/*
		fb only use mono color for cursor
		we put bg into color_1 and fg into color_2
	*/
	static uint32_t add[] = {PANEL_HWC_COLOR_12,CRT_HWC_COLOR_12};
	poke32(ufb,add[par->controller],(fg<<16) |(bg & 0xffff));
	return 0;
}
int hw_cursor_setdata(const struct sm750fb_par * par,
						const struct fb_cursor * cur,
						const u8 *pcol,const u8 *pmsk)
{
	int i,j,count,pitch,offset;
	uint8_t color,mask,opr;
	uint16_t data;
	uint16_t * pbuffer,*pstart;
	struct ufb_data * ufb;
	ufb = par->priv;

	/* ignore cursor stuff byfar */
	
	/*  in byte*/
	pitch = hwc_w >> 3;

	/* in byte	*/
	count = pitch * hwc_h;

	/* in ushort */
	offset = MAX_CURS_W * 2 / 8 / 2;
	
	data = 0;	
	pstart = (u16 *)par->cursor.pvAdd;
	//pstart -= offset;
	pbuffer = pstart;
	
	for(i=0;i<count;i++)
	{
		color = *pcol++;
		mask = *pmsk++;
		data = 0;

		if(cur->rop == ROP_XOR)
			opr = mask ^ color;
		else
			opr = mask & color;

		for(j=0;j<8;j++)
		{
			
			if(opr & (0x80 >> j))
			{	//use fg color,id = 2
				data |= 2 << (j*2);
			}else{
				//use bg color,id = 1
				data |= 1 << (j*2);
			}
		}
		*pbuffer = data;

		if((i+1) % pitch == 0){
			/* need a return */
			pstart += offset;
			pbuffer = pstart;
		}else{
			pbuffer++;
		}
		
	}

	/* fill cursor buffer via urb,total 1kb */
	i=0;
	while(i<1024){
		sm_fill_fb32(ufb,par->cursor.offset+i,par->cursor.pvAdd+i,32,1);
		i+=32;
	}
	return 0;	
}

int hw_2d_fillrect(const struct sm750fb_par * par,
			uint32_t base,uint32_t pitch,uint32_t bpp,
			uint32_t dx,uint32_t dy,uint32_t width,uint32_t height,
			uint32_t color,uint32_t rop2)
{	
	return ddk750_deRectFill(par->priv,base,pitch,bpp,dx,dy,width,height,color,rop2);
}

void hw_2d_copyarea(const struct sm750fb_par * par,
			uint32_t base,uint32_t pitch,uint32_t bpp,
			uint32_t dx,uint32_t dy,uint32_t sx,uint32_t sy,
			uint32_t width,uint32_t height)
{
	ddk750_deVideoMem2VideoMemBlt(par->priv,base,pitch,sx,sy,
									base,pitch,bpp,dx,dy,
									width,height,HW_ROP2_COPY);
}

int hw_2d_imageblit_mono(const struct sm750fb_par * par,
			uint32_t base,uint32_t pitch,uint32_t bpp,
			uint32_t dx,uint32_t dy,
			uint32_t width,uint32_t height,
			const unsigned char * data,
			uint32_t fgcol,uint32_t bgcol)
{
	int ret;
	struct ufb_data * ufb = par->priv;
	ret = ddk750_deSystemMem2VideoMemMonoBlt(ufb,(unsigned char *)data,width>>3,0,
											base,pitch,bpp,dx,dy,
											width,height,fgcol,bgcol,HW_ROP2_COPY);
	return ret;
}


int ufb_io_peek(struct ufb_data * ufb,void __user * data)
{	
//	ENTER();
	struct x_io_reg_parameter para;
	copy_from_user(&para,data,sizeof(para));
	para.value = SMI_READ_MMIO(ufb,para.offset);
	copy_to_user(data,&para,sizeof(para));
//	LEAVE(0);
	return 0;
}

int ufb_vm_peek(struct ufb_data * ufb,void __user * data)
{	
	ENTER();
	struct x_io_reg_parameter para;
	copy_from_user(&para,data,sizeof(para));
	para.value = SMI_READ_VMEM(ufb,para.offset);
	copy_to_user(data,&para,sizeof(para));
	LEAVE(0);
}


int ufb_io_poke(struct ufb_data * ufb,void __user * data)
{
	//ENTER();
	struct x_io_reg_parameter para;
	copy_from_user(&para,data,sizeof(para));
	SMI_WRITE_MMIO(ufb,para.offset,para.value);
//	copy_to_user(data,&para,sizeof(para));
//	LEAVE(0);
	return 0;
}

static int hw_usb_update_rect(struct fb_info * info,
		int x,int y,
		int w,int h,
		int bpp,int pitch)
{
	struct sm750fb_par * par;
	struct ufb_data * ufb;
	int contig,total,step,ret;
	uint32_t tmp,base,offset;
	char * pointer;
    ENTER();
	tmp = 0;
	bpp >>= 3;
	par = info->par;
	ufb= par->priv;

	contig = (w*bpp == pitch)?1:0;
	step = w*bpp;
	total = step*h;
	if(contig)
		step = total;

	offset = 0;

	base = par->priv_vm_offset;

	base += y * pitch + x*bpp;
	pointer = par->screen.pvAdd;	
	pointer += y * pitch + x*bpp;

	while(tmp < total){
		ret = smi_fill_dma(ufb,
			base + offset,
			pointer + offset,
			step);
		if(ret)
			return ret;
		tmp += step;
		offset += pitch;
	}	
	LEAVE(0);
}

int hw_usb_update_rect_repack_noblt(struct fb_info *info,
		int x,int y,int w,int h,int offset)
{
	int bpp,pitch;
	struct sm750fb_par * par;
	struct ufb_data * ufb;
	int total,length,Bpp,lines,sum;
	void * pointer,* pb2,*pb1,*tmp;
	int ret;

	bpp = info->var.bits_per_pixel;
	pitch = info->fix.line_length;

	ret = 0;
	Bpp = bpp >> 3;
	par = info->par;
	ufb= par->priv;

	length = w * Bpp;
	/* source pitch of 2d engine need align with 16 */
	length = PADDING(16,length);
	
	w = length / Bpp;

	total = length * h;
	pointer = par->screen.pvAdd + y * pitch + x * Bpp;
	pb2 = pointer + h * pitch;

	lines = BUF_DMA_SIZE/length;
	if(lines > h)
		lines = h;

    if((par->convert32to16)&&(bpp == 32))
      total /= 2;
    dbg_msg("convert switch 1: %ld\n",par->convert32to16);
    
	ret = PciMm_DmaStage0(ufb,SMI_VMEM_BAR0 + offset,total);
	if(ret < 0)
		return ret;

	do{
		tmp = ufb->buf_dma;
		sum = 0;
		pb1 = pointer + lines * pitch;
		if(pb1 > pb2)
			pb1 = pb2;

		do{
			memcpy(tmp,pointer,length);
			sum += length;
			tmp += length;
			pointer += pitch;
		}while(pointer < pb1);
		/* send to dma endpoint */
		ret = PciMm_DmaStage1(ufb,ufb->buf_dma,sum, par->convert32to16);
		if(ret < 0)
			return ret;
	}while(pointer < pb2);
	return ret;
}


int hw_usb_update_rect_repack(struct fb_info * info,
		int x,int y,
		int w,int h,
		int bpp,int pitch)
{
	struct sm750fb_par * par;
	struct ufb_data * ufb;
	int total,length,Bpp,lines,sum;
	void * pointer,* pb2,*pb1,*tmp;
	int ret;
    int reg;
    uint32_t offset;
    int dbg = 0;
	ret = 0;
	Bpp = bpp >> 3;
	par = info->par;
	ufb= par->priv;
  //pitch = 0x800;
     if(par->convert32to16)
      dbg = 0;
     par->convert32to16 = 0;
     
	length = w * Bpp;
	/* source pitch of 2d engine need align with 16 */
	length = PADDING(16,length);
  
    if((par->convert32to16)&&((length/2)%16))
      length+=16;
    
    //dbg_msg("convert switch 2: %ld\n",par->convert32to16);
    w = length / Bpp;

	total = length * h;
	pointer = par->screen.pvAdd + y * pitch + x * Bpp;
	pb2 = pointer + h * pitch;

	lines = BUF_DMA_SIZE/length;
	if(lines > h)
		lines = h;

    if((par->convert32to16)&&(bpp == 32))
      total /= 2;
	ret = PciMm_DmaStage0(ufb,SMI_VMEM_BAR0 + par->priv_vm_offset,total);
	if(ret < 0)
		return ret;
	do{
		tmp = ufb->buf_dma;
		sum = 0;
		pb1 = pointer + lines * pitch;
		if(pb1 > pb2)
			pb1 = pb2;

		do{
			memcpy(tmp,pointer,length);
			sum += length;
			tmp += length;
			pointer += pitch;
		}while(pointer < pb1);
		/* send to dma endpoint */
		ret = PciMm_DmaStage1(ufb,ufb->buf_dma,sum, par->convert32to16);
		if(ret < 0)
			return ret;
	}while(pointer < pb2);
#if 1
    if((par->convert32to16)&&(bpp == 32))
    {
        bpp = 16;
        length /= 2;
        pitch /= 2;
    }
//    dbg_msg("vv blit length: %lx, pitch: %lx, bpp: %lx, x: %lx, y:%lx, \n",length, pitch, bpp, x, y);
	/* copy private offscreen damage area upto onscreen */
	ret = (int)ddk750_deVideoMem2VideoMemBlt(
			ufb,
			par->priv_vm_offset,
			length,0,0,
			par->screen.offset,
			pitch,bpp,x,y,
			w,h,
			HW_ROP2_COPY);
#endif
if(dbg)
{
    dbg_msg("Get the start 200 bytes from OFFscreen\n");
    for(offset = par->priv_vm_offset; offset< (par->priv_vm_offset+0x8000); offset++)
    {
      reg = peek32_vm(ufb, offset);
      if(reg!=0xffffffff)
        dbg_msg("1:%lx-%lx",offset,reg);
    }
    dbg_msg("\n");
   #if 0
    dbg_msg("Get the start 200 bytes from ONscreen\n");
    for(offset = par->screen.offset; offset< (par->screen.offset+0x8000); offset++)
    {
      reg = peek32_vm(ufb, offset);
      if(reg!=0xffffffff)
        dbg_msg("2:%lx-%lx",offset,reg);
    }
    #endif
    dbg_msg("\n");
}
	if(ret == -1)
		return -ETIMEDOUT;/* time out */
	return ret;
}



int hw_update_damage(struct fb_info * info,
		int dx,int dy,
		int width,int height)
{
	int ret,bpp,pitch;

	ret = 0;
	bpp = info->var.bits_per_pixel;
	//pitch = info->fix.line_length;
    pitch = ((info->var.xres) * (info->var.bits_per_pixel)) >> 3;	
    pitch = PADDING(16,pitch);
    
	if(width & 3)
		width = PADDING(4,width);
	ret = hw_usb_update_rect_repack(info,dx,dy,width,height,bpp,pitch);
	return ret;
}

#if (CSC_PATCH == 1)
static void hw_wait_csc_idle(struct ufb_data * ufb)
{
	int wait = 0;
	uint32_t csctrl;
	do{		
		csctrl = peek32(ufb,0x1000fc);
		wait++;
		if(wait == 5)
			break;
	}while(csctrl & 0x80000000);
}
#endif

int ufb_io_video(struct fb_info * info,void * arg)
{
	int ret,cc;
#if 0
	int i,j;
#endif
	struct sm750fb_par * par;
	struct ufb_data * ufb;
	struct x_io_video_opt vopt;
	struct csc_regs * pRegs;
	ENTER();

	ret = 0;
	par = info->par;
	ufb = par->priv;
	ret = copy_from_user((void *)&vopt,(const void *)arg,sizeof(vopt));
	if(ret)
		LEAVE(-EFAULT);
	
	cc = vopt.csc.nOpt * sizeof(struct csc_regs);
	pRegs = kmalloc(cc,GFP_KERNEL);
	if(!pRegs)
		LEAVE(-ENOMEM);

	ret = copy_from_user((void*)pRegs,(const void*)vopt.csc.pRegs,cc);
	if(ret)
		LEAVE(-EFAULT);
	
	if(par->g_vopt.csc.pRegs != NULL)
		kfree(par->g_vopt.csc.pRegs);

	vopt.csc.pRegs = pRegs;
	memcpy(&par->g_vopt,&vopt,sizeof(vopt));

	/* copy data from virtual frame buffer (system memory) to usb-side device's video memory
	 * through USB */
	smi_fill_dma(ufb,
	vopt.yuv.offset_vm + par->screen.offset,
	vopt.yuv.offset_sm + par->screen.pvAdd,
	vopt.yuv.count);

#if 0
	if(vopt.overlay)
	{
		/* update overlay register if needed */
	}
#endif
	LEAVE(ret);
}

#if 0
int ufb_io_csc(struct fb_info * info,void * arg)
{
	int ret,idx,wait;
	uint32_t csctrl;
	struct sm750fb_par * par;
	struct ufb_data * ufb;
	struct x_io_csc parm;

	ENTER();
	
	idx = 0;
	par = info->par;
	ufb = par->priv;
	ret = copy_from_user((void*)&parm,(const void*)arg,sizeof(parm));
	if(ret)
		LEAVE(-EFAULT);

	memcpy(&lastcsc,&parm,sizeof(parm));
#if 1
	/* wait for CSC idle */
	wait = 0;
	do{		
		csctrl = peek32(ufb,0x1000fc);
		wait++;
		if(wait == 5)
			break;
	}while(csctrl & 0x80000000);
//	error("wait=%d\n",wait);
#endif
	
	/* set csc section registers */
	while(idx < 13){
		poke32(ufb,0x1000c8 + idx*4,parm.value[idx]);
		idx++;
	}

	/* kick csc engine */
	poke32(ufb,0x1000fc,(1<<31)|parm.control);
	LEAVE(0);
}
#endif


int ufb_io_transfer(struct fb_info * info,void * arg)
{
	int ret;
	struct sm750fb_par * par;
	struct ufb_data * ufb;
	struct x_io_transf parm;
	ENTER();

	par = info->par;
	ufb= par->priv;
	ret = copy_from_user((void*)&parm,(const void*)arg,sizeof(parm));
	if(ret)
		LEAVE(-EFAULT);
	
#if 1
	/* if total less than 64kb,do not use DMA*/
	if(parm.total < 0x10000){
		int i = 0;
		/* fill cursor buffer via urb,total 1kb */
		while(i<parm.total){
			sm_fill_fb32(ufb,par->screen.offset + parm.offset2 + i,
							par->screen.pvAdd + parm.offset1 + i,
							32,1);
			i+=32;
		}
	}
	else
#endif
	{
		smi_fill_dma(ufb,parm.offset2+par->screen.offset,par->screen.pvAdd + parm.offset1,parm.total);
	}
	LEAVE(0);	
}

#if (CSC_PATCH == 1)
static int csc_rect_intwith_box(int x1,int y1,int w1,int h1,int x2,int y2,int w2,int h2){
	if((x2-x1>w1) || (x1-x2>w2) || (y1-y2>h2) ||(y2-y1>h1))
		return -1;// not interected 
	return 0;
}

static int csc_rectpoint_match16(struct fb_info * info,struct csc_regs * pReg,int * actbox,uint32_t colorKey)
{
	struct sm750fb_par * par;
	struct ufb_data * ufb;
	int x,y,w,h;
	uint16_t * addr;
	uint16_t * addr0;
	uint16_t key;

	key = 0xffff & colorKey;

	par = info->par;
	ufb = par->priv;	
	x = pReg->value[(0xe8-0xc8)/4];
	y = x & 0xfff;
	x = (x >> 16) & 0xfff;

	addr0 = par->screen.pvAdd + y * par->screen.pitch + x * par->screen.Bpp;		
	/* left top point */
	addr = addr0;
	if(*addr != key)
		return -1;

	w = pReg->value[(0xec-0xc8)/4];
	h = w & 0xffff;
	w >>= 16;

	/* if real damaged area not interected with csc area,no csc redrawing needed */
	if(csc_rect_intwith_box(x,y,w,h,
		actbox[0],actbox[1],actbox[2],actbox[3]))
	{
		return -5;
	}


	/* right top point */
	addr = addr0 + w-1;
	if(*addr != key)
		return -2;
	
	/* left bottom point */
	addr = addr0 + (h-1) * (par->screen.pitch/2);
	if(*addr != key)
		return -3;
	
	/* right bottom point */
	addr += w - 1;
	if(*addr != key)
		return -4;
	
	return 0;	
}
#endif

static void ufb_Xdraw_routine(struct work_struct * work)
{
	int ret;	
	struct Xdraw_task * task;
	struct sm750fb_par * par;
	struct ufb_data * ufb;
	struct fb_info * info;
	struct x_io_damageArea area; 
	ENTER();
	
	task = container_of(work,struct Xdraw_task,work);
	info = task->priv;
	par = info->par;
	ufb= par->priv;
	/* do actually ufb transferring job */
	
	spin_lock(&ufb->sl_Xdraw);
	task = &ufb->buf_Xdraw.task[ufb->buf_Xdraw.ptr_r];
	info = task->priv;
	memcpy(&area,&task->area,sizeof(area));

	if(info == NULL){
		spin_unlock(&ufb->sl_Xdraw);
		error(" info is null,impossible should be...");
		LEAVE();
	}
	spin_unlock(&ufb->sl_Xdraw);
	ret = hw_usb_update_rect_repack(info,area.x,area.y,
			area.width,area.height,area.bpp,area.pitch);
	
	if(ret != 0){
		error(" failed!");
	}else{
		spin_lock(&ufb->sl_Xdraw);
		ufb->buf_Xdraw.ptr_r ++;
		if(ufb->buf_Xdraw.ptr_r == sizeof(ufb->buf_Xdraw.task)/sizeof(ufb->buf_Xdraw.task[0]))
			ufb->buf_Xdraw.ptr_r = 0;
		spin_unlock(&ufb->sl_Xdraw);
	}

	LEAVE();
}


int ufb_io_update_async(struct fb_info * info,void * arg)
#if 1
{
	int ret;
	struct boxRect box;
	struct sm750fb_par * par;
	struct ufb_data * ufb;
	struct x_io_damageArea area;

	//ENTER();
	ret = 0;
	par = info->par;
	ufb = par->priv;
	ret = copy_from_user((void*)&area,(const void *)arg,sizeof(area));
	if(ret)
		//LEAVE (-EFAULT);
       return (-EFAULT);
	/* sanity check */
	if(area.x < 0 || area.x > info->var.xres_virtual - 1)
		//LEAVE(-EINVAL);
	    return (-EINVAL);
	if(area.y < 0 || area.y > info->var.yres_virtual - 1)
		//LEAVE(-EINVAL);
        return (-EINVAL);
	if(area.width < 1 || area.width > info->var.xres_virtual)
		//LEAVE(-EINVAL);
	    return (-EINVAL);
	if(area.height < 1 || area.height > info->var.yres_virtual)
		//LEAVE(-EINVAL);
        return (-EINVAL);

	box.x1 = (uint16_t)area.x;
	box.y1 = (uint16_t)area.y;
	box.x2 = box.x1 + (uint16_t)area.width -1;
	box.y2 = box.y1 + (uint16_t)area.height -1;

	spin_lock(&par->slock_dj);
	if(box.x1 < par->damageArea.x1)
		par->damageArea.x1 = box.x1;
	
	if(box.y1 < par->damageArea.y1)
		par->damageArea.y1 = box.y1;
	
	if(box.x2 > par->damageArea.x2)
		par->damageArea.x2 = box.x2;
	
	if(box.y2 > par->damageArea.y2)
		par->damageArea.y2 = box.y2;

	spin_unlock(&par->slock_dj);
	//LEAVE(ret);
	return ret;
}
#else
{
	int ret;
	struct sm750fb_par * par;
	struct ufb_data * ufb;
	struct x_io_damageArea area;

	ENTER();
	ret = 0;
	par = info->par;
	ufb = par->priv;
	ret = copy_from_user((void*)&area,(const void *)arg,sizeof(area));
	if(ret)
		LEAVE (-EFAULT);

	/* sanity check */
	if(area.x < 0 || area.x > info->var.xres_virtual - 1)
		LEAVE(-EINVAL);
	
	if(area.y < 0 || area.y > info->var.yres_virtual - 1)
		LEAVE(-EINVAL);

	if(area.width < 1 || area.width > info->var.xres_virtual)
		LEAVE(-EINVAL);
	
	if(area.height < 1 || area.height > info->var.yres_virtual)
		LEAVE(-EINVAL);

	/* update */
	if(area.gui_update)
	{
		spin_lock(&ufb->sl_Xdraw);
		if(ufb->buf_Xdraw.ptr_w == ufb->buf_Xdraw.ptr_r){
			spin_unlock(&ufb->sl_Xdraw);
			ret = ufb_io_update(info,arg);
			LEAVE(ret);
		}
		
		INIT_WORK(&ufb->buf_Xdraw.task[ufb->buf_Xdraw.ptr_w].work,ufb_Xdraw_routine);
		ufb->buf_Xdraw.task[ufb->buf_Xdraw.ptr_w].priv = info;
		memcpy(&ufb->buf_Xdraw.task[ufb->buf_Xdraw.ptr_w].area,&area,sizeof(area));

		queue_work(ufb->wq_Xdraw,&ufb->buf_Xdraw.task[ufb->buf_Xdraw.ptr_w]);

		ufb->buf_Xdraw.ptr_w ++;
		if(ufb->buf_Xdraw.ptr_w == sizeof(ufb->buf_Xdraw.task)/sizeof(ufb->buf_Xdraw.task[0])){
			inf_msg(" ptr_w is 0,and ptr_r is %d\n",ufb->buf_Xdraw.ptr_r);
			ufb->buf_Xdraw.ptr_w = 0;		
		}
		spin_unlock(&ufb->sl_Xdraw);
	}
	LEAVE(ret);
}
#endif

#if 1
int ufb_io_update(struct fb_info * info,void* arg){
	int ret;
	struct sm750fb_par * par;
	struct ufb_data * ufb;
	struct x_io_damageArea area;
	ENTER();


	ret = 0;
	par = info->par;
	ufb = par->priv;
	ret = copy_from_user((void*)&area,(const void *)arg,sizeof(area));
	if(ret)
		LEAVE (-EFAULT);

	/* sanity check */
	if(area.x < 0 || area.x > info->var.xres_virtual - 1)
		LEAVE(-EINVAL);
	
	if(area.y < 0 || area.y > info->var.yres_virtual - 1)
		LEAVE(-EINVAL);

	if(area.width < 1 || area.width > info->var.xres_virtual)
		LEAVE(-EINVAL);
	
	if(area.height < 1 || area.height > info->var.yres_virtual)
		LEAVE(-EINVAL);

	area.bpp = info->var.bits_per_pixel;
	//area.pitch = info->fix.line_length;
    area.pitch = ((info->var.xres) * (info->var.bits_per_pixel)) >> 3;	
    area.pitch = PADDING(16,area.pitch);

	/* update */
	if(area.gui_update)
	{
		ret = hw_usb_update_rect_repack(info,area.x,area.y,
				area.width,area.height,area.bpp,area.pitch);
		if(ret == -1)
			error("ret = -1");
	}
	LEAVE(ret);
}

#else
int ufb_io_update(struct fb_info * info,void* arg)
{
	int ret;
	uint32_t sPitch,dPitch,sx,sy,dx,dy,bpp,width,height;
	struct sm750fb_par * par;
	struct ufb_data * ufb;
	struct x_io_damageArea area;
	ENTER();

	par = info->par;
	ufb = par->priv;
	ret = copy_from_user((void*)&area,(const void *)arg,sizeof(area));
	if(ret)
		LEAVE (-EFAULT);

	bpp = area.bpp;//bits per pixel
	sPitch = area.pitch;
	dPitch = sPitch;

	par->csc_playing = area.csc_playing;

#if 0
	area.x = 0;
	area.width = area.pitch / area.bpp;

	if(full_update){
		area.y = 0;
		area.height = info->var.yres_virtual;
	}
#endif

	/* safe check */
	if(area.x < 0)
		area.x = 0;
	
	if(area.x > area.pitch/area.bpp)
		area.x = area.pitch/area.bpp;
	
	if(area.y < 0)
		area.y = 0;

	width = area.width;
	height = area.height;

	sx = area.x;
	sy = area.y;

	dx = sx;
	dy = sy;

	if(area.gui_update)
	{
		ret = hw_usb_update_rect(info,area.x,area.y,
				area.width,area.height,area.bpp,area.pitch);

#if (CSC_PATCH == 1)

		/* for overlay video case, par->csc_playing == 0*/
		if(par->csc_playing != 0)
		{
			int reCsc = 1;
			struct csc_regs * pRegs;
			int i,j=0;		

			pRegs = par->g_vopt.csc.pRegs;
			/* detect if we need redraw the csc video */
			if(!area.csc_update){
				for(i=0;i< par->g_vopt.csc.nOpt;i++)
				{
					if(csc_rectpoint_match16(info,pRegs,&area.actualRect[0],area.colorKey))
					{
						reCsc = 0;
						break;
					}
					pRegs ++;
				}
			}
			/* detect over */

			if(reCsc != 0)
			{
				/* do a serial of CSC engine register setting */
				for(i = 0;i < par->g_vopt.csc.nOpt; i++){		
					hw_wait_csc_idle(ufb);
					pRegs = par->g_vopt.csc.pRegs;
					j = 0;
					while(j< 14)
					{
						poke32(ufb,0x1000c8+(j<<2),pRegs->value[j]);
						j++;
					}
					pRegs ++;
				}
			}

			deSetTransparency(ufb,1,0,1,area.colorKey);
		}//if csc_playing
#endif
		/* copy private offscreen damage area upto onscreen */
		ddk750_deVideoMem2VideoMemBlt(
				ufb,
				par->priv_vm_offset,
				sPitch,sx,sy,
				par->screen.offset,
				dPitch,bpp,dx,dy,
				width,height,
				HW_ROP2_COPY);

	}//if gui_update
	else if(par->csc_playing && area.csc_update)
	{
#if (CSC_PATCH == 1)
		int i,j;
		struct csc_regs * pRegs;
		/* do a serial of CSC engine register setting */
		for(i = 0;i < par->g_vopt.csc.nOpt; i++){		
			hw_wait_csc_idle(ufb);
			pRegs = par->g_vopt.csc.pRegs;
			j = 0;
			while(j< 14)
			{
				poke32(ufb,0x1000c8+(j<<2),pRegs->value[j]);
				j++;
			}
			pRegs ++;
		}
#endif

	}

	par->flip++;

	LEAVE(ret);
}
#endif

int ufb_io_updatesub(struct fb_info * info,void * arg)
{
	int ret,offset,Bpp;
	struct regionlist list0;
	struct regionlist *list;
	struct _region * ptr,*ptrEnd;
	struct sm750fb_par * par;
	struct ufb_data * ufb;

	par = info->par;
	ufb= par->priv;
	Bpp = info->var.bits_per_pixel>>3;

	ret = copy_from_user((void*)&list0,
			(const void *)arg,sizeof(list0));
	if(ret){
		error("@copy list0:%d\n",ret);
		return ret;
	}

	list = kmalloc(sizeof(*list) + sizeof(struct _region) * list0.cnt,GFP_KERNEL);
	if(!list){
		error("nomem\n");
		return -ENOMEM;
	}

	list->boxes = list + 1;
	memcpy(list,&list0,sizeof(list0));
	ret = copy_from_user(list->boxes,list0.boxes,
			sizeof(struct _region) * list0.cnt);
	if(ret)
	{
		error("@copy sub-box pointers:%d\n",ret);
		goto exit;
	}

	ptr = list->boxes;
	ptrEnd = ptr + list->cnt;
	offset = 0;
	while(ptr != ptrEnd){
		unsigned long w,h,length;
		w = ptr->x2 - ptr->x1 + 1;
		h = ptr->y2 - ptr->y1 + 1;
		length = w*Bpp;
		length = PADDING(16,length);
		if(ptr->bypass != 1)
		{
			ret = hw_usb_update_rect_repack_noblt(info,
					ptr->x1,ptr->y1,
					w,h,
					par->priv_vm_offset + offset);
			if(ret){
				goto exit;
			}
		}
		offset += length * h;
		ptr ++;
	}
#if 1
	/* do blit */
	ptr = list->boxes;
	offset = 0;
	while(ptr != ptrEnd){
		unsigned long w,h,sPitch;
		w = ptr->x2 - ptr->x1 + 1;
		h = ptr->y2 - ptr->y1 + 1;
		sPitch = w * Bpp;
		/* source pitch of 2d engine need align with 16 */
		sPitch = PADDING(16,sPitch);
		
		if(ptr->bypass != 1)
		{
			ret = (int)ddk750_deVideoMem2VideoMemBlt(
					ufb,
					par->priv_vm_offset + offset,
					sPitch,
					0,0,
					par->screen.offset,
					info->fix.line_length,
					info->var.bits_per_pixel,
					ptr->x1,ptr->y1,
					w,h,
					HW_ROP2_COPY);
			if(ret == -1){
				ret = -ETIMEDOUT;
				goto exit;
			}
		}
		offset += sPitch * h;
		ptr ++;
	}
#endif

exit:		
	kfree(list);
	return ret;
}

int hw_usb_kernel_to_vm(struct fb_info * info,
		int x,int y,
		int w,int h,
		int bpp,int pitch,int base)
{
	struct sm750fb_par * par;
	struct ufb_data * ufb;
	int total,length,Bpp,lines,sum;
	void * pointer,* pb2,*pb1,*tmp;
	int ret;

	ret = 0;
	Bpp = bpp >> 3;
	par = info->par;
	ufb= par->priv;

	length = w * Bpp;
	/* source pitch of 2d engine need align with 16 */
	length = PADDING(16,length);
	w = length / Bpp;

	total = length * h;
	pointer = base;
	pb2 = pointer + h * pitch;

	lines = BUF_DMA_SIZE/length;
	if(lines > h)
		lines = h;

    if((par->convert32to16)&&(bpp == 32))
      total /= 2;
    
	ret = PciMm_DmaStage0(ufb,SMI_VMEM_BAR0 + 0,total);
	do{
		tmp = ufb->buf_dma;
		sum = 0;
		pb1 = pointer + lines * pitch;
		if(pb1 > pb2)
			pb1 = pb2;

		do{
			memcpy(tmp,pointer,length);
			sum += length;
			tmp += length;
			pointer += pitch;
		}while(pointer < pb1);
		/* send to dma endpoint */
		ret = PciMm_DmaStage1(ufb,ufb->buf_dma,sum, par->convert32to16);
		if(ret < 0)
			return ret;
	}while(pointer < pb2);
#if 0
	/* copy private offscreen damage area upto onscreen */
	ret = (int)ddk750_deVideoMem2VideoMemBlt(
			ufb,
			y * pitch + x * Bpp,
			length,0,0,
			par->screen.offset,
			pitch,bpp,x,y,
			w,h,
			HW_ROP2_COPY);
	if(ret == -1)
		return -ETIMEDOUT;/* time out */
#endif
	return ret;
}
int ufb_io_kernel_to_vm(struct fb_info * info,void* arg){
	int ret;
	int yuv_size;
	struct sm750fb_par * par;
	struct ufb_data * ufb;
	struct x_io_damageArea area;
	ENTER();


	ret = 0;
	par = info->par;
	ufb = par->priv;
	ret = copy_from_user((void*)&area,(const void *)arg,sizeof(area));
	if(ret)
		LEAVE (-EFAULT);

	/* sanity check */
	if(area.x < 0 || area.x > info->var.xres_virtual - 1)
		LEAVE(-EINVAL);

	if(area.y < 0 || area.y > info->var.yres_virtual - 1)
		LEAVE(-EINVAL);

	if(area.width < 1 || area.width > info->var.xres_virtual)
		LEAVE(-EINVAL);

	if(area.height < 1 || area.height > info->var.yres_virtual)
		LEAVE(-EINVAL);

	if(info->var.bits_per_pixel == 32) {
		area.bpp = info->var.bits_per_pixel/2;
		area.pitch = info->fix.line_length/2;
	} else {
		area.bpp = info->var.bits_per_pixel;
		area.pitch = info->fix.line_length;
	}
	yuv_size = area.pitch * area.height;
	char *kyuv = kmalloc(yuv_size, GFP_KERNEL);
	ret = copy_from_user(kyuv, area.base, yuv_size);	
	if (ret)
		LEAVE(-EFAULT);

	ret = hw_usb_kernel_to_vm(info, area.x, area.y,
			area.width, area.height, area.bpp, area.pitch, kyuv);
	kfree(kyuv);
	if(ret == -1)
		error("ret = -1");
	LEAVE(ret);
}


