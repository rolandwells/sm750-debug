/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  MODE.C --- SMI DDK 
*  This file contains the source code for the mode table.
* 
*******************************************************************/
#include "defs.h"
#include "chip.h"
#include "clock.h"
#include "hardware.h"
#include "power.h"
#include "mode.h"
#include "os.h"

#include "ddkdebug.h"

#define SCALE_CONSTANT                      (1 << 12)

/* Maximum panel size scaling */
#define MAX_PANEL_SIZE_WIDTH                1920
#define MAX_PANEL_SIZE_HEIGHT               1440

/*
 *  Timing parameter for some popular modes.
 *  Note that this table is made according to standard VESA parameters for the
 *  popular modes.
 */
static mode_parameter_t gModeParamTable[] =
//const mode_parameter_t gModeParamTable[] =
{
/* 320 x 240 [4:3] */
 { 352, 320, 335,  8, NEG, 265, 240, 254, 2, NEG,  5600000, 15909, 60, NEG},

/* 400 x 300 [4:3] -- Not a popular mode */
 { 528, 400, 420, 64, NEG, 314, 300, 301, 2, NEG,  9960000, 18864, 60, NEG},
 
/* 480 x 272 -- Not a popular mode --> only used for panel testing */
/* { 525, 480, 482, 41, NEG, 286, 272, 274, 10, NEG, 9009000, 17160, 60, NEG}, */

/* 640 x 480  [4:3] */
/* The first 2 commented lines below are taken from SM502, the rest timing are
   taken from the VESA Monitor Timing Standard */
/* { 840, 640, 680, 64, NEG, 500, 480, 481, 3, NEG, 31500000, 37500, 75, NEG}, */
/* { 832, 640, 700, 56, NEG, 509, 480, 481, 3, NEG, 36000000, 43269, 85, NEG}, */
 { 800, 640, 656, 96, NEG, 525, 480, 490, 2, NEG, 25175000, 31500, 60, NEG},
 { 840, 640, 656, 64, NEG, 500, 480, 481, 3, NEG, 31500000, 37500, 75, NEG},
 { 832, 640, 696, 56, NEG, 509, 480, 481, 3, NEG, 36000000, 43269, 85, NEG},

/* 720 x 480  [3:2] */
 { 889, 720, 738,108, POS, 525, 480, 490, 2, NEG, 28000000, 31496, 60, NEG},

/* 720 x 540  [4:3] -- Not a popular mode */
 { 886, 720, 740, 96, POS, 576, 540, 545, 2, POS, 30600000, 34537, 60, NEG},

/* 800 x 480  [5:3] -- Not a popular mode */
 { 973, 800, 822, 56, POS, 524, 480, 490, 2, NEG, 30600000, 31449, 60, NEG},

/* 800 x 600  [4:3] */
/* The first 2 commented lines below are taken from SM502, the rest timing are
   taken from the VESA Monitor Timing Standard */
/* {1062, 800, 840,128, POS, 628, 600, 601, 4, POS, 40000000, 37665, 60, NEG}, */
/* {1054, 800, 842, 64, POS, 625, 600, 601, 3, POS, 56000000, 53131, 85, NEG}, */
 {1056, 800, 840,128, POS, 628, 600, 601, 4, POS, 40000000, 37879, 60, NEG},
 {1056, 800, 816, 80, POS, 625, 600, 601, 3, POS, 49500000, 46875, 75, NEG},
 {1048, 800, 832, 64, POS, 631, 600, 601, 3, POS, 56250000, 53674, 85, NEG},

/* 960 x 720  [4:3] -- Not a popular mode */
 {1245, 960, 992, 64, POS, 750, 720, 721, 3, POS, 56000000, 44980, 60, NEG},
      
/* 1024 x 600  [16:9] 1.7 */
 {1313,1024,1064,104, POS, 622, 600, 601, 3, POS, 49000000, 37319, 60, NEG},
     
/* 1024 x 768  [4:3] */
/* The first 2 commented lines below are taken from SM502, the rest timing are
   taken from the VESA Monitor Timing Standard */
/* {1340,1024,1060,136, NEG, 809, 768, 772, 6, NEG, 65000000, 48507, 60, NEG}, */
/* {1337,1024,1072, 96, NEG, 808, 768, 780, 3, NEG, 81000000, 60583, 75, NEG}, */
 {1344,1024,1048,136, NEG, 806, 768, 771, 6, NEG, 65000000, 48363, 60, NEG},
 {1312,1024,1040, 96, POS, 800, 768, 769, 3, POS, 78750000, 60023, 75, NEG},
 {1376,1024,1072, 96, POS, 808, 768, 769, 3, POS, 94500000, 68677, 85, NEG},
  
/* 1152 x 864  [4:3] -- Widescreen eXtended Graphics Array */
/* {1475,1152,1208, 96, NEG, 888, 864, 866, 3, NEG, 78600000, 53288, 60, NEG},*/
 {1475,1152,1208, 96, POS, 888, 864, 866, 3, POS, 78600000, 53288, 60, NEG},
 {1600,1152,1216,128, POS, 900, 864, 865, 3, POS,108000000, 67500, 75, NEG},
 
/* 1280 x 720  [16:9] -- HDTV (WXGA) */
 {1664,1280,1336,136, POS, 746, 720, 721, 3, POS, 74481000, 44760, 60, NEG},

/* 1280 x 768  [5:3] -- Not a popular mode */
 {1678,1280,1350,136, POS, 795, 768, 769, 3, POS, 80000000, 47676, 60, NEG},

/* 1280 x 800  [8:5] -- Not a popular mode */
 {1650,1280,1344,136, NEG, 824, 800, 800, 3, NEG, 81600000, 49455, 60, NEG},

/* 1280 x 960  [4:3] */
/* The first commented line below are taken from SM502, the rest timing are
   taken from the VESA Monitor Timing Standard */
/* {1618,1280,1330, 96, NEG, 977, 960, 960, 2, NEG, 94500000, 59259, 60, NEG},*/
 {1800,1280,1376,112, POS,1000, 960, 961, 3, POS,108000000, 60000, 60, NEG},
 {1728,1280,1344,160, POS,1011, 960, 961, 3, POS,148500000, 85938, 85, NEG},
    
/* 1280 x 1024 [5:4] */
/*{1626,1280,1332,112, POS,1046,1024,1024, 2, POS,102000000, 62731, 60, NEG}, */
/*{1344,1280,1308, 28, POS,1054,1024,1025, 3, POS, 85000000, 63244, 60, NEG}, */
 {1688,1280,1328,112, POS,1066,1024,1025, 3, POS,108000000, 63981, 60, NEG},
 {1688,1280,1296,144, POS,1066,1024,1025, 3, POS,135000000, 79976, 75, NEG},
 {1728,1280,1344,160, POS,1072,1024,1025, 3, POS,157500000, 91146, 85, NEG},

/* 1360 x 768 [16:9] */
 {1776,1360,1424,144, POS, 795, 768, 769, 3, POS, 84715000, 47700, 60, NEG},
 
/* 1366 x 768  [16:9] */
 {1722,1366,1424,112, NEG, 784, 768, 769, 3, NEG, 81000000, 47038, 60, NEG},
 
/* 1400 x 1050 [4:3] -- Hitachi TX38D95VC1CAH -- It is not verified yet, therefore
   temporarily disabled. */
// {1688,1400,1448,112, NEG,1068,1050,1051, 3, NEG,108000000, 61000, 60, NEG},
 
/* 1440 x 900  [8:5] -- Widescreen Super eXtended Graphics Array (WSXGA) */
 {1904,1440,1520,152, NEG, 932, 900, 901, 3, POS,106470000, 55919, 60, NEG},

/* 1440 x 960 [3:2] -- Not a popular mode */
 {1920,1440,1528,152, POS, 994, 960, 961, 3, POS,114509000, 59640, 60, NEG},

/* 1600 x 1200 [4:3]. -- Ultra eXtended Graphics Array */
 {2160,1600,1664,192, POS,1250,1200,1201, 3, POS,162000000, 75000, 60, NEG},
 {2160,1600,1664,192, POS,1250,1200,1201, 3, POS,202500000, 93750, 75, NEG},
 {2160,1600,1664,192, POS,1250,1200,1201, 3, POS,229500000,106250, 85, NEG},

/* 
 * The timing below is taken from the www.tinyvga.com/vga-timing.
 * With the exception of 1920x1080.
 */
 
/* 1680 x 1050 [8:5]. -- Widescreen Super eXtended Graphics Array Plus (WSXGA+) */ 
/* The first commented timing might be used for DVI LCD Monitor timing. */
/* {1840,1680,1728, 32, NEG,1080,1050,1053, 6, POS,119232000, 64800, 60, NEG}, */
 {2256,1680,1784,184, NEG,1087,1050,1051, 3, POS,147140000, 65222, 60, NEG},
 
/* 1792 x 1344 [4:3]. -- Not a popular mode */ 
 {2448,1792,1920,200, NEG,1394,1344,1345, 3, POS,204800000, 83660, 60, NEG},
 {2456,1792,1888,216, NEG,1417,1344,1345, 3, POS,261000000,106270, 75, NEG},
 
/* 1856 x 1392 [4:3]. -- Not a popular mode 
   The 1856 x 1392 @ 75Hz has not been tested due to high Horizontal Frequency
   where not all monitor can support it (including the developer monitor)
 */
 {2528,1856,1952,224, NEG,1439,1392,1393, 3, POS,218300000, 86353, 60, NEG},
/* {2560,1856,1984,224, NEG,1500,1392,1393, 3, POS,288000000,112500, 75, NEG},*/

/* 1920 x 1080 [16:9]. This is a make-up value, need to be proven. 
   The Pixel clock is calculated based on the maximum resolution of
   "Single Link" DVI, which support a maximum 165MHz pixel clock.
   The second values are taken from:
   http://www.tek.com/Measurement/App_Notes/25_14700/eng/25W_14700_3.pdf
 */
/* {2560,1920,2048,208, NEG,1125,1080,1081, 3, POS,172800000, 67500, 60, NEG}, */
 {2200,1920,2008, 44, NEG,1125,1080,1081, 3, POS,148500000, 67500, 60, NEG},
 
/* 1920 x 1200 [8:5]. -- Widescreen Ultra eXtended Graphics Array (WUXGA) */
 {2592,1920,2048,208, NEG,1242,1200,1201, 3, POS,193160000, 74522, 60, NEG},

/* 1920 x 1440 [4:3]. */
/* In the databook, it mentioned only support up to 1920x1440 @ 60Hz. 
   The timing for 75 Hz is provided here if necessary to do testing. - Some underflow
   has been noticed. */
 {2600,1920,2048,208, NEG,1500,1440,1441, 3, POS,234000000, 90000, 60, NEG},
/* {2640,1920,2064,224, NEG,1500,1440,1441, 3, POS,297000000,112500, 75, NEG}, */

/* End of table. */
 { 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG},
};

/* Static variable to store the mode information. */
static mode_parameter_t gPrimaryCurrentModeParam[MAX_SMI_DEVICE];
static mode_parameter_t gSecondaryCurrentModeParam[MAX_SMI_DEVICE];

/* 
 * Return a point to the gModeParamTable.
 * Function in other files used this to get the mode table pointer.
 */
mode_parameter_t *getStockModeParamTable()
{
    return (gModeParamTable);
}

/*
 * Return the size of the Stock Mode Param Table
 */
uint32_t getStockModeParamTableSize()
{
    return (sizeof(gModeParamTable) / sizeof(mode_parameter_t) - 1);
}

/* 
 * This function returns the current mode.
 * TODO: Need to re-write. This does not handle multiple devices.
 *       Might need to read the register value directly and do the calculation again.
 */
mode_parameter_t getCurrentModeParam(disp_control_t dispCtrl)
{
#if 1
    if (dispCtrl == PRIMARY_CTRL)
        return gPrimaryCurrentModeParam[getCurrentDevice()];
    else
        return gSecondaryCurrentModeParam[getCurrentDevice()];
#else
    /* There is a drawback here that it can not keep track of the clock polarity, 
       HSync Phase, and VSync Phase. */
    mode_parameter_t currentModeParam;
    uint32_t dispCtrlValue, value;
    
    if (dispCtrl == PRIMARY_CTRL)
    {
        dispCtrlValue = peekRegisterDWord(PRIMARY_DISPLAY_CTRL);
        
        value = peekRegisterDWord(PRIMARY_HORIZONTAL_TOTAL);
        currentModeParam.horizontal_total = FIELD_GET(value, PRIMARY_HORIZONTAL_TOTAL, TOTAL) + 1;
        currentModeParam.horizontal_display_end = FIELD_GET(value, PRIMARY_HORIZONTAL_TOTAL, DISPLAY_END) + 1;
            
        value = peekRegisterDWord(PRIMARY_HORIZONTAL_SYNC);
        currentModeParam.horizontal_sync_start = FIELD_GET(value, PRIMARY_HORIZONTAL_SYNC, START) + 1;
        currentModeParam.horizontal_sync_width = FIELD_GET(value, PRIMARY_HORIZONTAL_SYNC, WIDTH);
        
        currentModeParam.horizontal_sync_polarity = 
            (FIELD_GET(dispCtrlValue, PRIMARY_DISPLAY_CTRL, HSYNC_PHASE) == PRIMARY_DISPLAY_CTRL_HSYNC_PHASE_ACTIVE_HIGH) ? POS : NEG;

        /* Vertical timing. */
        value = peekRegisterDWord(PRIMARY_VERTICAL_TOTAL);
        currentModeParam.vertical_total = FIELD_GET(value, PRIMARY_VERTICAL_TOTAL, TOTAL) + 1;
        currentModeParam.vertical_display_end = FIELD_GET(value, PRIMARY_VERTICAL_TOTAL, DISPLAY_END) + 1;
        
        value = peekRegisterDWord(PRIMARY_VERTICAL_SYNC);
        currentModeParam.vertical_sync_start = FIELD_GET(value, PRIMARY_VERTICAL_SYNC, START) + 1;
        currentModeParam.vertical_sync_height = FIELD_GET(value, PRIMARY_VERTICAL_SYNC, HEIGHT);
        
        currentModeParam.vertical_sync_polarity =
            (FIELD_GET(dispCtrlValue, PRIMARY_DISPLAY_CTRL, VSYNC_PHASE) == PRIMARY_DISPLAY_CTRL_VSYNC_PHASE_ACTIVE_HIGH) ? POS : NEG;

        /* Refresh timing. */
        currentModeParam.pixel_clock = getPrimaryDispCtrlClock();
        
        currentModeParam.horizontal_frequency = 
            roundedDiv(currentModeParam.pixel_clock, currentModeParam.horizontal_total);
        currentModeParam.vertical_frequency = 
            roundedDiv(currentModeParam.horizontal_frequency, currentModeParam.vertical_total);
    
        /* Clock Phase. This clock phase only applies to Panel. */
        currentModeParam.clock_phase_polarity =
            (FIELD_GET(dispCtrlValue, PRIMARY_DISPLAY_CTRL, CLOCK_PHASE) == PRIMARY_DISPLAY_CTRL_CLOCK_PHASE_ACTIVE_HIGH) ? POS : NEG;
    }
    else
    {
        dispCtrlValue = peekRegisterDWord(SECONDARY_DISPLAY_CTRL);
        
        value = peekRegisterDWord(SECONDARY_HORIZONTAL_TOTAL);
        currentModeParam.horizontal_total = FIELD_GET(value, SECONDARY_HORIZONTAL_TOTAL, TOTAL);
        currentModeParam.horizontal_display_end = FIELD_GET(value, SECONDARY_HORIZONTAL_TOTAL, DISPLAY_END);
            
        value = peekRegisterDWord(SECONDARY_HORIZONTAL_SYNC);
        currentModeParam.horizontal_sync_start = FIELD_GET(value, SECONDARY_HORIZONTAL_SYNC, START);
        currentModeParam.horizontal_sync_width = FIELD_GET(value, SECONDARY_HORIZONTAL_SYNC, WIDTH);
        
        currentModeParam.horizontal_sync_polarity = 
            (FIELD_GET(dispCtrlValue, SECONDARY_DISPLAY_CTRL, HSYNC_PHASE) == SECONDARY_DISPLAY_CTRL_HSYNC_PHASE_ACTIVE_HIGH) ? POS : NEG;

        /* Vertical timing. */
        value = peekRegisterDWord(SECONDARY_VERTICAL_TOTAL);
        currentModeParam.vertical_total = FIELD_GET(value, SECONDARY_VERTICAL_TOTAL, TOTAL);
        currentModeParam.vertical_display_end = FIELD_GET(value, SECONDARY_VERTICAL_TOTAL, DISPLAY_END);
        
        value = peekRegisterDWord(SECONDARY_VERTICAL_SYNC);
        currentModeParam.vertical_sync_start = FIELD_GET(value, SECONDARY_VERTICAL_SYNC, START);
        currentModeParam.vertical_sync_height = FIELD_GET(value, SECONDARY_VERTICAL_SYNC, HEIGHT);
        
        currentModeParam.vertical_sync_polarity =
            (FIELD_GET(dispCtrlValue, SECONDARY_DISPLAY_CTRL, VSYNC_PHASE) == SECONDARY_DISPLAY_CTRL_VSYNC_PHASE_ACTIVE_HIGH) ? POS : NEG;

        /* Refresh timing. */
        currentModeParam.pixel_clock = getSecondaryDispCtrlClock();
        
        currentModeParam.horizontal_frequency = 
            roundedDiv(currentModeParam.pixel_clock, currentModeParam.horizontal_total);
        currentModeParam.vertical_frequency = 
            roundedDiv(currentModeParam.horizontal_frequency, currentModeParam.vertical_total);
    
        /* Clock Phase. This clock phase only applies to Panel. */
        currentModeParam.clock_phase_polarity =
            (FIELD_GET(dispCtrlValue, SECONDARY_DISPLAY_CTRL, CLOCK_PHASE) == SECONDARY_DISPLAY_CTRL_CLOCK_PHASE_ACTIVE_HIGH) ? POS : NEG;
    }
    
    return (currentModeParam);
#endif
}

/*
 *  Locate in-stock parameter table for the requested mode.
 *  Success: return a pointer to the mode_parameter_t entry.
 *  Fail: a NULL pointer.
 *
 */
mode_parameter_t *findVesaModeParam(
uint32_t width, 
uint32_t height, 
uint32_t refresh_rate)
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
 *  Convert the VESA timing into possible Voyager timing.
 *  If actual pixel clock is not equal to Vesa timing pixel clock.
 *  other parameter like horizontal total and sync have to be changed.
 *
 *  Input: Pointer to a Vesa mode parameters.
 *         Pointer to a an empty mode parameter structure to be filled.
 *         Actual pixel clock generated by SMI hardware.
 *
 *  Output:
 *      1) Fill up input structure mode_parameter_t with possible timing for Voyager.
 */
int32_t adjustVesaModeParam(
mode_parameter_t *pVesaMode, /* Pointer to Vesa mode parameter */
mode_parameter_t *pMode,     /* Pointer to Vogager mode parameter to be updated here */
uint32_t ulPClk         /* real pixel clock feasible by VGX */
)
{
    uint32_t blank_width, sync_start, sync_width;
    uint32_t sync_adjustment;

    /* Sanity check */
    if ( pVesaMode == (mode_parameter_t *)0 ||
         pMode     == (mode_parameter_t *)0 ||
         ulPClk  == 0)
    {
        return -1;
    }

    /* Copy VESA mode into SM750 mode. */
    *pMode = *pVesaMode;

    /* If it can generate the vesa reqiured pixel clock, and there are a minimum of
       24 pixel difference between the horizontal sync start and the horizontal display
       end, then there is nothing to change */
    if ((ulPClk == pVesaMode->pixel_clock) && 
        ((pVesaMode->horizontal_sync_start - pVesaMode->horizontal_display_end) > 24))
        return 0;

    pMode->pixel_clock = ulPClk; /* Update actual pixel clock into mode */

    /* Calculate the sync percentages of the VESA mode. */
    blank_width = pVesaMode->horizontal_total - pVesaMode->horizontal_display_end;
    sync_start = roundedDiv((pVesaMode->horizontal_sync_start -
                       pVesaMode->horizontal_display_end) * 100, blank_width);
    sync_width = roundedDiv(pVesaMode->horizontal_sync_width * 100, blank_width);

     /* Calculate the horizontal total based on the actual pixel clock and VESA line frequency. */
    pMode->horizontal_total = roundedDiv(pMode->pixel_clock,
                                    pVesaMode->horizontal_frequency);

    /* Calculate the sync start and width based on the VESA percentages. */
    blank_width = pMode->horizontal_total - pMode->horizontal_display_end;

#if 1//def SM750_AA
    {
        uint32_t sync_adjustment;
            
        /* There is minimum delay of 22 pixels between the horizontal display end 
           to the horizontal sync start. Therefore, the horizontal sync start value
           needs to be adjusted if the value falls below 22 pixels.
           The 22 pixels comes from the propagating delay from the CRT display to
           all the 11 display pipes inside SM750. The factor of 2 is caused by the
           double pixel support in SM750. The value used here is 24 to align to 
           8 bit character width.
         */
        sync_adjustment = roundedDiv(blank_width * sync_start, 100);
        if (sync_adjustment < 24)
            sync_adjustment = 24;
        pMode->horizontal_sync_start = pMode->horizontal_display_end + sync_adjustment;
    
        /* Check if the adjustment of the sync start will cause the sync width to go
           over the horizontal total. If it is, then reduce the width instead of
           changing the horizontal total.
         */
        /* Maximum value for sync width and back porch. */
        sync_adjustment = blank_width - sync_adjustment;
        pMode->horizontal_sync_width = roundedDiv(blank_width * sync_width, 100);
        if (sync_adjustment <= pMode->horizontal_sync_width)
            pMode->horizontal_sync_width = sync_adjustment/2;
    }
#else
    pMode->horizontal_sync_start = pMode->horizontal_display_end + roundedDiv(blank_width * sync_start, 100);
    pMode->horizontal_sync_width = roundedDiv(blank_width * sync_width, 100);
#endif
    
    /* Calculate the line and screen frequencies. */
    pMode->horizontal_frequency = roundedDiv(pMode->pixel_clock,
                                        pMode->horizontal_total);
    pMode->vertical_frequency = roundedDiv(pMode->horizontal_frequency,
                                      pMode->vertical_total);
    return 0;
}

/* 
 * Note about Centering:
 *  1. Centering algorithm is only provided in the Secondary Display Control
 *     path.
 *  2. In order to center the primary display, the auto-expansion enable bit
 *     must also be enabled. 
 *  3. Vertical Auto-centering does not work correctly, some lines on the 
 *     top portion are not displayed due to the delay line buffer needed 
 *     for the source and destination timing synchronization. Only horizontal
 *     centering is working.
 *  4. In order to have centering feature, the primary output should be routed
 *     back to the frame buffer using the ZV1 and use the secondary display
 *     control to display the image, which can also be centered with manual
 *     centering. However, the ZV1 has a limitation that it can not capture
 *     a mode with more than 85MHz pixel clock, which pretty much limit all
 *     the high mode (high panel size) --> might not be a good solution.
 */

#if 0
/*
 * This function center the source display screen on the destination screen.
 * It only works with Secondary control only, not with AutoExpansion. Might need to modify
 * this API in order to do more checking.
 *
 * Input:
 *      enable      - Enable/disable centering indicator
 *      srcWidth    - Source Width
 *      srcHeight   - Source Height
 *      dstWidth    - Destination Width
 *      dstHeight   - Destination Height
 */
void centerDisplay(
    uint32_t enable,               /* Enable scaling. */
    uint32_t srcWidth,             /* Source Width */
    uint32_t srcHeight,            /* Source Height */
    uint32_t dstWidth,             /* Destination Width */
    uint32_t dstHeight             /* Destination Height */
)
{
    uint32_t topLeftValue = 0, bottomRightValue = 0;
    uint32_t tlValue, brValue;
    
    DDKDEBUGPRINT((DISPLAY_LEVEL, "(centerDisplay) srcWidth: %d, srcHeight: %d, ", srcWidth, srcHeight));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "dstWidth: %d, dstHeight: %d\n", dstWidth, dstHeight));
    
    /* Enable centering. */
    if (enable == 1)
    {
        /* Center the vertical display */
        tlValue = 0;
        brValue = 0x07FF;
        if (dstHeight >= srcHeight)
        {
            tlValue = (dstHeight - srcHeight) >> 1;
            brValue = tlValue + srcHeight;
        }
        topLeftValue = FIELD_VALUE(topLeftValue, SECONDARY_AUTO_CENTERING_TL , TOP, tlValue);
        bottomRightValue = FIELD_VALUE(bottomRightValue, SECONDARY_AUTO_CENTERING_BR, BOTTOM, brValue);
        
        /* Center the horizontal display */
        tlValue = 0;
        brValue = 0x07FF;
        if (dstWidth >= srcWidth)
        {
            tlValue = (dstWidth - srcWidth) >> 1;
            brValue = tlValue + srcWidth;
        }
        topLeftValue = FIELD_VALUE(topLeftValue, SECONDARY_AUTO_CENTERING_TL , LEFT, tlValue);
        bottomRightValue = FIELD_VALUE(bottomRightValue, SECONDARY_AUTO_CENTERING_BR, RIGHT, brValue);
    
        pokeRegisterDWord(SECONDARY_AUTO_CENTERING_TL, topLeftValue);
        pokeRegisterDWord(SECONDARY_AUTO_CENTERING_BR, bottomRightValue);
        
        DDKDEBUGPRINT((DISPLAY_LEVEL, "centeringValue: %x\n", peekRegisterDWord(SECONDARY_AUTO_CENTERING_TL)));
    }
    else
    {
        /* Set the scaler value to 0 */
        pokeRegisterDWord(SECONDARY_AUTO_CENTERING_TL, 0);
        pokeRegisterDWord(SECONDARY_AUTO_CENTERING_BR, 0x07FF07FF);
        
        DDKDEBUGPRINT((DISPLAY_LEVEL, "centeringValue: %x\n", peekRegisterDWord(SECONDARY_AUTO_CENTERING_TL)));
    }
}
#endif

/*
 * This function scale the display data.
 *
 * Input:
 *      pLogicalMode    - Logical Mode parameters which is to be scaled
 */
int32_t scaleDisplay(
    logicalMode_t *pLogicalMode         /* Logical Mode */
)
{
    uint32_t value = 0, adjustedHorzTotal, adjustedPriFrequency, adjustedSecFrequency;
    uint32_t vertScaleFactor, horzScaleFactor, dstWidth, dstHeight;
    mode_parameter_t primaryModeParam, secondaryModeParam;
    logicalMode_t secLogicalMode;
    
    DDKDEBUGPRINT((DISPLAY_LEVEL, "(scaleDisplay) srcWidth: %d, srcHeight: %d, ", pLogicalMode->x, pLogicalMode->y));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "dstWidth: %d, dstHeight: %d\n", pLogicalMode->xLCD, pLogicalMode->yLCD));

    /* 
     * Enable scaling when the source width and height is smaller than the destination
     * width and height. 
     */
    if ((pLogicalMode->xLCD != 0) &&
        (pLogicalMode->yLCD != 0) &&
        ((pLogicalMode->x < pLogicalMode->xLCD) ||
         (pLogicalMode->y < pLogicalMode->yLCD)))
    {
        dstWidth = pLogicalMode->xLCD;
        dstHeight = pLogicalMode->yLCD;        
                
        /* Scale the vertical size */
        vertScaleFactor = 0;
        if (dstHeight > pLogicalMode->y)
            vertScaleFactor = (pLogicalMode->y - 1) * SCALE_CONSTANT / (dstHeight - 1);
        
        /* Scale the horizontal size */
        horzScaleFactor = 0;
        if (dstWidth > pLogicalMode->x)
            horzScaleFactor = (pLogicalMode->x - 1) * SCALE_CONSTANT / (dstWidth - 1);
        
        /* Enable Panel scaler path */
        if (pLogicalMode->dispCtrl == PRIMARY_CTRL)
        {
            mode_parameter_t *pVesaModeParam;
            
            /*
             * Disable all the scaler first before set the SECONDARY_CTRL display. 
             * If we didn't clear the expansion bit here, then the setCustomMode in 
             * SECONDARY_CTRL will fail.
             */
            value = peekRegisterDWord(SECONDARY_DISPLAY_CTRL);
            value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, EXPANSION, DISABLE);
            value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, LOCK_TIMING, DISABLE);
            pokeRegisterDWord(SECONDARY_DISPLAY_CTRL, value);
            
            /* Disable the Secondary scale path by setting the scaler value to 0 */
            pokeRegisterDWord(SECONDARY_SCALE , 0);
        
            /* Check for Vesa Mode timing for the intended panel display. */
            pVesaModeParam = (mode_parameter_t *)findVesaModeParam(pLogicalMode->xLCD, pLogicalMode->yLCD, pLogicalMode->hz);
            if (pVesaModeParam == (mode_parameter_t *)0)
                return (-1);
        
            /* Set the mode on the secondary channel */    
            secLogicalMode.x = pLogicalMode->xLCD;
            secLogicalMode.y = pLogicalMode->yLCD;
            secLogicalMode.hz = 60;
            secLogicalMode.baseAddress = pLogicalMode->baseAddress;
            secLogicalMode.bpp = pLogicalMode->bpp;
            secLogicalMode.dispCtrl = SECONDARY_CTRL;
            secLogicalMode.pitch = 0;
            secLogicalMode.xLCD = 0;
            secLogicalMode.yLCD = 0;
            if (setCustomMode(&secLogicalMode, pVesaModeParam) != 0)
                return (-1);
    
            /* Get the current mode parameter. */
            primaryModeParam = getCurrentModeParam(PRIMARY_CTRL);
            secondaryModeParam = getCurrentModeParam(SECONDARY_CTRL);
    
            /* Set up the vertical auto expansion values. For the line buffer, 
               always use the maximum number of line. */
            value = peekRegisterDWord(SECONDARY_VERTICAL_EXPANSION);
            value = FIELD_VALUE(value, SECONDARY_VERTICAL_EXPANSION, COMPARE_VALUE, 
                                ((primaryModeParam.vertical_display_end - 1) >> 3) & 0x00FF);
            value = FIELD_VALUE(value, SECONDARY_VERTICAL_EXPANSION, LINE_BUFFER, 4);
            value = FIELD_VALUE(value, SECONDARY_VERTICAL_EXPANSION, SCALE_FACTOR, vertScaleFactor);
            pokeRegisterDWord(SECONDARY_VERTICAL_EXPANSION, value);
        
            /* Set up the horizontal auto expansion values. */
            value = FIELD_VALUE(0, SECONDARY_HORIZONTAL_EXPANSION, COMPARE_VALUE, 
                                ((primaryModeParam.horizontal_display_end - 1) >> 3) & 0x00FF);
            value = FIELD_VALUE(value, SECONDARY_HORIZONTAL_EXPANSION, SCALE_FACTOR, horzScaleFactor);  
            pokeRegisterDWord(SECONDARY_HORIZONTAL_EXPANSION, value);            
            DDKDEBUGPRINT((DISPLAY_LEVEL, "vertScaleFactor: %x, horzScaleFactor: %x\n", vertScaleFactor, horzScaleFactor));
        
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
            value = peekRegisterDWord(SECONDARY_HORIZONTAL_TOTAL);
            value = FIELD_VALUE(value, SECONDARY_HORIZONTAL_TOTAL, TOTAL, adjustedHorzTotal); 
            pokeRegisterDWord(SECONDARY_HORIZONTAL_TOTAL, value);
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
            value = peekRegisterDWord(PRIMARY_HORIZONTAL_TOTAL);
            value = FIELD_VALUE(value, PRIMARY_HORIZONTAL_TOTAL, TOTAL, adjustedHorzTotal); 
            pokeRegisterDWord(PRIMARY_HORIZONTAL_TOTAL, value);
#endif

            DDKDEBUGPRINT((DISPLAY_LEVEL, "adjustedHorzTotal: %x\n", adjustedHorzTotal));
            DDKDEBUGPRINT((DISPLAY_LEVEL, "panelFreq: %x, crtFreq: %x, Src HT: %x, src VDE: %x, dstVDE: %x\n", 
                          adjustedPriFrequency, adjustedSecFrequency, primaryModeParam.horizontal_total, 
                          primaryModeParam.vertical_display_end, secondaryModeParam.vertical_display_end));

            /* Enable Auto Expansion bit. */
            value = peekRegisterDWord(SECONDARY_DISPLAY_CTRL);
            value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, EXPANSION, ENABLE);
            value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, LOCK_TIMING, ENABLE);
            pokeRegisterDWord(SECONDARY_DISPLAY_CTRL, value);
        }
        else
        {
            /* Secondary Control Scaling (Video Frame Buffer --> screen) */
            value = FIELD_VALUE(value, SECONDARY_SCALE , VERTICAL_SCALE, vertScaleFactor);
            value = FIELD_VALUE(value, SECONDARY_SCALE , HORIZONTAL_SCALE, horzScaleFactor);
            pokeRegisterDWord(SECONDARY_SCALE , value);
            DDKDEBUGPRINT((DISPLAY_LEVEL, "scaleFactor: %x\n", peekRegisterDWord(SECONDARY_SCALE)));
        }
        
        /* Set the interpolation when scaling is needed. */
        setInterpolation((horzScaleFactor == 0) ? 0 : 1, (vertScaleFactor == 0) ? 0 : 1);
    }
    else
    {
        /* Disable the interpolation if both display control are not in scaling mode. */
        if (pLogicalMode->dispCtrl == PRIMARY_CTRL)
        {
            /* Disable Panel scaler path */
            value = peekRegisterDWord(SECONDARY_DISPLAY_CTRL);
            value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, EXPANSION, DISABLE);
            value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, LOCK_TIMING, DISABLE);
            pokeRegisterDWord(SECONDARY_DISPLAY_CTRL, value);
            
            /* Disable interpolation */
            if (isScalingEnabled(SECONDARY_CTRL) != 1)
                setInterpolation(0, 0);
        }
        else if (isScalingEnabled(PRIMARY_CTRL) != 1)
        {
            /* Disable the Secondary scale path by setting the scaler value to 0 */
            pokeRegisterDWord(SECONDARY_SCALE , 0);
            
            /* Disable interpolation. */
            setInterpolation(0, 0);
        }
        
    }
    
    return 0;
}

/*
 * This function checks whether the scaling is enabled.
 *
 * Input:
 *      dispCtrl    - Display control to be checked.
 *
 * Output:
 *      0   - Scaling is not enabled
 *      1   - Scaling is enabled
 */
unsigned char isScalingEnabled(
    disp_control_t dispCtrl
)
{
    uint32_t value;
    
    /* If display control is not swapped, then check the expansion bit for PRIMARY_CTRL 
       and SECONDARY_SCALE register for SECONDARY_CTRL. */
    if (dispCtrl == PRIMARY_CTRL)
    {
        value = peekRegisterDWord(SECONDARY_DISPLAY_CTRL);
        if (FIELD_GET(value, SECONDARY_DISPLAY_CTRL, EXPANSION) == SECONDARY_DISPLAY_CTRL_EXPANSION_ENABLE)
            return 1;
    }
    else
    {
        value = peekRegisterDWord(SECONDARY_SCALE);
        if ((FIELD_GET(value, SECONDARY_SCALE, VERTICAL_SCALE) != 0) ||
            (FIELD_GET(value, SECONDARY_SCALE, HORIZONTAL_SCALE) != 0))
            return 1;
    }
    
    return 0;
}

/* 
 * Program the hardware for a specific video mode
 */
void programModeRegisters(
mode_parameter_t *pModeParam,   /* mode information about pixel clock, horizontal total, etc. */
uint32_t ulBpp,            /* Color depth for this mode */
uint32_t ulBaseAddress,    /* Offset in frame buffer */
uint32_t ulPitch,          /* Mode pitch value in byte: no of bytes between two lines. */
pll_value_t *pPLL               /* Pre-calculated values for the PLL */
)
{
    uint32_t ulTmpValue, ulReg, ulReservedBits;
    uint32_t palette_ram;
    uint32_t offset;

    /* Enable display power gate */
    ulTmpValue = peekRegisterDWord(CURRENT_GATE);
    ulTmpValue = FIELD_SET(ulTmpValue, CURRENT_GATE, DISPLAY, ON);
    setCurrentGate(ulTmpValue);

    if (pPLL->clockType == SECONDARY_PLL)
    {
        /* Secondary Display Control: SECONDARY_PLL */
        pokeRegisterDWord(SECONDARY_PLL_CTRL, formatPllReg(pPLL)); 

        /* Frame buffer base for this mode */
        pokeRegisterDWord(SECONDARY_FB_ADDRESS,
              FIELD_SET(0, SECONDARY_FB_ADDRESS, STATUS, PENDING)
            | FIELD_SET(0, SECONDARY_FB_ADDRESS, EXT, LOCAL)
            | FIELD_VALUE(0, SECONDARY_FB_ADDRESS, ADDRESS, ulBaseAddress));

        /* Pitch value (Sometime, hardware people calls it Offset) */
        pokeRegisterDWord(SECONDARY_FB_WIDTH,
//              FIELD_VALUE(0, SECONDARY_FB_WIDTH, WIDTH, ulPitch)
              FIELD_VALUE(0, SECONDARY_FB_WIDTH, WIDTH, pModeParam->horizontal_display_end*ulBpp/8)
            | FIELD_VALUE(0, SECONDARY_FB_WIDTH, OFFSET, ulPitch));

        pokeRegisterDWord(SECONDARY_HORIZONTAL_TOTAL,
              FIELD_VALUE(0, SECONDARY_HORIZONTAL_TOTAL, TOTAL, pModeParam->horizontal_total - 1)
            | FIELD_VALUE(0, SECONDARY_HORIZONTAL_TOTAL, DISPLAY_END, pModeParam->horizontal_display_end - 1));

        pokeRegisterDWord(SECONDARY_HORIZONTAL_SYNC,
              FIELD_VALUE(0, SECONDARY_HORIZONTAL_SYNC, WIDTH, pModeParam->horizontal_sync_width)
            | FIELD_VALUE(0, SECONDARY_HORIZONTAL_SYNC, START, pModeParam->horizontal_sync_start - 1));

        pokeRegisterDWord(SECONDARY_VERTICAL_TOTAL,
              FIELD_VALUE(0, SECONDARY_VERTICAL_TOTAL, TOTAL, pModeParam->vertical_total - 1)
            | FIELD_VALUE(0, SECONDARY_VERTICAL_TOTAL, DISPLAY_END, pModeParam->vertical_display_end - 1));

        pokeRegisterDWord(SECONDARY_VERTICAL_SYNC,
              FIELD_VALUE(0, SECONDARY_VERTICAL_SYNC, HEIGHT, pModeParam->vertical_sync_height)
            | FIELD_VALUE(0, SECONDARY_VERTICAL_SYNC, START, pModeParam->vertical_sync_start - 1));

        /* Set control register value */
        ulTmpValue =        
            (pModeParam->vertical_sync_polarity == POS
            ? FIELD_SET(0, SECONDARY_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH)
            : FIELD_SET(0, SECONDARY_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW))
          | (pModeParam->horizontal_sync_polarity == POS
            ? FIELD_SET(0, SECONDARY_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH)
            : FIELD_SET(0, SECONDARY_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW))
#if 0            
          | FIELD_SET(0, SECONDARY_DISPLAY_CTRL, SELECT, SECONDARY)
#endif          
          | FIELD_SET(0, SECONDARY_DISPLAY_CTRL, TIMING, ENABLE)
          | FIELD_SET(0, SECONDARY_DISPLAY_CTRL, PLANE, ENABLE) 
          | (ulBpp == 8
            ? FIELD_SET(0, SECONDARY_DISPLAY_CTRL, FORMAT, 8)
            : (ulBpp == 16
            ? FIELD_SET(0, SECONDARY_DISPLAY_CTRL, FORMAT, 16)
            : FIELD_SET(0, SECONDARY_DISPLAY_CTRL, FORMAT, 32)));

        /* TODO: Check if the auto expansion bit can be cleared here */
        ulReg = peekRegisterDWord(SECONDARY_DISPLAY_CTRL)
              & FIELD_CLEAR(SECONDARY_DISPLAY_CTRL, VSYNC_PHASE)
              & FIELD_CLEAR(SECONDARY_DISPLAY_CTRL, HSYNC_PHASE)
              & FIELD_CLEAR(SECONDARY_DISPLAY_CTRL, SELECT)
              & FIELD_CLEAR(SECONDARY_DISPLAY_CTRL, TIMING)
              & FIELD_CLEAR(SECONDARY_DISPLAY_CTRL, PLANE)
              & FIELD_CLEAR(SECONDARY_DISPLAY_CTRL, FORMAT)
              & FIELD_CLEAR(SECONDARY_DISPLAY_CTRL, LOCK_TIMING)
              & FIELD_CLEAR(SECONDARY_DISPLAY_CTRL, EXPANSION);

        pokeRegisterDWord(SECONDARY_DISPLAY_CTRL, ulTmpValue | ulReg);

        /* Palette RAM. */
        palette_ram = SECONDARY_PALETTE_RAM;
        
        /* Save the current mode param */
        gSecondaryCurrentModeParam[getCurrentDevice()] = *pModeParam;
    }
    else 
    {
        /* Primary display control clock: PRIMARY_PLL */
        pokeRegisterDWord(PRIMARY_PLL_CTRL, formatPllReg(pPLL));

        /* Program primary PLL, if applicable */
        if (pPLL->clockType == PRIMARY_PLL)
        {
            pokeRegisterDWord(PRIMARY_PLL_CTRL, formatPllReg(pPLL));

            /* Program to Non-VGA mode when using primary PLL */
            pokeRegisterDWord(VGA_CONFIGURATION, 
                FIELD_SET(peekRegisterDWord(VGA_CONFIGURATION), VGA_CONFIGURATION, PLL, PRIMARY));
        }
        
        /* Frame buffer base for this mode */
        pokeRegisterDWord(PRIMARY_FB_ADDRESS,
              FIELD_SET(0, PRIMARY_FB_ADDRESS, STATUS, CURRENT)
            | FIELD_SET(0, PRIMARY_FB_ADDRESS, EXT, LOCAL)
            | FIELD_VALUE(0, PRIMARY_FB_ADDRESS, ADDRESS, ulBaseAddress));

        /* Pitch value (Sometime, hardware people calls it Offset) */
        pokeRegisterDWord(PRIMARY_FB_WIDTH,
              FIELD_VALUE(0, PRIMARY_FB_WIDTH, WIDTH, ulPitch)
            | FIELD_VALUE(0, PRIMARY_FB_WIDTH, OFFSET, ulPitch));

        pokeRegisterDWord(PRIMARY_WINDOW_WIDTH,
              FIELD_VALUE(0, PRIMARY_WINDOW_WIDTH, WIDTH, pModeParam->horizontal_display_end - 1)
            | FIELD_VALUE(0, PRIMARY_WINDOW_WIDTH, X, 0));

        pokeRegisterDWord(PRIMARY_WINDOW_HEIGHT,
              FIELD_VALUE(0, PRIMARY_WINDOW_HEIGHT, HEIGHT, pModeParam->vertical_display_end - 1)
            | FIELD_VALUE(0, PRIMARY_WINDOW_HEIGHT, Y, 0));

        pokeRegisterDWord(PRIMARY_PLANE_TL,
              FIELD_VALUE(0, PRIMARY_PLANE_TL, TOP, 0)
            | FIELD_VALUE(0, PRIMARY_PLANE_TL, LEFT, 0));

        pokeRegisterDWord(PRIMARY_PLANE_BR, 
              FIELD_VALUE(0, PRIMARY_PLANE_BR, BOTTOM, pModeParam->vertical_display_end - 1)
            | FIELD_VALUE(0, PRIMARY_PLANE_BR, RIGHT, pModeParam->horizontal_display_end - 1));

        pokeRegisterDWord(PRIMARY_HORIZONTAL_TOTAL,
              FIELD_VALUE(0, PRIMARY_HORIZONTAL_TOTAL, TOTAL, pModeParam->horizontal_total - 1)
            | FIELD_VALUE(0, PRIMARY_HORIZONTAL_TOTAL, DISPLAY_END, pModeParam->horizontal_display_end - 1));

        pokeRegisterDWord(PRIMARY_HORIZONTAL_SYNC,
              FIELD_VALUE(0, PRIMARY_HORIZONTAL_SYNC, WIDTH, pModeParam->horizontal_sync_width)
            | FIELD_VALUE(0, PRIMARY_HORIZONTAL_SYNC, START, pModeParam->horizontal_sync_start - 1));

        pokeRegisterDWord(PRIMARY_VERTICAL_TOTAL,
              FIELD_VALUE(0, PRIMARY_VERTICAL_TOTAL, TOTAL, pModeParam->vertical_total - 1)
            | FIELD_VALUE(0, PRIMARY_VERTICAL_TOTAL, DISPLAY_END, pModeParam->vertical_display_end - 1));

        pokeRegisterDWord(PRIMARY_VERTICAL_SYNC,
              FIELD_VALUE(0, PRIMARY_VERTICAL_SYNC, HEIGHT, pModeParam->vertical_sync_height)
            | FIELD_VALUE(0, PRIMARY_VERTICAL_SYNC, START, pModeParam->vertical_sync_start - 1));

        /* Set control register value */
        ulTmpValue =
            (pModeParam->clock_phase_polarity == POS
            ? FIELD_SET(0, PRIMARY_DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_HIGH)
            : FIELD_SET(0, PRIMARY_DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_LOW))
          | (pModeParam->vertical_sync_polarity == POS
            ? FIELD_SET(0, PRIMARY_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH)
            : FIELD_SET(0, PRIMARY_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW))
          | (pModeParam->horizontal_sync_polarity == POS
            ? FIELD_SET(0, PRIMARY_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH)
            : FIELD_SET(0, PRIMARY_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW))
          | FIELD_SET(0, PRIMARY_DISPLAY_CTRL, TIMING, ENABLE)
          | FIELD_SET(0, PRIMARY_DISPLAY_CTRL, PLANE, ENABLE)
          | (ulBpp == 8
            ? FIELD_SET(0, PRIMARY_DISPLAY_CTRL, FORMAT, 8)
            : (ulBpp == 16
            ? FIELD_SET(0, PRIMARY_DISPLAY_CTRL, FORMAT, 16)
            : FIELD_SET(0, PRIMARY_DISPLAY_CTRL, FORMAT, 32)));

        /* Added some masks to mask out the reserved bits. 
         * Sometimes, the reserved bits are set/reset randomly when 
         * writing to the PRIMARY_DISPLAY_CTRL, therefore, the register
         * reserved bits are needed to be masked out.
         */
        ulReservedBits = FIELD_SET(0, PRIMARY_DISPLAY_CTRL, RESERVED_1_MASK, ENABLE) |
                         FIELD_SET(0, PRIMARY_DISPLAY_CTRL, RESERVED_2_MASK, ENABLE) |
                         FIELD_SET(0, PRIMARY_DISPLAY_CTRL, RESERVED_3_MASK, ENABLE);

        ulReg = (peekRegisterDWord(PRIMARY_DISPLAY_CTRL) & ~ulReservedBits)
              & FIELD_CLEAR(PRIMARY_DISPLAY_CTRL, CLOCK_PHASE)
              & FIELD_CLEAR(PRIMARY_DISPLAY_CTRL, VSYNC_PHASE)
              & FIELD_CLEAR(PRIMARY_DISPLAY_CTRL, HSYNC_PHASE)
              & FIELD_CLEAR(PRIMARY_DISPLAY_CTRL, TIMING)
              & FIELD_CLEAR(PRIMARY_DISPLAY_CTRL, VERTICAL_PAN)
              & FIELD_CLEAR(PRIMARY_DISPLAY_CTRL, HORIZONTAL_PAN)
              & FIELD_CLEAR(PRIMARY_DISPLAY_CTRL, PLANE)
              & FIELD_CLEAR(PRIMARY_DISPLAY_CTRL, FORMAT);

        pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, ulTmpValue | ulReg);

        /* 
         * PRIMARY_DISPLAY_CTRL register seems requiring few writes
         * before a value can be succesfully written in.
         * Added some masks to mask out the reserved bits.
         * Note: This problem happens by design. The hardware will wait for the
         *       next vertical sync to turn on/off the plane.
         */
        while((peekRegisterDWord(PRIMARY_DISPLAY_CTRL) & ~ulReservedBits) != (ulTmpValue|ulReg))
        {
            pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, ulTmpValue | ulReg);
        }

        /* Palette RAM */
        palette_ram = PRIMARY_PALETTE_RAM;
        
        /* Save the current mode param */
        gPrimaryCurrentModeParam[getCurrentDevice()] = *pModeParam;
    }

    /* In case of 8-bpp, fill palette */
    if (ulBpp==8)
    {
        /* Start with RGB = 0,0,0. */
        unsigned char red = 0, green = 0, blue = 0;
        uint32_t gray = 0;
        for (offset = 0; offset < 256 * 4; offset += 4)
        {
            /* Store current RGB value. */
            pokeRegisterDWord(palette_ram + offset, gray
                                ? RGB((gray + 50) / 100,
                                      (gray + 50) / 100,
                                      (gray + 50) / 100)
                                : RGB(red, green, blue));

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
        ulTmpValue = 0x000000;
        for (offset = 0; offset < 256 * 4; offset += 4)
        {
            pokeRegisterDWord(palette_ram + offset, ulTmpValue);

            /* Advance RGB by 1,1,1. */
            ulTmpValue += 0x010101;
        }
    }
}

/* 
 * This function gets the available clock type
 *
 */
clock_type_t getClockType(disp_control_t dispCtrl)
{
    clock_type_t clockType;
    unsigned char vgaPLL;
    
    switch(getChipType())
    {
        case SM718:
        case SM750:
            switch (dispCtrl)
            {
                case PRIMARY_CTRL:
                    clockType = PRIMARY_PLL;
                    break;
                default:
                case SECONDARY_CTRL:
                    clockType = SECONDARY_PLL;
                    break;
            }
            break;

        default:
            clockType = PRIMARY_PLL;
            break;
    }
        
    return clockType;
}

/*
 * Input:
 *     1) pLogicalMode contains information such as x, y resolution and bpp.
 *     2) A user defined parameter table for the mode.
 *
 * This function calculate and programs the hardware to set up the
 * requested mode.
 *
 * This function allows the use of user defined parameter table if
 * predefined Vesa parameter table (gModeParamTable) does not fit.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
int32_t setCustomMode(logicalMode_t *pLogicalMode, mode_parameter_t *pUserModeParam)
{
    mode_parameter_t pModeParam; /* physical parameters for the mode */
    pll_value_t pll;
    uint32_t ulActualPixelClk, ulTemp;

    /* Do not let the user set the mode for the secondary channel when the scaling
       is enabled. 
       User needs to disable the scaling by setting the primary channel without 
       scaling first before he/she can set the mode on the secondary channel */
    if ((pLogicalMode->dispCtrl == SECONDARY_CTRL) &&
        (isScalingEnabled(PRIMARY_CTRL) == 1))
        return (-1);

    /*
     * Minimum check on mode base address.
     * At least it shouldn't be bigger than the size of frame buffer.
     */
    if (getFrameBufSize() <= pLogicalMode->baseAddress)
        return -1;
    
    /*
     * Set up PLL, a structure to hold the value to be set in clocks.
     */
    pll.inputFreq = DEFAULT_INPUT_CLOCK; /* Defined in CLOCK.H */

    /* Get the Clock Type */
    pll.clockType = getClockType(pLogicalMode->dispCtrl);

    /* 
     * Call calcPllValue() to fill up the other fields for PLL structure.
     * Sometime, the chip cannot set up the exact clock required by User.
     * Return value from calcPllValue() gives the actual possible pixel clock.
     */
    ulActualPixelClk = calcPllValue(pUserModeParam->pixel_clock, &pll);
    DDKDEBUGPRINT((DISPLAY_LEVEL, "Actual Pixel Clock: %d\n", ulActualPixelClk));

    /* 
     * Adjust Vesa mode parameter to feasible mode parameter for SMI hardware.
     */
    if (adjustVesaModeParam(pUserModeParam, &pModeParam, ulActualPixelClk) != 0 )
    {
        return -1;
    }

    /* If calling function don't have a preferred pitch value, 
       work out a 16 byte aligned pitch value.
    */
    if (pLogicalMode->pitch == 0)
    {
        /* 
         * Pitch value calculation in Bytes.
         * Usually, it is (screen width) * (byte per pixel).
         * However, there are cases that screen width is not 16 pixel aligned, which is
         * a requirement for some OS and the hardware itself.
         * For standard 4:3 resolutions: 320, 640, 800, 1024 and 1280, they are all
         * 16 pixel aligned and pitch is simply (screen width) * (byte per pixel).
         *   
         * However, 1366 resolution, for example, has to be adjusted for 16 pixel aligned.
         */

        ulTemp = (pLogicalMode->x + 15) & ~15; /* This calculation has no effect on 640, 800, 1024 and 1280. */
        pLogicalMode->pitch = ulTemp * (pLogicalMode->bpp / 8);
    }

    /* Program the hardware to set up the mode. */
    programModeRegisters( 
        &pModeParam,
        pLogicalMode->bpp, 
        pLogicalMode->baseAddress, 
        pLogicalMode->pitch, 
        &pll);
        
    return (0);
}

/*
 * Input:
 *     1) pLogicalMode contains information such as x, y resolution, bpp, xLCD, and yLCD.
 *     2) A user defined parameter table for the mode.
 *
 * This function calculate and programs the hardware to set up the
 * requested mode and scale the mode if necessary.
 *
 * This function allows the use of user defined parameter table if
 * predefined Vesa parameter table (gModeParamTable) does not fit.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
int32_t setCustomModeEx(logicalMode_t *pLogicalMode, mode_parameter_t *pUserModeParam)
{
    int32_t returnValue = 0;
    
    /* For this version, the userData needs to be set to 0. If not, then return error. */
    //if (pLogicalMode->userData != (void *)0)
    //    return (-1);
    
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
    returnValue = setCustomMode(pLogicalMode, pUserModeParam);
    if (returnValue == 0)
    {
        /* Scale display if necessary */
        returnValue = scaleDisplay(pLogicalMode);
    }
    
    return (returnValue);
}

/*
 * Input pLogicalMode contains information such as x, y resolution, bpp, 
 * xLCD and yLCD. The main difference between setMode and setModeEx are
 * these two parameters (xLCD and yLCD). Use this setModeEx API to set
 * expansion while setMode API for regular setmode without any expansion.
 * Refer to MODE.h for the details.
 *
 * This function calculate and programs the hardware to set up the
 * requested mode.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
_X_EXPORT int32_t setModeEx(logicalMode_t *pLogicalMode)
{
    mode_parameter_t *pVesaModeParam; /* physical parameters for the mode */
    uint32_t modeWidth, modeHeight;
    
    /* Conditions to set the mode when scaling is needed (xLCD and yLCD is not zeroes)
     *      1. PRIMARY_CTRL
     *          a. Set the primary display control timing to the actual display mode.
     *          b. Set the secondary display control timing to the mode that equals to
     *             the panel size.
     *      2. SECONDARY_CTRL
     *          a. Set the secondary display control timing to the mode that equals to
     *             the panel size.
     */
    if ((pLogicalMode->dispCtrl == SECONDARY_CTRL) &&
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
    
    /* 
     * Check if we already have physical timing parameter for this mode.
     */
    pVesaModeParam = (mode_parameter_t *)findVesaModeParam(modeWidth, modeHeight, pLogicalMode->hz);
    if (pVesaModeParam == (mode_parameter_t *)0)
        return -1;

    return(setCustomModeEx(pLogicalMode, pVesaModeParam));
}

/*
 * Input pLogicalMode contains information such as x, y resolution and bpp.
 * Refer to MODE.h for the details.
 *
 * This function calculate and programs the hardware to set up the
 * requested mode.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
int32_t setMode(logicalMode_t *pLogicalMode)
{
   	/*	note use-less code below 	*/
#if 0	
	/* Initialize the panel size to 0 */
    pLogicalMode->xLCD = 0;
    pLogicalMode->yLCD = 0;
    pLogicalMode->userData = (void *)0;
#endif	
    
    /* Call the setModeEx to set the mode. */
    return setModeEx(pLogicalMode);
}

/*
 *  setInterpolation
 *      This function enables/disables the horizontal and vertical interpolation
 *      for the secondary display control. Primary display control does not have
 *      this capability.
 *
 *  Input:
 *      enableHorzInterpolation - Flag to enable/disable Horizontal interpolation
 *      enableVertInterpolation - Flag to enable/disable Vertical interpolation
 */
void setInterpolation(
    uint32_t enableHorzInterpolation,
    uint32_t enableVertInterpolation
)
{
    uint32_t value;

    value = peekRegisterDWord(SECONDARY_DISPLAY_CTRL);
    
    if (enableHorzInterpolation)
        value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, HORIZONTAL_MODE, INTERPOLATE);
    else
        value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, HORIZONTAL_MODE, REPLICATE);
        
    if (enableVertInterpolation)
        value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, VERTICAL_MODE, INTERPOLATE);
    else
        value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, VERTICAL_MODE, REPLICATE);
        
    pokeRegisterDWord(SECONDARY_DISPLAY_CTRL, value);
}
