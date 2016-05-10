/*
 * Voyager GX SDK
 *
 * 2d.h
 */
#ifndef _2D_H_
#define _2D_H_

#include "csc.h"

#define POKE_8(address, value)          pokeRegisterByte(address, value)
#define POKE_16(address, value)         pokeRegisterWord(address, value)
#define POKE_32(address, value)         pokeRegisterDWord(address, value)
#define PEEK_8(address)                 peekRegisterByte(address)
#define PEEK_16(address)                peekRegisterWord(address)
#define PEEK_32(address)                peekRegisterDWord(address)

/* Rotation Direction */
typedef enum _rotate_dir_t
{
    ROTATE_NORMAL = 0,
    ROTATE_90_DEGREE,
    ROTATE_180_DEGREE,
    ROTATE_270_DEGREE
}
rotate_dir_t;

/*
 * 2D Engine Initialization.
 * This function must be called before other 2D functions.
 * Assumption: A specific vidoe mode has been properly set up.
 */
_X_EXPORT void deInit(void);

/*
 * Reset 2D engine by 
 * 1) Aborting the current 2D operation.
 * 2) Re-enable 2D engine to normal state.
 */
_X_EXPORT void deReset(void);
 
/*
 * Wait until 2D engine is not busy.
 * All 2D operations are recommand to check 2D engine idle before start.
 *
 * Return: 0 = return because engine is idle and normal.
 *        -1 = return because time out (2D engine may have problem).
 */
_X_EXPORT int32_t deWaitForNotBusy(void);

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
int32_t deWaitIdle(uint32_t i);

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
uint32_t y2);    /* Note that the region will not include x2 and y2 */

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
uint32_t ulColor);   /* Color to compare. */

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
uint32_t rop2);  /* ROP value */

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
uint32_t height, /* width and height of rectange in pixel value */
uint32_t rop2);  /* ROP value */

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
uint32_t rop2);    /* ROP value */

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
uint32_t rop2);    /* ROP value */

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
uint32_t fbAddr);      /* Destination in Video memory to store the font */

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
uint32_t rop2);  /* ROP value */

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
uint32_t rop2);      /* ROP value */

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
);

/*
 * Rotation Blt.
 * 
 * This function rotates an image to the screen based on the given rotation direction
 * (0, 90, 180, or 270 degree).
 * 
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
);

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
);

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
);

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
);

#endif /* _2D_H_ */
