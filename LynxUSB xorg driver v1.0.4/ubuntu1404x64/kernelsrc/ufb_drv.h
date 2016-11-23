#ifndef SM750DRV_H__
#define SM750DRV_H__

#include "ufb_cmd.h"
#include "usbfb.h"

#define PCI_VENDOR_ID_SMI	0x126f
#define PCI_DEVICE_ID_750	0x750
#define PCI_DEVICE_ID_718	0x718

#ifdef _USE_RANDR_
#define	USE_HW_I2C 
#endif
//#define DEFAULT_MODE	"1400x1050-16@60"
//#define DEFAULT_MODE	"1920x1080-16@60"
//#define DEFAULT_MODE	"1280x1024-8@60"
#define DEFAULT_MODE	"1024x768-16@60"
//#define DEFAULT_MODE	"640x480-16@60"
//#define DEFAULT_MODE	"1280x720-16@60"

#define MAX_FB	1

#define BAR_MMIO	1
#define BAR_MEM		0

#define MB(x)	(x << 20)
#define KB(x) ((x)<<10)

#define VIDEOMEM_SIZE MB(16)
#define PRIVATE_OFFSCREEN_SIZE MB(0)
#define CURSOR_SPACE_SIZE KB(2)
#define REG_SIZE	MB(2)
#define BUF_ASYNC_PAGE 10
#define BUF_ASYNC_SIZE ((1<<BUF_ASYNC_PAGE)*4096)
#define BUF_DMA_PAGE 6
#define BUF_DMA_SIZE ((1<<BUF_DMA_PAGE)*4096)

#define VM_PRIV_SECT_SIZE MB(4)

#define PADDING(align,data)	(((data)+(align)-1)&(~((align)-1)))
#define INFO_2_UFB(info) (struct ufb_data *)(((struct sm750fb_par *)((info)->par))->priv)

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
	int size;//the maximum size it could be,fixed since driver initialized
	int offset;//offset from start of video memory
	int pitch;
	int Bpp;
	void * pvAdd;
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
 * per-framebuffer private data
 */
struct sm750fb_par {	
	uint32_t	pseudo_palette[256];
	enum sm750fb_controller	controller;
	struct sm750fb_mem 	cursor;
	struct sm750fb_mem 	screen;	
	struct fb_info * info;
	struct delayed_work free_framebuffer_work;
#ifdef BURST_CONSOLE
	struct delayed_work draw_framebuffer_work;
	struct workqueue_struct * wq_drawfb;  
	int msec_interval;
	struct boxRect damageArea;
	spinlock_t slock_dj;
#endif
	atomic_t fbcount;
	void * priv;//point to ufb
	int priv_vm_offset;
	int idx;/* 0 or 1*/
	int headX;/* head0,head1,...,head7 if dual card ?*/
	int csc_playing;
	uint32_t flip;
	struct x_io_video_opt g_vopt;
    int convert32to16;
    int flagofmirror;//0 for clone(of noRandr), 1 for dual, 2 for mirror.
};
#endif

