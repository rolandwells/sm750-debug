/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  PHWC.H --- Voyager GX SDK 
*  This file contains the definitions for the Primary cursor functions.
* 
*******************************************************************/
#ifndef _CURSOR_H_
#define _CURSOR_H_

#include "mode.h"

/*
 * This function initializes the cursor attributes.
 */
void initCursor(
    disp_control_t dispControl,     /* Display control (PRIMARY_CTRL or SECONDARY_CTRL) */
    uint32_t base,             /* Base Address */ 
    uint32_t color1,           /* Cursor color 1 in RGB 5:6:5 format */
    uint32_t color2,           /* Cursor color 2 in RGB 5:6:5 format */
    uint32_t color3            /* Cursor color 3 in RGB 5:6:5 format */
);

/*
 * This function sets the cursor position.
 */
void setCursorPosition(
    disp_control_t dispControl,     /* Display control (PRIMARY_CTRL or SECONDARY_CTRL) */
    uint32_t dx,               /* X Coordinate of the cursor */
    uint32_t dy,               /* Y Coordinate of the cursor */
    unsigned char topOutside,       /* Top Boundary Select: either partially outside (= 1) 
                                       or within the screen top boundary (= 0) */
    unsigned char leftOutside       /* Left Boundary Select: either partially outside (= 1) 
                                       or within the screen left boundary (= 0) */
);

/*
 * This function enables/disables the cursor.
 */
void enableCursor(
    disp_control_t dispControl,     /* Display control (PRIMARY_CTRL or SECONDARY_CTRL) */
    uint32_t enable
);

#endif /* _CURSOR_H_ */

