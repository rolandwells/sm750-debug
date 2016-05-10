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
#include "saa7118.h"

#include "ddkdebug.h"

/* SAA7118 Chip ID */
#define SAA7118_CHIP_ID                     0x10

/* Maximum number of supported SAA7118 devices */
#define MAX_CAPTURE_DEVICE                  2

/* I2C Address of each SAA7118 chip */
#define SAA7118_I2C_ADDRESS1                0x40  /* Decoder0 -> ZV0 */
#define SAA7118_I2C_ADDRESS2                0x42  /* Decoder1 -> ZV1 */

/* Default value for the SAA7118 register with Composite_Video input at 640x480. */
const unsigned char SAA7118InitTable[] = {
    /* 00   01   02   03   04   05   06   07   08   09   0A   0B   0C   0D   0E   0F */
     0x00,0x47,0xD2,0x10,0x90,0x90,0xEB,0x00,0x98,0x40,0x80,0x44,0x40,0x00,0x8B,0x2A,   /* 00 */
     0x0E,0x00,0x00,0x00,0x00,0x11,0xFE,0xD8,0x40,0x80,0x00,0x00,0x00,0x00,0x00,0x00,   /* 10 */
     0x00,0x00,0x00,0x03,0x10,0x10,0x00,0x00,0x00,0x00,0x80,0x48,0x40,0x00,0x00,0x00,   /* 20 */
     0xBC,0xDF,0x02,0x00,0xCD,0xCC,0x3A,0x00,0x03,0x20,0x00,0x00,0x00,0x00,0x00,0x00,   /* 30 */
     0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,   /* 40 */
     0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x47,0x06,0x03,0x00,0x00,0x00,0x00,   /* 50 */
     0x00,0x0D,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,   /* 60 */
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,   /* 70 */
     0x1C,0x00,0x00,0x01,0xC5,0x06,0x40,0x01,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,   /* 80 */     
     0x00,0x08,0x10,0x80,0x03,0x00,0xD0,0x02,0x0E,0x00,0xF2,0x00,0x80,0x02,0xF0,0x00,   /* 90 */ 
     0x01,0x00,0x00,0x00,0x80,0x40,0x40,0x00,0x80,0x04,0x00,0x00,0x40,0x02,0x00,0x00,   /* A0 */     
     0x00,0x04,0x00,0x04,0x00,0x00,0x00,0x00,0x20,0x21,0x01,0x01,0x20,0x21,0x01,0x01,   /* B0 */
};

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
    /* Check Video color system */ 
    switch (swI2CReadReg(g_pucDeviceAddress[g_ucCurrentDevice], 0x1E) & 0x03)
    {
        case 0:
            return VIDEO_FORMAT_BLACK_WHITE;
        case 1:
            return VIDEO_FORMAT_NTSC;
        case 2:
            return VIDEO_FORMAT_PAL;
        case 3:
            return VIDEO_FORMAT_SECAM;
    }
    
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
    if (swI2CReadReg(g_pucDeviceAddress[g_ucCurrentDevice], 0x1F) & 0x01)
        return 1;               /* Video source is detected */
    else
        return 0;               /* No Video source is detected */
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
    unsigned char ucRegValue;
    
    ucRegValue = swI2CReadReg(g_pucDeviceAddress[g_ucCurrentDevice], IPORT_CONFIG)
                 & ~IPORT_CONFIG_ITU656_MASK;
    
    /* Enable the ITU-656 when the sync signal is embedded in the data. */ 
    if (syncType == SYNC_EMBEDDED)
        ucRegValue |= IPORT_CONFIG_ITU656_ENABLE;

    swI2CWriteReg(g_pucDeviceAddress[g_ucCurrentDevice], IPORT_CONFIG, ucRegValue);
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
 *
 *  Notes:
 *      ^ Overlapped modes. Need to set REFA(0x23[4]) = 1, GAFIX(0x03[2]) = 1, 
 *        and DOSL(0x88[7:6]) = 0/1/2/3.
 */
void setInputMode(
    unsigned char inputMode
)
{
    swI2CWriteReg(g_pucDeviceAddress[g_ucCurrentDevice], 
                  INPUT_CTRL, 
                  (inputMode & INPUT_CTRL_MODE_SELECTION_MASK));                  
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
    swI2CWriteReg(g_pucDeviceAddress[g_ucCurrentDevice],
                  VERTICAL_INPUT_WINDOW_START_MSB,
                  (unsigned char)((lineOffset >> 8) & 0x0F));
                  
    swI2CWriteReg(g_pucDeviceAddress[g_ucCurrentDevice],
                  VERTICAL_INPUT_WINDOW_START_LSB,
                  (unsigned char)lineOffset);
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
    swI2CWriteReg(g_pucDeviceAddress[g_ucCurrentDevice],
                  HORIZONTAL_INPUT_WINDOW_START_MSB,
                  (unsigned char)((horizontalOffset >> 8) & 0x0F));
                  
    swI2CWriteReg(g_pucDeviceAddress[g_ucCurrentDevice],
                  HORIZONTAL_INPUT_WINDOW_START_LSB,
                  (unsigned char)horizontalOffset);
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
    unsigned char ucRegValue;
    
    ucRegValue = swI2CReadReg(g_pucDeviceAddress[g_ucCurrentDevice], XPORT_INPUT_CONFIG)
            & ~XPORT_INPUT_CONFIG_FIELD_ID_MASK;
    
    if (invertFieldID == 1)
        ucRegValue |= XPORT_INPUT_CONFIG_FIELD_ID_INVERT;
    else
        ucRegValue |= XPORT_INPUT_CONFIG_FIELD_ID_NORMAL;
        
    swI2CWriteReg(g_pucDeviceAddress[g_ucCurrentDevice],
                  XPORT_INPUT_CONFIG,
                  ucRegValue);
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
    unsigned char ucRegValue;
    
    ucRegValue = swI2CReadReg(g_pucDeviceAddress[g_ucCurrentDevice], IPORT_CONFIG)
                 & ~IPORT_CONFIG_DATA_SIZE_MASK;
    
    /* Enable the 16-bit data transfer */ 
    if (dataSize == DATA_SIZE_16_BIT)
        ucRegValue |= IPORT_CONFIG_DATA_SIZE_16_BIT;             
    
    swI2CWriteReg(g_pucDeviceAddress[g_ucCurrentDevice], IPORT_CONFIG, ucRegValue);
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
    /* 
     * Set the data size 
     */
    setVideoDecoderDataSize(dataSize);
    
    /*
     * Set the sync type
     */
    setVideoDecoderSyncType(syncType);
                  
    /*
     * Set Input Mode 
     */
    setInputMode(inputMode);                  
    
    /*
     * Set the field ID invertion if necessary
     */
    invertInputVideoFieldID(invertFieldID); 
                  
    /* 
     * Set the horizontal offset. This will move the data rightward  
     */
    setVideoDecoderHorizontalInputStart(horizontalOffset);
                  
    /* 
     * Set the line/vertical offset. This will move the data downward  
     */
    setVideoDecoderVerticalInputStart(verticalOffset);
                  
    return 0;                                    
}

#ifdef DDKDEBUG
void printVideoDecoderRegister(
    unsigned char ucDeviceAddress
)
{
    unsigned char i;
    
    for (i = 0; i < 0xBF; i++)
    {
        if ((i % 16) == 0)
            DDKDEBUGPRINT((CAPTURE_LEVEL, "\n"));
    
        DDKDEBUGPRINT((CAPTURE_LEVEL, "%02x ", (unsigned char) swI2CReadReg(ucDeviceAddress, i)));
    }
    
    DDKDEBUGPRINT((CAPTURE_LEVEL, "\n"));
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
    unsigned char ucRegIndex, ucRegValue;
    unsigned char ucDeviceAddress;
    uint32_t delayCount;
    
    /* Initialize all the SAA7118 chip if they have not been initialized yet. */
    if (g_ucInitialized == 0)
    {
        /* Set the Number of Devices to 0 */
        g_ucNumberOfDevices = 0;
        
        /* Clear the Device Address Arrays */
        memset((unsigned char *)g_pucDeviceAddress, 0, (sizeof(g_pucDeviceAddress)/sizeof(unsigned char)));
        
        /* Initialize the i2c bus */
        swI2CInit(DEFAULT_I2C_SCL, DEFAULT_I2C_SDA);
    
        for (ucDeviceAddress = SAA7118_I2C_ADDRESS1; ucDeviceAddress <= SAA7118_I2C_ADDRESS2; ucDeviceAddress+=2)
        {
            /* Check if SAA7118 Chip exists */
            if ((swI2CReadReg(ucDeviceAddress, 0x00) & 0xF0) == SAA7118_CHIP_ID)
            {
#ifdef DDKDEBUG
                DDKDEBUGPRINT((CAPTURE_LEVEL, "Chip ID: %02x\n", swI2CReadReg(ucDeviceAddress, 0x00)));
#endif            
                /* Save the Device Address in the device address array */
                g_pucDeviceAddress[g_ucNumberOfDevices] = ucDeviceAddress;
                
                /* Increment the counter */
                g_ucNumberOfDevices++;
                
                /* Init SAA7118 */
                for (ucRegIndex = 0xbf; ucRegIndex > 0; ucRegIndex--)
                    swI2CWriteReg(ucDeviceAddress, ucRegIndex, SAA7118InitTable[ucRegIndex]);
                    
#ifdef DDKDEBUG
                printVideoDecoderRegister(ucDeviceAddress);
#endif                    
            }
        }
        
        /* Set the Initialized flag */
        g_ucInitialized = 1;
        
        /* Put some delay here to make sure the chip is powered and working.
           This delay is not accurate since different system has different 
           clock speed. */
        delayCount = 0x10000000; /* 0xA000000 */
        while (delayCount)
            delayCount--;
    }
    
    return g_ucNumberOfDevices;
}
