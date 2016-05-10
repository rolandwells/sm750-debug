/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  ALPHA.H --- Voyager GX SDK 
*  This file contains the definitions for the ALPHA functions.
* 
*******************************************************************/
#include <stdint.h>
#include <xf86str.h>
#ifndef _ALPHA_H_
#define _ALPHA_H_

/* FIFO Request Level */
typedef enum _alpha_fifo_t
{
    ALPHA_FIFO_LEVEL_1 = 0,
    ALPHA_FIFO_LEVEL_3,
    ALPHA_FIFO_LEVEL_7,
    ALPHA_FIFO_LEVEL_11
}
alpha_fifo_t;

typedef enum _alpha_format_t
{
    ALPHA_FORMAT_RGB565 = 0,        /* Normal RGB 565 mode */
    ALPHA_FORMAT_8BIT_A44,          /* 8-bit index Alpha | 4:4 mode */
    ALPHA_FORMAT_ARGB_4444          /* 16-bit ARGB 4:4:4:4 mode, A = alpha value*/
}
alpha_format_t;

/* Function Prototype */


/* 
 * This function sets the alpha window size.
 *   
 *  Input:
 *      width       - Video Window width
 *      height      - Video Window height
 *             
 */
void alphaSetWindowSize(
    uint32_t width,
    uint32_t height
);

/*
 * This function gets the alpha window size.
 *  
 *  Output:
 *      pAlphaWidth     - Pointer to a buffer to save the alpha window width
 *      pAlphaHeight    - Pointer to a buffer to save the alpha window height
 *             
 */
void alphaGetWindowSize(
    uint32_t *pAlphaWidth,
    uint32_t *pAlphaHeight
);

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
);

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
);

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
);

/*
 * This function sets the alpha FIFO Request Level.
 *  
 *  Input:
 *      alphaFIFO - Alpha FIFO Request Level
 *                     
 */
void alphaSetFIFOLevel(
    alpha_fifo_t alphaFIFO
);

/*
 * This function sets the RGB565 color lookup table for each 16 
 * 4-bit indexed colors.
 * 
 * Input:
 *      pColorLookupTable   - Pointer to a RGB565 colors of each 4-bit indexed 
 *                            colors. There should be 16 members
 */
void alphaSetColorLookupTable(
    uint32_t *pColorLookupTable    /* Color Lookup Table */
);

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
);

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
);

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
);

/*
 * This function enables/disables the alpha display
 *
 * Input:
 *      enableAlpha - Enable/Disable Alpha Display Plane
 */
_X_EXPORT void alphaEnableDisplay(
    uint32_t enableAlpha
);

/*****************************************************************************
 * Old Function
 *****************************************************************************/
#if 0

typedef struct _alpha_t
{
   unsigned char   EN;           /* Alpha Plane, 0: Disable, 1: Enable */
   uint32_t   Address;
   unsigned short  Width;
   unsigned short  Height;
   unsigned short  Top_Location;
   unsigned short  Left_Location;
   unsigned char   Sel;          /* 0: Use per-pixel value, 1: Use alpha value */
   unsigned char   Avalue; 
   unsigned char   CK;           /* Color Key, 0: Disable, 1: Enable  */
   unsigned char   Format;       /* 01:16-bit RGB, 10:8-bit 4:4, 11:16-bit 4:4:4:4*/
   unsigned short  CK_Mask;
   unsigned short  CK_Value;
   unsigned short  Lookup1;
   unsigned short  Lookup2;
}
alpha_t;

void set_ALPHA(alpha_t alpha);
void Set_ALPHA_Position(unsigned short Top_Position, unsigned short Left_Position);
#endif

#endif /* _ALPHA_H_ */
