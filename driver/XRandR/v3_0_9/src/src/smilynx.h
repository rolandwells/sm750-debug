/*
Copyright (C) 2008 Francisco Jerez.  All Rights Reserved.

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
*/

#ifndef _SMI_LYNX_H
#define _SMI_LYNX_H

#include "smi.h"


typedef struct
{    
    CARD16 h_res;
    CARD16 v_res;
    CARD16 vsync;//vertical refresh rate:only 60,75,85 is valid for sm712   
    CARD8 SVR[14];//shadow vga register:svr40==>svr4d    
    CARD8 CCR[2];//clock control register: ccr6c ccr6d    
}SMI712CrtTiming,*SMI712CrtTimingPtr;

typedef struct 
{
    CARD16 h_res;
    CARD16 v_res;
    CARD16 vsync;
    CARD8   FPR[14];//FPR50 => FPR57 + FPR 5A
    CARD8   CCR[2];//CCR6E && CCR6F
}SMI712PnlTiming,*SMI712PnlTimingPtr;

/* Driver data structure; this should contain all needed info for a mode */
typedef struct
{
    CARD16 mode;

    CARD8 SR17, SR18;
    CARD8 SR20, SR21, SR22, SR23, SR24;
    CARD8 SR30, SR31, SR32, SR34;
    CARD8 SR40, SR41, SR42, SR43, SR44, SR45, SR48, SR49, SR4A, SR4B, SR4C;
    CARD8 SR50, SR51, SR52, SR53, SR54, SR55, SR56, SR57, SR5A;
    CARD8 SR66, SR68, SR69, SR6A, SR6B, SR6C, SR6D, SR6E, SR6F;
    CARD8 SR81, SRA0;

    CARD8 CR30, CR33, CR33_2, CR3A;
    CARD8 CR40[14], CR40_2[14];
    CARD8 CR90[15], CR9F, CR9F_2;
    CARD8 CRA0[14];

    CARD8	smiDACMask, smiDacRegs[256][3];
    CARD8	smiFont[8192];

    CARD32	DPR10, DPR1C, DPR20, DPR24, DPR28, DPR2C, DPR30, DPR3C, DPR40,
		DPR44;
    CARD32	VPR00, VPR0C, VPR10;
    CARD32	CPR00;
    CARD32	FPR00_, FPR0C_, FPR10_;
}SMILynxRegRec, *SMILynxRegPtr;


typedef struct
{
	

}SMI712_RegRec,*SNU712_RegRecPtr;
#if 0
typedef struct
{		
    unsigned char * mmio;
    int lastInstance;
	CARD32	totalVideoRam;
	
	int xlcd;
	int ylcd;
	int	TFT;
}SMI712EntRec,*SMI712EntPtr;
#endif

/* smilynx_hw.c */
/* Initialize the CRTC-independent hardware registers */
Bool SMILynx_HWInit(ScrnInfoPtr pScrn);
void SMILynx_Save (ScrnInfoPtr pScrn);
void SMILynx_Restore (ScrnInfoPtr, SMILynxRegPtr);
void SMILynx_DisplayPowerManagementSet(ScrnInfoPtr pScrn,int PowerManagementMode, int flags);
xf86MonPtr SMILynx_ddc1(ScrnInfoPtr pScrn);
void SMILynx_PrintRegs(ScrnInfoPtr);

/* smilynx_crtc.c */
Bool SMILynx_CrtcPreInit(ScrnInfoPtr pScrn);
Bool SMI712_CrtcPreInit(ScrnInfoPtr pScrn);


/* smilynx_output.c */
Bool SMILynx_OutputPreInit(ScrnInfoPtr pScrn);
Bool SMI712_OutputPreInit(ScrnInfoPtr pScrn);
void SMI712_OutputModeSet(xf86OutputPtr output, DisplayModePtr mode, DisplayModePtr adjusted_mode);
void SMI712_AccelSync(ScrnInfoPtr pScrn);
int sm712_mess_reg_status(ScrnInfoPtr);




//#define SM712ENT(pSmi)    ((SMI712EntPtr)(pSmi->entityPrivate))
//#define CAST_2_712ENT(pEntPriv)   ((SMI712EntPtr)(pEntPriv))

#endif
