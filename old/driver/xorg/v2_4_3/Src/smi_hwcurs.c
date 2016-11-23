/* Header:   //Mercury/Projects/archives/XFree86/4.0/smi_hwcurs.c-arc   1.12   27 Nov 2000 15:47:48   Frido  $ */

/*
Copyright (C) 1994-1999 The XFree86 Project, Inc.  All Rights Reserved.
Copyright (C) 2000 Silicon Motion, Inc.  All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the names of the XFree86 Project and
Silicon Motion shall not be used in advertising or otherwise to promote the
sale, use or other dealings in this Software without prior written
authorization from the XFree86 Project and Silicon Motion.
*/
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/siliconmotion/smi_hwcurs.c,v 1.2 2001/03/03 22:26:13 tsi Exp $ */

#include "cursorstr.h"
#include "smi.h"

#define MAX_CURSOR		32
#define MAX_CURSOR_501  64
#define MAX_CURSOR_750  64
#define SMI_CURSOR_SIZE     1024
#define SMI501_CURSOR_SIZE  2048
#define SMI750_CURSOR_SIZE  2048
/* 501-specific cursor rotation routine */
void SMI501_RotateCursorShape(xf86CursorInfoPtr infoPtr, int angle, unsigned char *pByte);


static unsigned short
InterleaveBytes(source, mask)
{
    unsigned char  ibit;
    unsigned short usWord = 0;
    unsigned char  ucBitMask = 0x01;
    
    /* 
     * This function will interleave the bits in the source and mask bytes
     * to create a word that looks like this:
     *
     *    [M7 M6 M5 M4 M3 M2 M1 M0] [S7 S6 S5 S4 S3 S2 S1 S0]
     * Results in:
     *    [M7 S7 M6 S6 M5 S5 M4 S4 M3 S3 M2 S2 M1 S1 M0 S0]
     */
    
    for (ibit = 0; ibit < 8; ibit++)
    {
        usWord |= (source & ucBitMask) <<  ibit;
        usWord |= (mask   & ucBitMask) << (ibit + 1);
        ucBitMask <<= 1;
    }
    
    return (usWord);
}    

static unsigned char *
SMI_RealizeCursor(xf86CursorInfoPtr infoPtr, CursorPtr pCurs)
{
	SMIPtr pSmi = SMIPTR(infoPtr->pScrn);
	CursorBitsPtr bits = pCurs->bits;
	unsigned char * ram;
	unsigned char * psource = bits->source;
	unsigned char * pmask = bits->mask;
	int x, y, srcwidth, i;
    unsigned int MaxCursor;


	/* Allocate memory */
	ram   = (unsigned char *) xcalloc(1, SMI_CURSOR_SIZE);
    MaxCursor = MAX_CURSOR;
    	    
	if (ram == NULL)
	{
		return(NULL);
	}

	/* Calculate cursor information */
	srcwidth = ((bits->width + 31) / 8) & ~3;
	i = 0;

	switch (pSmi->rotate)
	{
		default:
			/* Copy cursor image */
			for (y = 0; y < min(MaxCursor, bits->height); y++)
			{
				for (x = 0; x < min(MaxCursor / 8, srcwidth); x++)
				{
					unsigned char mask   = byte_reversed[*pmask++];
					unsigned char source = byte_reversed[*psource++] & mask;

					ram[i++] = ~mask;
					ram[i++] = source;
					if (i & 4) i += 4;
				}

				pmask   += srcwidth - x;
				psource += srcwidth - x;

				/* Fill remaining part of line with no shape */
				for (; x < MaxCursor / 8; x++)
				{
					ram[i++] = 0xFF;
					ram[i++] = 0x00;
					if (i & 4) i += 4;
				}
			}

			/* Fill remaining part of memory with no shape */
			for (; y < MaxCursor; y++)
			{
				for (x = 0; x < MaxCursor / 8; x++)
				{
					ram[i++] = 0xFF;
					ram[i++] = 0x00;
					if (i & 4) i += 4;
				}
			}
			break;

		case SMI_ROTATE_CW:
			/* Initialize cursor memory */
			for (i = 0; i < 1024;)
			{
				ram[i++] = 0xFF;
				ram[i++] = 0x00;
				if (i & 4) i += 4;
			}

			/* Rotate cursor image */
			for (y = 0; y < min(MaxCursor, bits->height); y++)
			{
				unsigned char bitmask = 0x01 << (y & 7);
				int           index   = ((MaxCursor - y - 1) / 8) * 2;
				if (index & 4) index += 4;

				for (x = 0; x < min(MaxCursor / 8, srcwidth); x++)
				{
					unsigned char mask   = *pmask++;
					unsigned char source = *psource++ & mask;

					i = index + (x * 8) * 16;
					if (mask || (source & mask))
					{
						unsigned char bit;
						for (bit = 0x01; bit; bit <<= 1)
						{
							if (mask & bit)
							{
								ram[i + 0] &= ~bitmask;
							}

							if (source & bit)
							{
								ram[i + 1] |= bitmask;
							}

							i += 16;
						}
					}
				}

				pmask   += srcwidth - x;
				psource += srcwidth - x;
			}
			break;

		case SMI_ROTATE_CCW:
			/* Initialize cursor memory */
			for (i = 0; i < 1024;)
			{
				ram[i++] = 0xFF;
				ram[i++] = 0x00;
				if (i & 4) i += 4;
			}

			/* Rotate cursor image */
			for (y = 0; y < min(MaxCursor, bits->height); y++)
			{
				unsigned char bitmask = 0x80 >> (y & 7);
				int			  index	  = (y >> 3) * 2;
				if (index & 4) index += 4;

				for (x = 0; x < min(MaxCursor / 8, srcwidth); x++)
				{
					unsigned char mask   = *pmask++;
					unsigned char source = *psource++ & mask;

					i = index + (MaxCursor - x * 8 - 1) * 16;
					if (mask || (source & mask))
					{
						unsigned char bit;
						for (bit = 0x01; bit; bit <<= 1)
						{
							if (mask & bit)
							{
								ram[i + 0] &= ~bitmask;
							}

							if (source & bit)
							{
								ram[i + 1] |= bitmask;
							}

							i -= 16;
						}
					}
				}

				pmask   += srcwidth - x;
				psource += srcwidth - x;
			}
			break;
	}

	return(ram);
}

static unsigned char *
SMI_501_RealizeCursor(xf86CursorInfoPtr infoPtr, CursorPtr pCurs)
{
	SMIPtr pSmi = SMIPTR(infoPtr->pScrn);
	CursorBitsPtr bits = pCurs->bits;
	unsigned char  * ram;
	unsigned short * usram;
	unsigned char  * psource = bits->source;
	unsigned char  * pmask = bits->mask;
	int x, y, srcwidth, i;
	unsigned int MaxCursor;


	/* Allocate memory */
	ram   = (unsigned char *) xcalloc(1, SMI501_CURSOR_SIZE);
    
	usram = (unsigned short *)ram;
	MaxCursor = MAX_CURSOR_501;
    	    
	if (ram == NULL)
	{
		return(NULL);
	}

	/* Calculate cursor information */
	srcwidth = ((bits->width + 31) / 8) & ~3;

	i = 0;
    
	/* Copy cursor image */
	for (y = 0; y < min(MaxCursor, bits->height); y++)
	{
		for (x = 0; x < min(MaxCursor / 8, srcwidth); x++)
		{
			unsigned char mask   = *pmask++;
			unsigned char source = *psource++ & mask;

			usram[i++] = InterleaveBytes(source, mask);
		}

		pmask   += srcwidth - x;
		psource += srcwidth - x;

		/* Fill remaining part of line with no shape */
		for (; x < MaxCursor / 8; x++)
		{
			usram[i++] = 0x0000;
		}
	}

	/* Fill remaining part of memory with no shape */
	for (; y < MaxCursor; y++)
	{
		for (x = 0; x < MaxCursor / 8; x++)
		{
			usram[i++] = 0x0000;
		}
	}

    SMI501_RotateCursorShape(infoPtr, pSmi->rotate ,ram);

	return(ram);
}



/* From the SMI Windows CE driver */
void SMI501_RotateCursorShape(xf86CursorInfoPtr infoPtr, int angle, unsigned char *pByte)
{
	BYTE *pCursor;
	unsigned long ulBase, ulIndex;
	BYTE src[256], dst[256]; /* 128 = 8 x 32 */
	BYTE  jMask, j, bitMask;
	int x, y, cx = 32, cy = 32;
    
	pCursor = pByte;

	memset(src, 0x00, sizeof(src));
	memset(dst, 0x00, sizeof(dst));

	/* Save the original pointer shape into local memory shapeRow[] */
	for (y=0; y<cy; y++)
	{
		for (x=0; x<cx/4; x++)
			src[y*8+x] = pByte[x];
		pByte+=16;
	}

	switch (angle)
	{
	case SMI_ROTATE_CCW:
    
		for (y = 0; y < cy; y++)
		{		
			jMask = 0x02 << ((y&3)*2);
			for (x = 0; x < cx; x++)
			{
				j  = src[y*8 + x/4];
				bitMask = 0x02 << ((x&3)*2);

				ulBase  = (31 - x) * 8;
				ulIndex = (ulBase + y / 4);
				
				if (j & bitMask)
					dst[ulIndex] |= jMask;
				if (j & (bitMask>>1))
					dst[ulIndex] |= (jMask >> 1);
			}
		}
		break;


	case SMI_ROTATE_CW:
    
		for (y = 0; y < cy; y++)
		{
			jMask  = 0x80 >> ((y&3)*2);

			/* Write available bits into shapeRow */
			for (x = 0; x < cx; x++)
			{
				j = src[y*8 + x/4];
				bitMask = 0x02 << ((x&3)*2);

				ulBase = x * 8;
				ulIndex = (ulBase + (31 - y) / 4);

				if (j & bitMask)
					dst[ulIndex] |= jMask;
				if (j & (bitMask>>1))
					dst[ulIndex] |= (jMask >> 1);
			}
		}
		break;
    		
	default:
        	return;
	}
    
	for (y=0; y<cy; y++)
	{
		for (x=0; x<cx/4; x++)
			pCursor[x] = dst[y*8+x];
		pCursor+=16;
	}
    
}



























static void
SMI_LoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	CARD8 tmp;


    /* Load storage location. */
    if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
	{   
		VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x80,
				pSmi->FBCursorOffset / 2048);
		tmp = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x81) & 0x80;
		VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x81,
				tmp | ((pSmi->FBCursorOffset / 2048) >> 8));

    	/* Program FPR copy when on the 730 */
    	if (pSmi->Chipset == SMI_COUGAR3DR)
    	{
    	    CARD32 fpr15c;

    	    /* put address in upper word, and disable the cursor */
    	    fpr15c  = READ_FPR(pSmi, FPR15C) & FPR15C_MASK_HWCCOLORS;
    	    fpr15c |= (pSmi->FBCursorOffset / 2048) << 16;
    	    WRITE_FPR(pSmi, FPR15C, fpr15c);
    	}
	}        
	else	/* SMI 501 family */
	{
    	/* Write address, disabling the HW cursor */
		
		
		if (!pSmi->IsSecondary) {
			WRITE_DCR(pSmi, DCRF0,	pSmi->FBCursorOffset);	/* Panel HWC Addr */
			xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, VERBLEV,
			"Primary FBCursorOffset at 0x%08X\n", pSmi->FBCursorOffset);
		} else{
			WRITE_DCR(pSmi, DCR230,pSmi->videoRAMBytes+pSmi->FBCursorOffset);	/* CRT	 HWC Addr */
			xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, VERBLEV,
			"Secondary FBCursorOffset at 0x%08X\n", pSmi->FBCursorOffset);
		}

	}
    
	/* Copy cursor image to framebuffer storage */
	memcpy(pSmi->FBBase + pSmi->FBCursorOffset, src, 1024);
    
}

static void
SMI_ShowCursor(ScrnInfoPtr pScrn)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	char tmp;


	/* Show cursor */
	if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
    {
    	tmp = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x81);
		VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x81, tmp | 0x80);

    	/* Program FPR copy when on the 730 */
    	if (pSmi->Chipset == SMI_COUGAR3DR)
    	{
    	    CARD32 fpr15c;

    	    /* turn on the top bit */
    	    fpr15c  = READ_FPR(pSmi, FPR15C);
    	    fpr15c |= FPR15C_MASK_HWCENABLE;
    	    WRITE_FPR(pSmi, FPR15C, fpr15c);
    	}
    }
    else	/* SMI 501 */
    {    
		CARD32 uiPanelTmp;
		CARD32 uiCrtTmp;

		if (!pSmi->IsSecondary) {
	    uiPanelTmp  = READ_DCR(pSmi, DCRF0);
		uiPanelTmp |= SMI501_MASK_HWCENABLE;
		WRITE_DCR(pSmi, DCRF0,  uiPanelTmp);
	    }else{
	    uiCrtTmp    = READ_DCR(pSmi, DCR230);
		uiCrtTmp   |= SMI501_MASK_HWCENABLE;
		WRITE_DCR(pSmi, DCR230, uiCrtTmp);
	    }
	}        

}

static void
SMI_HideCursor(ScrnInfoPtr pScrn)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	char tmp;


	/* Hide cursor */
	if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
    {
		tmp = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x81);
		VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x81, tmp & ~0x80);

    	/* Program FPR copy when on the 730 */
    	if (pSmi->Chipset == SMI_COUGAR3DR)
    	{
    	    CARD32 fpr15c;

    	    /* turn off the top bit */
    	    fpr15c  = READ_FPR(pSmi, FPR15C);
    	    fpr15c &= ~FPR15C_MASK_HWCENABLE;
    	    WRITE_FPR(pSmi, FPR15C, fpr15c);
    	}
	}
	else
	{
		CARD32 uiPanelTmp;
		CARD32 uiCrtTmp;

		if (!pSmi->IsSecondary) {
		uiPanelTmp	= READ_DCR(pSmi, DCRF0);

		/*
		 * Belcon: Fix #001
		 *   Save hardware cursor address register
		 */
		if(pSmi->dcrF0 == 0) {
			pSmi->dcrF0 = uiPanelTmp;
		}
		/* #001 ended */

		uiPanelTmp &= ~SMI501_MASK_HWCENABLE;
		WRITE_DCR(pSmi, DCRF0,	uiPanelTmp);
		}else{
		uiCrtTmp	= READ_DCR(pSmi, DCR230);

		/*
		 * Belcon: Fix #001
		 *   Save hardware cursor address register
		 */
		if(pSmi->dcr230 == 0) {
			pSmi->dcr230 = uiCrtTmp;
		}
		/* #001 ended */

		uiCrtTmp   &= ~SMI501_MASK_HWCENABLE;
		WRITE_DCR(pSmi, DCR230, uiCrtTmp);
		}

	}        

}

static void
SMI_SetCursorPosition(ScrnInfoPtr pScrn, int x, int y)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	int xoff, yoff;


	/* Calculate coordinates for rotation */
	switch (pSmi->rotate)
	{
		default:
			xoff = x;
			yoff = y;
			break;

		case SMI_ROTATE_CW:
			xoff = pSmi->ShadowHeight - y - MAX_CURSOR;
			yoff = x;
			break;

		case SMI_ROTATE_CCW:
			xoff = y;
			yoff = pSmi->ShadowWidth - x - MAX_CURSOR;
			break;
	}

	/* Program coordinates */
    if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
    {
		if (xoff >= 0)
		{
			VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x88, xoff & 0xFF);
			VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x89,
					(xoff >> 8) & 0x07);
		}
		else
		{
			VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x88,
					(-xoff) & (MAX_CURSOR - 1));
			VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x89, 0x08);
		}

		if (yoff >= 0)
		{
			VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x8A, yoff & 0xFF);
			VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x8B,
					(yoff >> 8) & 0x07);
		}
		else
		{
			VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x8A,
					(-yoff) & (MAX_CURSOR - 1));
			VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x8B, 0x08);
		}

    	/* Program FPR copy when on the 730 */
    	if (pSmi->Chipset == SMI_COUGAR3DR)
    	{
    	    CARD32 fpr158;

    	    if (xoff >= 0)
    	    {
    	        fpr158 = (xoff & FPR158_MASK_MAXBITS)<<16;
    	    }
    	    else
    	    {
    	        fpr158 = (((-xoff) & FPR158_MASK_MAXBITS) | FPR158_MASK_BOUNDARY)<<16;
    	    }

    	    if (yoff >= 0)
    	    {
    	        fpr158 |= (yoff & FPR158_MASK_MAXBITS);
    	    }
    	    else
    	    {
    	        fpr158 |= (((-yoff) & FPR158_MASK_MAXBITS) | FPR158_MASK_BOUNDARY);
    	    }

    	    /* Program combined coordinates */
    	    WRITE_FPR(pSmi, FPR158, fpr158);

    	}
	}
	else	/* SMI 501 Family */
	{
    	    CARD32 hwcLocVal;

    	    if (xoff >= 0)
    	    {
    	        hwcLocVal = (xoff & SMI501_MASK_MAXBITS);
    	    }
    	    else
    	    {
    	        hwcLocVal = (((-xoff) & SMI501_MASK_MAXBITS) | SMI501_MASK_BOUNDARY);
    	    }

    	    if (yoff >= 0)
    	    {
    	        hwcLocVal |= (yoff & SMI501_MASK_MAXBITS)<<16;
    	    }
    	    else
    	    {
    	        hwcLocVal |= (((-yoff) & SMI501_MASK_MAXBITS) | SMI501_MASK_BOUNDARY)<<16;
    	    }

    	    /* Program combined coordinates */
			if (!pSmi->IsSecondary) {
			WRITE_DCR(pSmi, DCRF4,	hwcLocVal);  /* Panel HWC Location */
			}else{
			WRITE_DCR(pSmi, DCR234, hwcLocVal);  /* CRT   HWC Location */
			}

    	
	}        

}

static void
SMI_SetCursorColors(ScrnInfoPtr pScrn, int bg, int fg)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	unsigned char packedFG, packedBG;

	if (bg == fg) {
		return;
	}

	if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
	{
		/* Pack the true color into 8 bit */
		packedFG = (fg & 0xE00000) >> 16
				 | (fg & 0x00E000) >> 11
				 | (fg & 0x0000C0) >> 6
				 ;
		packedBG = (bg & 0xE00000) >> 16
				 | (bg & 0x00E000) >> 11
				 | (bg & 0x0000C0) >> 6
				 ;

		/* Program the colors */
		VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x8C, packedFG);
		VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x8D, packedBG);

    	/* Program FPR copy when on the 730 */
    	if (pSmi->Chipset == SMI_COUGAR3DR)
    	{
    	    CARD32 fpr15c;

    	    fpr15c  = READ_FPR(pSmi, FPR15C) & FPR15C_MASK_HWCADDREN;
    	    fpr15c |= packedFG;
    	    fpr15c |= packedBG<<8;
    	    WRITE_FPR(pSmi, FPR15C, fpr15c);
    	}
	}
	else
	{
    	/* for the SMI501 HWCursor, there are 4 possible colors, one of which
         * is transparent:  M,S:  0,0 = Transparent
         *						  0,1 = color 1
         *						  1,0 = color 2
         *						  1,1 = color 3
         *  To simplify implementation, we use color2 == bg and
         *									   color3 == fg
         *	Color 1 is don't care, so we set it to color 2's value
         */
		unsigned  int packedFGBG;
        
		/* Pack the true color components into 16 bit RGB -- 5:6:5 */
		packedFGBG  =  (bg & 0xF80000) >> 8
				    |  (bg & 0x00FC00) >> 5
				    |  (bg & 0x0000F8) >> 3
				    ;

		packedFGBG |= (bg & 0xF80000) << 8
				    | (bg & 0x00FC00) << 11
				    | (bg & 0x0000F8) << 13
				    ;
                   
//		if (!pSmi->IsSecondary) {
			WRITE_DCR(pSmi, DCRF8,  packedFGBG);    /* Panel HWC Color 1,2 */
//		}else{
			WRITE_DCR(pSmi, DCR238, packedFGBG);    /* CRT	HWC Color 1,2 */
//		}

        
		packedFGBG  =  (fg & 0xF80000) >> 8
				    |  (fg & 0x00FC00) >> 5
				    |  (fg & 0x0000F8) >> 3
				    ;
//		if (!pSmi->IsSecondary) {
			WRITE_DCR(pSmi, DCRFC,	packedFGBG);	/* Panel HWC Color 3 */
//		}else{
		WRITE_DCR(pSmi, DCR23C, packedFGBG);	/* CRT	 HWC Color 3 */
//		}
	}        

}

Bool
SMI_HWCursorInit(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	SMIPtr pSmi = SMIPTR(pScrn);
	xf86CursorInfoPtr infoPtr;
	Bool ret;


	/* Create cursor infor record */
	infoPtr = xf86CreateCursorInfoRec();
	if (infoPtr == NULL)
	{
		return(FALSE);
	}

    pSmi->CursorInfoRec = infoPtr;

	/* Fill in the information */
    if (SMI_MSOC == pSmi->Chipset)
    {
        infoPtr->MaxWidth  = MAX_CURSOR_501;
    	infoPtr->MaxHeight = MAX_CURSOR_501;
    }
    else if (SMI_MSOCE == pSmi->Chipset)
    {
        infoPtr->MaxWidth  = MAX_CURSOR_750;
    	infoPtr->MaxHeight = MAX_CURSOR_750;
    }    
	else
	{
        infoPtr->MaxWidth  = MAX_CURSOR;
    	infoPtr->MaxHeight = MAX_CURSOR;
    } 	
    if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
    {    
		infoPtr->Flags	= HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_8
						| HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK
						| HARDWARE_CURSOR_AND_SOURCE_WITH_MASK
						| HARDWARE_CURSOR_BIT_ORDER_MSBFIRST
						| HARDWARE_CURSOR_INVERT_MASK;
		infoPtr->RealizeCursor	= SMI_RealizeCursor;
    }
    else
    {   
    	infoPtr->Flags	= HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_1
						| HARDWARE_CURSOR_TRUECOLOR_AT_8BPP
						| HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK; 
		infoPtr->RealizeCursor	= SMI_501_RealizeCursor; 
    }    

    infoPtr->SetCursorColors   = SMI_SetCursorColors;
    infoPtr->SetCursorPosition = SMI_SetCursorPosition;
    infoPtr->LoadCursorImage   = SMI_LoadCursorImage;
    infoPtr->HideCursor        = SMI_HideCursor;
    infoPtr->ShowCursor        = SMI_ShowCursor;
    infoPtr->UseHWCursor       = NULL;

	/* Proceed with cursor initialization */
    ret = xf86InitCursor(pScreen, infoPtr);
	/* Force to use hardware cursor */
	xf86ForceHWCursor(pScreen, TRUE);
	SMI_SetCursorColors(pScrn, 0xffffff, 0x0);

	return(ret);
}
