/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  VALPHA.C --- Voyager GX SDK 
*  This file contains the definitions for the Video Alpha functions.
* 
*******************************************************************/
#include "defs.h"
#include "hardware.h"
#include "valpha.h"

#define SCALE_CONSTANT                      (1 << 12)

/* 
 * This function sets the video alpha window size.
 *   
 *  Input:
 *      width       - Video Alpha Window width
 *      height      - Video Alpha Window height
 *             
 */
void videoAlphaSetWindowSize(
    uint32_t width,
    uint32_t height
)
{
    uint32_t value, startX, startY;

    /* Get the video window width and height */
    value = peekRegisterDWord(VIDEO_ALPHA_PLANE_TL);
    startX = FIELD_GET(value, VIDEO_ALPHA_PLANE_TL, LEFT);
    startY = FIELD_GET(value, VIDEO_ALPHA_PLANE_TL, TOP);

    /* Set bottom and right position */
    pokeRegisterDWord(VIDEO_ALPHA_PLANE_BR, 
        FIELD_VALUE(0, VIDEO_ALPHA_PLANE_BR, BOTTOM, startY + height - 1) |
        FIELD_VALUE(0, VIDEO_ALPHA_PLANE_BR, RIGHT, startX + width - 1)); 
}

/*
 * This function gets the video alpha window size.
 *  
 *  Output:
 *      pAlphaWidth     - Pointer to a buffer to save the video alpha window width
 *      pAlphaHeight    - Pointer to a buffer to save the video alpha window height
 *             
 */
void videoAlphaGetWindowSize(
    uint32_t *pWidth,
    uint32_t *pHeight
)
{
    uint32_t positionTopLeft, positionRightBottom;
    uint32_t videoAlphaWidth, videoAlphaHeight;
    
    /* Get the video window width and height */
    positionTopLeft = peekRegisterDWord(VIDEO_ALPHA_PLANE_TL);
    positionRightBottom = peekRegisterDWord(VIDEO_ALPHA_PLANE_BR);
    
    videoAlphaWidth  = FIELD_GET(positionRightBottom, VIDEO_ALPHA_PLANE_BR, RIGHT) - 
                       FIELD_GET(positionTopLeft, VIDEO_ALPHA_PLANE_TL, LEFT) + 1;
    videoAlphaHeight = FIELD_GET(positionRightBottom, VIDEO_ALPHA_PLANE_BR, BOTTOM) - 
                       FIELD_GET(positionTopLeft, VIDEO_ALPHA_PLANE_TL, TOP) + 1;
              
    if (pWidth != ((uint32_t *)0))
        *pWidth = videoAlphaWidth;
    
    if (pHeight != ((uint32_t *)0))
        *pHeight = videoAlphaHeight;
}

/*
 * This function sets the video alpha starting coordinate position.
 *  
 *  Input:
 *      x       - X Coordinate of the video alpha window starting position
 *      y       - Y Coordinate of the video alpha window starting position
 *             
 */
void videoAlphaSetPosition(
    uint32_t x,
    uint32_t y
)
{
    uint32_t videoAlphaWidth, videoAlphaHeight;
    
    /* Get the video window width and height */
    videoAlphaGetWindowSize(&videoAlphaWidth, &videoAlphaHeight);
    
    /* Set top and left position */
    pokeRegisterDWord(VIDEO_ALPHA_PLANE_TL,
        FIELD_VALUE(0, VIDEO_ALPHA_PLANE_TL, TOP, y) |
        FIELD_VALUE(0, VIDEO_ALPHA_PLANE_TL, LEFT, x));
    
    /* Set bottom and right position */    
    videoAlphaSetWindowSize(videoAlphaWidth, videoAlphaHeight);
}

/*
 * This function gets the video alpha starting coordinate position.
 *  
 *  Input:
 *      pX      - Pointer to a buffer to save the x Coordinate of the current 
 *                video alpha coordinate
 *      pY      - Pointer to a buffer to save the y Coordinate of the current 
 *                video alpha coordinate
 *             
 */
void videoAlphaGetPosition(
    uint32_t *pX,
    uint32_t *pY
)
{
    uint32_t positionTopLeft;
    
    /* Get the starting coordinate of the alpha plane */
    positionTopLeft = peekRegisterDWord(VIDEO_ALPHA_PLANE_TL);
    
    if (pX)
        *pX = (uint32_t) FIELD_GET(positionTopLeft, VIDEO_ALPHA_PLANE_TL, LEFT);
    
    if (pY)
        *pY = (uint32_t) FIELD_GET(positionTopLeft, VIDEO_ALPHA_PLANE_TL, TOP);
}

/*
 * This function enables/disables video alpha chroma key feature.
 *  
 *  Input:
 *      enableChromaKey - Flag to enable/disable the chroma key.
 *      chromaKeyMask   - Chroma key mask
 *      chromaKeyValue  - Chroma key value       
 */
void videoAlphaSetChroma(
    uint32_t enableChromaKey,  /* Flag to enable/disable the chroma key
                                       on the Alpha display plane */
    uint32_t chromaKeyMask,    /* Chroma Key Mask */
    uint32_t chromaKeyValue    /* Chroma Key Value */
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL);
    if (enableChromaKey)
    {
        value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, CHROMA_KEY, ENABLE);
        
        /* Set the chroma key mask and value */
        pokeRegisterDWord(VIDEO_ALPHA_CHROMA_KEY,
            FIELD_VALUE(0, VIDEO_ALPHA_CHROMA_KEY, MASK, chromaKeyMask) |
            FIELD_VALUE(0, VIDEO_ALPHA_CHROMA_KEY, VALUE, chromaKeyValue));
    }
    else
        value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, CHROMA_KEY, DISABLE);
        
    pokeRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL, value);
}

/*
 * This function sets the video alpha FIFO Request Level.
 *  
 *  Input:
 *      fifo    - Video Alpha FIFO Request Level
 *                     
 */
void videoAlphaSetFIFOLevel(
    video_alpha_fifo_t fifo
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL);
    switch(fifo)
    {
        default:
        case VIDEO_ALPHA_FIFO_LEVEL_1:
            value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, FIFO, 1);
            break; 
        case VIDEO_ALPHA_FIFO_LEVEL_3:
            value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, FIFO, 3);
            break;
        case VIDEO_ALPHA_FIFO_LEVEL_7:
            value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, FIFO, 7);
            break;
        case VIDEO_ALPHA_FIFO_LEVEL_11:
            value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, FIFO, 11);
            break;
    }
    
    pokeRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL, value);
}

/*
 * This function sets the RGB565 color lookup table for each 16 
 * 4-bit indexed colors.
 * 
 * Input:
 *      pColorLookupTable   - Pointer to a RGB565 colors of each 4-bit indexed 
 *                            colors. It has to have 16 members.
 */
void videoAlphaSetColorLookupTable(
    uint32_t *pColorLookupTable   /* Color Lookup Table */
)
{
    uint32_t index, value;
    
    for (index = 0; index < 16; index+=2)
    {
        value = FIELD_VALUE(0, VIDEO_ALPHA_COLOR_LOOKUP_01, 0, pColorLookupTable[index]) |
                FIELD_VALUE(0, VIDEO_ALPHA_COLOR_LOOKUP_01, 1, pColorLookupTable[index+1]);
        pokeRegisterDWord(VIDEO_ALPHA_COLOR_LOOKUP_01 + index, value);
    }
}

/*
 * This function select the video alpha method, either to use per-pixel
 * or by the given alpha value.
 * 
 * Input:
 *      useAlphaValue   - Use the given alpha value.
 *                          0 - Use per-pixel alpha value
 *                          1 - Use the given alpha value
 *      alphaValue      - Alpha value to be used in the video alpha plane
 */
void videoAlphaSelectMethod(
    uint32_t useAlphaValue,
    uint32_t alphaValue
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL);
    if (useAlphaValue)
        value = FIELD_SET(0, VIDEO_ALPHA_DISPLAY_CTRL, SELECT, ALPHA) |
                FIELD_VALUE(value, VIDEO_ALPHA_DISPLAY_CTRL, ALPHA, alphaValue);
    else
        value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, SELECT, PER_PIXEL); 
    
    pokeRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL, value);
}

/*
 * This function gets the current video alpha method and value (when applicable) used.
 *
 * Input:
 *      pAlphaValue - Pointer to save the alpha value used (if the return
 *                    value is 1.
 * 
 * Returns:
 *      0   - Per-pixel value method
 *      1   - Alpha value method
 */
uint32_t videoAlphaGetAlphaMethod(
    uint32_t *pAlphaValue
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL);
    
    if (pAlphaValue)
        *pAlphaValue = FIELD_GET(value, VIDEO_ALPHA_DISPLAY_CTRL, ALPHA);
    
    if (FIELD_GET(value, VIDEO_ALPHA_DISPLAY_CTRL, SELECT) == VIDEO_ALPHA_DISPLAY_CTRL_SELECT_ALPHA)
        return 1;

    return 0;
}

/*
 * This function sets the video Alpha buffer
 *
 * Input:
 *      bufferStartAddress  - Buffer Start Address
 *      bufferSize          - Buffer Size
 *      bufferPitch         - Bytes per line of the video buffer
 */
void videoAlphaSetBuffer(
    uint32_t startAddress,     /* Memory address of video Alpha buffer with 128-bit alignment */
    uint32_t bufferSize,       /* Size of the video buffer */
    uint32_t bufferPitch       /* Number of 128-bit aligned bytes per line of the video buffer */
)
{
    uint32_t value;

    /* Set Video Alpha Start Address */
    pokeRegisterDWord(VIDEO_ALPHA_FB_ADDRESS,
        FIELD_SET(0, VIDEO_ALPHA_FB_ADDRESS, STATUS, PENDING) |
        FIELD_SET(0, VIDEO_ALPHA_FB_ADDRESS, EXT, LOCAL) |
        FIELD_VALUE(0, VIDEO_ALPHA_FB_ADDRESS, ADDRESS, startAddress));
        
    /* Set Video Buffer 0 Last Address */
    pokeRegisterDWord(VIDEO_ALPHA_FB_LAST_ADDRESS,
            FIELD_SET(0, VIDEO_ALPHA_FB_LAST_ADDRESS, EXT, LOCAL) |
        FIELD_VALUE(0, VIDEO_ALPHA_FB_LAST_ADDRESS, ADDRESS, startAddress + bufferSize));
        
    /* Setup the pitch and width of the alpha plane */
    pokeRegisterDWord(VIDEO_ALPHA_FB_WIDTH,
        FIELD_VALUE(0, VIDEO_ALPHA_FB_WIDTH, WIDTH, bufferPitch) |
        FIELD_VALUE(0, VIDEO_ALPHA_FB_WIDTH, OFFSET, bufferPitch));        
}

/*
 * This function sets the video Alpha buffer initial vertical scale.
 *
 * Input:
 *      bufferInitScale     - Buffer Initial vertical scale
 */
void videoAlphaSetInitialScale(
    unsigned short verticalInitScale,
    unsigned short horizontalInitScale
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_ALPHA_INITIAL_SCALE);
    
    /* Set the Video Initial Vertical Scale */
    value = FIELD_VALUE(value, VIDEO_ALPHA_INITIAL_SCALE, VERTICAL, verticalInitScale);
    
    /* Set the Video Initial Horizontal Scale */
    value = FIELD_VALUE(value, VIDEO_ALPHA_INITIAL_SCALE, HORIZONTAL, horizontalInitScale);
        
    pokeRegisterDWord(VIDEO_ALPHA_INITIAL_SCALE, value);        
}

/*
 * This function scales the video Alpha.
 *
 * Input:
 *    srcWidth     - The source video width
 *    srcHeight    - The source video height
 *    dstWidth     - The destination video width      
 *    dstHeight    - The destination video height
 */
void videoAlphaScale(
    uint32_t srcWidth,
    uint32_t srcHeight,
    uint32_t dstWidth,
    uint32_t dstHeight
)
{
    uint32_t value = 0;
    uint32_t scaleFactor;
    
    /* Scale the vertical size */
    if (dstHeight >= srcHeight)
    {
        /* Expand size */
        value = FIELD_SET(value, VIDEO_ALPHA_SCALE, VERTICAL_MODE, EXPAND);
        
        /* Calculate the factor */
        scaleFactor = srcHeight * SCALE_CONSTANT / dstHeight;
        value = FIELD_VALUE(value, VIDEO_ALPHA_SCALE, VERTICAL_SCALE, scaleFactor);
    }
    else
    {        
        /* Shrink size */
        value = FIELD_SET(value, VIDEO_ALPHA_SCALE, VERTICAL_MODE, SHRINK);
        
        /* Calculate the factor */
        scaleFactor = dstHeight * SCALE_CONSTANT / srcHeight;
        value = FIELD_VALUE(value, VIDEO_ALPHA_SCALE, VERTICAL_SCALE, scaleFactor);
    }
    
    /* Scale the horizontal size */
    if (dstWidth >= srcWidth)
    {
        /* Expand size */
        value = FIELD_SET(value, VIDEO_ALPHA_SCALE, HORIZONTAL_MODE, EXPAND);
        
        /* Calculate the factor */
        scaleFactor = srcWidth * SCALE_CONSTANT / dstWidth;
        value = FIELD_VALUE(value, VIDEO_ALPHA_SCALE, HORIZONTAL_SCALE, scaleFactor);
    }
    else
    {
        /* Shrink size */
        value = FIELD_SET(value, VIDEO_ALPHA_SCALE, HORIZONTAL_MODE, SHRINK);
        
        /* Calculate the factor */
        scaleFactor = dstWidth * SCALE_CONSTANT / srcWidth;
        value = FIELD_VALUE(value, VIDEO_ALPHA_SCALE, HORIZONTAL_SCALE, scaleFactor);
    }
    
    pokeRegisterDWord(VIDEO_ALPHA_SCALE, value);
}

/*
 * This function forces half scale factor (do not need to set the video
 * scale factor).
 *
 * Input:
 *      enableHorzHalfShrink    - Flag to enable/disable 1/2 horizontal 
 *                                scale factor
 *      enableVertHalfShrink    - Flag to enable/disable 1/2 vertical scale 
 *                                factor
 */
void videoAlphaForceHalfShrink(
    uint32_t enableHorzHalfShrink,
    uint32_t enableVertHalfShrink
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL);
    if (enableHorzHalfShrink)
        value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, HORZ_SCALE, HALF);
    else
        value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, HORZ_SCALE, NORMAL);
        
    if (enableVertHalfShrink)
        value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, VERT_SCALE, HALF);
    else
        value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, VERT_SCALE, NORMAL);
    
    pokeRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL, value);
}

/*
 * This function enables/disables the horizontal and vertical interpolation.
 *
 * Input:
 *      enableHorzInterpolation   - Flag to enable/disable Horizontal interpolation
 *      enableVertInterpolation   - Flag to enable/disable Vertical interpolation
 */
void videoAlphaSetInterpolation(
    uint32_t enableHorzInterpolation,
    uint32_t enableVertInterpolation
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL);
    
    if (enableHorzInterpolation)
        value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, HORZ_MODE, INTERPOLATE);
    else
        value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, HORZ_MODE, REPLICATE);
        
    if (enableVertInterpolation)
        value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, VERT_MODE, INTERPOLATE);
    else
        value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, VERT_MODE, REPLICATE);
        
    pokeRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL, value);
}

/*
 * This function initializes the video alpha plane before usage.
 *   
 *  Input:
 *      x               - x screen coordinate of the alpha display
 *      y               - y screen coordinate of the alpha display
 *      srcWidth        - Video Alpha Source window width
 *      srcHeight       - Video Alpha Source window height
 *      dstWidth,       - Destination window width
 *      dstHeight,      - Destination window height 
 *      base            - Source base address of the alpha display
 *      pitch           - Alpha Source base Address pitch
 *      alphaFormat     - Alpha Format:
 *                           VIDEO_ALPHA_FORMAT_8BIT_INDEXED - Normal 8-bit indexed
 *                           VIDEO_ALPHA_FORMAT_RGB565 - Normal RGB565
 *                           VIDEO_ALPHA_FORMAT_8BIT_A44 - 8-bit index Alpha | 4:4 mode
 *                           VIDEO_ALPHA_FORMAT_ARGB_4444 - 16-bit ARGB
 *      useAlphaValue   - Use alpha value
 *      alphaValue      - Alpha value to be used to calculate the alpha
 *      pixelPanning    - Starting pixel number for Smooth pixel panning                      
 */
void videoAlphaInit(
    uint32_t x,                /* X Coordinate of the video Alpha Display */
    uint32_t y,                /* Y Coordinate of the video Alpha Display */
    uint32_t srcWidth,         /* Width of the source video alpha display window */
    uint32_t srcHeight,        /* Height of the source video alpha display window */
    uint32_t dstWidth,         /* Width of the destination video alpha display window */
    uint32_t dstHeight,        /* Height of the destination video alpha display window */
    uint32_t base,             /* Base address of the source video alpha display */
    uint32_t pitch,            /* Pitch of the video alpha display */
    video_alpha_format_t format,    /* Video Alpha Format */
    uint32_t useAlphaValue,    /* Flag to indicate of using the given Alpha Value */
    uint32_t alphaValue,       /* Alpha value. Only valid when the useAlphaValue is
                                       set to 1 */
    uint32_t pixelPanning      /* Starting Pixel number for Smooth Pixel Panning */                                       
)
{
    uint32_t value = 0;
    
    /* Set the X and Y Coordinate */
    videoAlphaSetPosition(x, y);
    
    /* Set Video Alpha Scale */
    videoAlphaScale(srcWidth, srcHeight, dstWidth, dstHeight);
    
    /* Set Alpha address */
    videoAlphaSetBuffer(base, pitch * srcHeight, pitch);
    
    /* Set alpha window width */
    videoAlphaSetWindowSize(dstWidth, dstHeight);        
    
    /* Setup Alpha settings and enable it */
    videoAlphaSelectMethod(useAlphaValue, alphaValue);
    
    value = peekRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL);
    
    /* Set the alpha format */
    switch(format)
    {
        case VIDEO_ALPHA_FORMAT_8BIT_INDEXED:
            value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, FORMAT, 8);
            break;
        default:
        case VIDEO_ALPHA_FORMAT_RGB565:       /* Normal RGB 565 mode */
            value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, FORMAT, 16);
            break;
        case VIDEO_ALPHA_FORMAT_8BIT_A44:   /* 8-bit index Alpha | 4:4 mode */
            value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, FORMAT, ALPHA_4_4);
            break;
        case VIDEO_ALPHA_FORMAT_ARGB_4444:    /* 16-bit ARGB 4:4:4:4 mode */
            value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, FORMAT, ALPHA_4_4_4_4);
            break;
    }
            
    /* Set starting pixel number for smooth pixel panning */
    value = FIELD_VALUE(value, VIDEO_ALPHA_DISPLAY_CTRL, PIXEL, pixelPanning);

    pokeRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL, value);    
}

/*
 * This function enables/disables the video alpha display
 *
 * Input:
 *      enable  - Enable/Disable Video Alpha Display Plane
 */
void videoAlphaEnableDisplay(
    uint32_t enable
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL);
    if (enable)
        value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, PLANE, ENABLE);
    else
        value = FIELD_SET(value, VIDEO_ALPHA_DISPLAY_CTRL, PLANE, DISABLE);
    
    pokeRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL, value);
}
