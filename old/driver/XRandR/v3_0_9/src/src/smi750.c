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

Except as contained in this notice, the names of The XFree86 Project and
Silicon Motion shall not be used in advertising or otherwise to promote the
sale, use or other dealings in this Software without prior written
authorization from The XFree86 Project or Silicon Motion.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "smi.h"
#include "smi_crtc.h"
#include "750ddk.h"
#include "smi750.h"
#include "regsmi.h"
#define DPMS_SERVER
#if XORG_VERSION_CURRENT < 10706000
#include <X11/extensions/dpms.h>
#endif
#include "compiler.h"



/* Want to see register dumps for now */
#undef VERBLEV
#define VERBLEV		1
static void
WriteAttr(SMIPtr pSmi, int index, int value)
{
    (void) inb(pSmi->PIOBase + VGA_IOBASE_COLOR + VGA_IN_STAT_1_OFFSET);

    index |= 0x20;
    outb(pSmi->PIOBase + VGA_ATTR_INDEX, index);
    outb(pSmi->PIOBase + VGA_ATTR_DATA_W, value);
}

static int
ReadAttr(SMIPtr pSmi, int index)
{
    (void) inb(pSmi->PIOBase + VGA_IOBASE_COLOR + VGA_IN_STAT_1_OFFSET);

    index |= 0x20;
    outb(pSmi->PIOBase + VGA_ATTR_INDEX, index);
    return (inb(pSmi->PIOBase + VGA_ATTR_DATA_R));
}

#define WriteMiscOut(value)	outb(pSmi->PIOBase + VGA_MISC_OUT_W, value)
#define ReadMiscOut()		inb(pSmi->PIOBase + VGA_MISC_OUT_R)
#define WriteSeq(index, value)	outb(pSmi->PIOBase + VGA_SEQ_INDEX, index);\
				outb(pSmi->PIOBase + VGA_SEQ_DATA, value)

static int
ReadSeq(SMIPtr pSmi, int index)
{
    outb(pSmi->PIOBase + VGA_SEQ_INDEX, index);
    return (inb(pSmi->PIOBase + VGA_SEQ_DATA));
}

#define WriteGr(index, value)				\
    outb(pSmi->PIOBase + VGA_GRAPH_INDEX, index);	\
    outb(pSmi->PIOBase + VGA_GRAPH_DATA, value)

static int
ReadGr(SMIPtr pSmi, int index)
{
    outb(pSmi->PIOBase + VGA_GRAPH_INDEX, index);

    return (inb(pSmi->PIOBase + VGA_GRAPH_DATA));
}

#define WriteCrtc(index, value)						     \
    outb(pSmi->PIOBase + (VGA_IOBASE_COLOR + VGA_CRTC_INDEX_OFFSET), index); \
    outb(pSmi->PIOBase + (VGA_IOBASE_COLOR + VGA_CRTC_DATA_OFFSET), value)

static void
SeqReset(SMIPtr pSmi, Bool start)
{
    if (start) {
	WriteSeq(0x00, 0x01);		/* Synchronous Reset */
    }
    else {
	WriteSeq(0x00, 0x03);		/* End Reset */
    }
}


void SMI750_RestoreFonts(ScrnInfoPtr pScrn)
{
    SMIPtr	pSmi = SMIPTR(pScrn);	
	vgaHWPtr hwp = VGAHWPTR(pScrn);
    unsigned char miscOut, attr10, gr1, gr3, gr4, gr5, gr6, gr8, seq2, seq4, scrn;

    if (pSmi->fonts == NULL)
		return;

    /* save the registers that are needed here */
    miscOut = ReadMiscOut();
    attr10 = ReadAttr(pSmi, 0x10);
    gr1 = ReadGr(pSmi, 0x01);
    gr3 = ReadGr(pSmi, 0x03);
    gr4 = ReadGr(pSmi, 0x04);
    gr5 = ReadGr(pSmi, 0x05);
    gr6 = ReadGr(pSmi, 0x06);
    gr8 = ReadGr(pSmi, 0x08);
    seq2 = ReadSeq(pSmi, 0x02);
    seq4 = ReadSeq(pSmi, 0x04);

    /* Force into colour mode */
    WriteMiscOut(miscOut | 0x01);

    scrn = ReadSeq(pSmi, 0x01) | 0x20;
    SeqReset(pSmi, TRUE);
    WriteSeq(0x01, scrn);
    SeqReset(pSmi, FALSE);

    WriteAttr(pSmi, 0x10, 0x01);	/* graphics mode */
    if (pScrn->depth == 4) {
	/* GJA */
	WriteGr(0x03, 0x00);	/* don't rotate, write unmodified */
	WriteGr(0x08, 0xFF);	/* write all bits in a byte */
	WriteGr(0x01, 0x00);	/* all planes come from CPU */
    }

    WriteSeq(0x02, 0x04);   /* write to plane 2 */
    WriteSeq(0x04, 0x06);   /* enable plane graphics */
    WriteGr(0x04, 0x02);    /* read plane 2 */
    WriteGr(0x05, 0x00);    /* write mode 0, read mode 0 */
    WriteGr(0x06, 0x05);    /* set graphics */
    slowbcopy_tobus(pSmi->fonts, hwp->Base, 8192);

    WriteSeq(0x02, 0x08);   /* write to plane 3 */
    WriteSeq(0x04, 0x06);   /* enable plane graphics */
    WriteGr(0x04, 0x03);    /* read plane 3 */
    WriteGr(0x05, 0x00);    /* write mode 0, read mode 0 */
    WriteGr(0x06, 0x05);    /* set graphics */
    slowbcopy_tobus(pSmi->fonts + 8192, hwp->Base, 8192);

    scrn = ReadSeq(pSmi, 0x01) & ~0x20;
    SeqReset(pSmi, TRUE);
    WriteSeq(0x01, scrn);
    SeqReset(pSmi, FALSE);

    /* restore the registers that were changed */
    WriteMiscOut(miscOut);
    WriteAttr(pSmi, 0x10, attr10);
    WriteGr(0x01, gr1);
    WriteGr(0x03, gr3);
    WriteGr(0x04, gr4);
    WriteGr(0x05, gr5);
    WriteGr(0x06, gr6);
    WriteGr(0x08, gr8);
    WriteSeq(0x02, seq2);
    WriteSeq(0x04, seq4);
}


/* save vga fonts */
void SMI750_SaveFonts(ScrnInfoPtr pScrn)
{	
    SMIPtr	pSmi = SMIPTR(pScrn);
	vgaHWPtr hwp = VGAHWPTR(pScrn);
    unsigned char miscOut, attr10, gr4, gr5, gr6, seq2, seq4, scrn;	

	if(pSmi->fonts != NULL)
		return;
	
	/* if in graphics mode,don't save anythig */	
	attr10 = ReadAttr(pSmi,0x10);
	if(attr10 & 1)
		return;
	pSmi->fonts = xalloc(8192*2);

	/* save the registers that needed here  */
	miscOut = ReadMiscOut();
	gr4 = ReadGr(pSmi,0x4);
	gr5 = ReadGr(pSmi,0x5);
    gr6 = ReadGr(pSmi, 0x06);
    seq2 = ReadSeq(pSmi, 0x02);
    seq4 = ReadSeq(pSmi, 0x04);

	/* force into colour mode  */	
    WriteMiscOut(miscOut | 0x01);
    scrn = ReadSeq(pSmi, 0x01) | 0x20;
    SeqReset(pSmi, TRUE);
    WriteSeq(0x01, scrn);
    SeqReset(pSmi, FALSE);
    WriteAttr(pSmi, 0x10, 0x01);	/* graphics mode */

	/* fonts 1 */
    WriteSeq(0x02, 0x04);	/* write to plane 2 */
    WriteSeq(0x04, 0x06);	/* enable plane graphics */
    WriteGr(0x04, 0x02);	/* read plane 2 */
    WriteGr(0x05, 0x00);	/* write mode 0, read mode 0 */
    WriteGr(0x06, 0x05);	/* set graphics */
    slowbcopy_frombus(hwp->Base, pSmi->fonts, 8192);

	/* fonts 2 */	
    WriteSeq(0x02, 0x08);	/* write to plane 3 */
    WriteSeq(0x04, 0x06);	/* enable plane graphics */
    WriteGr(0x04, 0x03);	/* read plane 3 */
    WriteGr(0x05, 0x00);	/* write mode 0, read mode 0 */
    WriteGr(0x06, 0x05);	/* set graphics */
    slowbcopy_frombus(hwp->Base, pSmi->fonts + 8192, 8192);

	scrn = ReadSeq(pSmi,0x1) & ~0x20;
	SeqReset(pSmi,TRUE);
	WriteSeq(0x01,scrn);
	SeqReset(pSmi,FALSE);

	/* Restore clobbered registers */
	WriteAttr(pSmi, 0x10, attr10);
	WriteSeq(0x02, seq2);
	WriteSeq(0x04, seq4);
	WriteGr(0x04, gr4);
	WriteGr(0x05, gr5);
	WriteGr(0x06, gr6);
	WriteMiscOut(miscOut);	
}



void
SMI750_Save(ScrnInfoPtr pScrn)
{
    ENTER();	
    int		i, j;
    SMIPtr	pSmi = SMIPTR(pScrn);
    SMI750RegPtr save = pSmi->save;

	/* save mmio */
	
    for (i = REG_SYS_HEAD, j = 0; i <= REG_SYS_TAIL; i += 4, j++){
		//save->System[j] = (uint32_t)READ_SCR(pSmi, i);
		save->System[j] = PEEK32(i);
    }

    for (i = REG_PNL_HEAD, j = 0; i <= REG_PNL_TAIL; i += 4, j++){
		//save->PanelControl[j] = (uint32_t)READ_SCR(pSmi, i);
		save->PanelControl[j] = PEEK32(i);
    }
	
    for (i = REG_CRT_HEAD, j = 0; i<=REG_CRT_TAIL; i+= 4, j++) {
        //save->CRTControl[j] = (uint32_t)READ_SCR(pSmi, i);
        save->CRTControl[j] = PEEK32(i);
    }

    /*  alpha layer is tough ,bugs occurs when setting registers on alpha layer  */
#if 0
    for (i= REG_ALPH_HEAD, j = 0; i<=REG_ALPH_TAIL;i+=4,j++){
        save->AlphaControl[j] = (uint32_t)READ_SCR(pSmi,i);
    }
#endif

    for(i=REG_PCUR_HEAD,j = 0;i<=REG_PCUR_TAIL;i+=4,j++){
        //save->PriCursorControl[j] = (uint32_t)READ_SCR(pSmi,i);
        save->PriCursorControl[j] = PEEK32(i);
    }
    
    for(i=REG_SCUR_HEAD,j = 0;i<=REG_SCUR_TAIL;i+=4,j++){
        //save->SecCursorControl[j] = (uint32_t)READ_SCR(pSmi,i);
        save->SecCursorControl[j] = PEEK32(i);
    }
    
    LEAVE();
}



Bool
SMI750_HWInit(ScrnInfoPtr pScrn)
{
    //logicalMode_t panelMode; /* Mode for the LCD channel */
    //logicalMode_t crtMode;   /* Mode for CRT channel */
    //SMIPtr	pSmi = SMIPTR(pScrn);
	
    ENTER();    

    /* This must be the first function to call */
    if (initChip() != 0)
    {
        xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, VERBLEV,
		   "Silicon Motion chip is not detected!!!\n\n");
        LEAVE  (-1);
    }

	if(initDisplay() != 0)
	{
		//LEAVE(-1);
	}
		
	
    LEAVE (TRUE);	
}

// Setting Panel registers  for a Panel mode
void
SMI750_WriteMode_lcd(ScrnInfoPtr pScrn)
{
    ENTER();
    LEAVE ();
}

//simple restore --wish to add the function to DDK
void
SMI750_Restore(ScrnInfoPtr pScrn, SMI750RegPtr restore)
{

    ENTER();
	int		i, j;
    SMIPtr	pSmi = SMIPTR(pScrn);
    SMI750RegPtr	save = pSmi->save;

    for(i = REG_SYS_HEAD,j = 0 ;i<= REG_SYS_TAIL ;i+=4,j++)
    {
        POKE32(i,save->System[j]);
    }
    
    for(i = REG_PNL_HEAD, j = 0 ;i<= REG_PNL_TAIL ;i+=4,j++)
    {
        POKE32(i,save->PanelControl[j]);
    }
    
    for(i = REG_CRT_HEAD,j = 0 ;i<= REG_CRT_TAIL ;i+=4,j++)
    {
        POKE32(i,save->CRTControl[j]);
    }
#if 0    
    for(i = REG_ALPH_HEAD ;i<= REG_ALPH_TAIL ;i+=4)
    {
        WRITE_SCR(pSmi,i,save->AlphaControl[(i-REG_ALPH_HEAD)>>2]);
    }
#endif
    for(i = REG_PCUR_HEAD,j = 0 ;i<= REG_PCUR_TAIL ;i+=4,j++)
    {        
        POKE32(i,save->PriCursorControl[j]);
    }

    
    for(i = REG_SCUR_HEAD,j = 0 ;i<= REG_SCUR_TAIL ;i+=4,j++)
    {       
        POKE32(i,save->SecCursorControl[j]);
    }

	SMI750_RestoreFonts(pScrn);


    /* set alphaOkay flag to 0 when come into console */
 
    //SM750ENT(pSmi)->alphaOkay = 0;
    pSmi->entityPrivate->alphaOkay = 0;
	LEAVE ();
}

void
SMI750_DisplayPowerManagementSet(ScrnInfoPtr pScrn,int PowerManagementMode, int flags)
{
    SMIPtr pSmi = SMIPTR(pScrn);
	
    ENTER();
	
    if (pSmi->CurrentDPMS != PowerManagementMode)
	{
		/* Set the DPMS mode to every output and CRTC */
		xf86DPMSSet(pScrn, PowerManagementMode, flags);
		pSmi->CurrentDPMS = PowerManagementMode;
    }

    LEAVE ();
}


static char *
format_integer_base2(int32_t word)
{
    int		i;
    static char	buffer[33];

    for (i = 0; i < 32; i++) {    
	if (word & (1 << i))
	    buffer[31 - i] = '1';
	else
	    buffer[31 - i] = '0';
    }
    return (buffer);
}

void
SMI750_PrintRegs(ScrnInfoPtr pScrn)
{
    int		i;
    SMIPtr	pSmi = SMIPTR(pScrn);

    xf86ErrorFVerb(VERBLEV, "SMI750 System Setup:\n");
	
    xf86ErrorFVerb(VERBLEV, "\n\nSCR    x0       x4       x8       xC");
    for (i = 0x00; i <= 0x88; i += 4) {
	if ((i & 0xF) == 0x0) xf86ErrorFVerb(VERBLEV, "\n%02X|", i);
	xf86ErrorFVerb(VERBLEV, " %08lX", (uint32_t)READ_SCR(pSmi, i));
    }

    xf86ErrorFVerb(VERBLEV, "\n\nDCR    x0       x4       x8       xC");
    for (i = 0x00; i <= 0x38; i += 4) {
	if ((i & 0xF) == 0x0) xf86ErrorFVerb(VERBLEV, "\n%02X|", i);
	xf86ErrorFVerb(VERBLEV, " %08lX", (uint32_t)READ_DCR(pSmi, i));
    }
	
    xf86ErrorFVerb(VERBLEV, "\n\nDCR    x0       x4       x8       xC");
    for (i = 0x200; i <= 0x218; i += 4) {
	if ((i & 0xF) == 0x0) xf86ErrorFVerb(VERBLEV, "\n%02X|", i);
	xf86ErrorFVerb(VERBLEV, " %08lX", (uint32_t)READ_DCR(pSmi, i));
    }
}

