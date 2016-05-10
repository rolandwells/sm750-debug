/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  ALPHA.C --- Voyager GX SDK 
*  This file contains the definitions for the ALPHA functions.
* 
*******************************************************************/
#include "defs.h"
#include "alpha.h"
#include "hardware.h"

/* 
 * This function sets the alpha window size.
 *   
 *  Input:
 *      width       - Alpha Window width
 *      height      - Alpha Window height
 *             
 */
void alphaSetWindowSize(
    uint32_t width,
    uint32_t height
)
{
    uint32_t value, startX, startY;

    /* Get the video window width and height */
    value = peekRegisterDWord(ALPHA_PLANE_TL);
    startX = FIELD_GET(value, ALPHA_PLANE_TL, LEFT);
    startY = FIELD_GET(value, ALPHA_PLANE_TL, TOP);

    /* Set bottom and right position */
    pokeRegisterDWord(ALPHA_PLANE_BR, 
        FIELD_VALUE(0, ALPHA_PLANE_BR, BOTTOM, startY + height - 1) |
        FIELD_VALUE(0, ALPHA_PLANE_BR, RIGHT, startX + width - 1)); 
}

/*
 * This function gets the alpha window size.
 *  
 *  Output:
 *      width       - Alpha Window width
 *      height      - Alpha Window height
 *             
 */
void alphaGetWindowSize(
    uint32_t *pAlphaWidth,
    uint32_t *pAlphaHeight
)
{
    uint32_t positionTopLeft, positionRightBottom;
    uint32_t alphaWidth, alphaHeight;
    
    /* Get the video window width and height */
    positionTopLeft = peekRegisterDWord(ALPHA_PLANE_TL);
    positionRightBottom = peekRegisterDWord(ALPHA_PLANE_BR);
    
    alphaWidth  = FIELD_GET(positionRightBottom, ALPHA_PLANE_BR, RIGHT) - 
                  FIELD_GET(positionTopLeft, ALPHA_PLANE_TL, LEFT) + 1;
    alphaHeight = FIELD_GET(positionRightBottom, ALPHA_PLANE_BR, BOTTOM) - 
                  FIELD_GET(positionTopLeft, ALPHA_PLANE_TL, TOP) + 1;
              
    if (pAlphaWidth != ((uint32_t *)0))
        *pAlphaWidth = alphaWidth;
    
    if (pAlphaHeight != ((uint32_t *)0))
        *pAlphaHeight = alphaHeight;
}

/*
 * This function sets the alpha starting coordinate position.
 *  
 *  Input:
 *      x       - X Coordinate of the alpha window starting position
 *      y       - Y Coordinate of the alpha window starting position
 *             
 */
void alphaSetPosition(
    uint32_t x,
    uint32_t y
)
{
    uint32_t alphaWidth, alphaHeight;
    
    /* Get the video window width and height */
    alphaGetWindowSize(&alphaWidth, &alphaHeight);
    
    /* Set top and left position */
    pokeRegisterDWord(ALPHA_PLANE_TL,
        FIELD_VALUE(0, ALPHA_PLANE_TL, TOP, y) |
        FIELD_VALUE(0, ALPHA_PLANE_TL, LEFT, x));
    
    /* Set bottom and right position */    
    alphaSetWindowSize(alphaWidth, alphaHeight);
}

/*
 * This function gets the alpha starting coordinate position.
 *  
 *  Input:
 *      pX      - Pointer to a buffer to save the x Coordinate of the current alpha coordinate
 *      pY      - Pointer to a buffer to save the y Coordinate of the current alpha coordinate
 *             
 */
void alphaGetPosition(
    uint32_t *pX,
    uint32_t *pY
)
{
    uint32_t positionTopLeft;
    
    /* Get the starting coordinate of the alpha plane */
    positionTopLeft = peekRegisterDWord(ALPHA_PLANE_TL);
    
    if (pX)
        *pX = (uint32_t) FIELD_GET(positionTopLeft, ALPHA_PLANE_TL, LEFT);
    
    if (pY)
        *pY = (uint32_t) FIELD_GET(positionTopLeft, ALPHA_PLANE_TL, TOP);
}

/*
 * This function enables/disables alpha chroma key feature.
 *  
 *  Input:
 *      enableChromaKey - Flag to enable/disable the chroma key.
 *      chromaKeyMask   - Chroma key mask
 *      chromaKeyValue  - Chroma key value       
 */
void alphaSetChroma(
    uint32_t enableChromaKey,  /* Flag to enable/disable the chroma key
                                       on the Alpha display plane */
    uint32_t chromaKeyMask,    /* Chroma Key Mask */
    uint32_t chromaKeyValue    /* Chroma Key Value */
)
{
    uint32_t value;
    
    value = peekRegisterDWord(ALPHA_DISPLAY_CTRL);
    if (enableChromaKey)
    {
        value = FIELD_SET(value, ALPHA_DISPLAY_CTRL, CHROMA_KEY, ENABLE);
        
        /* Set the chroma key mask and value */
        pokeRegisterDWord(ALPHA_CHROMA_KEY,
            FIELD_VALUE(0, ALPHA_CHROMA_KEY, MASK, chromaKeyMask) |
            FIELD_VALUE(0, ALPHA_CHROMA_KEY, VALUE, chromaKeyValue));
    }
    else
        value = FIELD_SET(value, ALPHA_DISPLAY_CTRL, CHROMA_KEY, DISABLE);
        
    pokeRegisterDWord(ALPHA_DISPLAY_CTRL, value);
}

/*
 * This function sets the alpha FIFO Request Level.
 *  
 *  Input:
 *      alphaFIFO - Alpha FIFO Request Level
 *                     
 */
void alphaSetFIFOLevel(
    alpha_fifo_t alphaFIFO
)
{
    uint32_t value;
    
    value = peekRegisterDWord(ALPHA_DISPLAY_CTRL);
    switch(alphaFIFO)
    {
        default:
        case ALPHA_FIFO_LEVEL_1:
            value = FIELD_SET(value, ALPHA_DISPLAY_CTRL, FIFO, 1);
            break; 
        case ALPHA_FIFO_LEVEL_3:
            value = FIELD_SET(value, ALPHA_DISPLAY_CTRL, FIFO, 3);
            break;
        case ALPHA_FIFO_LEVEL_7:
            value = FIELD_SET(value, ALPHA_DISPLAY_CTRL, FIFO, 7);
            break;
        case ALPHA_FIFO_LEVEL_11:
            value = FIELD_SET(value, ALPHA_DISPLAY_CTRL, FIFO, 11);
            break;
    }
    
    pokeRegisterDWord(ALPHA_DISPLAY_CTRL, value);
}

/*
 * This function sets the RGB565 color lookup table for each 16 
 * 4-bit indexed colors.
 * 
 * Input:
 *      pColorLookupTable   - Pointer to a RGB565 colors of each 4-bit indexed 
 *                            colors. It has to have 16 members.
 */
void alphaSetColorLookupTable(
    uint32_t *pColorLookupTable    /* Color Lookup Table */
)
{
    uint32_t index, value;
   
    for (index = 0; index < 16; index+=2)
    {
        value = FIELD_VALUE(0, ALPHA_COLOR_LOOKUP_01, 0, pColorLookupTable[index]) |
                FIELD_VALUE(0, ALPHA_COLOR_LOOKUP_01, 1, pColorLookupTable[index+1]);
        pokeRegisterDWord(ALPHA_COLOR_LOOKUP_01 + index, value);
    }
}

/*
 * This function select the alpha method, either to use per-pixel
 * or by the given alpha value.
 * 
 * Input:
 *      useAlphaValue   - Use the given alpha value.
 *                          0 - Use per-pixel alpha value
 *                          1 - Use the given alpha value
 *      alphaValue      - Alpha value to be used in the alpha plane
 */
void alphaSelectMethod(
    uint32_t useAlphaValue,
    uint32_t alphaValue
)
{
    uint32_t value;
    
    value = peekRegisterDWord(ALPHA_DISPLAY_CTRL);
    if (useAlphaValue)
        value = FIELD_SET(0, ALPHA_DISPLAY_CTRL, SELECT, ALPHA) |
                FIELD_VALUE(value, ALPHA_DISPLAY_CTRL, ALPHA, alphaValue);
    else
        value = FIELD_SET(value, ALPHA_DISPLAY_CTRL, SELECT, PER_PIXEL); 
    
    pokeRegisterDWord(ALPHA_DISPLAY_CTRL, value);
}

/*
 * This function gets the current alpha method and value (when applicable) used.
 *
 * Input:
 *      pAlphaValue - Pointer to save the alpha value used (if the return
 *                    value is 1 (Given alpha value method)
 * 
 * Returns:
 *      0   - Per-pixel value method
 *      1   - Given alpha value method
 */
uint32_t alphaGetAlphaMethod(
    uint32_t *pAlphaValue
)
{
    uint32_t value;
    
    value = peekRegisterDWord(ALPHA_DISPLAY_CTRL);
    
    if (pAlphaValue)
        *pAlphaValue = FIELD_GET(value, ALPHA_DISPLAY_CTRL, ALPHA);
    
    if (FIELD_GET(value, ALPHA_DISPLAY_CTRL, SELECT) == ALPHA_DISPLAY_CTRL_SELECT_ALPHA)
        return 1;

    return 0;
}

/*
 * This function initializes the alpha plane before usage.
 *   
 *  Input:
 *      x               - x screen coordinate of the alpha display
 *      y               - y screen coordinate of the alpha display
 *      width           - Alpha display window width
 *      height          - Alpha display window height
 *      base            - Source base address of the alpha display
 *      systemMemory    - Memory location of the alpha display source base address
 *                       0 = frame buffer
 *                       1 = system memory
 *      pitch           - Alpha Source base Address pitch
 *      alphaFormat     - Alpha Format:
 *                       ALPHA_FORMAT_RGB565 - Normal RGB565
 *                       ALPHA_FORMAT_8BIT_A44 - 8-bit index Alpha | 4:4 mode
 *                       ALPHA_FORMAT_ARGB_4444 - 16-bit ARGB
 *      useAlphaValue   - Use alpha value
 *      alphaValue      - Alpha value to be used to calculate the alpha
 *      pixelPanning    - Starting pixel number for Smooth pixel panning                      
 */
_X_EXPORT void alphaInit(
    uint32_t x,                /* X Coordinate of the Alpha Display */
    uint32_t y,                /* Y Coordinate of the Alpha Display */
    uint32_t width,            /* Width of the alpha display window */
    uint32_t height,           /* Height of the alpha display window */
    uint32_t base,             /* Base address of the alpha display */
    uint32_t systemMemory,     /* Memory location of the alpha display source
                                       base address.
                                            0 = frame buffer
                                            1 = system memory
                                     */
    uint32_t pitch,            /* Pitch of the alpha display */
    alpha_format_t alphaFormat,     /* Alpha Format */
    uint32_t useAlphaValue,    /* Flag to indicate of using the given Alpha Value */
    uint32_t alphaValue,       /* Alpha value. Only valid when the useAlphaValue is
                                       set to 1 */
    uint32_t pixelPanning      /* Starting Pixel number for Smooth Pixel Panning */                                       
)
{
    uint32_t value = 0;
    
    /* Set Alpha address */
    pokeRegisterDWord(ALPHA_FB_ADDRESS,
        FIELD_SET(0, ALPHA_FB_ADDRESS, STATUS, PENDING) |
        ((systemMemory) ?
            FIELD_SET(0, ALPHA_FB_ADDRESS, EXT, EXTERNAL) :
            FIELD_SET(0, ALPHA_FB_ADDRESS, EXT, LOCAL)) |
        FIELD_VALUE(0, ALPHA_FB_ADDRESS, ADDRESS, base));
        
    /* Setup the pitch and width of the alpha plane */
    pokeRegisterDWord(ALPHA_FB_WIDTH,
        FIELD_VALUE(0, ALPHA_FB_WIDTH, WIDTH, pitch) |
        FIELD_VALUE(0, ALPHA_FB_WIDTH, OFFSET, pitch));
    
    /* Set the X and Y Coordinate */
    alphaSetPosition(x, y);
    
    /* Set alpha window width */
    alphaSetWindowSize(width, height);        
    
    /* Setup Alpha settings and enable it */
    alphaSelectMethod(useAlphaValue, alphaValue);
    
    value = peekRegisterDWord(ALPHA_DISPLAY_CTRL);
    
    /* Set the alpha format */
    switch(alphaFormat)
    {
        default:
        case ALPHA_FORMAT_RGB565:       /* Normal RGB 565 mode */
            value = FIELD_SET(value, ALPHA_DISPLAY_CTRL, FORMAT, 16);
            break;
        case ALPHA_FORMAT_8BIT_A44:   /* 8-bit index Alpha | 4:4 mode */
            value = FIELD_SET(value, ALPHA_DISPLAY_CTRL, FORMAT, ALPHA_4_4);
            break;
        case ALPHA_FORMAT_ARGB_4444:    /* 16-bit ARGB 4:4:4:4 mode */
            value = FIELD_SET(value, ALPHA_DISPLAY_CTRL, FORMAT, ALPHA_4_4_4_4);
            break;
    }
            
    /* Set starting pixel number for smooth pixel panning */
    value = FIELD_VALUE(value, ALPHA_DISPLAY_CTRL, PIXEL, pixelPanning);

    pokeRegisterDWord(ALPHA_DISPLAY_CTRL, value);    
}

/*
 * This function enables/disables the alpha display
 *
 * Input:
 *      enableAlpha - Enable/Disable Alpha Display Plane
 */
_X_EXPORT void alphaEnableDisplay(
    uint32_t enableAlpha
)
{
    uint32_t value;
    
    value = peekRegisterDWord(ALPHA_DISPLAY_CTRL);
    if (enableAlpha)
        value = FIELD_SET(value, ALPHA_DISPLAY_CTRL, PLANE, ENABLE);
    else
        value = FIELD_SET(value, ALPHA_DISPLAY_CTRL, PLANE, DISABLE);
    
    pokeRegisterDWord(ALPHA_DISPLAY_CTRL, value);
}

