#ifndef DDK712_CHIP_H__
#define DDK712_CHIP_H__
#define DEFAULT_INPUT_CLOCK 14318181 /* Default reference clock */

enum LCD_TYPE{
	LCD712_USE_JUMP = -1,
	LCD712_DSTN = 1,
	LCD712_TFT = 0,
};

enum TFT_COLOR{
	TFT_USE_JUMP = -1,
	TFT_9BIT = 0,
	TFT_12BIT = 1,
	TFT_18BIT = 2,
	TFT_24BIT = 3,
	TFT_12P12 = 4,
	TFT_ANALOG = 5,
	TFT_18P18 = 6,
};

enum DSTN_COLOR{
	DSTN_USE_JUMP = -1,
	DSTN_16BIT = 0,
	DSTN_24BIT = 1,
};

typedef struct _init_parm_712{
	int devid;/* sm712 sm722*/
	unsigned int memClock;
	/* pci burst read,write enable bit*/
	int pci_burst;/* bit 0 stand for read and 1 for write*/
	/* lcd settings*/
	enum LCD_TYPE lcd;
	struct {
	enum TFT_COLOR tftColor;
	enum DSTN_COLOR dstnColor;
	}lcd_color;
}init_parm_712;

static inline unsigned int calcMCLK(int mnr,int mdr)
{
	/* don't worry about overlflow
	 * 14.318181 mhz * 255 == 3.4 G */
	return DEFAULT_INPUT_CLOCK * mnr / mdr;
}


unsigned int ddk712_calcPllValue(unsigned int,int*,int*);
void ddk712_hw_init(init_parm_712*);
void ddk712_set_mmio(int,volatile unsigned char *);

#endif
