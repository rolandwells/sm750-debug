/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  tstpic.c --- Voyager GX SDK 
*  This file contains some picture drawing functions for testing
*  purpose.
* 
*******************************************************************/
#include "mode.h"
#include "sw2d.h"
#include "tstpic.h"

/*
 * This function clears the screen with black color
 */
void picClearScreen(logicalMode_t *pLogicalMode)
{
    swRectFill(
        pLogicalMode->baseAddress, 
        pLogicalMode->pitch, 
        pLogicalMode->bpp, 
        0, 0, 
        pLogicalMode->x, pLogicalMode->y, 
        0, ROP2_COPY);
}

/*
 * This function draws 8 vertical color bars and a while border.
 * Everything is render with SW only.
 */
void picVerticalBars(logicalMode_t *pLogicalMode)
{
    uint32_t color[8];
    uint32_t ulWidth, i, ulTemp;

    /* Fill the color palette */
    if (pLogicalMode->bpp==32)
    {
        color[0] = BPP32_RED;
        color[1] = BPP32_GREEN;
        color[2] = BPP32_BLUE;
        color[3] = BPP32_WHITE;
        color[4] = BPP32_GRAY;
        color[5] = BPP32_YELLOW;
        color[6] = BPP32_CYAN;
        color[7] = BPP32_PINK;
    }
    else if (pLogicalMode->bpp==16)
    {
        color[0] = BPP16_RED;
        color[1] = BPP16_GREEN;
        color[2] = BPP16_BLUE;
        color[3] = BPP16_WHITE;
        color[4] = BPP16_GRAY;
        color[5] = BPP16_YELLOW;
        color[6] = BPP16_CYAN;
        color[7] = BPP16_PINK;
    }
    else
    {
        color[0] = BPP8_RED;
        color[1] = BPP8_GREEN;
        color[2] = BPP8_BLUE;
        color[3] = BPP8_WHITE;
        color[4] = BPP8_GRAY;
        color[5] = BPP8_YELLOW;
        color[6] = BPP8_CYAN;
        color[7] = BPP8_PINK;
    }

    /* Draw 8 vertical bars */
    ulWidth = pLogicalMode->x / 8;
    for (i=0; i<7; i++)
    {
        swRectFill(
            pLogicalMode->baseAddress, 
            pLogicalMode->pitch, 
            pLogicalMode->bpp, 
            (ulWidth*i), 0, 
            ulWidth, pLogicalMode->y, 
            color[i], ROP2_COPY);
    }

    /* The last bar need special treatment because not all width are 
       divisible by 8 */
    ulTemp = ulWidth*i;
    ulWidth = pLogicalMode->x - ulTemp;
    swRectFill(
        pLogicalMode->baseAddress, 
        pLogicalMode->pitch, 
        pLogicalMode->bpp, 
        ulTemp, 0, 
        ulWidth, pLogicalMode->y, 
        color[i], ROP2_COPY);

    /* Draw a white border */
    swRect(
        pLogicalMode->baseAddress, 
        pLogicalMode->pitch, 
        pLogicalMode->bpp, 
        0, 0, 
        pLogicalMode->x, pLogicalMode->y, 
        color[3], ROP2_COPY);
}


/*
 * This function draws 8 horizontal color bars and a while border.
 * Everything is render with SW only.
 */
void picHorizontalBars(logicalMode_t *pLogicalMode)
{
    uint32_t color[8];
    uint32_t ulHeight, i, ulTemp;

    /* Fill the color palette */
    if (pLogicalMode->bpp==32)
    {
        color[0] = BPP32_RED;
        color[1] = BPP32_GREEN;
        color[2] = BPP32_BLUE;
        color[3] = BPP32_WHITE;
        color[4] = BPP32_GRAY;
        color[5] = BPP32_YELLOW;
        color[6] = BPP32_CYAN;
        color[7] = BPP32_PINK;
    }
    else if (pLogicalMode->bpp==16)
    {
        color[0] = BPP16_RED;
        color[1] = BPP16_GREEN;
        color[2] = BPP16_BLUE;
        color[3] = BPP16_WHITE;
        color[4] = BPP16_GRAY;
        color[5] = BPP16_YELLOW;
        color[6] = BPP16_CYAN;
        color[7] = BPP16_PINK;
    }
    else
    {
        color[0] = BPP8_RED;
        color[1] = BPP8_GREEN;
        color[2] = BPP8_BLUE;
        color[3] = BPP8_WHITE;
        color[4] = BPP8_GRAY;
        color[5] = BPP8_YELLOW;
        color[6] = BPP8_CYAN;
        color[7] = BPP8_PINK;
    }

    /* Draw 8 horizontal bars */
    ulHeight = pLogicalMode->y / 8;
    for (i=0; i<7; i++)
    {
        swRectFill(
            pLogicalMode->baseAddress, 
            pLogicalMode->pitch, 
            pLogicalMode->bpp, 
            0, (ulHeight*i), 
            pLogicalMode->x, ulHeight,
            color[i], ROP2_COPY);
    }

    /* The last bar need special treatment because not all heights are 
       divisible by 8 */
    ulTemp = ulHeight*i;
    ulHeight = pLogicalMode->y - ulTemp;
    swRectFill(
        pLogicalMode->baseAddress, 
        pLogicalMode->pitch, 
        pLogicalMode->bpp, 
        0, ulTemp, 
        pLogicalMode->x, ulHeight,
        color[i], ROP2_COPY);

#if 0
    /* Test CSC */
    swVerticalLine(pLogicalMode->baseAddress,
                   pLogicalMode->pitch,
                   pLogicalMode->bpp,
                   0,
                   0,
                   pLogicalMode->y,
                   color[3],
                   ROP2_COPY);
#else
    /* Draw a white border */
    swRect(
        pLogicalMode->baseAddress, 
        pLogicalMode->pitch, 
        pLogicalMode->bpp, 
        0, 0, 
        pLogicalMode->x, pLogicalMode->y, 
        color[3], ROP2_COPY);
#endif
}
