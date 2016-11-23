#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/errno.h>
#include<linux/string.h>
#include<linux/mm.h>
#include<linux/slab.h>
#include<linux/delay.h>
#include<linux/fb.h>
#include<linux/ioport.h>
#include<linux/init.h>
#include<linux/pci.h>
#include<linux/vmalloc.h>
#include<linux/pagemap.h>
#include<linux/screen_info.h>
#include <linux/console.h>
#include "../sm750drv.h"

#include "ddk750_common.h"

/*
 * Use panel vertical sync as time delay function.
 * Input: Number of vertical sync to wait.
 */
void panelWaitVerticalSync(unsigned long vsync_count)
{
    unsigned long status;
    
    /* Do not wait when the Panel PLL is off or display control is already off. 
       This will prevent the software to wait forever. */
    if ((FIELD_GET(peekRegisterDWord(PANEL_PLL_CTRL), PANEL_PLL_CTRL, POWER) ==
         PANEL_PLL_CTRL_POWER_OFF) ||
        (FIELD_GET(peekRegisterDWord(PANEL_DISPLAY_CTRL), PANEL_DISPLAY_CTRL, TIMING) ==
         PANEL_DISPLAY_CTRL_TIMING_DISABLE))
    {
        return;
    }

    while (vsync_count-- > 0)
    {
        /* Wait for end of vsync. */
        do
        {
            status = FIELD_GET(peekRegisterDWord(SYSTEM_CTRL),
                               SYSTEM_CTRL,
                               PANEL_VSYNC);
        }
        while (status == SYSTEM_CTRL_PANEL_VSYNC_ACTIVE);
		
        /* Wait for start of vsync. */
        do
        {
            status = FIELD_GET(peekRegisterDWord(SYSTEM_CTRL),
                               SYSTEM_CTRL, 
                               PANEL_VSYNC);
        }
        while (status == SYSTEM_CTRL_PANEL_VSYNC_INACTIVE);
    }
}

/*
 * This functions uses software sequence to turn on/off the panel.
 *
 */
void swPanelPowerSequence(disp_state_t dispState, unsigned long vsync_delay)
{
    unsigned long panelControl = peekRegisterDWord(PANEL_DISPLAY_CTRL);
    if (dispState == DISP_ON)
    {
        /* Turn on FPVDDEN. */
        panelControl = FIELD_SET(panelControl,
                     PANEL_DISPLAY_CTRL, FPVDDEN, HIGH);
        pokeRegisterDWord(PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVerticalSync(vsync_delay);

        /* Turn on FPDATA. */
        panelControl = FIELD_SET(panelControl, 
                     PANEL_DISPLAY_CTRL, DATA, ENABLE);
        pokeRegisterDWord(PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVerticalSync(vsync_delay);

        /* Turn on FPVBIAS. */
        panelControl = FIELD_SET(panelControl, 
                     PANEL_DISPLAY_CTRL, VBIASEN, HIGH);
        pokeRegisterDWord(PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVerticalSync(vsync_delay);

        /* Turn on FPEN. */
        panelControl = FIELD_SET(panelControl, 
                     PANEL_DISPLAY_CTRL, FPEN, HIGH);
        pokeRegisterDWord(PANEL_DISPLAY_CTRL, panelControl);
    }

    else
    {
        /* Turn off FPEN. */
        panelControl = FIELD_SET(panelControl,
                                 PANEL_DISPLAY_CTRL, FPEN, LOW);
        pokeRegisterDWord(PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVerticalSync(vsync_delay);

        /* Turn off FPVBIASEN. */
        panelControl = FIELD_SET(panelControl, 
                                 PANEL_DISPLAY_CTRL, VBIASEN, LOW);
        pokeRegisterDWord(PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVerticalSync(vsync_delay);

        /* Turn off FPDATA. */
        panelControl = FIELD_SET(panelControl, 
                                 PANEL_DISPLAY_CTRL, DATA, DISABLE);
        pokeRegisterDWord(PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVerticalSync(vsync_delay);

        /* Turn off FPVDDEN. */
        panelControl = FIELD_SET(panelControl, 
                                 PANEL_DISPLAY_CTRL, FPVDDEN, LOW);
        pokeRegisterDWord(PANEL_DISPLAY_CTRL, panelControl);
    }
}

/*
 * This function turns on/off the display control.
 * Currently, it for CRT and Panel controls only.
 * Input: Panel or CRT, or ...
 *        On or Off.
 *
 * This function manipulate the physical display channels 
 * and devices.
 *
 * Note:
 *      Turning on/off the timing and the plane requires programming sequence.
 *      The plane can not be changed without turning on the timing. However,
 *      changing the plane has no effect when the timing (clock) is off. Below,
 *      is the description of the timing and plane combination setting.
 *
 *      +-----------+-----------+-----------------------------------------------+
 *      |  Timing   |   Plane   |                    Description                |
 *      +-----------+-----------+-----------------------------------------------+
 *      |    ON     |    OFF    | no Display but clock is on (consume power)    |
 *      |    ON     |    ON     | normal display                                |
 *      |    OFF    |    OFF    | no display and no clock (power down)          |
 *      |    OFF    |    ON     | no display and no clock (same as power down)  |
 *      +-----------+-----------+-----------------------------------------------+
 */
void setDisplayControl(disp_control_t dispControl, disp_state_t dispState)
{
    unsigned long ulDisplayCtrlReg, ulReservedBits;

    if (dispControl == PANEL_CTRL)
    {
        ulDisplayCtrlReg = peekRegisterDWord(PANEL_DISPLAY_CTRL);

        //DDKDEBUGPRINT((DISPLAY_LEVEL, "(setDispCtrl) PANEL_DISPLAY_CTRL before set: %x\n", peekRegisterDWord(PANEL_DISPLAY_CTRL)));

        /* Turn on/off the Panel display control */
        if (dispState == DISP_ON)
        {
            /* 	Timing should be enabled first before enabling the panel because changing at the
               		same time does not guarantee that the plane will also enabled or disabled. 
             		*/
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, PANEL_DISPLAY_CTRL, TIMING, ENABLE);
            pokeRegisterDWord(PANEL_DISPLAY_CTRL, ulDisplayCtrlReg);
            
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, PANEL_DISPLAY_CTRL, PLANE, ENABLE);
			ulReservedBits = FIELD_SET(0, PANEL_DISPLAY_CTRL, RESERVED_1_MASK, ENABLE) |
                             FIELD_SET(0, PANEL_DISPLAY_CTRL, RESERVED_2_MASK, ENABLE) |
                             FIELD_SET(0, PANEL_DISPLAY_CTRL, RESERVED_3_MASK, ENABLE);
			
            do
            {
                pokeRegisterDWord(PANEL_DISPLAY_CTRL, ulDisplayCtrlReg);
            } while((peekRegisterDWord(PANEL_DISPLAY_CTRL) & ~ulReservedBits) != (ulDisplayCtrlReg & ~ulReservedBits));
        }
        else
        {
            /* When turning off, there is no rule on the programming sequence since whenever the
               clock is off, then it does not matter whether the plane is enabled or disabled.
               Note: Modifying the plane bit will take effect on the next vertical sync. Need to
                     find out if it is necessary to wait for 1 vsync before modifying the timing
                     enable bit. 
             */
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, PANEL_DISPLAY_CTRL, PLANE, DISABLE);
            pokeRegisterDWord(PANEL_DISPLAY_CTRL, ulDisplayCtrlReg);

            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, PANEL_DISPLAY_CTRL, TIMING, DISABLE);
        }

        pokeRegisterDWord(PANEL_DISPLAY_CTRL, ulDisplayCtrlReg);

        //DDKDEBUGPRINT((DISPLAY_LEVEL, "(setDispCtrl) PANEL_DISPLAY_CTRL after set: %x\n", peekRegisterDWord(PANEL_DISPLAY_CTRL)));
    }
    else if (dispControl == CRT_CTRL)   /* CRT */
    {
        ulDisplayCtrlReg = peekRegisterDWord(CRT_DISPLAY_CTRL);

        //DDKDEBUGPRINT((DISPLAY_LEVEL, "(setDispCtrl) CRT_DISPLAY_CTRL before set: %x\n", peekRegisterDWord(CRT_DISPLAY_CTRL)));

        if (dispState == DISP_ON)
        {
            /* Timing should be enabled first before enabling the panel because changing at the
               same time does not guarantee that the plane will also enabled or disabled. 
             */
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, CRT_DISPLAY_CTRL, TIMING, ENABLE);
            pokeRegisterDWord(CRT_DISPLAY_CTRL, ulDisplayCtrlReg);
            
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, CRT_DISPLAY_CTRL, PLANE, ENABLE);
        }
        else
        {
            /* When turning off, there is no rule on the programming sequence since whenever the
               clock is off, then it does not matter whether the plane is enabled or disabled.
               Note: Modifying the plane bit will take effect on the next vertical sync. Need to
                     find out if it is necessary to wait for 1 vsync before modifying the timing
                     enable bit.
             */
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, CRT_DISPLAY_CTRL, PLANE, DISABLE);
            pokeRegisterDWord(CRT_DISPLAY_CTRL, ulDisplayCtrlReg);

            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, CRT_DISPLAY_CTRL, TIMING, DISABLE);
        }
        pokeRegisterDWord(CRT_DISPLAY_CTRL, ulDisplayCtrlReg);

       //DDKDEBUGPRINT((DISPLAY_LEVEL, "(setDispCtrl) CRT_DISPLAY_CTRL after set: %x\n", peekRegisterDWord(CRT_DISPLAY_CTRL)));
    }
    else    /* VGA */
    {
        /* TODO: Enable the VGA timing. Currently it is not implemented. It might be
           implemented in the future */
    }
}

#if 0
/*
 * This function scale the display data.
 *
 * Input:
 *      enable      - Enable/disable scaling indicator
 *      dispControl - Display Control where the scaling will be performed
 *      srcWidth    - Source Width
 *      srcHeight   - Source Height
 *      dstWidth    - Destination Width
 *      dstHeight   - Destination Height
 */
void scaleDisplay(
    unsigned long enable,               /* Enable scaling. */
    disp_control_t dispControl,         /* Display Control */
    unsigned long srcWidth,             /* Source Width */
    unsigned long srcHeight,            /* Source Height */
    unsigned long dstWidth,             /* Destination Width */
    unsigned long dstHeight             /* Destination Height */
)
{
    unsigned long value = 0;
    unsigned long scaleFactor;
    
    DDKDEBUGPRINT((DISPLAY_LEVEL, "(scaleDisplay) srcWidth: %d, srcHeight: %d, ", srcWidth, srcHeight));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "dstWidth: %d, dstHeight: %d\n", dstWidth, dstHeight));
    
    /* Enable scaling. */
    if (enable == 1)
    {
        /* Scale the vertical size */
        scaleFactor = 0;
        if (dstHeight >= srcHeight)
            scaleFactor = srcHeight * SCALE_CONSTANT / dstHeight;
        value = FIELD_VALUE(value, CRT_SCALE , VERTICAL_SCALE, scaleFactor);
        
        /* Scale the horizontal size */
        scaleFactor = 0;
        if (dstWidth >= srcWidth)
            scaleFactor = srcWidth * SCALE_CONSTANT / dstWidth;
        value = FIELD_VALUE(value, CRT_SCALE , HORIZONTAL_SCALE, scaleFactor);
    
        pokeRegisterDWord(CRT_SCALE , value);
        
        DDKDEBUGPRINT((DISPLAY_LEVEL, "scaleFactor: %x\n", peekRegisterDWord(CRT_SCALE)));
        
        /* Enable Panel scaler path */
        if (dispControl != CRT_CTRL)
        {
            /* TODO: set up the Auto expansion registers. Is this necessary?
               Should it be done in the setmode function? */

            /* Enable Auto Expansion bit. */
            value = peekRegisterDWord(CRT_DISPLAY_CTRL);
            value = FIELD_SET(value, CRT_DISPLAY_CTRL, EXPANSION, ENABLE);
            value = FIELD_SET(value, CRT_DISPLAY_CTRL, LOCK_TIMING, ENABLE);
            pokeRegisterDWord(CRT_DISPLAY_CTRL, value);
        }
    }
    else
    {
        /* Set the scaler value to 0 */
        pokeRegisterDWord(CRT_SCALE , 0);
        
        DDKDEBUGPRINT((DISPLAY_LEVEL, "scaleFactor: %x\n", peekRegisterDWord(CRT_SCALE)));
        
        /* Program the scaler to use 1:1 scaler by setting the scaler register value to 0 */
        if (dispControl != CRT_CTRL)
        {
            value = peekRegisterDWord(CRT_DISPLAY_CTRL);
            value = FIELD_SET(value, CRT_DISPLAY_CTRL, EXPANSION, DISABLE);
            value = FIELD_SET(value, CRT_DISPLAY_CTRL, LOCK_TIMING, DISABLE);
            pokeRegisterDWord(CRT_DISPLAY_CTRL, value);
        }
    }
}
#endif
#if 0
/*
 * This function checks whether the scaling is necessary.
 *
 * Note: 
 *     This function is only applies to PANEL_PATH or Dual Display.
 */
unsigned char isScalingRequired(
    disp_control_t dispControl
)
{
    mode_parameter_t modeParam;
    unsigned long panelWidth, panelHeight;

    /* Get the current mode of the panel control */
    modeParam = getCurrentModeParam(dispControl);
    
    /* Scaling is not required in analog panel */
    if ((gPanelWidth != 0) && (gPanelHeight != 0))
    {
        /* Check whether the panel resolution is smaller than the panel size */
        if ((modeParam.horizontal_display_end < gPanelWidth) &&
            (modeParam.vertical_display_end < gPanelHeight))
            return 1;
    }
            
    return 0;        
}
#endif

/*
 * This function set the display path together with the HSync and VSync.
 *
 * Note:
 *     This function has to be called last after setting all the display Control
 *     and display output. 
 */
void setPath(
    disp_path_t dispPath, 
    disp_control_t dispControl, 
    disp_state_t dispState
)
{
    unsigned long control;
    mode_parameter_t modeParam;
	int t;
    
    /* Get the current mode parameter of the specific display control */
   // modeParam = getCurrentModeParam(dispControl);
    
    if (dispPath == PANEL_PATH)
    {
        control = peekRegisterDWord(PANEL_DISPLAY_CTRL);
        if (dispState == DISP_ON)
        {
#if 0        
            /* Adjust the Clock polarity */
            if (modeParam.clock_phase_polarity == POS)
                control = FIELD_SET(control, PANEL_DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_HIGH);
            else
                control = FIELD_SET(control, PANEL_DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_LOW);
                        
            /* Adjust the VSync polarity */
            if (modeParam.vertical_sync_polarity == POS)
                control = FIELD_SET(control, PANEL_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH);
            else
                control = FIELD_SET(control, PANEL_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW);

            /* Adjust the HSync polarity */
            if (modeParam.horizontal_sync_polarity == POS)
                control = FIELD_SET(control, PANEL_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH);
            else
                control = FIELD_SET(control, PANEL_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW);
#endif        
            if (isDisplayControlSwapped() == 0)
            {
                /* Display control is not swapped, so use the normal display data flow */
                if (dispControl == PANEL_CTRL)
                    control = FIELD_SET(control, PANEL_DISPLAY_CTRL, SELECT, PANEL);                    
                else    /* CRT Control */
                    control = FIELD_SET(control, PANEL_DISPLAY_CTRL, SELECT, CRT);
            }
            else
            {
                /* Display control is swapped, so used CRT display control for the
                   PANEL_CTRL and panel for the CRT_CTRL */
                if (dispControl == PANEL_CTRL)
                    control = FIELD_SET(control, PANEL_DISPLAY_CTRL, SELECT, CRT);                                        
                else
                    control = FIELD_SET(control, PANEL_DISPLAY_CTRL, SELECT, PANEL);
            }
        }    



        //inf_msg("monk:Read 0x80000 == %08x\n",peekRegisterDWord(0x80000));
        //inf_msg("monk:write 0x80000 with %08x\n",control);
        t = 0;
        while(peekRegisterDWord(PANEL_DISPLAY_CTRL) != control && t<99)
        {
            pokeRegisterDWord(PANEL_DISPLAY_CTRL, control);
            t++;
        }
        inf_msg("After tried %d times,Read 0x80000 == %08x\n",t,peekRegisterDWord(0x80000));
    }
    else    /* CRT Path */
    {
        control = peekRegisterDWord(CRT_DISPLAY_CTRL);
    
        if (dispState == DISP_ON)
        {
#if 0        
            /* Adjust the Clock polarity */
            if (modeParam.clock_phase_polarity == POS)
                control = FIELD_SET(control, CRT_DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_HIGH);
            else
                control = FIELD_SET(control, CRT_DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_LOW);
                
            /* Adjust the VSync polarity */
            if (modeParam.vertical_sync_polarity == POS)
                control = FIELD_SET(control, CRT_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH);
            else
                control = FIELD_SET(control, CRT_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW);
    
            /* Adjust the HSync polarity */
            if (modeParam.horizontal_sync_polarity == POS)
                control = FIELD_SET(control, CRT_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH);
            else
                control = FIELD_SET(control, CRT_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW);
#endif
            if (isDisplayControlSwapped() == 0)
            {
                /* Display control is not swapped, so use the normal display data flow */
                if (dispControl == PANEL_CTRL)
                    control = FIELD_SET(control, CRT_DISPLAY_CTRL, SELECT, PANEL);
                else
                    control = FIELD_SET(control, CRT_DISPLAY_CTRL, SELECT, CRT);
            }
            else
            {
                /* Display control is swapped, so used CRT display control for the
                   PANEL_CTRL and panel for the CRT_CTRL */
                if (dispControl == PANEL_CTRL)
                    control = FIELD_SET(control, CRT_DISPLAY_CTRL, SELECT, CRT);
                else
                    control = FIELD_SET(control, CRT_DISPLAY_CTRL, SELECT, PANEL);                
            }
            
            /* Enable the CRT Pixel */
            control = FIELD_SET(control, CRT_DISPLAY_CTRL, BLANK, OFF);
        }
        else
        {
            /* Disable the CRT Pixel */
            control = FIELD_SET(control, CRT_DISPLAY_CTRL, BLANK, ON);
        }            
        pokeRegisterDWord(CRT_DISPLAY_CTRL, control);
    }
}

/*
 * This function set the logical display output.
 *
 * The output is called logical because it is independent of physical implementation.
 * For example, CRT only mode is not using the internal secondary control. It uses the
 * Primary Control with its output directed to CRT DAC.
 *
 * Input:
 *      output      - Logical Display output
 *      swapDisplay - Swap the data display output (only applies to Dual display).
 *                    Otherwise, set it to 0
 *
 * Output:
 *      0   - Success
 *     -1   - Fail 
 */


long ddk750_setLogicalDispOutput(
    disp_output_t output,
    unsigned char swapDisplay
)
{
    unsigned long ulReg;

    switch (output)
    {
        case NO_DISPLAY:
        {        
            	/* 
            		In here, all the display device has to be turned off first before the
               		the display control. 
               	*/
            	setDisplayControl(PANEL_CTRL, DISP_OFF);    /* Turn off Panel control */
            	setDisplayControl(CRT_CTRL, DISP_OFF);      /* Turn off CRT control */
	        setDisplayControl(VGA_CTRL, DISP_OFF);      /* Turn off VGA control */
               
            	swPanelPowerSequence(DISP_OFF, 4);          /* Turn off Panel */
            	setDAC(DISP_OFF);                           /* Turn off DAC */
            	ddk750_setDPMS(DPMS_OFF);                          /* Turn off DPMS */            
            	setDualPanel(0);                            /* Turn off Panel 2 */    			
			
            	setPath(PANEL_PATH, PANEL_CTRL, DISP_OFF);  /* Turn off Panel path */
            	setPath(CRT_PATH, PANEL_CTRL, DISP_OFF);    /* Turn off CRT path */
            	break;
        }    
        case PANEL1_ONLY:
        {         
		/*PANEL1 only	(24 or 36 bit TFT) show primary controller data	*/
            	setDisplayControl(PANEL_CTRL, DISP_ON);     /* Turn on Panel control */            	
            	setDisplayControl(CRT_CTRL, DISP_OFF);  /* Turn off CRT control */ 
		setDisplayControl(VGA_CTRL, DISP_OFF);      /* Turn off VGA control */

		swPanelPowerSequence(DISP_ON, 4);           /* Turn on Panel */
           	setDAC(DISP_OFF);                           /* Turn off DAC */		
            	ddk750_setDPMS(DPMS_OFF);                          /* Turn off DPMS */            
            	setDualPanel(0);                            /* Turn off Panel 2 */            
            
            	setPath(PANEL_PATH, PANEL_CTRL, DISP_ON);   /* Turn on Panel Path and use panel data */
            	setPath(CRT_PATH, CRT_CTRL, DISP_ON);    /* Turn CRT path to secondary controller */            
            	break;
        }    
        case CRT1_CRT2_SIMUL:
        {            
		/* 	CRT1,2 show primary controller data	*/
            	setDisplayControl(PANEL_CTRL, DISP_ON);     /* Turn on Panel Control */
            	setDisplayControl(CRT_CTRL, DISP_OFF);      /* Turn off CRT control */
		setDisplayControl(VGA_CTRL, DISP_OFF);      /* Turn off VGA control */            

			
	    	swPanelPowerSequence(DISP_OFF, 4);          /* Turn off Panel */
		/* set bit 25 of 0x80000 to 1 can driver DVI-CRT with out turn on PANEL	*/
		pokeRegisterDWord(PANEL_DISPLAY_CTRL,FIELD_SET(peekRegisterDWord(PANEL_DISPLAY_CTRL),
							PANEL_DISPLAY_CTRL,DATA,ENABLE));
		
            	setDAC(DISP_ON);                            /* Turn on DAC */
            	ddk750_setDPMS(DPMS_ON);                           /* Turn on DPMS to drive CRT */				
            	setDualPanel(0);                            /* Turn off Panel 2 */           
				
            	setPath(PANEL_PATH, PANEL_CTRL, DISP_ON);  /* Turn off Panel Path */
            	setPath(CRT_PATH, PANEL_CTRL, DISP_ON);     /* Turn off CRT Path and use panel data */
            	break;
        }
        case PANEL_CRT_SIMUL:
        {            
		/* 
			Panel1 (18,24,36 bit TFT) and CRT1,2 show same content with primary controller
			panel type is none of my bussiness,pleae set it out side
			if 18-bit TFT selected then this option means PANEL1 + PANEL2 + CRTs
		*/
            	setDisplayControl(PANEL_CTRL, DISP_ON);     /* Turn on Panel Control */
            	setDisplayControl(CRT_CTRL, DISP_OFF);  /* Turn off CRT control */
            	setDisplayControl(VGA_CTRL, DISP_OFF);      /* Turn off VGA control */
		
            	swPanelPowerSequence(DISP_ON, 4);           /* Turn on Panel */		
            	setDAC(DISP_ON);                            /* Turn on DAC */
            	ddk750_setDPMS(DPMS_ON);                           /* Turn on DPMS to drive CRT */            
            
            	setPath(PANEL_PATH, PANEL_CTRL, DISP_ON);   /* Turn on Panel Path and use panel data */
            	setPath(CRT_PATH, PANEL_CTRL, DISP_ON);     /* Turn on CRT Path and use panel data */            
            	break;
        }    
	case PANEL_CRT_SIMUL_SEC:
	{
		/*	like above but show secondary controller data	*/
            	setDisplayControl(PANEL_CTRL, DISP_OFF);     /* Turn OFF Panel Control */
            	setDisplayControl(CRT_CTRL, DISP_ON);  /* Turn ON CRT control */
            	setDisplayControl(VGA_CTRL, DISP_OFF);      /* Turn off VGA control */
		
            	swPanelPowerSequence(DISP_ON, 4);           /* Turn on Panel */			
            	setDAC(DISP_ON);                            /* Turn on DAC */
            	ddk750_setDPMS(DPMS_ON);                           /* Turn on DPMS to drive CRT */            
            
            	setPath(PANEL_PATH, CRT_CTRL, DISP_ON);   /* Turn ON Panel Path and use secondary ctrl 	*/
            	setPath(CRT_PATH, CRT_CTRL, DISP_ON);     /* Turn on CRT Path and use secondary ctrl	 */            
            	break;		
	}
	case PANEL1_PANEL2_SIMUL:
	{
		/*	dual 18 bit TFT show primary controller data	*/
            	setDisplayControl(PANEL_CTRL, DISP_ON);     /* Turn ON Panel Control */
            	setDisplayControl(CRT_CTRL, DISP_OFF);  /* Turn OFF CRT control */
            	setDisplayControl(VGA_CTRL, DISP_OFF);      /* Turn off VGA control */
		
            	swPanelPowerSequence(DISP_ON, 4);           /* Turn on Panel */			
            	setDAC(DISP_OFF);                            /* Turn on DAC */
            	ddk750_setDPMS(DPMS_OFF);                           /* Turn on DPMS to drive CRT */          
		setPanelType(TFT_18BIT);
            
            	setPath(PANEL_PATH, PANEL_CTRL, DISP_ON);   /* Turn ON Panel Path and use secondary ctrl 	*/
            	setPath(CRT_PATH, PANEL_CTRL, DISP_ON);     /* Turn on CRT Path and use secondary ctrl	 */            
            	break;
	}
        case PANEL_CRT_DUAL: 
        {
		/*
			view1:PANEL1 + DVI-CRT 
			view2:VGA-CRT 
		*/
            	setDisplayControl(PANEL_CTRL, DISP_ON);     /* Turn on Panel Control */
            	setDisplayControl(CRT_CTRL, DISP_ON);       /* Turn on CRT control */
            	setDisplayControl(VGA_CTRL, DISP_OFF);      /* Turn off VGA control */
		
            	swPanelPowerSequence(DISP_ON, 4);           /* Turn on Panel */
            	setDAC(DISP_ON);                            		/* Turn on DAC */
            	ddk750_setDPMS(DPMS_ON);                       /* Turn on DPMS to drive CRT */            
            	setDualPanel(0);                            			/* Turn off Panel 2 */
			
            	if (swapDisplay == 1)
            	{                        		
                	setPath(PANEL_PATH, CRT_CTRL, DISP_ON);     /* Turn on Panel Path and use CRT data */
                	setPath(CRT_PATH, PANEL_CTRL, DISP_ON);     /* Turn on CRT Path and use Panel data */              
            	}
            	else
            	{
                	setPath(PANEL_PATH, PANEL_CTRL, DISP_ON);   /* Turn on Panel Path and use panel data */
                	setPath(CRT_PATH, CRT_CTRL, DISP_ON);       /* Turn on CRT Path and use CRT data */
            	}            
            	break;
        }    
    }
    return 0;
}


/*
 * This function enables/disables the dual panel
 *
 * Input: 
 *     enable  - Flag to enable/disable the dual Panel mode
 */
void setDualPanel(
    unsigned long enable
)
{
    unsigned long value;

    /* Enable/Disable the Dual Display */
    value = peekRegisterDWord(PANEL_DISPLAY_CTRL);
    if (enable == 1)
        value = FIELD_SET(value, PANEL_DISPLAY_CTRL, DUAL_DISPLAY, ENABLE);
    else
        value = FIELD_SET(value, PANEL_DISPLAY_CTRL, DUAL_DISPLAY, DISABLE);
    pokeRegisterDWord(PANEL_DISPLAY_CTRL, value);
    
    /* Force to 18-bit Panel. Do this after enable Dual Panel function above
       since setPanelType(TFT_18BIT) will check if the Dual Panel is enable or
       not before it set the Panel to 18-bit. */
    if (enable == 1)
        setPanelType(TFT_18BIT);        
}


/*
 * This function sets the panel type
 *
 * Input:
 *      panelType   - The type of the panel to be set  
 */
long setPanelType(
    panel_type_t panelType
)
{
    unsigned long value;
	
#if 0    
	/* monk note below*/
    /* Setting to TFT_18BIT will force the display to Dual Panel Display. */    
    if ((panelType == TFT_18BIT) && (isDualPanelEnable() == 0))
        setDualPanel(1);
    else
        setDualPanel(0);
#endif

    /* Set the panel type. */
    value = peekRegisterDWord(PANEL_DISPLAY_CTRL);
    switch (panelType)
    {
        case TFT_18BIT:
            /* TFT 18-bit is the same as TFT 24-bit with the exception that this
               setting is only used in Dual Panel. Note: there is no break here. */
             	value = FIELD_SET(value,PANEL_DISPLAY_CTRL,DUAL_DISPLAY,ENABLE);
        case TFT_24BIT:
            value = FIELD_SET(value, PANEL_DISPLAY_CTRL, DOUBLE_PIXEL, DISABLE);
            break;
        case TFT_36BIT:
            value = FIELD_SET(value, PANEL_DISPLAY_CTRL, DOUBLE_PIXEL, ENABLE);
            break;
        default:
            return (-1);
    }
    
    pokeRegisterDWord(PANEL_DISPLAY_CTRL, value);
    
    return 0;
}

/*
 * This function checks whether dual panel is enabled or not
 *
 * Output:
 *      0   - Not Enable
 *      1   - Enable  
 */
unsigned char isDualPanelEnable(void)
{
    unsigned long value;
    value = FIELD_GET(peekRegisterDWord(PANEL_DISPLAY_CTRL), PANEL_DISPLAY_CTRL, DUAL_DISPLAY);    
    return ((value == PANEL_DISPLAY_CTRL_DUAL_DISPLAY_ENABLE) ? 1 : 0);
}



/*
 *  displaySetInterpolation
 *      This function enables/disables the horizontal and vertical interpolation
 *      for the secondary display control. Primary display control does not have
 *      this capability.
 *
 *  Input:
 *      enableHorzInterpolation - Flag to enable/disable Horizontal interpolation
 *      enableVertInterpolation - Flag to enable/disable Vertical interpolation
 */
void displaySetInterpolation(
    unsigned long enableHorzInterpolation,
    unsigned long enableVertInterpolation
)
{
    unsigned long value;

    value = peekRegisterDWord(CRT_DISPLAY_CTRL);
    
    if (enableHorzInterpolation)
        value = FIELD_SET(value, CRT_DISPLAY_CTRL, HORIZONTAL_MODE, INTERPOLATE);
    else
        value = FIELD_SET(value, CRT_DISPLAY_CTRL, HORIZONTAL_MODE, REPLICATE);
        
    if (enableVertInterpolation)
        value = FIELD_SET(value, CRT_DISPLAY_CTRL, VERTICAL_MODE, INTERPOLATE);
    else
        value = FIELD_SET(value, CRT_DISPLAY_CTRL, VERTICAL_MODE, REPLICATE);
        
    pokeRegisterDWord(CRT_DISPLAY_CTRL, value);
}


#ifdef USE_DVICHIP
/*
 * This function initializes the display.
 *
 * Output:
 *      0   - Success
 *      1   - Fail 
 */
long ddk750_initDisplay()
{
    /* Initialize DVI. If the dviInit fail and the VendorID or the DeviceID are
       not zeroed, then set the failure flag. If it is zeroe, it might mean 
       that the system is in Dual CRT Monitor configuration. */
       
    /* De-skew enabled with default 111b value. 
       This will fix some artifacts problem in some mode on board 2.2.
       Somehow this fix does not affect board 2.1.
     */
    if ((dviInit(1,  /* Select Rising Edge */ 
                1,  /* Select 24-bit bus */
                0,  /* Select Single Edge clock */
                1,  /* Enable HSync as is */
                1,  /* Enable VSync as is */
                1,  /* Enable De-skew */
                7,  /* Set the de-skew setting to maximum setup */
                1,  /* Enable continuous Sync */
                1,  /* Enable PLL Filter */
                4   /* Use the recommended value for PLL Filter value */
        ) != 0) && (dviGetVendorID() != 0x0000) && (dviGetDeviceID() != 0x0000))
    {
        return (-1);
    }
    
    /* TODO: Initialize other display component */
    
    /* Success */
    return 0;
}
#endif


