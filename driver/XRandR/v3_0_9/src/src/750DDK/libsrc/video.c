/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  Video.C --- Voyager GX SDK 
*  This file contains the definitions for the Video functions.
* 
*******************************************************************/
#include "defs.h"
#include "hardware.h"
#include "video.h"

/* New video function */

#define SCALE_CONSTANT                      (1 << 12)

/* Offset Adjustment for the window */
static short gWidthAdjustment = 0;
static short gHeightAdjustment = 0;

/* Source Video Width and Height */
static uint32_t gSrcVideoWidth = 0;
static uint32_t gSrcVideoHeight = 0;

/*
 *  videoSetWindowAdjustment
 *      This function sets the video window adjustment. There are usually
 *      some garbage lines or pixels at the bottom and right of the video
 *      window. These function will adjust the video window accordingly.
 *
 *  Input:
 *      widthAdjustment     - Width adjustments in pixel
 *      heightAdjustment    - Height adjustment in line        
 */
void videoSetWindowAdjustment(
    short widthAdjustment,
    short heightAdjustment
)
{
    uint32_t width, height;
    videoGetWindowSize(&width, &height);
    
    gWidthAdjustment = widthAdjustment;
    gHeightAdjustment = heightAdjustment;
    
    videoSetWindowSize(width, height);
}

/*
 *  videoGetWindowAdjustment
 *      This function gets the video window adjustment.
 *
 *  Input:
 *      widthAdjustment     - Width adjustments in pixel
 *      heightAdjustment    - Height adjustment in line 
 */
void videoGetWindowAdjustment(
    short *pWidthAdjustment,
    short *pHeightAdjustment
)
{
    if (pWidthAdjustment != ((short *)0))
        *pWidthAdjustment = gWidthAdjustment;
    
    if (pHeightAdjustment != ((short *)0))
        *pHeightAdjustment = gHeightAdjustment;
}

/*
 * videoGetCurrentBufferDisplay
 *      This function gets the current buffer used by SM50x to display on the screen
 *
 *  Return:
 *      0   - Buffer 0
 *      1   - Buffer 1 
 */
unsigned char videoGetCurrentBufferDisplay()
{
    uint32_t vlaue;
    
    if (FIELD_GET(peekRegisterDWord(VIDEO_DISPLAY_CTRL), VIDEO_DISPLAY_CTRL, BUFFER) ==
                VIDEO_DISPLAY_CTRL_BUFFER_1)
    {
        return 1;
    }
    
    return 0;
}

/*
 * videoEnableDoubleBuffer
 *      This function enables/disables the double buffer usage
 *
 *  Input:
 *      enable  - Flag to enable/disable the double buffer. 
 */
void videoEnableDoubleBuffer(
    uint32_t enable
)
{
    uint32_t value;
   
    value = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
    
    /* Enable/Disable the double buffer */
    if (enable == 1)
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, DOUBLE_BUFFER, ENABLE);
    else
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, DOUBLE_BUFFER, DISABLE);
                
    pokeRegisterDWord(VIDEO_DISPLAY_CTRL, value);
}

/*
 * videoGetBufferStatus
 *      This function gets the status of the video buffer, either the buffer
 *      has been used or not.
 *
 *  Input:
 *      bufferIndex     - The index of the buffer which size to be retrieved
 *
 *  Output:
 *      0 - No flip pending
 *      1 - Flip pending
 */
uint32_t videoGetBufferStatus(
    uint32_t bufferIndex
)
{
    if (bufferIndex == 0)
        return (FIELD_GET(peekRegisterDWord(VIDEO_FB_0_ADDRESS), VIDEO_FB_0_ADDRESS, STATUS));
    else
        return (FIELD_GET(peekRegisterDWord(VIDEO_FB_1_ADDRESS), VIDEO_FB_1_ADDRESS, STATUS));
}

/*
 * videoGetPitch
 *      This function gets the video plane pitch
 *
 * Output:
 *      pitch   - Number of bytes per line of the video plane 
 *                specified in 128-bit aligned bytes.
 */
unsigned short videoGetPitch()
{
    return (FIELD_GET(peekRegisterDWord(VIDEO_FB_WIDTH), VIDEO_FB_WIDTH, WIDTH));
}

/*
 * videoGetLineOffset
 *      This function gets the video plane line offset
 *
 * Output:
 *      lineOffset  - Number of 128-bit aligned bytes per line 
 *                    of the video plane.
 */
unsigned short videoGetLineOffset()
{
    return (FIELD_GET(peekRegisterDWord(VIDEO_FB_WIDTH), VIDEO_FB_WIDTH, OFFSET));
}

/*
 * videoGetBufferSize
 *      This function gets the buffer size
 *
 *  Input:
 *      bufferIndex - The index of the buffer which size to be retrieved
 */
uint32_t videoGetBufferSize(
    uint32_t bufferIndex
)
{
    uint32_t value;
    
    if (bufferIndex == 0)
    {
        /* Get video buffer 0 size */
        value = (uint32_t) 
            FIELD_GET(peekRegisterDWord(VIDEO_FB_0_LAST_ADDRESS), VIDEO_FB_0_LAST_ADDRESS, ADDRESS);
            
        value -= (uint32_t)
            FIELD_GET(peekRegisterDWord(VIDEO_FB_0_ADDRESS), VIDEO_FB_0_ADDRESS, ADDRESS);
    }
    else
    {
        /* Get video buffer 1 size */
        value = (uint32_t) 
            FIELD_GET(peekRegisterDWord(VIDEO_FB_1_LAST_ADDRESS), VIDEO_FB_1_LAST_ADDRESS, ADDRESS);
            
        value -= (uint32_t)
            FIELD_GET(peekRegisterDWord(VIDEO_FB_1_ADDRESS), VIDEO_FB_1_ADDRESS, ADDRESS);
    }
    
    /* Add with one line offset, since the Video FB 0/1 Last Address is the last line of the
       video buffer. */
    value += (uint32_t) videoGetLineOffset();
    
    return value;
}


/*
 * videoGetBuffer
 *      This function gets the video buffer
 *
 *  Input:
 *      bufferIndex - The index of the buffer to get
 *
 *  Output:
 *      The video buffer of the requested index.
 */
uint32_t videoGetBuffer(
    unsigned char bufferIndex
)
{
    if (bufferIndex == 0)
        return (FIELD_GET(peekRegisterDWord(VIDEO_FB_0_ADDRESS), VIDEO_FB_0_ADDRESS, ADDRESS));
    else
        return (FIELD_GET(peekRegisterDWord(VIDEO_FB_1_ADDRESS), VIDEO_FB_1_ADDRESS, ADDRESS));    
}

/*
 * videoSetBufferLastAddress
 *      This function sets the video buffer last address.
 *      The value can be calculated by subtracting one line offset 
 *      from the buffer size (Total number of line offset * 
 *      source video height).
 *
 *  Input:
 *      bufferIndex         - The index of the buffer to be set
 *      bufferSize          - Size of the video buffer.
 */
void videoSetBufferLastAddress(
    unsigned char bufferIndex,          /* The index of the buffer to be set. */
    uint32_t bufferSize            /* Size of the video buffer */
)
{
    uint32_t bufferStart;

    /* Get the buffer start Address. */
    bufferStart = videoGetBuffer(bufferIndex);
    
    /* Substract with one line offset to get the last address value when added
       with the bufferStart. */    
    bufferSize -= (uint32_t) videoGetLineOffset();
        
    if (bufferIndex == 0)
    {
        /* Set Video Buffer 0 Last Address */
        pokeRegisterDWord(VIDEO_FB_0_LAST_ADDRESS,
            FIELD_SET(0, VIDEO_FB_0_LAST_ADDRESS, EXT, LOCAL) |
            FIELD_VALUE(0, VIDEO_FB_0_LAST_ADDRESS, ADDRESS, bufferStart + bufferSize));
    }
    else
    {   
        /* Set Video Buffer 1 Last Address */ 
        pokeRegisterDWord(VIDEO_FB_1_LAST_ADDRESS,
            FIELD_SET(0, VIDEO_FB_1_LAST_ADDRESS, EXT, LOCAL) |
            FIELD_VALUE(0, VIDEO_FB_1_LAST_ADDRESS, ADDRESS, bufferStart + bufferSize));
    }
}

/*
 * videoSetBuffer
 *      This function sets the video buffer
 *
 *  Input:
 *      bufferIndex         - The index of the buffer to be set
 *      bufferStartAddress  - The starting address of the buffer
 */
void videoSetBuffer(
    unsigned char bufferIndex,          /* The index of the buffer to be set. */
    uint32_t bufferStartAddress    /* Video buffer with 128-bit alignment */
)
{
    uint32_t bufferSize;

    /* Get the buffer size first */
    bufferSize = videoGetBufferSize(bufferIndex);
           
    if (bufferIndex == 0)
    {
        /* Set Video Buffer 0 Start Address */
        pokeRegisterDWord(VIDEO_FB_0_ADDRESS,
            FIELD_SET(0, VIDEO_FB_0_ADDRESS, STATUS, PENDING) |
            FIELD_SET(0, VIDEO_FB_0_ADDRESS, EXT, LOCAL) |
            FIELD_VALUE(0, VIDEO_FB_0_ADDRESS, ADDRESS, bufferStartAddress));
    }
    else
    {   
        /* Set Video Buffer 1 Start Address */
        pokeRegisterDWord(VIDEO_FB_1_ADDRESS,
            FIELD_SET(0, VIDEO_FB_1_ADDRESS, STATUS, PENDING) |
            FIELD_SET(0, VIDEO_FB_1_ADDRESS, EXT, LOCAL) |
            FIELD_VALUE(0, VIDEO_FB_1_ADDRESS, ADDRESS, bufferStartAddress));
    }
    
    /* Do not forget to update the Video Last Address */
    videoSetBufferLastAddress(bufferIndex, bufferSize);
}

/*
 * videoSetPitchOffset
 *      This function sets the video plane pitch and offset
 *
 *  Input:
 *      pitch           - Number of bytes per line of the video plane 
 *                        specified in 128-bit aligned bytes.
 *      lineOffset      - Number of 128-bit aligned bytes per line 
 *                        of the video plane.
 */
void videoSetPitchOffset(
    unsigned short pitch,
    unsigned short lineOffset
)
{
    /* Set Video Buffer Offset (pitch) */
    pokeRegisterDWord(VIDEO_FB_WIDTH,
        FIELD_VALUE(0, VIDEO_FB_WIDTH, WIDTH, pitch) |
        FIELD_VALUE(0, VIDEO_FB_WIDTH, OFFSET, lineOffset));
        
    /* Need to update the buffer last address */
    videoSetBufferLastAddress(0, lineOffset * gSrcVideoHeight);
    videoSetBufferLastAddress(1, lineOffset * gSrcVideoHeight);
}

/*
 *  videoSetWindowSize
 *      This function sets the video window size.
 *
 *  Input:
 *      width       - Video Window width
 *      height      - Video Window height
 */
void videoSetWindowSize(
    uint32_t width,
    uint32_t height
)
{
    uint32_t value, startX, startY;

    /* Get the video window width and height */
    value = peekRegisterDWord(VIDEO_PLANE_TL);
    startX = FIELD_GET(value, VIDEO_PLANE_TL, LEFT);
    startY = FIELD_GET(value, VIDEO_PLANE_TL, TOP);

    /* Set bottom and right position */
    pokeRegisterDWord(VIDEO_PLANE_BR, 
        FIELD_VALUE(0, VIDEO_PLANE_BR, BOTTOM, startY + height - 1 - gHeightAdjustment) |
        FIELD_VALUE(0, VIDEO_PLANE_BR, RIGHT, startX + width - 1 - gWidthAdjustment)); 
}

/*
 *  videoGetWindowSize
 *      This function gets the video window size.
 *
 *  Output:
 *      width       - Video Window width
 *      height      - Video Window height
 */
void videoGetWindowSize(
    uint32_t *pVideoWidth,
    uint32_t *pVideoHeight
)
{
    uint32_t positionTopLeft, positionRightBottom;
    uint32_t videoWidth, videoHeight;
    
    /* Get the video window width and height */
    positionTopLeft = peekRegisterDWord(VIDEO_PLANE_TL);
    positionRightBottom = peekRegisterDWord(VIDEO_PLANE_BR);
    videoWidth  = FIELD_GET(positionRightBottom, VIDEO_PLANE_BR, RIGHT) - 
                  FIELD_GET(positionTopLeft, VIDEO_PLANE_TL, LEFT) + 1 +
                  gWidthAdjustment;
    videoHeight = FIELD_GET(positionRightBottom, VIDEO_PLANE_BR, BOTTOM) - 
                  FIELD_GET(positionTopLeft, VIDEO_PLANE_TL, TOP) + 1 +
                  gHeightAdjustment;
              
    if (pVideoWidth != ((uint32_t *)0))
        *pVideoWidth = videoWidth;
    
    if (pVideoHeight != ((uint32_t *)0))
        *pVideoHeight = videoHeight;
}

/*
 *  videoSetPosition
 *      This function sets the video starting coordinate position.
 *
 *  Input:
 *      startX      - X Coordinate of the video window starting position
 *      startY      - Y Coordinate of the video window starting position
 */
void videoSetPosition(
    uint32_t startX,
    uint32_t startY
)
{
    uint32_t videoWidth, videoHeight;
    
    /* Get the video window width and height */
    videoGetWindowSize(&videoWidth, &videoHeight);

    /* Set top and left position */
    pokeRegisterDWord(VIDEO_PLANE_TL,
        FIELD_VALUE(0, VIDEO_PLANE_TL, TOP, startY) |
        FIELD_VALUE(0, VIDEO_PLANE_TL, LEFT, startX));
    
    /* Set bottom and right position */    
    videoSetWindowSize(videoWidth, videoHeight);
}

/*
 *  videoSetConstants
 *      This function sets the video constants. The actual component will be
 *      added by this constant to get the expected component value.
 *
 *  Input:
 *      yConstant       - Y Constant Value
 *      redConstant     - Red Constant Value
 *      greenConstant   - Green Constant Value
 *      blueConstant    - Blue Constant Value
 */
void videoSetConstants(
    unsigned char  yConstant,               /* Y Adjustment */
    unsigned char  redConstant,             /* Red Conversion constant */
    unsigned char  greenConstant,           /* Green Conversion constant */
    unsigned char  blueConstant             /* Blue Conversion constant */
)
{
    /* Set Capture Size */
    pokeRegisterDWord(VIDEO_YUV_CONSTANTS,
        FIELD_VALUE(0, VIDEO_YUV_CONSTANTS, Y, yConstant) |
        FIELD_VALUE(0, VIDEO_YUV_CONSTANTS, R, redConstant) |
        FIELD_VALUE(0, VIDEO_YUV_CONSTANTS, G, greenConstant) |
        FIELD_VALUE(0, VIDEO_YUV_CONSTANTS, B, blueConstant));
}

/*
 *  videoSetInitialScale
 *      This function sets the video buffer initial vertical scale.
 *
 *  Input:
 *      bufferIndex         - Index of the buffer which vertical scale value
 *                            to be set.
 *      bufferInitScale     - Buffer Initial vertical scale value
 */
void videoSetInitialScale(
    unsigned char bufferIndex,
    unsigned short bufferInitScale
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_INITIAL_SCALE);
    if (bufferIndex == 0)
        value = FIELD_VALUE(value, VIDEO_INITIAL_SCALE, FB_0, bufferInitScale);
    else
        value = FIELD_VALUE(value, VIDEO_INITIAL_SCALE, FB_1, bufferInitScale);
    
    pokeRegisterDWord(VIDEO_INITIAL_SCALE, value);
}

/*
 *  videoGetInitialScale
 *      This function gets the video buffer initial vertical scale.
 *
 *  Input:
 *      pbuffer0InitScale   - Pointer to variable to store buffer 0 initial vertical scale
 *      pbuffer1InitScale   - Pointer to variable to store buffer 1 initial vertical scale
 */
void videoGetInitialScale(
    unsigned short *pBuffer0InitScale,
    unsigned short *pBuffer1InitScale
)
{
    if (pBuffer0InitScale)
    {
        *pBuffer0InitScale = (unsigned short)
            FIELD_GET(peekRegisterDWord(VIDEO_INITIAL_SCALE), VIDEO_INITIAL_SCALE, FB_0);
    }
            
    if (pBuffer1InitScale)
    {
        *pBuffer1InitScale = (unsigned short)
            FIELD_GET(peekRegisterDWord(VIDEO_INITIAL_SCALE), VIDEO_INITIAL_SCALE, FB_1);
    }
}

/*
 *  videoScale
 *      This function scales the video.
 *
 *  Input:
 *      srcWidth     - The source video width
 *      srcHeight    - The source video height
 *      dstWidth     - The destination video width 
 *      dstHeight    - The destination video height
 */
void videoScale(
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
        value = FIELD_SET(value, VIDEO_SCALE, VERTICAL_MODE, EXPAND);
        
        /* Calculate the factor */
        scaleFactor = srcHeight * SCALE_CONSTANT / dstHeight;
        value = FIELD_VALUE(value, VIDEO_SCALE, VERTICAL_SCALE, scaleFactor);
    }
    else
    {        
        /* Shrink size */
        value = FIELD_SET(value, VIDEO_SCALE, VERTICAL_MODE, SHRINK);
        
        /* Calculate the factor */
        scaleFactor = dstHeight * SCALE_CONSTANT / srcHeight;
        value = FIELD_VALUE(value, VIDEO_SCALE, VERTICAL_SCALE, scaleFactor);
    }
    
    /* Scale the horizontal size */
    if (dstWidth >= srcWidth)
    {
        /* Expand size */
        value = FIELD_SET(value, VIDEO_SCALE, HORIZONTAL_MODE, EXPAND);
        
        /* Calculate the factor */
        scaleFactor = srcWidth * SCALE_CONSTANT / dstWidth;
        value = FIELD_VALUE(value, VIDEO_SCALE, HORIZONTAL_SCALE, scaleFactor);
    }
    else
    {
        /* Shrink size */
        value = FIELD_SET(value, VIDEO_SCALE, HORIZONTAL_MODE, SHRINK);
        
        /* Calculate the factor */
        scaleFactor = dstWidth * SCALE_CONSTANT / srcWidth;
        value = FIELD_VALUE(value, VIDEO_SCALE, HORIZONTAL_SCALE, scaleFactor);
    }
    
    pokeRegisterDWord(VIDEO_SCALE, value);
}

/*
 *  videoSetFIFOLevel
 *      This function sets the video FIFO Request Level.
 *
 *  Input:
 *      videoSource - Buffer source selection
 */
void videoSetFIFOLevel(
    video_fifo_t videoFIFO
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
    switch(videoFIFO)
    {
        default:
        case FIFO_LEVEL_1:
            value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, FIFO, 1);
            break; 
        case FIFO_LEVEL_3:
            value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, FIFO, 3);
            break;
        case FIFO_LEVEL_7:
            value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, FIFO, 7);
            break;
        case FIFO_LEVEL_11:
            value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, FIFO, 11);
            break;
    }
    
    pokeRegisterDWord(VIDEO_DISPLAY_CTRL, value);
}

/*
 *  videoSetSourceBuffer
 *      This function sets the video to use the capture buffer as the source.
 *
 *  Input:
 *      videoSource - Buffer source selection
 */
void videoSetSourceBuffer(
    video_sync_source_t videoSource
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
    if (videoSource == CAPTURE_BUFFER)
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, CAPTURE, ENABLE);
    else
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, CAPTURE, DISABLE);
    
    pokeRegisterDWord(VIDEO_DISPLAY_CTRL, value);
}

/*
 *  videoSwapYUVByte
 *      This function swaps the YUV data byte.
 *
 *  Input:
 *      byteSwap    - Flag to enable/disable YUV data byte swap.
 */
void videoSwapYUVByte(
   video_byteswap_t byteSwap  
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
    if (byteSwap == SWAP_BYTE)
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, BYTE_SWAP, ENABLE);
    else
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, BYTE_SWAP, DISABLE);
    
    pokeRegisterDWord(VIDEO_DISPLAY_CTRL, value);
}

/*
 *  videoForceHalfShrink
 *      This function forces half scale factor (do not need to set the video
 *      scale factor).
 *
 *  Input:
 *      enableHorzHalfShrink    - Flag to enable/disable 1/2 horizontal 
 *                                scale factor
 *      enableVertHalfShrink    - Flag to enable/disable 1/2 vertical scale 
 *                                factor
 */
void videoForceHalfShrink(
    uint32_t enableHorzHalfShrink,
    uint32_t enableVertHalfShrink
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
    if (enableHorzHalfShrink)
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, HORIZONTAL_SCALE, HALF);
    else
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, HORIZONTAL_SCALE, NORMAL);
        
    if (enableVertHalfShrink)
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, VERTICAL_SCALE, HALF);
    else
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, VERTICAL_SCALE, NORMAL);
    
    pokeRegisterDWord(VIDEO_DISPLAY_CTRL, value);
}

/*
 *  videoSetInterpolation
 *      This function enables/disables the horizontal and vertical interpolation.
 *
 *  Input:
 *      enableHorzInterpolation   - Flag to enable/disable Horizontal interpolation
 *      enableVertInterpolation   - Flag to enable/disable Vertical interpolation
 */
void videoSetInterpolation(
    uint32_t enableHorzInterpolation,
    uint32_t enableVertInterpolation
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
    
    if (enableHorzInterpolation)
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, HORIZONTAL_MODE, INTERPOLATE);
    else
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, HORIZONTAL_MODE, REPLICATE);
        
    if (enableVertInterpolation)
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, VERTICAL_MODE, INTERPOLATE);
    else
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, VERTICAL_MODE, REPLICATE);
        
    pokeRegisterDWord(VIDEO_DISPLAY_CTRL, value);
}

/*
 *  videoSetStartPanningPixel
 *      This function sets the starting pixel number for smooth pixel panning.
 *
 *  Input:
 *      startPixel  - Starting pixel number for smooth pixel panning
 */
void videoSetStartPanningPixel(
    unsigned char startPixel
)
{
    pokeRegisterDWord(VIDEO_DISPLAY_CTRL, 
                      peekRegisterDWord(VIDEO_DISPLAY_CTRL) | 
                      FIELD_VALUE(0, VIDEO_DISPLAY_CTRL, PIXEL, startPixel));    
}

/*
 *  videoSetGamma
 *      This function enables/disables gamma control.
 *
 *  Input:
 *      enableGammaCtrl - The gamma enable control
 *
 *  NOTE:
 *      The gamma can only be enabled in RGB565 and RGB888. Enable this gamma
 *      without proper format will have no effect.
 */
void videoSetGammaCtrl(
    uint32_t enableGammaCtrl
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
    
    if (enableGammaCtrl)
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, GAMMA, ENABLE);
    else
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, GAMMA, DISABLE);
        
    pokeRegisterDWord(VIDEO_DISPLAY_CTRL, value);    
}

/*
 *  isVideoEnable
 *      This function check whether the video plane is already enabled or not.
 *
 *  Output:
 *      0   - Disable
 *      1   - Enable
 */
unsigned char isVideoEnable()
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
    
    return ((FIELD_GET(value, VIDEO_DISPLAY_CTRL, PLANE) == VIDEO_DISPLAY_CTRL_PLANE_ENABLE) ? 1 : 0);
}

/*
 *  videoSetCtrl
 *      This function enable/disable the video plane.
 *
 *  Input:
 *      videoCtrl   - Enable/Disable video
 */
static void videoSetCtrl(
    video_ctrl_t videoCtrl
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
    
    if (videoCtrl == VIDEO_ON)
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, PLANE, ENABLE);
    else
        value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, PLANE, DISABLE);
                
    pokeRegisterDWord(VIDEO_DISPLAY_CTRL, value); 
}

/*
 *  videoSetFormat
 *      This function sets the video format.
 *
 *  Input:
 *      videoFormat - The video content format
 *                    * FORMAT_8BIT_INDEX - 8-bit Indexed mode
 *                    * FORMAT_RGB565 - 16-bit RGB 5:6:5 mode
 *                    * FORMAT_RGB888 - 32-bit RGB 8:8:8 mode
 *                    * FORMAT_YUYV - 16-bit YUYV mode
 */
static void videoSetFormat(
    video_format_t  videoFormat
)
{
    uint32_t value;
    
    value = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
    switch (videoFormat)
    {
        case FORMAT_8BIT_INDEX:
            value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, FORMAT, 8);
            break;
        default:
        case FORMAT_RGB565:
            value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, FORMAT, 16);
            break;
        case FORMAT_RGB888:
            value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, FORMAT, 32);
            break;
        case FORMAT_YUYV:
            value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, FORMAT, YUV);
            break;
    }
    
    pokeRegisterDWord(VIDEO_DISPLAY_CTRL, value);
}

/*
 *  videoSetup
 *      This function setups the video.
 *
 *  Input:
 *      x               - X Coordinate of the video window
 *      y               - Y Coordinate of the video window
 *      srcWidth        - The source video width
 *      srcHeight       - The source video height
 *      dstWidth        - The destination video width
 *      dstHeight       - The destination video height
 *      doubleBuffer    - Double Buffer enable flag
 *      srcAddress0     - The source of the video buffer 0 to display
 *      srcAddress1     - The source of the video buffer 1 to display
 *                        (only for double buffering).
 *      srcPitch        - The source video plane pitch in bytes
 *      srcLineOffset   - The source video plane line offset in bytes.
 *                        In normal usage, set it the same as the srcBufferPitch
 *      videoFormat     - Source video format
 *
 *  Output:
 *      0   - Success
 *     -1  - Fail
 */
unsigned char videoSetup(
    uint32_t x,                /* X Coordinate of the video window */
    uint32_t y,                /* Y Coordinate of the video window */
    uint32_t srcWidth,         /* The source video width */
    uint32_t srcHeight,        /* The source video height */
    uint32_t dstWidth,         /* The destination video width */
    uint32_t dstHeight,        /* The destination video height */
    uint32_t doubleBuffer,     /* Double Buffer enable flag */
    uint32_t srcAddress0,      /* The source of the video buffer 0 to display */
    uint32_t srcAddress1,      /* The source of the video buffer 1 to display
                                       (only for double buffering).
                                     */
    uint32_t srcPitch,         /* The source video plane pitch in bytes */
    uint32_t srcLineOffset,    /* The source video plane line offset in bytes.
                                       Set it the same as srcPitch in normal
                                       usage. */
    video_format_t videoFormat      /* Source video format */
)
{
    /* Save the source video width and height */
    gSrcVideoWidth = srcWidth;
    gSrcVideoHeight = srcHeight;
    
    /* Disable the video plane first */
    videoSetCtrl(VIDEO_OFF);
    
    /* Set the video position */
    videoSetPosition(x, y);
    
    /* Set the scale factor */
    videoScale(srcWidth, srcHeight, dstWidth, dstHeight);
    
    /* Set the video format */
    videoSetFormat(videoFormat);
    
    /* Set the buffer pitch */
    videoSetPitchOffset(srcPitch, srcLineOffset);
    
    /* Enable double buffer */
    videoEnableDoubleBuffer(doubleBuffer);
    
    /* Set the video buffer 0 and 1 */
    videoSetBuffer(0, srcAddress0);
    videoSetBuffer(1, srcAddress1);
        
    /* Set the destination video window */
    videoSetWindowSize(dstWidth, dstHeight);
    
    return 0;
}

/*
 *  startVideo
 *      This function starts the video.
 */
void startVideo()
{
    /* Enable the video plane */
    videoSetCtrl(VIDEO_ON);
}

/*
 *  stopVideo
 *      This function stops the video.
 */
_X_EXPORT void stopVideo()
{
    /* Just disable the video plane */
    videoSetCtrl(VIDEO_OFF);
}



