/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  Video.H --- Voyager GX SDK 
*  This file contains the definitions for the Video functions.
* 
*******************************************************************/
#ifndef _VIDEO_H_
#define _VIDEO_H_

/* New Structure */

/****************************************************************************
   Structure and data type definition 
 ****************************************************************************/

/* video format:
   - 8-bit indexed mode 
   - 16-bit RGB 5:6:5 mode
   - 32-bit RGB 8:8:8 mode
   - 16-bit YUYV mode
 */
typedef enum _video_format_t
{
    FORMAT_8BIT_INDEX = 0,
    FORMAT_RGB565,
    FORMAT_RGB888,
    FORMAT_YUYV
}
video_format_t;

/* YUV Data Byte Swap */
typedef enum _video_byteswap_t
{
    NORMAL = 0,
    SWAP_BYTE
}
video_byteswap_t;

/* Turn on/off video */
typedef enum _video_sync_source_t
{
    NORMAL_BUFFER = 0,
    CAPTURE_BUFFER
}
video_sync_source_t;

/* FIFO Request Level */
typedef enum _video_fifo_t
{
    FIFO_LEVEL_1 = 0,
    FIFO_LEVEL_3,
    FIFO_LEVEL_7,
    FIFO_LEVEL_11
}
video_fifo_t;

/* Turn on/off video */
typedef enum _video_ctrl_t
{
    VIDEO_OFF = 0,
    VIDEO_ON
}
video_ctrl_t;


/****************************************************************************
   Function prototype 
 ****************************************************************************/

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
);

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
);

/*
 * videoGetCurrentBufferDisplay
 *      This function gets the current buffer used by SM50x to display on the screen
 *
 *  Return:
 *      0   - Buffer 0
 *      1   - Buffer 1 
 */
unsigned char videoGetCurrentBufferDisplay();

/*
 * videoEnableDoubleBuffer
 *      This function enables/disables the double buffer usage
 *
 *  Input:
 *      enable  - Flag to enable/disable the double buffer. 
 */
void videoEnableDoubleBuffer(
    uint32_t enable
);

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
);

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
);

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
);

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
);

/*
 * videoGetPitch
 *      This function gets the video plane pitch
 *
 * Output:
 *      pitch   - Number of bytes per line of the video plane 
 *                specified in 128-bit aligned bytes.
 */
unsigned short videoGetPitch();

/*
 * videoGetLineOffset
 *      This function gets the video plane line offset
 *
 * Output:
 *      lineOffset  - Number of 128-bit aligned bytes per line 
 *                    of the video plane.
 */
unsigned short videoGetLineOffset();

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
);

/*
 *  videoGetWindowSize
 *      This function gets the video window size.
 *
 *  Output:
 *      width       - Video Window width
 *      height      - Video Window height
 */
void videoGetWindowSize(
    uint32_t *pWidth,
    uint32_t *pHeight
);

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
);

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
);

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
);

/*
 *  videoSetFIFOLevel
 *      This function sets the video FIFO Request Level.
 *
 *  Input:
 *      videoSource - Buffer source selection
 */
void videoSetFIFOLevel(
    video_fifo_t videoFIFO
);

/*
 *  videoSetSourceBuffer
 *      This function sets the video to use the capture buffer as the source.
 *
 *  Input:
 *      videoSource - Buffer source selection
 */
void videoSetSourceBuffer(
    video_sync_source_t videoSource
);

/*
 *  videoSwapYUVByte
 *      This function swaps the YUV data byte.
 *
 *  Input:
 *      byteSwap    - Flag to enable/disable YUV data byte swap.
 */
void videoSwapYUVByte(
   video_byteswap_t byteSwap  
);

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
);

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
);

/*
 *  videoSetStartPanningPixel
 *      This function sets the starting pixel number for smooth pixel panning.
 *
 *  Input:
 *      startPixel  - Starting pixel number for smooth pixel panning
 */
void videoSetStartPanningPixel(
    unsigned char startPixel
);

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
);

/*
 *  isVideoEnable
 *      This function check whether the video plane is already enabled or not.
 *
 *  Output:
 *      0   - Disable
 *      1   - Enable
 */
unsigned char isVideoEnable();

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
);

/*
 *  startVideo
 *      This function starts the video.
 */
void startVideo();

/*
 *  stopVideo
 *      This function stops the video.
 */
_X_EXPORT void stopVideo();

#endif /* _VIDEO_H_ */
