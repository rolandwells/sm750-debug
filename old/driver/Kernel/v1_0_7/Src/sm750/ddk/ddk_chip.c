#include "ddk750_common.h"

#if 0
inline unsigned long roundedDiv(ulong,ulong)
{
	return (2 * num + denom) / (2 * denom);
}
#endif


#if 0
/* below extern val should be in sm750hw.c */
extern unsigned char __iomem * pVMEM;
extern unsigned char __iomem * pMMIO;
unsigned long peekRegisterDWord(unsigned long offset)
{
	return readl(pMMIO + offset);
}

void pokeRegisterDWord(unsigned long offset,data)
{
	writel(data,pMMIO+offset);
}
#endif

unsigned long ddk750_getFrameBufSize()
{
	unsigned long sizeSymbol, memSize;
    	sizeSymbol = FIELD_GET(peekRegisterDWord(MISC_CTRL), MISC_CTRL, LOCALMEM_SIZE);
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
long ddk750_initChipParm(initchip_param_t *pInitParam)
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
	setPowerMode(pInitParam->powerMode);

	/* Enable display power gate & LOCALMEM power gate*/
   	ulReg = peekRegisterDWord(CURRENT_GATE);
	ulReg = FIELD_SET(ulReg, CURRENT_GATE, DISPLAY, ON);
	ulReg = FIELD_SET(ulReg,CURRENT_GATE,LOCALMEM,ON);	
	setCurrentGate(ulReg);

	/*	set panel pll and graphic mode	*/
	ulReg = peekRegisterDWord(VGA_CONFIGURATION);
	ulReg = FIELD_SET(ulReg,VGA_CONFIGURATION,PLL,PANEL);
	ulReg = FIELD_SET(ulReg,VGA_CONFIGURATION,MODE,GRAPHIC);
	pokeRegisterDWord(VGA_CONFIGURATION,ulReg);

    
#ifndef VALIDATION_CHIP
    /* Set the Main Chip Clock */
    setChipClock(MHz(pInitParam->chipClock));
#endif

    /* Set up memory clock. */
    setMemoryClock(MHz(pInitParam->memClock));

    /* Set up master clock */
    setMasterClock(MHz(pInitParam->masterClock));
    
#ifdef VALIDATION_CHIP
    /* Patch up SM750 for validation purpose. Disable the test Clock after
       setting the power mode
     */
    ulReg = peekRegisterDWord(MISC_CTRL);
    ulReg = FIELD_SET(ulReg, MISC_CTRL, CLK_SELECT, OSC);
    pokeRegisterDWord(MISC_CTRL, ulReg);
#endif    
    
    /* Reset the memory controller. If the memory controller is not reset in SM750, 
       the system might hang when sw accesses the memory. 
       The memory should be resetted after changing the MXCLK.
     */
    if (pInitParam->resetMemory == 1)
    {     
        ulReg = peekRegisterDWord(MISC_CTRL);
        ulReg = FIELD_SET(ulReg, MISC_CTRL, LOCALMEM_RESET, RESET);
        pokeRegisterDWord(MISC_CTRL, ulReg);
    
        ulReg = FIELD_SET(ulReg, MISC_CTRL, LOCALMEM_RESET, NORMAL);
        pokeRegisterDWord(MISC_CTRL, ulReg);    
    }
    
    if (pInitParam->setAllEngOff == 1)
    {
        enable2DEngine(0);

        /* Disable Overlay, if a former application left it on */
        ulReg = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, VIDEO_DISPLAY_CTRL, PLANE, DISABLE); 
        pokeRegisterDWord(VIDEO_DISPLAY_CTRL, ulReg);

        /* Disable video alpha, if a former application left it on */
        ulReg = peekRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, VIDEO_ALPHA_DISPLAY_CTRL, PLANE, DISABLE); 
        pokeRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL, ulReg);

        /* Disable alpha plane, if a former application left it on */
        ulReg = peekRegisterDWord(ALPHA_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, ALPHA_DISPLAY_CTRL, PLANE, DISABLE); 
        pokeRegisterDWord(ALPHA_DISPLAY_CTRL, ulReg);
#if 0
        /* Disable LCD hardware cursor, if a former application left it on */
        ulReg = peekRegisterDWord(PANEL_HWC_ADDRESS);
        ulReg = FIELD_SET(ulReg, PANEL_HWC_ADDRESS, ENABLE, DISABLE); 
        pokeRegisterDWord(PANEL_HWC_ADDRESS, ulReg);

        /* Disable CRT hardware cursor, if a former application left it on */
        ulReg = peekRegisterDWord(CRT_HWC_ADDRESS);
        ulReg = FIELD_SET(ulReg, CRT_HWC_ADDRESS, ENABLE, DISABLE); 
        pokeRegisterDWord(CRT_HWC_ADDRESS, ulReg);

        /* Disable ZV Port 0, if a former application left it on */
        ulReg = peekRegisterDWord(ZV0_CAPTURE_CTRL);
        ulReg = FIELD_SET(ulReg, ZV0_CAPTURE_CTRL, CAP, DISABLE); 
        pokeRegisterDWord(ZV0_CAPTURE_CTRL, ulReg);

        /* Disable ZV Port 1, if a former application left it on */
        ulReg = peekRegisterDWord(ZV1_CAPTURE_CTRL);
        ulReg = FIELD_SET(ulReg, ZV1_CAPTURE_CTRL, CAP, DISABLE); 
        pokeRegisterDWord(ZV1_CAPTURE_CTRL, ulReg);
      
        /* Disable ZV Port Power, if a former application left it on */
        enableZVPort(0);
#endif  
#ifdef DMA
        /* Disable DMA Channel, if a former application left it on */
        ulReg = peekRegisterDWord(DMA_ABORT_INTERRUPT);
        ulReg = FIELD_SET(ulReg, DMA_ABORT_INTERRUPT, ABORT_1, ABORT);
        pokeRegisterDWord(DMA_ABORT_INTERRUPT, ulReg);
        
        /* Disable DMA Power, if a former application left it on */
        enableDMA(0);
#endif


    }	

    /* We can add more initialization as needed. */
        
    return 0;
}


/*
 * This function set up the master clock (MCLK).
 *
 * Input: Frequency to be set.
 */
void setMasterClock(unsigned long frequency)
{
#ifdef VALIDATION_CHIP
    unsigned long ulReg;

    /* There are only 4 choices for master clock and each choice has a fixed
       bit field position in the Power Gate register.
    */
    if (frequency != 0)
    {
        ulReg = peekRegisterDWord(CURRENT_GATE);
        
        if (frequency > MHz(84))
        {
            /* Range of 84 or above, set to 110 MHz */
            ulReg = FIELD_SET(ulReg, CURRENT_GATE, MCLK, 112MHZ);
        }
        else if ((frequency > MHz(56)) && (frequency <= MHz(84)))
        {
            /* Range between 56 and 84, set to 82 MHz */
            ulReg = FIELD_SET(ulReg, CURRENT_GATE, MCLK, 84MHZ);
        }
        else if ((frequency > MHz(42)) && (frequency <= MHz(56)))
        {
            /* Range between 42 and 56, set to 56 MHz */
            ulReg = FIELD_SET(ulReg, CURRENT_GATE, MCLK, 56MHZ);
        }
        else
        {
            /* Range below or equal 42, set to 42 MHz */
            ulReg = FIELD_SET(ulReg, CURRENT_GATE, MCLK, 42MHZ);
        }
        
        setCurrentGate(ulReg);
    }
    
    //DDKDEBUGPRINT((INIT_LEVEL, "setMasterClock: Current Clock Status Register (0x000040) = 0x%08x\n", peekRegisterDWord(CURRENT_GATE)));
#else
    unsigned long ulReg, divisor;
    
    if (frequency != 0)
    {   
        /* Calcualte the divisor */
        divisor = (unsigned long) roundedDiv(getChipClock(), frequency);
        
        /* Set the corresponding divisor in the register. */
        ulReg = peekRegisterDWord(CURRENT_GATE);
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
        
        setCurrentGate(ulReg);
    }
#endif
}



/*
 * This function set up the memory clock.
 *
 * Input: Frequency to be set.
 */
void setMemoryClock(unsigned long frequency)
{
#ifdef VALIDATION_CHIP
    unsigned long ulReg;

    /* There are only 4 choices for memory clock and each choice has a fixed
       bit field position in the Power Gate register.
       Since there might be different input on the clock to be set, therefore,
       it is better to use a clock range. 
    */
    if (frequency != 0)
    {
        ulReg = peekRegisterDWord(CURRENT_GATE);
        
        if (frequency > MHz(168))
        {
            /* Range between 168 or above, set to 329 MHz */
            ulReg = FIELD_SET(ulReg, CURRENT_GATE, M2XCLK, 336MHZ);
        }
        else if ((frequency > MHz(112)) && (frequency <= MHz(168)))
        {
            /* Range between 112 and 168, set to 165 MHz */
            ulReg = FIELD_SET(ulReg, CURRENT_GATE, M2XCLK, 168MHZ);
        }
        else if ((frequency > MHz(84)) && (frequency <= MHz(112)))
        {
            /* Range between 84 and 112, set to 110 MHz */
            ulReg = FIELD_SET(ulReg, CURRENT_GATE, M2XCLK, 112MHZ);
        }
        else
        {
            /* Range below or equal 84, set to 82 MHz */
            ulReg = FIELD_SET(ulReg, CURRENT_GATE, M2XCLK, 84MHZ);
        }
        
        setCurrentGate(ulReg);
    }
    
#else
    unsigned long ulReg, divisor;
    
    if (frequency != 0)        
    {   
        /* Calcualte the divisor */
        divisor = (unsigned long) roundedDiv(getChipClock(), frequency);
        
        /* Set the corresponding divisor in the register. */
        ulReg = peekRegisterDWord(CURRENT_GATE);
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
        
        setCurrentGate(ulReg);
    }
#endif
}

/* Perform a rounded division. 
 * For example, if the result is 4.5, this function returns 5.
 * If the result is 4.4, this function returns 4.
 */


#ifdef VALIDATION_CHIP
void setChipClock(unsigned long frequency)
{
}

/*
 * This function gets the Main Chip Clock value.
 *
 * Output:
 *      The Actual Main Chip clock value.
 */
unsigned long getChipClock()
{
    return (DEFAULT_INPUT_CLOCK * 23);
}
#else
/*
 * This function set up the main chip clock.
 *
 * Input: Frequency to be set.
 */
void setChipClock(unsigned long frequency)
{
    pll_value_t pll;
    unsigned long ulActualMxClk;

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
        //DDKDEBUGPRINT((INIT_LEVEL, "setMemoryClock: frequency = 0x%08x\n", frequency));
        ulActualMxClk = calcPllValue(frequency, &pll);
        //DDKDEBUGPRINT((INIT_LEVEL, "setMemoryClock: Current Clock = 0x%08x\n", ulActualMxClk));
    
        /* Master Clock Control: MXCLK_PLL */
        pokeRegisterDWord(MXCLK_PLL_CTRL, formatPllReg(&pll));
    }
}

/*
 * This function gets the Main Chip Clock value.
 *
 * Output:
 *      The Actual Main Chip clock value.
 */
unsigned long getChipClock()
{
    pll_value_t pll;
    return getPllValue(MXCLK_PLL, &pll);
}
#endif


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
unsigned long getPllValue(clock_type_t clockType, pll_value_t *pPLL)
{
    unsigned long ulPllReg = 0;
    
    pPLL->inputFreq = DEFAULT_INPUT_CLOCK;
    pPLL->clockType = clockType;
    
    switch (clockType)
    {
#ifndef VALIDATION_CHIP
        case MXCLK_PLL:
            ulPllReg = peekRegisterDWord(MXCLK_PLL_CTRL);
            break;
#endif
        case PANEL_PLL:
            ulPllReg = peekRegisterDWord(PANEL_PLL_CTRL);
            break;
        case CRT_PLL:
            ulPllReg = peekRegisterDWord(CRT_PLL_CTRL);
            break;
        case VGA0_PLL:
            ulPllReg = peekRegisterDWord(VGA_PLL0_CTRL);
            break;
        case VGA1_PLL:
            ulPllReg = peekRegisterDWord(VGA_PLL1_CTRL);
            break;
    }
    
    pPLL->M = FIELD_GET(ulPllReg, PANEL_PLL_CTRL, M);
    pPLL->N = FIELD_GET(ulPllReg, PANEL_PLL_CTRL, N);
    pPLL->OD = FIELD_GET(ulPllReg, PANEL_PLL_CTRL, OD);
    
#ifdef VALIDATION_CHIP
    if (pPLL->OD > 2)
        pPLL->POD = 2;
    else
        pPLL->POD = pPLL->OD;
#else
    pPLL->POD = FIELD_GET(ulPllReg, PANEL_PLL_CTRL, POD);
#endif    

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

#ifndef VALIDATION_CHIP
    /* The maximum of post divider is 8. */
    for (POD=0; POD<=3; POD++)
#endif    
    {
    
#ifndef VALIDATION_CHIP
        /* MXCLK_PLL does not have post divider. */
        if ((POD > 0) && (pPLL->clockType == MXCLK_PLL))
            break;
#endif
    
        /* Work out 2 to the power of POD */
        podPower = twoToPowerOfx(POD);
        
        /* OD has only 2 bits [15:14] and its value must between 0 to 3 */
        for (OD=0; OD<=3; OD++)
        {
            /* Work out 2 to the power of OD */
            odPower = twoToPowerOfx(OD);
            
#ifdef VALIDATION_CHIP
            if (odPower > 4)
                podPower = 4;
            else
                podPower = odPower;
#endif            

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

#ifdef VALIDATION_CHIP
                        if (OD > 2)
                            POD = 2;
                        else
                            POD = OD;
#endif
                        
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
#ifndef VALIDATION_CHIP
      | FIELD_VALUE(0, PANEL_PLL_CTRL, POD,    pPLL->POD)
#endif      
      | FIELD_VALUE(0, PANEL_PLL_CTRL, OD,     pPLL->OD)
      | FIELD_VALUE(0, PANEL_PLL_CTRL, N,      pPLL->N)
      | FIELD_VALUE(0, PANEL_PLL_CTRL, M,      pPLL->M);

    //DDKDEBUGPRINT((DISPLAY_LEVEL, "formatPllReg: PLL register value = 0x%08x\n", ulPllReg));

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

long ddk750_dumpRegisters(unsigned long start,unsigned long end,pf_output  pf,char * bufSrc)
{
	int add;
	char buf[1024];
	pf(buf,"--+ start dump +--\n");
	strcat(bufSrc,buf);
	for(add=start;add<=((end)&~15);add+=0x10)
	{
		pf(buf,"0x%08x : %08x,%08x,%08x,%08x \n",add,
			peekRegisterDWord(add),
			peekRegisterDWord(add+0x4),
			peekRegisterDWord(add+0x8),
			peekRegisterDWord(add+0xc));
		strcat(bufSrc,buf);
	}
	return 0;
	
}

/*
 * Checks if the display control is swapped
 */
unsigned long isDisplayControlSwapped()
{
    unsigned long value;    
    value = getScratchData(SWAP_DISPLAY_CTRL_FLAG);    
    if (value != (-1))
    	return value;    
    return 0;
}


/*
 * Get Scratch Data
 */
unsigned long getScratchData(
    unsigned long dataFlag
)
{
    unsigned char ucValue;
    
    switch (dataFlag)
    {
        case SWAP_DISPLAY_CTRL_FLAG:
            ucValue = peekRegisterByte(SCRATCH_DATA);
            if (ucValue & 0x01)
                return 1;
            else
                return 0;
            break;
        default:
            /* Do nothing */
            break;
    }
    
    return (-1);
}


