/*******************************************************************
* 
*         Copyright (c) 2009 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  dvi.c --- SM750 DDK 
*  This file contains the API function for DVI controller chip.
* 
*******************************************************************/
#include <stdlib.h>
#include "defs.h"
#include "dvi.h"
#include "hardware.h"
#include "sii164.h"

#include "ddkdebug.h"

/* This global variable contains all the supported driver and its corresponding 
   function API. Please set the function pointer to NULL whenever the function
   is not supported. */
static dvi_ctrl_device_t g_dcftSupportedDviController[] =
{
#ifdef DVI_CTRL_SII164
    {
        sii164InitChip,
        sii164ResetChip,
        sii164GetChipString,
        sii164GetVendorID,
        sii164GetDeviceID,
        sii164SetPower,
        sii164EnableHotPlugDetection,
        sii164IsConnected,
        sii164CheckInterrupt,
        sii164ClearInterrupt,
    },
#endif
};

/* Name of the DVI Controller chip */
static char *gDviCtrlChipName = "Unknown DVI Controller Chip";

/* Initialize the configuration to (-1), which means that it has not been initialized yet */
static unsigned char g_ucDeviceConfiguration[MAX_SMI_DEVICE] =
{
    (unsigned char)(-1), (unsigned char)(-1), (unsigned char)(-1), (unsigned char)(-1)
};

/*
 *  long getCurrentDviCtrl();
 *      This function gets the Dvi Controller of the current SM750 device.
 *
 *  Output:
 *      Null pointer if fail
 *      Pointer to the set of the supported DVI Controller functions.
 */
dvi_ctrl_device_t *getDviCtrl()
{
    unsigned char deviceConfig;
    
    deviceConfig = g_ucDeviceConfiguration[getCurrentDevice()];
    if (deviceConfig != (unsigned char)(-1))
        return ((dvi_ctrl_device_t *)&g_dcftSupportedDviController[deviceConfig]);
    
    return ((dvi_ctrl_device_t *)0);
}

/*
 *  dviInit
 *      This function initialize and detect the DVI controller chip.
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail.
 */
long dviInit(
    unsigned char edgeSelect,
    unsigned char busSelect,
    unsigned char dualEdgeClkSelect,
    unsigned char hsyncEnable,
    unsigned char vsyncEnable,
    unsigned char deskewEnable,
    unsigned char deskewSetting,
    unsigned char continuousSyncEnable,
    unsigned char pllFilterEnable,
    unsigned char pllFilterValue
)
{
    unsigned char dviCtrlIndex;
    unsigned short deviceIndex;
    dvi_ctrl_device_t *pCurrentDviCtrl;
    
    for (dviCtrlIndex = 0; 
         dviCtrlIndex < (sizeof(g_dcftSupportedDviController) / sizeof(dvi_ctrl_device_t)); 
         dviCtrlIndex++) 
    {
        pCurrentDviCtrl = (dvi_ctrl_device_t *)&g_dcftSupportedDviController[dviCtrlIndex];
        if ((pCurrentDviCtrl->pfnInit != (PFN_DVICTRL_INIT)0) &&
            (pCurrentDviCtrl->pfnInit(edgeSelect, busSelect, dualEdgeClkSelect, hsyncEnable, 
                              vsyncEnable, deskewEnable, deskewSetting, continuousSyncEnable, 
                              pllFilterEnable, pllFilterValue) == 0))
        {
            g_ucDeviceConfiguration[getCurrentDevice()] = dviCtrlIndex;
            return 0;
        }
    }
    
    return (-1);
}

/*
 *  dviCtrlResetChip
 *      This function resets the DVI Controller Chip.
 */
void dviResetChip()
{
    dvi_ctrl_device_t *pCurrentDviCtrl;
    
    pCurrentDviCtrl = getDviCtrl();
    if (pCurrentDviCtrl != (dvi_ctrl_device_t *)0)
        pCurrentDviCtrl->pfnResetChip();
}

/*
 *  dviGetChipString
 *      This function return a char string name of the current DVI Controller chip.
 *      It's convenient for application need to display the chip name.
 *      
 */
char *dviGetChipString()
{
    dvi_ctrl_device_t *pCurrentDviCtrl;
    
    pCurrentDviCtrl = getDviCtrl();
    if (pCurrentDviCtrl != (dvi_ctrl_device_t *)0)
        return pCurrentDviCtrl->pfnGetChipString();
    
    return gDviCtrlChipName; 
}

/*
 *  dviGetVendorID
 *      This function gets the vendor ID of the DVI controller chip.
 *
 *  Output:
 *      Vendor ID
 */
unsigned short dviGetVendorID()
{
    dvi_ctrl_device_t *pCurrentDviCtrl;
    
    pCurrentDviCtrl = getDviCtrl();
    if (pCurrentDviCtrl != (dvi_ctrl_device_t *)0)
        return pCurrentDviCtrl->pfnGetVendorId();
    
    return 0x0000;
}

/*
 *  dviGetDeviceID
 *      This function gets the device ID of the DVI controller chip.
 *
 *  Output:
 *      Device ID
 */
unsigned short dviGetDeviceID()
{
    dvi_ctrl_device_t *pCurrentDviCtrl;
    
    pCurrentDviCtrl = getDviCtrl();
    if (pCurrentDviCtrl != (dvi_ctrl_device_t *)0)
        return pCurrentDviCtrl->pfnGetDeviceId();
    
    return 0x0000;
}

/*
 *  dviEnableHotPlugDetection
 *      This function enables the Hot Plug detection.
 *
 *  Input:
 *      enableHotPlug   - Enable (=1) / disable (=0) Hot Plug detection 
 */
void dviEnableHotPlugDetection(
    unsigned char enableHotPlug
)
{
    dvi_ctrl_device_t *pCurrentDviCtrl;
    
    pCurrentDviCtrl = getDviCtrl();
    if (pCurrentDviCtrl != (dvi_ctrl_device_t *)0)
        pCurrentDviCtrl->pfnEnableHotPlugDetection(enableHotPlug);
}

/*  
 *  dviIsConnected
 *      Check if the DVI Monitor is connected.
 *
 *  Output:
 *      0   - Not Connected
 *      1   - Connected
 */
unsigned char dviIsConnected()
{
    dvi_ctrl_device_t *pCurrentDviCtrl;
    
    pCurrentDviCtrl = getDviCtrl();
    if (pCurrentDviCtrl != (dvi_ctrl_device_t *)0)
        return pCurrentDviCtrl->pfnIsConnected();
    
    /* Assume that it is always connected. */
    return 0x01;
}

/*
 *  dviSetPower
 *      This function sets the power configuration of the DVI Controller Chip.
 *
 *  Input:
 *      powerUp - Flag to set the power down or up
 */
void dviSetPower(
    unsigned char powerUp
)
{
    dvi_ctrl_device_t *pCurrentDviCtrl;
    
    pCurrentDviCtrl = getDviCtrl();
    if (pCurrentDviCtrl != (dvi_ctrl_device_t *)0)
        pCurrentDviCtrl->pfnSetPower(powerUp);
}
