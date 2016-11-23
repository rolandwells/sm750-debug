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
#include <linux/console.h>
#ifdef CONFIG_MTRR
#include <asm/mtrr.h>
#endif

#include"sm750drv.h"
#include"sm750hw.h"

static int noaccel = 0;
static int dumpreg = 0;
#ifdef CONFIG_MTRR
static int nomtrr  = 0;
#endif

//static char * mode_option __devinitdata = NULL;
static char * mode_option  = NULL;
static char defaultMode[] = {DEFAULT_MODE};
static char * exp_res = NULL;
static enum sm750fb_pnltype pnltype = PNLTYPE_24;
static enum sm750fb_output output = OP_SIMUL_CRT1_CRT2;
static struct sm750fb_state default_state = {
	.chip_clk = DEFAULT_CHIP_CLK,
	.mem_clk = DEFAULT_MEMORY_CLK,
	.master_clk = DEFAULT_MASTER_CLK,
	.de_off = 0,
	.output = OP_SIMUL_CRT1_CRT2,
	.xLCD = 0,
	.yLCD = 0,
};

const char * output_text[] = {
	"simul,tft + crt",
	"simul,tft1 + tft2",
	"simul,crt1 + crt2",
};

const char * pnltype_text[] ={
	"24 bit ",
	"18 bit ",
	"36 bit  (double pixel)",
};

#ifdef MODULE
module_param(noaccel,bool,S_IRUGO);
MODULE_PARM_DESC(noaccel,"1 for disable hardware acceleration [default 0]");

module_param(dumpreg,bool,S_IRUGO);
MODULE_PARM_DESC(dumpreg,"1 for enable register dump operation [default 0]");

#ifdef CONFIG_MTRR
module_param(nomtrr,bool,S_IRUGO);
MODULE_PARM_DESC(nomtrr,"1 for disable memory type register range [default 0]");
#endif

module_param(output,int,S_IRUGO);
MODULE_PARM_DESC(output,"0:simul TFT+CRT ; 1:simul TFT+TFT ; 2:simul CRT+CRT(default 2)");

module_param(exp_res,charp,S_IRUGO);
MODULE_PARM_DESC(exp_res,"<xres_exp>x<yres_exp>");

module_param(pnltype,int,S_IRUGO);
MODULE_PARM_DESC(pnltype,"0:24 bit TFT ; 1:18 bit TFT ; 2:36bit TFT (default 0)");

module_param(mode_option,charp,S_IRUGO);
MODULE_PARM_DESC(mode_option,"Initial video mode \"<xres>x<yres>[-<depth>][@<refresh>]\"");

#endif

/* below extern val should be in sm750hw.c */
extern unsigned char __iomem * pVMEM;
extern unsigned char __iomem * pMMIO;

static int __init sm750fb_init(void);
#ifdef MODULE
static void __exit sm750fb_exit(void);
#endif

static int __devinit sm750fb_pci_probe(struct pci_dev *pdev,const struct pci_device_id *ent);
static void __devexit sm750fb_pci_remove(struct pci_dev *pdev);

#ifdef CONFIG_PM
static int sm750fb_suspend(struct pci_dev *dev, pm_message_t mesg);
static int sm750fb_resume(struct pci_dev *dev);
#endif 

static int __init sm750fb_setup(char *);
static int __init sm750_set_drv_status(void);

static struct sm750fb_share* sm750fb_create_share(struct pci_dev*,const struct pci_device_id*);
static int sm750fb_set_fbinfo(struct fb_info*,struct sm750fb_share*,enum sm750fb_controller);

static inline void sm750fb_clr_share(struct sm750fb_share*);
static inline void sm750fb_clr_par(struct sm750fb_par*);

static int sm750fb_ops_check_var(struct fb_var_screeninfo*,struct fb_info*);
static int sm750fb_ops_set_par(struct fb_info*);
static int sm750fb_ops_setcolreg(unsigned,unsigned,unsigned,unsigned,unsigned,struct fb_info*);
static int sm750fb_ops_blank(int,struct fb_info*);
static int sm750fb_ops_pan_display(struct fb_var_screeninfo*,struct fb_info*);
static void sm750fb_ops_fillrect(struct fb_info*,const struct fb_fillrect*);
static void sm750fb_ops_copyarea(struct fb_info*,const struct fb_copyarea*);
static void sm750fb_ops_imageblit(struct fb_info*,const struct fb_image*);
static int sm750fb_ops_cursor(struct fb_info*,struct fb_cursor *);
//static int sm750fb_ops_sync(struct fb_info*);
static inline int h_total(struct fb_var_screeninfo *var);
static inline int v_total(struct fb_var_screeninfo *var);

#ifdef __BIG_ENDIAN
static ssize_t sm750fb_ops_write(struct fb_info *info, const char __user *buf,
			    size_t count, loff_t *ppos);
static ssize_t sm750fb_ops_read(struct fb_info *info, char __user *buf,
			   size_t count, loff_t *ppos);
#endif
static inline uint chan_to_field(ulong,struct fb_bitfield*);

char pcFbname[2][16] = {
"sm750fb-PRI",
"sm750fb-SEC",
};

#if 0
struct fb_videomode {
	const char *name;	/* optional */
	u32 refresh;		/* optional */
	u32 xres;
	u32 yres;
	u32 pixclock;			// 1/ddk.mode:[10]*(10^12)
	u32 left_margin;			// ddk.mode:[0] -[2] - [3]
	u32 right_margin;		// ddk.mode:[2] - [1]
	u32 upper_margin;		// ddk.mode:[5] - [7] - [8]
	u32 lower_margin;		// ddk.mode:[7] - [6]
	u32 hsync_len;			// ddk.mode:[4]
	u32 vsync_len;			// ddk.mode[9]
	u32 sync;				
	u32 vmode;
	u32 flag;
};
#endif
 	

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

	/*	1280x720		[1.78:1]	*/	
        {NULL, 60,  1280,  720,  13426, 162, 86, 22, 1,  136, 3,
        FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,FB_VMODE_NONINTERLACED},

        {NULL,60,1360,768,11804,208,64,23,1,144,3,
        FB_SYNC_HOR_HIGH_ACT|FB_VMODE_NONINTERLACED},


	/*	1360 x 768	[1.77083:1]	*/
	 {NULL,  60, 1360, 768, 11804, 208,  64, 23, 1, 144, 3,
         FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED},
			
	/*	1368 x 768      [1.78:1]	*/
        {NULL, 60,  1368,  768,  11647, 216, 72, 23, 1,  144, 3,
        FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED}, 
	
 	/* 	1440 x 900		[16:10]	*/
	{NULL, 60, 1440, 900, 9392, 232, 80, 28, 1, 152, 3,
	FB_SYNC_VERT_HIGH_ACT,FB_VMODE_NONINTERLACED},

	/*	1440x960		[15:10]	*/
	{NULL, 60, 1440, 960, 8733, 240, 88, 30, 1, 152, 3, 
	 FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED}, 

	/*	1920x1080	[16:9]	*/
	{NULL, 60, 1920, 1080, 6734, 148, 88, 41, 1, 44, 3,
	FB_SYNC_VERT_HIGH_ACT,FB_VMODE_NONINTERLACED},	        
};


static const struct fb_ops sm750fb_ops = {
	.owner = THIS_MODULE,
	.fb_check_var = sm750fb_ops_check_var,
	.fb_set_par = sm750fb_ops_set_par,
	.fb_setcolreg = sm750fb_ops_setcolreg,
	.fb_blank = sm750fb_ops_blank,
	.fb_pan_display = sm750fb_ops_pan_display,
	.fb_fillrect = sm750fb_ops_fillrect,
	.fb_copyarea = sm750fb_ops_copyarea,
	.fb_imageblit = sm750fb_ops_imageblit,
	.fb_cursor = sm750fb_ops_cursor,
	//.fb_sync = sm750fb_ops_sync,	
#ifdef __BIG_ENDIAN	
	.fb_write = sm750fb_ops_write,
	.fb_read = sm750fb_ops_read,
#endif
};


/*
	if the name is invalid for a ulong type,
	then no guarantee will be provided that the reture value is with meaning
*/
static ulong my_atoul(const char *name)
{
    ulong val = 0;

    for (;; name++) {
	switch (*name) {
	    case '0' ... '9':
		val = 10*val+(*name-'0');
		break;
	    default:
		return val;
	}
    }
}

static int getTFTsize(char * expstring,uint * x,uint * y)
{
	char * pcx,*pcy;
	pcy = expstring;
	pcx = strsep(&pcy,"x");
	if(pcx!=NULL && pcy!=NULL)
	{
		*x = my_atoul(pcx);
		*y = my_atoul(pcy);
		return 0;
	}
	return -1;
}

static struct sm750fb_share* sm750fb_create_share(struct pci_dev * pdev,	const struct pci_device_id * ent)
{
	struct sm750fb_share * share;
	SMENTER;
	share = kzalloc(sizeof(struct sm750fb_share),GFP_KERNEL);
	if(!share){
		err_msg("Could not allocate memory for p_share.\n");
		goto err;		
	}
	
	share->pdev = pdev;
	share->state = default_state;
	
#if 1
	share->resMem = pdev->resource[BAR_MEM];//copy struct resource from pci_dev
	share->resReg = pdev->resource[BAR_MMIO];
#endif

	/* print some debug message	*/
	dbg_msg("resMem:\n->start:%08x\n->end:%08x\n->name:%s\n",\
		share->resMem.start,share->resMem.end,share->resMem.name);

	dbg_msg("resReg:\n->start:%08x\n->end:%08x\n->name:%s\n",\
		share->resReg.start,share->resReg.end,share->resReg.name);
#if 0
	/* reserve the mem and mmio regions	*/
	if(!request_mem_region(pci_resource_start(pdev,BAR_MMIO),
				pci_resource_len(pdev,BAR_MMIO),SM750_MODULE_NAME))
	{
		err_msg("can not request mem region.\n");
		goto err_request_mem;
	}

	resource_size_t size = pci_resource_len(pdev,BAR_MEM);
	if(!request_mem_region(pci_resource_start(pdev,BAR_MEM),
					size,	SM750_MODULE_NAME))

	{
		err_msg("can not request mmio region.\n");
		goto err_request_mmio;
	}
#else
	if (pci_request_regions(pdev, SM750_MODULE_NAME)) {
		err_msg("cannot request PCI regions\n");
		goto err_request_regions;
	}

#endif
	/* now map mmio and mem	*/
	share->pvReg = ioremap(pci_resource_start(pdev,BAR_MMIO),
				pci_resource_len(pdev,BAR_MMIO));
	
	pMMIO = share->pvReg;
	if(pMMIO == NULL)
	{
		err_msg("failed to ioremap mmio .\n");
		goto err_map;
	}
	dbg_msg("After ioremap share->pvReg is %p , %p\n",share->pvReg,pMMIO);

	share->mem_size = hw_get_memsize();	
	
	dbg_msg("sm750 actual memory size = %lu.\n",share->mem_size);

	/*
		for 750 pci-e interface  no need to resize memory to 62 
		but yes for 502 host interface.
		502's mmio physica address is just 
		62mega + memory start address

		Maybe 718 host interface need below code ,  so temporarily
		leave these code ...
	*/
#if 0	
	if(share->mem_size == MB(64)){
		dbg_msg("resize memory to 62mega\n");
		share->mem_size = MB(62);
	}	
#endif	

	share->pvMem = ioremap(pci_resource_start(pdev,BAR_MEM),share->mem_size);	
	pVMEM = share->pvMem;
	if(pVMEM == NULL)
		err_msg("failed to ioremap video ram .\n");
	else
		dbg_msg("After ioremap share->pvMem is %p \n",share->pvMem);
	
	/* clear memory first */
	memset(share->pvMem,0,share->mem_size);

#ifdef CONFIG_MTRR
	if (!nomtrr) {
		share->mtrr.vram = mtrr_add(share->resMem.start,
					  			share->mem_size,
					  			MTRR_TYPE_WRCOMB, 1);
		if (share->mtrr.vram < 0) {
			err_msg("unable to setup MTRR\n");
		} else {
			share->mtrr.vram_valid = 1;			
		}
	}
#endif
	
	SMEXIT;
	return share;	

err_map:	
err_request_regions:
	sm750fb_clr_share(share);
	kfree(share);		
err:	
	return NULL;
}

/*
	*sm750fb_set_fbinfo* is the most weekest routine of this driver
	This routine makes kernel panic usually,so be careful on it
	e.g. 	*info->fbops* must be set before you call *fb_find_mode*		
						--Monk Liu
*/
static int sm750fb_set_fbinfo(struct fb_info* info,struct sm750fb_share* share,enum sm750fb_controller channel)
{
	
/* 
 *	set member of info  
 *	share need be initiated before this routine be called
 */
 	int ret;
	struct sm750fb_par * par;
	struct fb_var_screeninfo * var ;
	int osize,os_linelength;
	
	SMENTER;
	par = info->par;
	var = &info->var;

	/*
	  	setting info
		info->fbops must be first inited, fb_find_mode need it  
	*/
	info->fbops = &sm750fb_ops;
	
	if(mode_option == NULL){
		mode_option = defaultMode;
	}	

	ret = fb_find_mode(var,info,mode_option,sm750modbExt,ARRAY_SIZE(sm750modbExt),NULL,16);
	
	if(ret!=1){
		dbg_msg("specify mode not in sm750modbExt.\n");	
		ret = fb_find_mode(&info->var,info,mode_option,NULL,0,NULL,16);
	}	

	if(ret==0 || ret == 4){
		err_msg("failed to get specify mode. \n");
		ret = -EINVAL;
		goto err_mode;		
	}
	
	/*
		some member of info->var had been set by fb_find_mode
	*/
	
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
	/*
	 *	setting par 
	 */
	 
	par->share = share;
	par->info = info;	
	share->pfb[channel] = info;	
	
	os_linelength = (info->var.xres_virtual * info->var.bits_per_pixel ) >> 3;
	os_linelength = PADDING(16,os_linelength);//sm750 line offset is 16 byte aligned
	osize = os_linelength * info->var.yres_virtual;
	par->controller = channel;	

	if(hw_alloc_mem(par,osize,(channel == CTRL_PRI )?REASON_PRI_SURFACE:REASON_SEC_SURFACE)){
		err_msg("failed to alloc vidmem for onscreen.\n");
		goto err_vm_os;
	}	

	/*
		IF expansion used on secondary channel ,then hardware cursor should be off
	*/
	
	if(channel == CTRL_SEC && par->share->state.xLCD != 0 && par->share->state.yLCD != 0)
	{
		info->fbops->fb_cursor = NULL;
	}
	else
	{
		if(hw_alloc_mem(par,MAX_CURS_SIZE,(channel == CTRL_PRI )?REASON_PRI_CURSOR:REASON_SEC_CURSOR))
		{
			err_msg("failed to alloc vidmem for cursor.\n");
			goto err_vm_cur;
		}
	}	
		
	info->pseudo_palette = par->pseudo_palette;
	info->screen_base = par->screen.pvAdd;
	info->screen_size = osize;
	info->flags = FBINFO_FLAG_DEFAULT|
			FBINFO_HWACCEL_IMAGEBLIT|
			FBINFO_HWACCEL_FILLRECT|
			FBINFO_HWACCEL_COPYAREA|			
			0;
/*
 * 	setting info->fix
 */
	info->fix.type = FB_TYPE_PACKED_PIXELS;
	info->fix.type_aux = 0;
	info->fix.xpanstep = 1;
	info->fix.ypanstep = 1;
	info->fix.ywrapstep = 0;
	info->fix.accel = FB_ACCEL_NONE;//don't care about it	

	strlcpy(info->fix.id,pcFbname[channel],sizeof(info->fix.id));

	info->fix.smem_start = par->screen.offset + par->share->resMem.start;
	info->fix.smem_len = osize;
	info->fix.line_length = os_linelength;
	info->fix.mmio_start = par->share->resReg.start;
	info->fix.mmio_len = par->share->resReg.end - par->share->resReg.start + 1;	
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
	
/*
 *	setting info->var 
 */ 
	info->var.activate = FB_ACTIVATE_NOW;
	info->var.accel_flags = 0;
	info->var.vmode = FB_VMODE_NONINTERLACED;	

	dbg_msg("#1 show info->cmap : \nstart=%d,len=%d,red=%p,green=%p,blue=%p,transp=%p\n",
			info->cmap.start,info->cmap.len,
			info->cmap.red,info->cmap.green,info->cmap.blue,
			info->cmap.transp);

	if(fb_alloc_cmap(&info->cmap,256,0) < 0){
		err_msg("Could not allcate memory for cmap.\n");
		ret = -EINVAL;
		goto err_cmap;
	}

	dbg_msg("#2 show info->cmap : \nstart=%d,len=%d,red=%p,green=%p,blue=%p,transp=%p\n",
			info->cmap.start,info->cmap.len,
			info->cmap.red,info->cmap.green,info->cmap.blue,
			info->cmap.transp);
	//fb_set_cmap(&info->cmap,info);		
	SMEXIT;
	return 0;
err_vm_cur:
err_vm_os:
#if 0
	kfree(par->cursor);
err_km_2:
	kfree(par->screen);
err_km_1:
#endif
err_cmap:
err_mode:
	return ret;	
}
/*
 * free member of sm750fb_share which allocated by kzalloc
 */
static inline void sm750fb_clr_share(struct sm750fb_share * share)
{
	return;
}

static inline void sm750fb_clr_par(struct sm750fb_par* par)
{
	return;
}

static struct pci_device_id sm750fb_pci_table[] = {
	{PCI_VENDOR_ID_SMI,PCI_DEVICE_ID_750,PCI_ANY_ID,PCI_ANY_ID,0,0,0},
	{PCI_VENDOR_ID_SMI,PCI_DEVICE_ID_718,PCI_ANY_ID,PCI_ANY_ID,0,0,0},
	{0,}
};

static struct pci_driver sm750fb_driver = {
	.name =	SM750_MODULE_NAME,
	.id_table =	sm750fb_pci_table,
	.probe =		sm750fb_pci_probe,
	.remove =	__devexit_p(sm750fb_pci_remove),
#ifdef CONFIG_PM
	.suspend = sm750fb_suspend,
	.resume = sm750fb_resume,
#endif
};

MODULE_AUTHOR("monk Liu <monk_liu@126.com>");
MODULE_DESCRIPTION("Framebuffer driver for SMI(R) " SUPPORT_CHIP "chipsets");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DEVICE_TABLE(pci,sm750fb_pci_table);

static int __init sm750fb_setup(char *options)
{
	char *opt;
	if(!options || !*options){
		inf_msg("no options\n");
		return 0;
	}else{
		inf_msg("options:%s\n",options);
	}
	while((opt = strsep(&options,",")) != NULL)
	{
		if(!strncmp(opt,"noaccel",strlen("noaccel"))){
			noaccel = 1;}
#ifdef CONFIG_MTRR
		else if (!strncmp(opt, "nomtrr", strlen("nomtrr"))) {
			nomtrr = 1;}
#endif
		else if(!strncmp(opt,"dumpreg",strlen("dumpreg"))){
			dumpreg = 1;}
#if 0		
		else if(!strncmp(opt,"simul",strlen("simul")))
			simul = 1;
		else if(!strncmp(opt,"dual",strlen("dual")))
			dualview = 1;
#endif
		else if(!strncmp(opt,"tft+crt",strlen("tft+crt")))	{
			output = OP_SIMUL_TFT_CRT;}
		else if(!strncmp(opt,"tft1+tft2",strlen("tft1+tft2"))){
			output = OP_SIMUL_TFT1_TFT2;}
		else if(!strncmp(opt,"crt1+crt2",strlen("crt1+crt2"))){
			output = OP_SIMUL_CRT1_CRT2;}
		else if(!strncmp(opt,"exp:",strlen("exp:"))){
			exp_res = opt+strlen("exp:");}
		else if(!strncmp(opt,"18bit",strlen("18bit"))){
			pnltype = PNLTYPE_18;}
		else if(!strncmp(opt,"24bit",strlen("24bit"))){
			pnltype = PNLTYPE_24;}
		else if(!strncmp(opt,"36bit",strlen("36bit"))){
			pnltype = PNLTYPE_36;}	
		else
		{
			/* only first time correspond string found be considered as mode string	*/
			mode_option = (mode_option == NULL) ?opt:mode_option;
			inf_msg("mode string  = %s \n",mode_option);
		}		
	}	
	return 0;
}

/*
 * This routine will always be called in init part
 */ 
static int __init sm750_set_drv_status()
{
	default_state.output = output;
	if(default_state.output == OP_SIMUL_TFT_CRT)
	{
		default_state.panelbit = pnltype;
		if(getTFTsize(exp_res,&default_state.xLCD,&default_state.yLCD))
		{
			/* seems exp_res is invalid*/
			default_state.xLCD = 0;
			default_state.yLCD = 0;
		}		
	}
	else if(default_state.output == OP_SIMUL_TFT1_TFT2)
	{
		default_state.panelbit = PNLTYPE_18;
	}	
	
	default_state.de_off = noaccel; 
#ifdef CONFIG_MTRR	
	default_state.mtrr_off = nomtrr;
#endif

	/* output information*/
	inf_msg("--+ output driver configuration information +--\n");

	inf_msg("screen mode : %s \n",mode_option?mode_option:"default mode");
	inf_msg("2d accel : %s \n",default_state.de_off?"off":"on");
#ifdef CONFIG_MTRR
	inf_msg("mtrr : %s\n",default_state.mtrr_off?"off":"on");
#endif
	inf_msg("output : %s\n",output_text[default_state.output]);
	if(output!=OP_SIMUL_CRT1_CRT2)
		inf_msg("panel type : %s\n",pnltype_text[default_state.panelbit]);
	if(default_state.xLCD !=0 && default_state.yLCD !=0)
		inf_msg("expansion mode : %dx%d @60hz\n",default_state.xLCD,default_state.yLCD);
	
	inf_msg("--+ end +--\n");	
	return 0;
}


static int __init sm750fb_init()
{	
	char *option = NULL;	
	int ret;

	SMENTER;
	inf_msg("Frame buffer driver for smi(R) " SUPPORT_CHIP " chipsets\n");
	inf_msg("Driver Version: " SM750FB_VERSION " \n");
	
	if(fb_get_options("sm750fb",&option))
		return -ENODEV;
	sm750fb_setup(option);
	sm750_set_drv_status();
	ret = pci_register_driver(&sm750fb_driver);
	SMEXIT;
	return ret;
}

module_init(sm750fb_init);
#ifdef MODULE
static void __exit sm750fb_exit()
{
	inf_msg("module \"sm750fb\" exit.\n");
	pci_unregister_driver(&sm750fb_driver);	
}

module_exit(sm750fb_exit);
#endif

static int __devinit sm750fb_pci_probe(struct pci_dev *pdev,
					const struct pci_device_id *ent)
{

	struct fb_info *info_first = NULL;
	struct sm750fb_share *p_share = NULL;	//shared data of par_pnl 
	enum sm750fb_controller controller;
	SMENTER;	

	/* enable device */
	if(pci_enable_device(pdev)){
		err_msg("can not enable device.\n");
		goto err;
	}
	
	p_share = sm750fb_create_share(pdev,ent);
	if(!p_share){
		err_msg("Could not create p_share.\n");
		goto err_share_create;
	}
		
	info_first = framebuffer_alloc(sizeof(struct sm750fb_par),&pdev->dev);
	if(!info_first){
		err_msg("Could not allocate memory for info_first.\n");
		goto err_info_pnl_alloc;
	}

	if(p_share->state.output == OP_SIMUL_TFT_CRT &&
		p_share->state.xLCD!=0 && p_share->state.yLCD!=0){
		/* use secondary channel to implement expansion feature	*/
		controller = CTRL_SEC;}
	else{
		/*	use primary channel cuz no expansion need	*/
		controller = CTRL_PRI;}

	if(sm750fb_set_fbinfo(info_first,p_share,controller))
	{
		err_msg("failed to initial fb_info.\n");
		goto err_info_pnl_set;
	}
	
	pci_set_drvdata(pdev,p_share);
	
	/*	
		init hardware to known stats
		Setting clocks, powermode, 2d_engine status.etc...
 	*/	

	if(hw_init_chip(&p_share->state))
	{
		err_msg("failed to init chip.\n");
		goto err_init_chip;
	}

	if(register_framebuffer(info_first) < 0){
		err_msg("failed to register framebuffer.\n");
		goto err_fb_register;
	}

	inf_msg("Register_framebuffer done.\n");
	
	SMEXIT;	
	return 0;

err_fb_register:	
err_init_chip:
err_info_pnl_set:
	framebuffer_release(info_first);
err_info_pnl_alloc:
	sm750fb_clr_share(p_share);//clear member of p_share
	kfree(p_share);
err_share_create:
err:
	return -ENODEV;		
}


static void __devexit sm750fb_pci_remove(struct pci_dev *pdev)
{
	struct fb_info * info;
	struct sm750fb_share * share;
	struct sm750fb_par * par;
	int index = MAX_FB;
	
	SMENTER;
	share = pci_get_drvdata(pdev);
	
	while(index-- !=0)
	{
		info = share->pfb[index];
		if(info!=NULL)
			par = info->par;
		else
			continue;

		/*	If we goto sleep mode here , then we will fail on re-insmod 	*/
		//hw_suspend(par);
		sm750fb_ops_blank(FB_BLANK_POWERDOWN, info);

		unregister_framebuffer(info);
		sm750fb_clr_par(par);
		framebuffer_release(info);				
	}
	
	
#ifdef CONFIG_MTRR
	if (share->mtrr.vram_valid)
		mtrr_del(share->mtrr.vram, share->resMem.start, share->mem_size);
#endif
	
	iounmap(share->pvReg);
	iounmap(share->pvMem);	
	pci_release_regions(pdev);
	sm750fb_clr_share(share);
	kfree(share);
	pci_set_drvdata(pdev,NULL);
	SMEXIT;	
}

#ifdef CONFIG_PM
static int sm750fb_suspend(struct pci_dev *dev, pm_message_t mesg)
{
	struct sm750fb_share * share;
	struct fb_info * info;
	struct sm750fb_par * par;
	int retv;

	share = (struct sm750fb_share *)pci_get_drvdata(dev);
	if(share->state.output == OP_SIMUL_TFT_CRT && share->state.xLCD!=0)
		info = share->pfb[CTRL_SEC];
	else
		info = share->pfb[CTRL_PRI];
	par = (struct sm750fb_par *) info->par;

	switch (mesg.event) {
	case PM_EVENT_FREEZE:
	case PM_EVENT_PRETHAW:
		dev->dev.power.power_state = mesg;
		return 0;
	}	
	
	acquire_console_sem();
	
	if (mesg.event == PM_EVENT_SUSPEND) {		
		fb_set_suspend(info, 1);
		
		retv = pci_save_state(dev);	
		pci_disable_device(dev);
		retv = pci_choose_state(dev, mesg);	
		retv = pci_set_power_state(dev, retv);

		/* set 750 to sleep mode	*/
		hw_suspend(par);
	}
	
	dev->dev.power.power_state = mesg;
	release_console_sem();	
	return 0;

}


static int sm750fb_resume(struct pci_dev *dev)
{
	struct sm750fb_share * share;
	struct fb_info * info;
	struct sm750fb_par * par;
	
	int retv;
	
	share = (struct sm750fb_share *)pci_get_drvdata(dev);
	if(share->state.output == OP_SIMUL_TFT_CRT && share->state.xLCD!=0)
		info = share->pfb[CTRL_SEC];
	else
		info = share->pfb[CTRL_PRI];
	par = (struct sm750fb_par *) info->par;
	
	/*
		video memory is massed up 
		don't worry about onscreen data (frame buffer uppeer layer do handle it)
		cursor space memory need re-clear
	*/
	
	if(dev->dev.power.power_state.event != PM_EVENT_FREEZE){
		retv = pci_set_power_state(dev, PCI_D0);
		retv = pci_restore_state(dev);
		if (pci_enable_device(dev)){
			return 0;
		}
		pci_set_master(dev);		
	}
	
	hw_init_chip(&share->state);	
	hw_cursor_clear(par);
	sm750fb_ops_set_par(info);
	
	
	acquire_console_sem();
	fb_set_suspend (info, 0);
	release_console_sem();		
	return 0;
}
#endif 

static inline int h_total(struct fb_var_screeninfo *var)
{
	return var->xres + var->left_margin +var->right_margin + var->hsync_len;
}

static inline int v_total(struct fb_var_screeninfo *var)
{
	return var->yres + var->upper_margin +var->lower_margin + var->vsync_len;
}

static int sm750fb_ops_check_var(struct fb_var_screeninfo* var,struct fb_info* info)
{
	/*
	 * checks var and eventually tweaks it to something supported.
	 * Do not modify PAR
	 */
	 
 	int ret = 0;
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
		return -EINVAL;
	}		
	return ret;
}

static int sm750fb_ops_set_par(struct fb_info* info)
{
	struct sm750fb_par * par = info->par;
	struct fb_var_screeninfo * var = &info->var;
	struct fb_fix_screeninfo * fix = &info->fix;
	int ret;
	int length;
/*	
 	modify info->par 
 	modify info->fix
	set the hw video mode	
	Do not modify info->var 
 */
	SMENTER;	
	/* modify par */
	
	length = (var->xres_virtual * var->bits_per_pixel) >> 3;	
	length = PADDING(16,length);
	par->screen.size = length * var->yres_virtual;	
	/* modify fix */
	fix->smem_len = par->screen.size;
	
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
	fix->line_length = length;
	
	/* set hardware */
	ret = hw_set_mode(par,var,fix);
	if(ret){
		err_msg("failed to set mode.\n");
		goto err_set_mode;
	}
	
	//sm750fb_ops_pan_display(var,info);
	if(dumpreg == 1)
		hw_dump_reg(par,DUMPREG_HEAD|DUMPREG_PANEL|DUMPREG_CRT);
	
	SMEXIT;
	return 0;	
err_set_mode:
	return -EINVAL;		
}
static inline uint chan_to_field(ulong chan,struct fb_bitfield* bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}
static int sm750fb_ops_setcolreg(unsigned regno,unsigned red,
				unsigned green,unsigned blue,
				unsigned transp,struct fb_info* info)
{
	struct sm750fb_par * par = info->par;
	struct fb_var_screeninfo * var = &info->var;
	ulong val;

	dbg_msg("regno=%d,red=%d,green=%d,blue=%d\n",regno,red,green,blue);
	if(regno > 255)
	{
		err_msg("regno = %d\n",regno);
		return -EINVAL;
	}

	if(info->var.grayscale)
		red = green = blue = (red * 77 + green * 151 + blue * 28) >> 8;

	if(var->bits_per_pixel == 8 && info->fix.visual == FB_VISUAL_PSEUDOCOLOR){
/*
	funny that 8bpp index mode is not sensitive of big_endian problem
	below code is just all right for both little endian and big endian
	I do not know Y... 	8-(
	just let it be...
*/		
		red >>= 8;
		green >>= 8;
		blue >>= 8;
		hw_set_colreg(par,regno,red,green,blue);
		goto end;
	}
 	
	if(info->fix.visual == FB_VISUAL_TRUECOLOR && regno < 16 )
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

			goto end;
		}				
	}

	err_msg("MONK :regno = %d failed to set colreg.\n",regno);
	goto err;

end:	
	return 0;
err:
	return -EINVAL;
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
	return hw_set_blank(info->par,blank_mode);	
}

static int sm750fb_ops_pan_display(struct fb_var_screeninfo* var,struct fb_info* info)
{
	return  hw_pan_display(info->par,var);		
}
static void sm750fb_ops_fillrect(struct fb_info* info,const struct fb_fillrect* region)
{
	u32 color;
	ulong base,pitch,bpp,rop;
	struct sm750fb_par * par = info->par;

	if(noaccel){
		dbg_msg("soft fill rect\n");
		goto soft;
	}
	
	base = par->screen.offset;
	pitch = info->fix.line_length;
	bpp = info->var.bits_per_pixel;

	if(info->state != FBINFO_STATE_RUNNING)
	{
		err_msg("info->state !=FBINFO_STATE_RUNNING\n");
		return;
	}
	
	if(info->var.bits_per_pixel == 8)
		color = region->color;
	else
		color = ((u32*)(info->pseudo_palette))[region->color];
	
	if(region->rop == ROP_COPY)
		rop = HW_ROP2_COPY;
	else
		rop = HW_ROP2_XOR;

	hw_2d_fillrect(par,
			base,pitch,bpp,
			region->dx,region->dy,
			region->width,region->height,
			color,rop);
	return;
soft:
	cfb_fillrect(info,region);	
}
static void sm750fb_ops_copyarea(struct fb_info* info,const struct fb_copyarea* region)
{
	struct sm750fb_par * par = info->par;
	ulong base,pitch,bpp;
#if 1
	if(noaccel)	
#endif		
	{
		goto soft;
	}
	
	base = par->screen.offset;
	pitch = info->fix.line_length;
	bpp = info->var.bits_per_pixel;

	hw_2d_copyarea(par,
			base,pitch,bpp,
			region->dx,region->dy,
			region->sx,region->sy,
			region->width,region->height);
	return;
soft:
	cfb_copyarea(info,region);
}
static void sm750fb_ops_imageblit(struct fb_info* info,const struct fb_image* image)
{
	ulong base,pitch,bpp;
	ulong fgcol,bgcol;
	int srcmono;
	
	struct sm750fb_par * par = info->par;
	srcmono = 0;
	
#if 1
	if(noaccel)
#endif		
	{
		goto soft;
	}

	base = par->screen.offset;
	pitch = info->fix.line_length;
	bpp = info->var.bits_per_pixel;

	if(image->depth == 1)
	{
		srcmono = 1;
		if(info->fix.visual == FB_VISUAL_TRUECOLOR ||info->fix.visual == FB_VISUAL_DIRECTCOLOR)
		{
			fgcol = ((u32*)info->pseudo_palette)[image->fg_color];
			bgcol = ((u32*)info->pseudo_palette)[image->bg_color];		
		}
		else
		{
			fgcol = image->fg_color;
			bgcol = image->bg_color;
		}
	}

	if(srcmono){
		hw_2d_imageblit_mono(par,base,pitch,bpp,
			image->dx,image->dy,
			image->width,image->height,
			image->data,fgcol,bgcol);
		return;
	}
soft:
	cfb_imageblit(info,image);
}

static int sm750fb_ops_cursor(struct fb_info* info,struct fb_cursor * fbcur)
{
	struct sm750fb_par * par = info->par;
	ulong dx,dy;
	u32 bg,fg;	
	const u8 *pcol,*pmsk;
	int ret = 0;

#ifdef DEBUG
	if(fbcur->set!=0)
		dbg_msg("cursor->set = %04x\n",fbcur->set);
#endif

	if(fbcur->image.width > MAX_CURS_W || fbcur->image.height > MAX_CURS_H){
		ret = -ENXIO;
		err_msg("fb_cursor image width or height can not > %d \n",MAX_CURS_W);
		goto exit;
	}
	if(fbcur->image.depth > 1){
		ret = -ENXIO;
		err_msg("fb_cursor image depth can not > 1 \n");
		goto exit;
	}


	hw_cursor_enable(par,0);

	if(fbcur->set & FB_CUR_SETSIZE){
		dbg_msg("set cursor size to [%d,%d].\n",fbcur->image.width,fbcur->image.height);
		
		par->cursor.size = PADDING(8,(fbcur->image.width * fbcur->image.height * 2)) >> 3;		
		hw_cursor_setsize(fbcur->image.width,fbcur->image.height);		
	}
	if(fbcur->set & FB_CUR_SETPOS){
		
		dbg_msg("set cursor position to [%d,%d].\n",	fbcur->image.dx,fbcur->image.dy);
		
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
	 	dbg_msg("set cursor data\n");
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
	return ret;
}

#ifdef __BIG_ENDIAN
/*
	below two function will receive big-endian colour data and translate it into
	little-endian data cuz sm750 can not hardware support big-endian data
*/
static ssize_t sm750fb_ops_write(struct fb_info *info, const char __user *buf,
			    size_t count, loff_t *ppos)
{
	unsigned long p = *ppos;
	struct inode *inode = file->f_path.dentry->d_inode;	
	int fbidx = iminor(inode);
	struct fb_info *info = registered_fb[fbidx];
	u32 *buffer, *src;
	u32 __iomem *dst;
	int c, i, cnt = 0, err = 0;
	unsigned long total_size;

	if (!info || !info->screen_base)
		return -ENODEV;

	if (info->state != FBINFO_STATE_RUNNING)
		return -EPERM;

	
	total_size = info->screen_size;

	if (total_size == 0)
		total_size = info->fix.smem_len;

	if (p > total_size)
		return -EFBIG;

	if (count > total_size) {
		err = -EFBIG;
		count = total_size;
	}

	if (count + p > total_size) {
		if (!err)
			err = -ENOSPC;

		count = total_size - p;
	}

	buffer = kmalloc((count > PAGE_SIZE) ? PAGE_SIZE : count,
			 GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	dst = (u32 __iomem *) (info->screen_base + p);

	if (info->fbops->fb_sync)
		info->fbops->fb_sync(info);

	while (count) {
		c = (count > PAGE_SIZE) ? PAGE_SIZE : count;
		src = buffer;

		if (copy_from_user(src, buf, c)) {
			err = -EFAULT;
			break;
		}

		for (i = c >> 2; i--; ){
			fb_writel( (*src& 0xff00ff00>>8)|(*src& 0x00ff00ff<<8), dst++);
			src++;
		}
		if (c & 3) {
			u8 *src8 = (u8 *) src;
			u8 __iomem *dst8 = (u8 __iomem *) dst;

			for (i = c & 3; i--; ){
				if (i&1){
				fb_writeb(*src8++, ++dst8);
				}
				else{
				fb_writeb(*src8++, --dst8);
				dst8 +=2;
				}
			}
			dst = (u32 __iomem *) dst8;
		}

		*ppos += c;
		buf += c;
		cnt += c;
		count -= c;
	}

	kfree(buffer);

	return (cnt) ? cnt : err;


}
static ssize_t sm750fb_ops_read(struct fb_info *info, char __user *buf,
			   size_t count, loff_t *ppos)
{
	unsigned long p = *ppos;
	struct inode *inode = file->f_path.dentry->d_inode;
	int fbidx = iminor(inode);
	struct fb_info *info = registered_fb[fbidx];
	u32 *buffer, *dst;
	u32 __iomem *src;
	int c, i, cnt = 0, err = 0;
	unsigned long total_size;

	if (!info || ! info->screen_base)
		return -ENODEV;

	if (info->state != FBINFO_STATE_RUNNING)
		return -EPERM;

	total_size = info->screen_size;

	if (total_size == 0)
		total_size = info->fix.smem_len;

	if (p >= total_size)
		return 0;

	if (count >= total_size)
		count = total_size;

	if (count + p > total_size)
		count = total_size - p;

	buffer = kmalloc((count > PAGE_SIZE) ? PAGE_SIZE : count,
			 GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	src = (u32 __iomem *) (info->screen_base + p);

	if (info->fbops->fb_sync)
		info->fbops->fb_sync(info);

	while (count) {
		c  = (count > PAGE_SIZE) ? PAGE_SIZE : count;
		dst = buffer;
		for (i = c >> 2; i--; ){
			*dst = fb_readl(src++);
			*dst  = (*dst & 0xff00ff00>>8)|(*dst & 0x00ff00ff<<8);
			dst++;
			}
		if (c & 3) {
			u8 *dst8 = (u8 *) dst;
			u8 __iomem *src8 = (u8 __iomem *) src;

			for (i = c & 3; i--;){
				if (i&1){
				*dst8++ = fb_readb(++src8);
				}
				else{
				*dst8++ = fb_readb(--src8);
				src8 +=2;
				}
			}
			src = (u32 __iomem *) src8;
		}

		if (copy_to_user(buf, buffer, c)) {
			err = -EFAULT;
			break;
		}
		*ppos += c;
		buf += c;
		cnt += c;
		count -= c;
	}

	kfree(buffer);

	return (err) ? err : cnt;

}
#endif

#if 0
static int sm750fb_ops_sync(struct fb_info*info)
{
	struct sm750fb_par * par = info->par;
	//normally we need a wait_idle function
	//to excute when 2d acceleration function be used	
	if(hw_2d_wait_idle(par))
	{
		err_msg("failed to wai idle, seems 2d engine die.\n");
		return -ETIMEDOUT;
	}
	return 0;
}
#endif




