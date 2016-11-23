/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  panning.h --- SMI DDK 
*  This file contains the definitions for the mode tables.
* 
*******************************************************************/
#ifndef _PANNING_H_
#define _PANNING_H_

/*
 * This function initializes the panning mode.
 *
 * Parameters:
 *      virtualWidth    - The new width of the panning window
 *      virtualHeight   - The new height of the panning window.
 *      pitch           - The new pitch value for the panning mode.
 *
 * Return:
 *      0x00    - Initialization is successful
 *      0x01    - Initialization is failed
 */
unsigned char initPanning(
    uint32_t virtualWidth,
    uint32_t virtualHeight,
    uint32_t pitch
);

/*
 * This function checks whether the vertical or horizontal panning is enabled.
 *
 * Return:
 *      0x00    - Panning is disabled
 *      0x01    - Horizontal Panning is enabled
 *      0x02    - Vertical Panning is enabled
 */
unsigned char isPanningEnable();

/*
 * This function gets the horizontal panning value.
 *
 * Return:
 *      The number of pixels left to be panning. A value of 0 indicates that
 *      the panning is done. The maximum pixel value is 256 (0xff)
 */
unsigned char getHorizontalPanningValue();

/*
 * This function gets the horizontal panning direction
 *
 * Return:
 *      0   - Panning left
 *      1   - Panning right
 */
unsigned char getHorizontalPanningDirection();

/*
 * This function sets the horizontal panning value.
 *
 * Parameters:
 *      direction       - Direction of the horizontal panning:
 *                          0 - Panning left
 *                          1 - Panning right
 *      horizontalValue - Number of pixel to be panned horizontally (0 to 256 max).
 *      hsyncWaitValue  - Number of horizontal sync pulses to wait for each
 *                        horizontal pan.
 */
void setHorizontalPanning(
    unsigned char direction,
    unsigned char horizontalValue,
    unsigned char hsyncWaitValue
);

/*
 * This function gets the vertical panning value.
 *
 * Return:
 *      The number of lines left to be panning. A value of 0 indicates that
 *      the panning is done. The maximum lines value is 256 (0xff)
 */
unsigned char getVerticalPanningValue();

/*
 * This function gets the horizontal panning direction
 *
 * Return:
 *      0   - Panning up
 *      1   - Panning down
 */
unsigned char getVerticalPanningDirection();

/*
 * This function sets the vertical panning value.
 *
 * Parameters:
 *      direction       - Direction of the vertical panning:
 *                          0 - Panning up
 *                          1 - Panning down
 *      verticalValue   - Number of lines to be panned vertically (0 to 256 max).
 *      vsyncWaitValue  - Number of vertical sync pulses to wait for each
 *                        vertical pan.
 */
void setVerticalPanning(
    unsigned char direction,
    unsigned char verticalValue,
    unsigned char vsyncWaitValue
);
    
/*
 * This function start the panning feature.
 *
 * Parameters:
 *      horizontalEnable    - Enable the horizontal panning
 *      verticalEnable      - Enable the vertical panning 
 */
void startPanning(
    unsigned char horizontalEnable,
    unsigned char verticalEnable
);

#endif  /* _PANNING_H_*/
