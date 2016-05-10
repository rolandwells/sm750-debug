/*
Copyright (C) 1994-1999 The XFree86 Project, Inc.  All Rights Reserved.
Copyright (C) 2000 Silicon Motion, Inc.  All Rights Reserved.
Copyright (C) 2008 Mandriva Linux.  All Rights Reserved.
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
/*	put smi750.h after 750ddk because of dependent problem	*/
#include "smi750.h"
#include "hwi2c.h"
#include "swi2c.h"


#if SMI_DEBUG
static char * dpms_str[]={"DPMSModeOn","DPMSModeStandby","DPMSModeSuspend","DPMSModeOff"};
static char * head_str[]={"TFT1","DVI","VGA","TFT2"};
static char * tft_str[] ={"18bit","24bit","36bit"};
#endif

static void SMI750_OutputDPMS_PNL(xf86OutputPtr,int);
static void SMI750_OutputDPMS_CRT(xf86OutputPtr,int);
static xf86OutputStatus SMI750_OutputDetect_PNL(xf86OutputPtr);
static xf86OutputStatus SMI750_OutputDetect_CRT(xf86OutputPtr);
static int SMI750_OutputModeValid_PNL(xf86OutputPtr output, DisplayModePtr mode);
static int SMI750_OutputModeValid_CRT(xf86OutputPtr output, DisplayModePtr mode);




char* outputName[]={"PNL","CRT"}; 

static xf86OutputStatus (*pfn_outputDetect[])(xf86OutputPtr)=
{SMI750_OutputDetect_PNL,SMI750_OutputDetect_CRT};

static void (*pfn_outputDPMS[])(xf86OutputPtr,int)=
{SMI750_OutputDPMS_PNL,SMI750_OutputDPMS_CRT};

/*
static void (*pfn_outputGetModes[])(xf86OutputPtr) = 
{SMI750_OutputGetModes_PNL,SMI750_OutputGetModes_CRT};
*/

static int (*pfn_outputValid[])(xf86OutputPtr,DisplayModePtr)=
{SMI750_OutputModeValid_PNL,SMI750_OutputModeValid_CRT};

/*   return 0 if the mode can be found in DDK  */
static int ModeFound(xf86OutputPtr output,DisplayModePtr mode)
{
    uint hz;
    ScrnInfoPtr	pScrn = output->scrn;
    ENTER();
    if(mode->HSync > 71.0f && mode->HSync < 77.1f)
        hz = 75;
    else
        hz = 60;        

    XMSG("mode->X,Y,H = %d,%d,%d\n",mode->HDisplay,mode->VDisplay,hz);
    if((mode_parameter_t *)findVesaModeParam(mode->HDisplay, mode->VDisplay, hz)!=NULL)
        LEAVE(0);
    XMSG("mode not supported by DDK\n");        
    LEAVE(-1);
}

static void
SMI750_OutputDPMS_PNL(xf86OutputPtr output, int dpms)
{
    ScrnInfoPtr	pScrn = output->scrn;
    SMIPtr		pSmi = SMIPTR(pScrn); 
	int idx;	
    ENTER();

    switch (dpms) 
	{
    case DPMSModeOn:		
		idx = 0;
		swPanelPowerSequence(DISP_ON,4);
		break;
    case DPMSModeStandby:		
		idx=1;
		break;
    case DPMSModeSuspend:
		idx=2;
		break;
    case DPMSModeOff:
		idx=3;		
		swPanelPowerSequence(DISP_OFF,4);	
		break;
    }
	
    DEBUG("Set PNL DPMS ==> %s \n",dpms_str[idx]);
    LEAVE();
}

static void
SMI750_OutputDPMS_CRT(xf86OutputPtr output,int dpms)
{
    ScrnInfoPtr	pScrn = output->scrn;
    SMIPtr		pSmi = SMIPTR(pScrn);
	int idx;	
    ENTER();

    switch (dpms) 
	{
    case DPMSModeOn:		
		idx = 0;
		setDPMS(DPMS_ON);
		break;
    case DPMSModeStandby:
		idx = 1;
		setDPMS(DPMS_STANDBY);
		break;
    case DPMSModeSuspend:
		idx = 2;
		setDPMS(DPMS_SUSPEND);
		break;
    case DPMSModeOff:
		idx = 3;
		setDPMS(DPMS_OFF);		
		break;
    }
    
	DEBUG("Set CRT DPMS ==> %s \n",dpms_str[idx]);
	LEAVE();	
}

#if 1
DisplayModePtr SMI750_OutputGetModes(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    xf86MonPtr pMon = NULL;
	SMIOutputPrivatePtr priv = (SMIOutputPrivatePtr)output->driver_private;
	I2CBusPtr bus;
	char * EdidBuffer;

    ENTER();

	EdidBuffer = xalloc(128);

	if(!EdidBuffer){
		XERR("not enough memory for allocating EDID buffer\n");
		LEAVE(NULL);
	}

	XINF("i2c bus index == %d\n",priv->path);

	bus = (priv->path == PANEL_PATH ? pSmi->I2C_primary:pSmi->I2C_secondary);

	/* 	some thing weird: some times Xserver method to access EDID is okay while some times not
		but use hardware i2c to access DVI EDID should be always okay
	*/
	
    if(xf86LoaderCheckSymbol("xf86PrintEDID"))
	{		
		if(bus)
		{
			pMon = xf86OutputGetEDID(output,bus);
			if(pMon)
			{
				xf86PrintEDID(pMon);
				xf86OutputSetEDID(output,pMon);
				xfree(EdidBuffer);
				LEAVE(xf86OutputGetEDIDModes(output));
			}
#if SMI_DEBUG == 1			
			else{
				XERR("Seems monitor not found\n");	
			}
#endif			
		}
#if SMI_DEBUG == 1		
		else{
			XERR("Seems bus not found\n");
		}
#endif		
	}
#if SMI_DEBUG == 1	
	else{
		XERR("should me be seen ?\n");
	}
#endif	

	XINF("try DDK method\n");
	long returnValue;
	if(priv->path == PANEL_PATH)
	{	
			
		returnValue = edidReadMonitorEx(PANEL_PATH,EdidBuffer, 128, 
										0, DEFAULT_I2C_SCL, DEFAULT_I2C_SDA);
		if (returnValue == 0)
		{
			pMon = xf86InterpretEDID(pScrn->scrnIndex,EdidBuffer);
			if(pMon){
				xf86OutputSetEDID(output,pMon);
				XINF("Found monitor by DDK\n");
				xfree(EdidBuffer);
				LEAVE(xf86OutputGetEDIDModes(output));
			}else{
				XERR("DDK cannot get monitor\n");
			}
		}
		else
		{
			XERR("DDK cannot detect EDID Version\n");
		}
	}
	else
	{	
		/*
			because 750ddk.so is loaded by siliconmotion_drv.so
			so if one function of 750ddk.so is referenced as function pointer
			it will be failed .
			but calling the function is okay.
			I suggest combine 750ddk.so into siliconmotion_drv.so 
			which is a must for function pointer passing method

			below code pass xorg_I2CUDelay address into edidRead
			but X will exit with error when it be executed
			while just calling xorg_I2CUDelay is okay
				monk  @  10/20/2010
		*/
		returnValue = edidReadMonitorEx_software(CRT_PATH,EdidBuffer,128, 
										0, 17, 18,										
										//xorg_I2CUDelay
										NULL
										);
		
		
		if (returnValue == 0)
		{
			pMon = xf86InterpretEDID(pScrn->scrnIndex, EdidBuffer);
			if(pMon){
				xf86OutputSetEDID(output,pMon);
				XINF("Found monitor by DDK\n");
				xfree(EdidBuffer);
				LEAVE(xf86OutputGetEDIDModes(output));
			}else{
				XERR("DDK cannot get monitor\n");
			}
		}
	}

	XMSG("DDK seems no mode found\n");
	
	xfree(EdidBuffer);
	LEAVE(NULL);	
}

#else

DisplayModePtr
SMI750_OutputGetModes(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    xf86MonPtr pMon = NULL;
    unsigned char lcdEdidBuffer[128];
	SMIOutputPrivatePtr priv = (SMIOutputPrivatePtr)output->driver_private;

    ENTER();
	LEAVE(NULL);

    if(xf86LoaderCheckSymbol("xf86PrintEDID"))
	{ /* Ensure the DDC module is loaded*/
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
		
	
		/* Try DDC1 *///read edid by ddk
		long returnValue;
		XINF("priv->path = %d\n",priv->path);

		returnValue = edidReadMonitorEx(PANEL_PATH,lcdEdidBuffer, sizeof(lcdEdidBuffer), 
										0, DEFAULT_I2C_SCL, DEFAULT_I2C_SDA);
		if (returnValue == 0)
		{
			pMon = xf86InterpretEDID(pScrn->scrnIndex, lcdEdidBuffer);
			if(pMon){
			    xf86OutputSetEDID(output,pMon);
			    LEAVE(xf86OutputGetEDIDModes(output));
			}
		}
		else
		{
			      xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			 "this driver cannot detect EDID Version\n");
		}
    }

    LEAVE(NULL);
}

#endif

xf86OutputStatus
SMI750_OutputDetect_PNL(xf86OutputPtr output)
{
    ENTER();

    //LEAVE(XF86OutputStatusDisconnected);
    LEAVE(XF86OutputStatusConnected);
    
}

xf86OutputStatus
SMI750_OutputDetect_CRT(xf86OutputPtr output)
{
    ENTER();
	
	//LEAVE(XF86OutputStatusDisconnected);
	LEAVE(XF86OutputStatusConnected);
}

static int
SMI750_OutputModeValid_PNL(xf86OutputPtr output, DisplayModePtr mode)
{
    ScrnInfoPtr pScrn = output->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);	
	int xlcd,ylcd;
	
    ENTER();

	LEAVE(MODE_OK);

	xlcd = pSmi->entityPrivate->xlcd;
	ylcd = pSmi->entityPrivate->ylcd;

	XMSG("mode:%dx%d@%d\n",mode->HDisplay,mode->VDisplay,mode->VRefresh);

    /* FIXME May also need to test for IS_MSOC(pSmi) here.
     * Only accept modes matching the panel size because the panel cannot
     * be centered neither shrinked/expanded due to hardware bugs.
     * Note that as int32_t as plane tr/br and plane window x/y are set to 0
     * and the mode height matches the panel height, it will work and
     * set the mode, but at offset 0, and properly program the crt.
     * But use panel dimensions so that "full screen" programs will do
     * their own centering. */
   // if (output->name && strcmp(output->name, "LVDS") == 0 &&
   //	(mode->HDisplay != pSmi->lcdWidth || mode->VDisplay != pSmi->lcdHeight))
  //	LEAVE(MODE_PANEL);

    /* The driver is actually programming modes, instead of loading registers
     * state from static tables. But still, only accept modes that should
     * be handled correctly by all hardwares. On the MSOC, currently, only
     * the crt can be programmed to different resolution modes.
     */
    //if (mode->HDisplay & 15)
	//LEAVE(MODE_BAD_WIDTH);

    if((mode->Clock < pSmi->clockRange.minClock) ||
       (mode->Clock > pSmi->clockRange.maxClock) ||
       ((mode->Flags & V_INTERLACE) && !pSmi->clockRange.interlaceAllowed) ||
       ((mode->Flags & V_DBLSCAN) && (mode->VScan > 1) && !pSmi->clockRange.doubleScanAllowed)){
		LEAVE(MODE_CLOCK_RANGE);
    }    
	else if(xlcd > 0 && ylcd > 0 && 
			(mode->HDisplay > xlcd || mode->VDisplay > ylcd))
	{	
		LEAVE(MODE_CLOCK_RANGE);
	}
/*	
	else if(ModeFound(output,mode))
	{
	    LEAVE(MODE_CLOCK_RANGE);
	}
*/
    LEAVE(MODE_OK);
}



static int
SMI750_OutputModeValid_CRT(xf86OutputPtr output, DisplayModePtr mode)
{
    ScrnInfoPtr pScrn = output->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();
    XMSG("mode:%dx%d@%d\n",mode->HDisplay,mode->VDisplay,mode->VRefresh);
	
	XMSG("mode->Clock = %d,pSmi->clockRange.minClock = %d,pSmi->clockRange.maxClock = %d\n",
		mode->Clock,pSmi->clockRange.minClock,pSmi->clockRange.maxClock);

    /* FIXME May also need to test for IS_MSOC(pSmi) here.
     * Only accept modes matching the panel size because the panel cannot
     * be centered neither shrinked/expanded due to hardware bugs.
     * Note that as int32_t as plane tr/br and plane window x/y are set to 0
     * and the mode height matches the panel height, it will work and
     * set the mode, but at offset 0, and properly program the crt.
     * But use panel dimensions so that "full screen" programs will do
     * their own centering. */
   // if (output->name && strcmp(output->name, "LVDS") == 0 &&
   //	(mode->HDisplay != pSmi->lcdWidth || mode->VDisplay != pSmi->lcdHeight))
  //	LEAVE(MODE_PANEL);

    /* The driver is actually programming modes, instead of loading registers
     * state from static tables. But still, only accept modes that should
     * be handled correctly by all hardwares. On the MSOC, currently, only
     * the crt can be programmed to different resolution modes.
     */
    //if (mode->HDisplay & 15)
	//LEAVE(MODE_BAD_WIDTH);

    if((mode->Clock < pSmi->clockRange.minClock) ||
       (mode->Clock > pSmi->clockRange.maxClock) ||
       ((mode->Flags & V_INTERLACE) && !pSmi->clockRange.interlaceAllowed) ||
       ((mode->Flags & V_DBLSCAN) && (mode->VScan > 1) && !pSmi->clockRange.doubleScanAllowed))
	{	

		LEAVE(MODE_CLOCK_RANGE);
	}	
/*    
	else if(ModeFound(output,mode))
	{
	    LEAVE(MODE_CLOCK_RANGE);
	}
*/	
    LEAVE(MODE_OK);
}


Bool
SMI750_OutputPreInit(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi;
    xf86OutputPtr		output;
	SMIOutputPrivatePtr outputPriv;
    xf86OutputFuncsPtr	outputFuncs;
    SMIEntPtr pEntPriv;
	
    ENTER();
	
	pSmi = SMIPTR(pScrn);
	pEntPriv = pSmi->entityPrivate;	
	int c = (pSmi->DualView?2:1);	
	int idx = 0;
	Bool expansion = (pEntPriv->xlcd>0 && pEntPriv->ylcd>0);
	
	while(idx < c)
	{	
		if(!SMI_OutputFuncsInit_base(&outputFuncs))
			LEAVE(FALSE);	
		
	    outputFuncs->dpms		= pfn_outputDPMS[idx];
	    outputFuncs->get_modes = SMI750_OutputGetModes;
	    outputFuncs->detect		= pfn_outputDetect[idx];
	    outputFuncs->mode_valid = pfn_outputValid[idx];

		outputPriv = xnfcalloc(sizeof(SMIOutputPrivateRec),1);	
		outputPriv->index = idx;
		outputPriv->path = idx;
		
		if(idx == 0)
		{   /* for output 0: it's always be Panel path */
			if(pEntPriv->TFT != -1)
				outputPriv->head = HEAD_TFT0;
			else
				outputPriv->head = HEAD_DVI;
		}
		else
		{   /* for output 1: it's always be Crt path */
			if(pEntPriv->TFT == TFT_18BIT)
				outputPriv->head = HEAD_TFT1;
			else
				outputPriv->head = HEAD_VGA;
		}

	    if (! (output = xf86OutputCreate(pScrn, outputFuncs, outputName[idx])))
			LEAVE(FALSE);

		output->driver_private = outputPriv;		
        output->possible_crtcs = pSmi->DualView?3:1;        
        output->possible_clones = 3;
        if(outputPriv->path == 0 && expansion)
        {
            /*  only link to crtc_1 [secondary channel]
                           but remember that if no expansion ,crtc_0 will be by default Primary channel
                     */
            output->possible_crtcs = 1;//note: Secondary controller will be ctrc 0 if expansion enabled  
            output->possible_clones = 0;
        }

        if(outputPriv->path == 1 && expansion)
            output->possible_clones = 1;      
	  
	    output->interlaceAllowed = FALSE;
	    output->doubleScanAllowed = FALSE;
		XINF("output[%d]->possible_crtcs = %d \n",idx,output->possible_crtcs);
			
		idx++;
	}
	
    LEAVE(TRUE);
}


Bool SMI750_LinkPathCrtcc(ScrnInfoPtr pScrn,disp_path_t path,disp_control_t control)
{
	/*	just select the contoller for the path and do nothing else	*/
	uint32_t ulreg;
	ENTER();

	DEBUG("%s path <== %s controller \n",
				path==PANEL_PATH?"Panel":"Crt",
				control==PRIMARY_CTRL?"Primary":"Secondary");

	
	if(path == PANEL_PATH)
	{
		int times = 30;
		ulreg = PEEK32(PRIMARY_DISPLAY_CTRL);
		/* monk: a bug for 718,bit 29:28 can be se correctly by only one time 
				need a while loop to handle this hardware limitation
				seems for 718,only the vertical timing finish sync 
				is a right period for writing bit 29:28
		*/
		while(times --)
		{
			ulreg = FIELD_VALUE(ulreg,PRIMARY_DISPLAY_CTRL,SELECT,control == PRIMARY_CTRL?0:2);
			POKE32(PRIMARY_DISPLAY_CTRL,ulreg);
			if(PEEK32(PRIMARY_DISPLAY_CTRL) == ulreg)
				break;
		}
		
		if(times < 0){
			XERR("Time out when setting select of RPIMARY_DISPLAY_CTRL\n");
			LEAVE(FALSE);
		}
		
	}
	else
	{
		ulreg = PEEK32(SECONDARY_DISPLAY_CTRL);
		ulreg = FIELD_VALUE(ulreg,SECONDARY_DISPLAY_CTRL,SELECT,control == PRIMARY_CTRL?0:2);	
		POKE32(SECONDARY_DISPLAY_CTRL,ulreg);
	}
	LEAVE(TRUE);
}

Bool SMI750_SetHead(ScrnInfoPtr pScrn,output_head_t head,disp_state_t state)
{
	/*
		Do minimal fundamental steps to reach the order!
		Possibaly affect other heads  !!
	*/
	ENTER();	
	
	DEBUG("%s head : %s\n",head_str[head],state == DISP_ON?"on":"off");
	
	switch(head)
	{
		case HEAD_TFT0:
			swPanelPowerSequence(state,4);
			break;
		case HEAD_DVI:
			if(state == DISP_ON){
				swPanelPowerSequence(DISP_ON,4);
				setDAC(DISP_ON);
			}else{
				setDAC(DISP_OFF);
			}			
			break;
		case HEAD_VGA:
			if(state == DISP_ON){
				setDAC(DISP_ON);
				setDPMS(DPMS_ON);
			}else{
				setDPMS(DPMS_OFF);
			}				
			break;
		default:
		case HEAD_TFT1:
			/*	nothing to turn on trun off for TFT2 head	*/
			break;		
	}
	LEAVE(TRUE);
}


void SMI750_OutputModeSet(xf86OutputPtr output, DisplayModePtr mode, DisplayModePtr adjusted_mode)
{
	ScrnInfoPtr pScrn;
	xf86CrtcPtr crtc;	
	SMIPtr pSmi;
    SMIEntPtr pEntPriv;
	disp_control_t channel;
	disp_path_t path;
	output_head_t head;
	int okay;
	
    ENTER();
	pScrn = output->scrn;
	pSmi = SMIPTR(pScrn);
	pEntPriv = pSmi->entityPrivate;
	SMIOutputPrivatePtr priv = (SMIOutputPrivatePtr)output->driver_private;
	
	RROutputPtr rrop = output->randr_output;

	if((crtc = output->crtc) != NULL)
	{	
		channel = ((SMICrtcPrivatePtr)crtc->driver_private)->controller;
		path = priv->path;
		head = priv->head;
		
		if(pSmi->DualView == FALSE){
        	SMI750_LinkPathCrtcc(pScrn,PANEL_PATH ,channel);	
		    SMI750_LinkPathCrtcc(pScrn,CRT_PATH ,channel);
		    SMI750_SetHead(pScrn,HEAD_DVI,DISP_ON);
		    SMI750_SetHead(pScrn,HEAD_VGA,DISP_ON);			
		}else{			
    		SMI750_LinkPathCrtcc(pScrn,path,channel);
    		SMI750_SetHead(pScrn,head,DISP_ON);
    		if(path == PANEL_PATH && head!= HEAD_DVI)
    			setPanelType(pEntPriv->TFT);
        }
	}
    else
	{	
		XERR("Output->crtct == NULL !!\n");
	}	
    LEAVE();
}


