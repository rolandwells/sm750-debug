#include "ddk750_common.h"

#define SCALE_CONSTANT                      (1 << 12)
/* Maximum panel size scaling */
#define MAX_PANEL_SIZE_WIDTH                1920
#define MAX_PANEL_SIZE_HEIGHT               1440

#if 1
static mode_parameter_t gModeParamTable[] =
{
	/* 320 x 240 */
	{ 352, 320, 335,  8, NEG, 265, 240, 254, 2, NEG,  5600000, 15909, 60, NEG},

	/* 400 x 300 */
	{ 528, 400, 420, 64, NEG, 314, 300, 301, 2, NEG,  9960000, 18864, 60, NEG},

	/* 480 x 272 */
	{ 525, 480, 482, 41, NEG, 286, 272, 274, 10, NEG, 9009000, 17160, 60, NEG},

	/* 640 x 480  [4:3] */
	{ 800, 640, 656, 96, NEG, 525, 480, 490, 2, NEG, 25175000, 31500, 60, NEG},
	{ 840, 640, 680, 64, NEG, 500, 480, 481, 3, NEG, 31500000, 37500, 75, NEG},
	{ 832, 640, 700, 56, NEG, 509, 480, 481, 3, NEG, 36000000, 43269, 85, NEG},

	/* 720 x 480  [3:2] */
	{ 889, 720, 738,108, POS, 525, 480, 490, 2, NEG, 28000000, 31496, 60, NEG},

	/* 720 x 540  [4:3] */
	{ 886, 720, 740, 96, POS, 576, 540, 545, 2, POS, 30600000, 34537, 60, NEG},

	/* 800 x 480  [5:3] */
	{ 973, 800, 822, 56, POS, 524, 480, 490, 2, NEG, 30600000, 31449, 60, NEG},

	/* 800 x 600  [4:3] */
	{1062, 800, 840,128, POS, 628, 600, 601, 4, POS, 40000000, 37665, 60, NEG},
	{1056, 800, 816, 80, POS, 625, 600, 601, 3, POS, 49500000, 46875, 75, NEG},
	{1054, 800, 842, 64, POS, 625, 600, 601, 3, POS, 56000000, 53131, 85, NEG},

	/* 960 x 720  [4:3] */
	{1245, 960, 992, 64, POS, 750, 720, 721, 3, POS, 56000000, 44980, 60, NEG},

	/* 1024 x 600  [16:9] 1.7 */
	{1313,1024,1064,104, POS, 622, 600, 601, 3, POS, 49000000, 37319, 60, NEG},

	/* 1024 x 768  [4:3] */
	{1340,1024,1060,136, NEG, 809, 768, 772, 6, NEG, 65000000, 48507, 60, NEG},
	{1337,1024,1072, 96, NEG, 808, 768, 780, 3, NEG, 81000000, 60583, 75, POS},
	{1376,1024,1070, 96, POS, 808, 768, 769, 3, POS, 94500000, 68677, 85, POS},

	/* 1152 x 864  [4:3] */
	{1475,1152,1208, 96, NEG, 888, 864, 866, 3, NEG, 78600000, 53288, 60, POS},

	/* 1280 x 720 [16:9] */
	{1664,1280,1336,136, POS, 746, 720, 721, 3, POS, 74481000, 44760, 60, POS},

	/* 1280 x 768  [5:3] */
	{1678,1280,1350,136, POS, 795, 768, 769, 3, POS, 80000000, 47676, 60, POS},

	/* 1280 x 800  [8:5] */
	{1650,1280,1344,136, NEG, 824, 800, 800, 3, NEG, 81600000, 49455, 60, POS},

	/* 1280 x 960  [4:3] */
	{1618,1280,1330, 96, NEG, 977, 960, 960, 2, NEG, 94500000, 59259, 60, POS},

	/* 1280 x 1024 [5:4] */
	/*{1626,1280,1332,112, POS,1046,1024,1024, 2, POS,102000000, 62731, 60, POS}, */
	{1688,1280,1328,112, POS,1066,1024,1025, 3, POS,108000000, 63981, 60, POS},

	/* 1360 x 768 [16:9] */
	{1776,1360,1424,144, POS, 795, 768, 769, 3, POS, 84715000, 47700, 60, POS},

	/* 1366 x 768  [16:9] */
	{1722,1366,1424,112, NEG, 784, 768, 769, 3, NEG, 81000000, 47038, 60, POS},

	/* 1440 x 900 */
	{1904,1440,1520,152, NEG, 932, 900, 901, 3, POS,106470000, 55919, 60, POS},

	/* 1440 x 960 [16:9] */
	{1920,1440,1528,152, POS, 994, 960, 961, 3, POS,114509000, 59640, 60, POS},

	/* 1600 x 1200 [4:3]. */
	{2160,1600,1664,192, POS,1250,1200,1201, 3, POS,162000000, 75000, 60, POS},
	{2160,1600,1664,192, POS,1250,1200,1201, 3, POS,202500000, 93750, 75, POS},
	{2160,1600,1664,192, POS,1250,1200,1201, 3, POS,229500000,106250, 85, POS},

	/* 
	 * The timing below is taken from the www.tinyvga.com/vga-timing.
	 * With the exception of 1920x1080.
	 */

	/* 1680 x 1050 [8:5]. */ 
	{2256,1680,1784,184, NEG,1087,1050,1051, 3, POS,147140000, 65222, 60, POS},

	/* 1792 x 1344 [4:3]. */ 
	{2448,1792,1920,200, NEG,1394,1344,1345, 3, POS,204800000, 83660, 60, POS},
	{2456,1792,1888,216, NEG,1417,1344,1345, 3, POS,261000000,106270, 75, POS},

	/* 1856 x 1392 [4:3]. */
	{2528,1856,1952,224, NEG,1439,1392,1393, 3, POS,218300000, 86353, 60, POS},
	/* {2560,1856,1984,224, NEG,1500,1392,1393, 3, POS,288000000,112500, 75, POS}, */

	/* 1920 x 1080 [16:9]. This is a make-up value, need to be proven. 
	   The Pixel clock is calculated based on the maximum resolution of
	   "Single Link" DVI, which support a maximum 165MHz pixel clock.
	   The second values are taken from:
http://www.tek.com/Measurement/App_Notes/25_14700/eng/25W_14700_3.pdf
*/
	// {2560,1920,2048,208, NEG,1125,1080,1081, 3, POS,172800000, 67500, 60, POS},
	{2200,1920,2008, 44, NEG,1125,1080,1081, 3, POS,148500000, 67500, 60, POS},

	/* 1920 x 1200 [8:5]. */
	{2592,1920,2048,208, NEG,1242,1200,1201, 3, POS,193160000, 74522, 60, POS},

	/* 1920 x 1440 [4:3]. Do we need to adjust if using double pixel?? */
	/* In the databook, it mentioned only support up to 1920x1440 @ 60Hz. 
	   The timing for 75 Hz is provided here if necessary to do testing. */
	{2600,1920,2048,208, NEG,1500,1440,1441, 3, POS,234000000, 90000, 60, POS},
	/* {2640,1920,2064,224, NEG,1500,1440,1441, 3, POS,297000000,112500, 75, POS}, */

	/* End of table. */
	{ 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG},
};
#endif

long ddk750_setCustomMode(void * ufb,logicalMode_t *pLogicalMode, mode_parameter_t *pUserModeParam)
{
	pll_value_t pll;
	unsigned long ulActualPixelClk, ulTemp;
	/*
	 * Set up PLL, a structure to hold the value to be set in clocks.
	 */
	pll.inputFreq = DEFAULT_INPUT_CLOCK; /* Defined in CLOCK.H */

	/* Get the Clock Type */
	pll.clockType = getClockType(pLogicalMode->dispCtrl);
	ulActualPixelClk = calcPllValue(pUserModeParam->pixel_clock, &pll);

	if (pLogicalMode->pitch == 0)
	{
		ulTemp = (pLogicalMode->x + 15) & ~15; /* This calculation has no effect on 640, 800, 1024 and 1280. */
		pLogicalMode->pitch = ulTemp * (pLogicalMode->bpp / 8);
	}

	/* Program the hardware to set up the mode. */
	programModeRegisters( 
			ufb,
			pUserModeParam,
			pLogicalMode->bpp, 
			pLogicalMode->baseAddress, 
			pLogicalMode->pitch, 
			&pll);	
	return(0);
}


#if 0
/* 
 * Program the hardware for a specific video mode
 */
void programModeRegisters(
		mode_parameter_t *pModeParam,   /* mode information about pixel clock, horizontal total, etc. */
		unsigned long ulBpp,            /* Color depth for this mode */
		unsigned long ulBaseAddress,    /* Offset in frame buffer */
		unsigned long ulPitch,          /* Mode pitch value in byte: no of bytes between two lines. */
		pll_value_t *pPLL               /* Pre-calculated values for the PLL */
		)
{
	unsigned long ulTmpValue, ulReg, ulReservedBits;

	if (pPLL->clockType == CRT_PLL)
	{
		/* CRT Display: CRT_PLL */
		poke32(ufb,CRT_PLL_CTRL, formatPllReg(pPLL)); 

		/* Frame buffer base for this mode */
		poke32(ufb,CRT_FB_ADDRESS,
				FIELD_SET(0, CRT_FB_ADDRESS, STATUS, PENDING)
				| FIELD_SET(0, CRT_FB_ADDRESS, EXT, LOCAL)
				| FIELD_VALUE(0, CRT_FB_ADDRESS, ADDRESS, ulBaseAddress));

		/* Pitch value (Sometime, hardware people calls it Offset) */
		poke32(ufb,CRT_FB_WIDTH,
				FIELD_VALUE(0, CRT_FB_WIDTH, WIDTH, ulPitch)
				| FIELD_VALUE(0, CRT_FB_WIDTH, OFFSET, ulPitch));

		poke32(ufb,CRT_HORIZONTAL_TOTAL,
				FIELD_VALUE(0, CRT_HORIZONTAL_TOTAL, TOTAL, pModeParam->horizontal_total - 1)
				| FIELD_VALUE(0, CRT_HORIZONTAL_TOTAL, DISPLAY_END, pModeParam->horizontal_display_end - 1));

		poke32(ufb,CRT_HORIZONTAL_SYNC,
				FIELD_VALUE(0, CRT_HORIZONTAL_SYNC, WIDTH, pModeParam->horizontal_sync_width)
				| FIELD_VALUE(0, CRT_HORIZONTAL_SYNC, START, pModeParam->horizontal_sync_start - 1));

		poke32(ufb,CRT_VERTICAL_TOTAL,
				FIELD_VALUE(0, CRT_VERTICAL_TOTAL, TOTAL, pModeParam->vertical_total - 1)
				| FIELD_VALUE(0, CRT_VERTICAL_TOTAL, DISPLAY_END, pModeParam->vertical_display_end - 1));

		poke32(ufb,CRT_VERTICAL_SYNC,
				FIELD_VALUE(0, CRT_VERTICAL_SYNC, HEIGHT, pModeParam->vertical_sync_height)
				| FIELD_VALUE(0, CRT_VERTICAL_SYNC, START, pModeParam->vertical_sync_start - 1));

		/* Set control register value */
		ulTmpValue =
			(pModeParam->vertical_sync_polarity == POS
			 ? FIELD_SET(0, CRT_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH)
			 : FIELD_SET(0, CRT_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW))
			| (pModeParam->horizontal_sync_polarity == POS
					? FIELD_SET(0, CRT_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH)
					: FIELD_SET(0, CRT_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW))
			| FIELD_SET(0, CRT_DISPLAY_CTRL, SELECT, CRT)
			| FIELD_SET(0, CRT_DISPLAY_CTRL, TIMING, ENABLE)
			| FIELD_SET(0, CRT_DISPLAY_CTRL, PLANE, ENABLE)
			| (ulBpp == 8
					? FIELD_SET(0, CRT_DISPLAY_CTRL, FORMAT, 8)
					: (ulBpp == 16
						? FIELD_SET(0, CRT_DISPLAY_CTRL, FORMAT, 16)
						: FIELD_SET(0, CRT_DISPLAY_CTRL, FORMAT, 32)));

		ulReg = peek32(ufb,CRT_DISPLAY_CTRL)
			& FIELD_CLEAR(CRT_DISPLAY_CTRL, VSYNC_PHASE)
			& FIELD_CLEAR(CRT_DISPLAY_CTRL, HSYNC_PHASE)
			& FIELD_CLEAR(CRT_DISPLAY_CTRL, SELECT)
			& FIELD_CLEAR(CRT_DISPLAY_CTRL, TIMING)
			& FIELD_CLEAR(CRT_DISPLAY_CTRL, PLANE)
			& FIELD_CLEAR(CRT_DISPLAY_CTRL, FORMAT);

		poke32(ufb,CRT_DISPLAY_CTRL, ulTmpValue | ulReg);
	}
	else 
	{		
		/* Panel display: PANEL_PLL */
		poke32(ufb,PANEL_PLL_CTRL, formatPllReg(pPLL));
		/* Program panel PLL, if applicable */
		if (pPLL->clockType == PANEL_PLL)
		{
			poke32(ufb,PANEL_PLL_CTRL, formatPllReg(pPLL));
#ifndef VALIDATION_CHIP
			/* Program to Non-VGA mode when using Panel PLL */
			poke32(ufb,VGA_CONFIGURATION, 
					FIELD_SET(peek32(ufb,VGA_CONFIGURATION), VGA_CONFIGURATION, PLL, PANEL));
#endif
		}

		/* Frame buffer base for this mode */
		ulReg = FIELD_SET(0, PANEL_FB_ADDRESS, STATUS, CURRENT)
			| FIELD_SET(0, PANEL_FB_ADDRESS, EXT, LOCAL)
			| FIELD_VALUE(0, PANEL_FB_ADDRESS, ADDRESS, ulBaseAddress);

		poke32(ufb,PANEL_FB_ADDRESS,ulReg); 		

		/* Pitch value (Sometime, hardware people calls it Offset) */
		poke32(ufb,PANEL_FB_WIDTH, ulReg=
				FIELD_VALUE(0, PANEL_FB_WIDTH, WIDTH, ulPitch)
				| FIELD_VALUE(0, PANEL_FB_WIDTH, OFFSET, ulPitch));

		poke32(ufb,PANEL_WINDOW_WIDTH,ulReg=
				FIELD_VALUE(0, PANEL_WINDOW_WIDTH, WIDTH, pModeParam->horizontal_display_end)
				| FIELD_VALUE(0, PANEL_WINDOW_WIDTH, X, 0));

		poke32(ufb,PANEL_WINDOW_HEIGHT,ulReg=
				FIELD_VALUE(0, PANEL_WINDOW_HEIGHT, HEIGHT, pModeParam->vertical_display_end)
				| FIELD_VALUE(0, PANEL_WINDOW_HEIGHT, Y, 0));

		poke32(ufb,PANEL_PLANE_TL,ulReg=
				FIELD_VALUE(0, PANEL_PLANE_TL, TOP, 0)
				| FIELD_VALUE(0, PANEL_PLANE_TL, LEFT, 0));

		poke32(ufb,PANEL_PLANE_BR, ulReg=
				FIELD_VALUE(0, PANEL_PLANE_BR, BOTTOM, pModeParam->vertical_display_end - 1)
				| FIELD_VALUE(0, PANEL_PLANE_BR, RIGHT, pModeParam->horizontal_display_end - 1));

		poke32(ufb,PANEL_HORIZONTAL_TOTAL,ulReg=
				FIELD_VALUE(0, PANEL_HORIZONTAL_TOTAL, TOTAL, pModeParam->horizontal_total - 1)
				| FIELD_VALUE(0, PANEL_HORIZONTAL_TOTAL, DISPLAY_END, pModeParam->horizontal_display_end - 1));

		poke32(ufb,PANEL_HORIZONTAL_SYNC,ulReg=
				FIELD_VALUE(0, PANEL_HORIZONTAL_SYNC, WIDTH, pModeParam->horizontal_sync_width)
				| FIELD_VALUE(0, PANEL_HORIZONTAL_SYNC, START, pModeParam->horizontal_sync_start - 1));

		poke32(ufb,PANEL_VERTICAL_TOTAL,ulReg=
				FIELD_VALUE(0, PANEL_VERTICAL_TOTAL, TOTAL, pModeParam->vertical_total - 1)
				| FIELD_VALUE(0, PANEL_VERTICAL_TOTAL, DISPLAY_END, pModeParam->vertical_display_end - 1));

		poke32(ufb,PANEL_VERTICAL_SYNC,ulReg=
				FIELD_VALUE(0, PANEL_VERTICAL_SYNC, HEIGHT, pModeParam->vertical_sync_height)
				| FIELD_VALUE(0, PANEL_VERTICAL_SYNC, START, pModeParam->vertical_sync_start - 1));

		/* Added some masks to mask out the reserved bits. 
		 * Sometimes, the reserved bits are set/reset randomly when 
		 * writing to the PANEL_DISPLAY_CTRL, therefore, the register
		 * reserved bits are needed to be masked out.
		 */

		/* Set control register value */
		ulReservedBits = FIELD_SET(0, PANEL_DISPLAY_CTRL, RESERVED_1_MASK, ENABLE) |
			FIELD_SET(0, PANEL_DISPLAY_CTRL, RESERVED_2_MASK, ENABLE) |
			FIELD_SET(0, PANEL_DISPLAY_CTRL, RESERVED_3_MASK, ENABLE)|
			FIELD_VALUE(0,PANEL_DISPLAY_CTRL,VSYNC,1);

		ulTmpValue = peek32(ufb,PANEL_DISPLAY_CTRL);	
		ulTmpValue &= ~ulReservedBits;

		if(pModeParam->vertical_sync_polarity == POS )
			ulTmpValue = FIELD_SET(ulTmpValue,PANEL_DISPLAY_CTRL,VSYNC_PHASE,ACTIVE_HIGH);
		else
			ulTmpValue = FIELD_SET(ulTmpValue,PANEL_DISPLAY_CTRL,VSYNC_PHASE,ACTIVE_LOW);

		if(pModeParam->horizontal_sync_polarity == POS )
			ulTmpValue = FIELD_SET(ulTmpValue,PANEL_DISPLAY_CTRL,HSYNC_PHASE,ACTIVE_HIGH);
		else
			ulTmpValue = FIELD_SET(ulTmpValue,PANEL_DISPLAY_CTRL,HSYNC_PHASE,ACTIVE_LOW);

		if(pModeParam->clock_phase_polarity == POS)
			ulTmpValue = FIELD_SET(ulTmpValue,PANEL_DISPLAY_CTRL,CLOCK_PHASE,ACTIVE_HIGH);
		else
			ulTmpValue = FIELD_SET(ulTmpValue,PANEL_DISPLAY_CTRL,CLOCK_PHASE,ACTIVE_LOW);


		ulTmpValue = FIELD_SET(ulTmpValue,PANEL_DISPLAY_CTRL,TIMING,ENABLE);
		ulTmpValue = FIELD_SET(ulTmpValue,PANEL_DISPLAY_CTRL,PLANE,ENABLE);
		ulTmpValue = 	FIELD_SET(ulTmpValue,PANEL_DISPLAY_CTRL,DUAL_DISPLAY,DISABLE);
		ulTmpValue = 	FIELD_SET(ulTmpValue, PANEL_DISPLAY_CTRL,FIFO, 3);
		ulTmpValue =   FIELD_SET(ulTmpValue,PANEL_DISPLAY_CTRL,GAMMA,DISABLE);

		if(ulBpp==8)
			ulTmpValue = FIELD_SET(ulTmpValue,PANEL_DISPLAY_CTRL,FORMAT,8);
		else if(ulBpp==16)
			ulTmpValue = FIELD_SET(ulTmpValue,PANEL_DISPLAY_CTRL,FORMAT,16);
		else
			ulTmpValue = FIELD_SET(ulTmpValue,PANEL_DISPLAY_CTRL,FORMAT,32);

		unsigned long A,B;
		B = ulTmpValue;
		int t=0;
		do
		{
			poke32(ufb,PANEL_DISPLAY_CTRL,B);
			A = peek32(ufb,PANEL_DISPLAY_CTRL) & ~ulReservedBits;
			t++;
		}while(A!=B && t<10); 
	}
}

#else

/* 
 * Program the hardware for a specific video mode
 */
void programModeRegisters(
		void * ufb,
		mode_parameter_t *pModeParam,   /* mode information about pixel clock, horizontal total, etc. */
		unsigned long ulBpp,            /* Color depth for this mode */
		unsigned long ulBaseAddress,    /* Offset in frame buffer */
		unsigned long ulPitch,          /* Mode pitch value in byte: no of bytes between two lines. */
		pll_value_t *pPLL               /* Pre-calculated values for the PLL */
		)
{
	unsigned long ulTmpValue, ulReg, ulReservedBits;
	//unsigned long palette_ram;
	//unsigned long offset;

	/* Enable display power gate */
	ulTmpValue = peek32(ufb,CURRENT_GATE);
	ulTmpValue = FIELD_SET(ulTmpValue, CURRENT_GATE, DISPLAY, ON);
	setCurrentGate(ufb,ulTmpValue);

	if (pPLL->clockType == CRT_PLL)
	{
		/* CRT Display: CRT_PLL */
		poke32(ufb,CRT_PLL_CTRL, formatPllReg(pPLL)); 

		/* Frame buffer base for this mode */
		poke32(ufb,CRT_FB_ADDRESS,
				FIELD_SET(0, CRT_FB_ADDRESS, STATUS, PENDING)
				| FIELD_SET(0, CRT_FB_ADDRESS, EXT, LOCAL)
				| FIELD_VALUE(0, CRT_FB_ADDRESS, ADDRESS, ulBaseAddress));

		/* Pitch value (Sometime, hardware people calls it Offset) */
		poke32(ufb,CRT_FB_WIDTH,
				FIELD_VALUE(0, CRT_FB_WIDTH, WIDTH, ulPitch)
				| FIELD_VALUE(0, CRT_FB_WIDTH, OFFSET, ulPitch));

		poke32(ufb,CRT_HORIZONTAL_TOTAL,
				FIELD_VALUE(0, CRT_HORIZONTAL_TOTAL, TOTAL, pModeParam->horizontal_total - 1)
				| FIELD_VALUE(0, CRT_HORIZONTAL_TOTAL, DISPLAY_END, pModeParam->horizontal_display_end - 1));

		poke32(ufb,CRT_HORIZONTAL_SYNC,
				FIELD_VALUE(0, CRT_HORIZONTAL_SYNC, WIDTH, pModeParam->horizontal_sync_width)
				| FIELD_VALUE(0, CRT_HORIZONTAL_SYNC, START, pModeParam->horizontal_sync_start - 1));

		poke32(ufb,CRT_VERTICAL_TOTAL,
				FIELD_VALUE(0, CRT_VERTICAL_TOTAL, TOTAL, pModeParam->vertical_total - 1)
				| FIELD_VALUE(0, CRT_VERTICAL_TOTAL, DISPLAY_END, pModeParam->vertical_display_end - 1));

		poke32(ufb,CRT_VERTICAL_SYNC,
				FIELD_VALUE(0, CRT_VERTICAL_SYNC, HEIGHT, pModeParam->vertical_sync_height)
				| FIELD_VALUE(0, CRT_VERTICAL_SYNC, START, pModeParam->vertical_sync_start - 1));

		/* Set control register value */
		ulTmpValue =        
			(pModeParam->vertical_sync_polarity == POS
			 ? FIELD_SET(0, CRT_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH)
			 : FIELD_SET(0, CRT_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW))
			| (pModeParam->horizontal_sync_polarity == POS
					? FIELD_SET(0, CRT_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH)
					: FIELD_SET(0, CRT_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW))
			| FIELD_SET(0, CRT_DISPLAY_CTRL, SELECT, CRT)
			| FIELD_SET(0, CRT_DISPLAY_CTRL, TIMING, ENABLE)
			| FIELD_SET(0, CRT_DISPLAY_CTRL, PLANE, ENABLE) 
			| (ulBpp == 8
					? FIELD_SET(0, CRT_DISPLAY_CTRL, FORMAT, 8)
					: (ulBpp == 16
						? FIELD_SET(0, CRT_DISPLAY_CTRL, FORMAT, 16)
						: FIELD_SET(0, CRT_DISPLAY_CTRL, FORMAT, 32)));

		/* TODO: Check if the auto expansion bit can be cleared here */
		ulReg = peek32(ufb,CRT_DISPLAY_CTRL)
			& FIELD_CLEAR(CRT_DISPLAY_CTRL, VSYNC_PHASE)
			& FIELD_CLEAR(CRT_DISPLAY_CTRL, HSYNC_PHASE)
			& FIELD_CLEAR(CRT_DISPLAY_CTRL, SELECT)
			& FIELD_CLEAR(CRT_DISPLAY_CTRL, TIMING)
			& FIELD_CLEAR(CRT_DISPLAY_CTRL, PLANE)
			& FIELD_CLEAR(CRT_DISPLAY_CTRL, FORMAT)
			& FIELD_CLEAR(CRT_DISPLAY_CTRL, LOCK_TIMING)
			& FIELD_CLEAR(CRT_DISPLAY_CTRL, EXPANSION);

		poke32(ufb,CRT_DISPLAY_CTRL, ulTmpValue | ulReg);

		/* Save the current mode param */
		//gCRTCurrentModeParam[getCurrentDevice()] = *pModeParam;	
	}
	else 
	{
		/* Panel display: PANEL_PLL */
		poke32(ufb,PANEL_PLL_CTRL, formatPllReg(pPLL));

		/* Program panel PLL, if applicable */
		if (pPLL->clockType == PANEL_PLL)
		{
			poke32(ufb,PANEL_PLL_CTRL, formatPllReg(pPLL));
#ifndef VALIDATION_CHIP
			/* Program to Non-VGA mode when using Panel PLL */
			poke32(ufb,VGA_CONFIGURATION, 
					FIELD_SET(peek32(ufb,VGA_CONFIGURATION), VGA_CONFIGURATION, PLL, PANEL));
#endif
		}

		/* Frame buffer base for this mode */
		poke32(ufb,PANEL_FB_ADDRESS,
				FIELD_SET(0, PANEL_FB_ADDRESS, STATUS, CURRENT)
				| FIELD_SET(0, PANEL_FB_ADDRESS, EXT, LOCAL)
				| FIELD_VALUE(0, PANEL_FB_ADDRESS, ADDRESS, ulBaseAddress));

		/* Pitch value (Sometime, hardware people calls it Offset) */
		poke32(ufb,PANEL_FB_WIDTH,
				FIELD_VALUE(0, PANEL_FB_WIDTH, WIDTH, ulPitch)
				| FIELD_VALUE(0, PANEL_FB_WIDTH, OFFSET, ulPitch));

		poke32(ufb,PANEL_WINDOW_WIDTH,
				FIELD_VALUE(0, PANEL_WINDOW_WIDTH, WIDTH, pModeParam->horizontal_display_end - 1)
				| FIELD_VALUE(0, PANEL_WINDOW_WIDTH, X, 0));

		poke32(ufb,PANEL_WINDOW_HEIGHT,
				FIELD_VALUE(0, PANEL_WINDOW_HEIGHT, HEIGHT, pModeParam->vertical_display_end - 1)
				| FIELD_VALUE(0, PANEL_WINDOW_HEIGHT, Y, 0));

		poke32(ufb,PANEL_PLANE_TL,
				FIELD_VALUE(0, PANEL_PLANE_TL, TOP, 0)
				| FIELD_VALUE(0, PANEL_PLANE_TL, LEFT, 0));

		poke32(ufb,PANEL_PLANE_BR, 
				FIELD_VALUE(0, PANEL_PLANE_BR, BOTTOM, pModeParam->vertical_display_end - 1)
				| FIELD_VALUE(0, PANEL_PLANE_BR, RIGHT, pModeParam->horizontal_display_end - 1));

		poke32(ufb,PANEL_HORIZONTAL_TOTAL,
				FIELD_VALUE(0, PANEL_HORIZONTAL_TOTAL, TOTAL, pModeParam->horizontal_total - 1)
				| FIELD_VALUE(0, PANEL_HORIZONTAL_TOTAL, DISPLAY_END, pModeParam->horizontal_display_end - 1));

		poke32(ufb,PANEL_HORIZONTAL_SYNC,
				FIELD_VALUE(0, PANEL_HORIZONTAL_SYNC, WIDTH, pModeParam->horizontal_sync_width)
				| FIELD_VALUE(0, PANEL_HORIZONTAL_SYNC, START, pModeParam->horizontal_sync_start - 1));

		poke32(ufb,PANEL_VERTICAL_TOTAL,
				FIELD_VALUE(0, PANEL_VERTICAL_TOTAL, TOTAL, pModeParam->vertical_total - 1)
				| FIELD_VALUE(0, PANEL_VERTICAL_TOTAL, DISPLAY_END, pModeParam->vertical_display_end - 1));

		poke32(ufb,PANEL_VERTICAL_SYNC,
				FIELD_VALUE(0, PANEL_VERTICAL_SYNC, HEIGHT, pModeParam->vertical_sync_height)
				| FIELD_VALUE(0, PANEL_VERTICAL_SYNC, START, pModeParam->vertical_sync_start - 1));

		/* Set control register value */
		ulTmpValue =
			(pModeParam->clock_phase_polarity == POS
			 ? FIELD_SET(0, PANEL_DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_HIGH)
			 : FIELD_SET(0, PANEL_DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_LOW))
			| (pModeParam->vertical_sync_polarity == POS
					? FIELD_SET(0, PANEL_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH)
					: FIELD_SET(0, PANEL_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW))
			| (pModeParam->horizontal_sync_polarity == POS
					? FIELD_SET(0, PANEL_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH)
					: FIELD_SET(0, PANEL_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW))
			| FIELD_SET(0, PANEL_DISPLAY_CTRL, TIMING, ENABLE)
			| FIELD_SET(0, PANEL_DISPLAY_CTRL, PLANE, ENABLE)
			| (ulBpp == 8
					? FIELD_SET(0, PANEL_DISPLAY_CTRL, FORMAT, 8)
					: (ulBpp == 16
						? FIELD_SET(0, PANEL_DISPLAY_CTRL, FORMAT, 16)
						: FIELD_SET(0, PANEL_DISPLAY_CTRL, FORMAT, 32)));

		/* Added some masks to mask out the reserved bits. 
		 * Sometimes, the reserved bits are set/reset randomly when 
		 * writing to the PANEL_DISPLAY_CTRL, therefore, the register
		 * reserved bits are needed to be masked out.
		 */
		ulReservedBits = FIELD_SET(0, PANEL_DISPLAY_CTRL, RESERVED_1_MASK, ENABLE) |
			FIELD_SET(0, PANEL_DISPLAY_CTRL, RESERVED_2_MASK, ENABLE) |
			FIELD_SET(0, PANEL_DISPLAY_CTRL, RESERVED_3_MASK, ENABLE)|
			FIELD_SET(0,PANEL_DISPLAY_CTRL,VSYNC,ACTIVE_LOW);

		ulReg = (peek32(ufb,PANEL_DISPLAY_CTRL) & ~ulReservedBits)
			& FIELD_CLEAR(PANEL_DISPLAY_CTRL, CLOCK_PHASE)
			& FIELD_CLEAR(PANEL_DISPLAY_CTRL, VSYNC_PHASE)
			& FIELD_CLEAR(PANEL_DISPLAY_CTRL, HSYNC_PHASE)
			& FIELD_CLEAR(PANEL_DISPLAY_CTRL, TIMING)
			& FIELD_CLEAR(PANEL_DISPLAY_CTRL, VERTICAL_PAN)
			& FIELD_CLEAR(PANEL_DISPLAY_CTRL, HORIZONTAL_PAN)
			& FIELD_CLEAR(PANEL_DISPLAY_CTRL, PLANE)
			& FIELD_CLEAR(PANEL_DISPLAY_CTRL, FORMAT);

		poke32(ufb,PANEL_DISPLAY_CTRL, ulTmpValue | ulReg);

		/* May a hardware bug or just my test chip (not confirmed).
		 * PANEL_DISPLAY_CTRL register seems requiring few writes
		 * before a value can be succesfully written in.
		 * Added some masks to mask out the reserved bits.
		 * Note: This problem happens by design. The hardware will wait for the
		 *       next vertical sync to turn on/off the plane.
		 */
		while((peek32(ufb,PANEL_DISPLAY_CTRL) & ~ulReservedBits) != (ulTmpValue|ulReg))
		{
			poke32(ufb,PANEL_DISPLAY_CTRL, ulTmpValue | ulReg);
		}

		/* Save the current mode param */
		//gPanelCurrentModeParam[getCurrentDevice()] = *pModeParam;
	}
}
#endif

long ddk750_setModeEx(void * ufb,logicalMode_t *pLogicalMode)
{
	mode_parameter_t *pVesaModeParam; /* physical parameters for the mode */
	unsigned long modeWidth, modeHeight;

	/* Conditions to set the mode when scaling is needed (xLCD and yLCD is not zeroes)
	 *      A. When display control is not swapped:
	 *          1. PANEL_CTRL
	 *              a. Set the primary display control timing to the actual display mode.
	 *              b. Set the secondary display control timing to the mode that equals to
	 *                 the panel size.
	 *          2. CRT_CTRL
	 *              a. Set the secondary display control timing to the mode that equals to
	 *                 the panel size.
	 *      B. When display control is swapped:
	 *          1. PANEL_CTRL
	 *              a. Set the secondary display control timing to the mode that equals to
	 *                 the panel size.
	 *          2. CRT_CTRL
	 *              a. Not supported 
	 */
	if ((((isDisplayControlSwapped() == 0) && (pLogicalMode->dispCtrl == CRT_CTRL)) ||
				((isDisplayControlSwapped() == 1) && (pLogicalMode->dispCtrl == PANEL_CTRL))) &&
			(pLogicalMode->xLCD != 0) && (pLogicalMode->yLCD != 0))
	{
		modeWidth = pLogicalMode->xLCD;
		modeHeight = pLogicalMode->yLCD;
	}
	else
	{
		modeWidth = pLogicalMode->x;
		modeHeight = pLogicalMode->y;
	}

	pVesaModeParam = (mode_parameter_t *)findVesaModeParam(modeWidth, modeHeight, pLogicalMode->hz);
	if (pVesaModeParam == (mode_parameter_t *)0)
		return -1;

	return(setCustomModeEx(ufb,pLogicalMode, pVesaModeParam));
}


long setCustomModeEx(void * ufb,logicalMode_t *pLogicalMode, mode_parameter_t *pUserModeParam)
{
	long returnValue = 0;  

	/* Return error when the mode is bigger than the panel size. Might be temporary solution
	   depending whether we will support panning or not. */
	if (((pLogicalMode->xLCD != 0) && 
				(pLogicalMode->yLCD != 0)) &&
			((pLogicalMode->xLCD < pLogicalMode->x) ||
			 (pLogicalMode->yLCD < pLogicalMode->y)))
		return (-1);

	/* Return error when the panel size exceed the maximum mode that we can support. */
	if ((pLogicalMode->xLCD > MAX_PANEL_SIZE_WIDTH) ||
			(pLogicalMode->yLCD > MAX_PANEL_SIZE_HEIGHT))
		return (-1);    /* Unsupported panel size. */

	/* Set the mode first */
	returnValue = ddk750_setCustomMode(ufb,pLogicalMode, pUserModeParam);
	if (returnValue == 0)
	{
		/* Scale display if necessary */
		returnValue = scaleDisplay(ufb,pLogicalMode);
	}

	return (returnValue);
}



mode_parameter_t *findVesaModeParam(
		unsigned long width, 
		unsigned long height, 
		unsigned long refresh_rate)
{
	mode_parameter_t *mode_table = gModeParamTable;

	/* Walk the entire mode table. */
	while (mode_table->pixel_clock != 0)
	{
		if ((mode_table->horizontal_display_end == width)
				&& (mode_table->vertical_display_end == height)
				&& (mode_table->vertical_frequency == refresh_rate))
		{
			return(mode_table);
		}
		mode_table++; /* Next entry */
	}

	/* No match, return NULL pointer */
	return((mode_parameter_t *)0);
}


/*
 * This function scale the display data.
 *
 * Input:
 *      pLogicalMode    - Logical Mode parameters which is to be scaled
 */
long scaleDisplay(
		void * ufb,
		logicalMode_t *pLogicalMode         /* Logical Mode */
		)
{
	unsigned long value = 0;
	//	uint32_t adjustedHorzTotal, adjustedPriFrequency, adjustedSecFrequency;
	uint32_t vertScaleFactor, horzScaleFactor, dstWidth, dstHeight;
	//    mode_parameter_t primaryModeParam, secondaryModeParam;
	//logicalMode_t secLogicalMode;

	/* Enable scaling when the source width and height is smaller than the destination
	   width and height. */

	if ((pLogicalMode->xLCD != 0) &&
			(pLogicalMode->yLCD != 0) &&
			(pLogicalMode->x < pLogicalMode->xLCD) &&
			(pLogicalMode->y < pLogicalMode->yLCD))     
	{
		dstWidth = pLogicalMode->xLCD;
		dstHeight = pLogicalMode->yLCD;

		/* Scale the vertical size */
		vertScaleFactor = 0;
		vertScaleFactor = pLogicalMode->y * SCALE_CONSTANT / dstHeight;

		/* Scale the horizontal size */
		horzScaleFactor = 0;
		horzScaleFactor = pLogicalMode->x * SCALE_CONSTANT / dstWidth;

		if (isDisplayControlSwapped() == 0)
		{
#if 0        
			/* Display control is not swapped */            
			/* Enable Panel scaler path */
			if (pLogicalMode->dispCtrl == PANEL_CTRL)
			{
				/* Check for Vesa Mode timing for the intended panel display. */
				mode_parameter_t *pVesaModeParam;
				pVesaModeParam = (mode_parameter_t *)findVesaModeParam(pLogicalMode->xLCD, pLogicalMode->yLCD, pLogicalMode->hz);
				if (pVesaModeParam == (mode_parameter_t *)0)
					return (-1);

				/* Set the mode on the secondary channel */    
				secLogicalMode.x = pLogicalMode->xLCD;
				secLogicalMode.y = pLogicalMode->yLCD;
				secLogicalMode.hz = 60;
				secLogicalMode.baseAddress = pLogicalMode->baseAddress;
				secLogicalMode.bpp = pLogicalMode->bpp;
				secLogicalMode.dispCtrl = CRT_CTRL;
				secLogicalMode.pitch = 0;
				secLogicalMode.xLCD = 0;
				secLogicalMode.yLCD = 0;
				if (ddk750_setCustomMode(&secLogicalMode, pVesaModeParam) != 0)
					return (-1);

				/* Get the current mode parameter. */
				//primaryModeParam = getCurrentModeParam(PANEL_CTRL);
				//secondaryModeParam = getCurrentModeParam(CRT_CTRL);

				/* Set up the vertical auto expansion values. For the line buffer, 
				   always use the maximum number of line. */
				value = peek32(ufb,CRT_VERTICAL_EXPANSION);
				value = FIELD_VALUE(value, CRT_VERTICAL_EXPANSION, COMPARE_VALUE, 
						((primaryModeParam.vertical_display_end - 1) >> 3) & 0x00FF);
				value = FIELD_VALUE(value, CRT_VERTICAL_EXPANSION, LINE_BUFFER, 4);
				value = FIELD_VALUE(value, CRT_VERTICAL_EXPANSION, SCALE_FACTOR, vertScaleFactor);
				poke32(ufb,CRT_VERTICAL_EXPANSION, value);

				/* Set up the horizontal auto expansion values. */
				value = FIELD_VALUE(0, CRT_HORIZONTAL_EXPANSION, COMPARE_VALUE, 
						((primaryModeParam.horizontal_display_end - 1) >> 3) & 0x00FF);
				value = FIELD_VALUE(value, CRT_HORIZONTAL_EXPANSION, SCALE_FACTOR, horzScaleFactor);  
				poke32(ufb,CRT_HORIZONTAL_EXPANSION, value);            

				/* Adjust the horizontal total value using the following formula:
				   srcHz = dstHz
				   srcPixelClock / srcHT / srcVDE = dstPixelClock / dstHT / dstVDE

				   srcHT = ((dstHT * srcPixelClock) / dstPixelClock) * (dstVDE / srcVDE)
				   or
				   dstHT = ((srcHT * dstPixelClock) / srcPixelClock) * (srcVDE / dstVDE)

				   This formula is needed to match the time needed on the visible source display
				   to the time needed on the visible destination display. Thus, the hardware can lock it.
NOTE: srcVDE and dstVDE are used instead of srcVT and dstVT since we only care
about the display portions.

According to HW Engineer, the dstHT formula is used so that the source timing 
can be maintained the same while limit the modification on the destination timing.
Original thinking is adjusting the source timing so that the vertical refresh on
the destination can be set to a fixed frequency, 60Hz. However, since the lock 
timing mechanism itself violates the destination timing, there is no need to
maintain the destination timing.
*/

#if 1
				/*
				 * dstHT = ((srcHT * dstPixelClock) / srcPixelClock) * (srcVDE / dstVDE)
				 */
				adjustedPriFrequency = primaryModeParam.pixel_clock / 1000;
				adjustedSecFrequency = secondaryModeParam.pixel_clock / 1000;
#if 1            
				adjustedHorzTotal = (primaryModeParam.horizontal_total * adjustedSecFrequency) / adjustedPriFrequency;
				adjustedHorzTotal = (adjustedHorzTotal * pLogicalMode->y) / secondaryModeParam.vertical_display_end;
#else
				adjustedHorzTotal = (primaryModeParam.horizontal_total * adjustedSecFrequency) / adjustedPriFrequency;
				adjustedHorzTotal = (adjustedHorzTotal * primaryModeParam.vertical_total) / secondaryModeParam.vertical_total;
#endif
				value = peek32(ufb,CRT_HORIZONTAL_TOTAL);
				value = FIELD_VALUE(value, CRT_HORIZONTAL_TOTAL, TOTAL, adjustedHorzTotal); 
				poke32(ufb,CRT_HORIZONTAL_TOTAL, value);
#else            
				/* 
				 * srcHT = ((dstHT * srcPixelClock) / dstPixelClock) * (dstVDE / srcVDE) 
				 */
				adjustedPriFrequency = primaryModeParam.pixel_clock / 1000;
				adjustedSecFrequency = secondaryModeParam.pixel_clock / 1000;
#if 1            
				adjustedHorzTotal = (secondaryModeParam.horizontal_total * adjustedPriFrequency) / adjustedSecFrequency;
				adjustedHorzTotal = (adjustedHorzTotal * secondaryModeParam.vertical_display_end) / pLogicalMode->y;
#else
				adjustedHorzTotal = (secondaryModeParam.horizontal_total * adjustedPriFrequency) / adjustedSecFrequency;
				adjustedHorzTotal = (adjustedHorzTotal * secondaryModeParam.vertical_total) / primaryModeParam.vertical_total;
#endif
				value = peek32(ufb,PANEL_HORIZONTAL_TOTAL);
				value = FIELD_VALUE(value, PANEL_HORIZONTAL_TOTAL, TOTAL, adjustedHorzTotal); 
				poke32(ufb,PANEL_HORIZONTAL_TOTAL, value);
#endif

				/* Enable Auto Expansion bit. */
				value = peek32(ufb,CRT_DISPLAY_CTRL);
				value = FIELD_SET(value, CRT_DISPLAY_CTRL, EXPANSION, ENABLE);
				value = FIELD_SET(value, CRT_DISPLAY_CTRL, LOCK_TIMING, ENABLE);
				poke32(ufb,CRT_DISPLAY_CTRL, value);
			}
			else
#endif
			{
#if 1           
				/* CRT Scaling (Video Frame Buffer --> screen) */
				value = FIELD_VALUE(value, CRT_SCALE , VERTICAL_SCALE, vertScaleFactor);
				value = FIELD_VALUE(value, CRT_SCALE , HORIZONTAL_SCALE, horzScaleFactor);
				poke32(ufb,CRT_SCALE , value);
#endif
			}
		}
		else
		{
			/* Display control is swapped. */
			/* Panel Scaling (Video Frame Buffer --> screen) */
			if (pLogicalMode->dispCtrl == PANEL_CTRL)
			{
				value = FIELD_VALUE(value, CRT_SCALE , VERTICAL_SCALE, vertScaleFactor);
				value = FIELD_VALUE(value, CRT_SCALE , HORIZONTAL_SCALE, horzScaleFactor);
				poke32(ufb,CRT_SCALE , value);        
			}
		}        
		/* Set the interpolation when scaling is needed. */

		if(pLogicalMode->bpp != 8)
			displaySetInterpolation(ufb,1, 1);
		else
			displaySetInterpolation(ufb,0, 0);
	}
	else
	{
		/* Set the scaler value to 0 */
		poke32(ufb,CRT_SCALE , 0);        

		/* Program the scaler to use 1:1 scaler by setting the scaler register value to 0 */
		//if ((pLogicalMode->dispCtrl != CRT_CTRL) && (isDisplayControlSwapped() == 0))
		{
			value = peek32(ufb,CRT_DISPLAY_CTRL);
			value = FIELD_SET(value, CRT_DISPLAY_CTRL, EXPANSION, DISABLE);
			value = FIELD_SET(value, CRT_DISPLAY_CTRL, LOCK_TIMING, DISABLE);
			poke32(ufb,CRT_DISPLAY_CTRL, value);
		}

		/* Disable interpolation when scaling is not needed. */
		displaySetInterpolation(ufb,0, 0);
	}

	return 0;
}


