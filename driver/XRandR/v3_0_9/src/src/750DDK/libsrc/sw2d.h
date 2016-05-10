/*
 * Voyager GX SDK
 *
 * $Workfile:   sw2d.h
 *
*/
#ifndef _SW2D_H_
#define _SW2D_H_

/* Some color definitions */
#define BPP32_RED    0x00ff0000
#define BPP32_GREEN  0x0000ff00
#define BPP32_BLUE   0x000000ff
#define BPP32_WHITE  0x00ffffff
#define BPP32_GRAY   0x00808080
#define BPP32_YELLOW 0x00ffff00
#define BPP32_CYAN   0x0000ffff
#define BPP32_PINK   0x00ff00ff
#define BPP32_BLACK  0x00000000

#define BPP16_RED    0x0000f800
#define BPP16_GREEN  0x000007e0
#define BPP16_BLUE   0x0000001f
#define BPP16_WHITE  0x0000ffff
#define BPP16_GRAY   0x00008410
#define BPP16_YELLOW 0x0000ffe0
#define BPP16_CYAN   0x000007ff
#define BPP16_PINK   0x0000f81f
#define BPP16_BLACK  0x00000000

#define BPP8_RED     0x000000b4
#define BPP8_GREEN   0x0000001e
#define BPP8_BLUE    0x00000005
#define BPP8_WHITE   0x000000ff
#define BPP8_GRAY    0x000000ec
#define BPP8_YELLOW  0x000000d2
#define BPP8_CYAN    0x00000023
#define BPP8_PINK    0x000000b9
#define BPP8_BLACK   0x00000000

/* Raster Op functions */
#define ROP2_XOR  0x06
#define ROP2_COPY 0x0C

/* Blt Direction definitions */
#define TOP_TO_BOTTOM 0
#define LEFT_TO_RIGHT 0
#define BOTTOM_TO_TOP 1
#define RIGHT_TO_LEFT 1

uint32_t swRasterOp2(uint32_t S, uint32_t D, uint32_t rop2);

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
uint32_t rop2);     /* ROP value */

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
uint32_t y);        /* Position (X, Y) to set in pixel value */

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
uint32_t rop2);    /* ROP value */

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
uint32_t rop2);     /* ROP value */

void swHorizontalLine(
uint32_t destBase, /* Base address of destination surface counted from beginning of video frame buffer */
uint32_t pitch,    /* Pitch value of destination surface in BYTES */
uint32_t bpp,      /* Color depth of destination surface: 8, 16 or 32 */
uint32_t x,
uint32_t y,        /* Starting point (X, Y) of line */
uint32_t length,   /* Length of line */
uint32_t color,    /* Color */
uint32_t rop2);    /* ROP value */

void swVerticalLine(
uint32_t destBase, /* Base address of destination surface counted from beginning of video frame buffer */
uint32_t pitch,    /* Pitch value of destination surface in BYTES */
uint32_t bpp,      /* Color depth of destination surface: 8, 16 or 32 */
uint32_t x,
uint32_t y,        /* Starting point (X, Y) of line */
uint32_t length,   /* Length of line */
uint32_t color,    /* Color */
uint32_t rop2);    /* ROP value */

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
uint32_t rop2);  /* ROP value */

#endif //_SW2D_H_
