/*******************************************************************
* 
*         Copyright (c) 2009 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  DVI.H --- SMI DDK 
*  This file contains the source code for the DVI controller chip
* 
*******************************************************************/
#ifndef _DVI_H_
#define _DVI_H_

/*
 *  Function pointer declaration 
 */
typedef long (*PFN_DVICTRL_INIT)(
    unsigned char edgeSelect,
    unsigned char busSelect,
    unsigned char dualEdgeClkSelect,
    unsigned char hsyncEnable,
    unsigned char vsyncEnable,
    unsigned char deskewEnable,
    unsigned char deskewSetting,
    unsigned char continuousSyncEnable,
    unsigned char pllFilterEnable,
    unsigned char pllFilterValue);
typedef void (*PFN_DVICTRL_RESETCHIP)();
typedef char* (*PFN_DVICTRL_GETCHIPSTRING)();
typedef unsigned short (*PFN_DVICTRL_GETVENDORID)();
typedef unsigned short (*PFN_DVICTRL_GETDEVICEID)();
typedef void (*PFN_DVICTRL_SETPOWER)(unsigned char powerUp);
typedef void (*PFN_DVICTRL_HOTPLUGDETECTION)(unsigned char enableHotPlug);
typedef unsigned char (*PFN_DVICTRL_ISCONNECTED)();
typedef unsigned char (*PFN_DVICTRL_CHECKINTERRUPT)();
typedef void (*PFN_DVICTRL_CLEARINTERRUPT)();

/* Structure to hold all the function pointer to the DVI Controller. */
typedef struct _dvi_ctrl_device_t
{
    PFN_DVICTRL_INIT                pfnInit;
    PFN_DVICTRL_RESETCHIP           pfnResetChip;
    PFN_DVICTRL_GETCHIPSTRING       pfnGetChipString;
    PFN_DVICTRL_GETVENDORID         pfnGetVendorId;
    PFN_DVICTRL_GETDEVICEID         pfnGetDeviceId;
    PFN_DVICTRL_SETPOWER            pfnSetPower;
    PFN_DVICTRL_HOTPLUGDETECTION    pfnEnableHotPlugDetection;
    PFN_DVICTRL_ISCONNECTED         pfnIsConnected;
    PFN_DVICTRL_CHECKINTERRUPT      pfnCheckInterrupt;
    PFN_DVICTRL_CLEARINTERRUPT      pfnClearInterrupt;
} dvi_ctrl_device_t;

/* 
 *  Function prototypes
 */

/*
 *  long getCurrentDviCtrl();
 *      This function gets the Dvi Controller of the current SM750 device.
 *
 *  Output:
 *      Null pointer if fail
 *      Pointer to the set of the supported DVI Controller functions.
 */
dvi_ctrl_device_t *getDviCtrl();
 
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
);

/*
 *  dviCtrlResetChip
 *      This function resets the DVI Controller Chip.
 */
void dviResetChip();

/*
 *  dviGetChipString
 *      This function return a char string name of the current DVI Controller chip.
 *      It's convenient for application need to display the chip name.
 *      
 */
char *dviGetChipString();

/*
 *  dviGetVendorID
 *      This function gets the vendor ID of the DVI controller chip.
 *
 *  Output:
 *      Vendor ID
 */
unsigned short dviGetVendorID();

/*
 *  dviGetDeviceID
 *      This function gets the device ID of the DVI controller chip.
 *
 *  Output:
 *      Device ID
 */
unsigned short dviGetDeviceID();

/*
 *  dviEnableHotPlugDetection
 *      This function enables the Hot Plug detection.
 *
 *  Input:
 *      enableHotPlug   - Enable (=1) / disable (=0) Hot Plug detection 
 */
void dviEnableHotPlugDetection(
    unsigned char enableHotPlug
);

/*  
 *  dviIsConnected
 *      Check if the DVI Monitor is connected.
 *
 *  Output:
 *      0   - Not Connected
 *      1   - Connected
 */
unsigned char dviIsConnected();

/*
 *  dviSetPower
 *      This function sets the power configuration of the DVI Controller Chip.
 *
 *  Input:
 *      powerUp - Flag to set the power down or up
 */
void dviSetPower(
    unsigned char powerUp
);

#endif  /* _DVI_H_ */
