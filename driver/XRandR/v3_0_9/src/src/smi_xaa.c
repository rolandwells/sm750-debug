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
authorization from the XFree86 Project and silicon Motion.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "smi.h"
#include "smi_501.h"
#include "smi750.h"
#include "smilynx.h"

#include "miline.h"
#include "xaalocal.h"
#include "xaarop.h"
#include "servermd.h"

static void SMI750_SetupForScreenToScreenCopy(ScrnInfoPtr, int, int, int,
					   unsigned int, int);
static void SMI750_SubsequentScreenToScreenCopy(ScrnInfoPtr, int, int, int, int,
					     int, int);
static void SMI750_SetupForSolidFill(ScrnInfoPtr, int, int, unsigned);
static void SMI750_SubsequentSolidFillRect(ScrnInfoPtr, int, int, int, int);
static void SMI750_SetupForSolidLine(ScrnInfoPtr, int, int, unsigned);
static void SMI750_SubsequentSolidHorVertLine(ScrnInfoPtr, int, int, int, int);
static void SMI750_SetupForCPUToScreenColorExpandFill(ScrnInfoPtr, int, int, int,
						   unsigned int);
static void SMI750_SubsequentCPUToScreenColorExpandFill(ScrnInfoPtr, int, int, int,
						     int, int);
static void SMI750_SetupForMono8x8PatternFill(ScrnInfoPtr, int, int, int, int, int,
					   unsigned int);
static void SMI750_SubsequentMono8x8PatternFillRect(ScrnInfoPtr, int, int, int,
						 int, int, int);
static void SMI750_SetupForColor8x8PatternFill(ScrnInfoPtr, int, int, int,
					    unsigned int, int);
static void SMI750_SubsequentColor8x8PatternFillRect(ScrnInfoPtr, int, int, int,
						  int, int, int);
static void SMI712_SetupForScreenToScreenCopy(ScrnInfoPtr, int, int, int,
					   unsigned int, int);
static void SMI712_SubsequentScreenToScreenCopy(ScrnInfoPtr, int, int, int, int,
					     int, int);
static void SMI712_SetupForSolidFill(ScrnInfoPtr, int, int, unsigned);
static void SMI712_SubsequentSolidFillRect(ScrnInfoPtr, int, int, int, int);
static void SMI712_SetupForSolidLine(ScrnInfoPtr, int, int, unsigned);
static void SMI712_SubsequentSolidHorVertLine(ScrnInfoPtr, int, int, int, int);
static void SMI712_SetupForCPUToScreenColorExpandFill(ScrnInfoPtr, int, int, int,
						   unsigned int);
static void SMI712_SubsequentCPUToScreenColorExpandFill(ScrnInfoPtr, int, int, int,
						     int, int);
static void SMI712_SetupForMono8x8PatternFill(ScrnInfoPtr, int, int, int, int, int,
					   unsigned int);
static void SMI712_SubsequentMono8x8PatternFillRect(ScrnInfoPtr, int, int, int,
						 int, int, int);
static void SMI712_SetupForColor8x8PatternFill(ScrnInfoPtr, int, int, int,
					    unsigned int, int);
static void SMI712_SubsequentColor8x8PatternFillRect(ScrnInfoPtr, int, int, int,
						  int, int, int);
						  
#if SMI_USE_IMAGE_WRITES
static void SMI750_SetupForImageWrite(ScrnInfoPtr, int, unsigned int, int, int,
				   int);
static void SMI750_SubsequentImageWriteRect(ScrnInfoPtr, int, int, int, int, int);
static void SMI712_SetupForImageWrite(ScrnInfoPtr, int, unsigned int, int, int,
				   int);
static void SMI712_SubsequentImageWriteRect(ScrnInfoPtr, int, int, int, int, int);
#endif

#if 0
Bool
SMI_XAAInit(ScreenPtr pScreen)
{
    XAAInfoRecPtr infoPtr;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SMIPtr pSmi = SMIPTR(pScrn);
    /*BoxRec AvailFBArea;*/
    Bool ret;
    /*int numLines, maxLines;*/

    ENTER();

    pSmi->XAAInfoRec = infoPtr = XAACreateInfoRec();
    if (infoPtr == NULL)
	LEAVE(FALSE);

    infoPtr->Flags = PIXMAP_CACHE
		   | LINEAR_FRAMEBUFFER
		   | OFFSCREEN_PIXMAPS;

    infoPtr->Sync = SMI_AccelSync;


    if (xf86IsEntityShared(pScrn->entityList[0]))
	infoPtr->RestoreAccelState = SMI_EngineReset;
    /* Screen to screen copies */
    infoPtr->ScreenToScreenCopyFlags = NO_PLANEMASK |
                                        ONLY_TWO_BITBLT_DIRECTIONS;
    infoPtr->SetupForScreenToScreenCopy = SMI750_SetupForScreenToScreenCopy;
    infoPtr->SubsequentScreenToScreenCopy = SMI750_SubsequentScreenToScreenCopy;
    /*if (pScrn->bitsPerPixel == 24) {
	infoPtr->ScreenToScreenCopyFlags |= NO_TRANSPARENCY;
    }*/
    /*if ((pSmi->Chipset == SMI_LYNX3D) && (pScrn->bitsPerPixel == 8)) {
	infoPtr->ScreenToScreenCopyFlags |= GXCOPY_ONLY;
    }*/

    /* Solid Fills */
    
#if 1    
    infoPtr->SolidFillFlags = NO_PLANEMASK;
    infoPtr->SetupForSolidFill = SMI750_SetupForSolidFill;
    infoPtr->SubsequentSolidFillRect = SMI750_SubsequentSolidFillRect;
#endif
#if 0

    /* Solid Lines */
#if 1    
    infoPtr->SolidLineFlags = NO_PLANEMASK;
    infoPtr->SetupForSolidLine = SMI_SetupForSolidLine;
    infoPtr->SubsequentSolidHorVertLine = SMI_SubsequentSolidHorVertLine;
#endif

    /* Color Expansion Fills */
#if 1    
    /*infoPtr->CPUToScreenColorExpandFillFlags = ROP_NEEDS_SOURCE
					     | NO_PLANEMASK
					     | BIT_ORDER_IN_BYTE_MSBFIRST
					     | LEFT_EDGE_CLIPPING
					     | CPU_TRANSFER_PAD_DWORD
					     | SCANLINE_PAD_DWORD;*/
    infoPtr->CPUToScreenColorExpandFillFlags = NO_PLANEMASK | 
                                                BIT_ORDER_IN_BYTE_MSBFIRST |
                                                LEFT_EDGE_CLIPPING |
                                                CPU_TRANSFER_PAD_DWORD |
                                                SCANLINE_PAD_DWORD ;
    infoPtr->ColorExpandBase = pSmi->DataPortBase;
    infoPtr->ColorExpandRange = pSmi->DataPortSize;
    infoPtr->SetupForCPUToScreenColorExpandFill =
	    SMI_SetupForCPUToScreenColorExpandFill;
    infoPtr->SubsequentCPUToScreenColorExpandFill =
	    SMI_SubsequentCPUToScreenColorExpandFill;
#endif

    /* 8x8 Mono Pattern Fills */
#if 1    
    infoPtr->Mono8x8PatternFillFlags = NO_PLANEMASK
				     | HARDWARE_PATTERN_PROGRAMMED_BITS
				     | HARDWARE_PATTERN_SCREEN_ORIGIN
                                     | NO_TRANSPARENCY
				     | BIT_ORDER_IN_BYTE_MSBFIRST;
    infoPtr->SetupForMono8x8PatternFill = SMI_SetupForMono8x8PatternFill;
    infoPtr->SubsequentMono8x8PatternFillRect =
	SMI_SubsequentMono8x8PatternFillRect;
#endif
    /* 8x8 Color Pattern Fills */
#if 1    
    /*if (!SMI_LYNX3D_SERIES(pSmi->Chipset) || (pScrn->bitsPerPixel != 24)) {*/
    infoPtr->Color8x8PatternFillFlags = NO_PLANEMASK
        | HARDWARE_PATTERN_SCREEN_ORIGIN;
    infoPtr->SetupForColor8x8PatternFill =
        SMI_SetupForColor8x8PatternFill;
    infoPtr->SubsequentColor8x8PatternFillRect =
        SMI_SubsequentColor8x8PatternFillRect;
    //}*/
#endif    

#if SMI_USE_IMAGE_WRITES
    /* Image Writes */
    /*infoPtr->ImageWriteFlags = ROP_NEEDS_SOURCE*/
    infoPtr->ImageWriteFlags = NO_PLANEMASK
                             | LEFT_EDGE_CLIPPING
                             | CPU_TRANSFER_BASE_FIXED
			     | CPU_TRANSFER_PAD_QWORD
			     | SCANLINE_PAD_DWORD;
                             //| NO_GXCOPY;
    infoPtr->ImageWriteBase = pSmi->DataPortBase;
    infoPtr->ImageWriteRange = pSmi->DataPortSize;
    infoPtr->SetupForImageWrite = SMI_SetupForImageWrite;
    infoPtr->SubsequentImageWriteRect = SMI_SubsequentImageWriteRect;
#endif

    /* Clipping */
#if 1    
    infoPtr->ClippingFlags = HARDWARE_CLIP_SCREEN_TO_SCREEN_COPY
			   | HARDWARE_CLIP_MONO_8x8_FILL
			   | HARDWARE_CLIP_COLOR_8x8_FILL
			   | HARDWARE_CLIP_SOLID_FILL
			   | HARDWARE_CLIP_SOLID_LINE
			   | HARDWARE_CLIP_DASHED_LINE;
    infoPtr->SetClippingRectangle = SMI_SetClippingRectangle;
    infoPtr->DisableClipping = SMI_DisableClipping;

    /* Pixmap Cache */
    /*if (pScrn->bitsPerPixel == 24) {
	infoPtr->CachePixelGranularity = 16;
    } else {*/
    infoPtr->CachePixelGranularity = 128 / pScrn->bitsPerPixel;
    //}
#endif

#endif
    /* Offscreen Pixmaps */
    infoPtr->maxOffPixWidth  = 4096;
    infoPtr->maxOffPixHeight = 4096;
    /*if (pScrn->bitsPerPixel == 24) {
	infoPtr->maxOffPixWidth = 4096 / 3;

	if (pSmi->Chipset == SMI_LYNX) {
	    infoPtr->maxOffPixHeight = 4096 / 3;
	}
    }*/

    deInit();

    SMI_EngineReset(pScrn);

    ret = XAAInit(pScreen, infoPtr);

    LEAVE(ret);
}
#else


Bool
SMI_XAAInit(ScreenPtr pScreen)
{
    XAAInfoRecPtr infoPtr;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SMIPtr pSmi = SMIPTR(pScrn);
    /*BoxRec AvailFBArea;*/
    Bool ret;
    /*int numLines, maxLines;*/

    ENTER();

    pSmi->XAAInfoRec = infoPtr = XAACreateInfoRec();    
    if (infoPtr == NULL)
    	LEAVE(FALSE);

    infoPtr->Flags = PIXMAP_CACHE
		   | LINEAR_FRAMEBUFFER
		   | OFFSCREEN_PIXMAPS;  

    if (xf86IsEntityShared(pScrn->entityList[0]))
    	infoPtr->RestoreAccelState = SMI_EngineReset;
    	
    /* Screen to screen copies */
    infoPtr->ScreenToScreenCopyFlags = NO_PLANEMASK |
                                        ONLY_TWO_BITBLT_DIRECTIONS;
    
    /* Clipping */
#if 1    
    infoPtr->ClippingFlags = HARDWARE_CLIP_SCREEN_TO_SCREEN_COPY
			   | HARDWARE_CLIP_MONO_8x8_FILL
			   | HARDWARE_CLIP_COLOR_8x8_FILL
			   | HARDWARE_CLIP_SOLID_FILL
			   | HARDWARE_CLIP_SOLID_LINE
			   | HARDWARE_CLIP_DASHED_LINE;
    infoPtr->SetClippingRectangle = SMI_SetClippingRectangle;
    infoPtr->DisableClipping = SMI_DisableClipping;
#endif

    if(IS_SM750(pSmi)|| IS_SM718(pSmi))
    {
        infoPtr->Sync = SMI750_AccelSync;
        
        infoPtr->SetupForScreenToScreenCopy = SMI750_SetupForScreenToScreenCopy;
        infoPtr->SubsequentScreenToScreenCopy = SMI750_SubsequentScreenToScreenCopy;        
#if 1    
        infoPtr->SolidFillFlags = NO_PLANEMASK;
        infoPtr->SetupForSolidFill = SMI750_SetupForSolidFill;
        infoPtr->SubsequentSolidFillRect = SMI750_SubsequentSolidFillRect;
#endif
#if 1    
        infoPtr->SolidLineFlags = NO_PLANEMASK;
        infoPtr->SetupForSolidLine = SMI750_SetupForSolidLine;
        infoPtr->SubsequentSolidHorVertLine = SMI750_SubsequentSolidHorVertLine;
#endif
        
#if 1    
        /*infoPtr->CPUToScreenColorExpandFillFlags = ROP_NEEDS_SOURCE
                             | NO_PLANEMASK
                             | BIT_ORDER_IN_BYTE_MSBFIRST
                             | LEFT_EDGE_CLIPPING
                             | CPU_TRANSFER_PAD_DWORD
                             | SCANLINE_PAD_DWORD;*/
        infoPtr->CPUToScreenColorExpandFillFlags = NO_PLANEMASK | 
                                                    BIT_ORDER_IN_BYTE_MSBFIRST |
                                                    LEFT_EDGE_CLIPPING |
                                                    CPU_TRANSFER_PAD_DWORD |
                                                    NO_TRANSPARENCY |
                                                    SCANLINE_PAD_DWORD ;
        infoPtr->ColorExpandBase = pSmi->DataPortBase;
        infoPtr->ColorExpandRange = pSmi->DataPortSize;
        infoPtr->SetupForCPUToScreenColorExpandFill =
            SMI750_SetupForCPUToScreenColorExpandFill;
        infoPtr->SubsequentCPUToScreenColorExpandFill =
            SMI750_SubsequentCPUToScreenColorExpandFill;
#endif

#if 1    
        infoPtr->Mono8x8PatternFillFlags = NO_PLANEMASK
                         | HARDWARE_PATTERN_PROGRAMMED_BITS
                         | HARDWARE_PATTERN_SCREEN_ORIGIN
                                         | NO_TRANSPARENCY
                         | BIT_ORDER_IN_BYTE_MSBFIRST;
        infoPtr->SetupForMono8x8PatternFill = SMI750_SetupForMono8x8PatternFill;
        infoPtr->SubsequentMono8x8PatternFillRect = SMI750_SubsequentMono8x8PatternFillRect;
#endif
#if 1    
        /*if (!SMI_LYNX3D_SERIES(pSmi->Chipset) || (pScrn->bitsPerPixel != 24)) {*/
        infoPtr->Color8x8PatternFillFlags = NO_PLANEMASK
            | HARDWARE_PATTERN_SCREEN_ORIGIN;
        infoPtr->SetupForColor8x8PatternFill = SMI750_SetupForColor8x8PatternFill;
        infoPtr->SubsequentColor8x8PatternFillRect = SMI750_SubsequentColor8x8PatternFillRect;
        //}*/
#endif    
#if SMI_USE_IMAGE_WRITES        
        /*infoPtr->ImageWriteFlags = ROP_NEEDS_SOURCE*/
        infoPtr->ImageWriteFlags = NO_PLANEMASK
                                 | LEFT_EDGE_CLIPPING
                                 | CPU_TRANSFER_BASE_FIXED
                     | CPU_TRANSFER_PAD_QWORD
                     | SCANLINE_PAD_DWORD;
                                 //| NO_GXCOPY;
        infoPtr->ImageWriteBase = pSmi->DataPortBase;
        infoPtr->ImageWriteRange = pSmi->DataPortSize;
        infoPtr->SetupForImageWrite = SMI750_SetupForImageWrite;
        infoPtr->SubsequentImageWriteRect = SMI750_SubsequentImageWriteRect;
#endif
        infoPtr->CachePixelGranularity = 128 / pScrn->bitsPerPixel;
        infoPtr->maxOffPixWidth  = 4096;
        infoPtr->maxOffPixHeight = 4096;
    }
    else if(IS_SM712(pSmi))
    {
        infoPtr->Sync = SMI712_AccelSync;
       
#if 1        
        /* Screen to screen copies */
        infoPtr->ScreenToScreenCopyFlags = NO_PLANEMASK
                         | ONLY_TWO_BITBLT_DIRECTIONS;
        infoPtr->SetupForScreenToScreenCopy = SMI712_SetupForScreenToScreenCopy;
        infoPtr->SubsequentScreenToScreenCopy = SMI712_SubsequentScreenToScreenCopy;
        if (pScrn->bitsPerPixel == 24) {
            infoPtr->ScreenToScreenCopyFlags |= NO_TRANSPARENCY;
        }
#endif        
#if 1        
        /* Solid Fills */
        infoPtr->SolidFillFlags = NO_PLANEMASK;
        infoPtr->SetupForSolidFill = SMI712_SetupForSolidFill;
        infoPtr->SubsequentSolidFillRect = SMI712_SubsequentSolidFillRect;
#endif

#if 1
        
        /* Solid Lines */
        infoPtr->SolidLineFlags = NO_PLANEMASK;
        infoPtr->SetupForSolidLine = SMI712_SetupForSolidFill;
        infoPtr->SubsequentSolidHorVertLine = SMI712_SubsequentSolidHorVertLine;
#endif

#if 1
        
        /* Color Expansion Fills */
        infoPtr->CPUToScreenColorExpandFillFlags = ROP_NEEDS_SOURCE
                             | NO_PLANEMASK
                             | BIT_ORDER_IN_BYTE_MSBFIRST
                             | LEFT_EDGE_CLIPPING
                             | CPU_TRANSFER_PAD_DWORD
                             | SCANLINE_PAD_DWORD;
        infoPtr->ColorExpandBase = pSmi->DataPortBase;
        infoPtr->ColorExpandRange = pSmi->DataPortSize;
        infoPtr->SetupForCPUToScreenColorExpandFill =
            SMI712_SetupForCPUToScreenColorExpandFill;
        infoPtr->SubsequentCPUToScreenColorExpandFill =
            SMI712_SubsequentCPUToScreenColorExpandFill;
#endif

#if 1
        
        /* 8x8 Mono Pattern Fills */
        infoPtr->Mono8x8PatternFillFlags = NO_PLANEMASK
                         | HARDWARE_PATTERN_PROGRAMMED_BITS
                         | HARDWARE_PATTERN_SCREEN_ORIGIN
                         | BIT_ORDER_IN_BYTE_MSBFIRST;
        infoPtr->SetupForMono8x8PatternFill = SMI712_SetupForMono8x8PatternFill;
        infoPtr->SubsequentMono8x8PatternFillRect =
        SMI712_SubsequentMono8x8PatternFillRect;

 #endif

 #if 1
        /* 8x8 Color Pattern Fills */
        //if (!SMI_LYNX3D_SERIES(pSmi->Chipset) || (pScrn->bitsPerPixel != 24))
        {
        infoPtr->Color8x8PatternFillFlags = NO_PLANEMASK
                          | HARDWARE_PATTERN_SCREEN_ORIGIN;
        infoPtr->SetupForColor8x8PatternFill =
            SMI712_SetupForColor8x8PatternFill;
        infoPtr->SubsequentColor8x8PatternFillRect =
            SMI712_SubsequentColor8x8PatternFillRect;
        }
#endif        
        /* Pixmap Cache */
        if (pScrn->bitsPerPixel == 24)
            infoPtr->CachePixelGranularity = 16;
        else 
            infoPtr->CachePixelGranularity = 128 / pScrn->bitsPerPixel;
            
        /* Offscreen Pixmaps */
        infoPtr->maxOffPixWidth  = 4096;
        infoPtr->maxOffPixHeight = 4096;
        if (pScrn->bitsPerPixel == 24){
            infoPtr->maxOffPixWidth = 4096 / 3;        
        }
    }
    else
    {       
                
    }

    if(IS_SM750(pSmi)|| IS_SM718(pSmi))
        deInit();
            
    SMI_EngineReset(pScrn);
    ret = XAAInit(pScreen, infoPtr);
    LEAVE(ret);
}
#endif

/******************************************************************************/
/*	Screen to Screen Copies						      */
/******************************************************************************/
static void SMI712_SetupForScreenToScreenCopy
(ScrnInfoPtr pScrn, int xdir, int ydir, int rop,unsigned int planemask, int trans)
{
    CARD32 stride;
    SMIPtr pSmi = SMIPTR(pScrn);
    ENTER();
    DEBUG("xdir=%d ydir=%d rop=%02X trans=%08X\n", xdir, ydir, rop, trans);

#if __BYTE_ORDER == __BIG_ENDIAN
    if (pScrn->depth >= 24)
    trans = lswapl(trans);
#endif
    pSmi->AccelCmd = XAAGetCopyROP(rop)
           | SMI_BITBLT
           | SMI_START_ENGINE;

    if ((xdir == -1) || (ydir == -1)) {
        pSmi->AccelCmd |= SMI_RIGHT_TO_LEFT;
    }

    stride = pScrn->displayWidth | (pScrn->displayWidth<<16);

    SMI712_AccelSync(pScrn);

    if (trans != -1) 
    {
        pSmi->AccelCmd |= SMI_TRANSPARENT_SRC | SMI_TRANSPARENT_PXL;
        //WaitQueue();    
        WRITE_DPR(pSmi, 0x20, trans);
    }

    if (pSmi->ClipTurnedOn) 
    {
        //WaitQueue();
        WRITE_DPR(pSmi, 0x2C, pSmi->ScissorsLeft);
        WRITE_DPR(pSmi,0x2e,pSmi->ScissorsRight); //monk add 
        pSmi->ClipTurnedOn = FALSE;
    }
    
    WRITE_DPR(pSmi,0x10,stride);
    WRITE_DPR(pSmi,0x3c,stride);
    
    LEAVE();    
}


static void
SMI750_SetupForScreenToScreenCopy(ScrnInfoPtr pScrn, int xdir, int ydir, int rop,
			       unsigned int planemask, int trans)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
    DEBUG(" Scrn2Scrn: xdir=%d ydir=%d rop=%02X trans=%08X\n", xdir, ydir, rop, trans);

#if __BYTE_ORDER == __BIG_ENDIAN
    if (pScrn->depth >= 24)
	trans = lswapl(trans);
#endif
    /*
        pSmi->AccelCmd = XAAGetCopyROP(rop)
		   | SMI_BITBLT
		   | SMI_START_ENGINE;
        */
    pSmi->AccelCmd = FIELD_VALUE(0, DE_CONTROL, ROP, XAAGetCopyROP(rop)) |
                        FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP3) |
                        FIELD_SET(0, DE_CONTROL, COMMAND, BITBLT) |
                        FIELD_SET(0, DE_CONTROL, STATUS, START);
    

    if (trans != -1)
    {
    	/*pSmi->AccelCmd |= SMI_TRANSPARENT_SRC | SMI_TRANSPARENT_PXL;*/
    	//WaitQueue();
        //deWaitForNotBusy();
    	//WRITE_DPR(pSmi, 0x20, trans);
        deSetTransparency(1, 0, 1, trans);
    }
    else 
    {
        deSetTransparency(0, 0, 1, trans);
    }
    
    if ((xdir == -1) || (ydir == -1)) {
        pSmi->AccelCmd = FIELD_SET(pSmi->AccelCmd, DE_CONTROL, DIRECTION,
                                    RIGHT_TO_LEFT);
    } else {
        pSmi->AccelCmd = FIELD_SET(pSmi->AccelCmd, DE_CONTROL, DIRECTION,
                                    LEFT_TO_RIGHT);
    }

    if (pSmi->ClipTurnedOn) 
    {
    	//WaitQueue();
        //deWaitForNotBusy();
        //deWaitForNotBusy();
    	//WRITE_DPR(pSmi, 0x2C, pSmi->ScissorsLeft);
        deSetClipping(1, pSmi->ScissorsLeft&0xFFFF, pSmi->ScissorsLeft>>16, 
                        pSmi->ScissorsRight&0xFFFF, pSmi->ScissorsRight>>16);
    	pSmi->ClipTurnedOn = FALSE;
    }
    pSmi->AccelCmd |= deGetTransparency();
    WRITE_DPR(pSmi, DE_PITCH & 0xff, 
        (FIELD_VALUE(0, DE_PITCH, DESTINATION, (pScrn->displayWidth)) |
        (FIELD_VALUE(0, DE_PITCH, SOURCE, (pScrn->displayWidth)))));
    WRITE_DPR(pSmi, DE_WINDOW_WIDTH&0xff, 
        (FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (pScrn->displayWidth)) |
        (FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE, (pScrn->displayWidth)))));

    LEAVE();
}

static void
SMI712_SubsequentScreenToScreenCopy(ScrnInfoPtr pScrn, int x1, int y1, int x2,
				 int y2, int w, int h)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
    DEBUG("x1=%d y1=%d x2=%d y2=%d w=%d h=%d\n", x1, y1, x2, y2, w, h);

    if (pSmi->AccelCmd & SMI_RIGHT_TO_LEFT) {
	x1 += w - 1;
	y1 += h - 1;
	x2 += w - 1;
	y2 += h - 1;
    }

    if (pScrn->bitsPerPixel == 24) {
	x1 *= 3;
	x2 *= 3;
	w  *= 3;

	if (pSmi->Chipset == SMI_LYNX) {
	    y1 *= 3;
	    y2 *= 3;
	}

	if (pSmi->AccelCmd & SMI_RIGHT_TO_LEFT) {
	    x1 += 2;
	    x2 += 2;
	}
    }

    SMI712_AccelSync(pScrn);
    WRITE_DPR(pSmi, 0x00, (x1 << 16) + (y1 & 0xFFFF));
    WRITE_DPR(pSmi, 0x04, (x2 << 16) + (y2 & 0xFFFF));
    WRITE_DPR(pSmi, 0x08, (w  << 16) + (h  & 0xFFFF));
    WRITE_DPR(pSmi, 0x0C, pSmi->AccelCmd);

    LEAVE();
}


static void
SMI750_SubsequentScreenToScreenCopy(ScrnInfoPtr pScrn, int x1, int y1, int x2,
				 int y2, int w, int h)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
    DEBUG(" Scrn2Scrn: x1=%d y1=%d x2=%d y2=%d w=%d h=%d\n", x1, y1, x2, y2, w, h);

    if (pSmi->AccelCmd & FIELD_SET(0, DE_CONTROL, DIRECTION, RIGHT_TO_LEFT)) {
	x1 += w - 1;
	y1 += h - 1;
	x2 += w - 1;
	y2 += h - 1;
    }

    /*if (pScrn->bitsPerPixel == 24) {
	x1 *= 3;
	x2 *= 3;
	w  *= 3;

	if (pSmi->Chipset == SMI_LYNX) {
	    y1 *= 3;
	    y2 *= 3;
	}

	if (pSmi->AccelCmd & SMI_RIGHT_TO_LEFT) {
	    x1 += 2;
	    x2 += 2;
	}
    }*/

    //WaitIdle();
    deWaitForNotBusy();
    /*while(deWaitForNotBusy()!=0);*/
    /*WRITE_DPR(pSmi, 0x00, (x1 << 16) + (y1 & 0xFFFF));
    WRITE_DPR(pSmi, 0x04, (x2 << 16) + (y2 & 0xFFFF));
    WRITE_DPR(pSmi, 0x08, (w  << 16) + (h  & 0xFFFF));*/
    /*WRITE_DPR(pSmi, 0x0C, pSmi->AccelCmd);*/
    WRITE_DPR(pSmi, DE_SOURCE&0xff,
            FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE)|
            FIELD_VALUE(0, DE_SOURCE, X_K1, x1&0xFFFF)|
            FIELD_VALUE(0, DE_SOURCE, Y_K2, y1&0xFFFF));
    WRITE_DPR(pSmi, DE_DESTINATION&0xff,
            FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
            FIELD_VALUE(0, DE_DESTINATION, X, x2&0xFFFF)  |
            FIELD_VALUE(0, DE_DESTINATION, Y, y2&0xFFFF));
    WRITE_DPR(pSmi, DE_DIMENSION&0xff,
            FIELD_VALUE(0, DE_DIMENSION, X, w&0xFFFF) |
            FIELD_VALUE(0, DE_DIMENSION, Y_ET, h&0xFFFF));

    WRITE_DPR(pSmi, DE_CONTROL&0xff, pSmi->AccelCmd);
    LEAVE();
}

/******************************************************************************/
/*   Solid Fills							      */
/******************************************************************************/
static void
SMI712_SetupForSolidFill(ScrnInfoPtr pScrn, int color, int rop,
		      unsigned int planemask)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    CARD32 stride = pScrn->displayWidth | (pScrn->displayWidth<<16);

    ENTER();
    DEBUG("color=%08X rop=%02X\n", color, rop);

    pSmi->AccelCmd = XAAGetPatternROP(rop)
		   | SMI_BITBLT
		   | SMI_START_ENGINE;

#if __BYTE_ORDER == __BIG_ENDIAN
    if (pScrn->depth >= 24) {
	/* because of the BGR values are in the MSB bytes,
	 * 'white' is not possible and -1 has a different meaning.
	 * As a work around (assuming white is more used as
	 * light yellow (#FFFF7F), we handle this as beining white.
	 * Thanks to the SM501 not being able to work in MSB on PCI
	 * on the PowerPC */
	if (color == 0x7FFFFFFF)
	    color = -1;
	color = lswapl(color);
    }
#endif
    SMI712_AccelSync(pScrn);
    if (pSmi->ClipTurnedOn)
    {
    	//WaitQueue();
    	WRITE_DPR(pSmi, 0x2C, pSmi->ScissorsLeft);
    	WRITE_DPR(pSmi, 0x2e,pSmi->ScissorsRight); //monk add 
    	pSmi->ClipTurnedOn = FALSE;
    }
    else 
    {
    	//WaitQueue();
    }
    WRITE_DPR(pSmi, 0x14, color);
    WRITE_DPR(pSmi, 0x34, 0xFFFFFFFF);
    WRITE_DPR(pSmi, 0x38, 0xFFFFFFFF);
    /* monk add*/
    WRITE_DPR(pSmi,0x10,stride);
    WRITE_DPR(pSmi,0x3c,stride);

    LEAVE();
}




static void
SMI750_SetupForSolidFill(ScrnInfoPtr pScrn, int color, int rop,
		      unsigned int planemask)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
    DEBUG("color=%08X rop=%02X\n", color, rop);

    /*pSmi->AccelCmd = XAAGetPatternROP(rop)
		   | SMI_BITBLT
		   | SMI_START_ENGINE;*/
    pSmi->AccelCmd = FIELD_VALUE(0, DE_CONTROL, ROP, XAAGetPatternROP(rop)) |
                        FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP3) |
                        FIELD_SET(0, DE_CONTROL, COMMAND, BITBLT) |
                        FIELD_SET(0, DE_CONTROL, STATUS, START) |
                        FIELD_SET(0, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT);


#if __BYTE_ORDER == __BIG_ENDIAN
    if (pScrn->depth >= 24) {
	/* because of the BGR values are in the MSB bytes,
	 * 'white' is not possible and -1 has a different meaning.
	 * As a work around (assuming white is more used as
	 * light yellow (#FFFF7F), we handle this as beining white.
	 * Thanks to the SM501 not being able to work in MSB on PCI
	 * on the PowerPC */
	if (color == 0x7FFFFFFF)
	    color = -1;
	color = lswapl(color);
    }
#endif
    if (pSmi->ClipTurnedOn) {
	//WaitQueue();
        deWaitForNotBusy();
	/*WRITE_DPR(pSmi, 0x2C, pSmi->ScissorsLeft);*/
        deSetClipping(1, pSmi->ScissorsLeft&0xFFFF, pSmi->ScissorsLeft>>16, 
                        pSmi->ScissorsRight&0xFFFF, pSmi->ScissorsRight>>16);
	pSmi->ClipTurnedOn = FALSE;
    } else {
	//WaitQueue();
        deWaitForNotBusy();
    }
    /*WRITE_DPR(pSmi, 0x14, color);
    WRITE_DPR(pSmi, 0x34, 0xFFFFFFFF);
    WRITE_DPR(pSmi, 0x38, 0xFFFFFFFF);*/
    deWaitForNotBusy();
    deSetTransparency(0, 0, 0, 0);
    pSmi->AccelCmd |= deGetTransparency();
    WRITE_DPR(pSmi, DE_PITCH&0xff, 
        (FIELD_VALUE(0, DE_PITCH, DESTINATION, (pScrn->displayWidth)) |
        (FIELD_VALUE(0, DE_PITCH, SOURCE, (pScrn->displayWidth)))));
    WRITE_DPR(pSmi, DE_WINDOW_WIDTH&0xff, 
        (FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (pScrn->displayWidth)) |
        (FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE, (pScrn->displayWidth)))));
    WRITE_DPR(pSmi, DE_FOREGROUND&0xff, color);
    WRITE_DPR(pSmi, DE_MONO_PATTERN_LOW&0xff, 0xFFFFFFFF);
    WRITE_DPR(pSmi, DE_MONO_PATTERN_HIGH&0xff, 0xFFFFFFFF);

    LEAVE();
}

void
SMI712_SubsequentSolidFillRect(ScrnInfoPtr pScrn, int x, int y, int w, int h)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
    DEBUG("x=%d y=%d w=%d h=%d\n", x, y, w, h);

    if (pScrn->bitsPerPixel == 24) {
	x *= 3;
	w *= 3;

	if (pSmi->Chipset == SMI_LYNX) {
	    y *= 3;
	}
    }

#if 0    
    if (IS_MSOC(pSmi)) {
	/* Clip to prevent negative screen coordinates */
	if (x < 0)
	    x = 0;
	if (y < 0)
	    y = 0;
    }
#endif   

    //WaitQueue();
    SMI712_AccelSync(pScrn);
    WRITE_DPR(pSmi, 0x04, (x << 16) | (y & 0xFFFF));
    WRITE_DPR(pSmi, 0x08, (w << 16) | (h & 0xFFFF));
    WRITE_DPR(pSmi, 0x0C, pSmi->AccelCmd);

    LEAVE();
}



void
SMI750_SubsequentSolidFillRect(ScrnInfoPtr pScrn, int x, int y, int w, int h)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
    DEBUG("x=%d y=%d w=%d h=%d\n", x, y, w, h);

    /*if (pScrn->bitsPerPixel == 24) {
	x *= 3;
	w *= 3;

	if (pSmi->Chipset == SMI_LYNX) {
	    y *= 3;
	}
    }

    if (IS_MSOC(pSmi)) {*/
	/* Clip to prevent negative screen coordinates */
	/*if (x < 0)
	    x = 0;
	if (y < 0)
	    y = 0;
    }*/

    //WaitQueue();
    deWaitForNotBusy();
    /*WRITE_DPR(pSmi, 0x04, (x << 16) | (y & 0xFFFF));
    WRITE_DPR(pSmi, 0x08, (w << 16) | (h & 0xFFFF));
    WRITE_DPR(pSmi, 0x0C, pSmi->AccelCmd);*/
    /*deRectFill(pSmi->FBOffset>>3, pSmi->stride, pScrn->bitsPerPixel,
                x, y, w, h, pSmi->fillColor, pSmi->AccelCmd&0xFF);*/
    WRITE_DPR(pSmi, DE_DESTINATION&0xFF, ( 
                FIELD_SET(0, DE_DESTINATION, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_DESTINATION, X, x&0xFFFF) |
                FIELD_VALUE(0, DE_DESTINATION, Y, y&0xFFFF)));
    WRITE_DPR(pSmi, DE_DIMENSION&0xFF, ( 
                FIELD_VALUE(0, DE_DIMENSION, X, w&0xFFFF) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, h&0xFFFF)));
    WRITE_DPR(pSmi, DE_CONTROL&0xff, pSmi->AccelCmd);

    LEAVE();
}

/******************************************************************************/
/*   Solid Lines							      */
/******************************************************************************/


static void
SMI750_SetupForSolidLine(ScrnInfoPtr pScrn, int color, int rop,
                        unsigned int planemask)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    ENTER();
    DEBUG("color=%08X rop=%02X\n", color, rop);
    pSmi->AccelCmd = FIELD_VALUE(0, DE_CONTROL, ROP, XAAGetCopyROP(rop)) |
                        FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
                        FIELD_SET(0, DE_CONTROL, COMMAND, SHORT_STROKE) |
                        FIELD_SET(0, DE_CONTROL, STATUS, START) |
                        FIELD_SET(0, DE_CONTROL, STEP_X, NEGATIVE) |
                        FIELD_SET(0, DE_CONTROL, STEP_Y, NEGATIVE) |
                        FIELD_SET(0, DE_CONTROL, LAST_PIXEL, OFF);

#if __BYTE_ORDER == __BIG_ENDIAN
    if (pScrn->depth >= 24) {
	/* because of the BGR values are in the MSB bytes,
	 * 'white' is not possible and -1 has a different meaning.
	 * As a work around (assuming white is more used as
	 * light yellow (#FFFF7F), we handle this as beining white.
	 * Thanks to the SM501 not being able to work in MSB on PCI
	 * on the PowerPC */
	if (color == 0x7FFFFFFF)
	    color = -1;
	color = lswapl(color);
    }
#endif
    if (pSmi->ClipTurnedOn) {
	//WaitQueue();
        deWaitForNotBusy();
	/*WRITE_DPR(pSmi, 0x2C, pSmi->ScissorsLeft);*/
        deSetClipping(1, pSmi->ScissorsLeft&0xFFFF, pSmi->ScissorsLeft>>16, 
                        pSmi->ScissorsRight&0xFFFF, pSmi->ScissorsRight>>16);
	pSmi->ClipTurnedOn = FALSE;
    } else {
	//WaitQueue();
        deWaitForNotBusy();
    }
    /*WRITE_DPR(pSmi, 0x14, color);
    WRITE_DPR(pSmi, 0x34, 0xFFFFFFFF);
    WRITE_DPR(pSmi, 0x38, 0xFFFFFFFF);*/
    deWaitForNotBusy();
    deSetTransparency(0, 0, 0, 0);
    pSmi->AccelCmd |= deGetTransparency();
    WRITE_DPR(pSmi, DE_PITCH&0xff, 
        (FIELD_VALUE(0, DE_PITCH, DESTINATION, (pScrn->displayWidth)) |
        (FIELD_VALUE(0, DE_PITCH, SOURCE, (pScrn->displayWidth)))));
    WRITE_DPR(pSmi, DE_WINDOW_WIDTH&0xff, 
        (FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (pScrn->displayWidth)) |
        (FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE, (pScrn->displayWidth)))));
    WRITE_DPR(pSmi, DE_FOREGROUND&0xff, color);
    WRITE_DPR(pSmi, DE_MONO_PATTERN_LOW&0xff, 0xFFFFFFFF);
    WRITE_DPR(pSmi, DE_MONO_PATTERN_HIGH&0xff, 0xFFFFFFFF);

    LEAVE();
}

static void
SMI712_SubsequentSolidHorVertLine(ScrnInfoPtr pScrn, int x, int y, int len,
							   int dir)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    int w, h;

    ENTER();
    DEBUG("x=%d y=%d len=%d dir=%d\n", x, y, len, dir);

    if (dir == DEGREES_0) {
	w = len;
	h = 1;
    } else {
	w = 1;
	h = len;
    }

    if (pScrn->bitsPerPixel == 24) {
	x *= 3;
	w *= 3;

	if (pSmi->Chipset == SMI_LYNX) {
	    y *= 3;
	}
    }

    WaitQueue();
    WRITE_DPR(pSmi, 0x04, (x << 16) | (y & 0xFFFF));
    WRITE_DPR(pSmi, 0x08, (w << 16) | (h & 0xFFFF));
    WRITE_DPR(pSmi, 0x0C, pSmi->AccelCmd);

    LEAVE();
}


static void
SMI750_SubsequentSolidHorVertLine(ScrnInfoPtr pScrn, int x, int y, int len,
							   int dir)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    int w, h;

    ENTER();
    DEBUG("x=%d y=%d len=%d dir=%d\n", x, y, len, dir);

    if (dir == DEGREES_0) {
	w = len-1;
	h = 1;
        pSmi->AccelCmd |= FIELD_SET(0, DE_CONTROL, DIRECTION, RIGHT_TO_LEFT) |
                            FIELD_SET(0, DE_CONTROL, MAJOR, X);
    } else {
	w = 1;
	h = len-1;
        pSmi->AccelCmd |= FIELD_SET(0, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT) |
                            FIELD_SET(0, DE_CONTROL, MAJOR, Y);
    }
    /*if (pScrn->bitsPerPixel == 24) {
	x *= 3;
	w *= 3;

	if (pSmi->Chipset == SMI_LYNX) {
	    y *= 3;
	}
    }*/

    //WaitQueue();
    deWaitForNotBusy();
    /*WRITE_DPR(pSmi, 0x04, (x << 16) | (y & 0xFFFF));
    WRITE_DPR(pSmi, 0x08, (w << 16) | (h & 0xFFFF));
    WRITE_DPR(pSmi, 0x0C, pSmi->AccelCmd);*/
    /*deLine(pSmi->FBOffset>>3, pSmi->stride, pScrn->bitsPerPixel,
            x, y, x+w-1, y+h-1, pSmi->fillColor, pSmi->AccelCmd&0xFF);*/
    WRITE_DPR(pSmi, DE_DESTINATION&0xFF, (
                FIELD_SET(0, DE_DESTINATION, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_DESTINATION, X, x&0xffff) |
                FIELD_VALUE(0, DE_DESTINATION, Y, y&0xffff)));
    WRITE_DPR(pSmi, DE_DIMENSION&0xFF, (
                FIELD_VALUE(0, DE_DIMENSION, X, w&0xffff) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, h&0xffff)));
    WRITE_DPR(pSmi, DE_CONTROL&0xFF, pSmi->AccelCmd);

    LEAVE();
}

/******************************************************************************/
/*  Color Expansion Fills						      */
/******************************************************************************/
static void
SMI712_SetupForCPUToScreenColorExpandFill(ScrnInfoPtr pScrn, int fg, int bg,
				       int rop, unsigned int planemask)
{
    SMIPtr pSmi = SMIPTR(pScrn);    
    CARD32 stride = pScrn->displayWidth | (pScrn->displayWidth<<16);

    ENTER();
    DEBUG("fg=%08X bg=%08X rop=%02X\n", fg, bg, rop);

#if __BYTE_ORDER == __BIG_ENDIAN
    if (pScrn->depth >= 24) 
    {
    	/* see remark elswere */
    	if (fg == 0x7FFFFFFF)
    	    fg = -1;
    	fg = lswapl(fg);
    	bg = lswapl(bg);
    }
#endif

    pSmi->AccelCmd = XAAGetCopyROP(rop)
		   | SMI_HOSTBLT_WRITE
		   | SMI_SRC_MONOCHROME
		   | SMI_START_ENGINE;
		   
    SMI712_AccelSync(pScrn);
    if (bg == -1) 
    {
    	pSmi->AccelCmd |= SMI_TRANSPARENT_SRC;
    	//WaitQueue();	
    	WRITE_DPR(pSmi, 0x14, fg);
    	WRITE_DPR(pSmi, 0x18, ~fg);
    	WRITE_DPR(pSmi, 0x20, fg);
    } 
    else
    {
#if __BYTE_ORDER == __BIG_ENDIAN
    	if (bg == 0xFFFFFF7F)
    	    bg = -1;
#endif
    	WaitQueue();
    	WRITE_DPR(pSmi, 0x14, fg);
    	WRITE_DPR(pSmi, 0x18, bg);
    }
    WRITE_DPR(pSmi,0x10,stride);
    WRITE_DPR(pSmi,0x3c,stride);
    LEAVE();
}



static void
SMI750_SetupForCPUToScreenColorExpandFill(ScrnInfoPtr pScrn, int fg, int bg,
				       int rop, unsigned int planemask)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
    DEBUG("fg=%08X bg=%08X rop=%02X\n", fg, bg, rop);

#if __BYTE_ORDER == __BIG_ENDIAN
    if (pScrn->depth >= 24) {
	/* see remark elswere */
	if (fg == 0x7FFFFFFF)
	    fg = -1;
	fg = lswapl(fg);
	bg = lswapl(bg);
    }
#endif

    /*pSmi->AccelCmd = XAAGetCopyROP(rop)
		   | SMI_HOSTBLT_WRITE
		   | SMI_SRC_MONOCHROME
		   | SMI_START_ENGINE;*/

    pSmi->AccelCmd = FIELD_VALUE(0, DE_CONTROL, ROP, XAAGetCopyROP(rop)) |
                        FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP3) |
                        FIELD_SET(0, DE_CONTROL, COMMAND, HOST_WRITE) |
                        FIELD_SET(0, DE_CONTROL, HOST, MONO) |
                        FIELD_SET(0, DE_CONTROL, STATUS, START);

    if (bg == -1) {
	//pSmi->AccelCmd |= SMI_TRANSPARENT_SRC;

	//WaitQueue();
        //deWaitForNotBusy();
	/*WRITE_DPR(pSmi, 0x14, fg);
	WRITE_DPR(pSmi, 0x18, ~fg);
	WRITE_DPR(pSmi, 0x20, fg);*/
        //bg = ~fg;
        deSetTransparency(1, 0, 1, bg);
        //deSetTransparency(1, 0, 0, fg);
    } else {
#if __BYTE_ORDER == __BIG_ENDIAN
	if (bg == 0xFFFFFF7F)
	    bg = -1;
#endif
	//WaitQueue();
        //deWaitForNotBusy();
        deSetTransparency(0, 0, 0, 0);
	/*WRITE_DPR(pSmi, 0x14, fg);
	WRITE_DPR(pSmi, 0x18, bg);*/
    }
    pSmi->AccelCmd |= deGetTransparency();
    WRITE_DPR(pSmi, DE_PITCH&0xff, 
        (FIELD_VALUE(0, DE_PITCH, DESTINATION, (pScrn->displayWidth)) |
        (FIELD_VALUE(0, DE_PITCH, SOURCE, (pScrn->displayWidth)))));
    WRITE_DPR(pSmi, DE_WINDOW_WIDTH&0xff, 
        (FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (pScrn->displayWidth)) |
        (FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE, (pScrn->displayWidth)))));
    WRITE_DPR(pSmi, DE_SOURCE&0xff,
        FIELD_SET(0, DE_SOURCE, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_SOURCE, X_K1_MONO, 0));
    WRITE_DPR(pSmi, DE_FOREGROUND&0xff, fg);
    WRITE_DPR(pSmi, DE_BACKGROUND&0xff, bg);
    WRITE_DPR(pSmi, DE_MONO_PATTERN_LOW&0xff, 0xFFFFFFFF);
    WRITE_DPR(pSmi, DE_MONO_PATTERN_HIGH&0xff, 0xFFFFFFFF);

    LEAVE();
}

void
SMI712_SubsequentCPUToScreenColorExpandFill(ScrnInfoPtr pScrn, int x, int y, int w,
					 int h, int skipleft)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
    DEBUG("x=%d y=%d w=%d h=%d skipleft=%d\n", x, y, w, h, skipleft);

    if (pScrn->bitsPerPixel == 24)
    {
    	x        *= 3;
    	w        *= 3;
    	skipleft *= 3;
#if 0
    	if (pSmi->Chipset == SMI_LYNX) {
    	    y *= 3;
    	}
#endif	
    }

    SMI712_AccelSync(pScrn);        
    if (skipleft) 
    {
    	//WaitQueue();
    	WRITE_DPR(pSmi, 0x2C, (pSmi->ScissorsLeft & 0xFFFF0000)
    			      | (x + skipleft) | 0x2000);
    	pSmi->ClipTurnedOn = TRUE;
    }
    else
    {
    	if (pSmi->ClipTurnedOn) 
    	{
    	    //WaitQueue();
    	    WRITE_DPR(pSmi, 0x2C, pSmi->ScissorsLeft);
    	    pSmi->ClipTurnedOn = FALSE;
    	} 
    	else 
    	{
    	    //WaitQueue();
    	}
    }
    WRITE_DPR(pSmi, 0x00, 0);
    WRITE_DPR(pSmi, 0x04, (x << 16) | (y & 0xFFFF));
    WRITE_DPR(pSmi, 0x08, (w << 16) | (h & 0xFFFF));
    WRITE_DPR(pSmi, 0x0C, pSmi->AccelCmd);
    LEAVE();
}



void
SMI750_SubsequentCPUToScreenColorExpandFill(ScrnInfoPtr pScrn, int x, int y, int w,
					 int h, int skipleft)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    CARD32 TL;

    ENTER();
	return;
    DEBUG("x=%d y=%d w=%d h=%d skipleft=%d\n", x, y, w, h, skipleft);

    /*if (pScrn->bitsPerPixel == 24) {
	x        *= 3;
	w        *= 3;
	skipleft *= 3;

	if (pSmi->Chipset == SMI_LYNX) {
	    y *= 3;
	}
    }*/

    if (skipleft) {
	//WaitQueue();
        TL = (pSmi->ScissorsLeft & 0xFFFF0000) | (x + skipleft) | 0x2000;
        deWaitForNotBusy();
        deSetClipping(1, TL&0xFFFF, TL>>16, 
                        pSmi->ScissorsRight&0xFFFF, pSmi->ScissorsRight>>16);
	/*WRITE_DPR(pSmi, 0x2C, (pSmi->ScissorsLeft & 0xFFFF0000)
			      | (x + skipleft) | 0x2000);*/
	pSmi->ClipTurnedOn = TRUE;
    } else {
	if (pSmi->ClipTurnedOn) {
	    //WaitQueue();
            deWaitForNotBusy();
	    /*WRITE_DPR(pSmi, 0x2C, pSmi->ScissorsLeft);*/
            deSetClipping(1, pSmi->ScissorsLeft&0xFFFF, 
                            pSmi->ScissorsLeft>>16, pSmi->ScissorsRight&0xFFFF, 
                            pSmi->ScissorsRight>>16);
	    pSmi->ClipTurnedOn = FALSE;
	} else {
	    //WaitQueue();
            deWaitForNotBusy();
	}
    }
    /*WRITE_DPR(pSmi, 0x00, 0);
    WRITE_DPR(pSmi, 0x04, (x << 16) | (y & 0xFFFF));
    WRITE_DPR(pSmi, 0x08, (w << 16) | (h & 0xFFFF));
    WRITE_DPR(pSmi, 0x0C, pSmi->AccelCmd);*/
    /*deSystemMem2VideoMemMonoBlt(0, 0, 0,
                                pSmi->FBOffset>>3, pSmi->stride,
                                pScrn->bitsPerPixel, x, y, w, h, 
                                pSmi->monoFg, pSmi->monoBg, 
                                pSmi->AccelCmd&0xFF);*/
    deWaitForNotBusy();
    WRITE_DPR(pSmi, DE_DESTINATION&0xFF, (
                FIELD_SET(0, DE_DESTINATION, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_DESTINATION, X, x&0xffff) |
                FIELD_VALUE(0, DE_DESTINATION, Y, y&0xffff)));
    WRITE_DPR(pSmi, DE_DIMENSION&0xFF, (
                FIELD_VALUE(0, DE_DIMENSION, X, w&0xffff) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, h&0xffff)));
    WRITE_DPR(pSmi, DE_CONTROL&0xFF, pSmi->AccelCmd);

    LEAVE();
}

/******************************************************************************/
/* 8x8 Mono Pattern Fills						      */
/******************************************************************************/
static void
SMI712_SetupForMono8x8PatternFill(ScrnInfoPtr pScrn, int patx, int paty, int fg,
			       int bg, int rop, unsigned int planemask)
{
    SMIPtr pSmi = SMIPTR(pScrn);    
    CARD32 stride = pScrn->displayWidth | (pScrn->displayWidth<<16);

    ENTER();
    DEBUG("patx=%08X paty=%08X fg=%08X bg=%08X rop=%02X\n",
	  patx, paty, fg, bg, rop);

#if __BYTE_ORDER == __BIG_ENDIAN
    if (pScrn->depth >= 24)
    {
    	if (fg == 0x7FFFFFFF)
    	    fg = -1;
    	fg = lswapl(fg);
    	bg = lswapl(bg);
    }
#endif
    pSmi->AccelCmd = XAAGetPatternROP(rop)
		   | SMI_BITBLT
		   | SMI_START_ENGINE;
    SMI712_AccelSync(pScrn);    
    if (pSmi->ClipTurnedOn) 
    {
    	//WaitQueue();
    	WRITE_DPR(pSmi, 0x2C, pSmi->ScissorsLeft);
    	pSmi->ClipTurnedOn = FALSE;
    }

    if (bg == -1) 
    {
    	//WaitQueue();
    	WRITE_DPR(pSmi, 0x14, fg);
    	WRITE_DPR(pSmi, 0x18, ~fg);
    	WRITE_DPR(pSmi, 0x20, fg);
    	WRITE_DPR(pSmi, 0x34, patx);
    	WRITE_DPR(pSmi, 0x38, paty);
    } 
    else
    {
#if __BYTE_ORDER == __BIG_ENDIAN
    	if (bg == 0xFFFFFF7F)
    	    bg = -1;
#endif
    	//WaitQueue();
    	WRITE_DPR(pSmi, 0x14, fg);
    	WRITE_DPR(pSmi, 0x18, bg);
    	WRITE_DPR(pSmi, 0x34, patx);
    	WRITE_DPR(pSmi, 0x38, paty);
    }

    WRITE_DPR(pSmi,0x10,stride);
    WRITE_DPR(pSmi,0x3c,stride);

    LEAVE();
}

static void
SMI750_SetupForMono8x8PatternFill(ScrnInfoPtr pScrn, int patx, int paty, int fg,
			       int bg, int rop, unsigned int planemask)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
    /*DEBUG("patx=%08X paty=%08X fg=%08X bg=%08X rop=%02X\n",
	  patx, paty, fg, bg, rop);*/

#if __BYTE_ORDER == __BIG_ENDIAN
    if (pScrn->depth >= 24) {
	if (fg == 0x7FFFFFFF)
	    fg = -1;
	fg = lswapl(fg);
	bg = lswapl(bg);
    }
#endif
    /*pSmi->AccelCmd = XAAGetPatternROP(rop)*/
    /*pSmi->AccelCmd = XAAGetCopyROP(rop)
		   | SMI_BITBLT
		   | SMI_START_ENGINE;*/
    pSmi->AccelCmd = FIELD_VALUE(0, DE_CONTROL, ROP, XAAGetPatternROP(rop)) |
                        FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP3) |
                        FIELD_SET(0, DE_CONTROL, COMMAND, BITBLT) |
                        FIELD_SET(0, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT) |
                        FIELD_SET(0, DE_CONTROL, PATTERN, MONO) |
                        FIELD_SET(0, DE_CONTROL, STATUS, START);

    if (pSmi->ClipTurnedOn) {
	//WaitQueue();
        deWaitForNotBusy();
	//WRITE_DPR(pSmi, 0x2C, pSmi->ScissorsLeft);
        deSetClipping(1, pSmi->ScissorsLeft&0xFFFF, pSmi->ScissorsLeft >> 16,
                        pSmi->ScissorsRight&0xFFFF, pSmi->ScissorsRight >> 16);
	pSmi->ClipTurnedOn = FALSE;
    }

    if (bg == -1) {
	//WaitQueue();
        bg = ~fg;
        deSetTransparency(1, 0, 0, fg);
	/*WRITE_DPR(pSmi, 0x14, fg);
	WRITE_DPR(pSmi, 0x18, ~fg);
	WRITE_DPR(pSmi, 0x20, fg);
	WRITE_DPR(pSmi, 0x34, patx);
	WRITE_DPR(pSmi, 0x38, paty);*/
    } else {
#if __BYTE_ORDER == __BIG_ENDIAN
	if (bg == 0xFFFFFF7F)
	    bg = -1;
#endif
	//WaitQueue();
        /*deWaitForNotBusy();
	WRITE_DPR(pSmi, 0x14, fg);
	WRITE_DPR(pSmi, 0x18, bg);
	WRITE_DPR(pSmi, 0x34, patx);
	WRITE_DPR(pSmi, 0x38, paty);*/
        deSetTransparency(0, 0, 0, 0);
    }
    pSmi->AccelCmd |= deGetTransparency();
    deWaitForNotBusy();
    WRITE_DPR(pSmi, DE_FOREGROUND&0xFF, fg);
    WRITE_DPR(pSmi, DE_BACKGROUND&0xFF, bg);
    WRITE_DPR(pSmi, DE_MONO_PATTERN_LOW&0xFF, patx);
    WRITE_DPR(pSmi, DE_MONO_PATTERN_HIGH&0xFF, paty);
    WRITE_DPR(pSmi, DE_PITCH&0xff, 
        (FIELD_VALUE(0, DE_PITCH, DESTINATION, (pScrn->displayWidth)) |
        (FIELD_VALUE(0, DE_PITCH, SOURCE, (pScrn->displayWidth)))));
    WRITE_DPR(pSmi, DE_WINDOW_WIDTH&0xff, 
        (FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (pScrn->displayWidth)) |
        (FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE, (pScrn->displayWidth)))));

    LEAVE();
}

static void
SMI712_SubsequentMono8x8PatternFillRect(ScrnInfoPtr pScrn, int patx, int paty,
				     int x, int y, int w, int h)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    ENTER();
    DEBUG("x=%d y=%d w=%d h=%d\n", x, y, w, h);

    if (pScrn->bitsPerPixel == 24) 
    {
    	x *= 3;
    	w *= 3;
#if 0    	
    	if (pSmi->Chipset == SMI_LYNX) {
    	    y *= 3;    	    
    	}
#endif    	
    }
    SMI712_AccelSync(pScrn);    
    //WaitQueue();
    WRITE_DPR(pSmi, 0x04, (x << 16) | (y & 0xFFFF));
    WRITE_DPR(pSmi, 0x08, (w << 16) | (h & 0xFFFF));
    WRITE_DPR(pSmi, 0x0C, pSmi->AccelCmd);

    LEAVE();
}



static void
SMI750_SubsequentMono8x8PatternFillRect(ScrnInfoPtr pScrn, int patx, int paty,
				     int x, int y, int w, int h)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    int i;

    ENTER();
	return ;
    DEBUG("x=%d y=%d w=%d h=%d\n", x, y, w, h);

    /*if (pScrn->bitsPerPixel == 24) {
	x *= 3;
	w *= 3;
	if (pSmi->Chipset == SMI_LYNX) {
	    y *= 3;
	}
    }*/

    //WaitQueue();
    deWaitForNotBusy();
    /*WRITE_DPR(pSmi, 0x04, (x << 16) | (y & 0xFFFF));
    WRITE_DPR(pSmi, 0x08, (w << 16) | (h & 0xFFFF));
    WRITE_DPR(pSmi, 0x0C, pSmi->AccelCmd);*/
    WRITE_DPR(pSmi, DE_DESTINATION&0xFF, (
                FIELD_SET(0, DE_DESTINATION, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_DESTINATION, X, x&0xffff) |
                FIELD_VALUE(0, DE_DESTINATION, Y, y&0xffff)));
    WRITE_DPR(pSmi, DE_DIMENSION&0xFF, (
                FIELD_VALUE(0, DE_DIMENSION, X, w&0xffff) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, h&0xffff)));
    WRITE_DPR(pSmi, DE_CONTROL&0xFF, pSmi->AccelCmd);

    LEAVE();
}

/******************************************************************************/
/* 8x8 Color Pattern Fills						      */
/******************************************************************************/

static void
SMI712_SetupForColor8x8PatternFill(ScrnInfoPtr pScrn, int patx, int paty, int rop,
				unsigned int planemask, int trans_color)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    CARD32 stride = pScrn->displayWidth | (pScrn->displayWidth<<16);

    ENTER();
    DEBUG("patx=%d paty=%d rop=%02X trans_color=%08X\n",
	  patx, paty, rop, trans_color);

    pSmi->AccelCmd = XAAGetPatternROP(rop)
		   | SMI_BITBLT
		   | SMI_COLOR_PATTERN
		   | SMI_START_ENGINE;

#if __BYTE_ORDER == __BIG_ENDIAN
    if (pScrn->depth >= 24)
    	trans_color = lswapl(trans_color);
#endif

    SMI712_AccelSync(pScrn);    
    if (pScrn->bitsPerPixel <= 16) 
    {
    	/* PDR#950 */
    	CARD8* pattern = pSmi->FBBase +
	    (patx + paty * pScrn->displayWidth) * pSmi->Bpp;

    	//WaitIdle();
    	WRITE_DPR(pSmi, 0x0C, SMI_BITBLT | SMI_COLOR_PATTERN);
    	memcpy(pSmi->DataPortBase, pattern, 8 * pSmi->Bpp * 8);
    } 
    else 
    {
    	if (pScrn->bitsPerPixel == 24) 
    	{
    	    patx *= 3;
#if 0
    	    if (pSmi->Chipset == SMI_LYNX) 
    	    {
        		paty *= 3;
    	    }
#endif    	    
    	}

    	//WaitQueue();
    	WRITE_DPR(pSmi, 0x00, (patx << 16) | (paty & 0xFFFF));
    }

    //WaitQueue();

    if (trans_color == -1) 
    {
    	pSmi->AccelCmd |= SMI_TRANSPARENT_SRC | SMI_TRANSPARENT_PXL;
    	//WaitQueue();
    	WRITE_DPR(pSmi, 0x20, trans_color);
    }

    if (pSmi->ClipTurnedOn) 
    {
    	//WaitQueue();
    	WRITE_DPR(pSmi, 0x2C, pSmi->ScissorsLeft);
    	pSmi->ClipTurnedOn = FALSE;
    }
    
    WRITE_DPR(pSmi,0x10,stride);
    WRITE_DPR(pSmi,0x3c,stride);

    LEAVE();
}



static void
SMI750_SetupForColor8x8PatternFill(ScrnInfoPtr pScrn, int patx, int paty, int rop,
				unsigned int planemask, int trans_color)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
    DEBUG("patx=%d paty=%d rop=%02X trans_color=%08X\n",
	  patx, paty, rop, trans_color);

    /*pSmi->AccelCmd = XAAGetPatternROP(rop)
		   | SMI_BITBLT
		   | SMI_COLOR_PATTERN
		   | SMI_START_ENGINE;*/
    pSmi->AccelCmd = FIELD_VALUE(0, DE_CONTROL, ROP, XAAGetPatternROP(rop)) |
                        FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP3) |
                        FIELD_SET(0, DE_CONTROL, COMMAND, BITBLT) |
                        FIELD_SET(0, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT) |
                        FIELD_SET(0, DE_CONTROL, PATTERN, COLOR) |
                        FIELD_SET(0, DE_CONTROL, STATUS, START);

#if 0
#if __BYTE_ORDER == __BIG_ENDIAN
    if (pScrn->depth >= 24)
	trans_color = lswapl(trans_color);
#endif
    if (pScrn->bitsPerPixel <= 16) {
	/* PDR#950 */
	CARD8* pattern = pSmi->FBBase +
	    (patx + paty * pScrn->displayWidth) * pSmi->Bpp;

	WaitIdle();
	WRITE_DPR(pSmi, 0x0C, SMI_BITBLT | SMI_COLOR_PATTERN);
	memcpy(pSmi->DataPortBase, pattern, 8 * pSmi->Bpp * 8);
    } else {
	if (pScrn->bitsPerPixel == 24) {
	    patx *= 3;

	    if (pSmi->Chipset == SMI_LYNX) {
		paty *= 3;
	    }
	}

	//WaitQueue();
        deWaitForNotBusy();
	WRITE_DPR(pSmi, 0x00, (patx << 16) | (paty & 0xFFFF));
    }

    //WaitQueue();
    deWaitForNotBusy();
#endif

    if (trans_color != -1) {
        deSetTransparency(1, 0, 1, trans_color);
    }
    else {
        deSetTransparency(0, 0, 0, 0);
    }

    if (pSmi->ClipTurnedOn) {
        deWaitForNotBusy();
        deSetClipping(1, pSmi->ScissorsLeft&0xFFFF, pSmi->ScissorsLeft >> 16,
                        pSmi->ScissorsRight&0xFFFF, pSmi->ScissorsRight >> 16);
	pSmi->ClipTurnedOn = FALSE;
    }
    pSmi->AccelCmd |= deGetTransparency();
    deWaitForNotBusy();
    WRITE_DPR(pSmi, DE_SOURCE&0xff,
                FIELD_SET(0, DE_SOURCE, WRAP, DISABLE)|
                FIELD_VALUE(0, DE_SOURCE, X_K1, patx&0xffff) |
                FIELD_VALUE(0, DE_SOURCE, Y_K2, paty&0xffff));
    WRITE_DPR(pSmi, DE_PITCH&0xff, 
        (FIELD_VALUE(0, DE_PITCH, DESTINATION, (pScrn->displayWidth)) |
        (FIELD_VALUE(0, DE_PITCH, SOURCE, (pScrn->displayWidth)))));
    WRITE_DPR(pSmi, DE_WINDOW_WIDTH&0xff, 
        (FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (pScrn->displayWidth)) |
        (FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE, (pScrn->displayWidth)))));

    LEAVE();
}

static void
SMI712_SubsequentColor8x8PatternFillRect(ScrnInfoPtr pScrn, int patx, int paty,
				      int x, int y, int w, int h)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
    DEBUG("x=%d y=%d w=%d h=%d\n", x, y, w, h);

    if (pScrn->bitsPerPixel == 24) 
    {
    	x *= 3;
    	w *= 3;
#if 0
    	if (pSmi->Chipset == SMI_LYNX) {
    	    y *= 3;
    	}
#endif    	
    }
    
    SMI712_AccelSync(pScrn);

    //WaitQueue();
    WRITE_DPR(pSmi, 0x04, (x << 16) | (y & 0xFFFF));
    WRITE_DPR(pSmi, 0x08, (w << 16) | (h & 0xFFFF));	/* PDR#950 */
    WRITE_DPR(pSmi, 0x0C, pSmi->AccelCmd);

    LEAVE();
}


static void
SMI750_SubsequentColor8x8PatternFillRect(ScrnInfoPtr pScrn, int patx, int paty,
				      int x, int y, int w, int h)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
    DEBUG("x=%d y=%d w=%d h=%d\n", x, y, w, h);
#if 0
    if (pScrn->bitsPerPixel == 24) {
	x *= 3;
	w *= 3;

	if (pSmi->Chipset == SMI_LYNX) {
	    y *= 3;
	}
    }
#endif

    //WaitQueue();
    /*deWaitForNotBusy();
    WRITE_DPR(pSmi, 0x04, (x << 16) | (y & 0xFFFF));
    WRITE_DPR(pSmi, 0x08, (w << 16) | (h & 0xFFFF));	/* PDR#950 */
    //WRITE_DPR(pSmi, 0x0C, pSmi->AccelCmd);
    WRITE_DPR(pSmi, DE_DESTINATION&0xFF, (
                FIELD_SET(0, DE_DESTINATION, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_DESTINATION, X, x&0xffff) |
                FIELD_VALUE(0, DE_DESTINATION, Y, y&0xffff)));
    WRITE_DPR(pSmi, DE_DIMENSION&0xFF, (
                FIELD_VALUE(0, DE_DIMENSION, X, w&0xffff) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, h&0xffff)));
    WRITE_DPR(pSmi, DE_CONTROL&0xFF, pSmi->AccelCmd);

    LEAVE();
}

#if SMI_USE_IMAGE_WRITES
/******************************************************************************/
/*  Image Writes							      */
/******************************************************************************/
static void
SMI712_SetupForImageWrite(ScrnInfoPtr pScrn, int rop, unsigned int planemask,
		       int trans_color, int bpp, int depth)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
    DEBUG("rop=%02X trans_color=%08X bpp=%d depth=%d\n",
	  rop, trans_color, bpp, depth);

#if __BYTE_ORDER == __BIG_ENDIAN
    if (pScrn->depth >= 24)
	trans_color = lswapl(trans_color);
#endif
    pSmi->AccelCmd = XAAGetCopyROP(rop)
		   | SMI_HOSTBLT_WRITE
		   | SMI_START_ENGINE;

    if (trans_color != -1)
    {
#if __BYTE_ORDER == __BIG_ENDIAN
    	if (trans_color == 0xFFFFFF7F)
    	    trans_color = -1;
#endif
    	pSmi->AccelCmd |= SMI_TRANSPARENT_SRC | SMI_TRANSPARENT_PXL;

    	//WaitQueue();
        SMI712_AccelSync(pScrn);	
    	WRITE_DPR(pSmi, 0x20, trans_color);
    }

    LEAVE();
}



static void
SMI750_SetupForImageWrite(ScrnInfoPtr pScrn, int rop, unsigned int planemask,
		       int trans_color, int bpp, int depth)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
    DEBUG("rop=%02X trans_color=%08X bpp=%d depth=%d\n",
	  rop, trans_color, bpp, depth);

#if __BYTE_ORDER == __BIG_ENDIAN
    if (pScrn->depth >= 24) {
	/* see remark elswere */
	if (trans_color == 0x7FFFFFFF)
	    fg = -1;
	trans_color = lswapl(trans_color);
    }
#endif

    /*pSmi->AccelCmd = XAAGetCopyROP(rop)
		   | SMI_HOSTBLT_WRITE
		   | SMI_SRC_MONOCHROME
		   | SMI_START_ENGINE;*/

    //pSmi->AccelCmd = FIELD_VALUE(0, DE_CONTROL, ROP, XAAGetCopyROP(rop)) |
    pSmi->AccelCmd = FIELD_VALUE(0, DE_CONTROL, ROP, 0x0c) |
                        FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
                        FIELD_SET(0, DE_CONTROL, COMMAND, HOST_WRITE) |
                        FIELD_SET(0, DE_CONTROL, HOST, COLOR) |
                        FIELD_SET(0, DE_CONTROL, STATUS, START);

    if (trans_color != -1) {
        deSetTransparency(1, 0, 1, trans_color);
    } else {
        deSetTransparency(0, 0, 0, 0);
    }
    pSmi->AccelCmd |= deGetTransparency();
    WRITE_DPR(pSmi, DE_WINDOW_SOURCE_BASE&0xff, 0);
    WRITE_DPR(pSmi, DE_PITCH&0xff, 
        (FIELD_VALUE(0, DE_PITCH, DESTINATION, (pScrn->displayWidth)) |
        (FIELD_VALUE(0, DE_PITCH, SOURCE, (pScrn->displayWidth)))));
    WRITE_DPR(pSmi, DE_WINDOW_WIDTH&0xff, 
        (FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (pScrn->displayWidth)) |
        (FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE, (pScrn->displayWidth)))));
    WRITE_DPR(pSmi, DE_SOURCE&0xff,
        FIELD_SET(0, DE_SOURCE, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_SOURCE, X_K1, 0) |
        FIELD_VALUE(0, DE_SOURCE, Y_K2, 0));
    LEAVE();
}


static void
SMI712_SubsequentImageWriteRect(ScrnInfoPtr pScrn, int x, int y, int w, int h,
			     int skipleft)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
    DEBUG("x=%d y=%d w=%d h=%d skipleft=%d\n", x, y, w, h, skipleft);

    if (pScrn->bitsPerPixel == 24) 
    {
    	x        *= 3;
    	w        *= 3;
    	skipleft *= 3;

#if 0
    	if (pSmi->Chipset == SMI_LYNX) {
    	    y *= 3;
    	}
#endif	
    }
    
    SMI712_AccelSync(pScrn);
    if (skipleft)
    {
    	//WaitQueue();
    	WRITE_DPR(pSmi, 0x2C, (pSmi->ScissorsLeft & 0xFFFF0000) |
    			      (x + skipleft) | 0x2000);
    	pSmi->ClipTurnedOn = TRUE;
    }
    else
    {
    	if (pSmi->ClipTurnedOn) 
    	{
    	    //WaitQueue();
    	    WRITE_DPR(pSmi, 0x2C, pSmi->ScissorsLeft);
    	    pSmi->ClipTurnedOn = FALSE;
    	} 
    	else
    	{
    	    //WaitQueue();
    	}
    }
    WRITE_DPR(pSmi, 0x00, 0);
    WRITE_DPR(pSmi, 0x04, (x << 16) | (y * 0xFFFF));
    WRITE_DPR(pSmi, 0x08, (w << 16) | (h & 0xFFFF));
    WRITE_DPR(pSmi, 0x0C, pSmi->AccelCmd);

    LEAVE();
}


static void
SMI750_SubsequentImageWriteRect(ScrnInfoPtr pScrn, int x, int y, int w, int h,
			     int skipleft)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    CARD32 TL;

    ENTER();
    DEBUG("x=%d y=%d w=%d h=%d skipleft=%d\n", x, y, w, h, skipleft);

    /*if (pScrn->bitsPerPixel == 24) {
	x        *= 3;
	w        *= 3;
	skipleft *= 3;

	if (pSmi->Chipset == SMI_LYNX) {
	    y *= 3;
	}
    }*/

    if (skipleft) {
	//WaitQueue();
        TL = (pSmi->ScissorsLeft & 0xFFFF0000) | (x + skipleft) | 0x2000;
        deWaitForNotBusy();
        deSetClipping(1, TL&0xFFFF, TL>>16, 
                        pSmi->ScissorsRight&0xFFFF, pSmi->ScissorsRight>>16);
	/*WRITE_DPR(pSmi, 0x2C, (pSmi->ScissorsLeft & 0xFFFF0000)
			      | (x + skipleft) | 0x2000);*/
	pSmi->ClipTurnedOn = TRUE;
    } else {
	if (pSmi->ClipTurnedOn) {
	    //WaitQueue();
            deWaitForNotBusy();
	    /*WRITE_DPR(pSmi, 0x2C, pSmi->ScissorsLeft);*/
            deSetClipping(1, pSmi->ScissorsLeft&0xFFFF, 
                            pSmi->ScissorsLeft>>16, pSmi->ScissorsRight&0xFFFF, 
                            pSmi->ScissorsRight>>16);
	    pSmi->ClipTurnedOn = FALSE;
	} else {
	    //WaitQueue();
            deWaitForNotBusy();
	}
    }
    /*WRITE_DPR(pSmi, 0x00, 0);
    WRITE_DPR(pSmi, 0x04, (x << 16) | (y & 0xFFFF));
    WRITE_DPR(pSmi, 0x08, (w << 16) | (h & 0xFFFF));
    WRITE_DPR(pSmi, 0x0C, pSmi->AccelCmd);*/
    /*deSystemMem2VideoMemMonoBlt(0, 0, 0,
                                pSmi->FBOffset>>3, pSmi->stride,
                                pScrn->bitsPerPixel, x, y, w, h, 
                                pSmi->monoFg, pSmi->monoBg, 
                                pSmi->AccelCmd&0xFF);*/
    w = 5;
    h = 5;
    deWaitForNotBusy();
    WRITE_DPR(pSmi, DE_DESTINATION&0xFF, (
                FIELD_SET(0, DE_DESTINATION, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_DESTINATION, X, x&0xffff) |
                FIELD_VALUE(0, DE_DESTINATION, Y, y&0xffff)));
    WRITE_DPR(pSmi, DE_DIMENSION&0xFF, (
                FIELD_VALUE(0, DE_DIMENSION, X, w&0xffff) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, h&0xffff)));
    WRITE_DPR(pSmi, DE_CONTROL&0xFF, pSmi->AccelCmd);

    LEAVE();
}
#endif
