#ifndef DDK_FUNC_H__
#define DDK_FUNC_H__

long ddk750_setModeEx(logicalMode_t *pLogicalMode);
long ddk750_setCustomMode(logicalMode_t *pLogicalMode, mode_parameter_t *pUserModeParam);
unsigned long ddk750_getFrameBufSize(void);
long ddk750_initChipParm(initchip_param_t *);
void ddk750_setDPMS(DPMS_t state);
void ddk750_deInit(disp_control_t controller);
long ddk750_2d_waitIdle(void);
long ddk750_deRectFill( /*resolution_t resolution, point_t p0, point_t p1, unsigned long color, unsigned long rop2)*/
unsigned long dBase,  /* Base address of destination surface counted from beginning of video frame buffer */
unsigned long dPtich, /* Pitch value of destination surface in BYTES */
unsigned long bpp,    /* Color depth of destination surface: 8, 16 or 32 */
unsigned long x,
unsigned long y,      /* Upper left corner (X, Y) of rectangle in pixel value */
unsigned long width, 
unsigned long height, /* width and height of rectange in pixel value */
unsigned long color,  /* Color to be filled */
unsigned long rop2);   /* ROP value */
long ddk750_deVideoMem2VideoMemBlt(
unsigned long sBase,  /* Address of source: offset in frame buffer */
unsigned long sPitch, /* Pitch value of source surface in BYTE */
unsigned long sx,
unsigned long sy,     /* Starting coordinate of source surface */
unsigned long dBase,  /* Address of destination: offset in frame buffer */
unsigned long dPitch, /* Pitch value of destination surface in BYTE */
unsigned long bpp,    /* Color depth of destination surface */
unsigned long dx,
unsigned long dy,     /* Starting coordinate of destination surface */
unsigned long width, 
unsigned long height, /* width and height of rectangle in pixel value */
unsigned long rop2);   /* ROP value */
long ddk750_deSystemMem2VideoMemMonoBlt(
unsigned char *pSrcbuf, /* pointer to start of source buffer in system memory */
long srcDelta,          /* Pitch value (in bytes) of the source buffer, +ive means top down and -ive mean button up */
unsigned long startBit, /* Mono data can start at any bit in a byte, this value should be 0 to 7 */
unsigned long dBase,    /* Address of destination: offset in frame buffer */
unsigned long dPitch,   /* Pitch value of destination surface in BYTE */
unsigned long bpp,      /* Color depth of destination surface */
unsigned long dx,
unsigned long dy,       /* Starting coordinate of destination surface */
unsigned long width, 
unsigned long height,   /* width and height of rectange in pixel value */
unsigned long fColor,   /* Foreground color (corresponding to a 1 in the monochrome data */
unsigned long bColor,   /* Background color (corresponding to a 0 in the monochrome data */
unsigned long rop2);     /* ROP value */

#if 0
long ddk750_deSystemMem2VideoMemBlt(
unsigned char *pSrcbuf, /* pointer to source data in system memory */
long srcDelta,          /* width (in Bytes) of the source data, +ive means top down and -ive mean button up */
unsigned long dBase,    /* Address of destination: offset in frame buffer */
unsigned long dPitch,   /* Pitch value of destination surface in BYTE */
unsigned long bpp,      /* Color depth of destination surface */
unsigned long dx,
unsigned long dy,       /* Starting coordinate of destination surface */
unsigned long width, 
unsigned long height,   /* width and height of rectange in pixel value */
unsigned long rop2);     /* ROP value */
#endif

long ddk750_dumpRegisters(unsigned long start,unsigned long end,pf_output  pf,char*buf);
long ddk750_setLogicalDispOutput(disp_output_t output,unsigned char swapDisplay);
long ddk750_initDisplay(void);

#endif

