/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  MODE.H --- SMI DDK 
*  This file contains the definitions for the mode tables.
* 
*******************************************************************/
#include <xf86str.h>
#ifndef _MODE_H_
#define _MODE_H_

typedef enum _spolarity_t
{
    POS, /* positive */
    NEG, /* negative */
}
spolarity_t;

typedef struct _mode_parameter_t
{
    /* Horizontal timing. */
    uint32_t horizontal_total;
    uint32_t horizontal_display_end;
    uint32_t horizontal_sync_start;
    uint32_t horizontal_sync_width;
    spolarity_t horizontal_sync_polarity;

    /* Vertical timing. */
    uint32_t vertical_total;
    uint32_t vertical_display_end;
    uint32_t vertical_sync_start;
    uint32_t vertical_sync_height;
    spolarity_t vertical_sync_polarity;

    /* Refresh timing. */
    uint32_t pixel_clock;
    uint32_t horizontal_frequency;
    uint32_t vertical_frequency;
    
    /* Clock Phase. This clock phase only applies to Panel. */
    spolarity_t clock_phase_polarity;
}
mode_parameter_t;

typedef enum _disp_control_t
{
    PRIMARY_CTRL = 0,
    SECONDARY_CTRL   = 1,
#if 0    
    VGA_CTRL   = 2
#endif
}
disp_control_t;

typedef struct _logicalMode_t
{
    uint32_t x;            /* X resolution */
    uint32_t y;            /* Y resolution */
    uint32_t bpp;          /* Bits per pixel */
    uint32_t hz;           /* Refresh rate */

    uint32_t baseAddress;  /* Offset from beginning of frame buffer.
                                   It is used to control the starting location of a mode.
                                   Calling function must initialize this field.
                                 */

    uint32_t pitch;        /* Mode pitch in byte.
                                   If initialized to 0, setMode function will set
                                   up this field.
                                   If not zero, setMode function will use this value.
                                 */

    disp_control_t dispCtrl;    /* SECONDARY or PRIMARY display control channel */
    
    /* These two parameters are used in the setModeEx. */
    uint32_t xLCD;         /* Panel width */
    uint32_t yLCD;         /* Panel height */
    
    void *userData;             /* Not used now, set it to 0 (for future used only) */
}
logicalMode_t;

/* 
 * Return a point to the gModeParamTable.
 * Function in other files used this to get the mode table pointer.
 */
mode_parameter_t *getStockModeParamTable(void);

/*
 * Return the size of the Stock Mode Param Table
 */
uint32_t getStockModeParamTableSize();

/* 
 * This function returns the current mode.
 */
mode_parameter_t getCurrentModeParam(
    disp_control_t dispCtrl
);

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
int32_t setCustomMode(
    logicalMode_t *pLogicalMode, 
    mode_parameter_t *pUserModeParam
);

/*
 * Input:
 *     1) pLogicalMode contains information such as x, y resolution, bpp, xLCD, and yLCD.
 *     2) A user defined parameter table for the mode.
 *
 * Similar like setCustomMode, this function calculate and programs the hardware 
 * to set up the requested mode and also scale the mode if necessary.
 *
 * This function allows the use of user defined parameter table if
 * predefined Vesa parameter table (gModeParamTable) does not fit.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
int32_t setCustomModeEx(
    logicalMode_t *pLogicalMode, 
    mode_parameter_t *pUserModeParam
);

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
int32_t setMode(
    logicalMode_t *pLogicalMode
);

/*
 * Input pLogicalMode contains information such as x, y resolution, bpp, 
 * xLCD and yLCD. The main difference between setMode and setModeEx are
 * the xLCD and yLCD parameters. Use this setModeEx API to set the mode
 * and enable expansion. setMode API does not support expansion.
 *
 * This function calculate and programs the hardware to set up the
 * requested mode.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
_X_EXPORT int32_t setModeEx(
    logicalMode_t *pLogicalMode
);

mode_parameter_t *findVesaModeParam(
    uint32_t width, 
    uint32_t height, 
    uint32_t refresh_rate
);

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
);

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
);

#endif /* _MODE_H_ */

