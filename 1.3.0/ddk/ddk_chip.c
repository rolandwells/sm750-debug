#include "ddk750_common.h"

unsigned long ddk750_getFrameBufSize(void *ufb)
{
	unsigned long sizeSymbol, memSize;
    	sizeSymbol = FIELD_GET(peek32(ufb,MISC_CTRL), MISC_CTRL, LOCALMEM_SIZE);
    switch(sizeSymbol)
    {
        case MISC_CTRL_LOCALMEM_SIZE_8M:  memSize = MB(8);  break; /* 8  Mega byte */
        case MISC_CTRL_LOCALMEM_SIZE_16M: memSize = MB(16); break; /* 16 Mega byte */
        case MISC_CTRL_LOCALMEM_SIZE_32M: memSize = MB(32); break; /* 32 Mega byte */
        case MISC_CTRL_LOCALMEM_SIZE_64M: memSize = MB(64); break; /* 64 Mega byte */
        default:                         memSize = MB(0);  break; /* 0  Mege byte */
    }
    return memSize;
}


/*
 * Initialize chip and environment according to input parameters.
 *
 * Input: initchip_param_t structure.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 *
 */
long ddk750_initChipParm(void *ufb,initchip_param_t *pInitParam)
{
    unsigned long ulReg;
    //unsigned short deviceId;

 /* Set power mode.
       Check parameter validity first.
       If calling function didn't set it up properly or set to some
       weird value, always default it to 0.
*/
     
	if (pInitParam->powerMode != 0 ) 
		pInitParam->powerMode = 0;
	setPowerMode(ufb,pInitParam->powerMode);

	/* Enable display power gate & LOCALMEM power gate*/
   	ulReg = peek32(ufb,CURRENT_GATE);
	ulReg = FIELD_SET(ulReg, CURRENT_GATE, DISPLAY, ON);
	ulReg = FIELD_SET(ulReg,CURRENT_GATE,LOCALMEM,ON);	
	setCurrentGate(ufb,ulReg);

	/*	set panel pll and graphic mode	*/
	ulReg = peek32(ufb,VGA_CONFIGURATION);
	ulReg = FIELD_SET(ulReg,VGA_CONFIGURATION,PLL,PANEL);
	ulReg = FIELD_SET(ulReg,VGA_CONFIGURATION,MODE,GRAPHIC);
	poke32(ufb,VGA_CONFIGURATION,ulReg);

    
#if 1
    /* Set the Main Chip Clock */
    setChipClock(ufb,MHz(pInitParam->chipClock));

    /* Set up master clock */
    setMasterClock(ufb,MHz(pInitParam->masterClock));

    /* Set up memory clock. */
    setMemoryClock(ufb,MHz(pInitParam->memClock));    
#endif

    /* Reset the memory controller. If the memory controller is not reset in SM750, 
       the system might hang when sw accesses the memory. 
       The memory should be resetted after changing the MXCLK.
     */
    if (pInitParam->resetMemory == 1)
    {     
        ulReg = peek32(ufb,MISC_CTRL);
        ulReg = FIELD_SET(ulReg, MISC_CTRL, LOCALMEM_RESET, RESET);
        poke32(ufb,MISC_CTRL, ulReg);
    
        ulReg = FIELD_SET(ulReg, MISC_CTRL, LOCALMEM_RESET, NORMAL);
        poke32(ufb,MISC_CTRL, ulReg);    
    }
    
    if (pInitParam->setAllEngOff == 1)
    {
        //enable2DEngine(ufb,0);
        /* Disable Overlay, if a former application left it on */
        ulReg = peek32(ufb,VIDEO_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, VIDEO_DISPLAY_CTRL, PLANE, DISABLE); 
        poke32(ufb,VIDEO_DISPLAY_CTRL, ulReg);

        /* Disable video alpha, if a former application left it on */
        ulReg = peek32(ufb,VIDEO_ALPHA_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, VIDEO_ALPHA_DISPLAY_CTRL, PLANE, DISABLE); 
        poke32(ufb,VIDEO_ALPHA_DISPLAY_CTRL, ulReg);

        /* Disable alpha plane, if a former application left it on */
        ulReg = peek32(ufb,ALPHA_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, ALPHA_DISPLAY_CTRL, PLANE, DISABLE); 
        poke32(ufb,ALPHA_DISPLAY_CTRL, ulReg);

        /* Disable LCD hardware cursor, if a former application left it on */
        ulReg = peek32(ufb,PANEL_HWC_ADDRESS);
        ulReg = FIELD_SET(ulReg, PANEL_HWC_ADDRESS, ENABLE, DISABLE); 
        poke32(ufb,PANEL_HWC_ADDRESS, ulReg);

        /* Disable CRT hardware cursor, if a former application left it on */
        ulReg = peek32(ufb,CRT_HWC_ADDRESS);
        ulReg = FIELD_SET(ulReg, CRT_HWC_ADDRESS, ENABLE, DISABLE); 
        poke32(ufb,CRT_HWC_ADDRESS, ulReg);

        /* Disable ZV Port 0, if a former application left it on */
        ulReg = peek32(ufb,ZV0_CAPTURE_CTRL);
        ulReg = FIELD_SET(ulReg, ZV0_CAPTURE_CTRL, CAP, DISABLE); 
        poke32(ufb,ZV0_CAPTURE_CTRL, ulReg);

        /* Disable ZV Port 1, if a former application left it on */
        ulReg = peek32(ufb,ZV1_CAPTURE_CTRL);
        ulReg = FIELD_SET(ulReg, ZV1_CAPTURE_CTRL, CAP, DISABLE); 
        poke32(ufb,ZV1_CAPTURE_CTRL, ulReg);
      
        /* Disable ZV Port Power, if a former application left it on */
        enableZVPort(ufb,0);
#ifdef DMA
        /* Disable DMA Channel, if a former application left it on */
        ulReg = peek32(ufb,DMA_ABORT_INTERRUPT);
        ulReg = FIELD_SET(ulReg, DMA_ABORT_INTERRUPT, ABORT_1, ABORT);
        poke32(ufb,DMA_ABORT_INTERRUPT, ulReg);
        
        /* Disable DMA Power, if a former application left it on */
        enableDMA(ufb,0);
#endif
    }	
	enable2DEngine(ufb,1);

    /* We can add more initialization as needed. */
        
    return 0;
}


/*
 * This function set up the master clock (MCLK).
 *
 * Input: Frequency to be set.
 */
void setMasterClock(void * ufb,unsigned long frequency)
{
    unsigned long ulReg, divisor;
	printk("%s:frequency = %lu\n",__func__,frequency);
    
    if (frequency != 0)
    {   
        /* Calcualte the divisor */
        divisor = (unsigned long) roundedDiv(getChipClock(ufb), frequency);
        
        /* Set the corresponding divisor in the register. */
        ulReg = peek32(ufb,CURRENT_GATE);
        switch(divisor)
        {
            default:
            case 3:
                ulReg = FIELD_SET(ulReg, CURRENT_GATE, MCLK, DIV_3);
                break;
            case 4:
                ulReg = FIELD_SET(ulReg, CURRENT_GATE, MCLK, DIV_4);
                break;
            case 6:
                ulReg = FIELD_SET(ulReg, CURRENT_GATE, MCLK, DIV_6);
                break;
            case 8:
                ulReg = FIELD_SET(ulReg, CURRENT_GATE, MCLK, DIV_8);
                break;
        }
        
        setCurrentGate(ufb,ulReg);
    }
}



/*
 * This function set up the memory clock.
 *
 * Input: Frequency to be set.
 */
void setMemoryClock(void * ufb,unsigned long frequency)
{
    unsigned long ulReg, divisor;
	printk("%s:frequency = %lu\n",__func__,frequency);
    
    if (frequency != 0)        
    {   
        /* Calcualte the divisor */
        divisor = (unsigned long) roundedDiv(getChipClock(ufb), frequency);
        
        /* Set the corresponding divisor in the register. */
        ulReg = peek32(ufb,CURRENT_GATE);
        switch(divisor)
        {   
            default:
            case 1:
                ulReg = FIELD_SET(ulReg, CURRENT_GATE, M2XCLK, DIV_1);
                break;
            case 2:
                ulReg = FIELD_SET(ulReg, CURRENT_GATE, M2XCLK, DIV_2);
                break;
            case 3:
                ulReg = FIELD_SET(ulReg, CURRENT_GATE, M2XCLK, DIV_3);
                break;
            case 4:
                ulReg = FIELD_SET(ulReg, CURRENT_GATE, M2XCLK, DIV_4);
                break;
        }
        
        setCurrentGate(ufb,ulReg);
    }
}

/* Perform a rounded division. 
 * For example, if the result is 4.5, this function returns 5.
 * If the result is 4.4, this function returns 4.
 */


/*
 * This function set up the main chip clock.
 *
 * Input: Frequency to be set.
 */
void setChipClock(void *ufb,unsigned long frequency)
{
    pll_value_t pll;
    unsigned long ulActualMxClk;
	printk("%s:frequency = %lu\n",__func__,frequency);

    if (frequency != 0)
    {
        /*
         * Set up PLL, a structure to hold the value to be set in clocks.
         */
        pll.inputFreq = DEFAULT_INPUT_CLOCK; /* Defined in CLOCK.H */
        pll.clockType = MXCLK_PLL;

        /* 
         * Call calcPllValue() to fill up the other fields for PLL structure.
         * Sometime, the chip cannot set up the exact clock required by User.
         * Return value from calcPllValue() gives the actual possible clock.
         */
        ulActualMxClk = calcPllValue(frequency, &pll);
    
        /* Master Clock Control: MXCLK_PLL */
        poke32(ufb,MXCLK_PLL_CTRL, formatPllReg(&pll));
    }
}

/*
 * This function gets the Main Chip Clock value.
 *
 * Output:
 *      The Actual Main Chip clock value.
 */
unsigned long getChipClock(void *ufb)
{
    pll_value_t pll;
    return getPllValue(ufb,MXCLK_PLL, &pll);
}

/*
 * Get the programmable PLL register value.
 *
 * Input:
 *      clockType   - The clock Type that the PLL is associated with.
 *      pPLL        - Pointer to a PLL structure to be filled with the
 *                    PLL value read from the register.
 * Output:
 *      The actual clock value calculated, together with the values of
 *      PLL register stored in the pPLL pointer.
 */
unsigned long getPllValue(void * ufb,clock_type_t clockType, pll_value_t *pPLL)
{
    unsigned long ulPllReg = 0;
    
    pPLL->inputFreq = DEFAULT_INPUT_CLOCK;
    pPLL->clockType = clockType;
    
    switch (clockType)
    {
		case MXCLK_PLL:
			ulPllReg = peek32(ufb,MXCLK_PLL_CTRL);
			break;
        case PANEL_PLL:
            ulPllReg = peek32(ufb,PANEL_PLL_CTRL);
            break;
        case CRT_PLL:
            ulPllReg = peek32(ufb,CRT_PLL_CTRL);
            break;
        case VGA0_PLL:
            ulPllReg = peek32(ufb,VGA_PLL0_CTRL);
            break;
        case VGA1_PLL:
            ulPllReg = peek32(ufb,VGA_PLL1_CTRL);
            break;
    }
    
	if(clockType != MXCLK_PLL){
		pPLL->M = FIELD_GET(ulPllReg, PANEL_PLL_CTRL, M);
		pPLL->N = FIELD_GET(ulPllReg, PANEL_PLL_CTRL, N);
		pPLL->OD = FIELD_GET(ulPllReg, PANEL_PLL_CTRL, OD);
		pPLL->POD = FIELD_GET(ulPllReg, PANEL_PLL_CTRL, POD);
	}else{
		pPLL->M = FIELD_GET(ulPllReg, MXCLK_PLL_CTRL, M);
		pPLL->N = FIELD_GET(ulPllReg, MXCLK_PLL_CTRL, N);
		pPLL->OD = FIELD_GET(ulPllReg,MXCLK_PLL_CTRL, OD);
		pPLL->POD = 0;
	}
    return calcPLL(pPLL);
}



/*
 * Given a requested clock frequency, this function calculates the 
 * best M, N & OD values for the PLL.
 * 
 * Input: Requested pixel clock in Hz unit.
 *        The followiing fields of PLL has to be set up properly:
 *        pPLL->clockType, pPLL->inputFreq.
 *
 * Output: Update the PLL structure with the proper M, N and OD values
 * Return: The actual clock in Hz that the PLL is able to set up.
 *
 * The PLL uses this formula to operate: 
 * requestClk = inputFreq * M / N / (2 to the power of OD) / (2 to the power of POD)
 *
 * The PLL specification mention the following restrictions:
 *      1 MHz <= inputFrequency / N <= 25 MHz
 *      200 MHz <= outputFrequency <= 1000 MHz --> However, it seems that it can 
 *                                                 be set to lower than 200 MHz.
 */
unsigned long calcPllValue(
unsigned long ulRequestClk, /* Required pixel clock in Hz unit */
pll_value_t *pPLL           /* Structure to hold the value to be set in PLL */
)
{
    unsigned long M, N, OD, POD = 0, diff, pllClk, odPower, podPower;
    unsigned long bestDiff = 0xffffffff; /* biggest 32 bit unsigned number */

    /* Init PLL structure to know states */
    pPLL->M = 0;
    pPLL->N = 0;
    pPLL->OD = 0;
    pPLL->POD = 0;

    /* Sanity check: None at the moment */

    /* Convert everything in Khz range in order to avoid calculation overflow */
    pPLL->inputFreq /= 1000;
    ulRequestClk /= 1000;

    /* The maximum of post divider is 8. */
    for (POD=0; POD<=3; POD++)
    {
    
        /* MXCLK_PLL does not have post divider. */
        if ((POD > 0) && (pPLL->clockType == MXCLK_PLL))
            break;
    
        /* Work out 2 to the power of POD */
        podPower = twoToPowerOfx(POD);
        
        /* OD has only 2 bits [15:14] and its value must between 0 to 3 */
        for (OD=0; OD<=3; OD++)
        {
            /* Work out 2 to the power of OD */
            odPower = twoToPowerOfx(OD);
            
            /* N has 4 bits [11:8] and its value must between 2 and 15. 
               The N == 1 will behave differently --> Result is not correct. */
            for (N=2; N<=15; N++)
            {
                /* The formula for PLL is ulRequestClk = inputFreq * M / N / (2^OD)
                   In the following steps, we try to work out a best M value given the others are known.
                   To avoid decimal calculation, we use 1000 as multiplier for up to 3 decimal places of accuracy.
                */
                M = ulRequestClk * N * odPower * 1000 / pPLL->inputFreq;
                M = roundedDiv(M, 1000);

                /* M field has only 8 bits, reject value bigger than 8 bits */
                if (M < 256)
                {
                    /* Calculate the actual clock for a given M & N */        
                    pllClk = pPLL->inputFreq * M / N / odPower / podPower;

                    /* How much are we different from the requirement */
                    diff = absDiff(pllClk, ulRequestClk);
        
                    if (diff < bestDiff)
                    {
                        bestDiff = diff;

                        /* Store M and N values */
                        pPLL->M  = M;
                        pPLL->N  = N;
                        pPLL->OD = OD;
                        pPLL->POD = POD;
                    }
                }
            }
        }
    }
    
    /* Restore input frequency from Khz to hz unit */
    pPLL->inputFreq *= 1000;
    ulRequestClk *= 1000;

    /* Output debug information */
    //DDKDEBUGPRINT((DISPLAY_LEVEL, "calcPllValue: Requested Frequency = %d\n", ulRequestClk));
    //DDKDEBUGPRINT((DISPLAY_LEVEL, "calcPllValue: Input CLK = %dHz, M=%d, N=%d, OD=%d, POD=%d\n", pPLL->inputFreq, pPLL->M, pPLL->N, pPLL->OD, pPLL->POD));

    /* Return actual frequency that the PLL can set */
    return calcPLL(pPLL);
}


/*
 * Set up the corresponding bit field of the programmable PLL register.
 *
 * Input: Pointer to PLL structure with type and values set up properly.
 *        Usually, calcPllValue() function will be called before this to calculate the values first.
 *
 */
inline unsigned long formatPllReg(pll_value_t *pPLL)
{
    unsigned long ulPllReg = 0;

    /* Note that all PLL's have the same format. Here, we just use Panel PLL parameter
       to work out the bit fields in the register.
       On returning a 32 bit number, the value can be applied to any PLL in the calling function.
    */
    ulPllReg =
        FIELD_SET(  0, PANEL_PLL_CTRL, BYPASS, OFF)
      | FIELD_SET(  0, PANEL_PLL_CTRL, POWER,  ON)
      | FIELD_SET(  0, PANEL_PLL_CTRL, INPUT,  OSC)
      | FIELD_VALUE(0, PANEL_PLL_CTRL, POD,    pPLL->POD)
      | FIELD_VALUE(0, PANEL_PLL_CTRL, OD,     pPLL->OD)
      | FIELD_VALUE(0, PANEL_PLL_CTRL, N,      pPLL->N)
      | FIELD_VALUE(0, PANEL_PLL_CTRL, M,      pPLL->M);

    return(ulPllReg);
}


/* This function calculates 2 to the power of x 
   Input is the power number.
 */
inline unsigned long twoToPowerOfx(unsigned long x)
{
    unsigned long i;
    unsigned long result = 1;

    for (i=1; i<=x; i++)
        result *= 2;
    return result;
}
inline unsigned long absDiff(unsigned long a, unsigned long b)
{
    if ( a > b )
        return(a - b);
    else
        return(b - a);	
}

/*
 * A local function to calculate the clock value of the given PLL.
 *
 * Input:
 *      Pointer to a PLL structure to be calculated based on the
 *      following formula:
 *      inputFreq * M / N / (2 to the power of OD) / (2 to the power of POD)
 */
inline unsigned long calcPLL(pll_value_t *pPLL)
{
    return (pPLL->inputFreq * pPLL->M / pPLL->N / twoToPowerOfx(pPLL->OD) / twoToPowerOfx(pPLL->POD));
}


/* 
 * This function gets the available clock type
 *
 */
clock_type_t getClockType(disp_control_t dispCtrl)
{
	clock_type_t clockType;    
	switch (dispCtrl)
	{
		case PANEL_CTRL:
			clockType = PANEL_PLL;
			break;
		case VGA_CTRL:
			clockType = VGA0_PLL;
		default:
		case CRT_CTRL:
			clockType = CRT_PLL;
			break;
	}            
	return clockType;
}


/*
 * Checks if the display control is swapped
 */
unsigned long isDisplayControlSwapped()
{
    return 0;
}

