#ifndef LYNX_HW712_H__
#define LYNX_HW712_H__
#include "ddk712/ddk712_chip.h"

struct sm712_share{
	struct lynx_share share;
	/*
	 * 0: no hw cursor for crt channel
	 * not 0: enable hw cursor for crt channel
	 * */
	int hwc;
	unsigned int mclk;//memory clock
	/* below lcd color stuffs need display.h of 712 ddk*/
	enum LCD_TYPE lcd;
	struct {
	enum TFT_COLOR tftColor;
	enum DSTN_COLOR dstnColor;
	}lcd_color;
};

resource_size_t hw_sm712_getVMSize(struct lynx_share * share);
int hw_sm712_map(struct lynx_share* share,struct pci_dev* pdev);
int hw_sm712_inithw(struct lynx_share*,struct pci_dev *);
int hw_sm712_output_checkMode(struct lynxfb_output*,struct fb_var_screeninfo*);
int hw_sm712_output_setMode(struct lynxfb_output*,struct fb_var_screeninfo*,struct fb_fix_screeninfo*);
int hw_sm712_crtc_checkMode(struct lynxfb_crtc*,struct fb_var_screeninfo*);
int hw_sm712_crtc_setMode(struct lynxfb_crtc*,struct fb_var_screeninfo*,struct fb_fix_screeninfo*);
int hw_sm712_setColReg(struct lynxfb_crtc*,ushort,ushort,ushort,ushort);
int hw_sm712_setBLANK(struct lynxfb_output*,int);
void hw_sm712_initAccel(struct lynx_accel*, int devid);
void hw_sm712_crtc_clear(struct lynxfb_crtc*);
void hw_sm712_output_clear(struct lynxfb_output*);
int hw_sm712_deWait(void);

#endif
