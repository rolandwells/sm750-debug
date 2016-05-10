/* Header:   //Mercury/Projects/archives/XFree86/4.0/smi_driver.c-arc   1.42   03 Jan 2001 13:52:16   Frido  $ */

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
#include "smi_501.h"

#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
#include "xf86Resources.h"
#include "xf86RAC.h"
#endif

#include "xf86DDC.h"
#include "xf86int10.h"
#include "vbe.h"
#include "shadowfb.h"


#include "globals.h"
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,7,0,0,0)
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


extern unsigned int	total_video_memory_k;
/*
 * Add comment here about this module. 
 */

int hw_rev = 0;

/* Mode table. */
mode_table_t mode_table_501[] = {
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
    {1792, 1360, 1424, 112, POSITIVE, 795, 768, 771, 6, POSITIVE,
        85500000, 47712, 60, 39, 11, 0, 1},

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

    // End of table.
    {0, 0, 0, 0, NEGATIVE, 0, 0, 0, 0, NEGATIVE, 0, 0, 0, 0, 0, 0, 0},
};

extern int	entity_priv_index[MAX_ENTITIES];
int		free_video_memory = -1;

/**********************************************************************
 * regRead32
 *    Read the value of the 32-bit register specified by nOffset
 **********************************************************************/
ULONG
regRead32 (SMIPtr pSmi, ULONG nOffset)
{
    ULONG result;
    result = READ_SCR (pSmi, nOffset);
    /* ErrorF("READ_SCR %8X from register %8X\n", result, nOffset); */
    return (result);
}

/**********************************************************************
 * regWrite32
 *    Write the 32-bit value, nData, to the 32-bit register specified by
 *    nOffset
 **********************************************************************/
    void
regWrite32 (SMIPtr pSmi, ULONG nOffset, ULONG nData)
{
    /* ErrorF("MMIOWriting %8X to register %8X\n", nData, nOffset);  */
    WRITE_SCR (pSmi, nOffset, nData);
}


/* Perform a rounded division. */
    LONG
roundDiv (LONG num, LONG denom)
{
    /* n / d + 1 / 2 = (2n + d) / 2d */
    return (2 * num + denom) / (2 * denom);
}

/* Finds clock closest to the requested. */
    LONG
findClock_501 (LONG requested_clock, clock_select_t * clock, display_t display)
{
    LONG mclk;
    INT divider, shift;
    LONG best_diff = 0x7fffffff;

    INT divby2 = 0, i = 0;

    divby2 = (hw_rev < 0xC0) ? 0 : 1;

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


/* Find the requested mode in the mode table. */
    mode_table_t *
findMode_501 (mode_table_t * mode_table, INT width, INT height, INT refresh_rate)
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

/* Converts the VESA timing into Voyager timing. */
    void
adjustMode_501 (mode_table_t * vesaMode, mode_table_t * mode, display_t display)
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
    if (hw_rev >= 0xC0)
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
setModeRegisters_501 (SMIPtr pSmi, reg_table_t * register_table,
        mode_table_t * mode, display_t display, INT bpp,
        INT fbPitch)
{
    clock_select_t clock = { 0, 0, 0, 0, 0, 0, 0 };
    unsigned long pllclock;
    /* Calculate the clock register values. */
    /*	Changed by Belcon at Jul 20, 2007 */
    //      findClock(mode->pixel_clock * 2, &clock, display);
    // xf86DrvMsg("", X_INFO, "Belcon: setModeRegisters(), %d\n", mode->pixel_clock);
    findClock_501 (mode->pixel_clock, &clock, display);

#if 0
    if (hw_rev >= 0xC0)
    {
        pllclock = regRead32 (pSmi, CURRENT_POWER_PLLCLOCK);

        pllclock &= FIELD_CLEAR (CURRENT_POWER_PLLCLOCK, MULTIPLE_M)
            & FIELD_CLEAR (CURRENT_POWER_PLLCLOCK, DIVIDE_N)
            & FIELD_CLEAR (CURRENT_POWER_PLLCLOCK, DIVIDEBY2)
            & FIELD_CLEAR (CURRENT_POWER_PLLCLOCK, INPUT_SELECT)
            & FIELD_CLEAR (CURRENT_POWER_PLLCLOCK, POWER);

        regWrite32 (pSmi, CURRENT_POWER_PLLCLOCK,
                FIELD_VALUE (pllclock, CURRENT_POWER_PLLCLOCK, MULTIPLE_M,
                    clock.multipleM) | FIELD_VALUE (pllclock,
                        CURRENT_POWER_PLLCLOCK,
                        DIVIDE_N,
                        clock.
                        dividerN) |
                FIELD_VALUE (pllclock, CURRENT_POWER_PLLCLOCK, DIVIDEBY2,
                    clock.divby2) | FIELD_SET (pllclock,
                        CURRENT_POWER_PLLCLOCK,
                        POWER,
                        ON) |
                FIELD_SET (pllclock, CURRENT_POWER_PLLCLOCK, INPUT_SELECT,
                    CRYSTAL) | FIELD_SET (pllclock,
                        CURRENT_POWER_PLLCLOCK,
                        TEST_OUTPUT,
                        DISABLE) |
                FIELD_SET (pllclock, CURRENT_POWER_PLLCLOCK, TESTMODE,
                    DISABLE));

    }
#endif

    if (display == PANEL)
    {
#if 0
        /* Set clock value for panel. */
        if (hw_rev >= 0xC0)
            register_table->clock
                = FIELD_SET (0, CURRENT_POWER_CLOCK, PLLCLK_SELECT, ENABLE) |
                FIELD_SET (0, CURRENT_POWER_CLOCK, P2XCLK_DIVIDER, 1) |
                FIELD_SET (0, CURRENT_POWER_CLOCK, P2XCLK_SHIFT, 0);
        else
#endif
            register_table->clock
                = (clock.mclk == 288000000
                        ? FIELD_SET (0, CURRENT_POWER_CLOCK, P2XCLK_SELECT, 288)
                        : FIELD_SET (0, CURRENT_POWER_CLOCK, P2XCLK_SELECT, 336))
                | (clock.divider == 1
                        ? FIELD_SET (0, CURRENT_POWER_CLOCK, P2XCLK_DIVIDER, 1)
                        : (clock.divider == 3
                            ? FIELD_SET (0, CURRENT_POWER_CLOCK, P2XCLK_DIVIDER, 3)
                            : FIELD_SET (0, CURRENT_POWER_CLOCK, P2XCLK_DIVIDER, 5)))
                | FIELD_VALUE (0, CURRENT_POWER_CLOCK, P2XCLK_SHIFT, clock.shift);



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
        /* Set clock value for CRT. */
        /*		if (hw_rev>=0xC0){
                        register_table->clock
                        = FIELD_SET(0, CURRENT_POWER_CLOCK, PLLCLK_SELECT, ENABLE)|
                        FIELD_SET(0, CURRENT_POWER_CLOCK, V2XCLK_DIVIDER, 1)|
                        FIELD_SET(0, CURRENT_POWER_CLOCK, V2XCLK_SHIFT, 0);
                        }else{
         */
        // xf86DrvMsg("", X_INFO, "Belcon:divby2 is %d, divider is %d, shift is %d\n", clock.divby2, clock.divider, clock.shift);
        register_table->clock
            = (clock.mclk == 288000000
                    ? FIELD_SET (0, CURRENT_POWER_CLOCK, V2XCLK_SELECT, 288)
                    : FIELD_SET (0, CURRENT_POWER_CLOCK, V2XCLK_SELECT, 336))
            | (clock.divby2 == 1
                    ? FIELD_SET (0, CURRENT_POWER_CLOCK, V1XCLK, ENABLE)
                    : FIELD_SET (0, CURRENT_POWER_CLOCK, V1XCLK, DISABLE))
            | (clock.divider == 1
                    ? FIELD_SET (0, CURRENT_POWER_CLOCK, V2XCLK_DIVIDER, 1)
                    : FIELD_SET (0, CURRENT_POWER_CLOCK, V2XCLK_DIVIDER, 3))
            | FIELD_VALUE (0, CURRENT_POWER_CLOCK, V2XCLK_SHIFT, clock.shift);
        // xf86DrvMsg("", X_INFO, "Belcon: clock is 0x%x\n", register_table->clock);


        /*		}*/

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
programMode_501 (SMIPtr pSmi, reg_table_t * register_table)
{
    DWORD value, gate, clock;
    DWORD palette_ram;
    DWORD fb_size, offset;

    /* Get current power configuration. */
    gate = regRead32 (pSmi, CURRENT_POWER_GATE);
    gate |= 0x08;			/* Enable power to 2D engine */
    gate = FIELD_SET (gate, CURRENT_POWER_GATE, CSC, ENABLE);
    gate = FIELD_SET (gate, CURRENT_POWER_GATE, ZVPORT, ENABLE);
    gate = FIELD_SET (gate, CURRENT_POWER_GATE, GPIO_PWM_I2C, ENABLE);

    clock = regRead32 (pSmi, CURRENT_POWER_CLOCK);

    /*Engine clock / Main clock */
    /*	Jul 20, 2007. Changed by Belcon to release PLL2 for V2CLK */
    clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, MCLK_SELECT, 336);
    clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, MCLK_DIVIDER, 3);
    clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, MCLK_SHIFT, 0);
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
    if (pSmi->MCLK) {
        switch (pSmi->MCLK) {
            case 112000:
                clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SELECT, 336);
                clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_DIVIDER, 3);
                clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SHIFT, 0);
                break;

            case 168000:
                clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SELECT, 336);
                clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_DIVIDER, 1);
                clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SHIFT, 1);
                break;

            case 96000:
                clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SELECT, 288);
                clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_DIVIDER, 3);
                clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SHIFT, 0);
                break;

            case 144000:
            default:
                clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SELECT, 288);
                clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_DIVIDER, 1);
                clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SHIFT, 1);
                break;
        }
    } else {
        clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SELECT, 288);
        clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_DIVIDER, 1);
        clock = FIELD_SET (clock, CURRENT_POWER_CLOCK, M2XCLK_SHIFT, 1);
    }

    /* Program panel. */
    if (register_table->display == PANEL)
    {
        /* Program clock, enable display controller. */
        gate = FIELD_SET (gate, CURRENT_POWER_GATE, DISPLAY, ENABLE);
        clock &= FIELD_CLEAR (CURRENT_POWER_CLOCK, P2XCLK_SELECT)
            & FIELD_CLEAR (CURRENT_POWER_CLOCK, P2XCLK_DIVIDER)
            & FIELD_CLEAR (CURRENT_POWER_CLOCK, P2XCLK_SHIFT);
        setPower_501 (pSmi, gate, clock | register_table->clock, 0);

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

        //xf86DrvMsg("", X_INFO, "PANEL: value is 0x%x\n", value);
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

        regWrite32 (pSmi, PANEL_HORIZONTAL_TOTAL,
                register_table->horizontal_total);
        regWrite32 (pSmi, PANEL_HORIZONTAL_SYNC,
                register_table->horizontal_sync);
        regWrite32 (pSmi, PANEL_VERTICAL_TOTAL, register_table->vertical_total);
        regWrite32 (pSmi, PANEL_VERTICAL_SYNC, register_table->vertical_sync);

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
        panelPowerSequence_501 (pSmi, PANEL_ON, 4);
        /* Turn on DAC          -- Belcon */
        regWrite32 (pSmi, MISC_CTRL,
                FIELD_SET (regRead32 (pSmi, MISC_CTRL), MISC_CTRL,
                    DAC_POWER, ENABLE));
        regWrite32 (pSmi, CRT_DISPLAY_CTRL,
                FIELD_SET (regRead32 (pSmi, CRT_DISPLAY_CTRL),
                    CRT_DISPLAY_CTRL, SELECT, PANEL));
    }

    /* Program CRT. */
    else
    {
        /* Program clock, enable display controller. */
        /* FIELD_CLEAR(CURRENT_POWER_CLOCK, V1XCLK) 
         * Added by Belcon
         */
        gate = FIELD_SET (gate, CURRENT_POWER_GATE, DISPLAY, ENABLE);
        clock &= FIELD_CLEAR (CURRENT_POWER_CLOCK, V2XCLK_SELECT)
            & FIELD_CLEAR (CURRENT_POWER_CLOCK, V2XCLK_DIVIDER)
            & FIELD_CLEAR (CURRENT_POWER_CLOCK, V1XCLK)
            & FIELD_CLEAR (CURRENT_POWER_CLOCK, V2XCLK_SHIFT);
        setPower_501 (pSmi, gate, clock | register_table->clock, 0);

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
        //xf86DrvMsg("", X_INFO, "CRT: value is 0x%x\n", value);

        /* Program CRT registers. */
        regWrite32 (pSmi, CRT_FB_ADDRESS,
                FIELD_SET (0, CRT_FB_ADDRESS, STATUS, PENDING) |
                FIELD_SET (0, CRT_FB_ADDRESS, EXT, LOCAL) |
                FIELD_VALUE (0, CRT_FB_ADDRESS, ADDRESS, value));

        //xf86DrvMsg("", X_INFO, "CRT: address is 0x%x\n", FIELD_GET(regRead32(pSmi, CRT_FB_ADDRESS), CRT_FB_ADDRESS, ADDRESS));
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
        setDPMS_501 (pSmi, DPMS_ON);
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

    VOID
SetMode_501 (SMIPtr pSmi, ULONG nWidth, ULONG nHeight, FLONG fMode, ULONG nHertz,
        display_t display, INT fbPitch, INT bpp)
{
    mode_table_t mode;
    pmode_table_t vesaMode;
    reg_table_t register_table;
    unsigned long value, gate, value_reg10;
    unsigned int mode_index = 0;

    /* Locate the mode */
    // xf86DrvMsg("", X_INFO, "Belcon, SetMode(%s), %dx%dx%d, nHertz is %d, fbPitch is %d\n", display ? "CRT" : "panel", nWidth, nHeight, bpp, nHertz, fbPitch);
    vesaMode = findMode_501 (mode_table_501, nWidth, nHeight, nHertz);
    /*
       xf86DrvMsg("", X_INFO, "Belcon: vesaMode is %p\n", vesaMode);
       xf86DrvMsg("", X_INFO, "Belcon, SetMode(), find mode %d x %d @ %d, pixelclock is %d, hfreq is %d\n", vesaMode->horizontal_display_end, vesaMode->vertical_display_end, vesaMode->vertical_frequency, vesaMode->pixel_clock, vesaMode->horizontal_frequency);
     */

    hw_rev = FIELD_GET (regRead32 (pSmi, DEVICE_ID), DEVICE_ID, REVISION_ID);
    if (vesaMode != NULL)
    {
        // xf86DrvMsg("", X_INFO, "Belcon, vesaMode != NULL\n");
        /* Convert VESA timing into Voyager timing */
        adjustMode_501 (vesaMode, &mode, display);

        /* Fill the register structure */
        setModeRegisters_501 (pSmi, &register_table, &mode, display, bpp, fbPitch);

        /* Program the registers */
        programMode_501 (pSmi, &register_table);
#ifndef AUTO_CLOCK
        if ((hw_rev >= 0xc0) && (display == PANEL))
        {
            //SM502 use programable PLL
            //change to mode0
            value = regRead32 (pSmi, POWER_MODE_CTRL);
            mode_index = FIELD_GET (value, POWER_MODE_CTRL, MODE);
            // Mode 0, Clock Register at MMIO_base + 0x000044
            // Mode 1, Clock Register at MMIO_base + 0x00004c
            value = FIELD_SET (value, POWER_MODE_CTRL, MODE, MODE0);
            regWrite32 (pSmi, POWER_MODE_CTRL, value);


            //configue 0x74  mode_table
            value = 0;
            // get M
            if (!FIELD_GET (regRead32 (pSmi, MISC_CTRL), MISC_CTRL, CRYSTAL))
                value = FIELD_VALUE (value, SYSTEM_PLL3_CLOCK, M, vesaMode->M);
            else
                value =
                    FIELD_VALUE (value, SYSTEM_PLL3_CLOCK, M, vesaMode->M * 2);

            // get N
            value = FIELD_VALUE (value, SYSTEM_PLL3_CLOCK, N, vesaMode->N);
            //      xf86DrvMsg("", X_NOTICE,"Belcon:%d %d %d %d %d, %d %d %d %d %d, %d %d %d, %d %d %d %d\n ", vesaMode->horizontal_total, vesaMode->horizontal_display_end, vesaMode->horizontal_sync_start, vesaMode->horizontal_sync_width, vesaMode->horizontal_sync_polarity, vesaMode->vertical_total, vesaMode->vertical_display_end, vesaMode->vertical_sync_start, vesaMode->vertical_sync_height, vesaMode->vertical_sync_polarity, vesaMode->pixel_clock, vesaMode->horizontal_frequency, vesaMode->vertical_frequency, vesaMode->M, vesaMode->N, vesaMode->bit15, vesaMode->bit31);     
            //      xf86DrvMsg("", X_NOTICE,"Belcon:bit15 is 0x%x \n ", vesaMode->bit15);   
            if (vesaMode->bit15)
                value = FIELD_SET (value, SYSTEM_PLL3_CLOCK, DIVIDE, 2);
            else
                value = FIELD_SET (value, SYSTEM_PLL3_CLOCK, DIVIDE, 1);

            value = FIELD_SET (value, SYSTEM_PLL3_CLOCK, INPUT, CRYSTAL);
            value = FIELD_SET (value, SYSTEM_PLL3_CLOCK, POWER, ON);
            regWrite32 (pSmi, SYSTEM_PLL3_CLOCK, value);

            //configue 0x44
            if (mode_index == 1)
                value = regRead32 (pSmi, POWER_MODE1_CLOCK);
            else if (mode_index == 0)
                value = regRead32 (pSmi, POWER_MODE0_CLOCK);

            value &= 0x00FFFFFF;
            //      xf86DrvMsg("", X_NOTICE,"Belcon:bit31 is 0x%x \n ", vesaMode->bit31);   
            //      xf86DrvMsg("", X_NOTICE,"Belcon:value is 0x%x \n ", value);

            if (vesaMode->bit31)
                value = FIELD_SET (value, POWER_MODE0_CLOCK, PLL3_P1XCLK, ENABLE);
            else
                value =
                    FIELD_SET (value, POWER_MODE0_CLOCK, PLL3_P1XCLK, DISABLE);
            //      xf86DrvMsg("", X_NOTICE,"Belcon:value is 0x%x \n ", value);
            /*set 122M*/
            value = FIELD_SET (value, POWER_MODE0_CLOCK, PLL3, DISABLE);
            value_reg10 = regRead32 (pSmi, DRAM_CTRL);
            /*
               xf86DrvMsg("", X_NOTICE,"caesar:value is 0x%x \n ", value_reg10, FIELD_GET (value_reg10, DRAM_CTRL, EMBEDDED) );	  
               if(FIELD_GET (value_reg10, DRAM_CTRL, EMBEDDED) == DRAM_CTRL_EMBEDDED_DISABLE)
               {
               value = FIELD_SET (value, POWER_MODE0_CLOCK, M2XCLK_SELECT, 336);
               value = FIELD_SET (value, POWER_MODE0_CLOCK, M2XCLK_DIVIDER, 3);	  
               value = FIELD_SET (value, POWER_MODE0_CLOCK, M2XCLK_SHIFT, 0);	
               }
             */

            //      xf86DrvMsg("", X_NOTICE,"Belcon:value is 0x%x \n ", value);
            regWrite32 (pSmi, POWER_MODE0_CLOCK, value);

            // Set up same value for Power Mode1 clock. The driver logic uses mode0 and mode1 inter-changably during suspend and resume.
            regWrite32 (pSmi, POWER_MODE1_CLOCK, value);

            // Don't forget to set up power mode0 gate properly.
            gate = regRead32 (pSmi, CURRENT_POWER_GATE);
            gate = FIELD_SET (gate, CURRENT_POWER_GATE, 2D, ENABLE);
            regWrite32 (pSmi, POWER_MODE0_GATE, gate);

            //configue 0x080024 and 0x080028 panel H-clock
            value = 0;
            value =
                FIELD_VALUE (value, PANEL_HORIZONTAL_TOTAL, TOTAL,
                        vesaMode->horizontal_total - 1);
            value =
                FIELD_VALUE (value, PANEL_HORIZONTAL_TOTAL, DISPLAY_END,
                        vesaMode->horizontal_display_end - 1);
            regWrite32 (pSmi, PANEL_HORIZONTAL_TOTAL, value);
            value = 0;
            value =
                FIELD_VALUE (value, PANEL_HORIZONTAL_SYNC, WIDTH,
                        vesaMode->horizontal_sync_width);
            value =
                FIELD_VALUE (value, PANEL_HORIZONTAL_SYNC, START,
                        vesaMode->horizontal_sync_start - 1);
            regWrite32 (pSmi, PANEL_HORIZONTAL_SYNC, value);
            //configue 0x08002c and 0x080030 panel V-clock
            value = 0;
            value =
                FIELD_VALUE (value, PANEL_VERTICAL_TOTAL, TOTAL,
                        vesaMode->vertical_total - 1);
            value =
                FIELD_VALUE (value, PANEL_VERTICAL_TOTAL, DISPLAY_END,
                        vesaMode->vertical_display_end - 1);
            regWrite32 (pSmi, PANEL_VERTICAL_TOTAL, value);
            value = 0;
            value =
                FIELD_VALUE (value, PANEL_VERTICAL_SYNC, HEIGHT,
                        vesaMode->vertical_sync_height);
            value =
                FIELD_VALUE (value, PANEL_VERTICAL_SYNC, START,
                        vesaMode->vertical_sync_start - 1);
            regWrite32 (pSmi, PANEL_VERTICAL_SYNC, value);
            //config polarity
            value = regRead32 (pSmi, PANEL_DISPLAY_CTRL);
            value =
                FIELD_VALUE (value, PANEL_DISPLAY_CTRL, HSYNC_PHASE,
                        vesaMode->horizontal_sync_polarity);
            value =
                FIELD_VALUE (value, PANEL_DISPLAY_CTRL, VSYNC_PHASE,
                        vesaMode->vertical_sync_polarity);
            regWrite32 (pSmi, PANEL_DISPLAY_CTRL, value);
            //PCI burst enable
            value = regRead32 (pSmi, SYSTEM_CTRL);
            value = FIELD_SET (value, SYSTEM_CTRL, PCI_BURST, ENABLE);
            /* Jul 16,2007, Added by Belcon */
            value = FIELD_SET (value, SYSTEM_CTRL, PCI_BURST_READ, ENABLE);

            regWrite32 (pSmi, SYSTEM_CTRL, value);
        }
#endif
    }


}

    VOID
panelSetMode_501 (SMIPtr pSmi, ULONG nWidth, ULONG nHeight, FLONG fMode,
        ULONG nHertz, INT fbPitch, INT bpp)
{
    //  xf86DrvMsg("", X_INFO, "Belcon: panelSetMode, nHertz is %ld\n", nHertz);
    SetMode_501 (pSmi, nWidth, nHeight, fMode, nHertz /* was nHertz */ , PANEL,	   fbPitch, bpp);
}

    VOID
crtSetMode_501 (SMIPtr pSmi, ULONG nWidth, ULONG nHeight, FLONG fMode,
        ULONG nHertz, INT fbPitch, INT bpp)
{
    //  xf86DrvMsg("", X_INFO, "Belcon: crtSetMode, %d x %d, mode is %d, nHertz is %d, fbPitch is %d, bpp is %d\n", nWidth, nHeight, fMode, nHertz, fbPitch, bpp);
    SetMode_501 (pSmi, nWidth, nHeight, fMode, nHertz, CRT, fbPitch, bpp);
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
setPower_501 (SMIPtr pSmi, ULONG nGates, ULONG Clock, int control_value)
{
    ULONG gate_reg = 0, clock_reg = 0;
    ULONG control_value_orig;

    /* Get current power mode. */



    control_value_orig = FIELD_GET (regRead32 (pSmi, POWER_MODE_CTRL),
            POWER_MODE_CTRL, MODE);

    switch (control_value)
    {
        case POWER_MODE_CTRL_MODE_MODE0:

            /* Switch from mode 0 or sleep to mode 1. */
            gate_reg = POWER_MODE0_GATE;
            clock_reg = POWER_MODE0_CLOCK;
            control_value = FIELD_SET (control_value, POWER_MODE_CTRL, MODE, MODE0);
            break;

        case POWER_MODE_CTRL_MODE_MODE1:

            /* Switch from mode 1 or sleep to mode 0. */
            gate_reg = POWER_MODE1_GATE;
            clock_reg = POWER_MODE1_CLOCK;
            control_value = FIELD_SET (control_value, POWER_MODE_CTRL, MODE, MODE1);
            break;

        case POWER_MODE_CTRL_MODE_SLEEP:

            /* Switch from mode 1/0 to sleep. */
            control_value = FIELD_SET (control_value, POWER_MODE_CTRL, MODE, SLEEP);
            break;

        default:
            /* Invalid mode */
            return;
    }

    /* Program new power mode. */

    if ((nGates > 0) && (Clock > 0))
    {
        regWrite32 (pSmi, gate_reg, nGates);
        regWrite32 (pSmi, clock_reg, Clock);
    }
    regWrite32 (pSmi, POWER_MODE_CTRL, control_value);

    /* When returning from sleep, wait until finished. */
    /*          IGX -- comment out for now, gets us in an infinite loop! */
    /*	while (FIELD_GET(regRead32(pSmi, POWER_MODE_CTRL),
        POWER_MODE_CTRL,
        SLEEP_STATUS) == POWER_MODE_CTRL_SLEEP_STATUS_ACTIVE) ;
     */

}

/* Set DPMS state. */
    VOID
setDPMS_501 (SMIPtr pSmi, DPMS_t state)
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
panelWaitVSync_501 (SMIPtr pSmi, INT vsync_count)
{
    ULONG status;
    ULONG timeout;

    while (vsync_count-- > 0)
    {
        /* Wait for end of vsync */
        timeout = 0;
        do
        {
            status = FIELD_GET (regRead32 (pSmi, CMD_INTPR_STATUS),
                    CMD_INTPR_STATUS, PANEL_SYNC);
            if (++timeout == VSYNCTIMEOUT)
                break;
        }
        while (status == CMD_INTPR_STATUS_PANEL_SYNC_ACTIVE);

        /* Wait for start of vsync */
        timeout = 0;
        do
        {
            status = FIELD_GET (regRead32 (pSmi, CMD_INTPR_STATUS),
                    CMD_INTPR_STATUS, PANEL_SYNC);
            if (++timeout == VSYNCTIMEOUT)
                break;
        }
        while (status == CMD_INTPR_STATUS_PANEL_SYNC_INACTIVE);
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
panelPowerSequence_501 (SMIPtr pSmi, panel_state_t on_off, INT vsync_delay)
{
    ULONG panelControl = regRead32 (pSmi, PANEL_DISPLAY_CTRL);

    if (on_off == PANEL_ON)
    {
        /* Turn on FPVDDEN. */
        panelControl = FIELD_SET (panelControl,
                PANEL_DISPLAY_CTRL, FPVDDEN, HIGH);
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVSync_501 (pSmi, vsync_delay);

        /* Turn on FPDATA. */
        panelControl = FIELD_SET (panelControl,
                PANEL_DISPLAY_CTRL, DATA, ENABLE);
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVSync_501 (pSmi, vsync_delay);

        /*  Turn on FPVBIAS. */
        panelControl = FIELD_SET (panelControl,
                PANEL_DISPLAY_CTRL, VBIASEN, HIGH);
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVSync_501 (pSmi, vsync_delay);

        /* Turn on FPEN. */
        panelControl = FIELD_SET (panelControl, PANEL_DISPLAY_CTRL, FPEN, HIGH);
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panelControl);
    }

    else
    {
        /* Turn off FPEN. */
        panelControl = FIELD_SET (panelControl, PANEL_DISPLAY_CTRL, FPEN, LOW);
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVSync_501 (pSmi, vsync_delay);

        /*  Turn off FPVBIASEN. */
        panelControl = FIELD_SET (panelControl,
                PANEL_DISPLAY_CTRL, VBIASEN, LOW);
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVSync_501 (pSmi, vsync_delay);

        /* Turn off FPDATA. */
        panelControl = FIELD_SET (panelControl,
                PANEL_DISPLAY_CTRL, DATA, DISABLE);
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVSync_501 (pSmi, vsync_delay);

        /* Turn off FPVDDEN. */
        panelControl = FIELD_SET (panelControl,
                PANEL_DISPLAY_CTRL, FPVDDEN, LOW);
        regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panelControl);
    }
}


/**********************************************************************
 *
 * panelUseCRT
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
panelUseCRT (SMIPtr pSmi, BOOL bEnable)
{
    ULONG crt_ctrl = 0;

    crt_ctrl = regRead32 (pSmi, CRT_DISPLAY_CTRL);

    if (bEnable)
        /* Enable CRT graphics plane */
        crt_ctrl = FIELD_SET (crt_ctrl, CRT_DISPLAY_CTRL, PLANE, ENABLE);
    else
        /* Disable CRT graphics plane */
        crt_ctrl = FIELD_SET (crt_ctrl, CRT_DISPLAY_CTRL, PLANE, DISABLE);

    regWrite32 (pSmi, CRT_DISPLAY_CTRL, crt_ctrl);
}

    VOID
panelUseLCD (SMIPtr pSmi, BOOL bEnable)
{
    ULONG panel_ctrl = 0;

    panel_ctrl = regRead32 (pSmi, PANEL_DISPLAY_CTRL);
    if (bEnable)
        /* Enable panel graphics plane */
        panel_ctrl = FIELD_SET (panel_ctrl, PANEL_DISPLAY_CTRL, PLANE, ENABLE);
    else
        /* Disable panel graphics plane */
        panel_ctrl = FIELD_SET (panel_ctrl, PANEL_DISPLAY_CTRL, PLANE, DISABLE);
    regWrite32 (pSmi, PANEL_DISPLAY_CTRL, panel_ctrl);

}
#if 0
Bool entity_init_501(ScrnInfoPtr pScrn, int entity)
{
    DevUnion	*pPriv;
    SMIRegPtr	pSMIEnt;

	ENTER();

    xf86DrvMsg(pScrn->scrnIndex, X_NOTICE, "SM501 Found\n");

    xf86SetEntitySharable(entity);
    if (entity_priv_index[pScrn->entityList[0]] > MAX_ENTITIES) {
        xf86DrvMsg(pScrn->scrnIndex, X_NOTICE, "Too many entities\n");
        return (FALSE);
    }
    if (entity_priv_index[pScrn->entityList[0]] < 0) {
        entity_priv_index[pScrn->entityList[0]] =
            xf86AllocateEntityPrivateIndex();
    }
    pPriv = xf86GetEntityPrivate (pScrn->entityList[0], entity_priv_index[pScrn->entityList[0]]);
    if (!pPriv->ptr) {
        pPriv->ptr = xnfcalloc (sizeof (SMIRegRec), 1);
        pSMIEnt = pPriv->ptr;
        pSMIEnt->lastInstance = -1;
        pSMIEnt->DualHead = FALSE;
    } else {
        pSMIEnt = pPriv->ptr;
        pSMIEnt->DualHead = TRUE;
    }
    pSMIEnt->lastInstance++;
    xf86SetEntityInstanceForScreen(pScrn, pScrn->entityList[0],
            pSMIEnt->lastInstance);
    LEAVE(TRUE);
}
#endif

Bool smi_setdepbpp_501(ScrnInfoPtr pScrn)
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

Bool smi_set_dualhead_501(ScrnInfoPtr pScrn, SMIPtr pSmi)
{
    if (xf86IsEntityShared (pScrn->entityList[0])) {
        SMIRegPtr	pSMIEnt = SMIEntPriv (pScrn);
        if (!xf86IsPrimInitDone(pScrn->entityList[0])) {
            xf86SetPrimInitDone(pScrn->entityList[0]);
            pSmi->IsPrimary = TRUE;
            pSmi->IsSecondary = FALSE;
            pSMIEnt->pPrimaryScrn = pScrn;
            pSmi->IsLCD = TRUE;
            pSmi->IsCRT = FALSE;
        } else if (pSMIEnt->DualHead) {
            pSmi->IsSecondary = TRUE;
            pSMIEnt->pSecondaryScrn = pScrn;
            pSmi->IsCRT = TRUE;
        } else {
            return (FALSE);
        }
    }
    return (TRUE);
}

Bool smi_setvideomem_501(int config, ScrnInfoPtr pScrn, SMIPtr pSmi)
{
    unsigned int	total_memory = 8 * 1024;
    CARD32		memBase;
    DWORD		value = 0;

    pSmi->MapSize = 0x200000;
    /*map MMIO*/
#ifndef	XSERVER_LIBPCIACCESS	
    memBase = pSmi->PciInfo->memBase[1];
    pSmi->MapBase = xf86MapPciMem (pScrn->scrnIndex, VIDMEM_MMIO | VIDMEM_MMIO_32BIT, pSmi->PciTag, memBase, pSmi->MapSize);
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
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: pScrn->display->virtualX is %d\n", pScrn->display->virtualX);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: map io memory, virtualX is %d, virtualY is %d\n", pScrn->virtualX, pScrn->virtualY);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: map io memory, lcdWidth is %d, lcdHeight is %d\n", pSmi->lcdWidth, pSmi->lcdHeight);
    if (pSmi->MapBase == NULL) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                "Internal error: could not map "
                "MMIO registers.\n");
        return (FALSE);
    }
    pSmi->DPRBase = pSmi->MapBase + 0x100000;
    pSmi->VPRBase = pSmi->MapBase + 0x000000;
    pSmi->CPRBase = pSmi->MapBase + 0x090000;
    pSmi->DCRBase = pSmi->MapBase + 0x080000;
    pSmi->SCRBase = pSmi->MapBase + 0x000000;
    pSmi->IOBase = 0;
    pSmi->DataPortBase = pSmi->MapBase + 0x110000;
    pSmi->DataPortSize = 0x10000;

    /*
       if ((SMI_MSOC == pSmi->Chipset) && !(pScrn->virtualX)) {
       return TRUE;
       }
     */
#ifndef	XSERVER_LIBPCIACCESS	
    pScrn->memPhysBase = pSmi->PciInfo->memBase[0];
#else
    pScrn->memPhysBase = PCI_REGION_BASE(pSmi->PciInfo, 0, REGION_MEM); //caesar modified
#endif	
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

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: Unmap io memory\n");
#ifndef XSERVER_LIBPCIACCESS
    xf86UnMapVidMem (pScrn->scrnIndex, (pointer) pSmi->MapBase, pSmi->MapSize);	
#else
    pci_device_unmap_range(pSmi->PciInfo,(pointer)pSmi->MapBase,pSmi->MapSize);
#endif
    pSmi->MapBase = NULL;

    //	pSmi->videoRAMKBytes = 8 * 1024 - FB_RESERVE4USB / 1024;
    pSmi->videoRAMKBytes = total_memory - FB_RESERVE4USB / 1024;
    total_video_memory_k = pSmi->videoRAMKBytes;
    pSmi->videoRAMBytes = pSmi->videoRAMKBytes * 1024;
    pScrn->videoRam = pSmi->videoRAMKBytes;

    pSmi->FBReserved = pSmi->videoRAMBytes - 4096;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: Setting primary memory, free_video_memory is %d, total_memory is %d\n", free_video_memory, total_memory);
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
        //xf86DrvMsg("", X_INFO, "CRT, free_video_memory is %dK, FBOffset is 0x%x\n", free_video_memory, pSmi->FBOffset);
    }
    //xf86DrvMsg(pScrn->scrnIndex, X_INFO, "smi_setvideomem_501, pScrn->fbOffset is 0x%x\n", pScrn->fbOffset);

    return (TRUE);
}

#if 1
Bool smi_mapmemory_501(ScrnInfoPtr pScrn, SMIPtr pSmi)
{
    vgaHWPtr	hwp;
    CARD32		memBase;

	ENTER();
#ifndef XSERVER_LIBPCIACCESS
    memBase = pSmi->PciInfo->memBase[1];
#else
    memBase = PCI_REGION_BASE(pSmi->PciInfo,1,REGION_MEM);	//caesar modified
#endif
    pSmi->MapSize = 0x200000;

    //if(pSmi->IsSecondary) pSmi->MapSize = 0x150000;	//pci_device_map_range will not map succesfully if the mem base and size are the same


    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: LINE(%d), virtualX is %d, virtualY is %d\n", __LINE__, pScrn->virtualX, pScrn->virtualY);

    /*	map MMIO	*/	
#ifndef XSERVER_LIBPCIACCESS	
    pSmi->MapBase = xf86MapPciMem (pScrn->scrnIndex, VIDMEM_MMIO | VIDMEM_MMIO_32BIT, pSmi->PciTag, memBase, pSmi->MapSize);
#else
    if (pSmi->IsSecondary) {
        SMIRegPtr	pSMIEnt = SMIEntPriv (pScrn);
        SMIPtr pTmp = SMIPTR (pScrn);
        pTmp = SMIPTR(pSMIEnt->pPrimaryScrn);
        pSmi->MapBase = pTmp->MapBase;
    }
    else {
        void **result = (void**)&pSmi->MapBase;
        int err = pci_device_map_range(pSmi->PciInfo,
                memBase,
                pSmi->MapSize,
                PCI_DEV_MAP_FLAG_WRITABLE,
                (void**)&pSmi->MapBase);
        if(err)	
            return FALSE;		
    }
#endif


    if (pSmi->MapBase == NULL) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                "Internal error: could not map "
                "MMIO registers.\n");
        return (FALSE);
    }

    pSmi->DPRBase = pSmi->MapBase + 0x100000;
    pSmi->VPRBase = pSmi->MapBase + 0x000000;
    pSmi->CPRBase = pSmi->MapBase + 0x090000;
    pSmi->DCRBase = pSmi->MapBase + 0x080000;
    pSmi->SCRBase = pSmi->MapBase + 0x000000;
    pSmi->IOBase = 0;
    pSmi->DataPortBase = pSmi->MapBase + 0x110000;
    pSmi->DataPortSize = 0x10000;

    /*
       if ((SMI_MSOC == pSmi->Chipset) && !(pScrn->virtualX)) {
       return TRUE;
       }
     */

#ifndef XSERVER_LIBPCIACCESS
    pScrn->memPhysBase = pSmi->PciInfo->memBase[0];
#else
    pScrn->memPhysBase = PCI_REGION_BASE(pSmi->PciInfo, 0, REGION_MEM);//caesar modified
#endif

    if (!pSmi->IsSecondary)
        pSmi->fbMapOffset = 0x0;
    /*
       else
       pSmi->fbMapOffset = 0x400000;
     */
       else
           pSmi->fbMapOffset = pScrn->videoRam * 1024;

       xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: LINE(%d), virtualX is %d, virtualY is %d\n", __LINE__, pScrn->virtualX, pScrn->virtualY);
       if (pSmi->videoRAMBytes) {
           xf86DrvMsg(pScrn->scrnIndex, X_INFO, "mapmemory: LINE(%d), fbMapOffset is 0x%x\n", __LINE__, pSmi->fbMapOffset);
           /*	MAP MEM	*/
#ifndef XSERVER_LIBPCIACCESS
           pSmi->FBBase = xf86MapPciMem(pScrn->scrnIndex,
                   VIDMEM_FRAMEBUFFER, pSmi->PciTag,
                   pScrn->memPhysBase + pSmi->fbMapOffset,
                   pSmi->videoRAMBytes);
#else
           {
               void **result = (void**)	&pSmi->FBBase;
               int err = pci_device_map_range(pSmi->PciInfo,
                       pScrn->memPhysBase + pSmi->fbMapOffset,
                       pSmi->videoRAMBytes,
                       PCI_DEV_MAP_FLAG_WRITABLE|
                       PCI_DEV_MAP_FLAG_WRITE_COMBINE,
                       result);
               if(err)			
                   LEAVE(FALSE);
           }
#endif		


           if (pSmi->FBBase == NULL) {
               xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                       "Internal error: could not "
                       "map framebuffer.\n");
               LEAVE(FALSE);
           }
       }
#if 0
       pSmi->FBOffset = 0;
       pScrn->fbOffset = pSmi->FBOffset + pSmi->fbMapOffset;
#endif
       xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: LINE(%d), virtualX is %d, virtualY is %d\n", __LINE__, pScrn->virtualX, pScrn->virtualY);
       SMI_EnableMmio(pScrn);
       xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: LINE(%d), virtualX is %d, virtualY is %d\n", __LINE__, pScrn->virtualX, pScrn->virtualY);

       pSmi->FBCursorOffset = pSmi->videoRAMBytes - 2048;
       pSmi->FBReserved = pSmi->videoRAMBytes - 4096;

       if (pSmi->IsLCD) {
           pSmi->lcd = 1;
           /*
            * Hardcode panel size for now, we can add options for it later
            * Not that it seems to make any difference at all
            */
           //pSmi->lcdWidth = 1280;
           //pSmi->lcdHeight = 1024;
       }
       xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: LINE(%d), virtualX is %d, virtualY is %d\n", __LINE__, pScrn->virtualX, pScrn->virtualY);
       if (!pSmi->lcdWidth)
           pSmi->lcdWidth = pScrn->displayWidth;
       if (!pSmi->lcdHeight)
           pSmi->lcdHeight = pScrn->virtualY;


       LEAVE(TRUE);
}
#else
Bool smi_mapmemory_501(ScrnInfoPtr pScrn, SMIPtr pSmi)
{
    vgaHWPtr	hwp;
    CARD32		memBase;

    //	memBase = pSmi->PciInfo->memBase[1];
    memBase = PCI_REGION_BASE(pSmi->PciInfo, 1, REGION_MEM); //caesar modified
    pSmi->MapSize = 0x200000;

    if (pSmi->IsSecondary) pSmi->MapSize = 0x150000; 	//pci_device_map_range will not map succesfully if the mem base and size are the same

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: LINE(%d), virtualX is %d, virtualY is %d\n", __LINE__, pScrn->virtualX, pScrn->virtualY);

    /*map MMIO*/
#ifndef XSERVER_LIBPCIACCESS
    pSmi->MapBase = xf86MapPciMem (pScrn->scrnIndex, VIDMEM_MMIO | VIDMEM_MMIO_32BIT, pSmi->PciTag, memBase, pSmi->MapSize);
#else
    {
        void	**result = (void**)&pSmi->MapBase;
        int	  err = pci_device_map_range(pSmi->PciInfo,
                memBase,
                pSmi->MapSize,
                PCI_DEV_MAP_FLAG_WRITABLE,
                (void**)&pSmi->MapBase);

        if (err)
            return (FALSE);
    }
#endif

    if (pSmi->MapBase == NULL) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                "Internal error: could not map "
                "MMIO registers.\n");
        return (FALSE);
    }

    pSmi->DPRBase = pSmi->MapBase + 0x100000;
    pSmi->VPRBase = pSmi->MapBase + 0x000000;
    pSmi->CPRBase = pSmi->MapBase + 0x090000;
    pSmi->DCRBase = pSmi->MapBase + 0x080000;
    pSmi->SCRBase = pSmi->MapBase + 0x000000;
    pSmi->IOBase = 0;
    pSmi->DataPortBase = pSmi->MapBase + 0x110000;
    pSmi->DataPortSize = 0x10000;

    /*
       if ((SMI_MSOC == pSmi->Chipset) && !(pScrn->virtualX)) {
       return TRUE;
       }
     */

    //	pScrn->memPhysBase = pSmi->PciInfo->memBase[0];
    pScrn->memPhysBase = PCI_REGION_BASE(pSmi->PciInfo, 0, REGION_MEM); //caesar modified

    if (!pSmi->IsSecondary)
        pSmi->fbMapOffset = 0x0;
    /*
       else
       pSmi->fbMapOffset = 0x400000;
     */
       else
           pSmi->fbMapOffset = pScrn->videoRam * 1024;

       xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: LINE(%d), virtualX is %d, virtualY is %d\n", __LINE__, pScrn->virtualX, pScrn->virtualY);
       if (pSmi->videoRAMBytes) {
           xf86DrvMsg(pScrn->scrnIndex, X_INFO, "mapmemory: LINE(%d), fbMapOffset is 0x%x\n", __LINE__, pSmi->fbMapOffset);

           /*Map MEM*/
#ifndef XSERVER_LIBPCIACCESS		
           pSmi->FBBase = xf86MapPciMem(pScrn->scrnIndex,
                   VIDMEM_FRAMEBUFFER, pSmi->PciTag,
                   pScrn->memPhysBase + pSmi->fbMapOffset,
                   pSmi->videoRAMBytes);
#else
           {
               void	**result = (void**)&pSmi->FBBase;
               int	  err = pci_device_map_range(pSmi->PciInfo,
                       pScrn->memPhysBase +
                       pSmi->fbMapOffset,
                       pSmi->videoRAMBytes,
                       PCI_DEV_MAP_FLAG_WRITABLE |
                       PCI_DEV_MAP_FLAG_WRITE_COMBINE,
                       result);

               if (err)
                   LEAVE(FALSE);
           }
#endif


           if (pSmi->FBBase == NULL) {
               xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                       "Internal error: could not "
                       "map framebuffer.\n");
               return (FALSE);
           }
       }
#if 0
       pSmi->FBOffset = 0;
       pScrn->fbOffset = pSmi->FBOffset + pSmi->fbMapOffset;
#endif
       xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: LINE(%d), virtualX is %d, virtualY is %d\n", __LINE__, pScrn->virtualX, pScrn->virtualY);
       SMI_EnableMmio(pScrn);
       xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: LINE(%d), virtualX is %d, virtualY is %d\n", __LINE__, pScrn->virtualX, pScrn->virtualY);

       pSmi->FBCursorOffset = pSmi->videoRAMBytes - 2048;
       pSmi->FBReserved = pSmi->videoRAMBytes - 4096;

       if (pSmi->IsLCD) {
           pSmi->lcd = 1;
           /*
            * Hardcode panel size for now, we can add options for it later
            * Not that it seems to make any difference at all
            */
           //pSmi->lcdWidth = 1280;
           //pSmi->lcdHeight = 1024;
       }
       xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: LINE(%d), virtualX is %d, virtualY is %d\n", __LINE__, pScrn->virtualX, pScrn->virtualY);
       if (!pSmi->lcdWidth)
           pSmi->lcdWidth = pScrn->displayWidth;
       if (!pSmi->lcdHeight)
           pSmi->lcdHeight = pScrn->virtualY;


       return (TRUE);
}
#endif
void WaitForNotBusy_501(SMIPtr pSmi)
{

    DWORD dwVal;
    int i;
    if ((pSmi->Chipset != SMI_MSOC) /*||(pSmi->Chipset != SMI_MSOCE)*/)
        return;

    for (i = 0x1000000; i > 0; i--)
    {
        dwVal = regRead32(pSmi, CMD_INTPR_STATUS);
        if ((FIELD_GET(dwVal, CMD_INTPR_STATUS, 2D_ENGINE) == CMD_INTPR_STATUS_2D_ENGINE_IDLE) &&
                (FIELD_GET(dwVal, CMD_INTPR_STATUS, 2D_FIFO) == CMD_INTPR_STATUS_2D_FIFO_EMPTY) &&
                (FIELD_GET(dwVal, CMD_INTPR_STATUS, 2D_SETUP) == CMD_INTPR_STATUS_2D_SETUP_IDLE) &&
                (FIELD_GET(dwVal, CMD_INTPR_STATUS, CSC_STATUS) == CMD_INTPR_STATUS_CSC_STATUS_IDLE) &&
                (FIELD_GET(dwVal, CMD_INTPR_STATUS, 2D_MEMORY_FIFO) == CMD_INTPR_STATUS_2D_MEMORY_FIFO_EMPTY) &&
                (FIELD_GET(dwVal, CMD_INTPR_STATUS, COMMAND_FIFO) == CMD_INTPR_STATUS_COMMAND_FIFO_EMPTY) &&
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
