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

    /* Draw a white border */
    swRect(
        pLogicalMode->baseAddress, 
        pLogicalMode->pitch, 
        pLogicalMode->bpp, 
        0, 0, 
        pLogicalMode->x, pLogicalMode->y, 
        color[3], ROP2_COPY);
}

#define BARS 7
/*
 * This function draws horizontal bars with varying intensity from low to high.
 * Everything is render with SW only.
 */
void picHorizontalBarsVaryIntensity(logicalMode_t *pLogicalMode)
{
    uint32_t x, y, ulVertStep, ulHoriStep, ulIndex;
    uint32_t ulR, ulG, ulB, ulColor, ulIntensityLevel, ulBorder;
    unsigned char ucRMask[BARS], ucGMask[BARS], ucBMask[BARS], rShift, gShift;

    /* Varying intensity don't make sense in 8BPP modes */
    if (pLogicalMode->bpp==8)
    {
        picHorizontalBars(pLogicalMode);
        return;
    }

    if (pLogicalMode->bpp==16)
    {
        /* Initialize RGB Mask */
        ucRMask[0]=0x1f; ucGMask[0]=0x00; ucBMask[0]=0x00;
        ucRMask[1]=0x00; ucGMask[1]=0x1f; ucBMask[1]=0x00;
        ucRMask[2]=0x00; ucGMask[2]=0x00; ucBMask[2]=0x1f;
        ucRMask[3]=0x1f; ucGMask[3]=0x1f; ucBMask[3]=0x1f;
        ucRMask[4]=0x1f; ucGMask[4]=0x1f; ucBMask[4]=0x00;
        ucRMask[5]=0x00; ucGMask[5]=0x1f; ucBMask[5]=0x1f;
        ucRMask[6]=0x1f; ucGMask[6]=0x00; ucBMask[6]=0x1f;

        ulIntensityLevel = 32;
        rShift = 11;
        gShift = 6;
    }
    else /* 32BPP */
    {
        /* Initialize RGB Mask */
        ucRMask[0]=0xff; ucGMask[0]=0x00; ucBMask[0]=0x00;
        ucRMask[1]=0x00; ucGMask[1]=0xff; ucBMask[1]=0x00;
        ucRMask[2]=0x00; ucGMask[2]=0x00; ucBMask[2]=0xff;
        ucRMask[3]=0xff; ucGMask[3]=0xff; ucBMask[3]=0xff;
        ucRMask[4]=0xff; ucGMask[4]=0xff; ucBMask[4]=0x00;
        ucRMask[5]=0x00; ucGMask[5]=0xff; ucBMask[5]=0xff;
        ucRMask[6]=0xff; ucGMask[6]=0x00; ucBMask[6]=0xff;

        ulIntensityLevel = 256;
        rShift = 16;
        gShift = 8;
    }

    /* Draw 7 horizontal bars with intensity from low to high */

    ulVertStep = (pLogicalMode->y / BARS) + 1;
    ulIndex = 0;

    for (y=0; y<pLogicalMode->y; y+=ulVertStep)
    {
        /* Adjust last vertical step, in order to avoid drawing off the 
           screen limit.
        */
        if ((y+ulVertStep) >= pLogicalMode->y)
        {
            ulVertStep = pLogicalMode->y - y;
        }

        ulR = 0;
        ulG = 0;
        ulB = 0;
        ulHoriStep = (pLogicalMode->x / ulIntensityLevel) + 1;

        for (x=0; x<pLogicalMode->x; x+=ulHoriStep)
        {
    
            /* Adjust last ulHoriStep:
                1. Last rectange may not have a width equal to ulHoriStep.
                2. The last block is always highest intensity.
            */
            if ((x + ulHoriStep) >= pLogicalMode->x)
            {
                ulHoriStep = pLogicalMode->x - x;
                ulR = 0xff & ucRMask[ulIndex];
                ulG = 0xff & ucGMask[ulIndex];
                ulB = 0xff & ucBMask[ulIndex];
            }
    
            ulColor = (ulR<<rShift) | (ulG<<gShift) | ulB;
    
            swRectFill(
                pLogicalMode->baseAddress, 
                pLogicalMode->pitch, 
                pLogicalMode->bpp, 
                x, y,
                ulHoriStep, ulVertStep,
                ulColor, ROP2_COPY);
    
            ulR = (ulR+1) & ucRMask[ulIndex];
            ulG = (ulG+1) & ucGMask[ulIndex];
            ulB = (ulB+1) & ucBMask[ulIndex];
        }

        if (ulIndex < (BARS-1)) /* Protect mask array overflow */
            ulIndex++;
    }

    /* Draw a white border */
    if (pLogicalMode->bpp==16)
        ulBorder = BPP16_WHITE;
    else
        ulBorder = BPP32_WHITE;

    swRect(
        pLogicalMode->baseAddress, 
        pLogicalMode->pitch, 
        pLogicalMode->bpp, 
        0, 0, 
        pLogicalMode->x, pLogicalMode->y, 
        ulBorder, ROP2_COPY);
}

/*
 * This function draws vertical bars with varying intensity.
 * Everything is render with SW only.
 */
void picVerticalBarsVaryIntensity(logicalMode_t *pLogicalMode)
{
    uint32_t x, y, ulVertStep, ulHoriStep, ulIndex;
    uint32_t ulR, ulG, ulB, ulColor, ulIntensityLevel, ulBorder;
    unsigned char ucRMask[BARS], ucGMask[BARS], ucBMask[BARS], rShift, gShift;

    /* Varying intensity don't make sense in 8BPP modes */
    if (pLogicalMode->bpp==8)
    {
        picHorizontalBars(pLogicalMode);
        return;
    }

    if (pLogicalMode->bpp==16)
    {
        /* Initialize RGB Mask */
        ucRMask[0]=0x1f; ucGMask[0]=0x00; ucBMask[0]=0x00;
        ucRMask[1]=0x00; ucGMask[1]=0x1f; ucBMask[1]=0x00;
        ucRMask[2]=0x00; ucGMask[2]=0x00; ucBMask[2]=0x1f;
        ucRMask[3]=0x1f; ucGMask[3]=0x1f; ucBMask[3]=0x1f;
        ucRMask[4]=0x1f; ucGMask[4]=0x1f; ucBMask[4]=0x00;
        ucRMask[5]=0x00; ucGMask[5]=0x1f; ucBMask[5]=0x1f;
        ucRMask[6]=0x1f; ucGMask[6]=0x00; ucBMask[6]=0x1f;

        ulIntensityLevel = 32;
        rShift = 11;
        gShift = 6;
    }
    else /* 32BPP */
    {
        /* Initialize RGB Mask */
        ucRMask[0]=0xff; ucGMask[0]=0x00; ucBMask[0]=0x00;
        ucRMask[1]=0x00; ucGMask[1]=0xff; ucBMask[1]=0x00;
        ucRMask[2]=0x00; ucGMask[2]=0x00; ucBMask[2]=0xff;
        ucRMask[3]=0xff; ucGMask[3]=0xff; ucBMask[3]=0xff;
        ucRMask[4]=0xff; ucGMask[4]=0xff; ucBMask[4]=0x00;
        ucRMask[5]=0x00; ucGMask[5]=0xff; ucBMask[5]=0xff;
        ucRMask[6]=0xff; ucGMask[6]=0x00; ucBMask[6]=0xff;

        ulIntensityLevel = 256;
        rShift = 16;
        gShift = 8;
    }

    /* Draw 7 vertical bars with varying intensity from low to high */

    ulHoriStep = (pLogicalMode->x / BARS) + 1;
    ulIndex = 0;

    for (x=0; x<pLogicalMode->x; x+=ulHoriStep)
    {
        /* Adjust last horizontal step, in order to avoid drawing off the 
           screen limit.
        */
        if ((x+ulHoriStep) >= pLogicalMode->x)
        {
            ulHoriStep = pLogicalMode->x - x;
        }

        ulR = 0;
        ulG = 0;
        ulB = 0;
        ulVertStep = (pLogicalMode->y / ulIntensityLevel) + 1;

        for (y=0; y<pLogicalMode->y; y+=ulVertStep)
        {
    
            /* Adjust last ulHoriStep:
                1. Last rectange may not have a width equal to ulHoriStep.
                2. The last block is always highest intensity.
            */
            if ((y + ulVertStep) >= pLogicalMode->y)
            {
                ulVertStep = pLogicalMode->y - y;
                ulR = 0xff & ucRMask[ulIndex];
                ulG = 0xff & ucGMask[ulIndex];
                ulB = 0xff & ucBMask[ulIndex];
            }
    
            ulColor = (ulR<<rShift) | (ulG<<gShift) | ulB;
    
            swRectFill(
                pLogicalMode->baseAddress, 
                pLogicalMode->pitch, 
                pLogicalMode->bpp, 
                x, y,
                ulHoriStep, ulVertStep,
                ulColor, ROP2_COPY);
    
            ulR = (ulR+1) & ucRMask[ulIndex];
            ulG = (ulG+1) & ucGMask[ulIndex];
            ulB = (ulB+1) & ucBMask[ulIndex];
        }

        if (ulIndex < (BARS-1)) /* Protect mask array overflow */
            ulIndex++;
    }

    /* Draw a white border */
    if (pLogicalMode->bpp==16)
        ulBorder = BPP16_WHITE;
    else
        ulBorder = BPP32_WHITE;

    swRect(
        pLogicalMode->baseAddress, 
        pLogicalMode->pitch, 
        pLogicalMode->bpp, 
        0, 0, 
        pLogicalMode->x, pLogicalMode->y, 
        ulBorder, ROP2_COPY);
}

/*
 * This function draws horizontal bars with color scaling
 * Everything is render with SW only.
 */
void picHorizontalColorGrid(logicalMode_t *pLogicalMode)
{
    uint32_t x, y, ulVertStep, ulHoriStep, ulIndex;
    uint32_t ulR, ulG, ulB, ulColor, ulIntensityLevel, ulBorder;
    unsigned char ucRMask[BARS], ucGMask[BARS], ucBMask[BARS], rShift, gShift;

    /* Varying intensity don't make sense in 8BPP modes */
    if (pLogicalMode->bpp==8)
    {
        picHorizontalBars(pLogicalMode);
        return;
    }

    if (pLogicalMode->bpp==16)
    {
        /* Initialize RGB Mask */
        ucRMask[0]=0x1f; ucGMask[0]=0x00; ucBMask[0]=0x00;
        ucRMask[1]=0x00; ucGMask[1]=0x1f; ucBMask[1]=0x00;
        ucRMask[2]=0x00; ucGMask[2]=0x00; ucBMask[2]=0x1f;
        ucRMask[3]=0x1f; ucGMask[3]=0x1f; ucBMask[3]=0x1f;
        ucRMask[4]=0x1f; ucGMask[4]=0x1f; ucBMask[4]=0x00;
        ucRMask[5]=0x00; ucGMask[5]=0x1f; ucBMask[5]=0x1f;
        ucRMask[6]=0x1f; ucGMask[6]=0x00; ucBMask[6]=0x1f;

        ulIntensityLevel = 32;
        rShift = 11;
        gShift = 6;
    }
    else /* 32BPP */
    {
        /* Initialize RGB Mask */
        ucRMask[0]=0xff; ucGMask[0]=0x00; ucBMask[0]=0x00;
        ucRMask[1]=0x00; ucGMask[1]=0xff; ucBMask[1]=0x00;
        ucRMask[2]=0x00; ucGMask[2]=0x00; ucBMask[2]=0xff;
        ucRMask[3]=0xff; ucGMask[3]=0xff; ucBMask[3]=0xff;
        ucRMask[4]=0xff; ucGMask[4]=0xff; ucBMask[4]=0x00;
        ucRMask[5]=0x00; ucGMask[5]=0xff; ucBMask[5]=0xff;
        ucRMask[6]=0xff; ucGMask[6]=0x00; ucBMask[6]=0xff;

        ulIntensityLevel = 256;
        rShift = 16;
        gShift = 8;
    }

    /* Draw 7 horizontal bars with intensity from low to high */

    ulVertStep = (pLogicalMode->y / BARS) + 1;
    ulIndex = 0;

    for (y=0; y<pLogicalMode->y; y+=ulVertStep)
    {
        /* Adjust last vertical step, in order to avoid drawing off the 
           screen limit.
        */
        if ((y+ulVertStep) >= pLogicalMode->y)
        {
            ulVertStep = pLogicalMode->y - y;
        }

        ulR = 0;
        ulG = 0;
        ulB = 0;
        ulHoriStep = 1;

        for (x=0; x<pLogicalMode->x; x+=ulHoriStep)
        {
            ulColor = (ulR<<rShift) | (ulG<<gShift) | ulB;
    
            swRectFill(
                pLogicalMode->baseAddress, 
                pLogicalMode->pitch, 
                pLogicalMode->bpp, 
                x, y,
                ulHoriStep, ulVertStep,
                ulColor, ROP2_COPY);
    
            ulR = (ulR+1) & ucRMask[ulIndex];
            ulG = (ulG+1) & ucGMask[ulIndex];
            ulB = (ulB+1) & ucBMask[ulIndex];
        }

        if (ulIndex < (BARS-1)) /* Protect mask array overflow */
            ulIndex++;
    }

    /* Draw a white border */
    if (pLogicalMode->bpp==16)
        ulBorder = BPP16_WHITE;
    else
        ulBorder = BPP32_WHITE;

    swRect(
        pLogicalMode->baseAddress, 
        pLogicalMode->pitch, 
        pLogicalMode->bpp, 
        0, 0, 
        pLogicalMode->x, pLogicalMode->y, 
        ulBorder, ROP2_COPY);
}

/*
 * This function draws vertical bars with color scaling.
 * Everything is render with SW only.
 */
void picVerticalColorGrid(logicalMode_t *pLogicalMode)
{
    uint32_t x, y, ulVertStep, ulHoriStep, ulIndex;
    uint32_t ulR, ulG, ulB, ulColor, ulIntensityLevel, ulBorder;
    unsigned char ucRMask[BARS], ucGMask[BARS], ucBMask[BARS], rShift, gShift;

    /* Varying intensity don't make sense in 8BPP modes */
    if (pLogicalMode->bpp==8)
    {
        picHorizontalBars(pLogicalMode);
        return;
    }

    if (pLogicalMode->bpp==16)
    {
        /* Initialize RGB Mask */
        ucRMask[0]=0x1f; ucGMask[0]=0x00; ucBMask[0]=0x00;
        ucRMask[1]=0x00; ucGMask[1]=0x1f; ucBMask[1]=0x00;
        ucRMask[2]=0x00; ucGMask[2]=0x00; ucBMask[2]=0x1f;
        ucRMask[3]=0x1f; ucGMask[3]=0x1f; ucBMask[3]=0x1f;
        ucRMask[4]=0x1f; ucGMask[4]=0x1f; ucBMask[4]=0x00;
        ucRMask[5]=0x00; ucGMask[5]=0x1f; ucBMask[5]=0x1f;
        ucRMask[6]=0x1f; ucGMask[6]=0x00; ucBMask[6]=0x1f;

        ulIntensityLevel = 32;
        rShift = 11;
        gShift = 6;
    }
    else /* 32BPP */
    {
        /* Initialize RGB Mask */
        ucRMask[0]=0xff; ucGMask[0]=0x00; ucBMask[0]=0x00;
        ucRMask[1]=0x00; ucGMask[1]=0xff; ucBMask[1]=0x00;
        ucRMask[2]=0x00; ucGMask[2]=0x00; ucBMask[2]=0xff;
        ucRMask[3]=0xff; ucGMask[3]=0xff; ucBMask[3]=0xff;
        ucRMask[4]=0xff; ucGMask[4]=0xff; ucBMask[4]=0x00;
        ucRMask[5]=0x00; ucGMask[5]=0xff; ucBMask[5]=0xff;
        ucRMask[6]=0xff; ucGMask[6]=0x00; ucBMask[6]=0xff;

        ulIntensityLevel = 256;
        rShift = 16;
        gShift = 8;
    }

    /* Draw 7 vertical bars with color scaling */

    ulHoriStep = (pLogicalMode->x / BARS) + 1;
    ulIndex = 0;

    for (x=0; x<pLogicalMode->x; x+=ulHoriStep)
    {
        /* Adjust last horizontal step, in order to avoid drawing off the
           screen limit.
        */
        if ((x+ulHoriStep) >= pLogicalMode->x)
        {
            ulHoriStep = pLogicalMode->x - x;
        }

        ulR = 0;
        ulG = 0;
        ulB = 0;
        ulVertStep = 1;

        for (y=0; y<pLogicalMode->y; y+=ulVertStep)
        {
            ulColor = (ulR<<rShift) | (ulG<<gShift) | ulB;
    
            swRectFill(
                pLogicalMode->baseAddress, 
                pLogicalMode->pitch, 
                pLogicalMode->bpp, 
                x, y,
                ulHoriStep, ulVertStep,
                ulColor, ROP2_COPY);
    
            ulR = (ulR+1) & ucRMask[ulIndex];
            ulG = (ulG+1) & ucGMask[ulIndex];
            ulB = (ulB+1) & ucBMask[ulIndex];
        }

        if (ulIndex < (BARS-1)) /* Protect mask array overflow */
            ulIndex++;
    }

    /* Draw a white border */
    if (pLogicalMode->bpp==16)
        ulBorder = BPP16_WHITE;
    else
        ulBorder = BPP32_WHITE;

    swRect(
        pLogicalMode->baseAddress, 
        pLogicalMode->pitch, 
        pLogicalMode->bpp, 
        0, 0, 
        pLogicalMode->x, pLogicalMode->y, 
        ulBorder, ROP2_COPY);
}
