#include "ddk750_common.h"

/*
void *memcpy(void *dest, const void *src, size_t count)
{
	char *tmp = dest;
	const char *s = src;

	while (count--)
		*tmp++ = *s++;
	return dest;
}
*/

long ddk750_2d_waitIdle()
{
    unsigned long i = 0x1000000;
    while (i--)
    {
        unsigned long dwVal = peekRegisterDWord(SYSTEM_CTRL);
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
 * Reset 2D engine by 
 * 1) Aborting the current 2D operation.
 * 2) Re-enable 2D engine to normal state.
 */
void deReset()
{
    unsigned long sysCtrl;

    /* Abort current 2D operation */
    sysCtrl = peekRegisterDWord(SYSTEM_CTRL);
    sysCtrl = FIELD_SET(sysCtrl, SYSTEM_CTRL, DE_ABORT, ON);
    pokeRegisterDWord(SYSTEM_CTRL, sysCtrl);

    /* Re-enable 2D engine to normal state */
    sysCtrl = peekRegisterDWord(SYSTEM_CTRL);
    sysCtrl = FIELD_SET(sysCtrl, SYSTEM_CTRL, DE_ABORT, OFF);
    pokeRegisterDWord(SYSTEM_CTRL, sysCtrl);
}




/*
 * 2D Engine Initialization.
 * This function must be called before other 2D functions.
 * Assumption: A specific video mode has been properly set up.
 */
void ddk750_deInit(disp_control_t controller)
{
	ulong value,pf;	
    	enable2DEngine(1);
	/* Just be sure no left-over operations from other applications */
    	deReset(); 
    	/*
    		Set up 2D registers that won't change for a specific mode.
    		Drawing engine bus and pixel mask, always want to enable. 
    	*/
    	pokeRegisterDWord(DE_MASKS, 0xFFFFFFFF);
	if(controller == PANEL_CTRL)
	{
		switch (FIELD_GET(peekRegisterDWord(PANEL_DISPLAY_CTRL),PANEL_DISPLAY_CTRL,FORMAT))
	    	{
	    		case PANEL_DISPLAY_CTRL_FORMAT_8:
				pf = DE_STRETCH_FORMAT_PIXEL_FORMAT_8;
				break;
			case PANEL_DISPLAY_CTRL_FORMAT_16:
				pf = DE_STRETCH_FORMAT_PIXEL_FORMAT_16;
				break;
			case PANEL_DISPLAY_CTRL_FORMAT_32:
				pf = DE_STRETCH_FORMAT_PIXEL_FORMAT_32;
				break;			
	    	}	
	}
	else
	{
		switch (FIELD_GET(peekRegisterDWord(CRT_DISPLAY_CTRL),CRT_DISPLAY_CTRL,FORMAT))
	    	{
	    		case CRT_DISPLAY_CTRL_FORMAT_8:
				pf = DE_STRETCH_FORMAT_PIXEL_FORMAT_8;
				break;
			case CRT_DISPLAY_CTRL_FORMAT_16:
				pf = DE_STRETCH_FORMAT_PIXEL_FORMAT_16;
				break;
			case CRT_DISPLAY_CTRL_FORMAT_32:
				pf = DE_STRETCH_FORMAT_PIXEL_FORMAT_32;
				break;			
	    	}
	}
    	
	value = FIELD_SET(0,DE_STRETCH_FORMAT,PATTERN_XY, NORMAL)|
			FIELD_VALUE(0,DE_STRETCH_FORMAT,PATTERN_Y, 0)|
			FIELD_VALUE(0,DE_STRETCH_FORMAT,PATTERN_X, 0)|
			FIELD_VALUE(0,DE_STRETCH_FORMAT,PIXEL_FORMAT, pf)|
			FIELD_SET(0,DE_STRETCH_FORMAT,ADDRESSING, XY)|
			FIELD_VALUE(0,DE_STRETCH_FORMAT,SOURCE_HEIGHT, 3);
	
	pokeRegisterDWord(DE_STRETCH_FORMAT,value);
	/* Clipping and transparent are disable after INIT */
	deSetClipping(0, 0, 0, 0, 0);
    	deSetTransparency(0, 0, 0, 0);

}


/*
 * This function enable/disable clipping area for the 2d engine.
 * Note that the clipping area is always rectangular.
 * 
 */
long deSetClipping(
unsigned long enable, /* 0 = disable clipping, 1 = enable clipping */
unsigned long x1,     /* x1, y1 is the upper left corner of the clipping area */
unsigned long y1,     /* Note that the region includes x1 and y1 */
unsigned long x2,     /* x2, y2 is the lower right corner of the clippiing area */
unsigned long y2)     /* Note that the region will not include x2 and y2 */
{
    if (ddk750_2d_waitIdle() != 0)
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
long deSetTransparency(
unsigned long enable,     /* 0 = disable, 1 = enable transparency feature */
unsigned long tSelect,    /* 0 = compare source, 1 = compare destination */
unsigned long tMatch,     /* 0 = Opaque mode, 1 = transparent mode */
unsigned long ulColor)    /* Color to compare. */
{
    unsigned long de_ctrl;

    if (ddk750_2d_waitIdle() != 0)
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
 * This function uses 2D engine to fill a rectangular area with a specific color.
 * The filled area includes the starting points.
 */
long ddk750_deRectFill( /*resolution_t resolution, point_t p0, point_t p1, unsigned long color, unsigned long rop2)*/
unsigned long dBase,  /* Base address of destination surface counted from beginning of video frame buffer */
unsigned long dPtich, /* Pitch value of destination surface in BYTES */
unsigned long bpp,    /* Color depth of destination surface: 8, 16 or 32 */
unsigned long x,
unsigned long y,      /* Upper left corner (X, Y) of rectangle in pixel value */
unsigned long width, 
unsigned long height, /* width and height of rectange in pixel value */
unsigned long color,  /* Color to be filled */
unsigned long rop2)   /* ROP value */
{
    unsigned long de_ctrl, bytePerPixel;

    bytePerPixel = bpp/8;
    
    if (ddk750_2d_waitIdle() != 0)
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

	/*
		because bpp had been set by deInit routine, we do not need reset again
		but,if dual mode need support ,  bpp should by set each time cuz CRT may 
		get a different bpp to PANEL and deInit may only set 2d engine bpp filed to
		be the same as PANEL layer
	*/
#if 0        
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);
#endif

    de_ctrl = 
        FIELD_SET  (0, DE_CONTROL,  STATUS,     START)          |
        FIELD_SET  (0, DE_CONTROL,  DIRECTION,  LEFT_TO_RIGHT)  |
        FIELD_SET  (0, DE_CONTROL,LAST_PIXEL, ON)            |
        FIELD_SET  (0, DE_CONTROL,  COMMAND,    RECTANGLE_FILL) |
        FIELD_SET  (0, DE_CONTROL,  ROP_SELECT, ROP2)           |
        FIELD_VALUE(0, DE_CONTROL,  ROP,        rop2);
#if 0
    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());
#else
	POKE_32(DE_CONTROL,de_ctrl);
#endif

    return 0;
}



/*
 * This function sets the pixel format that will apply to the 2D Engine.
 */
void deSetPixelFormat(
    unsigned long bpp
)
{
    unsigned long de_format;
    
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
 * This function gets the transparency status from DE_CONTROL register.
 * It returns a double word with the transparent fields properly set,
 * while other fields are 0.
 */
unsigned long deGetTransparency()
{
    unsigned long de_ctrl;

    de_ctrl = PEEK_32(DE_CONTROL);

    de_ctrl &= 
        FIELD_MASK(DE_CONTROL_TRANSPARENCY_MATCH) | 
        FIELD_MASK(DE_CONTROL_TRANSPARENCY_SELECT)| 
        FIELD_MASK(DE_CONTROL_TRANSPARENCY);

    return de_ctrl;
}


/*
 * Video Memory to Video Memory data transfer.
 * Note: 
 *        It works whether the Video Memroy is off-screeen or on-screen.
 *        This function is a one to one transfer without stretching or 
 *        mono expansion.
 */
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
unsigned long rop2)   /* ROP value */
{
    unsigned long nDirection, de_ctrl, bytePerPixel;
    long opSign;


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

#if 0
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);
#endif

	if (ddk750_2d_waitIdle() != 0)
    	{
        	/* 	The 2D engine is always busy for some unknown reason.
           		Application can choose to return ERROR, or reset it and
           		continue the operation.
        	*/
       		return -1;
        	/* or */
       		 /* deReset(); */
    	}
    
#if 0
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
        unsigned long xChunk = 192 / bytePerPixel; /* chunk width is in pixels */
        
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
#if 0
        	POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());
#else
		POKE_32(DE_CONTROL,de_ctrl);
#endif
    }

    return 0;
}


/*
 * System memory to Video memory monochrome expansion.
 * Source is monochrome image in system memory.
 * This function expands the monochrome data to color image in video memory.
 */
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
unsigned long rop2)     /* ROP value */
{
    unsigned long bytePerPixel;
    unsigned long ulBytesPerScan;
    unsigned long ul4BytesPerScan;
    unsigned long ulBytesRemain;
    unsigned long de_ctrl = 0;
    unsigned char ajRemain[4];
    long i, j;

    bytePerPixel = bpp/8;

    startBit &= 7; /* Just make sure the start bit is within legal range */
    ulBytesPerScan = (width + startBit + 7) / 8;
    ul4BytesPerScan = ulBytesPerScan & ~3;
    ulBytesRemain = ulBytesPerScan & 3;

    if (ddk750_2d_waitIdle() != 0)
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

#if 0	
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);
#endif	
    	de_ctrl = FIELD_VALUE(0, DE_CONTROL, ROP, rop2)         |
              FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2)    |
              FIELD_SET(0, DE_CONTROL, COMMAND, HOST_WRITE) |
              FIELD_SET(0, DE_CONTROL, HOST, MONO)          |
              FIELD_SET(0, DE_CONTROL, STATUS, START);
#if 1
    	POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());
#else
	POKE_32(DE_CONTROL,de_ctrl);
#endif

    /* Write MONO data (line by line) to 2D Engine data port */
    for (i=0; i<height; i++)
    {
        /* For each line, send the data in chunks of 4 bytes */
        for (j=0; j<(ul4BytesPerScan/4); j++)
        {
            POKE_32(DE_DATA_PORT, *(unsigned long *)(pSrcbuf + (j * 4)));
        }

        if (ulBytesRemain)
        {
            memcpy(ajRemain, pSrcbuf+ul4BytesPerScan, ulBytesRemain);
            POKE_32(DE_DATA_PORT, *(unsigned long *)ajRemain);
        }

        pSrcbuf += srcDelta;
    }

    return 0;
}


#if 0
/* 
 * System memory to Video memory data transfer
 * Note: 
 *         We also call it HOST Blt.
 *        This function is a one to one transfer without stretching or 
 *        mono expansion.
 */
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
unsigned long rop2)     /* ROP value */
{
    unsigned long bytePerPixel;
    unsigned long ulBytesPerScan;
    unsigned long ul8BytesPerScan;
    unsigned long ulBytesRemain;
    unsigned long de_ctrl = 0;
    unsigned char ajRemain[8];
    long i, j;

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
    if (ddk750_2d_waitIdle() != 0)
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
#if 0        
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);
#endif

	de_ctrl = PEEK_32(DE_CONTROL);
    	de_ctrl = 
        FIELD_VALUE(de_ctrl, DE_CONTROL, ROP, rop2)         |
        FIELD_SET(de_ctrl, DE_CONTROL, ROP_SELECT, ROP2)    |
        FIELD_SET(de_ctrl, DE_CONTROL, COMMAND, HOST_WRITE) |
        FIELD_SET(de_ctrl, DE_CONTROL, HOST, COLOR)         |
        FIELD_SET(de_ctrl, DE_CONTROL, STATUS, START);

#if 0
    	POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());
#endif
	POKE_32(DE_CONTROL,de_ctrl);
		

    /* Write bitmap/image data (line by line) to 2D Engine data port */
    for (i = 0; i < height; i++)
    {
        /* For each line, send the data in chunks of 4 bytes. */
        for (j=0; j < (ul8BytesPerScan/4);  j++)
            POKE_32(DE_DATA_PORT, *(unsigned long *)(pSrcbuf + (j * 4)));

        if (ulBytesRemain)
        {
            memcpy(ajRemain, pSrcbuf+ul8BytesPerScan, ulBytesRemain);
            POKE_32(DE_DATA_PORT, *(unsigned long *)ajRemain);
            POKE_32(DE_DATA_PORT, *(unsigned long *)(ajRemain+4));
        }

        pSrcbuf += srcDelta;
    }

    return 0;
}
#endif
