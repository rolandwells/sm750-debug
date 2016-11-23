#include	<linux/version.h>
#include	<linux/kernel.h>
#include	<linux/errno.h>
#include	<linux/init.h>
#include	<linux/slab.h>
#include	<linux/module.h>
#include	<linux/kref.h>
#include	<linux/uaccess.h>
#include	<linux/usb.h>
#include	<linux/mutex.h>
#include	<linux/fb.h>
#include	<linux/ioport.h>
#include	<linux/vmalloc.h>
#include	<linux/pagemap.h>
#include	<linux/screen_info.h>
#include 	<linux/console.h>
#include	<linux/timer.h>
#include	<linux/delay.h>
//#include	<linux/atomic.h>
#include	<linux/workqueue.h>
#include	<linux/spinlock.h>
#include	<linux/i2c.h>
#include	<linux/i2c-algo-bit.h>

#include "./ddk/ddk750_type.h"
#include "./ddk/edid.h"
#include	"usbfb.h"
#include	"plx.h"
#include	"ufb_drv.h"
#include	"ufb_hw750.h"
#ifdef _USE_RANDR_
#include 	"./ddk/ddk750_regs.h"
#include 	"./ddk/ddk750_func.h"
#include 	"./ddk/ddk750_intern.h"
#include    "ufb_hw750.h"
#endif

#define DRV_NAME "smi-ufb"
#define VER_MAJOR 1
#define VER_MINOR 3
#define VER_PATCH 1
#define VER_ATTACH(a,b,c) #a"."#b"."#c
#define VER_ATTACH2(a,b,c) VER_ATTACH(a,b,c)
#define SMI_KFB_VER_STRING VER_ATTACH2(VER_MAJOR,VER_MINOR,VER_PATCH)

static int fbconsole = 1;
static int smHeads[8] = {0,0,0,0,0,0,0,0};
struct mutex g_io_mutex;/* used for preventing multi-thread accessing */
//for userspace udev
static char g_env_verdor[] = "VENDOR=126F";
static char g_env_device[] = "DEVICE=0750";
static char g_env_head_index[30];

int smi_indent = 0;
static int outputs = 0;

static char * mode_option = NULL;
static char defaultMode[] = {DEFAULT_MODE};
//static enum sm750fb_pnltype pnltype = PNLTYPE_24;
//static enum sm750fb_output output = OP_SIMUL_CRT1_CRT2;
static struct sm750fb_state default_state = {
	.chip_clk = DEFAULT_CHIP_CLK,
	.mem_clk = DEFAULT_MEMORY_CLK,
	.master_clk = DEFAULT_MASTER_CLK,
	.de_off = 1,
	.output = OP_SIMUL_CRT1_CRT2,
	.xLCD = 0,
	.yLCD = 0,
};

#define FB_SYNC_HOR_LOW_ACT 0
#define FB_SYNC_VERT_LOW_ACT 0

static const struct fb_videomode sm750modbExt[] = {
	/*  	1024x600-60 VESA 	[1.71:1]	*/
	{NULL,  60, 1024, 600, 20423, 144,  40, 18, 1, 104, 3,
	FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED},

	/* 	1024x600-70 VESA */
	{NULL,  70, 1024, 600, 17211, 152,  48, 21, 1, 104, 3,
	FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED},

	/*  	1024x600-75 VESA */
	{NULL,  75, 1024, 600, 15822, 160,  56, 23, 1, 104, 3,
	FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED},

	/*  	1024x600-85 VESA */
	{NULL,  85, 1024, 600, 13730, 168,  56, 26, 1, 112, 3,
	FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED},

	/*	720x480	*/
	{NULL, 60,  720,  480,  37427, 88,   16, 13, 1,   72,  3,
	FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED},


	{NULL,60,1360,768,11804,208,64,23,1,144,3,
	FB_SYNC_HOR_HIGH_ACT|FB_VMODE_NONINTERLACED},

	/*	1360 x 768	[1.77083:1]	*/
	{NULL,  60, 1360, 768, 11804, 208,  64, 23, 1, 144, 3,
	FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED},

	/*	1368 x 768      [1.78:1] */
	{NULL, 60,  1368,  768,  11647, 216, 72, 23, 1,  144, 3,
	FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED}, 

	/* 	1440 x 900		[16:10]	*/
	{NULL, 60, 1440, 900, 9392, 232, 80, 28, 1, 152, 3,
	FB_SYNC_VERT_HIGH_ACT,FB_VMODE_NONINTERLACED},

	/*	1440x960		[15:10]	*/
	{NULL, 60, 1440, 960, 8733, 240, 88, 30, 1, 152, 3, 
	FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED}, 

#ifdef USE_HDMICHIP
	/* CEA-861-D.pdf 1280x720p@60,vic=4 */
	{"CEA-1280x720p@60hz",60,1280,720,13481,220,110,20,5,40,5,
	FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,FB_VMODE_NONINTERLACED},
#else
	/*	1280x720		[1.78:1]	*/	
	{NULL, 60,  1280,  720,  13426, 162, 86, 22, 1,  136, 3,
	FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,FB_VMODE_NONINTERLACED},
#endif

#ifdef USE_HDMICHIP
	{"CEA-1920X1080P@60hz",60,1920,1080,6741,148,88,36,4,44,5,
		FB_SYNC_COMP_HIGH_ACT |
		FB_SYNC_HOR_HIGH_ACT | 
		FB_SYNC_VERT_HIGH_ACT,
		FB_VMODE_NONINTERLACED},
#else
	/*	1920x1080	[16:9]	*/
	{NULL, 60, 1920, 1080, 6734, 148, 88, 41, 1, 44, 3,
#if 1
	FB_SYNC_VERT_HIGH_ACT,FB_VMODE_NONINTERLACED},
#else
	FB_SYNC_HOR_HIGH_ACT,FB_VMODE_NONINTERLACED},
#endif
#endif
};


static inline int h_total(struct fb_var_screeninfo *var)
{
	return var->xres + var->left_margin +var->right_margin + var->hsync_len;
}

static inline int v_total(struct fb_var_screeninfo *var)
{
	return var->yres + var->upper_margin +var->lower_margin + var->vsync_len;
}

static void * rvmalloc(unsigned long size){
	void * mem;
	unsigned long adr;

	ENTER();
	size = PAGE_ALIGN(size);
	//mem = vmalloc_32(size);
	mem = vmalloc(size*2);
	if(!mem)
		LEAVE(NULL);
	memset(mem,0,size);
	adr = (unsigned long)mem;

	while(size > 0){
		SetPageReserved(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
	LEAVE(mem);
}

static void rvfree(void * mem,unsigned long size){
	unsigned long adr;
	ENTER();
	if(!mem)
		LEAVE();
	
	adr = (unsigned long)mem;
	while((long)size > 0){
		ClearPageReserved(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
	vfree(mem);
	LEAVE();
}

static void __init ufb_setup(char * options){
	char * opt;
	if(!options || !*options){
		inf_msg("no options\n");	
	}else{
		inf_msg("options:%s\n",options);
	}

	while((opt = strsep(&options,",")) != NULL){
		/*
		if(!strncmp(opt,"nodual",strlen("nodual"))){
			dual = 0;
		}else if(!strncmp(opt,"dual",strlen("dual"))){
			dual = 1;
		}
		else
		*/
		{
			mode_option = (mode_option == NULL)?opt:mode_option;
			inf_msg("mode string = %s \n",mode_option);
		}
	}
	LEAVE();
}


static int sm750fb_ops_check_var(struct fb_var_screeninfo* var,struct fb_info* info)
{
	/*
	 * checks var and eventually tweaks it to something supported.
	 * Do not modify PAR
	 */
 	int ret;
	uint32_t length;
	ret = 0;
//	inf_msg("checking var:X = %d,Y = %d",var->xres,var->yres);
#if 0
	ulong ultmp;
	if(var->hsync_len > 255 || var->vsync_len > 63)
		return -EINVAL;
	if((var->xres + var->right_margin) > 4096 )
		return -EINVAL;
	if((var->yres + var->lower_margin) > 2048 )
		return -EINVAL;
	if(h_total(var) > 4096 || v_total(var) >2048 )
		return -EINVAL;
	ultmp = (var->xres * var->bits_per_pixel) >> 3;
	if(( ultmp & 0xf ) !=0 ){
		return -EINVAL;
	}
	if(var->xres_virtual > 4096 || var->yres_virtual > 2048 )
		return -EINVAL;
#endif
	
#if 0
	if (var->xres == 1680 && var->yres == 1050){
		//error("do not support 1680x1050 byfar");
		return(-EINVAL);//temporarily disable this mode,which show strong underflow under samsung2494
	}
#endif
	
	length = (var->xres_virtual * var->bits_per_pixel) >> 3;	
	length = PADDING(16,length);
	length *= var->yres_virtual;
	if(length > (info->screen_size - CURSOR_SPACE_SIZE)){
		printk("mode:%dx%dx%dbits need 0x%u bytes,while only 0x%lu available\n",
				var->xres,var->yres,var->bits_per_pixel,
				length,info->screen_size - CURSOR_SPACE_SIZE);
		return (-ENOMEM);
	}
	
	/*
	 * can cope with 8,16,32 bpp
	 */	
	if(var->bits_per_pixel <= 8)
		var->bits_per_pixel = 8;
	else if(var->bits_per_pixel <=16 )
		var->bits_per_pixel = 16;
	else
		var->bits_per_pixel = 32;
	switch(var->bits_per_pixel){
	case 8:
		var->red.offset = 0;
		var->red.length = 8;
		var->green.offset = 0;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.length = 0;
		var->transp.offset = 0;
		break;
	case 16:
		var->red.offset = 11;
		var->red.length = 5;
		var->green.offset = 5;	
		var->green.length = 6;
		var->blue.offset = 0;
		var->blue.length = 5;
		var->transp.length = 0;
		var->transp.offset = 0;
		break;	
	case 32:	
		var->red.offset = 16;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 0 ;
		var->blue.length = 8;
		break;
	default:
		error("do not support this bpp:%d",var->bits_per_pixel);
		return (-EINVAL);
	}		
	return (ret);
}


static int sm750fb_ops_set_par(struct fb_info* info)
{
	struct ufb_data * ufb;
	struct sm750fb_par * par = info->par;
	struct fb_var_screeninfo * var = &info->var;
	struct fb_fix_screeninfo * fix = &info->fix;
	int ret;
	int length;
	
	ENTER();	
	ufb = INFO_2_UFB(info);
	mutex_lock(&ufb->io_mutex);
	if(ufb->status == UFB_STAT_PLUGOUT){
		ret = -ENODEV;
		goto exit;
	}
#if 0
	error("xres,yres,xres_v,yres_v is %d,%d,%d,%d\n",
		var->xres,var->yres,var->xres_virtual,var->yres_virtual);
#endif
	ret = 0;
	length = (var->xres_virtual * var->bits_per_pixel) >> 3;	
	length = PADDING(16,length);
	par->screen.pitch = length;
	par->screen.Bpp = var->bits_per_pixel / 8;
	
	switch(var->bits_per_pixel)
	{
	case 8:
		info->fix.visual = FB_VISUAL_PSEUDOCOLOR;
		break;
	case 16:
	case 32:
	default:
		info->fix.visual = FB_VISUAL_TRUECOLOR;
		break;	
	}

	/* important */
	fix->line_length = length;
	/* set hardware */
	ret = hw_set_mode(par,var,fix);
	if(ret){
		error("failed to set mode.");
		goto exit;
	}
	
exit:
	mutex_unlock(&ufb->io_mutex);
	LEAVE (ret);
}
#ifdef _USE_RANDR_
//for the 2nd screen, randr.
static int ufb_set_2nd_screen(struct fb_info * info,void * arg)
{
	ENTER();	
    edid_data_p edid;
	struct ufb_data * ufb;
    crtc_logicalMode_p crtcLogical; 
    crtcLogical = (crtc_logicalMode_p)arg;
	struct sm750fb_par * par = info->par;
	int ret;
	int length;
  
	ufb = INFO_2_UFB(info);
	if(ufb->status == UFB_STAT_PLUGOUT){
		ret = -ENODEV;
		goto exit;
	}
#if 0
	error("xres,yres,xres_v,yres_v is %d,%d,%d,%d\n",
		var->xres,var->yres,var->xres_virtual,var->yres_virtual);
#endif
	ret = 0;

	/* set hardware */
////////////
	uint32_t pixelclock_hz;
	logicalMode_t logic;
	mode_parameter_t modparm;
		
	logic.x = crtcLogical->xres;
	logic.y = crtcLogical->yres;
	
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
	logic.baseAddress = 0;//by ilena: set to 0 case randr use 0......par->screen.offset;
	length = (crtcLogical->xres_virtual * crtcLogical->bits_per_pixel) >> 3;	
	length = PADDING(16,length);
	logic.pitch = length;
	logic.bpp = crtcLogical->bits_per_pixel;
//    par->flagofmirror = crtcLogical->mirrorflag;
    if(par->convert32to16)
    {
        logic.pitch >>= 1;
        logic.bpp >>= 1;
        dbg_msg("cut the pitch to %ld and bpp to %ld!\n", logic.pitch, logic.bpp);
    }
	logic.dispCtrl = CRT_CTRL;
	if(logic.xLCD != 0 && logic.yLCD != 0 )
	{
		ret = ddk750_setModeEx(ufb,&logic);
	}
	else
	{
		/* 	caculate  pixel clock in HZ 	*/			
		pixelclock_hz = sm750fb_ps_to_hz(crtcLogical->pixclock);
		/*	set mode parameter	*/
		modparm.pixel_clock = pixelclock_hz;	
		modparm.vertical_sync_polarity = (crtcLogical->sync & FB_SYNC_HOR_HIGH_ACT) ? POS:NEG;
		modparm.horizontal_sync_polarity = (crtcLogical->sync & FB_SYNC_VERT_HIGH_ACT) ? POS:NEG;
		modparm.clock_phase_polarity = (crtcLogical->sync& FB_SYNC_COMP_HIGH_ACT) ? POS:NEG;

		modparm.horizontal_display_end = crtcLogical->xres;
		modparm.horizontal_sync_width = crtcLogical->hsync_len;
		modparm.horizontal_sync_start = crtcLogical->xres + crtcLogical->right_margin;
		modparm.horizontal_total = crtcLogical->xres + crtcLogical->left_margin + crtcLogical->right_margin + crtcLogical->hsync_len;
		modparm.vertical_display_end = crtcLogical->yres;
		modparm.vertical_sync_height = crtcLogical->vsync_len;
		modparm.vertical_sync_start = crtcLogical->yres + crtcLogical->lower_margin;
		modparm.vertical_total = crtcLogical->yres + crtcLogical->upper_margin + crtcLogical->lower_margin + crtcLogical->vsync_len;
		modparm.vertical_frequency = (modparm.pixel_clock / modparm.horizontal_total)/modparm.vertical_total;
		ret = ddk750_setCustomMode(ufb,&logic,&modparm);

	}	

	if(ret){
		error("error in set mode !");
		goto ERR_SETMODE;		
	}
    if(par->flagofmirror == 1)
    {
			ret = ddk750_setLogicalDispOutput(ufb,PANEL_CRT_DUAL,0);
    }else if(par->flagofmirror ==2 )
    {
            ret = ddk750_setLogicalDispOutput(ufb,PANEL_CRT_SIMUL,0);
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
#if 0//for debug
	int i, j, k,l,m;
    for(i=0; i<0x90; ){
        j = peek32(ufb, i);
        i+=4;
        k = peek32(ufb, i);
        i+=4;
        l = peek32(ufb, i);
        i+=4;
        m = peek32(ufb, i);
        dbg_msg( "0x%8lx: 0x%8lx 0x%8lx 0x%8lx 0x%8lx\n",(i-0xc),j,k,l,m);
        i+=4;
       }
       
    for(i=0x80000; i<0x80070; ){
        j = peek32(ufb, i);
        i+=4;
        k = peek32(ufb, i);
        i+=4;
        l = peek32(ufb, i);
        i+=4;
        m = peek32(ufb, i);
        dbg_msg( "0x%8lx: 0x%8lx 0x%8lx 0x%8lx 0x%8lx\n",(i-0xc),j,k,l,m);
        i+=4;
        }

    for(i=0x80200; i<0x80250; ){
        j = peek32(ufb, i);
        i+=4;
        k = peek32(ufb, i);
        i+=4;
        l = peek32(ufb, i);
        i+=4;
        m = peek32(ufb, i);
        dbg_msg( "0x%8lx: 0x%8lx 0x%8lx 0x%8lx 0x%8lx\n",(i-0xc),j,k,l,m);
        i+=4;
       }
#endif	
/////////////////
	if(ret){
		error("failed to set mode.");
		goto exit;
	}
	
exit:
ERR_SETOUTPUT:	
ERR_SETMODE:
	LEAVE (ret);
 
}
#endif

static inline uint chan_to_field(uint32_t chan,struct fb_bitfield * bf){
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static int ufb_io_setmode(struct fb_info * info,void * arg)
{
	int ret;
	int linelen;
	char * modestr;
	struct cmd_io_setmode parm;
	struct fb_var_screeninfo *var;
	struct fb_fix_screeninfo *fix;
	ENTER();

	ret = 0;
	var = &info->var;
	fix = &info->fix;
	ret = copy_from_user((void*)&parm,(const void*)arg,sizeof(parm));
	if(ret)
		LEAVE(-EFAULT);

	modestr = kmalloc(parm.length+1,GFP_KERNEL);
	if(!modestr)
		LEAVE(-ENOMEM);

	memset(modestr,0,parm.length+1);
	ret = copy_from_user((void*)modestr,(const void*)parm.modestr,parm.length);
	if(ret){
		ret = -EFAULT;
		goto exit;
	}

	inf_msg("user want set mode:%s\n",modestr);

	ret = fb_find_mode(var,info,modestr,
		sm750modbExt,ARRAY_SIZE(sm750modbExt),NULL,16);
	
	if(ret!=1){
		dbg_msg("specify mode not in sm750modbExt.\n");	
		ret = fb_find_mode(var,info,modestr,NULL,0,NULL,16);
	}	

	if(ret==0 || ret == 4){
		error("failed to get specify mode.");
		ret = -EINVAL;
		goto exit;		
	}

	dbg_msg("member of info->var is :\n\
		xres=%d\n\
		yres=%d\n\
		xres_virtual=%d\n\
		yres_virtual=%d\n\
		xoffset=%d\n\
		yoffset=%d\n\
		bits_per_pixel=%d\n \
		...\n",var->xres,var->yres,var->xres_virtual,var->yres_virtual,
			var->xoffset,var->yoffset,var->bits_per_pixel);

	linelen = (var->xres_virtual * var->bits_per_pixel)/8;
	linelen = PADDING(16,linelen);


	//info->fix.type = FB_TYPE_PACKED_PIXELS;
	//info->fix.type_aux = 0;
	//info->fix.xpanstep = 1;
	//info->fix.ypanstep = 1;
	//info->fix.ywrapstep = 0;
	//info->fix.accel = FB_ACCEL_NONE;//don't care about it	
	//info->fix.smem_start = (unsigned long)par->screen.pvAdd;
	//info->fix.smem_len = PAGE_ALIGN(par->screen.size);
	info->fix.line_length = linelen;//seems fix.line_length is not fixed 
	//info->fix.mmio_start = 0;
	//info->fix.mmio_len = 0;

	switch(var->bits_per_pixel)
	{
	case 8:
		info->fix.visual = FB_VISUAL_PSEUDOCOLOR;
		break;
	case 16:
	case 32:
	default:
		info->fix.visual = FB_VISUAL_TRUECOLOR;
		break;	
	}
#ifdef _USE_RANDR_
   info->var.activate = FB_ACTIVATE_FORCE;//FB_ACTIVATE_NOW;
#else	
   info->var.activate = FB_ACTIVATE_NOW;
#endif   
   info->var.accel_flags = FB_ACCELF_TEXT;
   info->var.vmode = FB_VMODE_NONINTERLACED;   

   ret = sm750fb_ops_set_par(info);
exit:
   kfree(modestr);
   LEAVE(ret);
}

#ifdef BURST_CONSOLE
void ufb_update_console_damage_area(struct boxRect * pbox,struct sm750fb_par *par)
{
	spin_lock(&par->slock_dj);
	if(pbox->x1 < par->damageArea.x1)
		par->damageArea.x1 = pbox->x1;
	
	if(pbox->y1 < par->damageArea.y1)
		par->damageArea.y1 = pbox->y1;

	
	if(pbox->x2 > par->damageArea.x2)
		par->damageArea.x2 = pbox->x2;
	
	if(pbox->y2 > par->damageArea.y2)
		par->damageArea.y2 = pbox->y2;
	spin_unlock(&par->slock_dj);
}
#endif


static int sm750fb_ops_mmap(struct fb_info * info,struct vm_area_struct * vma)
{
	unsigned long start = vma->vm_start;
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long page,pos;
	int ret;

	ENTER();
	ret = 0;

	if(offset + size > info->fix.smem_len)
		LEAVE(-EINVAL);

	pos = (unsigned long)info->fix.smem_start+offset;

	while(size > 0){
		page = vmalloc_to_pfn((void*)pos);
		if(remap_pfn_range(vma,start,page,PAGE_SIZE,PAGE_SHARED))
			LEAVE(-EAGAIN);

		start += PAGE_SIZE;
		pos += PAGE_SIZE;
		if(size > PAGE_SIZE)
			size -= PAGE_SIZE;
		else
			size = 0;
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)	
	vma->vm_flags |= VM_RESERVED; /* avoid to swap out this VMA */
#else
	vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;
#endif

	LEAVE(0);
}
int ufb_strt_convert32to16(struct fb_info * info,void * arg)
{
    ENTER();
    struct sm750fb_par * par = info->par;
    
    if(*(int *)arg)
    {
        par->convert32to16 = TRUE;
        dbg_msg("kernel driver will convert 32 bpp to 16 bpp\n");
    }
    else
    {
        par->convert32to16 = FALSE;
        dbg_msg("kernel driver dont convert 32 bpp to 16 bpp\n");
    }
    LEAVE(0);
}
int ufb_get_edid(struct fb_info * info,void * arg)
{
    struct ufb_data * ufb;
    edid_data_p edid;
    int ret;
	ENTER();
    ufb = INFO_2_UFB(info);
    edid = (edid_data_p)arg;
    dbg_msg("edid displayPath:%lx\n",edid->displayPath);
    dbg_msg("edid pEDIDBuffer:%lx\n",edid->pEDIDBuffer);
    dbg_msg("edid bufferSize:%lx\n",edid->bufferSize);
    dbg_msg("edid edidExtNo:%lx\n",edid->edidExtNo);    
    ret = (int)edidReadMonitor(ufb, edid->displayPath, edid->pEDIDBuffer, edid->bufferSize, edid->edidExtNo);
    LEAVE(ret);
}

#ifdef _USE_RANDR_
int ufb_get_mirror_flag(struct fb_info * info,void * arg)
{
    ENTER();
    int flag_of_mirror;
    flag_of_mirror = *(int *)arg;
    struct sm750fb_par * par = info->par;
    par->flagofmirror = flag_of_mirror;
    printk("get flag_of_mirror=[%d]\n", flag_of_mirror);
    LEAVE(0);
}


int ufb_get_line_length(struct fb_info * info,void * arg)
{
    ENTER();
    int xres_virtual, length ;

    xres_virtual = *(int *)arg;
     info->var.xres_virtual = xres_virtual;
    	length = (info->var.xres_virtual * info->var.bits_per_pixel) >> 3;	
	length = PADDING(16,length);
    info->fix.line_length = length;
printk("get xres_virtual=[%d], line length =[%d]\n", xres_virtual,length);
    LEAVE(0);
}
#endif    

int sm750fb_ops_ioctl(struct fb_info * info,unsigned int cmd,unsigned long arg)
{
	int ret;
	struct ufb_data * ufb;
//	ENTER();
	ret = 0;
	ufb = INFO_2_UFB(info);
	mutex_lock(&ufb->io_mutex);
	if(ufb->status == UFB_STAT_PLUGOUT){
		ret = -ENODEV;
		mutex_unlock(&ufb->io_mutex);
		goto exit;
	}
	mutex_unlock(&ufb->io_mutex);
	switch (cmd){
		case 0x5599:
			break;		
		case UFB_CMD_PEEK:
			mutex_lock(&ufb->io_mutex);
			ret = ufb_io_peek(ufb,(void __user *)arg);
			mutex_unlock(&ufb->io_mutex);
			break;
		case UFB_CMD_VM_PEEK:
			mutex_lock(&ufb->io_mutex);
			ret = ufb_vm_peek(ufb,(void __user *)arg);
			mutex_unlock(&ufb->io_mutex);
			break;
		case UFB_CMD_POKE:
			mutex_lock(&ufb->io_mutex);
			ret = ufb_io_poke(ufb,(void __user *)arg);
			mutex_unlock(&ufb->io_mutex);
			break;
		case UFB_CMD_UPDATE_BLOCK:
			mutex_lock(&ufb->io_mutex);
			ret = ufb_io_update(info,(void __user *)arg);
			mutex_unlock(&ufb->io_mutex);
			break;
		case UFB_CMD_UPDATE:
			ret = ufb_io_update_async(info,(void __user *)arg);
			break;

		case UFB_CMD_UPDATESUB_BLOCK:/*always block mode*/
			mutex_lock(&ufb->io_mutex);
			ret = ufb_io_updatesub(info,(void __user*)arg);
			mutex_unlock(&ufb->io_mutex);
			break;
      
		case UFB_CMD_CONVERT_32_16:/*always block mode*/
			mutex_lock(&ufb->io_mutex);
			ret = ufb_strt_convert32to16(info,(void __user*)arg);
			mutex_unlock(&ufb->io_mutex);
			break;
      	case UFB_CMD_EDID_GET:/*always block mode*/
			mutex_lock(&ufb->io_mutex);
			ret = ufb_get_edid(info,(void __user*)arg);
			mutex_unlock(&ufb->io_mutex);
			break;
#ifdef _USE_RANDR_			
         case UFB_CMD_LINELENGTH_GET :
               mutex_lock(&ufb->io_mutex);
			ret = ufb_get_line_length(info,(void __user*)arg);
			mutex_unlock(&ufb->io_mutex);
			break;           
         case UFB_CMD_2ND_SCREEN :
              mutex_lock(&ufb->io_mutex);
			ret = ufb_set_2nd_screen(info,(void __user*)arg);
			mutex_unlock(&ufb->io_mutex);
			break;
         case UFB_CMD_MIRROR_FLAG :
              mutex_lock(&ufb->io_mutex);
			ret = ufb_get_mirror_flag(info,(void __user*)arg);
			mutex_unlock(&ufb->io_mutex);
			break;
#endif			
/*
		case UFB_CMD_TRANSF:
			ret = ufb_io_transfer(info,(void __user *)arg);
			break;
		case UFB_CMD_VIDEO:
			ret = ufb_io_video(info,(void __user *) arg);
			break;
*/
		case UFB_CMD_MODE:
			ret = ufb_io_setmode(info,(void __user*)arg);
			break;
		 case UFB_CMD_KERNEL_TO_VM:
                        mutex_lock(&ufb->io_mutex);
                        ret = ufb_io_kernel_to_vm(info,(void __user *)arg);
                        mutex_unlock(&ufb->io_mutex);
                        break;
		default:
			error("unsupported cmd:%x",cmd);
			break;
	}
exit:
	if(ret<0 ){
		error("return %d",ret);
//		dump_stack();
	}
//	LEAVE(ret);
	return ret;
}

static int sm750fb_ops_setcolreg(unsigned regno,unsigned red,
				unsigned green,unsigned blue,
				unsigned transp,struct fb_info* info)
{
	struct sm750fb_par * par = info->par;
	struct fb_var_screeninfo * var = &info->var;
	uint32_t val;
	struct ufb_data * ufb;
	int ret;

//	ENTER();
	ret = 0;
	ufb = INFO_2_UFB(info);
	mutex_lock(&ufb->io_mutex);
	if(ufb->status == UFB_STAT_PLUGOUT){
		ret = -ENODEV;
		goto exit;
	}
	if(regno > 255)
	{
		error("regno = %d",regno);
		ret = -EINVAL;
		goto exit;
	}

	if(info->var.grayscale)
		red = green = blue = (red * 77 + green * 151 + blue * 28) >> 8;

	if(var->bits_per_pixel == 8 && info->fix.visual == FB_VISUAL_PSEUDOCOLOR)
	{
		red >>= 8;
		green >>= 8;
		blue >>= 8;
		hw_set_colreg(par,regno,red,green,blue);
		goto exit;
	}
 	
	//if(info->fix.visual == FB_VISUAL_TRUECOLOR && regno < 16 )
	if(info->fix.visual == FB_VISUAL_TRUECOLOR && regno < 256 )
	{
		if(var->bits_per_pixel == 16 || var->bits_per_pixel == 32)
		{
			val = chan_to_field(red,&var->red);
			val |= chan_to_field(green,&var->green);
			val |= chan_to_field(blue,&var->blue);			
#ifdef __BIG_ENDIAN
			if(var->bits_per_pixel == 16)
			{
				par->pseudo_palette[regno] =((red & 0xf800) >> 8) | 
							 			((green & 0xe000) >> 13) |
							 			((green & 0x1c00) << 3) | 
				 						((blue & 0xf800) >> 3);
			}
			else
			{
				par->pseudo_palette[regno] = ((val & 0x00ff)<<24)|
										((val & 0xff00)<<8)|
										((val & 0xff0000)>>8);
			
			}
#else
			par->pseudo_palette[regno] = val;
#endif

			goto exit;
		}				
	}

exit:
	mutex_unlock(&ufb->io_mutex);
	return ret;
	//LEAVE(ret);
}

static int sm750fb_ops_blank(int blank_mode,struct fb_info* info)
{	
/**
 *      xxxfb_blank - NOT a required function. Blanks the display.
 *      @blank_mode: the blank mode we want. 
 *      @info: frame buffer structure that represents a single frame buffer
 *
 *      Blank the screen if blank_mode != FB_BLANK_UNBLANK, else unblank.
 *      Return 0 if blanking succeeded, != 0 if un-/blanking failed due to
 *      e.g. a video mode which doesn't support it.
 *
 *      Implements VESA suspend and powerdown modes on hardware that supports
 *      disabling hsync/vsync:
 *
 *      FB_BLANK_NORMAL = display is blanked, syncs are on.
 *      FB_BLANK_HSYNC_SUSPEND = hsync off
 *      FB_BLANK_VSYNC_SUSPEND = vsync off
 *      FB_BLANK_POWERDOWN =  hsync and vsync off
 *
 *      If implementing this function, at least support FB_BLANK_UNBLANK.
 *      Return !0 for any modes that are unimplemented.
 *
 */
	struct ufb_data * ufb;
	int ret;
	//ENTER();
	ret = 0;
	ufb = INFO_2_UFB(info);
	mutex_lock(&ufb->io_mutex);
	if(ufb->status == UFB_STAT_PLUGOUT){
		ret =  -ENODEV;
		goto exit;
	}
	ret = hw_set_blank(info->par,blank_mode);	
	mutex_unlock(&ufb->io_mutex);
exit:	
	//LEAVE(ret);
	return ret;
}




static void sm750fb_ops_fillrect(struct fb_info* info,const struct fb_fillrect* region)
{
//	uint32_t color,rop;
	uint32_t base,pitch,bpp;
	struct sm750fb_par * par = info->par;
	struct ufb_data * ufb;
	struct boxRect box;
    ENTER();
	ufb = INFO_2_UFB(info);	
	base = par->screen.offset;
	pitch = info->fix.line_length;
	bpp = info->var.bits_per_pixel;

	if(info->state != FBINFO_STATE_RUNNING)
	{
		error("info->state !=FBINFO_STATE_RUNNING");
		return;
	}

	if(ufb->status == UFB_STAT_PLUGOUT){
		return;
	}

	cfb_fillrect(info,region);
#ifdef BURST_CONSOLE
	box.x1 = (uint16_t) region->dx;
	box.y1 = (uint16_t) region->dy;
	box.x2 = (uint16_t) region->width + box.x1 -1;
	box.y2 = (uint16_t) region->height + box.y1 -1;

	ufb_update_console_damage_area(&box,par);
#else
	mutex_lock(&ufb->io_mutex);
	hw_update_damage(info,
				region->dx,region->dy,
				region->width,region->height);
	mutex_unlock(&ufb->io_mutex);
#endif
    LEAVE();
	//return;
}

static void sm750fb_ops_copyarea(struct fb_info* info,const struct fb_copyarea* region)
{
	struct sm750fb_par * par = info->par;
	uint32_t base,pitch,bpp;
	struct ufb_data * ufb;
	struct boxRect box;
	ufb = INFO_2_UFB(info);

	if(ufb->status == UFB_STAT_PLUGOUT)
		return;
	base = par->screen.offset;
	pitch = info->fix.line_length;
	bpp = info->var.bits_per_pixel;

	cfb_copyarea(info,region);
#ifdef BURST_CONSOLE
	box.x1 = (uint16_t) region->dx;
	box.y1 = (uint16_t) region->dy;
	box.x2 = (uint16_t) region->width + box.x1 - 1;
	box.y2 = (uint16_t) region->height + box.y1 - 1;
	ufb_update_console_damage_area(&box,par);
#else
	mutex_lock(&ufb->io_mutex);
	hw_update_damage(info,
				region->dx,region->dy,
				region->width,region->height);
	mutex_unlock(&ufb->io_mutex);
#endif
	return;
}


static void sm750fb_ops_imageblit(struct fb_info* info,const struct fb_image * image)
{
	//int err;
	struct ufb_data * ufb;
	struct sm750fb_par * par = info->par;
	static struct fb_event event;
	static struct fb_con2fbmap map;
	struct boxRect box;
    ENTER();
	event.info = info;
	event.data = &map;
	map.console = 2;
	map.framebuffer = 0;

	ufb = INFO_2_UFB(info);
	if(ufb->status == UFB_STAT_PLUGOUT)
		return;

	cfb_imageblit(info,image);
#ifdef BURST_CONSOLE
	box.x1 = (uint16_t) image->dx;
	box.y1 = (uint16_t) image->dy;
	box.x2 = (uint16_t) image->width + box.x1 - 1;
	box.y2 = (uint16_t) image->height + box.y1 - 1;	
	ufb_update_console_damage_area(&box,par);
#else
	mutex_lock(&ufb->io_mutex);
	err = hw_update_damage(info,
			image->dx,image->dy,
			image->width,image->height);
	mutex_unlock(&ufb->io_mutex);
#endif
	LEAVE();
}



static int sm750fb_ops_cursor(struct fb_info* info,struct fb_cursor * fbcur)
{
	struct sm750fb_par * par = info->par;
	struct ufb_data * ufb;
	uint32_t dx,dy;
	uint32_t bg,fg;	
	const uint8_t *pcol,*pmsk;
	int ret = 0;

	ufb = par->priv;
	mutex_lock(&ufb->io_mutex);
	if(ufb->status == UFB_STAT_PLUGOUT)
		goto exit;

	if(fbcur->image.width > MAX_CURS_W || fbcur->image.height > MAX_CURS_H){
		ret = -ENXIO;
		error("fb_cursor image width or height can not > %d",MAX_CURS_W);
		goto exit;
	}
	if(fbcur->image.depth > 1){
		ret = -ENXIO;
		error("fb_cursor image depth can not > 1");
		goto exit;
	}


	hw_cursor_enable(par,0);

	if(fbcur->set & FB_CUR_SETSIZE){
//		dbg_msg("set cursor size to [%d,%d].\n",fbcur->image.width,fbcur->image.height);
		
		par->cursor.size = PADDING(8,(fbcur->image.width * fbcur->image.height * 2)) >> 3;		
		hw_cursor_setsize(fbcur->image.width,fbcur->image.height);		
	}
	if(fbcur->set & FB_CUR_SETPOS){
		
//		dbg_msg("set cursor position to [%d,%d].\n",	fbcur->image.dx,fbcur->image.dy);
		
		dx = fbcur->image.dx - info->var.xoffset;
		dy = fbcur->image.dy - info->var.yoffset;

		ret = hw_cursor_setpos(par,dx,dy);
		if(ret){
			dbg_msg("goto exit after setpos\n");
			goto exit;
		}
	}
	
	if(fbcur->set & FB_CUR_SETCMAP){

#if 0		
		/* 	below code copied from intel fb driver
			but cursor will be unvisiable in 8 bpp mode 
		*/
		dbg_msg("set cursor cmap.\n");
		if(info->var.bits_per_pixel != 8){
			fg = par->pseudo_palette[fbcur->image.fg_color];
			bg = par->pseudo_palette[fbcur->image.bg_color];
		}else{
			fg = fbcur->image.fg_color;
			bg = fbcur->image.bg_color;
		}		
#else
 		/*sm750 cursor colour is 16 bpp*/
		bg = ((info->cmap.red[fbcur->image.bg_color] & 0xf800)) |
			 ((info->cmap.green[fbcur->image.bg_color] & 0xfc00) >> 5) |
			 ((info->cmap.blue[fbcur->image.bg_color] & 0xf800) >> 11) ;

		fg = ((info->cmap.red[fbcur->image.fg_color] & 0xf800)) |
			((info->cmap.green[fbcur->image.fg_color] & 0xfc00) >> 5) |
			((info->cmap.blue[fbcur->image.fg_color] & 0xf800) >> 11);
#endif		
		ret = hw_cursor_setcolor(par,bg,fg);
		if(ret)	
			goto exit;	
	}

	if(fbcur->set & (FB_CUR_SETSHAPE | FB_CUR_SETIMAGE)){
	/* Cursor is mono-color in Linux framebuffer driver 
	 * Which means 1 bit per pixel of cursor color
	 * sm750 hardware cursor layer size : 64x64x2bit
	 */
	 //	dbg_msg("set cursor data\n");
		pcol = fbcur->image.data;
		pmsk = fbcur->mask;
		hw_cursor_clear(par);	//reset cursor data to 0
		ret = hw_cursor_setdata(par,fbcur,pcol,pmsk);
		if(ret)
			goto exit;
	}
	if(fbcur->enable)		
		hw_cursor_enable(par,1);

exit:
	mutex_unlock(&ufb->io_mutex);
	return ret;
}

static int do_ufb_check_mhpg(struct ufb_data * ufb){
	char dviLast,crtLast;
	char dviNow,crtNow;
	struct fb_info * fbInfo0,*fbInfo1;
	char *envp[] = {
			g_env_verdor,
			g_env_device,
			g_env_head_index,
			NULL
		       };
	static char * gs_monitor_status[4] = {
		"00",
		"01",
		"10",
		"11",
	};
	ENTER();
	fbInfo0 = ufb->fbinfo[0];
	fbInfo1 = ufb->fbinfo[1];

	dviLast = ufb->flag_monitor & 1;
	crtLast = ufb->flag_monitor >> 1;
	ufb->flag_monitor = 0;

	if(hw_output0_connected(ufb)){		
		dviNow = 1;
	}
	else{
		dviNow = 0;
	}
	
	if(hw_output1_connected(ufb)){
		inf_msg("crt on\n");
		crtNow = 1;
	}
	else{
		inf_msg("crt off\n");
		crtNow = 0;
	}
	ufb->flag_monitor = (crtNow<<1)|(dviNow);
	
	if(dviLast != dviNow){	
		dbg_msg("dvi hotplug=>%s\n",dviNow?"plugIn":"plugOut");
		//sprintf(g_env_head_index, "HEAD0=%s",dviNow?"on":"off");
		//kobject_uevent_env(&fbInfo0->dev->kobj, KOBJ_CHANGE, envp);
	}	
	if(crtLast != crtNow){	
		dbg_msg("crt hotplug=>%s\n",crtNow?"plugIn":"plugOut");
		//sprintf(g_env_head_index, "HEAD1=%s",crtNow?"on":"off");
		//kobject_uevent_env(&fbInfo1->dev->kobj, KOBJ_CHANGE, envp);
	}	

	if(crtLast != crtNow || dviLast != dviNow){
		sprintf(g_env_head_index, "MONITOR=%s",gs_monitor_status[ufb->flag_monitor]);
		if(fbInfo0 != NULL)
			kobject_uevent_env(&fbInfo0->dev->kobj, KOBJ_CHANGE, envp);
	}

	LEAVE(ufb->flag_monitor);
}


static void ufb_check_mhpg(struct work_struct * work)
{
	struct ufb_data * ufb;
	ENTER();
	ufb = container_of(work,struct ufb_data,w_monitor.work);
	do_ufb_check_mhpg(ufb);	
	queue_delayed_work(ufb->wq_monitor,&ufb->w_monitor,UFB_MONITOR_CHECK_INTERVAL);
	LEAVE();
}

static void ufb_free_ufb(struct kref * kref){
	struct ufb_data * ufb;
	ENTER();

	ufb = container_of(kref,struct ufb_data,kref);
	if(ufb->wq_monitor){
		cancel_delayed_work(&ufb->w_monitor);
		flush_workqueue(ufb->wq_monitor);
		destroy_workqueue(ufb->wq_monitor);
	}

	usb_free_urb(ufb->urbs[0]);
	usb_free_urb(ufb->urbs[1]);
	rvfree(ufb->buf_vfb,VIDEOMEM_SIZE + PRIVATE_OFFSCREEN_SIZE);
	free_pages((unsigned long)ufb->buf_dma,BUF_DMA_PAGE);
	free_pages((unsigned long)ufb->buf_async,BUF_ASYNC_PAGE);
	kfree(ufb);
	LEAVE();
}


#ifdef BURST_CONSOLE
static void ufb_draw_framebuffer_work(struct work_struct * work){
	struct fb_info * info;
	struct sm750fb_par * par;
	struct ufb_data * ufb;
	unsigned long j1,j2,diff;
	uint16_t x1,y1,x2,y2;
	int err = 0;

	ENTER();

	par = container_of(work,struct sm750fb_par,draw_framebuffer_work.work);
	ufb = par->priv;
	info = par->info;
	while(1){
		if(ufb->status == UFB_STAT_PLUGOUT){
			printk("in workqueue:device plug out!\n");
			LEAVE();
		}

		j1 = jiffies;
		spin_lock(&par->slock_dj);
		x1 = par->damageArea.x1;
		y1 = par->damageArea.y1;
		x2 = par->damageArea.x2;
		y2 = par->damageArea.y2;
		if(x1 != 9999){
			par->damageArea.x1 = par->damageArea.y1 = 9999;
			par->damageArea.x2 = par->damageArea.y2 = 0;
		}
		spin_unlock(&par->slock_dj);

		/* do real drawing */
		if(x1 != 9999)
		{
			mutex_lock(&ufb->io_mutex);
			err = hw_update_damage(info,x1,y1,
					x2 - x1 + 1,
					y2 - y1 + 1);
			mutex_unlock(&ufb->io_mutex);
			printk(".");
		}

		if(err < 0){
			error("err=%d\n",err);
			LEAVE();
		}

		j2 = jiffies;
		diff = j2 - j1;
		if(par->msec_interval > diff)
			diff = par->msec_interval - diff;
		else
			diff = 0;

		current->state = TASK_INTERRUPTIBLE;
		schedule_timeout(diff);
	}
	LEAVE();
}
#endif

static void ufb_free_framebuffer_work(struct work_struct * work){
	struct fb_info * info;
	struct sm750fb_par * par;
	struct ufb_data * ufb;

	ENTER();
	printk("in free fb\n");
	par = container_of(work,struct sm750fb_par,free_framebuffer_work.work);
	ufb = par->priv;

#ifdef BURST_CONSOLE
	if(par->wq_drawfb){
		cancel_delayed_work(&par->draw_framebuffer_work);
		flush_workqueue(par->wq_drawfb);
		destroy_workqueue(par->wq_drawfb);
	}
#endif

	info = par->info;
	if(info!=NULL){
		unregister_framebuffer(info);
		framebuffer_release(info);
		smHeads[par->headX] = 0;
#if 0
		int i=0;
		while(i<8){
			printk("smHeads[%d]=%d\n",i,smHeads[i]);
			i++;
		}
#endif
		kref_put(&ufb->kref,ufb_free_ufb);
	}
	LEAVE();
}


static int ufb_ops_open(struct fb_info * info,int user){
	struct ufb_data * ufb;
	struct sm750fb_par * par;
	int ret;

	ENTER();	

	if((user == 0) && (!fbconsole))
		LEAVE(-ENODEV);

	par = info->par;
	ret = 0;
	ufb = INFO_2_UFB(info);
	mutex_lock(&ufb->io_mutex);
	if(ufb->status == UFB_STAT_PLUGOUT){
		ret =-ENODEV;
		goto exit;
	}
	atomic_inc(&par->fbcount);
	atomic_inc(&ufb->fbcount);
	//printk("openfb:%d,\n",info->node);
//	kref_get(&ufb->kref);
exit:
	mutex_unlock(&ufb->io_mutex);
	LEAVE(ret);
}

static int ufb_ops_release(struct fb_info * info,int user){
	struct ufb_data * ufb;
	struct sm750fb_par * par;
	int ret;

	ENTER();
	//printk("closefb:%d\n",info->node);
	par = info->par;
	ret = 0;
	ufb = INFO_2_UFB(info);
	mutex_lock(&ufb->io_mutex);
	
	if(atomic_read(&par->fbcount) > 0){
		atomic_dec(&par->fbcount);
		atomic_dec(&ufb->fbcount);
	}
	
	if(ufb->status == UFB_STAT_PLUGOUT){
		/* if status is plugged out,do real disconnect job*/		
		if(atomic_read(&par->fbcount) == 0)	{
			schedule_delayed_work(&par->free_framebuffer_work,0);
		}
	}
	mutex_unlock(&ufb->io_mutex);
	LEAVE(ret);
}

#if 0
static int sm750fb_ops_pan_display(struct fb_var_screeninfo* var,struct fb_info* info)
{
	return  hw_pan_display(info->par,var);		
}

static int sm750fb_ops_sync(struct fb_info * info){
	/* get called in cfb_xxxx function,so do not use mutexlock or forever sleep the process */
	int ret;
	ret = hw_2d_wait_idle(info->par);
	return ret;
}

#endif

static struct fb_ops sm750fb_ops = {
	.owner = THIS_MODULE,
	.fb_open = ufb_ops_open,
	.fb_release = ufb_ops_release,
	.fb_check_var = sm750fb_ops_check_var,
	.fb_set_par = sm750fb_ops_set_par,
	.fb_setcolreg = sm750fb_ops_setcolreg,
	.fb_blank = sm750fb_ops_blank,
	.fb_fillrect = sm750fb_ops_fillrect,
	.fb_copyarea = sm750fb_ops_copyarea,
	.fb_imageblit = sm750fb_ops_imageblit,
	.fb_cursor = sm750fb_ops_cursor,
	.fb_mmap = sm750fb_ops_mmap,
	.fb_ioctl = sm750fb_ops_ioctl,
//	.fb_pan_display = sm750fb_ops_pan_display,
//	.fb_sync = sm750fb_ops_sync,
};

static const struct usb_device_id support_table[] = {
//		{ USB_DEVICE(USB_VENDOR_ID_NETCHIP,PLX_3380)},
		{ USB_DEVICE(0x090c,0x0750)},
		{}
};

MODULE_DEVICE_TABLE(usb,support_table);


static int sm750fb_set_fbinfo(struct fb_info * info,struct ufb_data * ufb,int index)
{
	int ret,linelen;
	struct sm750fb_par * par;	
	struct fb_var_screeninfo * var ;
	ENTER();
	ret = 0;

	/* each fb_info got a pair with one sm750fb_par 
	 * and every sm750fb_par point to one ufb_data 
	 * */
	par = info->par;
	var = &info->var;
	
	info->fbops = &sm750fb_ops;	
	if(mode_option == NULL){
		mode_option = defaultMode;
	}	
	

	/* setting par member */
	//par->screen.size = (dual==1)?ufb->memsize/2:ufb->memsize;
	if(ufb->outputs < 1)
		ufb->outputs = 1;
	par->screen.size = ufb->memsize/ufb->outputs;
	par->screen.offset = 0;
	par->screen.pvAdd = ufb->buf_vfb;

	if(index)
	{
		par->screen.offset += par->screen.size;
		par->screen.pvAdd += par->screen.size;
	}

#if 1
	par->priv_vm_offset = ufb->priv_vm_offset;
#endif
	
	par->cursor.offset = par->screen.offset + par->screen.size - CURSOR_SPACE_SIZE;
	par->cursor.size = CURSOR_SPACE_SIZE;
	par->cursor.pvAdd = ufb->buf_vfb + par->cursor.offset;
	par->controller = (index==0)?CTRL_PRI:CTRL_SEC;

	/* setting info memver */
	info->pseudo_palette = par->pseudo_palette;
	info->screen_base = par->screen.pvAdd;
	info->screen_size = par->screen.size; //fixed vary

	info->flags = FBINFO_DEFAULT|
		FBINFO_HWACCEL_IMAGEBLIT|
		FBINFO_HWACCEL_FILLRECT|
		FBINFO_HWACCEL_COPYAREA|			
		0;
	

	if(index)
		strlcpy(info->fix.id,"usbfb",sizeof("usbfb"));
	else
		strlcpy(info->fix.id,"usbfb",sizeof("usbfb"));

	ret = fb_find_mode(var,info,mode_option,
		sm750modbExt,ARRAY_SIZE(sm750modbExt),NULL,16);
	
	if(ret!=1){
		dbg_msg("specify mode not in sm750modbExt.\n");	
		ret = fb_find_mode(var,info,mode_option,NULL,0,NULL,16);
	}	

	if(ret==0 || ret == 4){
		error("failed to get specify mode.");
		ret = -EINVAL;
		goto exit;		
	}

	
	dbg_msg("member of info->var is :\n\
		xres=%d\n\
		yres=%d\n\
		xres_virtual=%d\n\
		yres_virtual=%d\n\
		xoffset=%d\n\
		yoffset=%d\n\
		bits_per_pixel=%d\n \
		...\n",var->xres,var->yres,var->xres_virtual,var->yres_virtual,
			var->xoffset,var->yoffset,var->bits_per_pixel);

	linelen = (var->xres_virtual * var->bits_per_pixel)/8;
	linelen = PADDING(16,linelen);


	info->fix.type = FB_TYPE_PACKED_PIXELS;
	info->fix.type_aux = 0;
	info->fix.xpanstep = 1;
	info->fix.ypanstep = 1;
	info->fix.ywrapstep = 0;
	info->fix.accel = FB_ACCEL_NONE;//don't care about it	
	info->fix.smem_start = (unsigned long)par->screen.pvAdd;
	info->fix.smem_len = PAGE_ALIGN(par->screen.size);
	info->fix.line_length = linelen;//seems fix.line_length is not fixed 
	info->fix.mmio_start = 0;
	info->fix.mmio_len = 0;
	switch(var->bits_per_pixel)
	{
	case 8:
		info->fix.visual = FB_VISUAL_PSEUDOCOLOR;
		break;
	case 16:
	case 32:
	default:
		info->fix.visual = FB_VISUAL_TRUECOLOR;
		break;	
	}
#ifdef _USE_RANDR_
   info->var.activate = FB_ACTIVATE_FORCE;//FB_ACTIVATE_NOW;
#else	
   info->var.activate = FB_ACTIVATE_NOW;
#endif
   info->var.accel_flags = FB_ACCELF_TEXT;
   info->var.vmode = FB_VMODE_NONINTERLACED;   
   
   if(fb_alloc_cmap(&info->cmap,256,0) < 0){
	   error("Could not allcate memory for cmap.\n");
	   ret = -EINVAL;
	   goto exit;
   }
   LEAVE(0);
exit:
	LEAVE(ret);
}

static int sm750fb_probe(struct ufb_data *ufb,int index,int udevIdx)
{
	int ret;
	struct fb_info * fbInfo;
	struct sm750fb_par * par;	
	
	char *envp[] = {
			g_env_verdor,
			g_env_device,
			g_env_head_index,
			NULL
		       };

	ENTER();

	ret = 0;
	fbInfo = framebuffer_alloc(sizeof(struct sm750fb_par),ufb->gdev);
	if(!fbInfo){
		error("could not allocate memory for fbInfo");
		ret = -ENOMEM;
		goto exit;
	}
	
	par = fbInfo->par;
	par->info = fbInfo;
	par->priv = ufb;
	par->idx = index;
	par->csc_playing = 0;
#ifdef _USE_RANDR_
    par->flagofmirror = 0;
#else
    par->flagofmirror = 2;//add by ilena
#endif
	atomic_set(&par->fbcount,0);

	INIT_DELAYED_WORK(&par->free_framebuffer_work,ufb_free_framebuffer_work);

	ret = sm750fb_set_fbinfo(fbInfo,ufb,index);
	if(ret){
		error("failed to setup fb_info");
		goto err1;
	}

	ret = register_framebuffer(fbInfo);
	if(ret < 0 ){
		error("failed to register framebuffer");
		goto err1;
	}
	/* for userspace udev */
	mutex_lock(&g_io_mutex);	
	int i=0;	
	while(i<8){
		if(smHeads[i] == 0){
			udevIdx = i;
			smHeads[i] = 1;
			break;
		}else{
			i++;
		}
	}
	mutex_unlock(&g_io_mutex);
#if 0
	i=0;
	while(i<8){
		printk("smHeads[%d]=%d\n",i,smHeads[i]);
		i++;
	}
#endif

	par->headX = udevIdx;
	printk("HEAD=%d\n",par->headX);
	sprintf(g_env_head_index, "HEAD=head%d", par->headX);
	kobject_uevent_env(&fbInfo->dev->kobj, KOBJ_ADD, envp);
	ufb->fbinfo[index] = fbInfo;
	inf_msg("register framebuffer done!");
	kref_get(&ufb->kref);
#define _DELAY_ 4
	par->msec_interval = msecs_to_jiffies(_DELAY_);
	inf_msg("%d msec equals %d jiffies\n",_DELAY_,par->msec_interval);
#undef _DELAY_
	par->damageArea.x1 = 9999;
	par->damageArea.y1 = 9999;
	par->damageArea.x2 = 0;
	par->damageArea.y2 = 0;

	spin_lock_init(&par->slock_dj);

	/* draw console work */
	INIT_DELAYED_WORK(&par->draw_framebuffer_work,ufb_draw_framebuffer_work);
	par->wq_drawfb = create_singlethread_workqueue("drawfb_dj");
	queue_delayed_work(par->wq_drawfb,&par->draw_framebuffer_work,par->msec_interval);
	LEAVE(0);	
	
err1:
	framebuffer_release(fbInfo);
exit:
	LEAVE (ret);
	
}

/*
 * according to lsusb -v
 * netchip 3380 got 1 configuration(bConfigurationValue = 1)
 * 				got 1 interface (bInterfaceNumber = 0)
 * 				and got 10 endpoints 
 * 	below are the endpoints table,all wMaxPacketSize is 512bytes (
 * 	except that ep9 8bytes and ep10 16byets)
 *
 *  Num,Addr,Attr,dir,use
 * 	1,0x81,bulk,in
 * 	2,0x02,bulk,out
 * 	3,0x83,bulk,in
 * 	4,0x04,bulk,out
 * 	5,0x0d,bulk,out,CSR
 * 	6,0x8d,bulk,in,CSR
 * 	7,0x0e,bulk,out,PCI
 * 	8,0x8e,bulk,in,PCI
 * 	9,0x8f,interrupt,in,STAT
 * 	10,0x8c,interrupt,in,RC
 * 
 * */
static int usbfb_probe(struct usb_interface * interface,const struct usb_device_id * id)
{
#if 0
 	struct usb_host_interface * iface_desc;
	struct usb_endpoint_descriptor * endpoint;
#endif
	struct usb_device * udev;
	struct ufb_data * ufb;
	uint32_t state;
	int act_size;
	int ret,i;
  edid_data fbedid;
  char edid[128];

	ENTER();



	ret = 0;
	udev = usb_get_dev(interface_to_usbdev(interface));
	ufb = kzalloc(sizeof(*ufb),GFP_KERNEL);
	if(!ufb){
		error("Out of memory");
		goto error1;
	}

	kref_init(&ufb->kref);
	atomic_set(&ufb->fbcount,0);

	ufb->udev = udev;
	ufb->gdev = &ufb->udev->dev;
	ufb->interface = interface;

	/* 16m */
	ufb->buf_vfb = rvmalloc(VIDEOMEM_SIZE + PRIVATE_OFFSCREEN_SIZE);
	/* 1024k*/
	ufb->buf_async = (char*)__get_free_pages(GFP_KERNEL,BUF_ASYNC_PAGE);
	/* 256k*/
	ufb->buf_dma = (char*)__get_free_pages(GFP_KERNEL,BUF_DMA_PAGE);

	if(!ufb->buf_vfb || !ufb->buf_async || !ufb->buf_dma){
		error("cannnot alloc memory");
		ret = -ENOMEM;
		goto error2;
	}

	memset(ufb->buf_async,0,2<<BUF_ASYNC_PAGE);
	ufb->urbs[0] = usb_alloc_urb(0,GFP_KERNEL);
	ufb->urbs[1] = usb_alloc_urb(0,GFP_KERNEL);

	usb_set_intfdata(interface,ufb);

	/* init members*/	
	spin_lock_init(&ufb->sl_Xdraw);	

	init_completion(&ufb->uCsr_man.completion);
	spin_lock_init(&ufb->uCsr_man.err_lock);
	mutex_init(&ufb->uCsr_man.io_mutex);	
	
	init_completion(&ufb->uRpcie_man.completion);
	spin_lock_init(&ufb->uRpcie_man.err_lock);
	mutex_init(&ufb->uRpcie_man.io_mutex);

	init_completion(&ufb->uDma_man.completion);
	spin_lock_init(&ufb->uDma_man.err_lock);
	mutex_init(&ufb->uDma_man.io_mutex);

	mutex_init(&ufb->io_mutex);
	mutex_init(&ufb->dma_mutex);

	NcRpci_StartDevice(ufb);

	CsrUsbc_ReadUlong(ufb,USBSTAT,&state);
	switch(state & 7<<FULL_SPEED){
		case 1<<FULL_SPEED:
		inf_msg("full speed");
		break;
		case 1<<HIGH_SPEED:
		inf_msg("high speed");
		break;
		case 1<<SUPER_SPEED:
		inf_msg("super speed");
		break;
		default:
		inf_msg("unknown speed");
		break;
	}
	
	/*
	     Set up your USB device's initial programming
		 - Programs every aspect of your USB device, including the USB3382-RC
		 and your remote PCI device. It includes PCI configuration cycles
		 and BAR programming. For some devices initialization might include 
		 using USB3382-RC DMACs to transfer large amounts of data quickly
		 and efficiently.
	*/
		
	ret = ConstructDevice(ufb);
	if(ret){
		goto error4;
	}

	//hw_set_ufb(ufb);
	ufb->act_size =	act_size = hw_get_memsize(ufb);
	//ufb->act_size =	act_size = MB(16);
	inf_msg("remote usb-side sm750 video memory size is 0x%x\n",act_size);

	if(hw_init_chip(ufb,&default_state))
		error("init hardware error");

#if 1
	if(outputs < 1)
	{
		do_ufb_check_mhpg(ufb);
		if(ufb->flag_monitor == 3)
			ufb->outputs = 2;
		else
			ufb->outputs = 1;
	}else{
		ufb->outputs = outputs;
	}
	ufb->wq_monitor = NULL;
#else
	/* init a delayed work */
	INIT_DELAYED_WORK(&ufb->w_monitor,ufb_check_mhpg);
	/* init a kernel thread to do the monitor hot-plug check */
	ufb->wq_monitor = create_singlethread_workqueue("monitor");
	/* monitor the monitor hot-plug state */
	queue_delayed_work(ufb->wq_monitor,&ufb->w_monitor,0);
#endif

#if 0/*Test code for EDID*/
    fbedid.displayPath = 0;
    fbedid.bufferSize = 128;
    fbedid.pEDIDBuffer = &edid[0];
    fbedid.edidExtNo = 0;
    
    dbg_msg("Edid, where are you?\n");
    i = edidReadMonitor(ufb, fbedid.displayPath, fbedid.pEDIDBuffer, fbedid.bufferSize, fbedid.edidExtNo);
    if(i==0)
      dbg_msg("Yes! I get it!\n");
    else
      dbg_msg("Damn it! I lost.\n");
    
    fbedid.displayPath = 1;
    
    dbg_msg("Edid, where are you?\n");
     i = edidReadMonitor(ufb, fbedid.displayPath, fbedid.pEDIDBuffer, fbedid.bufferSize, fbedid.edidExtNo);
    if(i==0)
      dbg_msg("Yes! I get it!\n");
    else
      dbg_msg("Damn it! I lost.\n");
#endif
	/* Xdraw work */
//	ufb->wq_Xdraw = create_singlethread_workqueue("Xdraw");

	if(act_size == MB(16)){
		if(ufb->outputs==2)
			ufb->memsize = MB(10);
		else
			ufb->memsize = MB(8);
	}else{
		ufb->memsize = VIDEOMEM_SIZE;
	}
	inf_msg("total virtual framebuffer size = 0x%x\n",ufb->memsize);
	ufb->priv_vm_offset = ufb->memsize;;


	/* atleast one fb need be created */
	ret = sm750fb_probe(ufb,0,0);
	if(ret)
		goto error4;	

	if(ufb->outputs == 2){
		ret = sm750fb_probe(ufb,1,0);
		if(ret)
			goto error4;
	}

	hw_fill_2d_cache(ufb);
	LEAVE(0);
error4:
	/* release urb resource */
	usb_free_urb(ufb->urbs[0]);
	usb_free_urb(ufb->urbs[1]);

	rvfree(ufb->buf_vfb,VIDEOMEM_SIZE + PRIVATE_OFFSCREEN_SIZE);
	free_pages((unsigned long)ufb->buf_dma,BUF_DMA_PAGE);
	free_pages((unsigned long)ufb->buf_async,BUF_ASYNC_PAGE);
error2:
	kfree(ufb);
error1:
	LEAVE(ret);
}

static void usbfb_disconnect(struct usb_interface * interface){
//	struct usb_device * udev;
	struct ufb_data * ufb;
	struct fb_info * info;
	struct sm750fb_par * par;

	ENTER();	
//	udev = usb_get_dev(interface_to_usbdev(interface));
	ufb = usb_get_intfdata(interface);	
	usb_set_intfdata(interface,NULL);	

	mutex_lock(&ufb->io_mutex);
	ufb->status = UFB_STAT_PLUGOUT;

	//if(atomic_read(&ufb->fbcount) == 0)
	{
		/* if status is plugged out,deregister all framebuffers if not in use*/
		info = ufb->fbinfo[0];
		if(info!=NULL){
			par = info->par;
			int fbcnt;
			fbcnt = atomic_read(&par->fbcount);
			printk("fbcnt=%d\n",fbcnt);
			if(atomic_read(&par->fbcount) == 0){
#ifdef BURST_CONSOLE
				if(par->wq_drawfb){
					cancel_delayed_work(&par->draw_framebuffer_work);
					flush_workqueue(par->wq_drawfb);
					destroy_workqueue(par->wq_drawfb);
					printk("cancel first fb work\n");
				}
#endif
				printk("unreg fb1\n");
				unregister_framebuffer(info);
				smHeads[par->headX] = 0;
				framebuffer_release(info);
				kref_put(&ufb->kref,ufb_free_ufb);
			}
		}
		info = ufb->fbinfo[1];
		if(info!=NULL){
			par = info->par;
			int fbcnt;
			fbcnt = atomic_read(&par->fbcount);
			printk("fbcnt=%d\n",fbcnt);
			if(atomic_read(&par->fbcount) == 0){
#ifdef BURST_CONSOLE
				if(par->wq_drawfb){
					cancel_delayed_work(&par->draw_framebuffer_work);
					flush_workqueue(par->wq_drawfb);
					destroy_workqueue(par->wq_drawfb);
					printk("cancel second fb work\n");
				}
#endif
				printk("unreg fb2\n");
				unregister_framebuffer(info);
				smHeads[par->headX] = 0;
				framebuffer_release(info);
				kref_put(&ufb->kref,ufb_free_ufb);
			}
		}
	}
	kref_put(&ufb->kref,ufb_free_ufb);
	mutex_unlock(&ufb->io_mutex);
	LEAVE();
}

static int ufb_suspend(struct usb_interface * intf,pm_message_t message){	
	struct ufb_data * ufb;
	struct fb_info * info;
	ENTER();
	ufb = usb_get_intfdata(intf);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,35)
	acquire_console_sem();
#else
	console_lock();
#endif
	if(message.event & PM_EVENT_SLEEP){
		info = ufb->fbinfo[0];
		if(info)
			fb_set_suspend(info,1);

		info = ufb->fbinfo[1];
		if(info)
			fb_set_suspend(info,1);
	}
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,35)
	release_console_sem();
#else
	console_unlock();
#endif
	LEAVE(0);
}

static int ufb_resume(struct usb_interface * intf){
	int ret;
	struct ufb_data * ufb;
	struct fb_info * info;
	ENTER();

	ret = 0;
	ufb = usb_get_intfdata(intf);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,35)
	acquire_console_sem();
#else
	console_lock();
#endif
	NcRpci_StartDevice(ufb);

	ret = ConstructDevice(ufb);
	if(ret){
		error("error = %d,from constructdevice",ret);
		goto exit;
	}

	ret = hw_init_chip(ufb,&default_state);
	if(ret){
		error("error = %d,from hw_init_chip",ret);
		goto exit;
	}
	hw_fill_2d_cache(ufb);

	info = ufb->fbinfo[0];
	if(info){
		fb_set_suspend(info,0);
		sm750fb_ops_set_par(info);
	}

	info = ufb->fbinfo[1];
	if(info){
		fb_set_suspend(info,0);
		sm750fb_ops_set_par(info);
	}

exit:
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,35)
	release_console_sem();
#else
	console_unlock();
#endif
	LEAVE(ret);
}

static int ufb_post_reset(struct usb_interface * intf){
	int ret;
	ENTER();
	ret = 0;
	LEAVE(ret);
}

static int ufb_pre_reset(struct usb_interface * intf){
	int ret;
	ENTER();
	ret = 0;
	LEAVE(ret);
}

static int ufb_reset_resume(struct usb_interface * intf){
	int ret;
	ENTER();
	ret = ufb_resume(intf);
	LEAVE(ret);
}

static struct usb_driver smi_usbfb_driver = {
	.name = DRV_NAME,
	.probe = usbfb_probe,
	.disconnect = usbfb_disconnect,
	.id_table = support_table,
	.suspend = ufb_suspend,
	.resume = ufb_resume,
	.reset_resume = ufb_reset_resume,
	.pre_reset = ufb_pre_reset,
	.post_reset = ufb_post_reset,
	.supports_autosuspend = 0,
};

static int __init smi_usbfb_init(void){
	int ret;
	char * option = NULL;
	ENTER();

	mutex_init(&g_io_mutex);

	inf_msg("SMI ["DRV_NAME"] driver,version: " SMI_KFB_VER_STRING"\n");
	
	if(fb_get_options("ufb",&option))
		LEAVE (-ENODEV);

	ufb_setup(option);
	ret = usb_register(&smi_usbfb_driver);
	if(ret)
		error("usb register failed:%d",ret);
	LEAVE(ret);
}

static void __exit smi_usbfb_exit(void){
	ENTER();
	usb_deregister(&smi_usbfb_driver);
	LEAVE();
}

module_param(mode_option,charp,S_IRUGO);
MODULE_PARM_DESC(mode_option,"Initial video mode");

module_param(outputs,int,S_IRUGO);
MODULE_PARM_DESC(outputs,"");

module_param(fbconsole,bool,S_IWUSR|S_IRUSR|S_IWGRP|S_IRGRP);
MODULE_PARM_DESC(fbconsole,"set to 1 to allow console use this fb");

module_init(smi_usbfb_init);
module_exit(smi_usbfb_exit);

MODULE_AUTHOR("monk.liu@siliconmotion.com");
MODULE_LICENSE("GPL");

