/*
 * Voyager GX SDK
 *
 * sw2d.c
 *
 * This file contains the source code for dumb frame buffer operations.
 *
 */

/* Remove the defs.h when VALIDATION_CHIP definition is not needed anymore. */
#include "defs.h"

#include "hardware.h"
#include "sw2d.h"

uint32_t swRasterOp2(uint32_t S, uint32_t D, uint32_t rop2)
{
    uint32_t opValue;

    switch (rop2)
    {
    case 0:
    opValue = 0;
    break;
    case 1:
    opValue = ~(S | D);
    break;
    case 2:
    opValue = ~S & D;
    break;
    case 3:
    opValue = ~S;
    break;
    case 4:
    opValue = S & ~D;
    break;
    case 5:
    opValue = ~D;
    break;
    case 6:
    opValue = S ^ D;
    break;
    case 7:
    opValue = ~(S & D);
    break;
    case 8:
    opValue = S & D;
    break;
    case 9:
    opValue = ~ (S ^ D);
    break;
    case 0xA:
    opValue = D;
    break;
    case 0xB:
    opValue = ~S | D;
    break;
    case 0xC:
    opValue = S;
    break;
    case 0xD:
    opValue = S | ~D;
    break;
    case 0xE:
    opValue = S | D;
    break;
    case 0xF:
    opValue = 0xFFFFFFFF;
    break;
    default:
        opValue = S;
        break;
    }

    return(opValue);
}


/*
 * This function set up a pixel value in the frame buffer.
 *
 * Note:
 * 1) It can only set pixel within the frame buffer.
 * 2) This function is NOT for drawing surface created in system memory.
 *
 */
void swSetPixel(
uint32_t destBase, /* Base address of destination surface counted from beginning of video frame buffer */
uint32_t pitch,    /* Pitch value of destination surface in BYTES */
uint32_t bpp,      /* Color depth of destination surface: 8, 16 or 32 */
uint32_t x, 
uint32_t y,        /* Position (X, Y) to set in pixel value */
uint32_t color,    /* Color */
uint32_t rop2)     /* ROP value */
{
    uint32_t destAddr;
    unsigned char  destColor8;
    unsigned short destColor16;
    uint32_t  destColor32;

    destAddr = (y * pitch) + (x * bpp/8) + destBase;

    switch (bpp)
    {
        case 8:
#ifdef VALIDATION_CHIP
            /* In application with interrupt, calling peekWord will sometimes 
               caused the system hangs. */
            destColor8 = 0;
#else        
            destColor8 = peekByte(destAddr);
#endif
            destColor8 = (unsigned char)swRasterOp2(color, destColor8, rop2);
            pokeByte(destAddr, destColor8);
            break;

        case 16:
#ifdef VALIDATION_CHIP
            /* In application with interrupt, calling peekWord will sometimes 
               caused the system hangs. */
            destColor16 = 0;
#else
            destColor16 = peekWord(destAddr);
#endif
            destColor16 = (unsigned short)swRasterOp2(color, destColor16, rop2);
            pokeWord(destAddr, destColor16);
            break;

        case 32:
        default:
#ifdef VALIDATION_CHIP
            /* In application with interrupt, calling peekWord will sometimes 
               caused the system hangs. */
            destColor32 = 0;
#else
            destColor32 = peekDWord(destAddr);
#endif

            destColor32 = (uint32_t)swRasterOp2(color, destColor32, rop2);
            pokeDWord(destAddr, destColor32);
            break;
    }
}


/*
 * This function gets a pixel value in the frame buffer.
 *
 * Note:
 * 1) It can only get pixel within the frame buffer.
 * 2) This function is NOT for drawing surface created in system memory.
 * 3) This function always return a 32 bit pixel value disregard bpp = 8, 16, or 32.
 *    The calling funtion has to type cast the return value into Byte, word or 
 *    DWord according to BPP.
 *
 */
uint32_t swGetPixel(
uint32_t destBase, /* Base address of destination surface counted from beginning of video frame buffer */
uint32_t pitch,    /* Pitch value of destination surface in BYTES */
uint32_t bpp,      /* Color depth of destination surface: 8, 16 or 32 */
uint32_t x, 
uint32_t y)        /* Position (X, Y) to set in pixel value */
{
    uint32_t destAddr;
    uint32_t pixel = 0;

    destAddr = (y * pitch) + (x * bpp/8) + destBase;
    
    switch (bpp)
    {
        case 8:
            pixel = (uint32_t) peekByte(destAddr);
            break;

        case 16:
            pixel = (uint32_t) peekWord(destAddr);
            break;

        case 32:
        default:
            pixel = (uint32_t) peekDWord(destAddr);
            break;
    }

    return(pixel);
}


/*
 *  This function uses software only method to fill a rectangular area with a specific color.
 * Input: See comment of code below.
 * 
 */
void swRectFill(
uint32_t destBase, /* Base address of destination surface counted from beginning of video frame buffer */
uint32_t pitch,    /* Pitch value of destination surface in BYTES */
uint32_t bpp,      /* Color depth of destination surface: 8, 16 or 32 */
uint32_t x,
uint32_t y,        /* Upper left corner (X, Y) of rectangle in pixel value */
uint32_t width, 
uint32_t height,   /* width and height of rectange in pixel value */
uint32_t color,    /* Fill color */
uint32_t rop2)     /* ROP value */
{
    uint32_t dx, dy;

    for (dy=y; dy<(y+height); dy++)
        for (dx=x; dx<(x+width); dx++)
            swSetPixel(destBase, pitch, bpp, dx, dy, color, rop2);
}


/*
 * This function draws a hollow rectangle, no fills.
 */
void swRect(
uint32_t destBase, /* Base address of destination surface counted from beginning of video frame buffer */
uint32_t pitch,    /* Pitch value of destination surface in BYTES */
uint32_t bpp,      /* Color depth of destination surface: 8, 16 or 32 */
uint32_t x,
uint32_t y,        /* Upper left corner (X, Y) of rectangle in pixel value */
uint32_t width, 
uint32_t height,   /* width and height of rectange in pixel value */
uint32_t color,    /* border color */
uint32_t rop2)     /* ROP value */
{
    swRectFill(destBase, pitch, bpp, x,                 y,  width,      1, color, rop2);
    swRectFill(destBase, pitch, bpp, x,        y+height-1,  width,      1, color, rop2);
    swRectFill(destBase, pitch, bpp, x,                 y,      1, height, color, rop2);
    swRectFill(destBase, pitch, bpp, x+width-1,         y,      1, height, color, rop2);
}


void swHorizontalLine(
uint32_t destBase, /* Base address of destination surface counted from beginning of video frame buffer */
uint32_t pitch,    /* Pitch value of destination surface in BYTES */
uint32_t bpp,      /* Color depth of destination surface: 8, 16 or 32 */
uint32_t x,
uint32_t y,        /* Starting point (X, Y) of line */
uint32_t length,   /* Length of line */
uint32_t color,    /* Color */
uint32_t rop2)     /* ROP value */
{
    swRectFill(destBase, pitch, bpp, x, y, length, 1, color, rop2);
}


void swVerticalLine(
uint32_t destBase, /* Base address of destination surface counted from beginning of video frame buffer */
uint32_t pitch,    /* Pitch value of destination surface in BYTES */
uint32_t bpp,      /* Color depth of destination surface: 8, 16 or 32 */
uint32_t x,
uint32_t y,        /* Starting point (X, Y) of line */
uint32_t length,   /* Length of line */
uint32_t color,    /* Color */
uint32_t rop2)     /* ROP value */
{
    swRectFill(destBase, pitch, bpp, x, y, 1, length, color, rop2);
}


/*
 * Video Memory to Video Memroy data transfer.
 *
 * Note: 
 * 1) All addresses are offset from the beginning for frame buffer.
 * 2) Both source and destination have to be same bpp (color depth).
 * 
 */
void swVideoMem2VideoMemBlt(
uint32_t sBase,  /* Address of source: offset in frame buffer */
uint32_t sPitch, /* Pitch value of source surface in BYTE */
uint32_t sx,
uint32_t sy,     /* Starting coordinate of source surface */
uint32_t dBase,  /* Address of destination: offset in frame buffer */
uint32_t dPitch, /* Pitch value of destination surface in BYTE */
uint32_t bpp,   /* color depth of destiination, source must have same bpp */
uint32_t dx,
uint32_t dy,     /* Starting coordinate of destination surface */
uint32_t width, 
uint32_t height, /* width and height of rectange in pixel value */
uint32_t rop2)   /* ROP value */
{
    uint32_t color, nDirection;
    int32_t x, y, opSign;

    nDirection = LEFT_TO_RIGHT;
    opSign = 1;

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

    for (y=0; y<height; y++)
    {
        for (x=0; x<width; x++)
        {
            color = swGetPixel(sBase, sPitch, bpp, sx+(opSign*x), sy+(opSign*y));
            swSetPixel(dBase, dPitch, bpp, dx+(opSign*x), dy+(opSign*y), color, rop2);
        }
    }
}
