/*
 * Voyager GX SDK
 *
 * 2d.c
 *
 * This file contains the source code for the 2D engine.
 *
 */
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "hardware.h"
#include "power.h"
#include "sw2d.h"
#include "2d.h"
#include "os.h"

#include "ddkdebug.h"

/* Static macro */
#define BYTE_PER_PIXEL(bpp)         (bpp / 8)

/*
 * 2D Engine Initialization.
 * This function must be called before other 2D functions.
 * Assumption: A specific video mode has been properly set up.
 */
_X_EXPORT void deInit()
{
    enable2DEngine(1);

    deReset(); /* Just be sure no left-over operations from other applications */

    /* Set up 2D registers that won't change for a specific mode. */

    /* Drawing engine bus and pixel mask, always want to enable. */
    POKE_32(DE_MASKS, 0xFFFFFFFF);

    /* Pixel format, which can be 8, 16 or 32.
       Assuming setmode is call before 2D init, then pixel format
       is available in reg 0x80000 (Panel Display Control)
    */
    POKE_32(DE_STRETCH_FORMAT,
        FIELD_SET  (0, DE_STRETCH_FORMAT, PATTERN_XY,   NORMAL)  |
        FIELD_VALUE(0, DE_STRETCH_FORMAT, PATTERN_Y,    0)       |
        FIELD_VALUE(0, DE_STRETCH_FORMAT, PATTERN_X,    0)       |
        FIELD_SET  (0, DE_STRETCH_FORMAT, ADDRESSING,   XY)      |
        FIELD_VALUE(0, DE_STRETCH_FORMAT, SOURCE_HEIGHT,3));

    /* Clipping and transparent are disable after INIT */
    deSetClipping(0, 0, 0, 0, 0);
    deSetTransparency(0, 0, 0, 0);
}

/*
 * Reset 2D engine by 
 * 1) Aborting the current 2D operation.
 * 2) Re-enable 2D engine to normal state.
 */
_X_EXPORT void deReset()
{
    uint32_t sysCtrl;

    /* Abort current 2D operation */
    sysCtrl = PEEK_32(SYSTEM_CTRL);
    sysCtrl = FIELD_SET(sysCtrl, SYSTEM_CTRL, DE_ABORT, ON);
    POKE_32(SYSTEM_CTRL, sysCtrl);

    /* Re-enable 2D engine to normal state */
    sysCtrl = PEEK_32(SYSTEM_CTRL);
    sysCtrl = FIELD_SET(sysCtrl, SYSTEM_CTRL, DE_ABORT, OFF);
    POKE_32(SYSTEM_CTRL, sysCtrl);
}
 
/*
 * Wait until 2D engine is not busy.
 * All 2D operations are recommand to check 2D engine idle before start.
 *
 * Return: 0 = return because engine is idle and normal.
 *        -1 = return because time out (2D engine may have problem).
 */
_X_EXPORT int32_t deWaitForNotBusy(void)
{
    uint32_t i = 0x1000000;
    while (i--)
    {
        uint32_t dwVal = PEEK_32(SYSTEM_CTRL);
        if ((FIELD_GET(dwVal, SYSTEM_CTRL, DE_STATUS)      == SYSTEM_CTRL_DE_STATUS_IDLE) &&
            (FIELD_GET(dwVal, SYSTEM_CTRL, DE_FIFO)        == SYSTEM_CTRL_DE_FIFO_EMPTY) &&
            (FIELD_GET(dwVal, SYSTEM_CTRL, CSC_STATUS)     == SYSTEM_CTRL_CSC_STATUS_IDLE) &&
            (FIELD_GET(dwVal, SYSTEM_CTRL, DE_MEM_FIFO)    == SYSTEM_CTRL_DE_MEM_FIFO_EMPTY))
        {
            return 0; /* Return because engine idle */
        }
    }

    return -1; /* Return because time out */
}

/* deWaitIdle() function.
 *
 * This function is same as deWaitForNotBusy(), except application can
 * input the maximum number of times that this function will check 
 * the idle register.
 *
 * Its usage is mainly for debugging purpose.
 *
 * Wait until 2D engine is not busy.
 * All 2D operations are recommand to check 2D engine idle before start.
 *
 * Return: 0 = return because engine is idle and normal.
 *        -1 = return because time out (2D engine may have problem).
 */
int32_t deWaitIdle(uint32_t i)
{
    while (i--)
    {
        uint32_t dwVal = PEEK_32(SYSTEM_CTRL);
        if ((FIELD_GET(dwVal, SYSTEM_CTRL, DE_STATUS)      == SYSTEM_CTRL_DE_STATUS_IDLE) &&
            (FIELD_GET(dwVal, SYSTEM_CTRL, DE_FIFO)        == SYSTEM_CTRL_DE_FIFO_EMPTY) &&
            (FIELD_GET(dwVal, SYSTEM_CTRL, CSC_STATUS)     == SYSTEM_CTRL_CSC_STATUS_IDLE) &&
            (FIELD_GET(dwVal, SYSTEM_CTRL, DE_MEM_FIFO)    == SYSTEM_CTRL_DE_MEM_FIFO_EMPTY))
        {
            return 0; /* Return because engine idle */
        }
    }

    return -1; /* Return because time out */
}

/*
 * This function enable/disable clipping area for the 2d engine.
 * Note that the clipping area is always rectangular.
 * 
 */
_X_EXPORT int32_t deSetClipping(
uint32_t enable, /* 0 = disable clipping, 1 = enable clipping */
uint32_t x1,     /* x1, y1 is the upper left corner of the clipping area */
uint32_t y1,     /* Note that the region includes x1 and y1 */
uint32_t x2,     /* x2, y2 is the lower right corner of the clippiing area */
uint32_t y2)     /* Note that the region will not include x2 and y2 */
{
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }

    /* Upper left corner and enable/disable bit
       Note: This module defautls to clip outside region.
       "Clip inside" is not a useful feature since nothing gets drawn.
     */
    POKE_32(DE_CLIP_TL,
        FIELD_VALUE(0, DE_CLIP_TL, TOP, y1) |
        ((enable)?
          FIELD_SET(0, DE_CLIP_TL, STATUS, ENABLE)
        : FIELD_SET(0, DE_CLIP_TL, STATUS, DISABLE))|
        FIELD_SET  (0, DE_CLIP_TL, INHIBIT,OUTSIDE) |
        FIELD_VALUE(0, DE_CLIP_TL, LEFT, x1));

    /* Lower right corner */
    POKE_32(DE_CLIP_BR,
        FIELD_VALUE(0, DE_CLIP_BR, BOTTOM,y2) |
        FIELD_VALUE(0, DE_CLIP_BR, RIGHT, x2));

    return 0;
}

/* 
 * Function description:
 * When transparency is enable, the blt engine compares each pixel value 
 * (either source or destination) with DE_COLOR_COMPARE register.
 * If match, the destination pixel will NOT be updated.
 * If not match, the destination pixel will be updated.
 */
_X_EXPORT int32_t deSetTransparency(
uint32_t enable,     /* 0 = disable, 1 = enable transparency feature */
uint32_t tSelect,    /* 0 = compare source, 1 = compare destination */
uint32_t tMatch,     /* 0 = Opaque mode, 1 = transparent mode */
uint32_t ulColor)    /* Color to compare. */
{
    uint32_t de_ctrl;

    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }

    /* Set mask */
    if (enable)
    {
        POKE_32(DE_COLOR_COMPARE_MASK, 0x00ffffff);

        /* Set compare color */
        POKE_32(DE_COLOR_COMPARE, ulColor);
    }
    else
    {
        POKE_32(DE_COLOR_COMPARE_MASK, 0x0);
        POKE_32(DE_COLOR_COMPARE, 0x0);
    }

    /* Set up transparency control, without affecting other bits
       Note: There are two operatiing modes: Transparent and Opague.
       We only use transparent mode because Opaque mode may have bug.
    */
    de_ctrl = PEEK_32(DE_CONTROL)
              & FIELD_CLEAR(DE_CONTROL, TRANSPARENCY)
              & FIELD_CLEAR(DE_CONTROL, TRANSPARENCY_MATCH)
              & FIELD_CLEAR(DE_CONTROL, TRANSPARENCY_SELECT);

    /* For DE_CONTROL_TRANSPARENCY_MATCH bit, always set it
       to TRANSPARENT mode, OPAQUE mode don't seem working.
    */
    de_ctrl |=
    ((enable)?
      FIELD_SET(0, DE_CONTROL, TRANSPARENCY, ENABLE)
    : FIELD_SET(0, DE_CONTROL, TRANSPARENCY, DISABLE))        |
    ((tMatch)?
      FIELD_SET(0, DE_CONTROL, TRANSPARENCY_MATCH, TRANSPARENT)
    : FIELD_SET(0, DE_CONTROL, TRANSPARENCY_MATCH, OPAQUE)) |
    ((tSelect)?
      FIELD_SET(0, DE_CONTROL, TRANSPARENCY_SELECT, DESTINATION)
    : FIELD_SET(0, DE_CONTROL, TRANSPARENCY_SELECT, SOURCE));

    POKE_32(DE_CONTROL, de_ctrl);

    return 0;
}

/*
 * This function gets the transparency status from DE_CONTROL register.
 * It returns a double word with the transparent fields properly set,
 * while other fields are 0.
 */
_X_EXPORT uint32_t deGetTransparency()
{
    uint32_t de_ctrl;

    de_ctrl = PEEK_32(DE_CONTROL);

    de_ctrl &= 
        FIELD_MASK(DE_CONTROL_TRANSPARENCY_MATCH) | 
        FIELD_MASK(DE_CONTROL_TRANSPARENCY_SELECT)| 
        FIELD_MASK(DE_CONTROL_TRANSPARENCY);

    return de_ctrl;
}

/*
 * This function sets the pixel format that will apply to the 2D Engine.
 */
_X_EXPORT void deSetPixelFormat(
    uint32_t bpp
)
{
    uint32_t de_format;
    
    de_format = PEEK_32(DE_STRETCH_FORMAT);
    
    switch (bpp)
    {
        case 8:
            de_format = FIELD_SET(de_format, DE_STRETCH_FORMAT, PIXEL_FORMAT, 8);
            break;
        default:
        case 16:
            de_format = FIELD_SET(de_format, DE_STRETCH_FORMAT, PIXEL_FORMAT, 16);
            break;
        case 32:
            de_format = FIELD_SET(de_format, DE_STRETCH_FORMAT, PIXEL_FORMAT, 32);
            break;
    }
    
    POKE_32(DE_STRETCH_FORMAT, de_format);
}

/*
 * This function uses 2D engine to fill a rectangular area with a specific color.
 * The filled area includes the starting points.
 */
int32_t deRectFill( /*resolution_t resolution, point_t p0, point_t p1, uint32_t color, uint32_t rop2)*/
uint32_t dBase,  /* Base address of destination surface counted from beginning of video frame buffer */
uint32_t dPtich, /* Pitch value of destination surface in BYTES */
uint32_t bpp,    /* Color depth of destination surface: 8, 16 or 32 */
uint32_t x,
uint32_t y,      /* Upper left corner (X, Y) of rectangle in pixel value */
uint32_t width, 
uint32_t height, /* width and height of rectange in pixel value */
uint32_t color,  /* Color to be filled */
uint32_t rop2)   /* ROP value */
{
    uint32_t de_ctrl, bytePerPixel;

    bytePerPixel = bpp/8;
    
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }

    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);

    /* Program pitch (distance between the 1st points of two adjacent lines).
       Note that input pitch is BYTE value, but the 2D Pitch register uses
       pixel values. Need Byte to pixel convertion.
    */
    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, (dPtich/bytePerPixel)) |
        FIELD_VALUE(0, DE_PITCH, SOURCE,      (dPtich/bytePerPixel)));

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (dPtich/bytePerPixel)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      (dPtich/bytePerPixel)));

    POKE_32(DE_FOREGROUND, color);

    POKE_32(DE_DESTINATION,
        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_DESTINATION, X,    x)       |
        FIELD_VALUE(0, DE_DESTINATION, Y,    y));

    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    width) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));
        
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);

    de_ctrl = 
        FIELD_SET  (0, DE_CONTROL,  STATUS,     START)          |
        FIELD_SET  (0, DE_CONTROL,  DIRECTION,  LEFT_TO_RIGHT)  |
        //FIELD_SET  (0, DE_CONTROL,LAST_PIXEL, OFF)            |
        FIELD_SET  (0, DE_CONTROL,  COMMAND,    RECTANGLE_FILL) |
        FIELD_SET  (0, DE_CONTROL,  ROP_SELECT, ROP2)           |
        FIELD_VALUE(0, DE_CONTROL,  ROP,        rop2);

    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

    return 0;
}

/*
 * Video Memory to Video Memory data transfer.
 * Note: 
 *        It works whether the Video Memroy is off-screeen or on-screen.
 *        This function is a one to one transfer without stretching or 
 *        mono expansion.
 */
int32_t deVideoMem2VideoMemBlt(
uint32_t sBase,  /* Address of source: offset in frame buffer */
uint32_t sPitch, /* Pitch value of source surface in BYTE */
uint32_t sx,
uint32_t sy,     /* Starting coordinate of source surface */
uint32_t dBase,  /* Address of destination: offset in frame buffer */
uint32_t dPitch, /* Pitch value of destination surface in BYTE */
uint32_t bpp,    /* Color depth of destination surface */
uint32_t dx,
uint32_t dy,     /* Starting coordinate of destination surface */
uint32_t width, 
uint32_t height, /* width and height of rectangle in pixel value */
uint32_t rop2)   /* ROP value */
{
    uint32_t nDirection, de_ctrl, bytePerPixel;
    int32_t opSign;

    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }

    nDirection = LEFT_TO_RIGHT;
    opSign = 1;    /* Direction of ROP2 operation: 1 = Left to Right, (-1) = Right to Left */
    bytePerPixel = bpp/8;
    de_ctrl = 0;

    /* If source and destination are the same surface, need to check for overlay cases */
    if (sBase == dBase && sPitch == dPitch)
    {
        /* Determine direction of operation */
        if (sy < dy)
        {
            /* +----------+
               |S         |
               |   +----------+
               |   |      |   |
               |   |      |   |
               +---|------+   |
                   |         D|
                   +----------+ */
    
            nDirection = BOTTOM_TO_TOP;
        }
        else if (sy > dy)
        {
            /* +----------+
               |D         |
               |   +----------+
               |   |      |   |
               |   |      |   |
               +---|------+   |
                   |         S|
                   +----------+ */
    
            nDirection = TOP_TO_BOTTOM;
        }
        else
        {
            /* sy == dy */
    
            if (sx <= dx)
            {
                /* +------+---+------+
                   |S     |   |     D|
                   |      |   |      |
                   |      |   |      |
                   |      |   |      |
                   +------+---+------+ */
    
                nDirection = RIGHT_TO_LEFT;
            }
            else
            {
                /* sx > dx */
    
                /* +------+---+------+
                   |D     |   |     S|
                   |      |   |      |
                   |      |   |      |
                   |      |   |      |
                   +------+---+------+ */
    
                nDirection = LEFT_TO_RIGHT;
            }
        }
    }

    if ((nDirection == BOTTOM_TO_TOP) || (nDirection == RIGHT_TO_LEFT))
    {
        sx += width - 1;
        sy += height - 1;
        dx += width - 1;
        dy += height - 1;
        opSign = (-1);
    }

    /* Note:
       DE_FOREGROUND are DE_BACKGROUND are don't care.
       DE_COLOR_COMPARE and DE_COLOR_COMPARE_MAKS are set by set deSetTransparency().
    */

    /* 2D Source Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_SOURCE_BASE, sBase);

    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);

    /* Program pitch (distance between the 1st points of two adjacent lines).
       Note that input pitch is BYTE value, but the 2D Pitch register uses
       pixel values. Need Byte to pixel convertion.
    */
    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_PITCH, SOURCE,      (sPitch/bytePerPixel)));

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      (sPitch/bytePerPixel)));

    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);
    
#if 1
    /* This bug is fixed in SM718 for 16 and 32 bpp. However, in 8-bpp, the problem still exists. 
       The Version AA also have this problem on higher clock with 32-bit memory data bus, 
       therefore, it needs to be enabled here. 
       In version AA, the problem happens on the following configurations:
        1. M2XCLK = 336MHz w/ 32-bit, MCLK = 112MHz, and color depth set to 32bpp
        2. M2XCLK = 336MHz w/ 32-bit, MCLK = 84MHz, and color depth set to 16bpp or 32bpp.
       Somehow, the problem does not appears in 64-bit memory setting.
     */

    /* Workaround for 192 byte requirement when ROP is not COPY */
    if (
#ifdef VALIDATION_CHIP
        (bpp == 8) && 
#endif
        (rop2 != ROP2_COPY) && ((width * bytePerPixel) > 192))
    {
        /* Perform the ROP2 operation in chunks of (xWidth * nHeight) */
        uint32_t xChunk = 192 / bytePerPixel; /* chunk width is in pixels */
        
        DDKDEBUGPRINT((DE_LEVEL, "ROP != ROP_COPY, width * bytePerPixel = %x (> 192 bytes)\n", width * bytePerPixel));

        while (1)
        {
            deWaitForNotBusy();
            POKE_32(DE_SOURCE,
                FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_SOURCE, X_K1, sx)   |
                FIELD_VALUE(0, DE_SOURCE, Y_K2, sy));
            POKE_32(DE_DESTINATION,
                FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_DESTINATION, X,    dx)  |
                FIELD_VALUE(0, DE_DESTINATION, Y,    dy));
            POKE_32(DE_DIMENSION,
                FIELD_VALUE(0, DE_DIMENSION, X,    xChunk) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));

            de_ctrl = 
                FIELD_VALUE(0, DE_CONTROL, ROP, rop2) |
                FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
                FIELD_SET(0, DE_CONTROL, COMMAND, BITBLT) |
                ((nDirection == RIGHT_TO_LEFT) ? 
                FIELD_SET(0, DE_CONTROL, DIRECTION, RIGHT_TO_LEFT)
                : FIELD_SET(0, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT)) |
                FIELD_SET(0, DE_CONTROL, STATUS, START);

            POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

            if (xChunk == width) break;

            sx += (opSign * xChunk);
            dx += (opSign * xChunk);
            width -= xChunk;

            if (xChunk > width)
            {
                /* This is the last chunk. */
                xChunk = width;
            }
        }
    }
    else
#endif
    {
        deWaitForNotBusy();

        POKE_32(DE_SOURCE,
            FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
            FIELD_VALUE(0, DE_SOURCE, X_K1, sx)   |
            FIELD_VALUE(0, DE_SOURCE, Y_K2, sy));
        POKE_32(DE_DESTINATION,
            FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
            FIELD_VALUE(0, DE_DESTINATION, X,    dx)  |
            FIELD_VALUE(0, DE_DESTINATION, Y,    dy));
        POKE_32(DE_DIMENSION,
            FIELD_VALUE(0, DE_DIMENSION, X,    width) |
            FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));

        de_ctrl = 
            FIELD_VALUE(0, DE_CONTROL, ROP, rop2) |
            FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
            FIELD_SET(0, DE_CONTROL, COMMAND, BITBLT) |
            ((nDirection == RIGHT_TO_LEFT) ? 
            FIELD_SET(0, DE_CONTROL, DIRECTION, RIGHT_TO_LEFT)
            : FIELD_SET(0, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT)) |
            FIELD_SET(0, DE_CONTROL, STATUS, START);

        POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());
    }

    return 0;
}

/* 
 * System memory to Video memory data transfer
 * Note: 
 *         We also call it HOST Blt.
 *        This function is a one to one transfer without stretching or 
 *        mono expansion.
 */
_X_EXPORT int32_t deSystemMem2VideoMemBlt(
unsigned char *pSrcbuf, /* pointer to source data in system memory */
int32_t srcDelta,          /* width (in Bytes) of the source data, +ive means top down and -ive mean button up */
uint32_t dBase,    /* Address of destination: offset in frame buffer */
uint32_t dPitch,   /* Pitch value of destination surface in BYTE */
uint32_t bpp,      /* Color depth of destination surface */
uint32_t dx,
uint32_t dy,       /* Starting coordinate of destination surface */
uint32_t width, 
uint32_t height,   /* width and height of rectange in pixel value */
uint32_t rop2)     /* ROP value */
{
    uint32_t bytePerPixel;
    uint32_t ulBytesPerScan;
    uint32_t ul8BytesPerScan;
    uint32_t ulBytesRemain;
    uint32_t de_ctrl = 0;
    unsigned char ajRemain[8];
    int32_t i, j;

    bytePerPixel = bpp/8;

    /* HOST blt data port must take multiple of 8 bytes as input.
       If the source width does not match that requirement,
       we need to split it into two portions. The first portion
       is 8 byte multiple. The 2nd portion is the remaining bytes.
       The remaining bytes will be buffered to an 8 byte array and
       and send it to the host blt data port.
    */
    ulBytesPerScan = width * bpp / 8;
    ul8BytesPerScan = ulBytesPerScan & ~7;
    ulBytesRemain = ulBytesPerScan & 7;

    /* Program 2D Drawing Engine */
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }

    /* 2D Source Base.
       Use 0 for HOST Blt.
    */
    POKE_32(DE_WINDOW_SOURCE_BASE, 0);

    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);

    /* Program pitch (distance between the 1st points of two adjacent lines).
       Note that input pitch is BYTE value, but the 2D Pitch register uses
       pixel values. Need Byte to pixel convertion.
    */
    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, dPitch/bytePerPixel) |
        FIELD_VALUE(0, DE_PITCH, SOURCE,      dPitch/bytePerPixel));

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      (dPitch/bytePerPixel)));

    /* Note: For 2D Source in Host Write, only X_K1 field is needed, and Y_K2 field is not used.
             For 1 to 1 bitmap transfer, use 0 for X_K1 means source alignment from byte 0. */
    POKE_32(DE_SOURCE,
        FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_SOURCE, X_K1, 0)       |
        FIELD_VALUE(0, DE_SOURCE, Y_K2, 0));

    POKE_32(DE_DESTINATION,
        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_DESTINATION, X,    dx)    |
        FIELD_VALUE(0, DE_DESTINATION, Y,    dy));

    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    width) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));
        
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);

    de_ctrl = 
        FIELD_VALUE(0, DE_CONTROL, ROP, rop2)         |
        FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2)    |
        FIELD_SET(0, DE_CONTROL, COMMAND, HOST_WRITE) |
        FIELD_SET(0, DE_CONTROL, HOST, COLOR)         |
        FIELD_SET(0, DE_CONTROL, STATUS, START);

    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

    /* Write bitmap/image data (line by line) to 2D Engine data port */
    for (i = 0; i < height; i++)
    {
        /* For each line, send the data in chunks of 4 bytes. */
        for (j=0; j < (ul8BytesPerScan/4);  j++)
            POKE_32(DE_DATA_PORT, *(uint32_t *)(pSrcbuf + (j * 4)));

        if (ulBytesRemain)
        {
            memcpy(ajRemain, pSrcbuf+ul8BytesPerScan, ulBytesRemain);
            POKE_32(DE_DATA_PORT, *(uint32_t *)ajRemain);
            POKE_32(DE_DATA_PORT, *(uint32_t *)(ajRemain+4));
        }

        pSrcbuf += srcDelta;
    }

    return 0;
}

/*
 * System memory to Video memory monochrome expansion.
 * Source is monochrome image in system memory.
 * This function expands the monochrome data to color image in video memory.
 */
int32_t deSystemMem2VideoMemMonoBlt(
unsigned char *pSrcbuf, /* pointer to start of source buffer in system memory */
int32_t srcDelta,          /* Pitch value (in bytes) of the source buffer, +ive means top down and -ive mean button up */
uint32_t startBit, /* Mono data can start at any bit in a byte, this value should be 0 to 7 */
uint32_t dBase,    /* Address of destination: offset in frame buffer */
uint32_t dPitch,   /* Pitch value of destination surface in BYTE */
uint32_t bpp,      /* Color depth of destination surface */
uint32_t dx,
uint32_t dy,       /* Starting coordinate of destination surface */
uint32_t width, 
uint32_t height,   /* width and height of rectange in pixel value */
uint32_t fColor,   /* Foreground color (corresponding to a 1 in the monochrome data */
uint32_t bColor,   /* Background color (corresponding to a 0 in the monochrome data */
uint32_t rop2)     /* ROP value */
{
    uint32_t bytePerPixel;
    uint32_t ulBytesPerScan;
    uint32_t ul4BytesPerScan;
    uint32_t ulBytesRemain;
    uint32_t de_ctrl = 0;
    unsigned char ajRemain[4];
    int32_t i, j;

    bytePerPixel = bpp/8;

    startBit &= 7; /* Just make sure the start bit is within legal range */
    ulBytesPerScan = (width + startBit + 7) / 8;
    ul4BytesPerScan = ulBytesPerScan & ~3;
    ulBytesRemain = ulBytesPerScan & 3;

    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }

    /* 2D Source Base.
       Use 0 for HOST Blt.
    */
    POKE_32(DE_WINDOW_SOURCE_BASE, 0);

    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);

    /* Program pitch (distance between the 1st points of two adjacent lines).
       Note that input pitch is BYTE value, but the 2D Pitch register uses
       pixel values. Need Byte to pixel convertion.
    */
    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, dPitch/bytePerPixel) |
        FIELD_VALUE(0, DE_PITCH, SOURCE,      dPitch/bytePerPixel));

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      (dPitch/bytePerPixel)));

    /* Note: For 2D Source in Host Write, only X_K1_MONO field is needed, and Y_K2 field is not used.
             For mono bitmap, use startBit for X_K1. */
    POKE_32(DE_SOURCE,
        FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE)       |
        FIELD_VALUE(0, DE_SOURCE, X_K1_MONO, startBit));

    POKE_32(DE_DESTINATION,
        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_DESTINATION, X,    dx)    |
        FIELD_VALUE(0, DE_DESTINATION, Y,    dy));

    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    width) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));

    POKE_32(DE_FOREGROUND, fColor);
    POKE_32(DE_BACKGROUND, bColor);
    
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);

    de_ctrl = FIELD_VALUE(0, DE_CONTROL, ROP, rop2)         |
              FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2)    |
              FIELD_SET(0, DE_CONTROL, COMMAND, HOST_WRITE) |
              FIELD_SET(0, DE_CONTROL, HOST, MONO)          |
              FIELD_SET(0, DE_CONTROL, STATUS, START);

    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

    /* Write MONO data (line by line) to 2D Engine data port */
    for (i=0; i<height; i++)
    {
        /* For each line, send the data in chunks of 4 bytes */
        for (j=0; j<(ul4BytesPerScan/4); j++)
        {
            POKE_32(DE_DATA_PORT, *(uint32_t *)(pSrcbuf + (j * 4)));
        }

        if (ulBytesRemain)
        {
            memcpy(ajRemain, pSrcbuf+ul4BytesPerScan, ulBytesRemain);
            POKE_32(DE_DATA_PORT, *(uint32_t *)ajRemain);
        }

        pSrcbuf += srcDelta;
    }

    return 0;
}

/*
 * This function cache font table into frame buffer.
 *
 * Inputs:
 *      Pointer to font table in system memory.
 *      Charcter size in byte: How many bytes are used to store the font for one characer.
 *      As an example: For 8x8 mono font, the size is 8 bytes; 
 *                     For 8x16 mono font, the size is 16 bytes.
 *                     For 16x32 mono font, the size is 64 bytes.
 *      Number of characters in the font table.
 *      Pointer to location of frame buffer to store the font: This is an offset from the beginning of frame buffer.
 *  
 * Rules for storing fonts in off-screen.
 *     1) Base address of font table must be 16 byte (or 128 bit) aligned.
 *     2) Each font character must be stored in a 16 byte (128 bit) aligned
 *        location.
 *
 */
int32_t deCacheFontToFrameBuffer(
unsigned char *fontTable,   /* Pointer to font table in system memory */
uint32_t sizeOfChar,   /* How many bytes for one monochrome character */
uint32_t numberOfChar, /* Number of characters in the font table */
uint32_t fbAddr)       /* Destination in Video memory to store the font */
{
    int32_t i, j;
    uint32_t fontPitch;

    /*
     * Check if frame buffer address is 16 bytes aligned.
     */
    if (fbAddr & 15) return -1;

    /*
     * Each font character must occupy a minimum of 16 bytes.
     * For sizeOfChar less than 16, insert extra bytes in each character.
     */
    fontPitch = (sizeOfChar > 16)? sizeOfChar : 16; /* byte value */

    for (i=0; i< numberOfChar; i++)
    {
        for(j=0; j< sizeOfChar; j++)
            pokeByte(fbAddr+j, fontTable[i*sizeOfChar + j]);

        fbAddr += fontPitch;
    }
    
    return 0;
}

/*
 * Video memory to Video memory monochrome expansion.
 * Source is the starting location of monochrome image in Video memory.
 * This function expands the monochrome data to color image.
 *
 * Note:
 * This fnnction can be used to diaplay a mono-font charater to the screen.
 * Input source points to the starting location of the font character.
 */
int32_t deVideoMem2VideoMemMonoBlt(
uint32_t sBase,  /* Address of mono-chrome source data in frame buffer */
uint32_t dBase,  /* Base address of destination in frame buffer */
uint32_t dPitch, /* Pitch value of destination surface in BYTE */
uint32_t bpp,    /* Color depth of destination surface */
uint32_t dx,
uint32_t dy,     /* Starting coordinate of destination surface */
uint32_t width,  /* width of mono-chrome picture in pixel value */
uint32_t height, /* height of mono-chrome picture in pixel value */
uint32_t fColor, /* Foreground color (corresponding to a 1 in the monochrome data */
uint32_t bColor, /* Background color (corresponding to a 0 in the monochrome data */
uint32_t rop2)   /* ROP value */
{
    uint32_t bytePerPixel, de_ctrl, packed;

    bytePerPixel = bpp/8;

    switch ( width )
    {
        case 8:
            packed = DE_CONTROL_MONO_DATA_8_PACKED;
            break;
        case 16:
            packed = DE_CONTROL_MONO_DATA_16_PACKED;
            break;
        case 32:
            packed = DE_CONTROL_MONO_DATA_32_PACKED;
            break;
        default:
            packed = DE_CONTROL_MONO_DATA_NOT_PACKED;
    }

    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }

    /* Note:
       DE_COLOR_COMPARE and DE_COLOR_COMPARE_MAKS are set by set deSetTransparency().
    */

    /* Monochrome source data in frame buffer.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_SOURCE_BASE, sBase);

    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);

    /* Program pitch (distance between the 1st points of two adjacent lines).
       Note that input pitch is BYTE value, but the 2D Pitch register uses
       pixel values. Need Byte to pixel convertion.
    */
    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_PITCH, SOURCE,      0));      /* Source pitch is don't care */

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      0 )); /* Source window width is don't care */

    POKE_32(DE_SOURCE,
        FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_SOURCE, X_K1, 0)   |
        FIELD_VALUE(0, DE_SOURCE, Y_K2, 0));    /* Source data starts at location (0, 0) */

    POKE_32(DE_DESTINATION,
        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_DESTINATION, X,    dx)  |
        FIELD_VALUE(0, DE_DESTINATION, Y,    dy));

    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    width) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));

    POKE_32(DE_FOREGROUND, fColor);
    POKE_32(DE_BACKGROUND, bColor);
    POKE_32(DE_COLOR_COMPARE, bColor);
    
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);

    de_ctrl = 
        FIELD_VALUE(0, DE_CONTROL, ROP, rop2) |
        FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
        FIELD_SET(0, DE_CONTROL, COMMAND, FONT) |
        FIELD_SET(0, DE_CONTROL, HOST, MONO) |
        FIELD_VALUE(0, DE_CONTROL, MONO_DATA, packed) |
        FIELD_SET(0, DE_CONTROL, STATUS, START);

    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

    return 0;
}

/*
 * Video memory to Video memory monochrome expansion.
 * 
 * Difference between this function and deVideoMem2VideoMemMonoBlt():
 * 1) Input is base address of the whole font table.
 * 2) An extra input about which character in the font table to display.
 * 3) This function demos how to use registers DE_SOURCE and
 *    DE_WINDOW_WIDTH, where they are set to 0 in deVideoMem2VideoMemMonoBlt().
 */
int32_t deFontCacheTblMonoBlt(
uint32_t fontTblBase,/* Base address of monochrome font table in frame buffer */
uint32_t fontNumber, /* Which character in the font table, starting from 0 */
uint32_t dBase,      /* Base address of destination in frame buffer */
uint32_t dPitch,     /* Pitch value of destination surface in BYTE */
uint32_t bpp,        /* Color depth of destination surface */
uint32_t dx,
uint32_t dy,         /* Starting coordinate of destination surface */
uint32_t width,      /* width of each monochrome font in pixel value */
uint32_t height,     /* height of each monochrome font in pixel value */
uint32_t fColor,     /* Foreground color (corresponding to a 1 in the monochrome data */
uint32_t bColor,     /* Background color (corresponding to a 0 in the monochrome data */
uint32_t rop2)       /* ROP value */
{
    uint32_t bytePerPixel, distBetweenFont, de_ctrl, packed;

    bytePerPixel = bpp/8;

    /* Distance between fonts in pixel value */
    distBetweenFont = width * height;

    /* SM50x hardware requires each font character be a minimum of 128 
       pixel value apart.
       For 8x8 fonts, each font character is only 64 pixel apart.
       However, if application uses deCacheFontToFrameBuffer()
       to cache the font in video memory, each font character will be 
       stored as 128 pixels apart.
    */
    if (distBetweenFont < 128)
        distBetweenFont = 128;

    switch ( width )
    {
        case 8:
            packed = DE_CONTROL_MONO_DATA_8_PACKED;
            break;
        case 16:
            packed = DE_CONTROL_MONO_DATA_16_PACKED;
            break;
        case 32:
            packed = DE_CONTROL_MONO_DATA_32_PACKED;
            break;
        default:
            packed = DE_CONTROL_MONO_DATA_NOT_PACKED;
    }

    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }

    /* Note:
       DE_COLOR_COMPARE and DE_COLOR_COMPARE_MAKS are set by set deSetTransparency().
    */

    /* Base address of font table in frame buffer */
    POKE_32(DE_WINDOW_SOURCE_BASE, fontTblBase);

    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);

    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_PITCH, SOURCE,      0));      /* Source pitch is don't care */

    /* Pay attention how DE_WINDOW_WIDTH:SOURCE is different from
       deVideoMem2VideoMemMonoBlt()
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      distBetweenFont));

    /* Pay attention how DE_SOURCE:Y_K2 is different from 
       deVideoMem2VideoMemMonoBlt()
    */
    POKE_32(DE_SOURCE,
        FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_SOURCE, X_K1, 0)   |
        FIELD_VALUE(0, DE_SOURCE, Y_K2, fontNumber));

    POKE_32(DE_DESTINATION,
        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_DESTINATION, X,    dx)  |
        FIELD_VALUE(0, DE_DESTINATION, Y,    dy));

    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    width) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));

    POKE_32(DE_FOREGROUND, fColor);
    POKE_32(DE_BACKGROUND, bColor);
    POKE_32(DE_COLOR_COMPARE, bColor);
    
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);

    de_ctrl = 
        FIELD_VALUE(0, DE_CONTROL, ROP, rop2) |
        FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
        FIELD_SET(0, DE_CONTROL, COMMAND, FONT) |
        FIELD_SET(0, DE_CONTROL, HOST, MONO) |
        FIELD_VALUE(0, DE_CONTROL, MONO_DATA, packed) |
        FIELD_SET(0, DE_CONTROL, STATUS, START);

    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

    return 0;
}

/*
 * Stretch Blt.
 * 
 * The stretch blt is done by using the CSC engine.
 */
int32_t deStretchBlt(
    uint32_t sBase,    /* Source Base address */
    uint32_t sPitch,   /* Source pitch value in bytes */
    uint32_t sbpp,     /* Source bits per pixel */
    uint32_t sx,       /* Source x coordinate */            
    uint32_t sy,       /* Source y coordinate */
    uint32_t sWidth,   /* Width of source in pixel */
    uint32_t sHeight,  /* Height of source in lines */
    uint32_t dBase,    /* Destination base address */
    uint32_t dPitch,   /* Destination pitch value in bytes */
    uint32_t dbpp,     /* Destination bits per pixel */
    uint32_t dx,       /* Destination x coordinate */
    uint32_t dy,       /* Destination y coordinate */
    uint32_t dWidth,   /* Width of the destination display */
    uint32_t dHeight   /* Height of the destination display */
)
{
    /* Start converting the source to the destination */
    cscConvert(
        sx,                 /* Integer part of X coordinate in Y plane */            
        0,                  /* Fraction part of X coordinate in Y plane 
                               (not used in RGB Space) */
        sy,                 /* Integer part of Y coordinate in Y plane */
        0,                  /* Fraction part of Y coordinate in Y plane 
                               (not used in RGB Space)*/
        sWidth,             /* Width of source in pixel */
        sHeight,            /* Height of source in lines */
        sBase,              /* Y Source Base address or the RGB Base address */
        0,                  /* U Source Base Address (not used in RGB Space) */
        0,                  /* V Source Base Address (not used in RGB Space) */
        sPitch,             /* Y plane pitch value in bytes */
        0,                  /* UV plane pitch value in bytes (not used in 
                               RGB Space) */
        (sbpp == 32) ? SRC_RGBx888 : SRC_RGB565,    /* Source pixel format */
        YUYV_UVUV,          /* Byte order for YUV422 and YUV420I only 
                               (not used in RGB Space) */
        dx,                 /* X Coordinate of the destination */
        dy,                 /* Y Coordinate of the destination */
        dWidth,             /* Width of the destination display */
        dHeight,            /* Height of the destination display */
        dBase,              /* Destination base address */
        dPitch,             /* Horizontal pitch of destination in bytes */
        0,                  /* Vertical pitch of the destination in pixels */
        CSC_WRAP_DISABLE,   /* Wrap around control */
        (dbpp == 32) ? DST_RGBx888 : DST_RGB565,    /* Destination pixel format */
        CSC_FILTER_DISABLE, /* CSC Horizontal Linear Filter Enable */
        CSC_FILTER_DISABLE, /* CSC Vertical Linear Filter Enable */
        0,                  /* Y Adjustment */
        0,                  /* Red Conversion constant */
        0,                  /* Green Conversion constant */
        0                   /* Blue Conversion constant */
    );
        
    return 0;
}

/*
 * Rotation helper function.
 * 
 * This function sets the source coordinate, destination coordinate, window
 * dimension, and also the control register. This function is only used statically
 * to simplify the deRotateBlt function.
 *
 */
void deRotate(
    uint32_t sx,               /* X Coordinate of the source */
    uint32_t sy,               /* Y Coordinate of the source */
    uint32_t dx,               /* X Coordinate of the destination */
    uint32_t dy,               /* Y Coordinate of the destination */
    uint32_t width,            /* Width of the window */
    uint32_t height,           /* Height of the window */
    uint32_t de_ctrl           /* DE_CONTROL Control Value */
)
{
    /* Wait until the engine is not busy */
    deWaitForNotBusy();
                
    /* Set the source coordinate */
    POKE_32(DE_SOURCE,
        FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_SOURCE, X_K1, sx) | 
        FIELD_VALUE(0, DE_SOURCE, Y_K2, sy));
        
    /* Set the destination coordinate */    
    POKE_32(DE_DESTINATION,
        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_DESTINATION, X, dx) | 
        FIELD_VALUE(0, DE_DESTINATION, Y, dy));

    /* Set the source width and height dimension */
    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X, width) | 
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));
        
    /* Start the DE Control */
    POKE_32(DE_CONTROL, de_ctrl);
}

/*
 * Rotation Blt.
 * 
 * This function rotates an image to the screen based on the given rotation direction
 * (0, 90, 180, or 270 degree).
 * 
 * NOTE:
 *      1. The function takes care of any coordinates that pass the top and left
 *         boundary, but it does not take care of right and bottom boundary since
 *         more information is needed in order to do so. However, it seems that
 *         SM50x will take care the right and bottom by using wrap around.
 *         Depending on the implementation, this function might be modified further.
 *      2. There are 3 hardware bugs found on the rotation:
 *         a. The rotated image sometimes is not correct when using some width number
 *            due to FIFO bug. Therefore, the image is divided into segments and
 *            rotated based on segment by segment using 32/byte per pixel as the width.
 *            This value (32/byte per pixel) seems to be consistent in producing a good 
 *            rotated image.
 *         b. Rotating 0 degree using the actual Rotation BLT will have the same result
 *            as rotation 270 degree.
 *         c. Rotating 180 degree on some x destination coordinate will result in 
 *            incorrect image. To workaround this problem, two 90 degree rotations are
 *            used.
 *      3. The rop parameter might not be necessary if only one ROP operation is used.
 *         This might be deleted in the future as necessary.
 */
int32_t deVideoMem2VideoMemRotateBlt(
    uint32_t sBase,            /* Source Base Address */
    uint32_t sPitch,           /* Source pitch */
    uint32_t sx,               /* X Coordinate of the source */
    uint32_t sy,               /* Y Coordinate of the source */
    uint32_t dBase,            /* Destination Base Address */
    uint32_t dPitch,           /* Destination pitch */
    uint32_t bpp,              /* Color depth of destination surface */
    uint32_t dx,               /* X Coordinate of the destination */ 
    uint32_t dy,               /* Y Coordinate of the destination */
    uint32_t width,            /* Width  of un-rotated image in pixel value */
    uint32_t height,           /* Height of un-rotated image in pixel value */
    rotate_dir_t rotateDirection,   /* Direction of the rotation */
    uint32_t repeatEnable,     /* Enable repeat rotation control where the
                                       drawing engine is started again every vsync */
    uint32_t rop2              /* ROP control */
)
{
    uint32_t de_ctrl = 0; 
    uint32_t tempWidth, dxTemp, dyTemp;
    uint32_t maxRotationWidth;
    
    /* Maximum rotation width BLT */
    maxRotationWidth = 32 / BYTE_PER_PIXEL(bpp);

    /* Wait for the engine to be idle */
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */
        return -1;

        /* or */
        /* deReset(); */
    } 

    /* Return error if either the width or height is zero */ 
    if ((width == 0) || (height == 0))
        return -1;

    /* 2D Source Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_SOURCE_BASE, sBase);

    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);

    /* Program pitch (distance between the 1st points of two adjacent lines).
       Note that input pitch is BYTE value, but the 2D Pitch register uses
       pixel values. Need Byte to pixel convertion.
    */
    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, dPitch / BYTE_PER_PIXEL(bpp)) |
        FIELD_VALUE(0, DE_PITCH, SOURCE, sPitch / BYTE_PER_PIXEL(bpp)));

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, dPitch / BYTE_PER_PIXEL(bpp)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      sPitch / BYTE_PER_PIXEL(bpp)));
        
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);
       
    /* Setup Control Register */
    de_ctrl = FIELD_SET(0, DE_CONTROL, STATUS, START)    |
              FIELD_SET(0, DE_CONTROL, COMMAND, ROTATE)  |
              FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
              FIELD_VALUE(0, DE_CONTROL, ROP, rop2)      |
              ((repeatEnable == 1) ? 
                    FIELD_SET(0, DE_CONTROL, REPEAT_ROTATE, ENABLE) :
                    FIELD_SET(0, DE_CONTROL, REPEAT_ROTATE, DISABLE)) |
              deGetTransparency();
       
    /* 501 Hardware cannot handle rotblits > 32 bytes. Therefore the rotation 
       should be done part by part. Note on each rotation case. */
    switch (rotateDirection)
    {
        case ROTATE_NORMAL:
#if 0   /* Enable this on if the hardware 0 degree rotation bug has been fixed */

            /* Setup rotation direction to 0 degree */
            de_ctrl |= (FIELD_SET(0, DE_CONTROL, STEP_X, POSITIVE) |
                        FIELD_SET(0, DE_CONTROL, STEP_Y, POSITIVE));

            deRotate(sx, sy, dx, dy, width, height, de_ctrl);                        
#else
            /* 
             * Due to the hardware bug on the SM50x, rotate normal is simply done
             * by calling the normal bit BLT. Calling the rotation with 0 degree
             * will cause the hardware to rotate it 270 degree. 
             */
            return(deVideoMem2VideoMemBlt(
                    sBase,
                    sPitch,
                    sx,
                    sy,
                    dBase,
                    dPitch,
                    bpp,
                    dx,
                    dy,
                    width, 
                    height,
                    rop2));
#endif
            break;

        case ROTATE_180_DEGREE:
            /* The 180 degree rotation that has problem */

            return -1; /* Don't do anything until there is a HW fix */
            
            de_ctrl |= (FIELD_SET(0, DE_CONTROL, STEP_X, NEGATIVE) |
                        FIELD_SET(0, DE_CONTROL, STEP_Y, NEGATIVE));

            deRotate(sx, sy, dx, dy, width, height, de_ctrl);
            break;

        case ROTATE_90_DEGREE:
            /* 90 degrees rotation.  Calculate destination
			   coordinates:

				*---+
				|   |       +-----+
				|   |       |     |
				|   |  -->  |     |
				|   |       |     |
				|   |       *-----+
				+---+
			*/
            /* Update the new width */
            if (dy < width)
                width = dy+1;
                
            /* Set up the rotation direction to 90 degree */                
            de_ctrl |= (FIELD_SET(0, DE_CONTROL, STEP_X, NEGATIVE) |
                        FIELD_SET(0, DE_CONTROL, STEP_Y, POSITIVE));

#if 1   /* Disable this one if the hardware bug has been fixed */

            /* Do rotation part by part based on the maxRotationWidth */
            while (width > maxRotationWidth)
	        {
                deRotate(sx, sy, dx, dy, maxRotationWidth, height, de_ctrl);

                width -= maxRotationWidth;
                sx    += maxRotationWidth;
                dy    -= maxRotationWidth;
            }
#endif
            /* Rotate the rest of the segment */
            if (width > 0)
                deRotate(sx, sy, dx, dy, width, height, de_ctrl);
                
            break;
        
        case ROTATE_270_DEGREE:
            /* 270 degrees (90 degree CW) rotation.  Calculate destination
			   coordinates:

				*---+
				|   |       +-----*
				|   |       |     |  
				|   |  -->  |     | 
				|   |       |     |
				|   |       +-----+
				+---+
			*/
            de_ctrl |= (FIELD_SET(0, DE_CONTROL, STEP_X, POSITIVE) |
                        FIELD_SET(0, DE_CONTROL, STEP_Y, NEGATIVE));
 
#if 1   /* Disable this one if the hardware bug has been fixed */

            /* Do rotation part by part based on the maxRotationWidth */            
            while (width > maxRotationWidth)
	        {
                deRotate(sx, sy, dx, dy, maxRotationWidth, height, de_ctrl);
               
                width -= maxRotationWidth;
                sx    += maxRotationWidth;
                dy    += maxRotationWidth;
            }
#endif            
            /* Update the rest of the segment */
            if (width > 0)
                deRotate(sx, sy, dx, dy, width, height, de_ctrl);
            break;
    }

    return 0;
}

/* 
 * Function to draw a vertical line.
 *
 * Note:
 *      This function is using Short Stroke line
 */
int32_t deVerticalLine(
    uint32_t dBase,    /* Base address of destination surface counted from beginning of video frame buffer */
    uint32_t dPitch,   /* Pitch value of destination surface in BYTES */
    uint32_t bpp,      /* Color depth of destination surface: 8, 16 or 32 */
    uint32_t x,        /* Starting X Coordinate */
    uint32_t y,        /* Starting Y Coordinate */
    uint32_t length,   /* Length of the line */
    uint32_t color,    /* Color of the line */
    uint32_t rop2      /* ROP value */
)
{
    uint32_t de_ctrl;
    
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }

    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);
    
    /* Program pitch (distance between the 1st points of two adjacent lines).
       Note that input pitch is BYTE value, but the 2D Pitch register uses
       pixel values. Need Byte to pixel convertion.
    */
    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, dPitch / BYTE_PER_PIXEL(bpp)) |
        FIELD_VALUE(0, DE_PITCH, SOURCE, dPitch / BYTE_PER_PIXEL(bpp)));

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, dPitch / BYTE_PER_PIXEL(bpp)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      dPitch / BYTE_PER_PIXEL(bpp)));

    /* Set the Line Color */
    POKE_32(DE_FOREGROUND, color);

    /* Set the destination coordinate. */
    POKE_32(DE_DESTINATION,
        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_DESTINATION, X,    x)       |
        FIELD_VALUE(0, DE_DESTINATION, Y,    y));

    /* Set the line length and width */
    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    1) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, length));
        
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);

    /* Set the control register. For the Vertical line Short Stroke, the Direction Control
       should be set to 0 (which is defined as Left to Right). */
    de_ctrl = 
        FIELD_SET  (0, DE_CONTROL,  STATUS,     START)         |
        FIELD_SET  (0, DE_CONTROL,  DIRECTION,  LEFT_TO_RIGHT) |
        FIELD_SET  (0, DE_CONTROL,  MAJOR,      Y)             |
        FIELD_SET  (0, DE_CONTROL,  STEP_X,     NEGATIVE)      |
        FIELD_SET  (0, DE_CONTROL,  STEP_Y,     NEGATIVE)      |
        FIELD_SET  (0, DE_CONTROL,  LAST_PIXEL, OFF)           |
        FIELD_SET  (0, DE_CONTROL,  COMMAND,    SHORT_STROKE)  |
        FIELD_SET  (0, DE_CONTROL,  ROP_SELECT, ROP2)          |
        FIELD_VALUE(0, DE_CONTROL,  ROP,        rop2);

    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

    return 0;
}

/* 
 * Function to draw a horizontal line.
 *
 * Note:
 *      This function is using Short Stroke line
 */
int32_t deHorizontalLine(
    uint32_t dBase,    /* Base address of destination surface counted from beginning of video frame buffer */
    uint32_t dPitch,   /* Pitch value of destination surface in BYTES */
    uint32_t bpp,      /* Color depth of destination surface: 8, 16 or 32 */
    uint32_t x,        /* Starting X Coordinate */
    uint32_t y,        /* Starting Y Coordinate */
    uint32_t length,   /* Length of the line */
    uint32_t color,    /* Color of the line */
    uint32_t rop2      /* ROP value */
)
{
    uint32_t de_ctrl;
    
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }
    
    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);
    
    /* Program pitch (distance between the 1st points of two adjacent lines).
       Note that input pitch is BYTE value, but the 2D Pitch register uses
       pixel values. Need Byte to pixel convertion.
    */
    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, dPitch / BYTE_PER_PIXEL(bpp)) |
        FIELD_VALUE(0, DE_PITCH, SOURCE, dPitch / BYTE_PER_PIXEL(bpp)));
        
    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, dPitch / BYTE_PER_PIXEL(bpp)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      dPitch / BYTE_PER_PIXEL(bpp)));

    /* Set the Line Color */
    POKE_32(DE_FOREGROUND, color);

    /* Set the destination coordinate */
    POKE_32(DE_DESTINATION,
        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_DESTINATION, X,    x)       |
        FIELD_VALUE(0, DE_DESTINATION, Y,    y));

    /* Set the line length and width */
    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    length) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, 1));
        
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);

    /* Set the control register. For the Horizontal line Short Stroke, the Direction Control
       should be set to 1 (which is defined as Right to Left). */
    de_ctrl = 
        FIELD_SET  (0, DE_CONTROL,  STATUS,     START)         |
        FIELD_SET  (0, DE_CONTROL,  DIRECTION,  RIGHT_TO_LEFT) |
        FIELD_SET  (0, DE_CONTROL,  MAJOR,      X)             |
        FIELD_SET  (0, DE_CONTROL,  STEP_X,     NEGATIVE)      |
        FIELD_SET  (0, DE_CONTROL,  STEP_Y,     NEGATIVE)      |
        FIELD_SET  (0, DE_CONTROL,  LAST_PIXEL, OFF)           |
        FIELD_SET  (0, DE_CONTROL,  COMMAND,    SHORT_STROKE)  |
        FIELD_SET  (0, DE_CONTROL,  ROP_SELECT, ROP2)          |
        FIELD_VALUE(0, DE_CONTROL,  ROP,        rop2);

    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());
    
    return 0;
}

/* 
 * Function to draw a line.
 *
 * Note:
 *      This function is using Short Stroke Command for Vertical, Horizontal, and 
 *      Diagonal line. Other line are drawn using the Line Draw Command.
 */
int32_t deLine(
    uint32_t dBase,    /* Base address of destination surface counted from beginning of video frame buffer */
    uint32_t dPitch,   /* Pitch value of destination surface in BYTES */
    uint32_t bpp,      /* Color depth of destination surface: 8, 16 or 32 */
    int32_t x0,                /* Starting X Coordinate */
    int32_t y0,                /* Starting Y Coordinate */
    int32_t x1,                /* Ending X Coordinate */
    int32_t y1,                /* Ending Y Coordinate */
    uint32_t color,    /* Color of the line */
    uint32_t rop2      /* ROP value */
)
{
    uint32_t de_ctrl =
        FIELD_SET  (0, DE_CONTROL, STATUS,     START)         |
        FIELD_SET  (0, DE_CONTROL, DIRECTION,  LEFT_TO_RIGHT) |
        FIELD_SET  (0, DE_CONTROL, MAJOR,      X)             |
        FIELD_SET  (0, DE_CONTROL, STEP_X,     POSITIVE)      |
        FIELD_SET  (0, DE_CONTROL, STEP_Y,     POSITIVE)      |
        FIELD_SET  (0, DE_CONTROL, LAST_PIXEL, OFF)           |
        FIELD_SET  (0, DE_CONTROL, ROP_SELECT, ROP2)          |
        FIELD_VALUE(0, DE_CONTROL, ROP,        rop2);

    uint32_t dx, dy;
    
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }
    
    DDKDEBUGPRINT((DE_LEVEL, "x0=%d(%x), y0=%d(%0x)\n", x0, x0, y0, y0));
    DDKDEBUGPRINT((DE_LEVEL, "x1=%d(%x), y1=%d(%0x)\n", x1, x1, y1, y1));
    
    /* Return error if x0 and/or y0 are negative numbers. The hardware does not take
       any negative values on these two origin line coordinate.
       When drawing with a negative x0, the line might be drawn incorrectly.
       When drawing with a negative y0, the system might reboot or hang.
     */
    if ((x0 < 0) || (y0 < 0))
    {
        DDKDEBUGPRINT((ERROR_LEVEL, "Negative origin coordinates are not allowed (x0, y0) = (%d, %d).\n", x0, y0));
        return (-1);
    }

    /* Calculate delta X */
    if (x0 <= x1)
    {
        dx = x1 - x0 + 1;
        de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, NEGATIVE);
    }
    else
        dx = x0 - x1 + 1;

    /* Calculate delta Y */
    if (y0 <= y1)
    {
        dy = y1 - y0 + 1;
        de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, NEGATIVE);
    }
    else
        dy = y0 - y1 + 1;
        
    DDKDEBUGPRINT((DE_LEVEL, "dx=%d(%x), dy=%d(%x)\n", dx, dx, dy, dy));        

    /* Determine the major axis */
    if (dx < dy)
        de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, Y);                

    /*****************************************************
     * Draw the line based on the calculated information *
     *****************************************************/
     
    /* Vertical line? */
    if (x0 == x1)
        deVerticalLine(dBase, dPitch, bpp, x0, (y0 < y1) ? y0 : y1, dy, color, rop2);

    /* Horizontal line? */
    else if (y0 == y1)
        deHorizontalLine(dBase, dPitch, bpp, (x0 < x1) ? x0 : x1, y0, dx, color, rop2);

    else 
    {
        /****************************
         * Set the common registers *
         ****************************/
         
        /* 2D Destination Base.
           It is an address offset (128 bit aligned) from the beginning of frame buffer.
        */
        POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);

        /* Program pitch (distance between the 1st points of two adjacent lines).
           Note that input pitch is BYTE value, but the 2D Pitch register uses
           pixel values. Need Byte to pixel convertion.
        */
        POKE_32(DE_PITCH,
            FIELD_VALUE(0, DE_PITCH, DESTINATION, dPitch / BYTE_PER_PIXEL(bpp)) |
            FIELD_VALUE(0, DE_PITCH, SOURCE, dPitch / BYTE_PER_PIXEL(bpp)));
    
        /* Screen Window width in Pixels.
           2D engine uses this value to calculate the linear address in frame buffer for a given point.
        */
        POKE_32(DE_WINDOW_WIDTH,
            FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, dPitch / BYTE_PER_PIXEL(bpp)) |
            FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      dPitch / BYTE_PER_PIXEL(bpp)));

        /* Set the Line Color */
        POKE_32(DE_FOREGROUND, color);

        /* Set the destination coordinate */
        POKE_32(DE_DESTINATION,
            FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
            FIELD_VALUE(0, DE_DESTINATION, X,    x0)      |
            FIELD_VALUE(0, DE_DESTINATION, Y,    y0));
        
        /* Set the pixel format of the destination */
        deSetPixelFormat(bpp);
    
        /* Diagonal line? */
        if (dx == dy)
        {
            /* Set the line length and width */
            POKE_32(DE_DIMENSION,
                FIELD_VALUE(0, DE_DIMENSION, X,    1) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, dx));

            /* Set the command register. */
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, COMMAND, SHORT_STROKE);

            POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());            
        }

        /* Generic line */
        else
        {
            int32_t k1, k2, et, w;
            
            if (dx < dy)
            {
                k1 = 2 * dx;
                et = k1 - dy;
                k2 = et - dy;
                w  = dy + 1;
            } 
            else 
            {
                k1 = 2 * dy;
                et = k1 - dx;
                k2 = et - dx;
                w  = dx + 1;
            }
            
            DDKDEBUGPRINT((DE_LEVEL, "k1=%d(%x), k2=%d(%x)\n", k1, k1, k2, k2));
            
            /* Return error if the value of K1 and/or K2 is/are overflowed which is 
               determined using the following condition:
                        0 < k1 < (2^13 - 1)
                    -2^13 < k2 < 0
               Bit-14 is the sign bit.
               
               Failing to follow this guidance will result in incorrect line drawing.
               On failure, please use software to draw the correct line.
             */             
            if ((k1 > 8191) || (k2 < (0 - 8192)))
            {
                DDKDEBUGPRINT((ERROR_LEVEL, "K1=%d(0x%04x) and/or K2=%d(0x%04x) is/are overflowed.\n", k1, k1, k2, k2));
                return (-1);
            }
        
            /* Set the k1 and k2 */
            POKE_32(DE_SOURCE,
                FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_SOURCE, X_K1_LINE, k1) |
                FIELD_VALUE(0, DE_SOURCE, Y_K2_LINE, k2));

            /* Set the line length and width */
            POKE_32(DE_DIMENSION,
                FIELD_VALUE(0, DE_DIMENSION, X,    w) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, et));

            /* Set the control register. */
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, COMMAND, LINE_DRAW);

            POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());
        }
    }
    
    return 0;
}

