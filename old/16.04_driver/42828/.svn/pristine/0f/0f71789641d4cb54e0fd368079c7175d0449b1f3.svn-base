#ifndef LYNX_HW502_H__
#define LYNX_HW502_H__
#include "lynx_help.h"
#include "voyager.h"

#define DEFAULT_INPUT_CLOCK 24000000
#define VGX_MMIO 1
#define VGX_MEM  2

enum sm502_clocktype{
    /* Divider clocks */
	SM502_P2XCLK = 0,/* PANEL CONTROLLER CLOCK */
	SM502_V2XCLK = 1,/* CRT CONTROLLER CLOCK */
	SM502_MXCLK = 2,/* MAIN ENGINE CLOCK */
	SM502_M1XCLK = 3,/* SDRAM CONTROLLER CLOCK */

	/* the following are for sm502 and later */
    SM502_P1XCLK,       /* x1 panel divider clock */
    SM502_V1XCLK,       /* x1 CRT divider clock , used for secondary channel  */
    SM502_PANEL_PLL,    /* Programmable panel pixel clock, used for primary channel  */
    SM502_PANEL_PLL_2X, /* For TV, 2X clock is needed */
};


enum sm502_pnltype{
	sm502_24TFT = 0,/* 24 bit tft  */
	sm502_9TFT = 1,/* 9 bit tft */
	sm502_12TFT = 2,/* 12 bit tft */
};

/* vga channel is not concerned  */
enum sm502_dataflow{
	sm502_simul_pri,/* primary => all head */
	sm502_dual_normal,/* primary => panel head and secondary => crt */
	sm502_single_sec,
};


enum sm502_channel{
	sm502_primary = 0,
	/* enum value equal to the register filed data */
	sm502_secondary = 1,
};

enum sm502_path{
	sm502_panel = 1,
	sm502_crt = 2,
	sm502_pnc = 3,/* panel and crt */
};

struct init_status502{
	ushort powerMode;/* defaultly 0 */
	/* below clocks are in unit of MHZ*/
	ushort mem_clk;
	ushort master_clk;/* 2d engine clock */
	ushort setAllEngOff;
	ushort resetMemory;
};

struct sm502_state{
	struct init_status502 initParm;
	enum sm502_pnltype pnltype;
	enum sm502_dataflow dataflow;
	int nocrt;/* when in simul mode,nocrt == 1 means only panel output show content */
};

/* 	sm502_share stands for a presentation of two frame buffer
	that use one sm502 adaptor */

struct sm502_share{
	/* it's better to put lynx_share struct to the first place of sm502_share */
	struct lynx_share share;
	struct sm502_state state;
	/* 	0: no hardware cursor
		1: primary crtc hw cursor enabled,
		2: secondary crtc hw cursor enabled
		3: both ctrc hw cursor enabled
	*/
	int hwCursor;
	/* usb always use bottom memory: totalsize - 512kb */
	int usb_off;
	int aud_off;

};

struct sm502_pll_value{
	enum sm502_clocktype clockType;
	unsigned int inputFreq;

    /* Use this for all clockType != PANEL_PLL */
    unsigned int dividerClk; /* either x12 or x14 of the input reference crystal */
    unsigned int divider;
    unsigned int shift;

    /* Use this when clockType = PANEL_PLL */
    unsigned int M;
    unsigned int N;
    unsigned int divBy2;
};

int hw_sm502_map(struct lynx_share* share,struct pci_dev* pdev);
int hw_sm502_inithw(struct lynx_share*,struct pci_dev *);
int hw_sm502_deWait(void);
void hw_sm502_initAccel(struct lynx_accel*);
resource_size_t hw_sm502_getVMSize(struct lynx_share *);
int hw_sm502_output_checkMode(struct lynxfb_output*,struct fb_var_screeninfo*);
int hw_sm502_output_setMode(struct lynxfb_output*,struct fb_var_screeninfo*,struct fb_fix_screeninfo*);
int hw_sm502_crtc_checkMode(struct lynxfb_crtc*,struct fb_var_screeninfo*);
int hw_sm502_crtc_setMode(struct lynxfb_crtc*,struct fb_var_screeninfo*,struct fb_fix_screeninfo*);
int hw_sm502_setColReg(struct lynxfb_crtc*,ushort,ushort,ushort,ushort);
int hw_sm502_setBLANK(struct lynxfb_output*,int);
void hw_sm502_crtc_clear(struct lynxfb_crtc*);
void hw_sm502_output_clear(struct lynxfb_output*);
//static void sm502fb_wait_panelvsnc(int vsync_count);
extern unsigned long SmRead8(unsigned long nOffset);
extern unsigned long SmRead16(unsigned long nOffset);
extern unsigned long SmRead32(unsigned long nOffset);
extern volatile unsigned char __iomem * SmIoAddress(void);
extern void SmWrite8(unsigned long nOffset, unsigned long nData);
extern void SmWrite16(unsigned long nOffset, unsigned long nData);
extern void SmWrite32(unsigned long nOffset, unsigned long nData);
extern void SmMemset(unsigned long nOffset, unsigned char val, int count);
extern void sm501_set_gate( unsigned long gate);
extern int sm501_unit_power(struct device *,unsigned int,unsigned int);
extern int sm501_modify_reg(struct device*,unsigned long,unsigned long,unsigned long);

#endif
