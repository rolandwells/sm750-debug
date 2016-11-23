/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  Demo.c --- Voyager GX SDK 
*  This file contains the source code for the demo code.
* 
*******************************************************************/

#include "defs.h"
#include "hardware.h"
#include "panning.h"

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
 *
 * Note:
 *      Set the mode first before calling this initPanning.
 */
unsigned char initPanning(
    uint32_t virtualWidth,
    uint32_t virtualHeight,
    uint32_t pitch
)
{
    uint32_t value;
    
    /* Set the pitch for the background bitmap.*/
    value = peekRegisterDWord(PRIMARY_FB_WIDTH);
    value = FIELD_VALUE(value, PRIMARY_FB_WIDTH, OFFSET, pitch);
    pokeRegisterDWord(PRIMARY_FB_WIDTH, value);

    /* Set window width and height.
     * Errata: For panning, the definition of the register is not consistent.
     *         The width and height needs to be substracted by 1.
     */
    pokeRegisterDWord(PRIMARY_WINDOW_WIDTH, FIELD_VALUE(0, PRIMARY_WINDOW_WIDTH, WIDTH, virtualWidth - 1));
    pokeRegisterDWord(PRIMARY_WINDOW_HEIGHT, FIELD_VALUE(0, PRIMARY_WINDOW_HEIGHT, HEIGHT, virtualHeight - 1));

    /* Disable the panning first */
    value = peekRegisterDWord(PRIMARY_DISPLAY_CTRL);
    value = FIELD_SET(value, PRIMARY_DISPLAY_CTRL, HORIZONTAL_PAN, DISABLE);
    value = FIELD_SET(value, PRIMARY_DISPLAY_CTRL, VERTICAL_PAN, DISABLE);
    pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, value);
    
    /* Set both vertical and horizontal value to 0 */
    pokeRegisterDWord(PRIMARY_PAN_CTRL, 0);
    
    return 0;
}

/*
 * This function checks whether the vertical or horizontal panning is enabled.
 *
 * Return:
 *      0x00    - Panning is disabled
 *      0x01    - Horizontal Panning is enabled
 *      0x02    - Vertical Panning is enabled
 */
unsigned char isPanningEnable()
{
    uint32_t value;
    unsigned char panningEnable = 0;
    
    value = peekRegisterDWord(PRIMARY_DISPLAY_CTRL);
    
    if (FIELD_GET(value, PRIMARY_DISPLAY_CTRL, VERTICAL_PAN) == 
                PRIMARY_DISPLAY_CTRL_VERTICAL_PAN_ENABLE)
        panningEnable |= 0x02;

    if (FIELD_GET(value, PRIMARY_DISPLAY_CTRL, HORIZONTAL_PAN) == 
                PRIMARY_DISPLAY_CTRL_HORIZONTAL_PAN_ENABLE)
        panningEnable |= 0x01;       
        
    return panningEnable;
}

/*
 * This function gets the horizontal panning value.
 *
 * Return:
 *      The number of pixels left to be panning. A value of 0 indicates that
 *      the panning is done. The maximum pixel value is 256 (0xff)
 */
unsigned char getHorizontalPanningValue()
{
    return ((unsigned char)(FIELD_GET(peekRegisterDWord(PRIMARY_PAN_CTRL), 
                                      PRIMARY_PAN_CTRL, 
                                      HORIZONTAL_PAN)));
}

/*
 * This function gets the horizontal panning direction
 *
 * Return:
 *      0   - Panning left
 *      1   - Panning right
 */
unsigned char getHorizontalPanningDirection()
{
    uint32_t value;
    
    value = peekRegisterDWord(PRIMARY_DISPLAY_CTRL);
    
    if (FIELD_GET(value, PRIMARY_DISPLAY_CTRL, HORIZONTAL_PAN_DIR) ==
            PRIMARY_DISPLAY_CTRL_HORIZONTAL_PAN_DIR_RIGHT)
        return 1;
        
    return 0;
}

/*
 * This function sets the horizontal panning value.
 *
 * Parameters:
 *      direction       - Direction of the horizontal panning:
 *                          0 - Panning to the left
 *                          1 - Panning to the right
 *      panningValue    - Number of pixel to be panned horizontally (0 to 256 max).
 *      hsyncWaitValue  - Number of horizontal sync pulses to wait for each
 *                        horizontal pan.
 */
void setHorizontalPanning(
    unsigned char direction,
    unsigned char panningValue,
    unsigned char hsyncWaitValue
)
{
    uint32_t value;
    
    /* Update the panning direction */
    value = peekRegisterDWord(PRIMARY_DISPLAY_CTRL);
    if (direction == 0)
        value = FIELD_SET(value, PRIMARY_DISPLAY_CTRL, HORIZONTAL_PAN_DIR, LEFT);
    else
        value = FIELD_SET(value, PRIMARY_DISPLAY_CTRL, HORIZONTAL_PAN_DIR, RIGHT);
    pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, value);
    
    /* Update the panning value and number of hsync wait value */
    value = peekRegisterDWord(PRIMARY_PAN_CTRL);
    value = FIELD_VALUE(value, PRIMARY_PAN_CTRL, HORIZONTAL_PAN, panningValue); 
    value = FIELD_VALUE(value, PRIMARY_PAN_CTRL, HORIZONTAL_VSYNC, hsyncWaitValue);
    pokeRegisterDWord(PRIMARY_PAN_CTRL, value);
}

/*
 * This function gets the vertical panning value.
 *
 * Return:
 *      The number of lines left to be panning. A value of 0 indicates that
 *      the panning is done. The maximum lines value is 256 (0xff)
 */
unsigned char getVerticalPanningValue()
{
    return ((unsigned char)(FIELD_GET(peekRegisterDWord(PRIMARY_PAN_CTRL), 
                                      PRIMARY_PAN_CTRL, 
                                      VERTICAL_PAN)));
}

/*
 * This function gets the horizontal panning direction
 *
 * Return:
 *      0   - Panning up
 *      1   - Panning down
 */
unsigned char getVerticalPanningDirection()
{
    uint32_t value;
    
    value = peekRegisterDWord(PRIMARY_DISPLAY_CTRL);
    
    if (FIELD_GET(value, PRIMARY_DISPLAY_CTRL, VERTICAL_PAN_DIR) ==
            PRIMARY_DISPLAY_CTRL_VERTICAL_PAN_DIR_DOWN)
        return 1;
        
    return 0;
}

/*
 * This function sets the vertical panning value.
 *
 * Parameters:
 *      direction       - Direction of the vertical panning:
 *                          0 - Panning up
 *                          1 - Panning down
 *      panningValue    - Number of lines to be panned vertically (0 to 256 max).
 *      vsyncWaitValue  - Number of vertical sync pulses to wait for each
 *                        vertical pan.
 */
void setVerticalPanning(
    unsigned char direction,
    unsigned char panningValue,
    unsigned char vsyncWaitValue
)
{
    uint32_t value;

    /* Update the panning direction */
    value = peekRegisterDWord(PRIMARY_DISPLAY_CTRL);
    if (direction == 0)
        value = FIELD_SET(value, PRIMARY_DISPLAY_CTRL, VERTICAL_PAN_DIR, UP);
    else
        value = FIELD_SET(value, PRIMARY_DISPLAY_CTRL, VERTICAL_PAN_DIR, DOWN);
    pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, value);
    
    /* Update the panning value and number of hsync wait value */
    value = peekRegisterDWord(PRIMARY_PAN_CTRL);
    value = FIELD_VALUE(value, PRIMARY_PAN_CTRL, VERTICAL_PAN, panningValue); 
    value = FIELD_VALUE(value, PRIMARY_PAN_CTRL, VERTICAL_VSYNC, vsyncWaitValue);
    pokeRegisterDWord(PRIMARY_PAN_CTRL, value);
}

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
)
{
    uint32_t value;
    
    value = peekRegisterDWord(PRIMARY_DISPLAY_CTRL);
    
    /* Enable/Disable the horizontal panning */
    if (horizontalEnable == 0)
        value = FIELD_SET(value, PRIMARY_DISPLAY_CTRL, HORIZONTAL_PAN, DISABLE);
    else
        value = FIELD_SET(value, PRIMARY_DISPLAY_CTRL, HORIZONTAL_PAN, ENABLE);
    
    /* Enable/Disable the vertical panning */    
    if (verticalEnable == 0) 
        value = FIELD_SET(value, PRIMARY_DISPLAY_CTRL, VERTICAL_PAN, DISABLE);
    else
        value = FIELD_SET(value, PRIMARY_DISPLAY_CTRL, VERTICAL_PAN, ENABLE);

    pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, value);
}

