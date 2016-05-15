#include<linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
#include<linux/config.h>
#endif
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

#include <linux/console.h>
#ifdef CONFIG_MTRR
#include <asm/mtrr.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
/* no below two header files in 2.6.9 */
#include<linux/platform_device.h>
#include<linux/screen_info.h>
#else
/* nothing by far */
#endif

#include "lynx_drv.h"
#include "lynx_hw502.h"
#include "lynx_accel.h"

static volatile unsigned char __iomem * mmio502;
#define PEEK32(addr) readl((addr)+mmio502)
#define POKE32(addr,data) writel((data),(addr)+mmio502)

/* IMPORTANT:
 * you must use below two line macro to access 2d registers
 * do not use POKE32 and PEEK32 to access 2d registers
 * because all 2d register address is just an offset from its
 * chip's 2d base address
 * */
#if 0
#define dePOKE32(a,b) POKE32(a+DE_BASE_ADDR_TYPE1,b)
#define dePEEK32(a) PEEK32(a+DE_BASE_ADDR_TYPE1)
#endif

static inline int h_total(struct fb_var_screeninfo * var)
{
	return (var->xres + var->left_margin + var->right_margin + var->hsync_len);
}

static inline int v_total(struct fb_var_screeninfo * var)
{
	return (var->yres + var->upper_margin + var->lower_margin + var->vsync_len);
}

static void set_mmio(volatile unsigned char __iomem * addr)
{
	mmio502 = addr;
}

unsigned long SmRead8(unsigned long nOffset){
	return readb(mmio502 + nOffset);
}

unsigned long SmRead16(unsigned long nOffset){
	return readw(mmio502 + nOffset);
}

unsigned long SmRead32(unsigned long nOffset){
	return readl(mmio502 + nOffset);
}

void SmWrite8(unsigned long nOffset,unsigned long nData){
	writeb(nData,mmio502+nOffset);
}

void SmWrite16(unsigned long nOffset,unsigned long nData){
	writew(nData,mmio502 + nOffset);
}

void SmWrite32(unsigned long nOffset,unsigned long nData){
	writel(nData,mmio502 + nOffset);
}

volatile unsigned char __iomem * SmIoAddress(void)
{
	return mmio502;
}

void SmMemset(unsigned long nOffset,unsigned char val,int cnt){
	memset((void __force *)(mmio502 + nOffset),val,cnt);
}

void sm501_set_gate(unsigned long gate){
	unsigned long mode = SmRead32(POWER_MODE_CTRL);
	switch (mode){
		case 0:
			SmWrite32(POWER_MODE0_GATE,gate);
			break;
		case 1:
			SmWrite32(POWER_MODE1_GATE,gate);
			break;
		default:
			return;
	}
	msleep(16);
}
extern int sm501_unit_power(struct device * dev,unsigned int bit,unsigned int on)
{
	/*
	struct lynx_share * share;
	share = dev_get_drvdata(dev);
	*/
	ENTER();

	u32 reg;
	reg = PEEK32(CURRENT_POWER_GATE);
	reg &= ~(1 << bit);
	reg |= (1 << on);
	POKE32(POWER_MODE0_GATE,reg);
	LEAVE(0);
}

extern int sm501_modify_reg(struct device* dev,unsigned long regoff,unsigned long set,
							unsigned long clear)
{
	u32 reg;
	reg = PEEK32(regoff);
	reg |= set;
	reg &= ~clear;

	POKE32(regoff,reg);
	return reg;
}

EXPORT_SYMBOL_GPL(SmRead8);
EXPORT_SYMBOL_GPL(SmRead16);
EXPORT_SYMBOL_GPL(SmRead32);
EXPORT_SYMBOL_GPL(SmWrite8);
EXPORT_SYMBOL_GPL(SmWrite16);
EXPORT_SYMBOL_GPL(SmWrite32);
EXPORT_SYMBOL_GPL(SmIoAddress);
EXPORT_SYMBOL_GPL(SmMemset);
EXPORT_SYMBOL_GPL(sm501_set_gate);

int hw_sm502_map(struct lynx_share* share,struct pci_dev* pdev)
{
	ENTER();
	share->vidreg_start = pci_resource_start(pdev,1);
	dbg_msg("share->vidreg_start = %08x\n",share->vidreg_start);
	share->vidreg_size = MB(2);

	share->pvReg = ioremap(share->vidreg_start,share->vidreg_size);
	if(!share->pvReg){
		err_msg("mmio failed\n");
		LEAVE(-EFAULT);
	}
	share->accel.dprBase = share->pvReg + DE_BASE_ADDR_TYPE1;
	share->accel.dpPortBase = share->pvReg + DE_PORT_ADDR_TYPE1;

	set_mmio(share->pvReg);

	dbg_msg("share->pvReg = %p, mmio 60 = %08x \n",share->pvReg,PEEK32(0x60));

	share->vidmem_start = pci_resource_start(pdev,0);
	dbg_msg("share->vidmem_start = %08x\n",share->vidmem_start);

	share->vidmem_size = hw_sm502_getVMSize(share);

	inf_msg("memory size = %d\n",(int)share->vidmem_size);

	share->pvMem = ioremap(share->vidmem_start,share->vidmem_size);
	if(!share->pvMem){
		err_msg("Map video memory failed\n");
		LEAVE(-EFAULT);
	}

	LEAVE(0);
}

static unsigned int voyager_calcProgPllCtrl(unsigned int ulRequestClk,
		struct sm502_pll_value * pPLL)
{
	unsigned int M, N, diff, pllClk, i;
	unsigned int bestDiff = ~0; /* biggest 32 bit unsigned number */

	/* Sanity check */
	if (pPLL->inputFreq < MHz(5) || pPLL->inputFreq > MHz(100))
	{
		return 0;
	}

	/* Convert everything in Khz range in order to avoid calculation overflow */
	pPLL->inputFreq /= 1000;
	ulRequestClk /= 1000;

	/* Try both ulRequestClk and (ulRequestClk x 2) to work out the best M & N values. */
	for (i=0; i<2; i++)
	{
		/* N must between 2 and 24, according to design */
		for (N=2; N<25; N++)
		{
			M = ulRequestClk * N * 1000 / pPLL->inputFreq;
			M = roundedDiv(M, 1000);

			pllClk = pPLL->inputFreq * M / N;
			diff = absDiff(pllClk, ulRequestClk);
			//printk("Need=%u,M=%u,N=%u,div2=%u,clk=%d,diff=%u\n",ulRequestClk,M,N,i,pllClk,diff);
			if (diff < bestDiff)
			{
				bestDiff = diff;
				pPLL->M = M;
				pPLL->N = N;
				pPLL->divBy2 = i;
			}
		}
		ulRequestClk *= 2;
	}

	pPLL->inputFreq *= 1000;

//	printk("pPLL->inputFreq = %u,M = %u,N = %u,div2 = %u\n",pPLL->inputFreq,pPLL->M,pPLL->N,pPLL->divBy2);


	return (pPLL->divBy2 == 0)?
			(pPLL->inputFreq * pPLL->M / pPLL->N) :
			(pPLL->inputFreq * pPLL->M / pPLL->N / 2);

}

static unsigned int voyager_calcDividerClock(unsigned int ulRequestClk,
											struct sm502_pll_value *pPLL)
{
    unsigned int pll1, pll2, pllDiff, dividerClk, diff;
    unsigned short divider, divider_limit, shift;
    unsigned int best_diff = ~0; /* biggest 32 bit unsigned number */

    /* Sanity check */
    if (pPLL->inputFreq < MHz(5) || pPLL->inputFreq > MHz(100))
    {
        return 0;
    }

    /* Convert everything in Khz range in order to avoid overflow */
    pPLL->inputFreq /= 1000;
    ulRequestClk /= 1000;

    pll1 = pPLL->inputFreq * 12;
    pll2 = pPLL->inputFreq * 14;
    pllDiff = pll2 - pll1;

    /* For panel clock, try divider 1, 3 and 5, while all others only have 1 and 3 */
    if ( pPLL->clockType == SM502_P1XCLK || pPLL->clockType == SM502_P2XCLK )
        divider_limit = 5;
    else
        divider_limit = 3;

    for(dividerClk = pll1; dividerClk <= pll2; dividerClk += pllDiff)
    {
        for(divider = 1; divider <= divider_limit; divider += 2)
        {
            /* Try all 8 shift values. */
            for(shift = 0; shift < 8; shift++)
            {
                /* Calculate difference with requested clock. */
                diff = absDiff((roundedDiv(dividerClk, divider << shift)), ulRequestClk);

                /* If the difference is less than the current, use it. */
                if(diff < best_diff)
                {
                    /* Store best difference. */
                    best_diff = diff;

                    /*  Store clock values. */
                    pPLL->dividerClk = dividerClk;
                    pPLL->divider = divider;
                    pPLL->shift = shift;
                }
            }
        }
    }

    /* Restore frequency from Khz to hz unit */
    pPLL->dividerClk *= 1000;
    pPLL->inputFreq *= 1000;

    /* Return actual frequency that the PLL can set */
    return pPLL->dividerClk / (pPLL->divider << pPLL->shift);
}


static unsigned int voyager_fmtClkReg(struct sm502_pll_value *pPLL)
{
    unsigned int ulModeClockReg;
    unsigned int ulModeClockField;

    ulModeClockReg = PEEK32(CURRENT_POWER_CLOCK);

    if (pPLL->clockType==SM502_PANEL_PLL)
    {
        /* If use programmable PLL, disable divider clock and
           disable 2X clock.
         */
        ulModeClockReg &= FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_DIS)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, SEL)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_SELECT)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_DIVIDER)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_SHIFT);

        ulModeClockField =
            FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_DIS, 1X)
          | FIELD_SET(0, CURRENT_POWER_CLOCK, SEL, PLL);
    }
    else if (pPLL->clockType==SM502_PANEL_PLL_2X)
    {
        /* If use programmable PLL x 2, disable divider clock.
         */
        ulModeClockReg &= FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_DIS)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, SEL)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_SELECT)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_DIVIDER)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_SHIFT);

        ulModeClockField =
            FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_DIS, NORMAL)
          | FIELD_SET(0, CURRENT_POWER_CLOCK, SEL, PLL);
    }
    else if (pPLL->clockType==SM502_P2XCLK || pPLL->clockType==SM502_P1XCLK)
    {
        /* Use either 1X or 2X panel divider clock */
        ulModeClockReg &= FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_DIS)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, SEL)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_SELECT)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_DIVIDER)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_SHIFT);

        ulModeClockField =
            FIELD_SET(0, CURRENT_POWER_CLOCK, SEL, PXCLK)
          | (pPLL->clockType == SM502_P2XCLK
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_DIS, NORMAL)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_DIS, 1X))
          | (pPLL->dividerClk == (pPLL->inputFreq * 12)
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_SELECT, 12X)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_SELECT, 14X))
          | (pPLL->divider == 1
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_DIVIDER, 1)
            : (pPLL->divider == 3
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_DIVIDER, 3)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_DIVIDER, 5)))
          | FIELD_VALUE(0, CURRENT_POWER_CLOCK, P2XCLK_SHIFT, pPLL->shift);

    }
    else if (pPLL->clockType==SM502_V2XCLK || pPLL->clockType==SM502_V1XCLK)
    {
        /* Use either 1x or 2x CRT divider clock */
        ulModeClockReg &= FIELD_CLEAR(CURRENT_POWER_CLOCK, V2XCLK_DIS)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, V2XCLK_SELECT)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, V2XCLK_DIVIDER)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, V2XCLK_SHIFT);

        ulModeClockField =
            (pPLL->clockType == SM502_V2XCLK
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, V2XCLK_DIS, NORMAL)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, V2XCLK_DIS, 1X))
          | (pPLL->dividerClk == (pPLL->inputFreq * 12)
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, V2XCLK_SELECT, 12X)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, V2XCLK_SELECT, 14X))
          | (pPLL->divider == 1
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, V2XCLK_DIVIDER, 1)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, V2XCLK_DIVIDER, 3))
          | FIELD_VALUE(0, CURRENT_POWER_CLOCK, V2XCLK_SHIFT, pPLL->shift);
    }
    else if (pPLL->clockType==SM502_MXCLK)
    {
        /* 1X Master Clock */
        ulModeClockReg &= FIELD_CLEAR(CURRENT_POWER_CLOCK, MCLK_SELECT)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, MCLK_DIVIDER)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, MCLK_SHIFT);

        ulModeClockField =
             (pPLL->dividerClk == (pPLL->inputFreq * 12)
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, MCLK_SELECT, 12X)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, MCLK_SELECT, 14X))
          | (pPLL->divider == 1
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, MCLK_DIVIDER, 1)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, MCLK_DIVIDER, 3))
          | FIELD_VALUE(0, CURRENT_POWER_CLOCK, MCLK_SHIFT, pPLL->shift);
    }
    else
    {
        /* 1X Memroy Clock */
        ulModeClockReg &= FIELD_CLEAR(CURRENT_POWER_CLOCK, M2XCLK_SELECT)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, M2XCLK_DIVIDER)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, M2XCLK_SHIFT);

        ulModeClockField =
             (pPLL->dividerClk == (pPLL->inputFreq * 12)
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, M2XCLK_SELECT, 12X)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, M2XCLK_SELECT, 14X))
          | (pPLL->divider == 1
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, M2XCLK_DIVIDER, 1)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, M2XCLK_DIVIDER, 3))
          | FIELD_VALUE(0, CURRENT_POWER_CLOCK, M2XCLK_SHIFT, pPLL->shift);
    }

    return(ulModeClockReg | ulModeClockField);
}

static inline void voyager_setCurrentGate(unsigned int gate)
{
	unsigned int gate_reg;
#if 0
	unsigned int select;
    select = FIELD_GET(PEEK32(POWER_MODE_CTRL),
			      POWER_MODE_CTRL,
			      MODE);
    switch (select)
    {
       case POWER_MODE_CTRL_MODE_MODE0:
	    gate_reg = POWER_MODE0_GATE;
	    break;

       case POWER_MODE_CTRL_MODE_MODE1:
	    gate_reg = POWER_MODE1_GATE;
	    break;

       case POWER_MODE_CTRL_MODE_SLEEP:
       default:
	    return; /* Nothing to set in sleep mode */
	}
#else
	gate_reg = POWER_MODE0_GATE;
#endif
	POKE32(gate_reg,gate);
}

static inline void voyager_setCurrentClock(unsigned int clock)
{
    unsigned int clock_reg;
#if 0
    unsigned int control_value = 0;
    /* Get current power mode. */
    control_value = FIELD_GET(PEEK32(POWER_MODE_CTRL),
			      POWER_MODE_CTRL,
			      MODE);

    switch (control_value)
    {
       case POWER_MODE_CTRL_MODE_MODE0:
	    clock_reg = POWER_MODE0_CLOCK;
	    break;

       case POWER_MODE_CTRL_MODE_MODE1:
	    clock_reg = POWER_MODE1_CLOCK;
	    break;

       case POWER_MODE_CTRL_MODE_SLEEP:
       default:
	    return; /* Nothing to set in sleep mode */
	}
#else
	/* kernel driver always use power mode 0,so cut above crap*/
	clock_reg = POWER_MODE0_CLOCK;
#endif
    POKE32(clock_reg, clock);
}



static void voyager_set_clock(enum sm502_clocktype clksrc,unsigned int req_freq)
{
	unsigned int actualClk;
#if (DEBUG >= 1)
	static const char * clkNames[] = {
			[SM502_P2XCLK] = "PANEL CONTROLLER CLOCK",
			[SM502_V2XCLK] = "CRT CONTROLLER CLOCK",
			[SM502_MXCLK] = "MAIN ENGINE CLOCK",
			[SM502_M1XCLK] = "SDRAM CONTROLLER CLOCK",
			[SM502_P1XCLK] = "x1 panel divider clock",
			[SM502_V1XCLK] = "x1 CRT divider clock",
			[SM502_PANEL_PLL] = "Programmable panel pixel clock",
			[SM502_PANEL_PLL_2X] = "For TV, 2X clock is needed",
		};
#endif
	struct sm502_pll_value pll;
	ENTER();
	dbg_msg("Request %s clock to %u hz.\n",clkNames[clksrc],req_freq);

	if(req_freq){
		u32 tmp;
		memset(&pll,0,sizeof(struct sm502_pll_value));
		pll.clockType = clksrc;
		pll.inputFreq = DEFAULT_INPUT_CLOCK;

		switch (clksrc) {
			case SM502_PANEL_PLL:
				actualClk = voyager_calcProgPllCtrl(req_freq,&pll);
				voyager_setCurrentClock(voyager_fmtClkReg(&pll));
#if 1
				/* for programable pll,need ox74 be set */
				tmp =
				FIELD_SET(0,PROGRAMMABLE_PLL_CONTROL,TSTOE,DISABLE)|
				FIELD_SET(0,PROGRAMMABLE_PLL_CONTROL,PON,ON)|
				FIELD_SET(0,PROGRAMMABLE_PLL_CONTROL,SEL,CRYSTAL)|
				FIELD_VALUE(0,PROGRAMMABLE_PLL_CONTROL,M,pll.M)|
				FIELD_VALUE(0,PROGRAMMABLE_PLL_CONTROL,N,pll.N)|
				FIELD_VALUE(0,PROGRAMMABLE_PLL_CONTROL,K,pll.divBy2);
				POKE32(PROGRAMMABLE_PLL_CONTROL,tmp);
#endif
				break;
			case SM502_V1XCLK:
			case SM502_MXCLK:
			case SM502_M1XCLK:
				actualClk = voyager_calcDividerClock(req_freq,&pll);
				/* set current clock  */
				voyager_setCurrentClock(voyager_fmtClkReg(&pll));
				break;
			default:
				LEAVE();
		}

		dbg_msg("actual %s clock ==> %u hz.\n",clkNames[clksrc],actualClk);
	}
	LEAVE();
}

int hw_sm502_inithw(struct lynx_share* share,struct pci_dev * pdev)
{
	int ret;
	ushort mclk,m1xclk;
	unsigned int reg;
	struct sm502_share * spec_share;
	struct init_status502 * parm;

	ENTER();

	ret = 0;
	reg = PEEK32(DRAM_CTRL);
	spec_share = container_of(share,struct sm502_share,share);
	parm = &spec_share->state.initParm;

	/* for easy hardware setting,always use mode0*/
	POKE32(0x54,0);

	if(share->vidmem_size == MB(4)){
		/* for sm107,use banks 2*/
		reg = FIELD_SET(reg,DRAM_CTRL,CPU_BANKS,2);
	}else{
		reg = FIELD_SET(reg,DRAM_CTRL,CPU_BANKS,4);
	}

#ifdef CONFIG_SYS_HAS_CPU_LOONGSON2
	reg = FIELD_SET(reg,DRAM_CTRL,LOCAL,RESET);
#endif
	POKE32(DRAM_CTRL,reg);

#ifdef CONFIG_SYS_HAS_CPU_LOONGSON2
	reg = FIELD_SET(reg,DRAM_CTRL,LOCAL,NORMAL);
	POKE32(DRAM_CTRL,reg);
#endif

	/* set TFT Panel Type */
	switch(spec_share->state.pnltype){
		/* the enum value are designed to equal to the register value */
		case sm502_9TFT:
		case sm502_12TFT:
		case sm502_24TFT:
		POKE32(PANEL_DISPLAY_CTRL,
		FIELD_VALUE(PEEK32(PANEL_DISPLAY_CTRL),
					PANEL_DISPLAY_CTRL,TFT,spec_share->state.pnltype));
		break;
	}

	/* does user need CRT ?*/
	if(spec_share->state.nocrt){
		POKE32(MISC_CTRL,FIELD_SET(PEEK32(MISC_CTRL),MISC_CTRL,DAC_POWER,DISABLE));
		/* shut off dpms */
		POKE32(SYSTEM_CTRL,
				FIELD_SET(PEEK32(SYSTEM_CTRL),SYSTEM_CTRL,DPMS,VNHN));
		dbg_msg("read SYSTEM ctrl (nocrt) == %08x\n",PEEK32(SYSTEM_CTRL));
	}else{
		POKE32(MISC_CTRL,FIELD_SET(PEEK32(MISC_CTRL),MISC_CTRL,DAC_POWER,ENABLE));
		/* turn on dpms */
		POKE32(SYSTEM_CTRL,
				FIELD_SET(PEEK32(SYSTEM_CTRL),SYSTEM_CTRL,DPMS,VPHP));
		dbg_msg("read SYSTEM ctrl (crt) == %08x\n",PEEK32(SYSTEM_CTRL));
	}


	/* defaulty enable pci burst read and write */
	reg = PEEK32(SYSTEM_CTRL);
	POKE32(SYSTEM_CTRL,FIELD_SET(reg,SYSTEM_CTRL,PCI_BURST,ENABLE)|
#if 0
			FIELD_SET(reg,SYSTEM_CTRL,PCI_BURST_READ,ENABLE)|
			FIELD_SET(reg,SYSTEM_CTRL,PCI_RETRY,ENABLE)|
			FIELD_SET(reg,SYSTEM_CTRL,PCI_CLOCK_RUN,ENABLE)|
			FIELD_SET(reg,SYSTEM_CTRL,PCI_RETRY,ENABLE)
#endif
			0);

	/* defaulty open dac power */
	reg = PEEK32(MISC_CTRL);
	POKE32(MISC_CTRL,FIELD_SET(reg,MISC_CTRL,DAC_POWER,ENABLE));

#if 0
	/* find below line make system die deadly*/
	/* shut up all none-graphic-related power gate*/
	voyager_setCurrentGate(6);
#endif
	mclk = parm->master_clk;
	m1xclk = parm->mem_clk;

	if(mclk < 80){
		//inf_msg("set voyager engine clock to %d mhz\n",mclk);
		voyager_set_clock(SM502_MXCLK,MHZ(mclk));
	}

	if(m1xclk < 150){
		//inf_msg("set voyager sdram clock to %d mhz\n",m1xclk);
		voyager_set_clock(SM502_M1XCLK,MHZ(m1xclk));
	}

	if(!share->accel_off){
		hw_sm502_initAccel(&share->accel);
	}

	LEAVE(ret);
}

resource_size_t hw_sm502_getVMSize(struct lynx_share * share)
{
	int idx;
	static resource_size_t sm501_mem_local[] = {
		[0] = MB(4),
		[1] = MB(8),
		[2] = MB(16),
		[3] = MB(32),
		[4] = MB(64),
		[5] = MB(2),
	};
	ENTER();
	idx = FIELD_GET(PEEK32(DRAM_CTRL),DRAM_CTRL,SIZE);
	LEAVE(sm501_mem_local[idx]);
}

int hw_sm502_output_checkMode(struct lynxfb_output* output,struct fb_var_screeninfo* var)
{
#if 0
	ENTER();
	LEAVE(0);
#else
	return 0;
#endif

}


static void voyager_wait_panelvsync(int times)
{
#if 0
	while(times--)
	{;}
	return;
#else
	unsigned int status;
	while(times -- > 0){
		/* wait for end of vsync */
		do{
			status = FIELD_GET(PEEK32(CMD_INTPR_STATUS),
							CMD_INTPR_STATUS,
							PANEL_SYNC);
		}while(status == CMD_INTPR_STATUS_PANEL_SYNC_ACTIVE);

		/* wait for start of vsync */
		do{
			status = FIELD_GET(PEEK32(CMD_INTPR_STATUS),
							CMD_INTPR_STATUS,
							PANEL_SYNC);
		}while(status == CMD_INTPR_STATUS_PANEL_SYNC_INACTIVE);
	}
#endif
}

static void voyager_panel_power(int open,int times)
{
	unsigned int reg = PEEK32(PANEL_DISPLAY_CTRL);

	voyager_wait_panelvsync(4);
	if(open != 0){
		/* turn on fpvdden */
		reg = FIELD_SET(reg,
				PANEL_DISPLAY_CTRL,FPVDDEN,HIGH);
		POKE32(PANEL_DISPLAY_CTRL,reg);
		voyager_wait_panelvsync(times);

		/* turn on fpdata */
		reg = FIELD_SET(reg,
				PANEL_DISPLAY_CTRL,DATA,ENABLE);
		POKE32(PANEL_DISPLAY_CTRL,reg);
		voyager_wait_panelvsync(times);

		/* turn on fpvbias */
		reg = FIELD_SET(reg,
				PANEL_DISPLAY_CTRL,VBIASEN,HIGH);
		POKE32(PANEL_DISPLAY_CTRL,reg);
		voyager_wait_panelvsync(times);

		/* turn on fpen */
		reg = FIELD_SET(reg,
				PANEL_DISPLAY_CTRL,FPEN,HIGH);
		POKE32(PANEL_DISPLAY_CTRL,reg);
		voyager_wait_panelvsync(times);

	}else{
		/* turn off fpvdden */
		reg = FIELD_SET(reg,
				PANEL_DISPLAY_CTRL,FPVDDEN,LOW);
		POKE32(PANEL_DISPLAY_CTRL,reg);
		voyager_wait_panelvsync(times);

		/* turn off fpdata */
		reg = FIELD_SET(reg,
				PANEL_DISPLAY_CTRL,DATA,DISABLE);
		POKE32(PANEL_DISPLAY_CTRL,reg);
		voyager_wait_panelvsync(times);

		/* turn off fpvbias */
		reg = FIELD_SET(reg,
				PANEL_DISPLAY_CTRL,VBIASEN,LOW);
		POKE32(PANEL_DISPLAY_CTRL,reg);
		voyager_wait_panelvsync(times);

		/* turn off fpen */
		reg = FIELD_SET(reg,
				PANEL_DISPLAY_CTRL,FPEN,LOW);
		POKE32(PANEL_DISPLAY_CTRL,reg);
		voyager_wait_panelvsync(times);
	}
}

int hw_sm502_output_setMode(struct lynxfb_output* output,
							struct fb_var_screeninfo* var,
							struct fb_fix_screeninfo* fix)
{
	unsigned int reg;
	ENTER();
	//if(output->paths & sm502_panel)
	{
		reg = PEEK32(PANEL_DISPLAY_CTRL);
		reg = FIELD_SET(reg,PANEL_DISPLAY_CTRL,TIMING,ENABLE);
		reg = FIELD_SET(reg,PANEL_DISPLAY_CTRL,PLANE,ENABLE);
		reg |= 0x0f000000;
		POKE32(PANEL_DISPLAY_CTRL,reg);
		//voyager_panel_power(1,4);/* open panel sequence */
	}

	//if(output->paths & sm502_crt)
	{
		reg = PEEK32(CRT_DISPLAY_CTRL);
		reg = FIELD_VALUE(reg,CRT_DISPLAY_CTRL,SELECT,*output->channel);
		reg = FIELD_SET(reg,CRT_DISPLAY_CTRL,TIMING,ENABLE);
		reg = FIELD_SET(reg,CRT_DISPLAY_CTRL,PLANE,ENABLE);
		POKE32(CRT_DISPLAY_CTRL,reg);
	}
	LEAVE(0);
}

int hw_sm502_crtc_checkMode(struct lynxfb_crtc* crtc,struct fb_var_screeninfo* var)
{
	ENTER();
	/* video memory check move to upper routine */
	if(var->bits_per_pixel ==8 ||
		var->bits_per_pixel == 16 ||
		var->bits_per_pixel == 32)
	{}
	else
		LEAVE(-EINVAL);

	LEAVE(0);
}

static void set_bpp(enum sm502_channel ctrl,int bpp)
{
	unsigned int reg;
	if(ctrl == sm502_primary ){
		reg = PEEK32(PANEL_DISPLAY_CTRL);
		switch(bpp){
			case 8:
			case 16:
			case 32:
				POKE32(PANEL_DISPLAY_CTRL,
					FIELD_VALUE(reg,PANEL_DISPLAY_CTRL,FORMAT,bpp>>4));
				break;
		}

	}else{
		reg = PEEK32(CRT_DISPLAY_CTRL);
		switch(bpp){
			case 8:
			case 16:
			case 32:
				POKE32(CRT_DISPLAY_CTRL,
					FIELD_VALUE(reg,CRT_DISPLAY_CTRL,FORMAT,bpp>>4));
				break;
		}
	}

}

static void set_timing(enum sm502_channel ctrl,struct fb_var_screeninfo * var)
{
	unsigned int reg,base;

	if(ctrl == sm502_primary){
		/* for panel channel,set its paning and rectangle value */
		POKE32(PANEL_PAN_CTRL,0);
		POKE32(PANEL_PLANE_TL,0);
		POKE32(PANEL_PLANE_BR,(var->xres -1)|(var->yres -1)<<16);
		base = PANEL_HORIZONTAL_TOTAL;
	}else{
		base = CRT_HORIZONTAL_TOTAL;
	}

	/* program horizontal total and disp_end */
	reg = FIELD_VALUE(0,PANEL_HORIZONTAL_TOTAL,TOTAL,h_total(var) - 1)|
 			FIELD_VALUE(0,PANEL_HORIZONTAL_TOTAL,DISPLAY_END,var->xres - 1);

	POKE32(base + VOYAGER_OFF_DC_H_TOT,reg);

	/* program horizontal sync START and WIDTH */
	reg = FIELD_VALUE(0,PANEL_HORIZONTAL_SYNC,WIDTH,var->hsync_len)|
			FIELD_VALUE(0,PANEL_HORIZONTAL_SYNC,START,var->xres + var->right_margin - 1);
	POKE32(base + VOYAGER_OFF_DC_H_SYNC,reg);

	/* program vertical total and disp_end */
	reg = FIELD_VALUE(0,PANEL_VERTICAL_TOTAL,TOTAL,v_total(var) -1)|
			FIELD_VALUE(0,PANEL_VERTICAL_TOTAL,DISPLAY_END,var->yres - 1);

	POKE32(base + VOYAGER_OFF_DC_V_TOT,reg);

	/* program vertical sync START and WIDTH */
	reg = FIELD_VALUE(0,PANEL_VERTICAL_SYNC,HEIGHT,var->vsync_len)|
			FIELD_VALUE(0,PANEL_VERTICAL_SYNC,START,var->yres + var->lower_margin -1);

	POKE32(base + VOYAGER_OFF_DC_V_SYNC,reg);

}


int hw_sm502_crtc_setMode(struct lynxfb_crtc* crtc,
							struct fb_var_screeninfo* var,
							struct fb_fix_screeninfo* fix)
{
	unsigned int reg,tmp,pixclk;
	int fmt;
	struct lynx_share * share;
	struct lynxfb_par * par;
	ENTER();

	pixclk = ps_to_hz(var->pixclock);
	/* set timing first and color format */
	set_timing(crtc->channel,var);
	set_bpp(crtc->channel,var->bits_per_pixel);

	par = container_of(crtc,struct lynxfb_par,crtc);
	share = par->share;

#if 1
	if(!share->accel_off){
		/* set 2d engine pixel format according to mode bpp */
		switch(var->bits_per_pixel){
			case 8:
				fmt = 0;
				break;
			case 16:
				fmt = 1;
				break;
			case 32:
			default:
				fmt = 2;
				break;
		}
		hw_set2dformat(&share->accel,fmt);
	}
#endif

#if 1
	/* set start addr,width,offset,etc... */
	if(crtc->channel == sm502_primary){
		/* set pixel clock */
		voyager_set_clock(SM502_PANEL_PLL,pixclk);


		/* set fb address */
		POKE32(PANEL_FB_ADDRESS,crtc->oScreen);

		/* set windows position and globle width in unit of pixel/line */
		reg = FIELD_VALUE(0,PANEL_WINDOW_WIDTH,X,var->xoffset)|
				FIELD_VALUE(0,PANEL_WINDOW_WIDTH,WIDTH,var->xres_virtual);

		POKE32(PANEL_WINDOW_WIDTH,reg);

		reg = FIELD_VALUE(0,PANEL_WINDOW_HEIGHT,Y,var->yoffset)|
				FIELD_VALUE(0,PANEL_WINDOW_HEIGHT,HEIGHT,var->yres_virtual);
		POKE32(PANEL_WINDOW_HEIGHT,reg);

		/* set fb width and offset */
		tmp = PADDING(crtc->line_pad,
				var->xres * (var->bits_per_pixel>>3));
		reg = FIELD_VALUE(0,PANEL_FB_WIDTH,WIDTH,tmp)|
				FIELD_VALUE(0,PANEL_FB_WIDTH,OFFSET,fix->line_length);

		POKE32(PANEL_FB_WIDTH,reg);
	}else{
		/* set pixel clock */
		voyager_set_clock(SM502_V1XCLK,pixclk);

		/* set fb address */
		POKE32(CRT_FB_ADDRESS,crtc->oScreen);

		/* set fb width and offset */
		tmp = PADDING(crtc->line_pad,
				var->xres * (var->bits_per_pixel>>3));
		reg = FIELD_VALUE(0,CRT_FB_WIDTH,WIDTH,tmp)|
				FIELD_VALUE(0,CRT_FB_WIDTH,OFFSET,fix->line_length);

		POKE32(CRT_FB_WIDTH,reg);
	}
#endif
	LEAVE(0);
}


int hw_sm502_setColReg(struct lynxfb_crtc* crtc,ushort index,
							ushort red,ushort green,ushort blue)
{
	static unsigned int add[]={PANEL_PALETTE_RAM,CRT_PALETTE_RAM};
	POKE32(add[crtc->channel] + index*4 ,(red<<16)|(green<<8)|blue);
	return 0;
}


int hw_sm502_setBLANK(struct lynxfb_output* output,int blank)
{
	unsigned int dpms,pps,crtdb;
	ENTER();
	dpms = pps = crtdb =0;
	switch(blank)
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
		case FB_BLANK_UNBLANK:
#else
		case VESA_NO_BLANKING:
#endif
			dpms = SYSTEM_CTRL_DPMS_VPHP;
			pps = PANEL_DISPLAY_CTRL_DATA_ENABLE;
			crtdb = CRT_DISPLAY_CTRL_BLANK_OFF;
			break;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
		case FB_BLANK_NORMAL:
			dpms = SYSTEM_CTRL_DPMS_VPHP;
			pps = PANEL_DISPLAY_CTRL_DATA_DISABLE;
			crtdb = CRT_DISPLAY_CTRL_BLANK_ON;
			break;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
		case FB_BLANK_VSYNC_SUSPEND:
#else
		case VESA_VSYNC_SUSPEND:
#endif
			dpms = SYSTEM_CTRL_DPMS_VNHP;
			pps = PANEL_DISPLAY_CTRL_DATA_DISABLE;
			crtdb = CRT_DISPLAY_CTRL_BLANK_ON;
			break;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
		case FB_BLANK_HSYNC_SUSPEND:
#else
		case VESA_HSYNC_SUSPEND:
#endif
			dpms = SYSTEM_CTRL_DPMS_VPHN;
			pps = PANEL_DISPLAY_CTRL_DATA_DISABLE;
			crtdb = CRT_DISPLAY_CTRL_BLANK_ON;
			break;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
		case FB_BLANK_POWERDOWN:
#else
		case VESA_POWERDOWN:
#endif
			dpms = SYSTEM_CTRL_DPMS_VNHN;
			pps = PANEL_DISPLAY_CTRL_DATA_DISABLE;
			crtdb = CRT_DISPLAY_CTRL_BLANK_ON;
			break;
	}

	POKE32(SYSTEM_CTRL,FIELD_VALUE(PEEK32(SYSTEM_CTRL),SYSTEM_CTRL,DPMS,dpms));
	POKE32(CRT_DISPLAY_CTRL,FIELD_VALUE(PEEK32(CRT_DISPLAY_CTRL),CRT_DISPLAY_CTRL,BLANK,pps));
	POKE32(PANEL_DISPLAY_CTRL,FIELD_VALUE(PEEK32(PANEL_DISPLAY_CTRL),PANEL_DISPLAY_CTRL,DATA,pps));
	POKE32(CRT_DISPLAY_CTRL,FIELD_VALUE(PEEK32(CRT_DISPLAY_CTRL),
				CRT_DISPLAY_CTRL,BLANK,crtdb));
	LEAVE(0);
}

void hw_sm502_crtc_clear(struct lynxfb_crtc* crtc)
{

}

void hw_sm502_output_clear(struct lynxfb_output* output)
{

}

void hw_sm502_initAccel(struct lynx_accel * accel)
{
	u32 reg;
	/* enable DE engine */
	reg = FIELD_SET(PEEK32(CURRENT_POWER_GATE),CURRENT_POWER_GATE,2D,ENABLE);
	POKE32(POWER_MODE0_GATE,reg);

	/* engine reset */
	reg = PEEK32(SYSTEM_CTRL);
    reg = FIELD_SET(reg,SYSTEM_CTRL,DE_ABORT,2D_ABORT);
	POKE32(SYSTEM_CTRL,reg);

	reg = PEEK32(SYSTEM_CTRL);
	reg = FIELD_SET(reg,SYSTEM_CTRL,DE_ABORT,NORMAL);
	POKE32(SYSTEM_CTRL,reg);

	/* move 2d engine register setting into accel functions */
	accel->de_init(accel);
#if 0
	/* setup 2d engine registers */

	dePOKE32(DE_MASKS,0xFFFFFFFF);

	reg = FIELD_SET(0,DE_STRETCH_FORMAT,PATTERN_XY,NORMAL)|
			FIELD_VALUE(0,DE_STRETCH_FORMAT,PATTERN_Y,0)|
			FIELD_VALUE(0,DE_STRETCH_FORMAT,PATTERN_X,0)|
			FIELD_SET(0,DE_STRETCH_FORMAT,ADDRESSING,XY)|
			FIELD_VALUE(0,DE_STRETCH_FORMAT,SOURCE_HEIGHT,3);

	clr = FIELD_CLEAR(DE_STRETCH_FORMAT,PATTERN_XY)&
			FIELD_CLEAR(DE_STRETCH_FORMAT,PATTERN_Y)&
			FIELD_CLEAR(DE_STRETCH_FORMAT,PATTERN_X)&
			FIELD_CLEAR(DE_STRETCH_FORMAT,ADDRESSING)&
			FIELD_CLEAR(DE_STRETCH_FORMAT,SOURCE_HEIGHT);

	/* DE_STRETCH bpp format need be initilized in setMode routine */
	dePOKE32(DE_STRETCH_FORMAT,(dePEEK32(DE_STRETCH_FORMAT) & clr) | reg);

	/* disable clipping and transparent */
	dePOKE32(DE_CLIP_TL,0);
	dePOKE32(DE_CLIP_BR,0);
	dePOKE32(DE_COLOR_COMPARE_MASK,0);
	dePOKE32(DE_COLOR_COMPARE,0);

	reg = FIELD_SET(0,DE_CONTROL,TRANSPARENCY,DISABLE)|
			FIELD_SET(0,DE_CONTROL,TRANSPARENCY_MATCH,OPAQUE)|
			FIELD_SET(0,DE_CONTROL,TRANSPARENCY_SELECT,SOURCE);

	clr = FIELD_CLEAR(DE_CONTROL,TRANSPARENCY)&
			FIELD_CLEAR(DE_CONTROL,TRANSPARENCY_MATCH)&
			FIELD_CLEAR(DE_CONTROL,TRANSPARENCY_SELECT);

	dePOKE32(DE_CONTROL,(dePEEK32(DE_CONTROL)&clr)|reg);
#endif
}


/*
 * return 0:engine is idle
 * return none-zero: engine always not in idle status
 * */
int hw_sm502_deWait()
{
	int i=0x100000;
	while(i--){
		/*
		 * use CMD_INTPR to check will loop forever
		unsigned int dwVal = PEEK32(SYSTEM_CTRL);
		if((FIELD_GET(dwVal,CMD_INTPR_STATUS,2D_ENGINE) == CMD_INTPR_STATUS_2D_ENGINE_IDLE) &&
			(FIELD_GET(dwVal,CMD_INTPR_STATUS,2D_FIFO) == CMD_INTPR_STATUS_2D_FIFO_EMPTY) &&
			(FIELD_GET(dwVal,CMD_INTPR_STATUS,2D_SETUP) == CMD_INTPR_STATUS_2D_SETUP_IDLE) &&
			(FIELD_GET(dwVal,CMD_INTPR_STATUS,2D_MEMORY_FIFO) == CMD_INTPR_STATUS_2D_MEMORY_FIFO_EMPTY))
			*/
		unsigned int dwVal = PEEK32(SYSTEM_CTRL);
		if((FIELD_GET(dwVal,SYSTEM_CTRL,DE_STATUS) == SYSTEM_CTRL_DE_STATUS_IDLE) &&
			(FIELD_GET(dwVal,SYSTEM_CTRL,DE_FIFO)  == SYSTEM_CTRL_DE_FIFO_EMPTY))
		{
			return 0;
		}
	}
	/* timeout error */
	return -1;
}

