/*
Copyright (C) 1994-1999 The XFree86 Project, Inc.  All Rights Reserved.
Copyright (C) 2000 Silicon Motion, Inc.  All Rights Reserved.
Copyright (C) 2008 Mandriva Linux.  All Rights Reserved.

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

#ifndef _SMI_750_H
#define _SMI_750_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mode.h"
#include "display.h"
#include "smi.h"
#include "750ddk.h"


#define PEEK32(address)                peekRegisterDWord(address)
#define POKE32(address, value)         pokeRegisterDWord(address, value)

//simple save --wish to add the function to DDK
#define REG_SYS_HEAD    0x0
#define REG_SYS_TAIL    0x88
#define REG_PNL_HEAD    0x80000
#define REG_PNL_TAIL    0x80034
#define REG_CRT_HEAD    0x80200
#define REG_CRT_TAIL    0x80228
#define REG_ALPH_HEAD   0x80100
#define REG_ALPH_TAIL   0x80114
#define REG_PCUR_HEAD   0x800f0
#define REG_PCUR_TAIL   0x800fc
#define REG_SCUR_HEAD   0x80230
#define REG_SCUR_TAIL   0x80240
typedef struct
{
    CARD32 System[(REG_SYS_TAIL - REG_SYS_HEAD)/4+1];    //0x00 -- 0x88
    CARD32 PanelControl[(REG_PNL_TAIL - REG_PNL_HEAD)/4 +1];    //0x80000 -- 0x80034
    CARD32 CRTControl[(REG_CRT_TAIL - REG_CRT_HEAD)/4 + 1];     //0x80200 -- 0x80228
    CARD32 AlphaControl[(REG_ALPH_TAIL - REG_ALPH_HEAD)/4+1];     //0x80100 -- 0x80114
    CARD32 PriCursorControl[(REG_PCUR_TAIL - REG_PCUR_HEAD)/4+1];  
    CARD32 SecCursorControl[(REG_SCUR_TAIL - REG_SCUR_HEAD)/4+1];
    
} SMI750RegRec, *SMI750RegPtr;

typedef enum
{
	HEAD_TFT0,	
	HEAD_DVI,	
	HEAD_VGA,
	HEAD_TFT1,
}output_head_t;

#if 0
typedef struct
{		
	unsigned char *	mmio;
	int lastInstance;
	CARD32	totalVideoRam;	
	
	int xlcd;
	int ylcd;
	int	TFT;	
	int alphaOkay;
}SMI750EntRec,*SMI750EntPtr;

#define SM750ENT(pSmi)    ((SMI750EntPtr)((pSmi)->entityPrivate))   
#define CAST_2_750ENT(pEntPriv)   ((SMI750EntPtr)(pEntPriv))

#endif
void SMI750_Save(ScrnInfoPtr pScrn);

Bool SMI750_HWInit(ScrnInfoPtr pScrn);
void SMI750_WriteMode_lcd(ScrnInfoPtr pScrn);
void SMI750_Restore(ScrnInfoPtr pScrn, SMI750RegPtr restore);
void SMI750_DisplayPowerManagementSet(ScrnInfoPtr pScrn, int PowerManagementMode, int flags);
void SMI750_PrintRegs(ScrnInfoPtr pScrn);


/* smi750_crtc.c */
Bool SMI750_CrtcPreInit(ScrnInfoPtr pScrn);

/* smi750_output.c */
Bool SMI750_OutputPreInit(ScrnInfoPtr pScrn);
Bool SMI750_LinkPathCrtcc(ScrnInfoPtr pScrn,disp_path_t disp,disp_control_t control);
Bool SMI750_SetHead(ScrnInfoPtr pScrn,output_head_t head,disp_state_t state);
void SMI750_OutputModeSet(xf86OutputPtr output, DisplayModePtr mode, DisplayModePtr adjusted_mode);
void SMI750_AccelSync(ScrnInfoPtr pScrn);

//void SMI750_OutputModeSet(void* output, DisplayModePtr mode, DisplayModePtr adjusted_mode);


#endif
