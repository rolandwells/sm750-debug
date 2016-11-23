/* Header:   //Mercury/Projects/archives/XFree86/4.0/smi_shadow.c-arc   1.10   30 Nov 2000 11:40:38   Frido  $ */

/*
Copyright (C) 1994-2000 The XFree86 Project, Inc.  All Rights Reserved.
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
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/siliconmotion/smi_shadow.c,v 1.2 2000/12/05 21:18:37 dawes Exp $ */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "xf86.h"
#include "xf86_OSproc.h"
#include "smi.h"
#if  XORG_VERSION_CURRENT <  XORG_VERSION_NUMERIC(1,7,0,0,0)
#include "xf86Resources.h"
#endif
#ifndef XSERVER_LIBPCIACCESS
//#if  XORG_VERSION_CURRENT <=  XORG_VERSION_NUMERIC(7,1,1,0,0)
#include "xf86_ansic.h"
#endif
#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "shadowfb.h"
#include "servermd.h"



#define  ROTBLTWIDTH     8


void SMI_RotateZERO(ScrnInfoPtr pScrn, int num, BoxPtr pbox)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	int width, height, srcX, srcY, destX, destY;
	int leftoverwidth;

	/* 0 degrees .  Calculate destination
					   coordinates:

						*---+
						|   |       +-----*
						|   |       |     |  destX = shadowWidth - srcX - 1
						|   |  -->  |     |  destY = shadowHeight - srcY - 1
						|   |       |     |
						|   |       +-----+
						+---+
					*/
    srcX   = pbox->x1;
    srcY   = pbox->y1;
    width  = pbox->x2 - srcX;
    height = pbox->y2 - srcY;
		
	//using bitblt derectly, needn't to seperate like using Rotate
	WaitQueue(4);
	WRITE_DPR(pSmi, 0x00, (srcX  << 16)  + srcY);
	WRITE_DPR(pSmi, 0x04, (srcX << 16)  + srcY);
	WRITE_DPR(pSmi, 0x08, (width << 16)  + height);
	//WRITE_DPR(pSmi, 0x0C, 0xCC | SMI_ROTATE_BLT |
	WRITE_DPR(pSmi, 0x0C, 0xCC | SMI_BITBLT |					
		SMI_ROTATE_ZERO | SMI_START_ENGINE);
	return;
}

void SMI_RotateCW(ScrnInfoPtr pScrn, int num, BoxPtr pbox)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	int width, height, srcX, srcY, destX, destY;
	int leftoverwidth;

    srcX   = pbox->x1;
    srcY   = pbox->y1;
    width  = pbox->x2 - srcX;
    height = pbox->y2 - srcY;

////////////////////////////////
	/* 90 degrees CW rotation.  Calculate destination
   coordinates:

	*---+
	|   |       +-----*
	|   |       |     |  destX = shadowHeight - srcY - 1
	|   |  -->  |     |  destY = srcX
	|   |       |     |
	|   |       +-----+
	+---+
*/
  if ((SMI_MSOC == pSmi->Chipset) || (SMI_MSOCE == pSmi->Chipset))
  {
  	if (srcX < 0 || srcY < 0)
  	{
  		if (srcX < 0)
  		{
  			width += srcX;
  			srcX = 0;
  		}
  		if (srcY < 0)
  		{
  			height += srcY;
  			srcY = 0;
  		}
  
  		if (width < 0 || height < 0)
  		{
  			/* Perform no operation if width or height is negative */
  			return;
  		}
  	}
  	
  	/* Adjust source coordinates to be 32-byte aligned for 501 HW */
  	width += srcX % (32/(pScrn->bitsPerPixel/8));
  	srcX  -= srcX % (32/(pScrn->bitsPerPixel/8));
  
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "shadowWidth %d, shadowHeight %d\n", pSmi->ShadowWidth, pSmi->ShadowHeight);
	
  	destX = pSmi->ShadowHeight - srcY - 1;
  	destY = srcX;
  
  	/* 501 has some wierdness where it won't clip correctly
  	 * at the bottom of the screen.  Need to do this in SW. */
  	if ((srcX + width) > pSmi->width)
  	{
  		width -= ((srcX+width) - pSmi->width);
  		if (width <= 0)
  		{
  			return;
  		}
  	}
  	if ((srcY + height) > pSmi->height)
  	{
  		height -= ((srcY + height) - pSmi->height);
  		if (height <= 0)
  		{
  			return;
  		}
  	}
  	
  
  	leftoverwidth = width;
  		                        
  	/* 501 Hardware cannot handle rotblits > 32 bytes */
  	while (leftoverwidth > ROTBLTWIDTH)
  	{
  	
  		WaitQueue(4);
  		WRITE_DPR(pSmi, 0x00, (srcX  << 16)  + srcY);
  		WRITE_DPR(pSmi, 0x04, (destX << 16)  + destY);
  		WRITE_DPR(pSmi, 0x08, (ROTBLTWIDTH << 16)  + height);
  		WRITE_DPR(pSmi, 0x0C, 0xCC | SMI_ROTATE_BLT |
  				 SMI_ROTATE_CW | SMI_START_ENGINE);
  
  		leftoverwidth -= ROTBLTWIDTH;
  		srcX          += ROTBLTWIDTH;
  		destY         = srcX;
  	}
  
  	WaitQueue(4);
  	WRITE_DPR(pSmi, 0x00, (srcX << 16)   + srcY);
  	WRITE_DPR(pSmi, 0x04, (destX << 16)  + destY);
  	WRITE_DPR(pSmi, 0x08, (leftoverwidth << 16)  + height);
  	WRITE_DPR(pSmi, 0x0C, 0xCC | SMI_ROTATE_BLT |
  			 SMI_ROTATE_CW | SMI_START_ENGINE);
  }
  else /* NOT SMOC 501 chipset */
  {
  	destX = pSmi->ShadowHeight - srcY - 1;
  	destY = srcX;
  
  	WaitQueue(4);
  	WRITE_DPR(pSmi, 0x00, (srcX  << 16)  + srcY);
  	WRITE_DPR(pSmi, 0x04, (destX << 16)  + destY);
  	WRITE_DPR(pSmi, 0x08, (width << 16)  + height);
  	WRITE_DPR(pSmi, 0x0C, 0xCC | SMI_ROTATE_BLT |
  			 SMI_ROTATE_CW | SMI_START_ENGINE);
  }	
    return;
}

void SMI_RotateCCW(ScrnInfoPtr pScrn, int num, BoxPtr pbox)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	int width, height, srcX, srcY, destX, destY;
	int leftoverwidth;

  srcX   = pbox->x1;
  srcY   = pbox->y1;
  width  = pbox->x2 - srcX;
  height = pbox->y2 - srcY;

////////////////////////////////
/* 90 degrees CCW rotatation.  Calculate destination
   coordinates:

	*---+
	|   |       +-----+
	|   |       |     |  destX = srcY
	|   |  -->  |     |  destY = shadowWidth - srcX - 1
	|   |       |     |
	|   |       *-----+
	+---+
*/

  if ((SMI_MSOC == pSmi->Chipset) || (SMI_MSOCE == pSmi->Chipset)) 
  {
  	if (srcX < 0 || srcY < 0)
  	{
  		if (srcX < 0)
  		{
  			width += srcX;
  			srcX = 0;
  		}
  		if (srcY < 0)
  		{
  			height += srcY;
  			srcY = 0;
  		}
  
  		if (width < 0 || height < 0)
  		{
  			/* Punt on blits with a negative width or height*/
  			return;
  		}
  	}						
  	
  
  	/* Adjust source coordinates to be 32-byte aligned for 501 HW */
  	width += srcX % (32/(pScrn->bitsPerPixel/8));
  	srcX  -= srcX % (32/(pScrn->bitsPerPixel/8));
  
  	destX = srcY;
  	destY = pSmi->ShadowWidth - srcX - 1;
            
  	/* 501 has some wierdness where it won't clip correctly
  	 * at the bottom of the screen.  Need to do this in SW. */
  	if ((srcX + width) > pSmi->width)
  	{
  		width -= ((srcX+width) - pSmi->width);
  		if (width <= 0)
  		{
  			return;
  		}
  	}
  	if ((srcY + height) > pSmi->height)
  	{
  		height -= ((srcY + height) - pSmi->height);
  		if (height <= 0)
		{
  			return;
		}
  	}
  	leftoverwidth = width;
  
  	/* 501 Hardware cannot handle rotblits > 32 bytes */
  	while (leftoverwidth > ROTBLTWIDTH)
  	{
  		WaitQueue(4);
  		WRITE_DPR(pSmi, 0x00, (srcX        << 16) + srcY);
  		WRITE_DPR(pSmi, 0x04, (destX       << 16) + destY);
  		WRITE_DPR(pSmi, 0x08, (ROTBLTWIDTH << 16) + height);
  		WRITE_DPR(pSmi, 0x0C, 0xCC | SMI_ROTATE_BLT |
  				 SMI_ROTATE_CCW | SMI_START_ENGINE);
  
  		leftoverwidth -= ROTBLTWIDTH;
  		srcX          += ROTBLTWIDTH;
  		destY          = pSmi->ShadowWidth - srcX - 1;
  		
  		if (destY < 0)
  		{
  			destY  = 0;
  			height-= 1;
  		}
  	}
  
  	if (leftoverwidth > 0)
  	{
  		WaitQueue(4);
  		WRITE_DPR(pSmi, 0x00, (srcX          << 16)  + srcY);
  		WRITE_DPR(pSmi, 0x04, (destX         << 16)  + destY);
  		WRITE_DPR(pSmi, 0x08, (leftoverwidth << 16)  + height);
  		WRITE_DPR(pSmi, 0x0C, 0xCC | SMI_ROTATE_BLT |
  				SMI_ROTATE_CCW | SMI_START_ENGINE);
  	}
  }
  else
  {
  	destX = srcY;
  	destY = pSmi->ShadowWidth - srcX - 1;
  
  	WaitQueue(4);
  	WRITE_DPR(pSmi, 0x00, (srcX  << 16)  + srcY);
  	WRITE_DPR(pSmi, 0x04, (destX << 16)  + destY);
  	WRITE_DPR(pSmi, 0x08, (width << 16)  + height);
  	WRITE_DPR(pSmi, 0x0C, 0xCC | SMI_ROTATE_BLT |
  			SMI_ROTATE_CCW | SMI_START_ENGINE);
  }


////////////////////////////////
    return;
}

void SMI_RotateUD(ScrnInfoPtr pScrn, int num, BoxPtr pbox)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	int width, height, srcX, srcY, destX, destY;
	int leftoverwidth;

  srcX   = pbox->x1;
  srcY   = pbox->y1;
  width  = pbox->x2 - srcX;
  height = pbox->y2 - srcY;
		
//////////////////////////////
/* 180 degrees UD rotation.  Calculate destination
   coordinates:

	*---+
	|   |       +-----*
	|   |       |     |  destX = shadowWidth - srcX - 1
	|   |  -->  |     |  destY = shadowHeight - srcY - 1
	|   |       |     |
	|   |       +-----+
	+---+
*/
  if ((SMI_MSOC == pSmi->Chipset) || (SMI_MSOCE == pSmi->Chipset))  
  {
  	if (srcX < 0 || srcY < 0)
  	{
  		if (srcX < 0)
  		{
  			width += srcX;
  			srcX = 0;
  		}
  		if (srcY < 0)
  		{
  			height += srcY;
  			srcY = 0;
  		}
  
  		if (width < 0 || height < 0)
  		{
  			/* Perform no operation if width or height is negative */
  			return;
  		}
  	}
  	
  	/* Adjust source coordinates to be 32-byte aligned for 501 HW */
  	width += srcX % (32/(pScrn->bitsPerPixel/8));
  	srcX  -= srcX % (32/(pScrn->bitsPerPixel/8));
  
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "shadowWidth %d, shadowHeight %d\n", pSmi->ShadowWidth, pSmi->ShadowHeight);
  	//destX = pSmi->ShadowHeight - srcX - 1;
  	destX = pSmi->ShadowWidth - srcX - 1;
  	//destY = srcX;
  	destY = pSmi->ShadowHeight - srcY - 1;
  
  	/* 501 has some wierdness where it won't clip correctly
  	 * at the bottom of the screen.  Need to do this in SW. */
  	if ((srcX + width) > pSmi->width)
  	{
  		width -= ((srcX+width) - pSmi->width);
  		if (width <= 0)
  		{
  			return;
  		}
  	}
  	if ((srcY + height) > pSmi->height)
  	{
  		height -= ((srcY + height) - pSmi->height);
  		if (height <= 0)
  		{
  			return;
  		}
  	}
  	
  
  	leftoverwidth = width;
  		                        
  	/* 501 Hardware cannot handle rotblits > 32 bytes */
  	while (leftoverwidth > ROTBLTWIDTH)
  	{
  	
  		WaitQueue(4);
  		WRITE_DPR(pSmi, 0x00, (srcX  << 16)  + srcY);
  		WRITE_DPR(pSmi, 0x04, (destX << 16)  + destY);
  		WRITE_DPR(pSmi, 0x08, (ROTBLTWIDTH << 16)  + height);
  		WRITE_DPR(pSmi, 0x0C, 0xCC | SMI_ROTATE_BLT |
  				 SMI_ROTATE_UD | SMI_START_ENGINE);
  
  		leftoverwidth -= ROTBLTWIDTH;
  		srcX          += ROTBLTWIDTH;
  		//destY         = srcX;
  		destX = pSmi->ShadowWidth - srcX - 1;
  		if (destX < 0)
  		{
  			destX  = 0;
  		}
  
  	}
  	if (leftoverwidth > 0)
  	{
  		WaitQueue(4);
  		WRITE_DPR(pSmi, 0x00, (srcX << 16)   + srcY);
  		WRITE_DPR(pSmi, 0x04, (destX << 16)  + destY);
  		WRITE_DPR(pSmi, 0x08, (leftoverwidth << 16)  + height);
  		WRITE_DPR(pSmi, 0x0C, 0xCC | SMI_ROTATE_BLT |
  			 SMI_ROTATE_UD | SMI_START_ENGINE);
  	}
  }
  else /* NOT SMOC 501 chipset */
  {
  	destX = pSmi->ShadowHeight - srcY - 1;
  	destY = srcX;
  
  	WaitQueue(4);
  	WRITE_DPR(pSmi, 0x00, (srcX  << 16)  + srcY);
  	WRITE_DPR(pSmi, 0x04, (destX << 16)  + destY);
  	WRITE_DPR(pSmi, 0x08, (width << 16)  + height);
  	WRITE_DPR(pSmi, 0x0C, 0xCC | SMI_ROTATE_BLT |
  			 SMI_ROTATE_UD | SMI_START_ENGINE);
  }	
//////////////////////////////
    return;
}


/******************************************************************************\
|* SMI_RefreshArea
|*=============================================================================
|*
|*  PARAMETERS:		pScrn	Pointer to ScrnInfo structure.
|*					num		Number of boxes to refresh.
|*					pbox	Pointer to an array of boxes to refresh.
|*
|*  DESCRIPTION:	Refresh a portion of the shadow buffer to the visual screen
|*					buffer.  This is mainly used for rotation purposes.
|*												y
|*  RETURNS:		Nothing.
|*
\******************************************************************************/
void SMI_RefreshArea(ScrnInfoPtr pScrn, int num, BoxPtr pbox)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	int width, height, srcX, srcY, destX, destY;
	int leftoverwidth;

	xf86DrvMsg(pScrn->scrnIndex, X_INFO," caesar (%d,%d,%d,%d) \n", pbox->x1, pbox->y1, pbox->x2, pbox->y2); 
	//xf86DrvMsg(pScrn->scrnIndex, X_ERROR," caesar 0x%X \n", pSmi->rotate); 

	/* #671 */
	if (pSmi->polyLines)
	{
		pSmi->polyLines = FALSE;
		return;
	}

	if (pSmi->rotate || pSmi->RandRRotation)	
	{
		/* IF we need to do rotation, setup the hardware here. */
		WaitIdleEmpty();
xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Shadowpitch is 0x%x  pSmi->ShadowWidth = %d, pSmi->ShadowHeight = %d\n", 
		pSmi	->ShadowPitch, pSmi->ShadowWidth, pSmi->ShadowHeight);
		WRITE_DPR(pSmi, 0x10, pSmi->ShadowPitch);
		WRITE_DPR(pSmi, 0x3C, pSmi->ShadowPitch);
//		WRITE_DPR(pSmi, 0x10, 0x4000400);
//		WRITE_DPR(pSmi, 0x3C, 0x4000400);
		
        
        if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
        {
		    WRITE_DPR(pSmi, 0x44, pSmi->FBOffset >> 3);
        }
        else
        {
/* changed by Belcon */
xf86DrvMsg(pScrn->scrnIndex, X_INFO, "2d dest base address is 0x%x(%d), 0x%x\n", pSmi->FBOffset, pSmi->FBOffset, pScrn->fbOffset);
		 // WRITE_DPR(pSmi, 0x44, pSmi->FBOffset);
		  WRITE_DPR(pSmi, 0x44, pScrn->fbOffset);
        }    
	}

	/* #672 */
	if (pSmi->ClipTurnedOn)
	{
		WaitQueue(1);
		WRITE_DPR(pSmi, 0x2C, pSmi->ScissorsLeft);
		pSmi->ClipTurnedOn = FALSE;
	}

	while (num--)
	{
		/* Get coordinates of the box to refresh. */
		srcX   = pbox->x1;
		srcY   = pbox->y1;
		width  = pbox->x2 - srcX;
		height = pbox->y2 - srcY;

		DEBUG("x=%d y=%d w=%d h=%d\n", srcX, srcY, width, height);


		if ((width > 0) && (height > 0))
		{
			switch (pSmi->rotate)
			{
				case SMI_ROTATE_ZERO:
					//SMI_RotateZero(pScrn, num, pbox);
					SMI_RotateZERO(pScrn, num, pbox);
					break;
					
				case SMI_ROTATE_CW:
					SMI_RotateCW(pScrn, num, pbox);
					break;

				case SMI_ROTATE_CCW:
					SMI_RotateCCW(pScrn, num, pbox);
					break;

				case SMI_ROTATE_UD:
					SMI_RotateUD(pScrn, num, pbox);
					break;					

				default:
					/* No rotation, perform a normal copy. */
					if (pScrn->bitsPerPixel == 24)
					{
						srcX  *= 3;
						width *= 3;

						if (pSmi->Chipset == SMI_LYNX)
						{
							srcY *= 3;
						}
					}

					WaitQueue(4);
					WRITE_DPR(pSmi, 0x00, (srcX << 16)  + srcY);
					WRITE_DPR(pSmi, 0x04, (srcX << 16)  + srcY);
					WRITE_DPR(pSmi, 0x08, (width << 16) + height);
					WRITE_DPR(pSmi, 0x0C, SMI_BITBLT + SMI_START_ENGINE + 0xCC);
					break;
			}
		}
		pbox++;
	}

	if (pSmi->rotate || pSmi->RandRRotation)	
	{
		/* If we did a rotation, we need to restore the hardware state here. */
		WaitIdleEmpty();
		WRITE_DPR(pSmi, 0x10, (pSmi->Stride << 16) | pSmi->Stride);
		WRITE_DPR(pSmi, 0x3C, (pSmi->Stride << 16) | pSmi->Stride);
		WRITE_DPR(pSmi, 0x44, 0);
	}
}


/* Custom version for the 730 series (Cougar3DR).
   This chipset has problems with large rotate-blts. */

void SMI_RefreshArea730(ScrnInfoPtr pScrn, int num, BoxPtr pbox)
{
       SMIPtr pSmi = SMIPTR(pScrn);
       int width, height, srcX, srcY, destX, destY;
       int maxPixels, tempWidth;


       /* #671 */
       if (pSmi->polyLines)
       {
               pSmi->polyLines = FALSE;
               return;
       }

       if (pSmi->rotate)
       {
               /* IF we need to do rotation, setup the hardware here. */
              WaitIdleEmpty();
               WRITE_DPR(pSmi, 0x10, pSmi->ShadowPitch);
               WRITE_DPR(pSmi, 0x3C, pSmi->ShadowPitch);
              WRITE_DPR(pSmi, 0x44, pSmi->FBOffset >> 3);
       }

       /* #672 */
       if (pSmi->ClipTurnedOn)
       {
               WaitQueue(1);
               WRITE_DPR(pSmi, 0x2C, pSmi->ScissorsLeft);
               pSmi->ClipTurnedOn = FALSE;
       }

       /* SM731 cannot rotate-blt more than a certain number of pixels
          (based on a calculation from the Windows driver source */
       maxPixels = 1280 / pScrn->bitsPerPixel;

       while (num--)
       {
               /* Get coordinates of the box to refresh. */
               srcX   = pbox->x1;
               srcY   = pbox->y1;
               width  = pbox->x2 - srcX;
               height = pbox->y2 - srcY;

               DEBUG("x=%d y=%d w=%d h=%d\n", srcX, srcY, width, height);

               if ((width > 0) && (height > 0))
               {
                       switch (pSmi->rotate)
                       {
                               case SMI_ROTATE_CW:
                                       /* 90 degrees CW rotation.  Calculate destination
                                          coordinates:

                                               *---+
                                               |   |       +-----*
                                               |   |       |     |  destX = shadowHeight - srcY - 1
                                               |   |  -->  |     |  destY = srcX
                                               |   |       |     |
                                               |   |       +-----+
                                               +---+
                                       */
                                       destX = pSmi->ShadowHeight - srcY - 1;
                                       destY = srcX;

                                       for (tempWidth=width; tempWidth>0;)
                                       {
                                               if (width>maxPixels)
                                                       width = maxPixels;
                                              WaitQueue(4);
                                               WRITE_DPR(pSmi, 0x00, (srcX << 16)   + srcY);
                                              WRITE_DPR(pSmi, 0x04, (destX << 16)  + destY);
                                               WRITE_DPR(pSmi, 0x08, (width << 16)  + height);
                                               WRITE_DPR(pSmi, 0x0C, 0xCC | SMI_ROTATE_BLT |
                                                               SMI_ROTATE_CW | SMI_START_ENGINE);
                                               destY     += maxPixels;
                                               srcX      += maxPixels;
                                               tempWidth -= maxPixels;
                                               width      = tempWidth;
                                       }

                                       break;

                               case SMI_ROTATE_CCW:
                                       /* 90 degrees CCW rotatation.  Calculate destination
                                          coordinates:

                                               *---+
                                               |   |       +-----+
                                               |   |       |     |  destX = srcY
                                               |   |  -->  |     |  destY = shadowWidth - srcX - 1
                                               |   |       |     |
                                               |   |       *-----+
                                               +---+
                                       */
                                       destX = srcY;
                                       destY = pSmi->ShadowWidth - srcX - 1;

                                       for (tempWidth=width; tempWidth>0;)
                                       {
                                               if (width>maxPixels)
                                                       width = maxPixels;
                                               WaitQueue(4);
                                               WRITE_DPR(pSmi, 0x00, (srcX << 16)   + srcY);
                                               WRITE_DPR(pSmi, 0x04, (destX << 16)  + destY);
                                               WRITE_DPR(pSmi, 0x08, (width << 16)  + height);
                                               WRITE_DPR(pSmi, 0x0C, 0xCC | SMI_ROTATE_BLT |
                                                               SMI_ROTATE_CCW | SMI_START_ENGINE);
                                               destY     -= maxPixels;
                                               srcX      += maxPixels;
                                               tempWidth -= maxPixels;
                                               width      = tempWidth;
                                       }

                                       break;

                               default:
                                       /* No rotation, perform a normal copy. */
                                       if (pScrn->bitsPerPixel == 24)
                                       {
                                               srcX  *= 3;
                                               width *= 3;

                                               if (pSmi->Chipset == SMI_LYNX)
                                               {
                                                       srcY *= 3;
                                               }
                                       }

                                       WaitQueue(4);
                                       WRITE_DPR(pSmi, 0x00, (srcX << 16)  + srcY);
                                       WRITE_DPR(pSmi, 0x04, (srcX << 16)  + srcY);
                                       WRITE_DPR(pSmi, 0x08, (width << 16) + height);
                                       WRITE_DPR(pSmi, 0x0C, SMI_BITBLT + SMI_START_ENGINE + 0xCC);
                                       break;
                       }
               }

               pbox++;
       }

       if (pSmi->rotate)
       {
               /* If we did a rotation, we need to restore the hardware state here. */
               WaitIdleEmpty();
               WRITE_DPR(pSmi, 0x10, (pSmi->Stride << 16) | pSmi->Stride);
               WRITE_DPR(pSmi, 0x3C, (pSmi->Stride << 16) | pSmi->Stride);
               WRITE_DPR(pSmi, 0x44, 0);
       }

}


/******************************************************************************\
|* SMI_PointerMoved
|*=============================================================================
|*
|*  PARAMETERS:		index	Index of current screen.
|*					x		X location of pointer.
|*					y		Y location of pointer.
|*
|*  DESCRIPTION:	Adjust the pointer location if we are in rotation mode.
|*
|*  RETURNS:		Nothing.
|*
\******************************************************************************/
void SMI_PointerMoved(int index, int x, int y)
{
    ScrnInfoPtr pScrn = xf86Screens[index];
    SMIPtr pSmi = SMIPTR(pScrn);
    int newX, newY;

	switch (pSmi->rotate)
	{
		case SMI_ROTATE_CW:
			/* 90 degrees CW rotation. */
			newX = pScrn->pScreen->height - y - 1;
			newY = x;
			break;

		case SMI_ROTATE_CCW:
			/* 90 degrees CCW rotation. */
			newX = y;
			newY = pScrn->pScreen->width - x - 1;
			break;

		default:
			/* No rotation. */
			newX = x;
			newY = y;
			break;
  }

	/* Pass adjusted pointer coordinates original PointerMoved function. */
  if(pSmi->PointerMoved)
  {
    (*pSmi->PointerMoved)(index, newX, newY);
  }
}
