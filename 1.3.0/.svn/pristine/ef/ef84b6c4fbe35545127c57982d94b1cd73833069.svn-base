#ifndef DDK_FUNC_H__
#define DDK_FUNC_H__

long ddk750_setModeEx(void *,logicalMode_t *pLogicalMode);
long ddk750_setCustomMode(void *,logicalMode_t *pLogicalMode, mode_parameter_t *pUserModeParam);
unsigned long ddk750_getFrameBufSize(void *);
long ddk750_initChipParm(void*,initchip_param_t *);
void ddk750_setDPMS(void *,DPMS_t state);
void ddk750_deInit(void *,disp_control_t controller);
long ddk750_2d_waitIdle(void*);
long ddk750_deRectFill(
		void *,
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
		void*,
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
		void*,
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

long ddk750_deSystemMem2VideoMemMonoBlt_packet(
		void *,
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
unsigned long rop2,
void * buffer);     /* ROP value */
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

long ddk750_setLogicalDispOutput(void *,disp_output_t output,unsigned char swapDisplay);
long ddk750_initDisplay(void*);
unsigned char sii164IsConnected(void*);
unsigned long deGetTransparency(void*);
long deSetTransparency(
		void *,
unsigned long enable,     /* 0 = disable, 1 = enable transparency feature */
unsigned long tSelect,    /* 0 = compare source, 1 = compare destination */
unsigned long tMatch,     /* 0 = Opaque mode, 1 = transparent mode */
unsigned long ulColor);    /* Color to compare. */

int sii9022xInitChip(void*);
int sii9022xSetMode(void *,mode_parameter_t * pmode);
unsigned char sii9022xIsConnected(void*);
/* swi2c functions */
long swI2CInit(
		void *,
		unsigned char i2cClkGPIO, 
		unsigned char i2cDataGPIO
		);

unsigned char swI2CReadReg(
		void *,
		unsigned char deviceAddress, 
		unsigned char registerIndex
		);

long swI2CWriteReg(
		void *,
		unsigned char deviceAddress, 
		unsigned char registerIndex, 
		unsigned char data
		);
#endif

