/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  SAA7118.c --- Voyager GX SDK 
*  This file contains the source code for SAA7118 initialization.
* 
*******************************************************************/
#include <string.h>
#include "defs.h"
#include "capture.h"
#include "hardware.h"
#include "os.h"
#include "swi2c.h"

#include "ddkdebug.h"

/* Maximum number of supported SAA7118 devices */
#define MAX_CAPTURE_DEVICE                  2

/* The current device number in use. This variable is used to control multiple devices. */
static unsigned char g_ucCurrentDevice = 0xFF;

/* Number of devices that has been initialized and detected */
static unsigned char g_ucNumberOfDevices = 0;

/* The chip address of each device */
static unsigned char g_pucDeviceAddress[MAX_CAPTURE_DEVICE] = { 0, 0 };

/* Initialization flag */
static unsigned char g_ucInitialized = 0;

/*
 *  getCurrentVideoDecoder
 *      This function gets the current capture Device.
 *
 *  Output:
 *      The current device index.
 *      0xFF    - Device has not been initialized.
 */
unsigned char getCurrentVideoDecoder()
{
    return g_ucCurrentDevice;
}

/*
 *  setCurrentVideoDecoder
 *      This function sets the current capture Device.
 *
 *  Input:
 *      ucDeviceNumber - The device number to be set/accessed 
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
char setCurrentVideoDecoder(
    unsigned char ucDeviceNumber
)
{
    /* Error check */
    if (ucDeviceNumber < g_ucNumberOfDevices)
    {
        g_ucCurrentDevice = ucDeviceNumber;
        return 0;
    }
    return -1;
}

/*
 *   getTotalVideoDecoder 
 *       This function gets the total number of capture devices that has been
 *       initialized and detected.
 *       
 *   Output:
 *       Total number of initialized capture devices.
 *       
 */
unsigned char getTotalVideoDecoder()
{
    /* Initialize the capture device if it has not been initialized yet. */
    if (g_ucInitialized == 0)
        initVideoDecoder();
        
    return(g_ucNumberOfDevices);
}

/*
 *  checkVideoDecoderColorSystem
 *      This function checks the video format. 
 *
 *  Output:
 *      VIDEO_FORMAT_BLACK_WHITE    - B/W Format
 *      VIDEO_FORMAT_NTSC           - NTSC Format
 *      VIDEO_FORMAT_PAL            - PAL Format
 *      VIDEO_FORMAT_SECAM          - SECAM Format
 */
unsigned char checkVideoDecoderColorSystem() 
{
    return VIDEO_FORMAT_UNKNOWN;
}

/*
 *  detectVideoDecoderInput
 *      This function detects wheteher there is a video input coming to the
 *      video decoder. 
 *
 *  Output:
 *      0 - No Video Source is not detected
 *      1 - Video Source is detected
 *         
 */
unsigned char detectVideoDecoderInput() 
{
    return 1;               /* Video source is detected */
}

/*
 *  setVideoDecoderSyncType
 *      This function sets the sync type, either embedded in the data (ITU-656)
 *      or separate (ITU-601)
 *
 *  Input:
 *      SYNC_EMBEDDED    - The sync signal is embedded in the data.
 *      SYNC_INDEPENDENT - The sync signal is independent from the data.
 */
void setVideoDecoderSyncType(
    video_sync_type_t syncType
)
{
}

/*
 *  setInputMode
 *      This function sets input mode
 *
 *  Input:
 *      MODE_00..MODE_05    --> CVBS Modes 1
 *      MODE_06..MODE_0D    --> Y+C Modes 1
 *      MODE_0E..MODE_15    --> CVBS Modes 2
 *      MODE_16..MODE_1D    --> Y+C Modes2
 *      MODE_1E..MODE_1F    --> CVBS Modes 3
 *      MODE_20..MODE_21    --> Y-Pb-Pr Modes
 *      MODE_22..MODE_2D    --> Reserved
 *      MODE_2E..MODE_2F    --> Y-Pb-Pr Modes (Continue)
 *      MODE_30..MODE_3F    --> RGB Modes
 *
 *      MODE_00..MODE_15    --> VSB Modes^
 */
void setInputMode(
    unsigned char inputMode
)
{
}

/*
 *  setVideoDecoderVerticalInputStart
 *      This function sets input window start in vertical direction
 *
 *  Input:
 *      lineOffset  - Line offset
 */
void setVideoDecoderVerticalInputStart(
    unsigned short lineOffset
)
{
}

/*
 *  setVideoDecoderHorizontalInputStart
 *      This function sets input window start in horizontal direction
 *
 *  Input:
 *      lineOffset  - Line offset
 */
void setVideoDecoderHorizontalInputStart(
    unsigned short horizontalOffset
)
{
}

/*
 *  invertInputVideoFieldID
 *      This function invert the field ID of the input video source
 *
 *  Input:
 *      lineOffset  - Line offset
 */
void invertInputVideoFieldID(
    unsigned char invertFieldID
)
{
}

/*
 *  setVideoDecoderDataSize
 *      This function sets the video data bus (8-bit or 16-bit)
 *
 *  Input:
 *      dataSize    - Data bus where the data is transferred. Either 8_BIT
 *                    or 16_BIT
 */
void setVideoDecoderDataSize(
    video_data_size_t dataSize
)
{
}

/*
 *  configureVideoDecoder
 *      This function configures the video decoder.
 *  
 *  Input:
 *      dataSize        - Data bus where the data is transferred. Either 8_BIT
 *                        or 16_BIT
 *      syncType        - Sync type, either embedded in the data (ITU-656)
 *                        or separate (ITU-601)
 *      inputMode       - Input mode
 *                              MODE_00..MODE_05    --> CVBS Modes 1
 *                              MODE_06..MODE_0D    --> Y+C Modes 1
 *                              MODE_0E..MODE_15    --> CVBS Modes 2
 *                              MODE_16..MODE_1D    --> Y+C Modes2
 *                              MODE_1E..MODE_1F    --> CVBS Modes 3
 *                              MODE_20..MODE_21    --> Y-Pb-Pr Modes
 *                              MODE_22..MODE_2D    --> Reserved
 *                              MODE_2E..MODE_2F    --> Y-Pb-Pr Modes (Continue)
 *                              MODE_30..MODE_3F    --> RGB Modes
 *                              MODE_00..MODE_15    --> VSB Modes^
 *       
 *                          Notes:
 *                              ^ Overlapped modes. Need to set REFA(0x23[4]) = 1, 
 *                                GAFIX(0x03[2]) = 1, and DOSL(0x88[7:6]) = 0/1/2/3.
 *      invertFieldID   - Invert Input Field ID:
 *                          0   - Do not invert ID
 *                          1   - Invert the Field ID 
 *      horizontalOffset- Horizontal input window start in horizontal direction. 
 *      verticalOffset  - Vertical input window start in vertical direction.
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
uint32_t configureVideoDecoder(
    video_data_size_t dataSize,
    video_sync_type_t syncType,
    unsigned char inputMode,
    unsigned char invertFieldID,
    unsigned short horizontalOffset,
    unsigned short verticalOffset
)
{
    return 0;                                    
}

#ifdef DDKDEBUG
void printVideoDecoderRegister(
    unsigned char ucDeviceAddress
)
{
}
#endif

/*
 *  initVideoDecoder
 *      This function initialize the Video Capture Chip. 
 *
 *  Output:
 *      Total number of chip being detected.
 */
unsigned char initVideoDecoder() 
{
    /* Initialize all the SAA7118 chip if they have not been initialized yet. */
    if (g_ucInitialized == 0)
    {
        /* Set the Number of Devices to 0 */
        g_ucNumberOfDevices = 1;
        
        /* Set the Initialized flag */
        g_ucInitialized = 1;
    }
    
    return g_ucNumberOfDevices;
}
