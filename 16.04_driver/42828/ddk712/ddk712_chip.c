#include "ddk712_chip.h"
#include "ddk712_help.h"

/* MCLK X mdr = INPUT X mnr */
unsigned int ddk712_calcPllValue(unsigned int clock,int* o_mnr,int* o_mdr)
{
	int mdr,mnr;
	unsigned int mxm;
	unsigned int request,input;
	unsigned int quo,rem;
	unsigned int miniDiff,diff0;

	miniDiff = ~0;
	/* clocl diveded b 1000 to prevent from overflow exception */
	request = clock /1000;
	input = DEFAULT_INPUT_CLOCK / 1000;

	for(mdr=1;mdr<64;mdr++){
		mxm = request * mdr;
		quo = mxm / input;
		rem = mxm % input;
		if((rem * 100 / input) > 500)
			quo++;
		mnr = quo;

		if(mnr < 256){
			diff0 = absDiff(calcMCLK(mnr,mdr),clock);
			if(diff0 < miniDiff)
			{
				*o_mnr = mnr;
				*o_mdr = mdr;
			}
		}
	}

	return calcMCLK(*o_mnr,*o_mdr);
}

void ddk712_hw_init(init_parm_712 * param)
{
	int mnr,mdr;
	mnr = mdr = 1;

	/* for sm712 pci burst enabling
	 * either:
	 * open both burst write and read
	 * or
	 * open burst write and disable burst read
	 * nothing else
	 * */
	if(param->pci_burst == 2){
		/* burst write on and read off*/
		poke_scr(0x17,peek_scr(0x17)|0x20);
		poke_scr(0x15,peek_scr(0x15)&~0x80);
	}else if(param->pci_burst == 3){
		/* both burst  read and write on*/
		poke_scr(0x17, peek_scr(0x17)|0x20);
		poke_scr(0x15, peek_scr(0x15)|0x80);
	}else{
		/* no pci burst */
		poke_scr(0x17,peek_scr(0x17) & ~0x20);
	}

	/* use 3dx for crtc IO */
	poke8_io(0x3c2,peek8_io(0x3cc)|1);

	/* if memClock is 0,than use BIOS setting */
	if(param->memClock > 0){
		/* get approprite mnr,mdr for memory clock */
		ddk712_calcPllValue(param->memClock,&mnr,&mdr);
		poke_ccr(0x6a,mnr&0xff);
		poke_ccr(0x6b,mdr&0xff);
	}

	if(param->devid == 0x720){
		/* for sm722,always set pdr21 bit 5 to 0*/
		poke_pdr(0x21,peek_pdr(0x21)&~0x20);
	}

	/* Disable LCD framebuffer r/w operation */
	/* enable VPR power gate,VPR is used in frame buffer driver
	 * enable CPR power gate,and DPR power gate
	 * */
	poke_pdr(0x21,(peek_pdr(0x21)|0x30)&0xf8);

	/* Enable DAC defaultly */
	poke_pdr(0x21,peek_pdr(0x21)&0x7f);

	/* power down mode is standby mode, VCLK and MCLK divided by 4*/
	poke_pdr(0x20,(peek_pdr(0x20) & ~0xb0)|0x10);

	/* Disable horizontal expansion and auto centering */
	poke_fpr(0x32,peek_fpr(0x32)&~3);

	/* Disable verticle expansion/vertical centering/horizontal centering */
	poke_crt(0x9e,peek_crt(0x9e) & ~7);
	poke_crt(0x17, peek_crt(0x17) | 0x80);

	/* use vclk1 */
	poke_ccr(0x68,0x54);

	/* Disable panel video */
	poke_fpr(0xa0,0);
	poke_ccr(0x33,0);
	poke_ccr(0x3a,0);

	/* memory control regsiter */
	poke_mcr(0x60,1);

	/* below mcr register should be set by BIOS/GPIO pin */
#if 1
	/* memory bank set */
	poke_mcr(0x61,0);

	/* mcr6 should be set by onboard GPIO pin,but for internal memory 720
	 * only ff is workable,while 0x3e for sm712
	 * */
	if(param->devid == 0x720)
		poke_mcr(0x62,0xff);
	else
		poke_mcr(0x62,0x3e);

	/* MCR 63 is not claimed on datasheet,but experiment shows that sm712 need it be 0x1a
	 * for stable timing */
	poke_mcr(0x63,0x1a);

#endif

	if(param->lcd != LCD712_USE_JUMP){
		poke_fpr(0x30,(peek_fpr(0x30)&0xfe)|param->lcd);
		/* set other related registers*/
	}

	if(param->lcd_color.tftColor != TFT_USE_JUMP){
//		printk("monk:param->lcd_color.tftColor = %d\n",param->lcd_color.tftColor);
		poke_fpr(0x30,(peek_fpr(0x30)&0x8f)|(param->lcd_color.tftColor<<4));
//		printk("monk peek scr 0x30 = %02x\n",peek_fpr(0x30));
		/* set other related registers*/
	}

	if(param->lcd_color.dstnColor != DSTN_USE_JUMP){
		poke_fpr(0x30,(peek_fpr(0x30)&0x7f)|(param->lcd_color.dstnColor <<7));
		/* set other related registers*/
	}
}

