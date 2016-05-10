/* Header:   //Mercury/Projects/archives/XFree86/4.0/smi_driver.c-arc   1.42   30 March 2009 13:52:16   T-bag  $ */

/*
   Copyright (C) 1994-1999 The XFree86 Project, Inc.  All Rights Reserved.
   Copyright (C) 2000 Silicon Motion, Inc.  All Rights Reserved.

   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
   NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
   XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
   AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   Except as contained in this notice, the names of The XFree86 Project and
   Silicon Motion shall not be used in advertising or otherwise to promote the
   sale, use or other dealings in this Software without prior written
   authorization from The XFree86 Project or Silicon Motion.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/siliconmotion/smi_driver.c,v 1.30 2003/04/23 21:51:44 tsi Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "smi.h"
#include "smi_750.h"

#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
#include "xf86Resources.h"
#include "xf86RAC.h"
#endif

#include "xf86DDC.h"
#include "xf86int10.h"
#include "vbe.h"
#include "shadowfb.h"


#include "globals.h"
#if XORG_VERSION_CURRENT > XORG_VERSION_NUMERIC(1,7,0,0,0)
#include <X11/extensions/dpmsconst.h>
#else
#define DPMS_SERVER
#include  <X11/extensions/dpms.h>
#endif

/*
 * Forward definitions for the functions that make up the driver.
 */

/*
   static void SMI_UnmapMem(ScrnInfoPtr pScrn);
   static Bool SMI_MSOCSetMode(ScrnInfoPtr pScrn, DisplayModePtr mode);
 */


void setDPMS_750 (SMIPtr pSmi, DPMS_t state);
extern ULONG regRead32 (SMIPtr pSmi, ULONG nOffset);
extern void regWrite32 (SMIPtr pSmi, ULONG nOffset, ULONG nData);
extern LONG roundDiv (LONG num, LONG denom);
extern unsigned int	 total_video_memory_k;
/*
 * Add comment here about this module. 
 */

extern int hw_rev;

#if 1
/*PLL table*/
struct pll_value_t *pll_table;

#endif

/* Mode table. */
mode_table_t mode_table_750[] = {
    /*----------------------------------------------------------------------------------------
     * H.	H.    H.     H.   H.        V.   V.    V.    V.   V.        Pixel     H.     V.
     * tot.	disp. sync   sync sync      tot. disp. sync  sync sinc      clock     freq.  freq.
     *      end   start  wdth polarity       end   start hght polarity
     *---------------------------------------------------------------------------------------*/
    // 320 x 240
    {351, 320, 335, 8, NEGATIVE, 263, 240, 254, 2, NEGATIVE,
        6000000, 16000, 60, 2, 8, 0, 1},
    //mill -- add Hitachi Mono STN 320x240 panel 
    {351, 320, 335, 8, NEGATIVE, 263, 240, 254, 2, NEGATIVE,
        6000000, 16000, 75, 2, 8, 0, 1},
    // 400 x 300
    {528, 400, 420, 64, NEGATIVE, 314, 300, 301, 2, NEGATIVE,
        10000000, 18940, 60, 5, 6, 1, 1},

    // 640 x 480
    {800, 640, 656, 96, NEGATIVE, 525, 480, 490, 2, NEGATIVE,
        25175000, 31469, 60, 21, 10, 1, 1},
    {832, 640, 664, 40, NEGATIVE, 520, 480, 489, 3, NEGATIVE,
        31500000, 37861, 72, 21, 8, 1, 1},
    {840, 640, 656, 64, NEGATIVE, 500, 480, 481, 3, NEGATIVE,
        31500000, 37500, 75, 21, 8, 1, 1},
    {832, 640, 696, 56, NEGATIVE, 509, 480, 481, 3, NEGATIVE,
        36000000, 43269, 85, 3, 2, 0, 1},

    // 720 x 540
    {900, 720, 740, 96, POSITIVE, 576, 540, 545, 2, POSITIVE,
        31100000, 34560, 60, 13, 5, 1, 1},

    // 720 x 480
    {900, 720, 736, 72, POSITIVE, 497, 480, 481, 3, POSITIVE,
        26719000, 29820, 60, 11, 10, 0, 1},

    // 800 x 480
    /*
       {1056, 800, 864, 56, POSITIVE, 525, 480, 490, 2, POSITIVE,
       40000000, 31600, 60, 5, 3, 0, 1},	//72HZ
     */

    // 800 x 600
    {1024, 800, 824, 72, POSITIVE, 625, 600, 601, 2, POSITIVE,
        36000000, 35156, 56, 3, 2, 0, 1},
    {1056, 800, 840, 128, POSITIVE, 628, 600, 601, 4, POSITIVE,
        40000000, 37879, 60, 5, 3, 0, 1},
    {1040, 800, 856, 120, POSITIVE, 666, 600, 637, 6, POSITIVE,
        50000000, 48077, 72, 25, 6, 1, 1},
    {1056, 800, 816, 80, POSITIVE, 625, 600, 601, 3, POSITIVE,
        49500000, 46875, 75, 33, 4, 1, 0},
    {1048, 800, 832, 64, POSITIVE, 631, 600, 601, 3, POSITIVE,
        56250000, 53674, 85, 75, 8, 1, 0},

    //800 x 480 -- mill.chen
    {992, 800, 816, 80, POSITIVE, 497, 480, 481, 3, POSITIVE,
        29581000, 29820, 60, 5, 2, 1, 1},
    {1024, 800, 832, 80, POSITIVE, 500, 480, 481, 3, POSITIVE,
        35840000, 35000, 70, 6, 2, 1, 1},
    {1024, 800, 832, 80, POSITIVE, 502, 480, 481, 3, POSITIVE,
        38554000, 37650, 75, 45, 7, 1, 0},
    {1040, 800, 840, 80, POSITIVE, 505, 480, 481, 3, POSITIVE,
        44642000, 42925, 85, 67, 9, 1, 0},

    // 1024 x 600
    /*
       {1344, 1024, 1048, 136, POSITIVE, 628, 600, 601, 4, POSITIVE,
       50600000, 37679, 60, 19, 9, 0, 1},
     */

    //
    // 1024 x 768
    {1344, 1024, 1048, 136, NEGATIVE, 806, 768, 771, 6, NEGATIVE,
        65000000, 48363, 60, 65, 6, 1, 0},
    {1328, 1024, 1048, 136, NEGATIVE, 806, 768, 771, 6, NEGATIVE,
        75000000, 56476, 70, 25, 2, 1, 0},
    {1312, 1024, 1040, 96, POSITIVE, 800, 768, 769, 3, POSITIVE,
        78750000, 60023, 75, 105, 8, 1, 0},
    {1376, 1024, 1072, 96, POSITIVE, 808, 768, 769, 3, POSITIVE,
        94500000, 68677, 85, 63, 4, 1, 0},

    //1024 x 600 -- mill.chen
    {1312, 1024, 1064, 104, POSITIVE, 622, 600, 601, 3, POSITIVE,
        48964000, 37320, 60, 49, 6, 1, 0},
    {1328, 1024, 1072, 104, POSITIVE, 625, 600, 601, 3, POSITIVE,
        58100000, 43750, 70, 97, 10, 1, 0},
    {1344, 1024, 1080, 104, POSITIVE, 627, 600, 601, 3, POSITIVE,
        63202000, 47025, 75, 95, 9, 1, 0},
    {1360, 1024, 1080, 112, POSITIVE, 630, 600, 601, 3, POSITIVE,
        72828000, 53550, 85, 85, 7, 1, 0},


    // 1280 x 960
    {1800, 1280, 1376, 112, POSITIVE, 1000, 960, 961, 3, POSITIVE,
        108000000, 60000, 60, 9, 2, 0, 1},

    //1280 x 960
    {1728, 1280, 1344, 160, POSITIVE, 1011, 960, 961, 3, POSITIVE,
        148500000, 85938, 85, 99, 4, 1, 0},

    // 1280 x 1024
    {1688, 1280, 1328, 112, POSITIVE, 1066, 1024, 1025, 3, POSITIVE,
        108000000, 63981, 60, 9, 2, 0, 1},
    {1728, 1280, 1368, 136, POSITIVE, 1066, 1024, 1025, 3, POSITIVE,
        138540000, 79976, 75, 0, 0, 0, 0}, //we don't use last four digits here, so fill all zero into these four blanks

#if 0
    {1688, 1280, 1296, 144, POSITIVE, 1066, 1024, 1025, 3, POSITIVE,
        135000000, 79976, 75, 0, 0, 0, 0}, //we don't use last four digits here, so fill all zero into these four blanks 	
#endif
    {1744, 1280, 1376, 136, POSITIVE, 1075, 1024, 1025, 3, POSITIVE,
        159360000, 91146, 85, 0, 0, 0, 0},//we don't use last four digits here, so fill all zero into these four blanks

#if  0
    {1728, 1280, 1344, 160, POSITIVE, 1072, 1024, 1025, 3, POSITIVE,
        157500000, 91146, 85, 0, 0, 0, 0},//we don't use last four digits here, so fill all zero into these four blanks	
#endif
    //1280 x 768 --mill.chen
    {1680, 1280, 1344, 136, POSITIVE, 795, 768, 769, 3, POSITIVE,
        80136000, 47700, 60, 10, 3, 0, 1},
    {1712, 1280, 1360, 136, POSITIVE, 802, 768, 768, 3, POSITIVE,
        102997000, 60150, 75, 86, 5, 1, 0},

    // 1280 x 720 -- Belcon(12/19/2008)
    {1664, 1280, 1336, 136, POSITIVE, 746, 720, 721, 3, POSITIVE,
        74481000, 44760, 60, 28, 9, 0, 1},
    {1696, 1280, 1352, 136, POSITIVE, 752, 720, 721, 3, POSITIVE,
        95654000, 56400, 75, 4, 1, 0, 1},
    {1712, 1280, 1360, 136, POSITIVE, 756, 720, 721, 3, POSITIVE,
        110013000, 64260, 85, 55, 3, 1, 1},

    // 1360 x768 -- Belcon(12/10/2008)
    {1776, 1360, 1424, 144, POSITIVE, 795, 768, 769, 3, POSITIVE,
        84720000, 47703, 60, 39, 11, 0, 1},
    {1808, 1360, 1440, 144, POSITIVE, 802, 768, 769, 3, POSITIVE,
        108750000, 91146, 75, 0, 0, 0, 0},//we don't use last four digits here, so fill all zero into these four blanks
    {1824, 1360, 1448, 144, POSITIVE, 807, 768, 769, 3, POSITIVE,
        125120000, 91146, 85, 0, 0, 0, 0},//we don't use last four digits here, so fill all zero into these four blanks		

    // 1366 x768 -- Cheok(7/26/2007)
    {1722, 1366, 1424, 112, POSITIVE, 784, 768, 769, 3, POSITIVE,
        81000000, 47038, 60, 27, 8, 0, 1},

    // 1440x960 -- Belcon (12/22/2008)
    {1920, 1440, 1528, 152, POSITIVE, 994, 960, 961, 3, POSITIVE,
        114509000, 59640, 60, 24, 5, 0, 1},
    {1952, 1440, 1536, 160, POSITIVE, 1002, 960, 961, 3, POSITIVE,
        146693000, 75150, 75, 49, 4, 1, 1},
    {1952, 1440, 1536, 160, POSITIVE, 1008, 960, 961, 3, POSITIVE,
        167247000, 85680, 85, 7, 1, 0, 1},

    // 1600x1200 -- T-Bag (24/8/2009) 	
    {2160, 1600, 1664, 192, POSITIVE, 1250, 1200, 1201, 3, POSITIVE,
        162000000, 75000, 60, 0, 0, 0, 0},//we don't use last four digits here, so fill all zero into these four blanks
    {2192, 1600, 1720, 176, POSITIVE, 1253, 1200, 1201, 3, POSITIVE,
        205990000, 93750, 75, 0, 0, 0, 0},//we don't use last four digits here, so fill all zero into these four blanks


    // 1920x1080 -- T-Bag (24/8/2009)	
    {2576, 1920, 2040, 208, NEGATIVE, 1118, 1080, 1081, 3, POSITIVE,
        172800000, 67080, 60, 0, 0, 0, 0},//we don't use last four digits here, so fill all zero into these four blanks
    {2608, 1920, 2056, 208, NEGATIVE, 1128, 1080, 1081, 3, POSITIVE,
        220640000, 84600, 75, 0, 0, 0, 0},//we don't use last four digits here, so fill all zero into these four blanks

    // 1920x1200@60
    {2592, 1920, 2056, 200, NEGATIVE, 1245, 1200, 1203, 6, POSITIVE,
        193250000, 74556, 60, 0, 0, 0, 0},

    // 1920x1200@75
    {2608, 1920, 2056, 208, NEGATIVE, 1255, 1200, 1203, 6, POSITIVE,
        245250000, 94038, 75, 0, 0, 0, 0},

    // 1920x1200@85
    {2624, 1920, 2064, 208, NEGATIVE, 1262, 1200, 1203, 6, POSITIVE,
        281250000, 107184, 85, 0, 0, 0, 0},
    // 1920x1440@60
    {2600, 1920, 2048, 208, NEGATIVE, 1500, 1440, 1441, 3, POSITIVE,
	234000000, 90000, 60, 0, 0, 0, 0},

    // 1920x1440@75
    {2640, 1920, 2064, 224, NEGATIVE, 1500, 1440, 1441, 3, POSITIVE,
        297000000, 112500, 75, 0, 0, 0, 0},

    // End of table.
    {0, 0, 0, 0, NEGATIVE, 0, 0, 0, 0, NEGATIVE, 0, 0, 0, 0, 0, 0, 0},
};
mode_table_t mode_table_edid;

extern int	 entity_priv_index[MAX_ENTITIES];
extern int		free_video_memory;

/**********************************************************************
 * regRead32
 *    Read the value of the 32-bit register specified by nOffset
 **********************************************************************/
//ULONG
//regRead32 (SMIPtr pSmi, ULONG nOffset)
//{
// ULONG result;
// result = READ_SCR (pSmi, nOffset);
/* ErrorF("READ_SCR %8X from register %8X\n", result, nOffset); */
//return (result);
//}

/**********************************************************************
 * regWrite32
 *    Write the 32-bit value, nData, to the 32-bit register specified by
 *    nOffset
 **********************************************************************/
//void
//regWrite32 (SMIPtr pSmi, ULONG nOffset, ULONG nData)
//{
/* ErrorF("MMIOWriting %8X to register %8X\n", nData, nOffset);  */
//WRITE_SCR (pSmi, nOffset, nData);
//}


/* Perform a rounded division. */
//LONG
//roundDiv (LONG num, LONG denom)
//{
/* n / d + 1 / 2 = (2n + d) / 2d */
//return (2 * num + denom) / (2 * denom);
//}

/* Absolute differece between two numbers */
unsigned long absDiff(unsigned long a, unsigned long b)
{
    if ( a >= b )
        return(a - b);
    else
        return(b - a);
}

/* Finds clock closest to the requested. */
    LONG
findClock_750 (LONG requested_clock, clock_select_t * clock, display_t display)
{
    LONG mclk;
    INT divider, shift;
    LONG best_diff = 0x7fffffff;

    INT divby2 = 0, i = 0;

    divby2 = (hw_rev < 0xA0) ? 0 : 1;

    /* Try 288MHz and 336MHz clocks. */
    for (mclk = 288000000; mclk <= 336000000; mclk += 48000000)
    {
        /* For CRT, try dividers 1 and 3, for panel, try divider 5 as well. */
        for (divider = 1; divider <= (display == PANEL ? 5 : 3); divider += 2)
        {
            /* Try all 8 shift values. */
            for (shift = 0; shift < 8; shift++)
            {
                i = divby2;
                while (i >= 0)
                {
                    /* Calculate difference with requested clock. */
                    LONG diff =
                        roundDiv (mclk,
                                (divider << shift) << i) - requested_clock;
                    if (diff < 0)
                    {
                        diff = -diff;
                    }

                    // xf86DrvMsg("", X_INFO, "Belcon: mclk is %d, divider is %d,\n 2x V2XCLK is %d, v2_first is 0x%x, v2_second is 0x%x\n diff is %d( %d - %d)\n", mclk, divider << shift << i, (1-i), divider, shift, diff, roundDiv(mclk, (divider << shift) << i), requested_clock);

                    /* If the difference is less than the current, use it. */
                    if (diff < best_diff)
                    {
                        /* Store best difference. */
                        best_diff = diff;

                        /* Store clock values. */
                        clock->mclk = mclk;
                        clock->divider = divider;
                        clock->shift = shift;
                        clock->divby2 = 1 - i;
                        // xf86DrvMsg("", X_INFO, "Belcon: Best diff: mclk is %d, divider is %d,\n 2x V2XCLK is %d, v2_first is 0x%x, v2_second is 0x%x\n diff is %d( %d - %d)\n", mclk, divider << shift << i, (1-i), divider, shift, diff, roundDiv(mclk, (divider << shift) << i), requested_clock);
                    }
                    i--;
                }
            }
        }
    }

    /* Deleted by Belcon at Jul 20,2007. Not need now */
    /*
       clock->multipleM=requested_clock/(1024*1024);
       clock->dividerN=24;
     */

    /*
#ifdef AUTO_CLOCK
while (clock->multipleM>192){
clock->multipleM /= 2;
clock->dividerN /= 2;
}

if (hw_rev<0xC0)
return clock->mclk / (clock->divider << clock->shift);
else 
return  ( ((24000*clock->multipleM) / (clock->dividerN * (clock->divby2>=1?2:1)) )*1000);
#else
     */
return clock->mclk / (clock->divider << clock->shift);
// #endif

}


/* This function calculates 2 to the power of x 
 *    Input is the power number.
 *     */
unsigned long twoToPowerOfx(unsigned long x)
{
    unsigned long i;
    unsigned long result = 1;

    for (i=1; i<=x; i++)
        result *= 2;

    return result;
}


/*
 *  * Given a requested clock frequency, this function calculates the 
 *   * best M, N & OD values for the PLL.
 *    * 
 *     * Input: Requested pixel clock in Hz unit.
 *      *        The followiing fields of PLL has to be set up properly:
 *       *        pPLL->clockType, pPLL->inputFreq.
 *        *
 *         * Output: Update the PLL structure with the proper N, N and OD values
 *          * Return: The actual clock in Hz that the PLL is able to set up.
 *           *
 *            * The PLL uses this formula to operate: 
 *             * requestClk = inputFreq * M / N / (2 to the power of OD)
 *              *
 *               * The PLL specification mention the following restrictions:
 *                *      1 MHz <= inputFrequency / N <= 25 MHz
 *                 *      200 MHz <= outputFrequency <= 1000 MHz --> However, it seems that it can 
 *                  *                                                 be set lower than 200 MHz.
 *                   */
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

    pPLL->inputFreq = 14318180; 

    /* Sanity check: None at the moment */

    /* Convert everything in Khz range in order to avoid calculation overflow */
    pPLL->inputFreq /= 1000;
    ulRequestClk /= 1000;

#ifndef VALIDATION_CHIP
    /* The maximum of post divider is 4. */
    for (POD=0; POD<=2; POD++)
#endif    
    {
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
             *                The N == 1 will behave differently. */
            for (N=2; N<=15; N++)
            {
                /* The formula for PLL is ulRequestClk = inputFreq * M / N / (2^OD)
                 *                    In the following steps, we try to work out a best M value given the others are known.
                 *                                       To avoid decimal calculation, we use 1000 as multiplier for up to 3 decimal places of accuracy.
                 *                                                       */
                M = ulRequestClk * N * odPower * 1000 / pPLL->inputFreq;
                M = roundDiv(M, 1000);

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


    /* Return actual frequency that the PLL can set */
    return (pPLL->inputFreq * pPLL->M / pPLL->N / twoToPowerOfx(pPLL->OD) / twoToPowerOfx(pPLL->POD));
}  




/* Find the requested mode in the mode table. */
    mode_table_t *
findMode_750 (mode_table_t * mode_table, INT width, INT height, INT refresh_rate)
{
    /* Walk the entire mode table. */
    while (mode_table->pixel_clock != 0)
    {
        /* If this mode matches the requested mode, return it! */
        if ((mode_table->horizontal_display_end == width)
                && (mode_table->vertical_display_end == height)
                && (mode_table->vertical_frequency == refresh_rate))
        {
            return (mode_table);
        }

        /* Next entry in the mode table. */
        mode_table++;
    }

    /* No mode found. */
    return (NULL);
}

uint32_t roundedDiv(uint32_t num, uint32_t denom)
{
    /* n / d + 1 / 2 = (2n + d) / 2d */
    return (2 * num + denom) / (2 * denom);
}

#define DEFAULT_INPUT_CLOCK  14318181


static int32_t SMI_Getpll(pll_value_t* ppll,mode_table_t * pinmode_table)
{
    uint32_t uiActualPixelClk;    
    ppll->inputFreq = DEFAULT_INPUT_CLOCK;
    uiActualPixelClk = calcPllValue(pinmode_table->pixel_clock,ppll);
    return uiActualPixelClk;
}



void
SMI_DDCGetModes(DisplayModePtr Mode, mode_table_t * poutmode_table)
{
	poutmode_table->pixel_clock = Mode->Clock * 1000;      
	poutmode_table->horizontal_total = Mode->HTotal;
	poutmode_table->horizontal_display_end = Mode->HDisplay;
	poutmode_table->horizontal_sync_start = Mode->HSyncStart;
	poutmode_table->horizontal_sync_width = Mode->HSyncEnd - Mode->HSyncStart;
	poutmode_table->horizontal_sync_polarity = (Mode->Flags & V_PHSYNC)?POSITIVE:NEGATIVE;

	poutmode_table->vertical_total = Mode->VTotal;
	poutmode_table->vertical_display_end = Mode->VDisplay;
	poutmode_table->vertical_sync_start = Mode->VSyncStart;
	poutmode_table->vertical_sync_height = Mode->VSyncEnd - Mode->VSyncStart;
	poutmode_table->vertical_sync_polarity = (Mode->Flags & V_PVSYNC)?POSITIVE:NEGATIVE;

	poutmode_table->horizontal_frequency = roundedDiv(poutmode_table->pixel_clock,
                                        poutmode_table->horizontal_total);
	poutmode_table->vertical_frequency = roundedDiv(poutmode_table->horizontal_frequency,
                                      poutmode_table->vertical_total);
}

/* Converts the VESA timing into Voyager timing. */
    void
adjustMode_750 (mode_table_t * vesaMode, mode_table_t * mode, display_t display)
{
    LONG blank_width, sync_start, sync_width;
    clock_select_t clock;

    /* Calculate the VESA line and screen frequencies. */
    vesaMode->horizontal_frequency = roundDiv (vesaMode->pixel_clock,
            vesaMode->horizontal_total);
    vesaMode->vertical_frequency = roundDiv (vesaMode->horizontal_frequency,
            vesaMode->vertical_total);

    /* Calculate the sync percentages of the VESA mode. */
    blank_width = vesaMode->horizontal_total - vesaMode->horizontal_display_end;
    sync_start = roundDiv ((vesaMode->horizontal_sync_start -
                vesaMode->horizontal_display_end) * 100,
            blank_width);
    sync_width = roundDiv (vesaMode->horizontal_sync_width * 100, blank_width);

    /* Copy VESA mode into Voyager mode. */
    *mode = *vesaMode;
    // xf86DrvMsg("", X_INFO, "Belcon: adjustMode(), pixel clock %d, hfreq %d, vfreq %d, blank_width %d, sync_start %d, sync_width %d\n", mode->pixel_clock, mode->horizontal_total, mode->vertical_total, blank_width, sync_start, sync_width);
    /* Find the best pixel clock. */
    //      mode->pixel_clock = findClock(vesaMode->pixel_clock * 2, &clock, display) / 2;
    //      mode->pixel_clock = findClock(vesaMode->pixel_clock, &clock, display);
    mode->pixel_clock = vesaMode->pixel_clock;

    // xf86DrvMsg("", X_INFO, "Belcon: after findClock(), pixel_clock is %d\n", clock.mclk);
    /* Calculate the horizontal total based on the pixel clock and VESA line 
     * frequency.
     */
    if (hw_rev >= 0xA0)
    {
        mode->horizontal_total = roundDiv (vesaMode->pixel_clock,
                vesaMode->horizontal_frequency);
    }
    else
    {
        mode->horizontal_total = roundDiv (mode->pixel_clock,
                vesaMode->horizontal_frequency);
    }
    /* Calculate the sync start and width based on the VESA percentages. */
    blank_width = mode->horizontal_total - mode->horizontal_display_end;
    mode->horizontal_sync_start = mode->horizontal_display_end
        + roundDiv (blank_width * sync_start, 100);
    mode->horizontal_sync_width = roundDiv (blank_width * sync_width, 100);

    /* Calculate the line and screen frequencies. */
    mode->horizontal_frequency = roundDiv (mode->pixel_clock,
            mode->horizontal_total);
    mode->vertical_frequency = roundDiv (mode->horizontal_frequency,
            mode->vertical_total);
}



/* Fill the register structure. */
    void
setModeRegisters_750 (SMIPtr pSmi, reg_table_t * register_table,
        mode_table_t * mode, display_t display, INT bpp,
        INT fbPitch)
{
    clock_select_t clock = { 0, 0, 0, 0, 0, 0, 0 };
    unsigned long pllclock;

    findClock_750 (mode->pixel_clock, &clock, display);

    if (display == PANEL)
    {
        /* Set control register value. */
        register_table->control
            = (mode->vertical_sync_polarity == POSITIVE
                    ? FIELD_SET (0, PANEL_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH)
                    : FIELD_SET (0, PANEL_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW))
            | (mode->horizontal_sync_polarity == POSITIVE
                    ? FIELD_SET (0, PANEL_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH)
                    : FIELD_SET (0, PANEL_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW))
            | FIELD_SET (0, PANEL_DISPLAY_CTRL, TIMING, ENABLE)
            | FIELD_SET (0, PANEL_DISPLAY_CTRL, PLANE, ENABLE)
            | (bpp == 8
                    ? FIELD_SET (0, PANEL_DISPLAY_CTRL, FORMAT, 8)
                    : (bpp == 16
                        ? FIELD_SET (0, PANEL_DISPLAY_CTRL, FORMAT, 16)
                        : FIELD_SET (0, PANEL_DISPLAY_CTRL, FORMAT, 32)));


        /* Set timing registers. */
        register_table->horizontal_total
            = FIELD_VALUE (0, PANEL_HORIZONTAL_TOTAL, TOTAL,
                    mode->horizontal_total - 1)
            | FIELD_VALUE (0, PANEL_HORIZONTAL_TOTAL, DISPLAY_END,
                    mode->horizontal_display_end - 1);

        register_table->horizontal_sync
            = FIELD_VALUE (0, PANEL_HORIZONTAL_SYNC, WIDTH,
                    mode->horizontal_sync_width)
            | FIELD_VALUE (0, PANEL_HORIZONTAL_SYNC, START,
                    mode->horizontal_sync_start - 1);

        register_table->vertical_total
            = FIELD_VALUE (0, PANEL_VERTICAL_TOTAL, TOTAL,
                    mode->vertical_total - 1)
            | FIELD_VALUE (0, PANEL_VERTICAL_TOTAL, DISPLAY_END,
                    mode->vertical_display_end - 1);

        register_table->vertical_sync
            = FIELD_VALUE (0, PANEL_VERTICAL_SYNC, HEIGHT,
                    mode->vertical_sync_height)
            | FIELD_VALUE (0, PANEL_VERTICAL_SYNC, START,
                    mode->vertical_sync_start - 1);
    }
    else
    {
        /* Set control register value. */
        register_table->control
            = (mode->vertical_sync_polarity == POSITIVE
                    ? FIELD_SET (0, CRT_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH)
                    : FIELD_SET (0, CRT_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW))
            | (mode->horizontal_sync_polarity == POSITIVE
                    ? FIELD_SET (0, CRT_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH)
                    : FIELD_SET (0, CRT_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW))
            | FIELD_SET (0, CRT_DISPLAY_CTRL, SELECT, CRT)
            | FIELD_SET (0, CRT_DISPLAY_CTRL, TIMING, ENABLE)
            | FIELD_SET (0, CRT_DISPLAY_CTRL, PLANE, ENABLE)
            | (bpp == 8
                    ? FIELD_SET (0, CRT_DISPLAY_CTRL, FORMAT, 8)
                    : (bpp == 16
                        ? FIELD_SET (0, CRT_DISPLAY_CTRL, FORMAT, 16)
                        : FIELD_SET (0, CRT_DISPLAY_CTRL, FORMAT, 32)));

        /* Set timing registers. */
        register_table->horizontal_total
            = FIELD_VALUE (0, CRT_HORIZONTAL_TOTAL, TOTAL,
                    mode->horizontal_total - 1)
            | FIELD_VALUE (0, CRT_HORIZONTAL_TOTAL, DISPLAY_END,
                    mode->horizontal_display_end - 1);

        register_table->horizontal_sync
            = FIELD_VALUE (0, CRT_HORIZONTAL_SYNC, WIDTH,
                    mode->horizontal_sync_width)
            | FIELD_VALUE (0, CRT_HORIZONTAL_SYNC, START,
                    mode->horizontal_sync_start - 1);

        register_table->vertical_total
            = FIELD_VALUE (0, CRT_VERTICAL_TOTAL, TOTAL,
                    mode->vertical_total - 1)
            | FIELD_VALUE (0, CRT_VERTICAL_TOTAL, DISPLAY_END,
                    mode->vertical_display_end - 1);
        register_table->vertical_sync
            = FIELD_VALUE (0, CRT_VERTICAL_SYNC, HEIGHT,
                    mode->vertical_sync_height)
            | FIELD_VALUE (0, CRT_VERTICAL_SYNC, START,
                    mode->vertical_sync_start - 1);

    }

    /* Set up framebuffer pitch, from passed in value */
    register_table->fb_width = mode->horizontal_display_end * (bpp / 8);
    register_table->fb_width = (pSmi->lcdWidth * pSmi->Bpp + 15) & ~15;   /*fix the garbage screen bug after mode initing*/
    //register_table->fb_width = fbPitch;

    /* Calculate frame buffer width and height. */
    register_table->width = mode->horizontal_display_end;
    register_table->height = mode->vertical_display_end;

    /* Save display type. */
    register_table->display = display;
}

/* Program the mode with the registers specified. */
    void
programMode_750 (SMIPtr pSmi, reg_table_t * register_table)
{
    DWORD value, gate, clock;
    DWORD palette_ram;
    DWORD fb_size, offset;

    /* Get current power configuration. */
    gate = regRead32 (pSmi, CURRENT_GATE);
    gate = FIELD_SET (gate, CURRENT_GATE, VGA, ON);
    gate = FIELD_SET (gate, CURRENT_GATE, PWM, ON);
    gate = FIELD_SET (gate, CURRENT_GATE, I2C, ON);
    gate = FIELD_SET (gate, CURRENT_GATE, SSP, ON);
    gate = FIELD_SET (gate, CURRENT_GATE, GPIO, ON);
    gate = FIELD_SET (gate, CURRENT_GATE, ZVPORT, ON);
    gate = FIELD_SET (gate, CURRENT_GATE, CSC, ON);
    gate = FIELD_SET (gate, CURRENT_GATE, DE, ON);
    //gate = FIELD_SET (gate, CURRENT_GATE, DISPLAY, ON);
    gate = FIELD_SET (gate, CURRENT_GATE, LOCALMEM, ON);
    gate = FIELD_SET (gate, CURRENT_GATE, DMA, ON);




    /*Engine clock / Main clock */
    /*	Jul 20, 2007. Changed by Belcon to release PLL2 for V2CLK */
    //clock = FIELD_SET (clock, CURRENT_GATE, MCLK,112MHZ);
#if 0
    clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, MCLK_SELECT, 336);
    clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, MCLK_DIVIDER, 3);
    clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, MCLK_SHIFT, 0);
#endif
    /*
       clock = FIELD_SET(clock, CURRENT_POWER_CLOCK, MCLK_SELECT, 288);
       clock = FIELD_SET(clock, CURRENT_POWER_CLOCK, MCLK_DIVIDER, 1);
       clock = FIELD_SET(clock, CURRENT_POWER_CLOCK, MCLK_SHIFT, 1);
     */

    /*Memory clock */
    /*
     * FIXME:
     * Default memory clock is 144MHz. Currently, only 112MHz was supported.
     * 168MHz and 96MHz also should be supported. But just ignored here
     */

    pSmi->MCLK =  336000;
    if (pSmi->MCLK) {

        xf86DrvMsg("", X_INFO, "T-bag: here we are:  %d  pSmi->MCLK: %d\n", __LINE__,pSmi->MCLK);
        switch (pSmi->MCLK) {
            case 84000:
                gate = FIELD_SET(gate, CURRENT_GATE, M2XCLK, 84MHZ);
                /* 	
                        clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SELECT, 336);
                        clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_DIVIDER, 3);
                        clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SHIFT, 0);
                 */
                break;

            case 168000:
                gate = FIELD_SET( gate, CURRENT_GATE, M2XCLK, 168MHZ);
                /* 	
                        clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SELECT, 336);
                        clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_DIVIDER, 1);
                        clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SHIFT, 1);
                 */
                break;

            case 112000:
                gate = FIELD_SET( gate, CURRENT_GATE, M2XCLK, 112MHZ);
                /*
                   clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SELECT, 288);
                   clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_DIVIDER, 3);
                   clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SHIFT, 0);
                 */
                break;

            case 336000:
                gate = FIELD_SET( gate, CURRENT_GATE, M2XCLK, 336MHZ);
                break;
            default:
                gate = FIELD_SET( gate, CURRENT_GATE, M2XCLK, 336MHZ);	
                /*	
                        clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SELECT, 288);
                        clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_DIVIDER, 1);
                        clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SHIFT, 1);
                 */
                break;
        }
    } else {
        gate = FIELD_SET( gate, CURRENT_GATE, M2XCLK, 336MHZ);
        /*
           clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SELECT, 288);
           clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_DIVIDER, 1);
           clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SHIFT, 1);
         */
    }


    /* Program panel. */
    if (register_table->display == PANEL)
    {
        /* Program clock, enable display controller. */
        gate = FIELD_SET (gate, CURRENT_GATE, DISPLAY, ON);
        gate = FIELD_SET (gate, CURRENT_GATE, MCLK, 112MHZ);

        /*  
            clock &= FIELD_CLEAR (CURRENT_POWER_CLOCK, P2XCLK_SELECT)
            & FIELD_CLEAR (CURRENT_POWER_CLOCK, P2XCLK_DIVIDER)
            & FIELD_CLEAR (CURRENT_POWER_CLOCK, P2XCLK_SHIFT);
         */
        setPower_750 (pSmi, gate, clock | register_table->clock, 0);
        /* Calculate frame buffer address. */
        value = 0;
#if 0
        fb_size = register_table->fb_width * register_table->height;
        xf86DrvMsg("", X_INFO, "PANEL: fb_size is 0x%x\n", fb_size);
        if (FIELD_GET (regRead32 (pSmi, CRT_DISPLAY_CTRL),
                    CRT_DISPLAY_CTRL,
                    PLANE) == CRT_DISPLAY_CTRL_PLANE_ENABLE)
        {
            value = FIELD_GET (regRead32 (pSmi, CRT_FB_ADDRESS),
                    CRT_FB_ADDRESS, ADDRESS);
            xf86DrvMsg("", X_INFO, "PANEL: value is 0x%x\n", value);
            if (fb_size < value)
            {
                value = 0;
            }
            else
            {
                value += FIELD_GET (regRead32 (pSmi, CRT_FB_WIDTH),
                        CRT_FB_WIDTH,
                        OFFSET)
                    * (FIELD_GET (regRead32 (pSmi, CRT_VERTICAL_TOTAL),
                                CRT_VERTICAL_TOTAL, DISPLAY_END) + 1);
            }
        }
#endif
        value = pSmi->FBOffset;

        xf86DrvMsg("", X_INFO, "PANEL: value is 0x%x\n", value);
        /* Program panel registers. */
        regWrite32 (pSmi, PANEL_FB_ADDRESS,
                FIELD_SET (0, PANEL_FB_ADDRESS, STATUS, PENDING) |
                FIELD_SET (0, PANEL_FB_ADDRESS, EXT, LOCAL) |
                FIELD_VALUE (0, PANEL_FB_ADDRESS, ADDRESS, value));

        regWrite32 (pSmi, PANEL_FB_WIDTH,
                FIELD_VALUE (0, PANEL_FB_WIDTH, WIDTH,
                    register_table->fb_width) |
                FIELD_VALUE (0, PANEL_FB_WIDTH, OFFSET,
                    register_table->fb_width));

        regWrite32 (pSmi, PANEL_WINDOW_WIDTH,
                FIELD_VALUE (0, PANEL_WINDOW_WIDTH, WIDTH,
                    register_table->width) |
                FIELD_VALUE (0, PANEL_WINDOW_WIDTH, X, 0));

        regWrite32 (pSmi, PANEL_WINDOW_HEIGHT,
                FIELD_VALUE (0, PANEL_WINDOW_HEIGHT, HEIGHT,
                    register_table->height) |
                FIELD_VALUE (0, PANEL_WINDOW_HEIGHT, Y, 0));

        regWrite32 (pSmi, PANEL_PLANE_TL,
                FIELD_VALUE (0, PANEL_PLANE_TL, TOP, 0) |
                FIELD_VALUE (0, PANEL_PLANE_TL, LEFT, 0));

        regWrite32 (pSmi, PANEL_PLANE_BR,
                FIELD_VALUE (0, PANEL_PLANE_BR, BOTTOM,
                    register_table->height - 1) |
                FIELD_VALUE (0, PANEL_PLANE_BR, RIGHT,
                    register_table->width - 1));

        /*must set some registers before writing to 750 timing registers*/

        regWrite32 (pSmi, MISC_CTRL,FIELD_SET (regRead32 (pSmi, MISC_CTRL), MISC_CTRL,CLK_SELECT, OSC));
        //mandatory to set bits29:28 in register 0x80000 for smi750 with bios
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, FIELD_SET (regRead32 (pSmi, PANEL_DISPLAY_CTRL), PANEL_DISPLAY_CTRL,
                    SELECT, PANEL));

#if 1
        //     regWrite32 (pSmi, PANEL_DISPLAY_CTRL,
        //                      0x0f010106   );
        //  regWrite32 (pSmi, PANEL_PAN_CTRL,
        //                  0x00000000   );
        //  regWrite32 (pSmi, PANEL_COLOR_KEY,
        //                    0x010c0000   );
        //  regWrite32 (pSmi, MISC_CTRL,
        //                    0x01022060   );
        //  regWrite32 (pSmi,  MODE0_GATE,
        //                   0x00000406   );
        //regWrite32 (pSmi,  SYSTEM_CTRL,
        //                       0x20a00000   );

        regWrite32 (pSmi,  LOCALMEM_ARBITRATION,
                0x01765324  );

        regWrite32 (pSmi,  PCIMEM_ARBITRATION,
                0x01765324   );
        //regWrite32 (pSmi,  PANEL_PLL_CTRL,
        //                        0x00022b7b   );

        //value = regRead32 (pSmi, 0x0800f0);
        //regWrite32 (pSmi,  0x0800f0,
        //                        value&0x0fffffff   );
#endif
        //hardcode for smi750 with bios
        regWrite32 (pSmi,0x88,0x6);
        /*set 750 timing registers*/
        regWrite32 (pSmi, PANEL_HORIZONTAL_TOTAL,
                register_table->horizontal_total );
        regWrite32 (pSmi, PANEL_HORIZONTAL_SYNC,
                register_table->horizontal_sync );
        regWrite32 (pSmi, PANEL_VERTICAL_TOTAL,register_table->vertical_total );
        regWrite32 (pSmi, PANEL_VERTICAL_SYNC,register_table->vertical_sync );

        /* Program panel display control register. */
        value = regRead32 (pSmi, PANEL_DISPLAY_CTRL)
            & FIELD_CLEAR (PANEL_DISPLAY_CTRL, VSYNC_PHASE)
            & FIELD_CLEAR (PANEL_DISPLAY_CTRL, HSYNC_PHASE)
            & FIELD_CLEAR (PANEL_DISPLAY_CTRL, TIMING)
            & FIELD_CLEAR (PANEL_DISPLAY_CTRL, PLANE)
            & FIELD_CLEAR (PANEL_DISPLAY_CTRL, FORMAT);

        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, value | register_table->control);

        /* Palette RAM. */
        palette_ram = PANEL_PALETTE_RAM;

        /* Turn on panel. */
        panelPowerSequence_750 (pSmi, PANEL_ON, 4);
        /* Turn on DAC          -- Belcon */
        regWrite32 (pSmi, MISC_CTRL,
                FIELD_SET (regRead32 (pSmi, MISC_CTRL), MISC_CTRL,
                    DAC_POWER, ON));
#if 0   
        regWrite32 (pSmi, CRT_DISPLAY_CTRL,
                FIELD_SET (regRead32 (pSmi, CRT_DISPLAY_CTRL),
                    CRT_DISPLAY_CTRL, SELECT, PANEL));
#endif
        xf86DrvMsg("", X_INFO, "T-bag: here we are:  %d\n", __LINE__);
        return 0;   
    }

    /* Program CRT. */
    else
    {
        regWrite32 (pSmi, MISC_CTRL,
                FIELD_SET (regRead32 (pSmi, MISC_CTRL), MISC_CTRL,
                    DAC_POWER, ON));
        gate = FIELD_SET (gate, CURRENT_GATE, DISPLAY, ON);
        gate = FIELD_SET (gate, CURRENT_GATE, MCLK, 112MHZ);
        setPower_750 (pSmi, gate, clock | register_table->clock, 0);




        value = 0;
        value = pSmi->FBOffset;
        regWrite32 (pSmi, CRT_FB_ADDRESS,
                FIELD_SET (0, CRT_FB_ADDRESS, STATUS, PENDING) |
                FIELD_SET (0, CRT_FB_ADDRESS, EXT, LOCAL) |
                FIELD_VALUE (0, CRT_FB_ADDRESS, ADDRESS, value));

        regWrite32 (pSmi, CRT_FB_WIDTH,
                FIELD_VALUE (0, CRT_FB_WIDTH, WIDTH,
                    register_table->fb_width) |
                FIELD_VALUE (0, CRT_FB_WIDTH, OFFSET,
                    register_table->fb_width));


        regWrite32 (pSmi, MISC_CTRL,FIELD_SET (regRead32 (pSmi, MISC_CTRL), MISC_CTRL,CLK_SELECT, OSC));
        regWrite32 (pSmi, CRT_DISPLAY_CTRL, FIELD_SET (regRead32 (pSmi, CRT_DISPLAY_CTRL), CRT_DISPLAY_CTRL,
                    SELECT, CRT));

#if 0
        regWrite32 (pSmi,  LOCALMEM_ARBITRATION,
                0x01765324  );

        regWrite32 (pSmi,  PCIMEM_ARBITRATION,
                0x01765324   );
#endif
        regWrite32 (pSmi,0x88,0x6);



        regWrite32 (pSmi, CRT_HORIZONTAL_TOTAL,
                register_table->horizontal_total);
        regWrite32 (pSmi, CRT_HORIZONTAL_SYNC, register_table->horizontal_sync);
        regWrite32 (pSmi, CRT_VERTICAL_TOTAL, register_table->vertical_total);
        regWrite32 (pSmi, CRT_VERTICAL_SYNC, register_table->vertical_sync);

        value = regRead32 (pSmi, CRT_DISPLAY_CTRL)
            & FIELD_CLEAR (CRT_DISPLAY_CTRL, VSYNC_PHASE)
            & FIELD_CLEAR (CRT_DISPLAY_CTRL, HSYNC_PHASE)
            & FIELD_CLEAR (CRT_DISPLAY_CTRL, SELECT)
            & FIELD_CLEAR (CRT_DISPLAY_CTRL, TIMING)
            & FIELD_CLEAR (CRT_DISPLAY_CTRL, PLANE)
            & FIELD_CLEAR (CRT_DISPLAY_CTRL, FORMAT);	

        regWrite32 (pSmi, CRT_DISPLAY_CTRL, value | register_table->control);

        palette_ram = CRT_PALETTE_RAM;

        /* Turn on CRT. */
        setDPMS_750 (pSmi, DPMS_ON);




        /* Program clock, enable display controller. */
        /* FIELD_CLEAR(CURRENT_POWER_CLOCK, V1XCLK) 
         * Added by Belcon
         */
#if  0
        gate = FIELD_SET (gate, CURRENT_POWER_GATE, DISPLAY, ENABLE);
        clock &= FIELD_CLEAR (CURRENT_POWER_CLOCK, V2XCLK_SELECT)
            & FIELD_CLEAR (CURRENT_POWER_CLOCK, V2XCLK_DIVIDER)
            & FIELD_CLEAR (CURRENT_POWER_CLOCK, V1XCLK)
            & FIELD_CLEAR (CURRENT_POWER_CLOCK, V2XCLK_SHIFT);
        setPower (pSmi, gate, clock | register_table->clock, 0);

        /* Turn on DAC. */
        regWrite32 (pSmi, MISC_CTRL,
                FIELD_SET (regRead32 (pSmi, MISC_CTRL), MISC_CTRL,
                    DAC_POWER, ENABLE));

        /* Calculate frame buffer address. */
        value = 0;
#if 0
        fb_size = register_table->fb_width * register_table->height;
        xf86DrvMsg("", X_INFO, "CRT: fb_size is 0x%x\n", fb_size);
        // xf86DrvMsg("", X_INFO, "Belcon: programMode()[CRT], %d x %d\n", register_table->fb_width, register_table->height);              
        if (FIELD_GET (regRead32 (pSmi, PANEL_DISPLAY_CTRL),
                    PANEL_DISPLAY_CTRL,
                    PLANE) == PANEL_DISPLAY_CTRL_PLANE_ENABLE)
        {
            value = FIELD_GET (regRead32 (pSmi, PANEL_FB_ADDRESS),
                    PANEL_FB_ADDRESS, ADDRESS);
            xf86DrvMsg("", X_INFO, "CRT: value is 0x%x\n", value);
            if (fb_size < value)
            {
                value = 0;
            }
            else
            {
                value += FIELD_GET (regRead32 (pSmi, PANEL_FB_WIDTH),
                        PANEL_FB_WIDTH,
                        OFFSET)
                    * FIELD_GET (regRead32 (pSmi, PANEL_WINDOW_HEIGHT),
                            PANEL_WINDOW_HEIGHT, HEIGHT);
            }
        }
#endif
        value = pSmi->FBOffset;
        //	value += 4096;
        xf86DrvMsg("", X_INFO, "CRT: value is 0x%x\n", value);

        /* Program CRT registers. */
        regWrite32 (pSmi, CRT_FB_ADDRESS,
                FIELD_SET (0, CRT_FB_ADDRESS, STATUS, PENDING) |
                FIELD_SET (0, CRT_FB_ADDRESS, EXT, LOCAL) |
                FIELD_VALUE (0, CRT_FB_ADDRESS, ADDRESS, value));

        xf86DrvMsg("", X_INFO, "CRT: address is 0x%x\n", FIELD_GET(regRead32(pSmi, CRT_FB_ADDRESS), CRT_FB_ADDRESS, ADDRESS));
        regWrite32 (pSmi, CRT_FB_WIDTH,
                FIELD_VALUE (0, CRT_FB_WIDTH, WIDTH,
                    register_table->fb_width) |
                FIELD_VALUE (0, CRT_FB_WIDTH, OFFSET,
                    register_table->fb_width));

        regWrite32 (pSmi, CRT_HORIZONTAL_TOTAL,
                register_table->horizontal_total);
        regWrite32 (pSmi, CRT_HORIZONTAL_SYNC, register_table->horizontal_sync);
        regWrite32 (pSmi, CRT_VERTICAL_TOTAL, register_table->vertical_total);
        regWrite32 (pSmi, CRT_VERTICAL_SYNC, register_table->vertical_sync);

        /* Program CRT display control register. */
        value = regRead32 (pSmi, CRT_DISPLAY_CTRL)
            & FIELD_CLEAR (CRT_DISPLAY_CTRL, VSYNC_PHASE)
            & FIELD_CLEAR (CRT_DISPLAY_CTRL, HSYNC_PHASE)
            & FIELD_CLEAR (CRT_DISPLAY_CTRL, SELECT)
            & FIELD_CLEAR (CRT_DISPLAY_CTRL, TIMING)
            & FIELD_CLEAR (CRT_DISPLAY_CTRL, PLANE)
            & FIELD_CLEAR (CRT_DISPLAY_CTRL, FORMAT);

        regWrite32 (pSmi, CRT_DISPLAY_CTRL, value | register_table->control);

        /* Palette RAM. */
        palette_ram = CRT_PALETTE_RAM;

        /* Turn on CRT. */
        setDPMS (pSmi, DPMS_ON);
#endif 
    }
    /* In case of 8-bpp, fill palette. */
    if (FIELD_GET (register_table->control,
                PANEL_DISPLAY_CTRL, FORMAT) == PANEL_DISPLAY_CTRL_FORMAT_8)
    {
        /* Start with RGB = 0,0,0. */
        BYTE red = 0, green = 0, blue = 0;
        DWORD gray = 0;
        for (offset = 0; offset < 256 * 4; offset += 4)
        {
            /* Store current RGB value. */
            /* ERROR!!!!! IGX RGB should be a function, maybe RGB16?
               regWrite32(pSmi,  (palette_ram + offset), 
               (gray ? (RGB((gray + 50) / 100,
               (gray + 50) / 100,
               (gray + 50) / 100))
               : (RGB(red, green, blue))));
             */

            if (gray)
            {
                /* Walk through grays (40 in total). */
                gray += 654;
            }

            else
            {
                /* Walk through colors (6 per base color). */
                if (blue != 255)
                {
                    blue += 51;
                }
                else if (green != 255)
                {
                    blue = 0;
                    green += 51;
                }
                else if (red != 255)
                {
                    green = blue = 0;
                    red += 51;
                }
                else
                {
                    gray = 1;
                }
            }
        }
    }

    /* For 16- and 32-bpp,  fill palette with gamma values. */
    else
    {

        /* Start with RGB = 0,0,0. */
        value = 0x000000;
        for (offset = 0; offset < 256 * 4; offset += 4)
        {
            regWrite32 (pSmi, palette_ram + offset, value);

            /* Advance RGB by 1,1,1. */
            value += 0x010101;
        }
    }
}
void setSecTiming(SMIPtr pSmi,display_t channel,int x,int y,int hz)
{
    pmode_table_t vesaMode;
    pll_value_t pll;
    unsigned long value;

    vesaMode = (pmode_table_t)findMode_750(mode_table_750,x, y, hz);
    if (vesaMode == NULL)
        return;

    /*	set pll for second channel	*/	
    calcPllValue(vesaMode->pixel_clock, &pll);	
    value = FIELD_SET(  0, CRT_PLL_CTRL, BYPASS, OFF)
        | FIELD_SET(  0, CRT_PLL_CTRL, POWER,  ON)
        | FIELD_SET(  0, CRT_PLL_CTRL, INPUT,  OSC)
        | FIELD_VALUE(0, CRT_PLL_CTRL, OD,pll.OD)
        | FIELD_VALUE(0, CRT_PLL_CTRL, N,pll.N)
        | FIELD_VALUE(0, CRT_PLL_CTRL, M,pll.M);

    regWrite32 (pSmi, CRT_PLL_CTRL, value);

    /*	set timing parameters	*/
    regWrite32(pSmi,CRT_HORIZONTAL_TOTAL,
            FIELD_VALUE(0, CRT_HORIZONTAL_TOTAL, TOTAL, vesaMode->horizontal_total - 1)
            | FIELD_VALUE(0, CRT_HORIZONTAL_TOTAL, DISPLAY_END, vesaMode->horizontal_display_end - 1));

    regWrite32(pSmi,CRT_HORIZONTAL_SYNC,
            FIELD_VALUE(0, CRT_HORIZONTAL_SYNC, WIDTH, vesaMode->horizontal_sync_width)
            | FIELD_VALUE(0, CRT_HORIZONTAL_SYNC, START, vesaMode->horizontal_sync_start - 1));

    regWrite32(pSmi,CRT_VERTICAL_TOTAL,
            FIELD_VALUE(0, CRT_VERTICAL_TOTAL, TOTAL, vesaMode->vertical_total - 1)
            | FIELD_VALUE(0, CRT_VERTICAL_TOTAL, DISPLAY_END, vesaMode->vertical_display_end - 1));

    regWrite32(pSmi,CRT_VERTICAL_SYNC,
            FIELD_VALUE(0, CRT_VERTICAL_SYNC, HEIGHT, vesaMode->vertical_sync_height)
            | FIELD_VALUE(0, CRT_VERTICAL_SYNC, START, vesaMode->vertical_sync_start - 1));

    value = regRead32(pSmi,CRT_DISPLAY_CTRL);
    value = (vesaMode->vertical_sync_polarity == POSITIVE ?
            FIELD_SET(value, CRT_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH)
            : FIELD_SET(value, CRT_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW))
        | (vesaMode->horizontal_sync_polarity == POSITIVE ? 
                FIELD_SET(value, CRT_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH)
                : FIELD_SET(value, CRT_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW));
    regWrite32(pSmi,CRT_DISPLAY_CTRL,value);
}

void setScalar(SMIPtr pSmi, int x,int x2,int y ,int y2)
{
#define SCALE_CONSTANT	(1<<12)

    unsigned long vertScaleFactor, horzScaleFactor;
    unsigned long value;

    vertScaleFactor = y * SCALE_CONSTANT / y2;        
    horzScaleFactor = x * SCALE_CONSTANT / x2;

    value = 0;

    value = FIELD_VALUE(value, CRT_SCALE , VERTICAL_SCALE, vertScaleFactor);
    value = FIELD_VALUE(value, CRT_SCALE , HORIZONTAL_SCALE, horzScaleFactor);
    regWrite32(pSmi,CRT_SCALE , value);


    /*	set interpolation	*/
    value = regRead32(pSmi,CRT_DISPLAY_CTRL);
    value = FIELD_SET(value,CRT_DISPLAY_CTRL,EXPANSION,DISABLE);
    value = FIELD_SET(value,CRT_DISPLAY_CTRL,LOCK_TIMING,DISABLE);

    if (1)
        value = FIELD_SET(value, CRT_DISPLAY_CTRL, HORIZONTAL_MODE, INTERPOLATE);
    else


        if (1)
            value = FIELD_SET(value, CRT_DISPLAY_CTRL, VERTICAL_MODE, INTERPOLATE);
        else


            regWrite32(pSmi,CRT_DISPLAY_CTRL, value);

}

void setNoScalar(SMIPtr pSmi)
{
    unsigned long value;
    value = regRead32(pSmi,CRT_DISPLAY_CTRL);
    value = FIELD_SET(value, CRT_DISPLAY_CTRL, HORIZONTAL_MODE, REPLICATE);
    value = FIELD_SET(value, CRT_DISPLAY_CTRL, VERTICAL_MODE, REPLICATE);
    value = FIELD_SET(value,CRT_DISPLAY_CTRL,EXPANSION,DISABLE);
    value = FIELD_SET(value,CRT_DISPLAY_CTRL,LOCK_TIMING,DISABLE);
    regWrite32(pSmi,CRT_DISPLAY_CTRL,value);

    regWrite32(pSmi,CRT_SCALE,0);
}


VOID SetMode_750 (SMIPtr pSmi, ULONG nWidth, ULONG nHeight, FLONG fMode, ULONG nHertz,
        display_t display, INT fbPitch, INT bpp)
{
    mode_table_t mode;
    pmode_table_t vesaMode;
    reg_table_t register_table;
    unsigned long value, gate, value_reg10;
    unsigned int mode_index = 0;
    unsigned int pll_index = 0;
	#define  PLL_INDEX_MAX 	12

    regWrite32(pSmi, 0x70, 0x20cf0);
	    xf86DrvMsg("", X_INFO, "David:SetMode(), nWidth %ld is , nHeight is %ld\n", nWidth, nHeight );	
    pll_table = malloc( (PLL_INDEX_MAX+1)*sizeof(struct pll_value_t) );	
	if(!pSmi->edid_enable)
{
    /* Locate the mode */
    // xf86DrvMsg("", X_INFO, "Belcon, SetMode(%s), %dx%dx%d, nHertz is %d, fbPitch is %d\n", display ? "CRT" : "panel", nWidth, nHeight, bpp, nHertz, fbPitch);
    vesaMode = findMode_750 (mode_table_750, nWidth, nHeight, nHertz);
    /*
       xf86DrvMsg("", X_INFO, "Belcon: vesaMode is %p\n", vesaMode);
       xf86DrvMsg("", X_INFO, "Belcon, SetMode(), find mode %d x %d @ %d, pixelclock is %d, hfreq is %d\n", vesaMode->horizontal_display_end, vesaMode->vertical_display_end, vesaMode->vertical_frequency, vesaMode->pixel_clock, vesaMode->horizontal_frequency);
     */


    if( nWidth == 640 && nHeight == 480){
        pll_index = 0;

        if( nHertz == 60 ) {
            calcPllValue(25175000,&pll_table[pll_index]);
        } else if (  nHertz == 75) {
            calcPllValue(31500000,&pll_table[pll_index]);
        } else if ( nHertz == 85){
            calcPllValue(36000000,&pll_table[pll_index]);
        }

    } else if(nWidth == 800 && nHeight == 600){
        pll_index = 1;
        if( nHertz == 60 ) {
            calcPllValue(40000000,&pll_table[pll_index]);
        } else if (  nHertz == 75) {
            calcPllValue(49500000,&pll_table[pll_index]);
        } else if ( nHertz == 85){
            calcPllValue(56250000,&pll_table[pll_index]);
        }

    } else if(nWidth == 1024 && nHeight == 768){
        pll_index = 2;
        if( nHertz == 60 ) {
            calcPllValue(65000000,&pll_table[pll_index]);
        } else if (  nHertz == 75) {
            calcPllValue(78750000,&pll_table[pll_index]);
        } else if ( nHertz == 85){
            calcPllValue(94500000,&pll_table[pll_index]);
        }

    } else if(nWidth == 1280 && nHeight == 1024){
        pll_index = 3;
        if( nHertz == 60 ) {
            xf86DrvMsg("", X_INFO, "T-Bag:Test,(60HZ) FUNC: %s\n",__FUNCTION__);
            calcPllValue(108000000,&pll_table[pll_index]);
        } else if (  nHertz == 75) {
            xf86DrvMsg("", X_INFO, "T-Bag:Test,(75HZ) FUNC: %s\n",__FUNCTION__);
            calcPllValue(/*135000000*/138500000,&pll_table[pll_index]);
        } else if ( nHertz == 85){
            xf86DrvMsg("", X_INFO, "T-Bag:Test,(85HZ)  FUNC: %s\n",__FUNCTION__);
            calcPllValue(/*157500000*/157500000,&pll_table[pll_index]);
        }

    } else if(nWidth == 1600 && nHeight == 1200 ) {
        pll_index = 4;
        if( nHertz == 60 ) {
            calcPllValue(162000000,&pll_table[pll_index]);
        } else if (  nHertz == 75) {
            calcPllValue(206000000,&pll_table[pll_index]);
        }

    }  else if(nWidth == 1024 && nHeight == 600) {
        pll_index = 5;
        if( nHertz == 60 ) {
            calcPllValue(48960000,&pll_table[pll_index]);
        } else if (  nHertz == 75) {
            calcPllValue(63200000,&pll_table[pll_index]);
        } else /*if ( nHertz == 85)*/{
            calcPllValue(72830000,&pll_table[pll_index]);
        }

    } else if(nWidth == 1280 && nHeight == 720) {
        pll_index = 6;
        if( nHertz == 60 ) {
            calcPllValue(74780000,&pll_table[pll_index]);
        } else if (  nHertz == 75) {
            calcPllValue(95650000,&pll_table[pll_index]);
        } else if ( nHertz == 85){
            calcPllValue(110010000,&pll_table[pll_index]);
        }

    } else if(nWidth == 1360 && nHeight == 768) {
        pll_index = 7;
        if( nHertz == 60 ) {
            calcPllValue(84720000,&pll_table[pll_index]);
        } else if (  nHertz == 75) {
            calcPllValue(108750000,&pll_table[pll_index]);
        } else if ( nHertz == 85){
            calcPllValue(125120000,&pll_table[pll_index]);
        }

    } else if(nWidth == 1440 && nHeight == 960) {
        pll_index = 8;
        if( nHertz == 60 ) {
            calcPllValue(114510000,&pll_table[pll_index]);
        } else if (  nHertz == 75) {
            calcPllValue(146690000,&pll_table[pll_index]);
        } else if ( nHertz == 85){
            calcPllValue(167250000,&pll_table[pll_index]);
        }

    }  else if(nWidth == 1920 && nHeight == 1080) {
        pll_index = 9;
        if( nHertz == 60 ) {
            calcPllValue(172800000,&pll_table[pll_index]);
        } else if (  nHertz == 75) {
            calcPllValue(220640000,&pll_table[pll_index]);
        }
    } else if(nWidth == 1920 && nHeight == 1200) {
        pll_index = 10;
        if(nHertz == 60) {
            calcPllValue(193250000, &pll_table[pll_index]);
        } else if (nHertz == 75) {
            calcPllValue(245250000, &pll_table[pll_index]);
        } else if (nHertz == 85) {
            calcPllValue(281250000, &pll_table[pll_index]);
        }
    }  else if(nWidth == 1920 && nHeight == 1440) {
	pll_index = PLL_INDEX_MAX -1;
	if( nHertz == 60 ) {
	    calcPllValue(234000000,&pll_table[pll_index]);
	}
        if( nHertz == 75 ) {
            calcPllValue(297000000,&pll_table[pll_index]);
        }
    }
}
	else
{
	pll_index = PLL_INDEX_MAX;
	vesaMode = &mode_table_edid;
	SMI_Getpll(&pll_table[pll_index], vesaMode);
}
    //hw_rev = FIELD_GET (regRead32 (pSmi, DEVICE_ID), DEVICE_ID, REVISION_ID);
    if (vesaMode != NULL)
    {
        adjustMode_750 (vesaMode, &mode, display);
        /* Fill the register structure */
        setModeRegisters_750 (pSmi, &register_table, &mode, display, bpp, fbPitch);

        if(display == CRT)
        {
            //xf86DrvMsg("", X_INFO, "T-bag: here we are:  %d  vesaMode->M: %d  vesaMode->N: %d \n", __LINE__,vesaMode->M,vesaMode->N);
            value = 0;
            value = FIELD_SET(  0, CRT_PLL_CTRL, BYPASS, OFF)
                | FIELD_SET(  0, CRT_PLL_CTRL, POWER,  ON)
                | FIELD_SET(  0, CRT_PLL_CTRL, INPUT,  OSC)
                | FIELD_VALUE(0, CRT_PLL_CTRL, OD,pll_table[pll_index].OD)
                | FIELD_VALUE(0, CRT_PLL_CTRL, N,pll_table[pll_index].N)
                | FIELD_VALUE(0, CRT_PLL_CTRL, M,pll_table[pll_index].M);        
            regWrite32 (pSmi, CRT_PLL_CTRL, value);
        }
        else
        {
            value = 0;
            value = FIELD_SET(  0, PANEL_PLL_CTRL, BYPASS, OFF)
                | FIELD_SET(  0, PANEL_PLL_CTRL, POWER,  ON)
                | FIELD_SET(  0, PANEL_PLL_CTRL, INPUT,  OSC)
                | FIELD_VALUE(0, PANEL_PLL_CTRL, OD,pll_table[pll_index].OD)
                | FIELD_VALUE(0, PANEL_PLL_CTRL, N,pll_table[pll_index].N)
                | FIELD_VALUE(0, PANEL_PLL_CTRL, M,pll_table[pll_index].M);
            regWrite32 (pSmi, PANEL_PLL_CTRL, value);		
        }

        /* Program the registers */
        programMode_750 (pSmi, &register_table); 
    }
}

    VOID
panelSetMode_750 (SMIPtr pSmi, ULONG nWidth, ULONG nHeight, FLONG fMode,
        ULONG nHertz, INT fbPitch, INT bpp)
{
    //  xf86DrvMsg("", X_INFO, "Belcon: panelSetMode, nHertz is %ld\n", nHertz);

    SetMode_750 (pSmi, nWidth, nHeight, fMode, nHertz /* was nHertz */ , PANEL,
            fbPitch, bpp);
}

    VOID
crtSetMode_750 (SMIPtr pSmi, ULONG nWidth, ULONG nHeight, FLONG fMode,
        ULONG nHertz, INT fbPitch, INT bpp)
{
    //  xf86DrvMsg("", X_INFO, "Belcon: crtSetMode, %d x %d, mode is %d, nHertz is %d, fbPitch is %d, bpp is %d\n", nWidth, nHeight, fMode, nHertz, fbPitch, bpp);
    SetMode_750 (pSmi, nWidth, nHeight, fMode, nHertz, CRT, fbPitch, bpp);
}

/*
 *
 *
 *  From POWER.C  
 *
 *
 */

/* Program new power mode. */
    VOID
setPower_750 (SMIPtr pSmi, ULONG nGates, ULONG Clock, int control_value)
{
    ULONG gate_reg = 0, clock_reg = 0;
    ULONG control_value_orig;

    /* Get current power mode. */



    //added by t-bag
    control_value_orig = regRead32 (pSmi, POWER_MODE_CTRL);
    /*
       control_value_orig = FIELD_GET (regRead32 (pSmi, POWER_MODE_CTRL),
       POWER_MODE_CTRL, MODE);
     */

    switch (control_value)
    {
        case POWER_MODE_CTRL_MODE_MODE0:

            /* Switch from mode 0 or sleep to mode 1. */
            gate_reg = MODE0_GATE;
            //clock_reg = POWER_MODE0_CLOCK;
            control_value_orig = FIELD_SET (control_value_orig, POWER_MODE_CTRL, MODE, MODE0);
            break;

        case POWER_MODE_CTRL_MODE_MODE1:

            /* Switch from mode 1 or sleep to mode 0. */
            gate_reg = MODE1_GATE;
            //clock_reg = POWER_MODE1_CLOCK;
            control_value_orig = FIELD_SET (control_value_orig, POWER_MODE_CTRL, MODE, MODE1);
            break;

        case POWER_MODE_CTRL_MODE_SLEEP:

            gate_reg = MODE0_GATE;
            /* Switch from mode 1/0 to sleep. */
            control_value_orig = FIELD_SET (control_value_orig, POWER_MODE_CTRL, MODE, SLEEP);
            break;

        default:
            /* Invalid mode */
            return;
    }


    /* Set up other fields in Power Control Register */
    if (control_value == POWER_MODE_CTRL_MODE_SLEEP)
    {
        control_value_orig =
            FIELD_SET(  control_value_orig, POWER_MODE_CTRL, 336CLK, OFF)
            | FIELD_SET(  control_value_orig, POWER_MODE_CTRL, OSC_INPUT,  OFF);
    }
    else
    {
        control_value_orig =
            FIELD_SET(  control_value_orig, POWER_MODE_CTRL, 336CLK, ON)
            | FIELD_SET(  control_value_orig, POWER_MODE_CTRL, OSC_INPUT,  ON);
    }

    /* Program new power mode. */

    if(nGates > 0)
        regWrite32 (pSmi, gate_reg, nGates);
    //regWrite32 (pSmi, clock_reg, Clock);

    regWrite32 (pSmi, POWER_MODE_CTRL, control_value_orig);

    /* When returning from sleep, wait until finished. */
    /*          IGX -- comment out for now, gets us in an infinite loop! */
    /*	while (FIELD_GET(regRead32(pSmi, POWER_MODE_CTRL),
        POWER_MODE_CTRL,
        SLEEP_STATUS) == POWER_MODE_CTRL_SLEEP_STATUS_ACTIVE) ;
     */

}

/* Set DPMS state. */
    VOID
setDPMS_750 (SMIPtr pSmi, DPMS_t state)
{
    ULONG value;

    value = regRead32 (pSmi, SYSTEM_CTRL);
    switch (state)
    {
        case DPMS_ON:
            value = FIELD_SET (value, SYSTEM_CTRL, DPMS, VPHP);
            break;

        case DPMS_STANDBY:
            value = FIELD_SET (value, SYSTEM_CTRL, DPMS, VPHN);
            break;

        case DPMS_SUSPEND:
            value = FIELD_SET (value, SYSTEM_CTRL, DPMS, VNHP);
            break;

        case DPMS_OFF:
            value = FIELD_SET (value, SYSTEM_CTRL, DPMS, VNHN);
            break;
    }

    regWrite32 (pSmi, SYSTEM_CTRL, value);
}



/* Panel Code */



/**********************************************************************
 *
 * panelWaitVSync
 *
 * Purpose
 *    Wait for the specified number of panel Vsyncs
 *
 * Parameters
 *    [in]
 *        vsync_count - Number of Vsyncs to wait
 *
 *    [out]
 *        None
 *
 * Returns
 *    Nothing
 *
 **********************************************************************/
    static VOID
panelWaitVSync_750 (SMIPtr pSmi, INT vsync_count)
{
    ULONG status;
    ULONG timeout;

    while (vsync_count-- > 0)
    {
        /* Wait for end of vsync */
        timeout = 0;
        do
        {
            status = FIELD_GET (regRead32 (pSmi, SYSTEM_CTRL),
                    SYSTEM_CTRL, PANEL_VSYNC);
            /*
               status = FIELD_GET (regRead32 (pSmi, CMD_INTPR_STATUS),
               CMD_INTPR_STATUS, PANEL_SYNC);
             */
            if (++timeout == VSYNCTIMEOUT)
                break;
        }
        while (status == SYSTEM_CTRL_PANEL_VSYNC_ACTIVE);

        /* Wait for start of vsync */
        timeout = 0;
        do
        {
            status = FIELD_GET (regRead32 (pSmi, SYSTEM_CTRL),
                    SYSTEM_CTRL, PANEL_VSYNC);
            if (++timeout == VSYNCTIMEOUT)
                break;
        }
        while (status == SYSTEM_CTRL_PANEL_VSYNC_INACTIVE);
    }
}





/**********************************************************************
 *
 * panelPowerSequence
 *
 * Purpose
 *    Turn the panel On/Off
 *
 * Parameters
 *    [in]
 *        on_off      - Turn panel On/Off. Can be:
 *                      PANEL_ON
 *                      PANEL_OFF
 *        vsync_delay - Number of Vsyncs to wait after each signal is
 *                      turned on/off
 *
 *    [out]
 *        None
 *
 * Returns
 *    Nothing
 *
 **********************************************************************/
    VOID
panelPowerSequence_750 (SMIPtr pSmi, panel_state_t on_off, INT vsync_delay)
{
    ULONG panelControl = regRead32 (pSmi, PANEL_DISPLAY_CTRL);

    if (on_off == PANEL_ON)
    {
        /* Turn on FPVDDEN. */
        panelControl = FIELD_SET (panelControl,
                PANEL_DISPLAY_CTRL, FPVDDEN, HIGH);
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVSync_750 (pSmi, vsync_delay);

        /* Turn on FPDATA. */
        panelControl = FIELD_SET (panelControl,
                PANEL_DISPLAY_CTRL, DATA, ENABLE);
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVSync_750 (pSmi, vsync_delay);

        /*  Turn on FPVBIAS. */
        panelControl = FIELD_SET (panelControl,
                PANEL_DISPLAY_CTRL, VBIASEN, HIGH);
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVSync_750 (pSmi, vsync_delay);

        /* Turn on FPEN. */
        panelControl = FIELD_SET (panelControl, PANEL_DISPLAY_CTRL, FPEN, HIGH);
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panelControl);
    }

    else
    {
        /* Turn off FPEN. */
        panelControl = FIELD_SET (panelControl, PANEL_DISPLAY_CTRL, FPEN, LOW);
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVSync_750 (pSmi, vsync_delay);

        /*  Turn off FPVBIASEN. */
        panelControl = FIELD_SET (panelControl,
                PANEL_DISPLAY_CTRL, VBIASEN, LOW);
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVSync_750 (pSmi, vsync_delay);

        /* Turn off FPDATA. */
        panelControl = FIELD_SET (panelControl,
                PANEL_DISPLAY_CTRL, DATA, DISABLE);
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVSync_750 (pSmi, vsync_delay);

        /* Turn off FPVDDEN. */
        panelControl = FIELD_SET (panelControl,
                PANEL_DISPLAY_CTRL, FPVDDEN, LOW);
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panelControl);
    }
}


/**********************************************************************
 *
 * open_secondary_channel
 *
 * Purpose
 *    Enable/disable routing of panel output to CRT monitor
 *
 * Parameters
 *    [in]
 *        bEnable - TRUE enables routing of panel output to CRT monitor
 *                  FALSE disables routing of panel output to CRT monitor
 *
 *    [out]
 *        None
 *
 * Returns
 *    Nothing
 *
 **********************************************************************/
    VOID
open_secondary_channel (SMIPtr pSmi, BOOL bEnable)
{
    ULONG crt_ctrl = 0;

    crt_ctrl = regRead32 (pSmi, CRT_DISPLAY_CTRL);

    if (bEnable)
        /* Enable CRT graphics plane */
        crt_ctrl = FIELD_SET (crt_ctrl, CRT_DISPLAY_CTRL, PLANE, ENABLE);
    else
        /* Disable CRT graphics plane */
        crt_ctrl = FIELD_SET (crt_ctrl, CRT_DISPLAY_CTRL, PLANE, DISABLE);

    //  crt_ctrl = FIELD_SET (crt_ctrl, CRT_DISPLAY_CTRL, SELECT, CRT); 
    //  crt_ctrl = FIELD_SET (crt_ctrl, CRT_DISPLAY_CTRL, BLANK, OFF);
    regWrite32 (pSmi, CRT_DISPLAY_CTRL, crt_ctrl);
}

    VOID
open_primary_channel (SMIPtr pSmi, BOOL bEnable)
{
    ULONG panel_ctrl = 0;

    panel_ctrl = regRead32 (pSmi, PANEL_DISPLAY_CTRL);
    if (bEnable)
        /* Enable panel graphics plane */
        panel_ctrl = FIELD_SET (panel_ctrl, PANEL_DISPLAY_CTRL, PLANE, ENABLE);
    else
        /* Disable panel graphics plane */
        panel_ctrl = FIELD_SET (panel_ctrl, PANEL_DISPLAY_CTRL, PLANE, DISABLE);
    //panel_ctrl = FIELD_SET (panel_ctrl, PANEL_DISPLAY_CTRL, SELECT, PANEL);

    regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panel_ctrl);

}
#if 0
Bool entity_init_750(ScrnInfoPtr pScrn,int entityIndex)
{
	DevUnion *pPriv;
	SMIRegPtr pRegPtr;

	xf86SetEntitySharable(entityIndex);
	/*entityIndex should better not lager than 16*/
	if(entityIndex > MAX_ENT_IDX){
		XERR("entityIndex:%d is too large\n",entityIndex);
		return FALSE;
	}
	
	if(entity_priv_index[entityIndex] == -1)
	{
		entity_priv_index[entityIndex] = xf86AllocateEntityPrivateIndex();
		XINF("entity_prv_index[%d] = %d\n",entityIndex,entity_priv_index[entityIndex]);	
	}

	pPriv =  xf86GetEntityPrivate(entityIndex,entity_priv_index[entityIndex]);

	if(!pPriv->ptr){
		/* first time this entity goto this function */
		pPriv->ptr = xnfcalloc(sizeof(SMIRegRec),1);		
		pRegPtr = pPriv->ptr;
		pRegPtr->lastInstance = -1;
		pRegPtr->DualHead = FALSE;
	}else{
		pRegPtr = pPriv->ptr;
		pRegPtr->DualHead = TRUE;
	}
	
	pRegPtr->lastInstance++;
	xf86SetEntityInstanceForScreen(pScrn, entityIndex,pRegPtr->lastInstance);
	return TRUE;
}
#endif

Bool smi_setdepbpp_750(ScrnInfoPtr pScrn)
{
    if (!xf86SetDepthBpp(pScrn, 8, 8, 8,
                Support32bppFb | SupportConvert24to32 | PreferConvert24to32)) {
        return (FALSE);
    }
    /* Check that the returned depth is one we support */
    switch (pScrn->depth) {
        case 8:
        case 16:
        case 24:
            break;

        case 32:
            pScrn->depth = 24;
            break;

        default:
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "Given depth (%d) is not supported "
                    "by this driver\n", pScrn->depth);
            return (FALSE);
            break;
    }
    return (TRUE);
}

Bool smi_set_dualhead_750(ScrnInfoPtr pScrn, SMIPtr pSmi)
{

    if (xf86IsEntityShared (pScrn->entityList[0])) 
    {
        SMIRegPtr	pSMIEnt = SMIEntPriv (pScrn);
        if (!xf86IsPrimInitDone(pScrn->entityList[0])) {
            xf86SetPrimInitDone(pScrn->entityList[0]);
            pSmi->IsPrimary = TRUE;
            pSmi->IsSecondary = FALSE;
            pSMIEnt->pPrimaryScrn = pScrn;
            pSmi->IsLCD = TRUE;
            pSmi->IsCRT = FALSE;
        } else if (pSMIEnt->DualHead) {
		    pSMIEnt->pSecondaryScrn = pScrn;
            pSmi->IsSecondary = TRUE;       
			pSmi->IsPrimary = FALSE;
            pSmi->IsCRT = TRUE;
        } else {
            return (FALSE);
        }
    }
    return (TRUE);
}

Bool smi_setvideomem_750(int config, ScrnInfoPtr pScrn, SMIPtr pSmi)
{
    unsigned int	total_memory = 16 * 1024;
    int tmp;
    CARD32		memBase;
    DWORD		value = 0;

    pSmi->MapSize = 0x200000;
#ifndef	XSERVER_LIBPCIACCESS	
    memBase = pSmi->PciInfo->memBase[1];
    pSmi->MapBase = xf86MapPciMem (pScrn->scrnIndex, VIDMEM_MMIO | VIDMEM_MMIO_32BIT, 
            pSmi->PciTag, memBase, pSmi->MapSize);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: pScrn->display->virtualX is %d\n", 
            pScrn->display->virtualX);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: map io memory, virtualX is %d, virtualY is %d\n", 
            pScrn->virtualX, pScrn->virtualY);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: map io memory, lcdWidth is %d, lcdHeight is %d\n", 
            pSmi->lcdWidth, pSmi->lcdHeight);
    if (pSmi->MapBase == NULL) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                "Internal error: could not map "
                "MMIO registers.\n");
        return (FALSE);
    }
#else
    memBase = PCI_REGION_BASE(pSmi->PciInfo, 1, REGION_MEM); //caesar modified
    {
        void	**result = (void**)&pSmi->MapBase;
        int  err = pci_device_map_range(pSmi->PciInfo,
                memBase,
                pSmi->MapSize,
                PCI_DEV_MAP_FLAG_WRITABLE,
                (void**)&pSmi->MapBase);	
        if (err)
            return (FALSE); 
    }
#endif
    pSmi->DPRBase = pSmi->MapBase + 0x100000;
    pSmi->VPRBase = pSmi->MapBase + 0x000000;
    pSmi->CPRBase = pSmi->MapBase + 0x090000;
    pSmi->DCRBase = pSmi->MapBase + 0x080000;
    pSmi->SCRBase = pSmi->MapBase + 0x000000;
    pSmi->IOBase = 0;
    pSmi->DataPortBase = pSmi->MapBase + 0x110000;
    pSmi->DataPortSize = 0x10000;
    tmp = READ_SCR(pSmi, 0x04);
    tmp = ( tmp >> 12) & 0x3;
    switch (tmp) {
        case 0:
            total_memory = 16 * 1024;
            break;
        case 1:
            total_memory = 32 * 1024;
            break;
        case 2:
            total_memory = 64 * 1024;
            break;
        case 3:
            total_memory = 8 * 1024;
            break;
        default:
            xf86DrvMsg("", X_INFO, "Error Detecting Total Memory\n");
            break;
    }


    /*
       if ((SMI_MSOCE == pSmi->Chipset) && !(pScrn->virtualX)) {
       return TRUE;
       }
     */

#ifndef	XSERVER_LIBPCIACCESS	
    pScrn->memPhysBase = pSmi->PciInfo->memBase[0];
#else
    pScrn->memPhysBase = PCI_REGION_BASE(pSmi->PciInfo, 0, REGION_MEM); //caesar modified
#endif
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "memPhysBase  is 0x%x\n",  pScrn->memPhysBase);
#if 0	
    value = regRead32(pSmi, DRAM_CTRL);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: DRAM_CTRL 0x%x\n", value);
    if ((FIELD_GET(value, DRAM_CTRL, SIZE) == DRAM_CTRL_SIZE_4)) {
        /*
         * SM107 : Need to re-assign number of local memory banks
         */
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: SM107 found\n");
        regWrite32 (pSmi, DRAM_CTRL, FIELD_SET(value, DRAM_CTRL, BANKS, 2));
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: write 0x%x to DRAM_CTRL\n", FIELD_SET(value, DRAM_CTRL, BANKS, 2));
        total_memory = 4 * 1024;
    }
#endif
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: Unmap io memory\n");
#ifndef	XSERVER_LIBPCIACCESS	
    xf86UnMapVidMem (pScrn->scrnIndex, (pointer) pSmi->MapBase, pSmi->MapSize);
#else
    pci_device_unmap_range(pSmi->PciInfo,(pointer)pSmi->MapBase,pSmi->MapSize);
#endif

    //	pSmi->videoRAMKBytes = 8 * 1024 - FB_RESERVE4USB / 1024;

    pSmi->videoRAMKBytes = total_memory - FB_RESERVE4USB / 1024;
    total_video_memory_k = pSmi->videoRAMKBytes;
    pSmi->videoRAMBytes = pSmi->videoRAMKBytes * 1024;
    pScrn->videoRam = pSmi->videoRAMKBytes;

    pSmi->FBReserved = pSmi->videoRAMBytes - 4096;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
            "BDEBUG: Setting primary memory, free_video_memory is %d, total_memory is %d\n",
            free_video_memory, total_memory);
    if (xf86IsEntityShared(pScrn->entityList[0])) {
        pScrn->videoRam = (total_memory - FB_RESERVE4USB / 1024) / 2;
#if 0
        if (pSmi->IsSecondary) {
            //	pScrn->videoRam = 4 * 1024 - FB_RESERVE4USB / 1024;
            pScrn->videoRam = free_video_memory;
        } else {
            free_video_memory = total_memory - FB_RESERVE4USB / 1024;
            pScrn->videoRam = free_video_memory;
            //	pScrn->videoRam = 4 * 1024;
        }
    } else {
        free_video_memory = total_memory - FB_RESERVE4USB / 1024;
        pScrn->videoRam = free_video_memory;
#endif
    }
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: free_video_memory is %d\n", free_video_memory);
    pSmi->videoRAMKBytes = pScrn->videoRam;
    pSmi->videoRAMBytes = pScrn->videoRam * 1024;
    pSmi->fbMapOffset = pScrn->videoRam * 1024;

    if (!pSmi->IsSecondary) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Using %dk of videoram for primary head\n",
                pScrn->videoRam);
        pSmi->FBOffset = 0;
        //	pScrn->fbOffset = pSmi->FBOffset + pSmi->fbMapOffset;
        pScrn->fbOffset = pSmi->FBOffset;
        pSmi->fbMapOffset = pSmi->FBOffset;
    } else {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Using %dk of videoram for secondary head\n",
                pScrn->videoRam);
#if 1
        pScrn->fbOffset =  pScrn->videoRam * 1024;
        pSmi->FBOffset = pScrn->fbOffset;
        pSmi->fbMapOffset = pSmi->FBOffset;
#else
        pSmi->FBOffset = total_memory * 1024 - FB_RESERVE4USB - free_video_memory * 1024;
        pScrn->fbOffset = pSmi->FBOffset;
#endif
        xf86DrvMsg("", X_INFO, "CRT, free_video_memory is %dK, FBOffset is 0x%x\n", free_video_memory, pSmi->FBOffset);
    }
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "smi_setvideomem_750, pScrn->fbOffset is 0x%x\n", pScrn->fbOffset);

    return (TRUE);
}

Bool smi_mapmemory_750(ScrnInfoPtr pScrn, SMIPtr pSmi)
{
	vgaHWPtr	hwp;
    CARD32		memBase;	
	int total_memory;
	unsigned int tmp;
	SMIRegPtr pRegPtr = (SMIRegPtr)pSmi->pEnt_private->ptr;

	ENTER();

	pRegPtr->mmio_require++;
	/* #1 map mmio */
	if(pRegPtr->MMIOBase){	
		/* 	this entity had been mapped 
			and this is secondary head 
			don't use "pSmi->IsSecondary" above,because
			secondary screen also need map mmio when fist screen already
			unmapped the mmio */
			
	}else{	
		/* the first time map mmio */
		pRegPtr->MapSize = 0x200000;	    
#ifndef	XSERVER_LIBPCIACCESS	
	    memBase = pSmi->PciInfo->memBase[1];
		pRegPtr->MMIOBase = xf86MapPciMem (pScrn->scrnIndex, 
							VIDMEM_MMIO | VIDMEM_MMIO_32BIT,
							pSmi->PciTag,
							memBase,							
							pRegPtr->MapSize,
							);
		
	    if(!pRegPtr->MMIOBase){
	        XERR("Could not map MMIO registers.\n");
	        return (FALSE);
	    }
#else
	    memBase = PCI_REGION_BASE(pSmi->PciInfo, 1, REGION_MEM); //caesar modified
	    {
	        void ** result = (void**)&pRegPtr->MMIOBase;
	        int  err = pci_device_map_range(
					pSmi->PciInfo,
	                memBase,	                
	                pRegPtr->MapSize,
	                PCI_DEV_MAP_FLAG_WRITABLE,
	                result);	
	        if (err){
				XERR("map mmio failed\n");
	            return (FALSE); 
        	}
	    }
#endif		
	}
		
	pSmi->MapBase = pRegPtr->MMIOBase;	
	pSmi->MapSize = pRegPtr->MapSize;
	
	/* stupid stuffs */
    pSmi->DPRBase = pSmi->MapBase + 0x100000;
    pSmi->VPRBase = pSmi->MapBase + 0x000000;
    pSmi->CPRBase = pSmi->MapBase + 0x090000;
    pSmi->DCRBase = pSmi->MapBase + 0x080000;
    pSmi->SCRBase = pSmi->MapBase + 0x000000;
    pSmi->IOBase = 0;
    pSmi->DataPortBase = pSmi->MapBase + 0x110000;
    pSmi->DataPortSize = 0x10000;


	/* #2 map self-video memory */
#ifndef	XSERVER_LIBPCIACCESS	
    pScrn->memPhysBase = pSmi->PciInfo->memBase[0];
#else
    pScrn->memPhysBase = PCI_REGION_BASE(pSmi->PciInfo, 0, REGION_MEM); //caesar modified
#endif

	if(!pRegPtr->total_videoRam){
		/* get total video memory size */
		tmp = READ_SCR(pSmi, 0x04);
		tmp = ( tmp >> 12) & 0x3;
		switch (tmp){
			case 0:
				total_memory = 16 * 1024*1024;
				break;
			case 1:
				total_memory = 32 * 1024*1024;
				break;
			case 2:
				total_memory = 64 * 1024*1024;
				break;
			case 3:
				total_memory = 8 * 1024*1024;
				break;
			default:
				XERR("Error Detecting Total Memory\n");
				break;
		}		
		pRegPtr->total_videoRam = total_memory;
		XINF("sm750 total memory = %d\n",total_memory);	
	}else{
		total_memory = pRegPtr->total_videoRam;	
	}
	
	/* dummy stuffs */
	if(pRegPtr->DualHead)		
		pSmi->videoRAMBytes = total_memory >> 1;
	else
		pSmi->videoRAMBytes = total_memory;

	pSmi->videoRAMKBytes = pSmi->videoRAMBytes >> 10;
	pScrn->videoRam = pSmi->videoRAMKBytes;

	if(pSmi->IsSecondary)
	{
		pSmi->FBOffset = total_memory >> 1;
		pScrn->memPhysBase += total_memory >> 1;
	}
	else
	{
		pSmi->FBOffset = 0;
	}

	pSmi->fbMapOffset = 0;		/* always set fbmapOffset to zero,or fail will occure when use fbshadow  */	
	pScrn->fbOffset = pSmi->FBOffset;	/* don't remove this line*/
	/* cursor and reserved bound*/	
	pSmi->FBCursorOffset = pSmi->FBOffset + pSmi->videoRAMBytes - 2048;
	pSmi->FBReserved = pSmi->FBOffset + pSmi->videoRAMBytes - 4096;
	
#ifndef	XSERVER_LIBPCIACCESS	
	pSmi->FBBase = xf86MapPciMem(pScrn->scrnIndex,
		   VIDMEM_FRAMEBUFFER, pSmi->PciTag,
		   pScrn->memPhysBase + pSmi->fbMapOffset,
		   pSmi->videoRAMBytes);

	if (!pSmi->FBBase){
		XERR("MAP framebuffer failed.\n");
		LEAVE(FALSE);
	}
#else
	{
		void	**result = (void**)&pSmi->FBBase;					
		int err = pci_device_map_range(pSmi->PciInfo,
			   pScrn->memPhysBase + pSmi->fbMapOffset,
			   pSmi->videoRAMBytes,						   
			   PCI_DEV_MAP_FLAG_WRITABLE|
			   PCI_DEV_MAP_FLAG_WRITE_COMBINE,
			   result);
		if (err){
			XERR("MAP framebuffer failed.\n");
			LEAVE(FALSE); 
		}
	}
#endif

   if (pSmi->IsLCD){
       pSmi->lcd = 1;
       /*
            * Hardcode panel size for now, we can add options for it later
            * Not that it seems to make any difference at all
            */
		//pSmi->lcdWidth = 1280;
		//pSmi->lcdHeight = 1024;
   }
   if (!pSmi->lcdWidth)
       pSmi->lcdWidth = pScrn->displayWidth;
   if (!pSmi->lcdHeight)
       pSmi->lcdHeight = pScrn->virtualY;
   return (TRUE);
}

void WaitForNotBusy_750(SMIPtr pSmi)
{

    DWORD dwVal;
    int i;
    if (pSmi->Chipset != SMI_MSOCE)
        return;

    for (i = 0x1000000; i > 0; i--)
    {
        dwVal = regRead32(pSmi, SYSTEM_CTRL);
        if ((FIELD_GET(dwVal, SYSTEM_CTRL, DE_STATUS) == SYSTEM_CTRL_DE_STATUS_IDLE) &&
                (FIELD_GET(dwVal, SYSTEM_CTRL, DE_FIFO) == SYSTEM_CTRL_DE_FIFO_EMPTY) &&
                (FIELD_GET(dwVal, SYSTEM_CTRL, CSC_STATUS) == SYSTEM_CTRL_CSC_STATUS_IDLE) &&
                (FIELD_GET(dwVal, SYSTEM_CTRL, DE_MEM_FIFO ) == SYSTEM_CTRL_DE_MEM_FIFO_EMPTY) &&
                TRUE
           )
        {
            break;
        }
    }		
}
/*
   void DiableOverlay(SMIPtr pSmi)
   {
   DWORD dwVal;
   dwVal = READ_VPR(pSmi, 0x00);
   WRITE_VPR (pSmi, 0x00, dwVal & 0xfffffffb);
   return;
   }
   void EnableOverlay(SMIPtr pSmi)
   {
   DWORD dwVal;
   dwVal = READ_VPR(pSmi, 0x00);
   WRITE_VPR (pSmi, 0x00, dwVal | 0x00000004);
   return;
   }*/
