#ifndef SM750DRV_H__
#define SM750DRV_H__
#define PCI_VENDOR_ID_SMI	0x126f
#define PCI_DEVICE_ID_750	0x750
#define PCI_DEVICE_ID_718	0x718

/*	
 *	1.0.xx means version for sm750
 *	*single arch* frame buffer driver version
 *		xx is a decimal number [ 1 -> 99 ]
 */
#define SM750FB_VERSION		"1.0.7"
#define SUPPORT_CHIP		"Lynx sm750/sm718"
#define SM750_MODULE_NAME	"sm750/718 fb"

#define PFX	SM750_MODULE_NAME ":"
#define err_msg(fmt,args...)		printk(KERN_ERR PFX fmt, ## args)
#define war_msg(fmt,args...)	printk(KERN_WARNING PFX fmt, ## args)
#define inf_msg(fmt,args...)		printk(KERN_INFO PFX fmt, ## args)

#ifdef DEBUG
#define 	dbg_msg(fmt,args...)	printk(KERN_DEBUG PFX "%s:" fmt,__func__,## args)
#define 	SMENTER		dbg_msg("ENTER %s\n",__func__)
#define 	SMEXIT		dbg_msg("EXIT %s\n",__func__)

#else
//#define 	dbg_msg(fmt,args...)	while(0) printk(fmt, ## args)
#define 	dbg_msg(fmt,args...)
#define 	SMENTER		
#define 	SMEXIT		
#endif


#define DEFAULT_MODE	"640x480-16@60" 
#define MAX_FB	2

#if VALIDATION_CHIP
#define BAR_MMIO	3
#define BAR_MEM		1
#else
#define BAR_MMIO	1
#define BAR_MEM		0
#endif
#define MB(x)	(x << 20)
#define REG_SIZE	MB(2)
#define PADDING(align,data)	((data+align-1)&(~(align-1)))

#if 0
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char uchar;
#endif

enum sm750fb_pnltype{
	PNLTYPE_24 = 0,
	PNLTYPE_18 = 1,
	PNLTYPE_36 = 2,
};

/*
#define PNLTYPE_18	0
#define PNLTYPE_24 	1
#define PNLTYPE_36 	2
*/

enum sm750fb_output{
	/* 18/24/36 bit TFT + CRT */
	OP_SIMUL_TFT_CRT = 0,
	/*  two 18bit TFT and no CRT 	*/
	OP_SIMUL_TFT1_TFT2 = 1,
	/*  two CRT on */
	OP_SIMUL_CRT1_CRT2 = 2,	
};

struct sm750fb_state {
	ushort chip_clk;
	ushort mem_clk;
	ushort master_clk;
	ushort de_off;	
#ifdef CONFIG_MTRR
	int mtrr_off;
#endif
	enum sm750fb_output output;
	/* TFT INFORMATION*/
	int xLCD;
	int yLCD;
	enum sm750fb_pnltype panelbit;
};

/*
 * sm750_mem struct present a descript for a video memory
 */
struct sm750fb_mem {
	ulong		size;
	ulong		offset;//offset from start of video memory
	void __iomem	*pvAdd;//virtual address of this memory
};

enum sm750fb_controller {
	CTRL_PRI	=0,	/* panel channel	*/
	CTRL_SEC	=1,	/*	crt channel	*/
};

enum sm750_allocmem_reason {
	REASON_PRI_SURFACE	=0,
	REASON_SEC_SURFACE	=1,
	REASON_PRI_CURSOR		=2,
	REASON_SEC_CURSOR		=3,
};

/*
 * Data that shared between fbcrt and fbpnl
 */
struct sm750fb_share {
	struct pci_dev 	*pdev;
	struct fb_info 	*pfb[MAX_FB];
	struct sm750fb_state state;

	struct resource	resMem;//resource present for video memory 
	struct resource 	resReg;//resource present for mmio registers

	void __iomem	*pvReg;//start virtual address of mmio registers
	void __iomem	*pvMem;//start virtual address of video memory
	ulong 		mem_size;//size of actual vidmem in bytes	
#ifdef CONFIG_MTRR	
	struct {
		int vram;
		int vram_valid;
	}mtrr;
#endif	
	
};
/*
 * per-framebuffer private data
 */
struct sm750fb_par {	
	u32			pseudo_palette[16];
	enum sm750fb_controller	controller;
	struct sm750fb_mem 	cursor;
	struct sm750fb_mem 	screen;	
	struct sm750fb_share *	share;	
	struct fb_info *	info;
};
#endif

