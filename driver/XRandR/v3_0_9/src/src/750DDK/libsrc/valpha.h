/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  Valpha.H --- Voyager GX SDK 
*  This file contains the definitions for the Video Alpha functions.
* 
*******************************************************************/
#ifndef _VALPHA_H_
#define _VALPHA_H_

/* FIFO Request Level */
typedef enum _video_alpha_fifo_t
{
    VIDEO_ALPHA_FIFO_LEVEL_1 = 0,
    VIDEO_ALPHA_FIFO_LEVEL_3,
    VIDEO_ALPHA_FIFO_LEVEL_7,
    VIDEO_ALPHA_FIFO_LEVEL_11
}
video_alpha_fifo_t;

typedef enum _video_alpha_format_t
{
    VIDEO_ALPHA_FORMAT_8BIT_INDEXED = 0,  /* Normal 8-bit indexed mode */
    VIDEO_ALPHA_FORMAT_RGB565,            /* Normal RGB 565 mode */
    VIDEO_ALPHA_FORMAT_8BIT_A44,          /* 8-bit indexed Alpha | 4:4 mode */
    VIDEO_ALPHA_FORMAT_ARGB_4444          /* 16-bit ARGB 4:4:4:4 mode, A = alpha value*/
}
video_alpha_format_t;

/* Function Prototype */


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
);

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
);

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
);

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
);

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
);

/*
 * This function sets the video alpha FIFO Request Level.
 *  
 *  Input:
 *      fifo    - Video Alpha FIFO Request Level
 *                     
 */
void videoAlphaSetFIFOLevel(
    video_alpha_fifo_t fifo
);

/*
 * This function sets the RGB565 color lookup table for each 16 
 * 4-bit indexed colors.
 * 
 * Input:
 *      pColorLookupTable   - Pointer to a RGB565 colors of each 4-bit indexed 
 *                            colors. There should be 16 members
 */
void videoAlphaSetColorLookupTable(
    uint32_t *pColorLookupTable    /* Color Lookup Table */
);

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
);

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
);

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
);

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
 *      systemMemory    - Memory location of the alpha display source base address
 *                       0 = frame buffer
 *                       1 = system memory
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
);

/*
 * This function enables/disables the video alpha display
 *
 * Input:
 *      enable  - Enable/Disable Video Alpha Display Plane
 */
void videoAlphaEnableDisplay(
    uint32_t enable
);

#endif /* _VALPHA_H_ */
