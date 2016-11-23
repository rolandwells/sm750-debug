/*
Copyright (C) 1994-1999 The XFree86 Project, Inc.  All Rights Reserved.
Copyright (C) 2000 Silicon Motion, Inc.  All Rights Reserved.
Copyright (C) 2008 Francisco Jerez. All Rights Reserved.

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
#include "smilynx.h"
#include "display.h"
extern const SMI712CrtTiming g_sm712_ModeTable[];
extern int g_sm712_mode_cnt;
extern const SMI712PnlTiming g_sm712_ModeTable2[];
extern int g_sm712_mode2_cnt;


static Bool CLOSE(unsigned short uv,float fv){
	unsigned short uv2;
	uv2 = (unsigned short)fv;
	if(uv - uv2 < 3 && uv - uv2 > -3)
		return TRUE;
	else
		return FALSE;	
}

static void
SMILynx_OutputDPMS_crt(xf86OutputPtr output, int mode)
{
    ScrnInfoPtr pScrn = output->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    SMILynxRegPtr reg = pSmi->mode;
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    ENTER();

    

    switch (mode) {
    case DPMSModeOn:
	reg->SR31 |= 0x02; /* Enable CRT display*/
	reg->SR22 = (reg->SR22 & ~0x30) | 0x00; /* Set DPMS state*/
	break;
    case DPMSModeStandby:
	reg->SR31 |= 0x02; /* Enable CRT display*/
	reg->SR22 = (reg->SR22 & ~0x30) | 0x10; /* Set DPMS state*/
	break;
    case DPMSModeSuspend:
	reg->SR31 |= 0x02; /* Enable CRT display*/
	reg->SR22 = (reg->SR22 & ~0x30) | 0x20; /* Set DPMS state*/
	break;
    case DPMSModeOff:
	reg->SR31 &= ~0x02; /* Disable CRT display*/
	reg->SR22 = (reg->SR22 & ~0x30) | 0x30; /* Set DPMS state*/
	break;
    }

    /* Wait for vertical retrace */

    //while (hwp->readST01(hwp) & 0x8) ;
    //while (!(hwp->readST01(hwp) & 0x8)) ;

    /* Write the registers */
    VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x22, reg->SR22);
    VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31, reg->SR31);

    LEAVE();

}

static void
SMILynx_OutputDPMS_lcd(xf86OutputPtr output, int mode)
{
    ScrnInfoPtr pScrn = output->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    SMILynxRegPtr reg = pSmi->mode;
    xf86CrtcConfigPtr crtcConf = XF86_CRTC_CONFIG_PTR(pScrn);

    ENTER();

    switch (mode) {
    case DPMSModeOn:
	if(pSmi->DualView &&
	   output->crtc == crtcConf->crtc[1]){
	    /* Virtual Refresh is enabled */

	    reg->SR21 &= ~0x10; /* Enable LCD framebuffer read operation and DSTN dithering engine */
	}else{
	    if(pSmi->lcd == 2){
		/* LCD is DSTN */

		reg->SR21 &= ~0x10; /* Enable LCD framebuffer read operation and DSTN dithering engine */
		reg->SR21 &= ~0x20; /* Enable LCD framebuffer write operation */
	    }
	}

	reg->SR31 |= 0x01; /* Enable LCD display*/
	break;
    case DPMSModeStandby:
    case DPMSModeSuspend:
    case DPMSModeOff:
	reg->SR21 |= 0x30; /* Disable LCD framebuffer r/w operation */
	reg->SR31 &= ~0x01; /* Disable LCD display*/
	break;
    }

    /* Write the registers */
    VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21, reg->SR21);
    VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31, reg->SR31);

    LEAVE();


}

static void
SMILynx_OutputDPMS_bios(xf86OutputPtr output, int mode)
{
    ScrnInfoPtr pScrn = output->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();

    pSmi->pInt10->ax = 0x4F10;
    switch (mode) {
    case DPMSModeOn:
	pSmi->pInt10->bx = 0x0001;
	break;
    case DPMSModeStandby:
	pSmi->pInt10->bx = 0x0101;
	break;
    case DPMSModeSuspend:
	pSmi->pInt10->bx = 0x0201;
	break;
    case DPMSModeOff:
	pSmi->pInt10->bx = 0x0401;
	break;
    }
    pSmi->pInt10->cx = 0x0000;
    pSmi->pInt10->num = 0x10;
    xf86ExecX86int10(pSmi->pInt10);

    LEAVE();
}


static DisplayModePtr
SMILynx_OutputGetModes_crt(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    xf86MonPtr pMon = NULL;

    ENTER();

    if(xf86LoaderCheckSymbol("xf86PrintEDID")){ /* Ensure the DDC module is loaded*/
	/* Try VBE */
	if(pSmi->pVbe){
	    pMon = vbeDoEDID(pSmi->pVbe, NULL);
	    if ( pMon != NULL &&
		 (pMon->rawData[0] == 0x00) &&
		 (pMon->rawData[1] == 0xFF) &&
		 (pMon->rawData[2] == 0xFF) &&
		 (pMon->rawData[3] == 0xFF) &&
		 (pMon->rawData[4] == 0xFF) &&
		 (pMon->rawData[5] == 0xFF) &&
		 (pMon->rawData[6] == 0xFF) &&
		 (pMon->rawData[7] == 0x00)) {
		xf86OutputSetEDID(output,pMon);
		LEAVE(xf86OutputGetEDIDModes(output));
	    }
	}

	/* Try DDC2 */
	if(pSmi->I2C_primary){
	    pMon=xf86OutputGetEDID(output,pSmi->I2C_primary);
	    if(pMon){
		xf86OutputSetEDID(output,pMon);
		LEAVE(xf86OutputGetEDIDModes(output));
	    }
	}

	/* Try DDC1 */
	pMon=SMILynx_ddc1(pScrn);
	if(pMon){
	    xf86OutputSetEDID(output,pMon);
	    LEAVE(xf86OutputGetEDIDModes(output));
	}
    }

    LEAVE(NULL);
}



static DisplayModePtr
SMILynx_OutputGetModes_pnl(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    xf86MonPtr pMon = NULL;

    ENTER();


    if(xf86LoaderCheckSymbol("xf86PrintEDID")){ /* Ensure the DDC module is loaded*/
#if 1
	/* Try VBE */
	if(pSmi->pVbe){
	    pMon = vbeDoEDID(pSmi->pVbe, NULL);
	    if ( pMon != NULL &&
		 (pMon->rawData[0] == 0x00) &&
		 (pMon->rawData[1] == 0xFF) &&
		 (pMon->rawData[2] == 0xFF) &&
		 (pMon->rawData[3] == 0xFF) &&
		 (pMon->rawData[4] == 0xFF) &&
		 (pMon->rawData[5] == 0xFF) &&
		 (pMon->rawData[6] == 0xFF) &&
		 (pMon->rawData[7] == 0x00)) {
		xf86OutputSetEDID(output,pMon);
		LEAVE(xf86OutputGetEDIDModes(output));
	    }
	}
#endif

	/* Try DDC2 */
	if(pSmi->I2C_secondary){
	    pMon=xf86OutputGetEDID(output,pSmi->I2C_secondary);
	    if(pMon){
		xf86OutputSetEDID(output,pMon);
		LEAVE(xf86OutputGetEDIDModes(output));
	    }
	}

	/* Try DDC1 */
	pMon=SMILynx_ddc1_pnl(pScrn);
	if(pMon){
	    xf86OutputSetEDID(output,pMon);
	    LEAVE(xf86OutputGetEDIDModes(output));
	}
    }

    LEAVE(NULL);
}


static xf86OutputStatus
SMILynx_OutputDetect_crt(xf86OutputPtr output)
{
    SMIPtr pSmi = SMIPTR(output->scrn);
    SMILynxRegPtr mode = pSmi->mode;
    vgaHWPtr hwp = VGAHWPTR(output->scrn);
    CARD8 SR7D;
    Bool status;

    ENTER();

    SR7D = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x7D);

    VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21, mode->SR21 & ~0x88); /* Enable DAC and color palette RAM */
    VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x7B, 0x40); /* "TV and RAMDAC Testing Power", Green component */
    VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x7D, SR7D | 0x10); /* Enable monitor detect */

    /* Wait for vertical retrace */
    while (!(hwp->readST01(hwp) & 0x8)) ;
    while (hwp->readST01(hwp) & 0x8) ;

    status = VGAIN8(pSmi, 0x3C2) & 0x10;

    /* Restore previous state */
    VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21, mode->SR21);
    VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x7D, SR7D);

    if(status)
	LEAVE(XF86OutputStatusConnected);
    else
	LEAVE(XF86OutputStatusDisconnected);
}

static int SMI712_OutputModeValid_PNL(xf86OutputPtr output, DisplayModePtr mode)
{
    int index,ret;
    SMI712CrtTiming * timing;    
	ScrnInfoPtr pScrn = output->scrn;
	SMIPtr pSmi = SMIPTR(pScrn);
    
    int xres = mode->HDisplay;
    int yres = mode->VDisplay;
    float VRefresh = mode->VRefresh;
    float HSync = mode->HSync;       

    ENTER();    
	
	if(HSync < 1.0f){
		HSync = mode->Clock/mode->HTotal;
		HSync *= 1000;
		HSync /= mode->VTotal;
		//XERR("monk: hync == %f\n",HSync);
	}
	
    index = 0;
    timing = &g_sm712_ModeTable2[0];

    if(pSmi->entityPrivate->xlcd != 0 && pSmi->entityPrivate->ylcd != 0)
    {
        if(xres == pSmi->entityPrivate->xlcd && yres == pSmi->entityPrivate->ylcd)
            LEAVE(MODE_OK);
        else
            ret = (MODE_CLOCK_RANGE);
        
    }
    else if(xres*yres*pSmi->Bpp > (pSmi->DualView?pSmi->FBReserved/2:pSmi->FBReserved))
    {
     //   XERR("Mode %dx%dx%d require %d while we can only provide %d\n",xres,yres,pSmi->Bpp,
    //     xres*yres*pSmi->Bpp,pSmi->DualView?pSmi->FBReserved/2:pSmi->FBReserved);        
        ret = MODE_MEM;        
    }
    else
    {
        ret = MODE_CLOCK_RANGE;    
        while(index < g_sm712_mode2_cnt)
        {
            if(timing[index].h_res == xres &&
				timing[index].v_res == yres &&
				CLOSE(timing[index].vsync,HSync))
                LEAVE(MODE_OK);            
            index++;        
        }
    }
    //XERR("Mode [%d,%d]@%f not supported by sm712 PNL\n",xres,yres,VRefresh);
    LEAVE(ret);
}

static int SMI712_OutputModeValid_CRT(xf86OutputPtr output, DisplayModePtr mode)
{
    int index,ret;
    SMI712CrtTiming * timing;    
	ScrnInfoPtr pScrn = output->scrn;
	SMIPtr pSmi = SMIPTR(pScrn);
    
    int xres = mode->HDisplay;
    int yres = mode->VDisplay;
    float VRefresh = mode->VRefresh;
    float HSync = mode->HSync;       
    
    ENTER();    

    //XERR("CHECK Mode [%d,%d]@%f 712 CRT\n",xres,yres,VRefresh); 
#if 0    
    if(xres == 640 && yres == 480)
	{
	    XERR("HSync = %f,VRefresh = %f\n",HSync,VRefresh);
		XERR("Clock = %d khz\n",mode->Clock);
		XERR("htotal:%d,vtotal == %d\n",mode->HTotal,mode->VTotal);		
	}
#endif
	if(HSync < 1.0f){
		HSync = mode->Clock/mode->HTotal;
		HSync *= 1000;
		HSync /= mode->VTotal;
		//XERR("htotal:%d,vtotal == %d\n",mode->HTotal,mode->VTotal);		
		//XERR("hync == %f\n",HSync);
		//XERR("\n");
	}
		
     
    index = 0;
    timing = &g_sm712_ModeTable[0]; 
 
    if(xres*yres*pSmi->Bpp > (pSmi->DualView?pSmi->FBReserved/2:pSmi->FBReserved))
    {       
        //XERR("Mode %dx%dx%d require %d while we can only provide %d\n",xres,yres,pSmi->Bpp,
          //  xres*yres*pSmi->Bpp,pSmi->DualView?pSmi->FBReserved/2:pSmi->FBReserved);            
        ret = MODE_MEM;        
    }
    else
    {
        ret = MODE_CLOCK_RANGE;
        while(index < g_sm712_mode_cnt)
        {   
            if(timing[index].h_res == xres && 
				timing[index].v_res == yres &&
				CLOSE(timing[index].vsync,HSync))
                LEAVE(MODE_OK);
            index++;        
        }    
    }    
	
    //XERR("Mode [%d,%d]@%f is not supported by sm712 CRT\n",xres,yres,VRefresh); 
    LEAVE(ret); 
}


DisplayModePtr SMI712_OutputGetModes_pnl(xf86OutputPtr output)
{
    SMIPtr pSmi = SMIPTR(output->scrn);
 
    ENTER();
#define X (pSmi->entityPrivate->xlcd)
#define Y (pSmi->entityPrivate->ylcd)

#ifdef HAVE_XMODES
//    XMSG("lcdWidth,lcdHeight = %d %d \n",X,Y); 
    if(X != 0 && Y != 0)
        LEAVE(xf86CVTMode(X,Y,60.0f,FALSE,FALSE));
#endif
    LEAVE(NULL);
#undef X
#undef Y
}


Bool SMI712_OutputPreInit(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    xf86OutputPtr output;
    xf86OutputFuncsPtr outputFuncs;
    SMIOutputPrivatePtr outputPriv;

    ENTER();

    
	/* Output 0 is CRT */
	SMI_OutputFuncsInit_base(&outputFuncs);

	if(pSmi->useBIOS)
	    outputFuncs->dpms = SMILynx_OutputDPMS_bios;
	else
	    outputFuncs->dpms = SMILynx_OutputDPMS_crt;

	outputFuncs->get_modes = SMILynx_OutputGetModes_crt;
	outputFuncs->detect = SMI_OutputDetect_lcd;
	outputFuncs->mode_valid = SMI712_OutputModeValid_CRT;

	if(! (output = xf86OutputCreate(pScrn,outputFuncs,"CRT")))
	    LEAVE(FALSE);
    
    outputPriv = xnfcalloc(sizeof(SMIOutputPrivateRec),1);  
    outputPriv->index = 0;
    outputPriv->path = 0;//CRT head  for sm712
    
    output->driver_private = outputPriv;
	output->interlaceAllowed = FALSE;
	output->doubleScanAllowed = FALSE;
    output->possible_crtcs = 1;
    //output->possible_clones = 1 << 1;

	if(pSmi->DualView){
	    /* Output 1 is PNL */
	    SMI_OutputFuncsInit_base(&outputFuncs);
	    outputFuncs->dpms = SMILynx_OutputDPMS_lcd;
//	    outputFuncs->get_modes = SMI712_OutputGetModes_pnl;
	    outputFuncs->get_modes = SMILynx_OutputGetModes_pnl; 
		outputFuncs->detect = SMI_OutputDetect_lcd;
		outputFuncs->mode_valid = SMI712_OutputModeValid_PNL;

	    if(! (output = xf86OutputCreate(pScrn,outputFuncs,"PNL")))
    		LEAVE(FALSE);
		
        outputPriv = xnfcalloc(sizeof(SMIOutputPrivateRec),1);  
        outputPriv->index = 1;
        outputPriv->path = 1;//LCD head  for sm712
        
        output->driver_private = outputPriv;        
	    output->interlaceAllowed = FALSE;
	    output->doubleScanAllowed = FALSE;
	    output->possible_crtcs = 2;
	    //output->possible_clones = 1 << 1;
	}

    LEAVE(TRUE);
}


Bool SMILynx_OutputPreInit(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    xf86OutputPtr output;
    xf86OutputFuncsPtr outputFuncs;

    ENTER();

    if(pSmi->Chipset == SMI_COUGAR3DR){
	/* Output 0 is LCD */
	SMI_OutputFuncsInit_base(&outputFuncs);

	if(pSmi->useBIOS)
	    outputFuncs->dpms = SMILynx_OutputDPMS_bios;
	else
	    outputFuncs->dpms = SMILynx_OutputDPMS_lcd;

	outputFuncs->get_modes = SMI_OutputGetModes_native;
	outputFuncs->detect = SMI_OutputDetect_lcd;

	if(! (output = xf86OutputCreate(pScrn,outputFuncs,"LVDS")))
	    LEAVE(FALSE);

	output->possible_crtcs = 1 << 0;
	output->possible_clones = 0;
	output->interlaceAllowed = FALSE;
	output->doubleScanAllowed = FALSE;
    }else{
	/* Output 0 is LCD */
	SMI_OutputFuncsInit_base(&outputFuncs);

	if(pSmi->useBIOS)
	    outputFuncs->dpms = SMILynx_OutputDPMS_bios;
	else
	    outputFuncs->dpms = SMILynx_OutputDPMS_lcd;

	outputFuncs->get_modes = SMI_OutputGetModes_native;
	outputFuncs->detect = SMI_OutputDetect_lcd;

	if(! (output = xf86OutputCreate(pScrn,outputFuncs,"LVDS")))
	    LEAVE(FALSE);

	output->interlaceAllowed = FALSE;
	output->doubleScanAllowed = FALSE;
	output->possible_crtcs = (1 << 0) | (1 << 1);
	output->possible_clones = 1 << 1;

	if(pSmi->DualView){
	    /* Output 1 is CRT */
	    SMI_OutputFuncsInit_base(&outputFuncs);
	    outputFuncs->dpms = SMILynx_OutputDPMS_crt;
	    outputFuncs->get_modes = SMILynx_OutputGetModes_crt;

	    if(pSmi->Chipset == SMI_LYNX3DM)
		outputFuncs->detect = SMILynx_OutputDetect_crt;

	    if(! (output = xf86OutputCreate(pScrn,outputFuncs,"VGA")))
		LEAVE(FALSE);

	    output->interlaceAllowed = FALSE;
	    output->doubleScanAllowed = FALSE;

	    output->possible_crtcs = 1 << 0;
	    output->possible_clones = 1 << 0;
	}
    }

    LEAVE(TRUE);
}

void SMI712_OutputModeSet(xf86OutputPtr output, DisplayModePtr mode, DisplayModePtr adjusted_mode)
{
	ScrnInfoPtr pScrn;
//	xf86CrtcPtr crtc;
//	SMICrtcPrivatePtr crtcPriv;
	SMIOutputPrivatePtr outPriv;
	SMIPtr pSmi;
//	SMI712EntPtr pEnt;
//    SMIEntPtr pEnPriv;
	CARD8 tmp;

	ENTER();
    pScrn = output->scrn;
//    crtc = output->crtc;  
//    crtcPriv = SMICRTC(crtc);
    outPriv = SMIOUTPUT(output);    
    pSmi = SMIPTR(pScrn);
//    pEnPriv = pSmi->entityPrivate;

    //XERR("outPriv->index = %d,path = %d \n",outPriv->index,outPriv->path);

    if(!outPriv->path)
    {   /* sm712 CRT head */
        
    }
    else
    {   /* sm712 LCD head */
        tmp = VGAIN8_INDEX(pSmi,VGA_SEQ_INDEX,VGA_SEQ_DATA,0x30);
        //XERR("FPR30 = %x\n",tmp);
        switch (pSmi->entityPrivate->TFT)
        {            
            case TFT_18BIT:
                VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x30,(0x20|(tmp&0x8f)));
                break;
            case TFT_36BIT:
                VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x30,(0x60|(tmp&0x8f)));
                break;
            case TFT_12BIT:
                VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x30,(0x10|(tmp&0x8f)));
                break;
            case TFT_9BIT:
                VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x30,(0x00|(tmp&0x8f)));
                break;
/*                
            case TFT_D24BIT:
                VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x30,(0x40|(tmp&0x8f)));
                break;
            case TFT_ANALOG:
                VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x30,(0x50|(tmp&0x8f)));
                break;*/                
            
            case -1:
            case TFT_24BIT:
                VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x30,(0x30|(tmp&0x8f)));
                //XERR("set FPR30  TO %x\n",VGAIN8_INDEX(pSmi,VGA_SEQ_INDEX,VGA_SEQ_DATA,0x30));
                break;
        }
    }
    

	
	LEAVE();
}

