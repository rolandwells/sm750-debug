/* Header:   //Mercury/Projects/archives/XFree86/4.0/smi_accel.c-arc   1.16   03 Jan 2001 13:29:06   Frido  $ */

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
#include "smilynx.h"
#include "smi750.h"

void
SMI_GEReset(ScrnInfoPtr pScrn, int from_timeout, int line, char *file)
{
    SMIPtr	pSmi = SMIPTR(pScrn);
    int32_t	tmp;

    ENTER();

    if (from_timeout) 
    {
    	if (pSmi->GEResetCnt++ < 10 || xf86GetVerbosity() > 1)
    	    xf86DrvMsg(pScrn->scrnIndex,X_INFO,"\tSMI_GEReset called from %s line %d\n", file, line);
    }
    else
    {	
	    if(IS_SM750(pSmi) || IS_SM718(pSmi))
            deWaitForNotBusy();
        else
            WaitIdle();
    }

    if (IS_MSOC(pSmi)) {
	/*	12:13	Drawing Engine Abort
	 *		00:	Normal
	 *		11:	Abort 2D Engine
	 *	(0x3000 == bits 12 and 13 set)
	 */
	tmp = READ_SCR(pSmi, 0x0000) & ~0x00003000;
	WRITE_SCR(pSmi, 0x0000, tmp | 0x00003000);
	/* FIXME No need to wait here? */
	WRITE_SCR(pSmi, 0x0000, tmp);
    }
    else {
	tmp = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x15);
	VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x15, tmp | 0x30);
    }

    if (!IS_MSOC(pSmi))
	VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x15, tmp);
    SMI_EngineReset(pScrn);

    LEAVE();
}

/* The sync function for the GE */
void
SMI_AccelSync(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
	
    if(IS_SM750(pSmi)|| IS_SM718(pSmi))
    	deWaitForNotBusy();
    else if(IS_SM712(pSmi))
    	WaitIdle();

    LEAVE();
}

void SMI750_AccelSync(ScrnInfoPtr pScrn)
{
    deWaitForNotBusy();
}

void SMI712_AccelSync(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);

#if 1
    
    do
    {								
    	int	loop = MAXLOOP;									
    	mem_barrier();
    									
	    int	status;			
	    for (status = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX,VGA_SEQ_DATA, 0x16);
                loop && (status & 0x18) != 0x10;
                status = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX,VGA_SEQ_DATA, 0x16), loop--);
									
    	if (loop <= 0)							
    	    SMI_GEReset(pScrn, 1, __LINE__, __FILE__);
    }while (0); 
#else
    WaitIdle();
#endif
}



void SMI750_engineReset(ScrnInfoPtr pScrn)
{   
    SMIPtr pSmi = SMIPTR(pScrn);
    CARD32 DEDataFormat = 0;
    int i, stride;
    int xyAddress[] = { 320, 400, 512, 640, 800, 1024, 1280, 1600, 2048 };

    ENTER();
    //deInit();
    
    if (deWaitForNotBusy()!=0) {
        LEAVE(FALSE);
    }
    deReset();
    deSetPixelFormat(pScrn->bitsPerPixel);
    deSetTransparency(0, 0, 0, 0);
    WRITE_DPR(pSmi, 0x24, 0xFFFFFFFF);
    WRITE_DPR(pSmi, 0x28, 0xFFFFFFFF);
    WRITE_DPR(pSmi, 0x40, pScrn->fbOffset);
    WRITE_DPR(pSmi, 0x44, pScrn->fbOffset);
    SMI_DisableClipping(pScrn);   
    
    LEAVE();
}

void SMI712_engineReset(ScrnInfoPtr pScrn)
{
#if 0
    SMIPtr pSmi = SMIPTR(pScrn);
    CARD32 DEDataFormat = 0;
    int i, stride;
    int xyAddress[] = { 320, 400, 512, 640, 800, 1024, 1280, 1600, 2048 };
    
    ENTER();

    DEDataFormat = SMI_DEDataFormat(pScrn->bitsPerPixel);
    for (i = 0; i < sizeof(xyAddress) / sizeof(xyAddress[0]); i++) 
    {
    	if (xyAddress[i] == pScrn->virtualX) {
    	    DEDataFormat |= i << 16;
    	    break;
    	}
    }
    
    DEDataFormat |= 0x40000000; /* Force pattern origin coordinates to (0,0) */

    WaitIdle();//open source wait idle, need rewritten later...
    stride = pScrn->displayWidth;
    if (pSmi->Bpp == 3)
    	stride *= 3;
	
    /*
            holly crap! 712 and 502/750 nearly share with the same de engine register !
            weired ~  below code is valid both in 502/750 and 712...
        */
    WRITE_DPR(pSmi, 0x10, (stride << 16) | stride);
    WRITE_DPR(pSmi, 0x1C, DEDataFormat);
    WRITE_DPR(pSmi, 0x24, 0xFFFFFFFF);
    WRITE_DPR(pSmi, 0x28, 0xFFFFFFFF);
    WRITE_DPR(pSmi, 0x3C, (stride << 16) | stride);
    WRITE_DPR(pSmi, 0x40, pSmi->FBOffset >> 3);
    WRITE_DPR(pSmi, 0x44, pSmi->FBOffset >> 3);

    SMI_DisableClipping(pScrn);

    LEAVE();    
#else
SMIPtr pSmi = SMIPTR(pScrn);
CARD32 DEDataFormat = 0;
int i, stride;
static int xyAddress[] = { 320, 400, 512, 640, 800, 1024, 1280, 1600, 2048 };

ENTER();

DEDataFormat = SMI_DEDataFormat(pScrn->bitsPerPixel);
for (i = 0; i < sizeof(xyAddress) / sizeof(xyAddress[0]); i++) {
if (xyAddress[i] == pScrn->virtualX) {
    DEDataFormat |= i << 16;
    break;
}
}
DEDataFormat |= 0x40000000; /* Force pattern origin coordinates to (0,0) */

WaitIdle();
stride = pScrn->displayWidth;
if (pSmi->Bpp == 3)
stride *= 3;
WRITE_DPR(pSmi, 0x10, (stride << 16) | stride);
WRITE_DPR(pSmi, 0x1C, DEDataFormat);
WRITE_DPR(pSmi, 0x24, 0xFFFFFFFF);
WRITE_DPR(pSmi, 0x28, 0xFFFFFFFF);
WRITE_DPR(pSmi, 0x3C, (stride << 16) | stride);
WRITE_DPR(pSmi,0x40,pScrn->fbOffset);
WRITE_DPR(pSmi,0x44,pScrn->fbOffset);


SMI_DisableClipping(pScrn);

LEAVE();

#endif
}



#if 1
void
SMI_EngineReset(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();

    switch (pSmi->Chipset)
    {
    case SMI_750:
	case SMI_718:
        SMI750_engineReset(pScrn);
        break;
    case SMI_712:
        SMI712_engineReset(pScrn);
        break;
    default:
        DEBUG("not supported chip:%04x\n",pSmi->Chipset);
        break;
    }

    LEAVE();
    //LEAVE(TRUE);           
}
#else
void
SMI_EngineReset(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    CARD32 DEDataFormat = 0;
    int i, stride;
    int xyAddress[] = { 320, 400, 512, 640, 800, 1024, 1280, 1600, 2048 };

    ENTER();

    /*DEDataFormat = SMI_DEDataFormat(pScrn->bitsPerPixel);
    for (i = 0; i < sizeof(xyAddress) / sizeof(xyAddress[0]); i++) {
	if (xyAddress[i] == pScrn->virtualX) {
	    DEDataFormat |= i << 16;
	    break;
	}
    }
    DEDataFormat |= 0x40000000;*/ /* Force pattern origin coordinates to (0,0) */

    //WaitIdle();
    if (deWaitForNotBusy()!=0) {
        LEAVE(FALSE);
    }
    /*stride = pScrn->displayWidth;
    if (pSmi->Bpp == 3)
	stride *= 3;
    pSmi->stride = pScrn->displayWidth;
    /*WRITE_DPR(pSmi, 0x10, (stride << 16) | stride);
    WRITE_DPR(pSmi, 0x1C, DEDataFormat);
    WRITE_DPR(pSmi, 0x24, 0xFFFFFFFF);
    WRITE_DPR(pSmi, 0x28, 0xFFFFFFFF);
    WRITE_DPR(pSmi, 0x3C, (stride << 16) | stride);
    WRITE_DPR(pSmi, 0x40, pSmi->FBOffset >> 3);
    WRITE_DPR(pSmi, 0x44, pSmi->FBOffset >> 3);*/
    deReset();
    deSetPixelFormat(pScrn->bitsPerPixel);
    deSetTransparency(0, 0, 0, 0);
    WRITE_DPR(pSmi, 0x24, 0xFFFFFFFF);
    WRITE_DPR(pSmi, 0x28, 0xFFFFFFFF);
    /*WRITE_DPR(pSmi, 0x40, pSmi->FBOffset >> 3);
    WRITE_DPR(pSmi, 0x44, pSmi->FBOffset >> 3);*/
    WRITE_DPR(pSmi, 0x40, pScrn->fbOffset);
    WRITE_DPR(pSmi, 0x44, pScrn->fbOffset);

    SMI_DisableClipping(pScrn);

    LEAVE();
}
#endif

/******************************************************************************/
/*  Clipping								      */
/******************************************************************************/

void SMI750_SETCLIP(ScrnInfoPtr pScrn,int left,int top,int right,int bottom)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    
    ENTER();
    
    pSmi->ScissorsLeft = (top << 16) | (left & 0xFFFF) | 0x2000;
    pSmi->ScissorsRight = (bottom << 16) | (right & 0xFFFF);
    pSmi->ClipTurnedOn = FALSE;
    deSetClipping(1, pSmi->ScissorsLeft&0xffff, pSmi->ScissorsLeft >> 16,
                    pSmi->ScissorsRight&0xffff, pSmi->ScissorsRight >> 16);

    LEAVE();
}

void SMI712_SETCLIP(ScrnInfoPtr pScrn,int left,int top,int right,int bottom)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();    

    if (pScrn->bitsPerPixel == 24){
    	left  *= 3;
    	right *= 3;
#if 0     
        //do not handle 910 chips
    	if (pSmi->Chipset == SMI_LYNX){
    	    top *= 3;
    	    bottom *= 3;
    	}
#endif
    }
#if 0
    // do not handle 501 chips
    if (IS_MSOC(pSmi)){
    	++right;
    	++bottom;
    }
#endif
    pSmi->ScissorsLeft = (top << 16) | (left & 0xFFFF) | 0x2000;
    pSmi->ScissorsRight = (bottom << 16) | (right & 0xFFFF);
    pSmi->ClipTurnedOn = FALSE;
    WaitQueue();
    WRITE_DPR(pSmi, 0x2C, pSmi->ScissorsLeft);
    WRITE_DPR(pSmi, 0x30, pSmi->ScissorsRight);

    LEAVE();

}



void SMI_SetClippingRectangle(ScrnInfoPtr pScrn, int left, int top, int right,
			 int bottom)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    ENTER();
    DEBUG("left=%d top=%d right=%d bottom=%d\n", left, top, right, bottom);
    
    switch(pSmi->Chipset)
    {
    case SMI_750:
    case SMI_718:		
        SMI750_SETCLIP(pScrn,left,top,right,bottom);
        break;
    case SMI_712:
        SMI712_SETCLIP(pScrn,left,top,right,bottom);
        break;
    default:
        DEBUG("not supported chip:%04x\n",pSmi->Chipset);
        break;
    }
    
    LEAVE();

}

void SMI750_DISCLIP(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    ENTER();
    
    pSmi->ScissorsLeft = 0;
    pSmi->ClipTurnedOn = FALSE;
    deWaitForNotBusy();
    deSetClipping(0, pSmi->ScissorsLeft&0xFFFF, pSmi->ScissorsLeft>>16, 
                    pSmi->ScissorsRight&0xFFFF, pSmi->ScissorsRight>>16);
                    
    LEAVE();
}

void SMI712_DISCLIP(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    ENTER();

    pSmi->ScissorsLeft = 0;
    if(pScrn->bitsPerPixel == 24)
    {
        pSmi->ScissorsRight = (pScrn->virtualY<<16) | (pScrn->virtualX * 3);
    }
    else
    {
        pSmi->ScissorsRight = (pScrn->virtualY<<16) | pScrn->virtualX;
    }

    pSmi->ClipTurnedOn = FALSE;
    
    WaitQueue();
    WRITE_DPR(pSmi,0x2c,pSmi->ScissorsLeft);
    WRITE_DPR(pSmi,0x30,pSmi->ScissorsRight);
    
    LEAVE();

}


void SMI_DisableClipping(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();

    switch(pSmi->Chipset)
    {
    case SMI_750:
    case SMI_718:
        SMI750_DISCLIP(pScrn);        
        break;
    case SMI_712:
        SMI712_DISCLIP(pScrn);
        break;
    default:
        DEBUG("not supported chip:%04x\n",pSmi->Chipset);
        break;
    }
    
    LEAVE();
}

CARD32 SMI_DEDataFormat(int bpp) {
    CARD32 DEDataFormat = 0;

    switch (bpp) {
    case 8:
	DEDataFormat = 0x00000000;
	break;
    case 16:
	DEDataFormat = 0x00100000;
	break;
    case 24:
	DEDataFormat = 0x00300000;
	break;
    case 32:
	DEDataFormat = 0x00200000;
	break;
    }
    return DEDataFormat;
}

void
SMI_Rotate(ScrnInfoPtr pScrn, xf86CrtcPtr crtc, int x, int y, int w, int h)
{
    int dstX, dstY, dstPitch;
    uint32_t rBase;
    uint32_t de_ctrl = 0; 
    uint32_t maxRotationWidth;

    SMIPtr pSmi = SMIPTR(pScrn);
    if ( crtc == XF86_CRTC_CONFIG_PTR(pScrn)->crtc[0]) {
        rBase = READ_DCR(pSmi, PRIMARY_FB_ADDRESS&0xff);
    }
    else if ( crtc == XF86_CRTC_CONFIG_PTR(pScrn)->crtc[1]) {
        rBase = READ_DCR(pSmi, SECONDARY_FB_ADDRESS&0xff);
    }
    else {
        DEBUG("Wrong CRTC!\n");
        return;
    }

    switch (crtc->rotation) {
        case 2:
            dstX = y;
            dstY = crtc->mode.VDisplay - x - 2;
            dstPitch = crtc->mode.HDisplay;

            maxRotationWidth = 32 / pSmi->Bpp;
            deWaitForNotBusy();

            WRITE_DPR(pSmi, DE_WINDOW_SOURCE_BASE&0xff, 0);

            WRITE_DPR(pSmi, DE_WINDOW_DESTINATION_BASE&0xff, rBase);

            WRITE_DPR(pSmi, DE_PITCH&0xff,
                    FIELD_VALUE(0, DE_PITCH, DESTINATION, crtc->mode.HDisplay) |
                    FIELD_VALUE(0, DE_PITCH, SOURCE, pScrn->displayWidth));

            WRITE_DPR(pSmi, DE_WINDOW_WIDTH&0xff,
                    FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, crtc->mode.HDisplay) |
                    FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      pScrn->displayWidth));

            de_ctrl = FIELD_SET(0, DE_CONTROL, QUICK_START, ENABLE) |
                FIELD_SET(0, DE_CONTROL, COMMAND, ROTATE)  |
                FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
                FIELD_VALUE(0, DE_CONTROL, ROP, 0xc)      |
                FIELD_SET(0, DE_CONTROL, REPEAT_ROTATE, DISABLE) |
                deGetTransparency();

            if (dstY < w)
                w= dstY+1;

            de_ctrl |= (FIELD_SET(0, DE_CONTROL, STEP_X, NEGATIVE) |
                    FIELD_SET(0, DE_CONTROL, STEP_Y, POSITIVE));
                
            WRITE_DPR(pSmi, DE_CONTROL&0xff, de_ctrl);
            WRITE_DPR(pSmi, DE_SOURCE&0xff,
                    FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
                    FIELD_VALUE(0, DE_SOURCE, X_K1, x) |
                    FIELD_VALUE(0, DE_SOURCE, Y_K2, y));
            WRITE_DPR(pSmi, DE_DESTINATION&0xff,
                    FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
                    FIELD_VALUE(0, DE_DESTINATION, X, dstX) | 
                    FIELD_VALUE(0, DE_DESTINATION, Y, dstY));
            WRITE_DPR(pSmi, DE_DIMENSION&0xff,
                    FIELD_VALUE(0, DE_DIMENSION, X, maxRotationWidth) |
                    FIELD_VALUE(0, DE_DIMENSION, Y_ET, h));

            while (w > maxRotationWidth)
            {
                WRITE_DPR(pSmi, DE_SOURCE&0xff,
                        FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
                        FIELD_VALUE(0, DE_SOURCE, X_K1, x) |
                        FIELD_VALUE(0, DE_SOURCE, Y_K2, y));
                WRITE_DPR(pSmi, DE_DESTINATION&0xff,
                        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
                        FIELD_VALUE(0, DE_DESTINATION, X, dstX) | 
                        FIELD_VALUE(0, DE_DESTINATION, Y, dstY));
                WRITE_DPR(pSmi, DE_DIMENSION&0xff,
                        FIELD_VALUE(0, DE_DIMENSION, X, maxRotationWidth) |
                        FIELD_VALUE(0, DE_DIMENSION, Y_ET, h));
                w -= maxRotationWidth;
                x += maxRotationWidth;
                dstY -= maxRotationWidth;
            }

            if (w > 0) {
                WRITE_DPR(pSmi, DE_SOURCE&0xff,
                        FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
                        FIELD_VALUE(0, DE_SOURCE, X_K1, x) |
                        FIELD_VALUE(0, DE_SOURCE, Y_K2, y));
                WRITE_DPR(pSmi, DE_DESTINATION&0xff,
                        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
                        FIELD_VALUE(0, DE_DESTINATION, X, dstX) | 
                        FIELD_VALUE(0, DE_DESTINATION, Y, dstY));
                WRITE_DPR(pSmi, DE_DIMENSION&0xff,
                        FIELD_VALUE(0, DE_DIMENSION, X, w) |
                        FIELD_VALUE(0, DE_DIMENSION, Y_ET, h));
            }
            break;
        case 4:
        case 8:
        default :
            return;
    }
    SMI_EngineReset(pScrn);
    return;
}
