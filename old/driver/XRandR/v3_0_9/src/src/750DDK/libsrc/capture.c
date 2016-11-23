/*******************************************************************
 * 
 *         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
 * 
 *  All rights are reserved. Reproduction or in part is prohibited
 *  without the written consent of the copyright owner.
 * 
 *  Capture.C --- SM718 & SM750 SDK 
 *  This file contains the definitions for the Capture functions.
 * 
 *  Note:
 *      The VGX capture registers sets contain the same registers
 *      definition, with the exception on newer chip (SM502 and
 *      beyond) which have some additional new definition. With
 *      this in mind, we can use the ZV0 register definition
 *      when setting the other ZV register. Only when reading from
 *      and writing to the register, then the actual register address
 *      is used (use an offset).
 *      
 *******************************************************************/
#include <stdlib.h>
#include "defs.h"
#include "capture.h"
#include "chip.h"
#include "hardware.h"
#include "os.h"
#include "power.h"

#include "ddkdebug.h"

/* Different register offset location between the first and the second */
#define ZV_OFFSET_GAP                       0x8000

/* ZV Current Device Index being used. By default, ZV Index 0 is used */
static unsigned char g_ucCurrentZVDevice = 0;

/**********************************************************************
 *
 * ZV Port 0 and 1 Feature Implementation
 *
 **********************************************************************/

/*
 *  getTotalZVPort
 *      This function gets the total number of ZV Ports available.
 *
 *  Output:
 *      Total number of ZV Ports available
 */
unsigned char getTotalZVPort()
{
    logical_chip_type_t chipType;
    
    /* Check the chip type since not all the chip support the same number of
       ZV Devices
     */
    chipType = getChipType();
    
    switch(getChipType())
    {
        case SM718:
        case SM750:
            return 2;
        default:
            return 0;
    }
}

/*
 *  getCurrentZVPort
 *      This function gets the current ZV port that is being used.
 *
 *  Output:
 *      Current ZV device
 */
unsigned char getCurrentZVPort()
{
    return g_ucCurrentZVDevice;
}

/*
 *  setCurrentZVPort
 *      This function sets the current ZV port to be used.
 *
 *  Input:
 *      0   - Success
 *     -1  - Fail
 */
char setCurrentZVPort(
    unsigned char ucZVDevice
)
{
    /* Error Checking. Otherwise, it should not set anything */
    if (ucZVDevice < getTotalZVPort())
    {
        g_ucCurrentZVDevice = ucZVDevice;
        return 0;
    }
    
    return -1;
}

/*
 *  getZVRegisterOffset
 *      This function gets the offset of the current capture register.
 *
 *  Input:
 *      Register to be looked at.
 */
static uint32_t getZVRegisterOffset(
    uint32_t ulRegIndex
)
{
    uint32_t ulRegOffset;
    
    /* Get the register offset */
    if (g_ucCurrentZVDevice == 0)
        ulRegOffset = 0;
    else
        ulRegOffset = ZV_OFFSET_GAP;
        
    return (ulRegIndex + ulRegOffset);
}

/*
 *  isCaptureEnable
 *      This function checks if the capture is enabled or not. This is only
 *      check the current set ZV Port.
 *
 *  Output:
 *      0   - Capture is disabled.
 *      1   - Capture is enabled.
 */
unsigned char isCaptureEnable()
{
    uint32_t value;
    
    /* Check the capture enable bit */
    value = peekRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL));
    
    return (FIELD_GET(value, ZV0_CAPTURE_CTRL, CAP) == ZV0_CAPTURE_CTRL_CAP_ENABLE);
}

/*
 *  isCaptureEnableEx
 *      This function check if any (0 or more) of the capture devices is enabled
 *      or not. This will check every available ZV Port
 *
 *  Output:
 *      0x00            - Capture is disabled.
 *      0x01            - ZV Port 0 is enabled.
 *      0x02            - ZV Port 1 is enabled.
 *      (0x01 | 0x02)   - Both zv port are enabled
 */
unsigned char isCaptureEnableEx()
{
    uint32_t value;
    unsigned char tempZVDeviceNumber, deviceIndex;
    unsigned char deviceEnable = 0, maskDevice = 1;
    
    /* Save the current ZV Port device */
    tempZVDeviceNumber = g_ucCurrentZVDevice;
    
    for (deviceIndex = 0; deviceIndex < getTotalZVPort(); deviceIndex++)
    {
        /* Set the current device number */ 
        setCurrentZVPort(deviceIndex);
        
        /* Check the register value */
        if (isCaptureEnable())
            deviceEnable |= maskDevice;
        
        maskDevice <<= 1;
    }
    
    /* Restore the current ZV Port device */
    setCurrentZVPort(tempZVDeviceNumber);
    
    return deviceEnable;
}

/* 
 * Interlace detection interrupt handler. In order for the interlace status scan to be valid
 * the hardware needs to go through two capture sync to detect the status.
 */
static unsigned char gInterlaceInterrupt = 0;
void interlaceDetectionHandler()
{
    gInterlaceInterrupt++;
}

/* 
 * isCaptureInterlace
 *      This function checks whether the capture video source is interlaced or progressive
 *
 * Output:
 *      0   - Progressive
 *      1   - Interlace
 */
unsigned char isCaptureInterlace()
{
    uint32_t value, captureSize, timeout = 0x10000000;
    unsigned char captureEnable = 0, returnValue = 0;
    
    /* To detect the interlace status, the capture has to be enabled first before the
       hardware can detect the input video. However, if the capture is not set up properly
       then it might capture the video to a wrong memory address. One way to solve this
       is by setting the capture size to 0. This solution has not been tested. */
       
    /* Get the current capture size */
    captureSize = peekRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_SIZE));
    
    /* Set it to 0 */
    pokeRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_SIZE), 0);
    
    /* Reset the interlace interrupt counter and hook the interrupt. */
    gInterlaceInterrupt = 0;
    hookZVPortInterrupt((g_ucCurrentZVDevice == 0) ? ZVPORT0 : ZVPORT1, interlaceDetectionHandler);
    
    /* Enable capture */
    captureEnable = isCaptureEnable();
    startZVPortCapture();
    
    while ((timeout > 0) && (gInterlaceInterrupt < 2))
    {
        timeout--;
    }
    
    /* Check the capture enable bit */
    value = peekRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL));
    if (FIELD_GET(value, ZV0_CAPTURE_CTRL, SCAN) == ZV0_CAPTURE_CTRL_SCAN_INTERLACE)
        returnValue = 1;

    /* Stop the capture if it is not enabled previously. */
    if (captureEnable == 0)
        stopZVPortCapture();

    /* Unhook the interrupt. */        
    unhookZVPortInterrupt((g_ucCurrentZVDevice == 0) ? ZVPORT0 : ZVPORT1, interlaceDetectionHandler);
        
    /* Restore the capture size register */
    pokeRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_SIZE), captureSize);
        
    return returnValue;
}

/* getCurrentBufferCapture
 *      This function gets the current buffer captured
 *
 * Output:
 *      0   - Buffer 0
 *      1   - Buffer 1
 */
unsigned char getCurrentBufferCapture()
{
    uint32_t value;
    
    /* Check the capture enable bit */
    value = peekRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL));
    
    if (FIELD_GET(value, ZV0_CAPTURE_CTRL, CURRENT_BUFFER) == ZV0_CAPTURE_CTRL_CURRENT_BUFFER_1)
        return 1;
        
    return 0;
}

/* getCurrentBufferFieldStatus
 *      This function gets the current buffer Field Status
 *
 * Output:
 *      0   - Even field
 *      1   - Odd Field
 */
unsigned char getCurrentBufferFieldStatus()
{
    uint32_t value;
    
    /* Check the capture enable bit */
    value = peekRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL));
    if (FIELD_GET(value, ZV0_CAPTURE_CTRL, FIELD_INPUT) == ZV0_CAPTURE_CTRL_FIELD_INPUT_ODD_FIELD)
        return 1;   /* Odd Field */
    
    /* Even field */
    return 0;
}

/*
 *  getZVPortCaptureBuffer
 *      This function gets the capture buffer
 *
 *  Input:
 *      bufferIndex             - The index of the capture buffer address which status 
 *                                to be retrieved (0 = buffer 0, 1 = buffer 1)
 *      pCaptureBuffer0Address  - Pointer to a variable to store the capture buffer address.
 *      pBufferOffset           - Pointer to a variable to store the number of 128-bit aligned 
 *                                bytes per line of the capture buffer
 *
 *  Return:
 *      Status of the captured buffer
 *          0   - Flip not pending
 *          1   - Flip pending
 */
unsigned char getZVPortCaptureBuffer(
    unsigned char bufferIndex,
    uint32_t *pCaptureBufferAddress,
    unsigned short *pBufferOffset
)
{
    uint32_t value;
    unsigned char status = 0;
    
    if (bufferIndex == 0)
    {
        value = peekRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_BUF0_ADDRESS));
        
        if (FIELD_GET(value, ZV0_CAPTURE_BUF0_ADDRESS, STATUS) == ZV0_CAPTURE_BUF0_ADDRESS_STATUS_PENDING)
            status = 1;
            
        if (pCaptureBufferAddress)
            *pCaptureBufferAddress = FIELD_GET(value, ZV0_CAPTURE_BUF0_ADDRESS, ADDRESS);
    }
    else
    {
        value = peekRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_BUF1_ADDRESS));
        
        if (FIELD_GET(value, ZV0_CAPTURE_BUF1_ADDRESS, STATUS) == ZV0_CAPTURE_BUF1_ADDRESS_STATUS_PENDING)
            status = 1;
            
        if (pCaptureBufferAddress)
            *pCaptureBufferAddress = FIELD_GET(value, ZV0_CAPTURE_BUF1_ADDRESS, ADDRESS);
    }
    
    if (pBufferOffset)
        *pBufferOffset = FIELD_GET(peekRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_BUF_OFFSET)), 
                                    ZV0_CAPTURE_BUF_OFFSET, OFFSET);
       
    return status; 
}

/*
 *  setZVPortCaptureOffset
 *      This function sets the current capture buffer offset.
 *
 *  Input:
 *      bufferOffset            - Number of 128-bit aligned bytes per line of the capture buffer
 *                                Set it to 0 when there is no modification necessary.
 */
void setZVPortCaptureOffset(
    unsigned short bufferOffset
)
{
    /* Set Capture Buffer Offset (pitch) */
    pokeRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_BUF_OFFSET),
        FIELD_VALUE(peekRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_BUF_OFFSET)), 
                    ZV0_CAPTURE_BUF_OFFSET, OFFSET, bufferOffset));
}

/*
 *  setZVPortCaptureBuffer
 *      This function sets the current capture buffer.
 *
 *  Input:
 *      bufferIndex             - The index of the capture buffer address which to be 
 *                                set (0 = buffer 0, 1 = buffer 1)
 *      captureBufferAddress    - The capture buffer address
 */
void setZVPortCaptureBuffer(
    unsigned char bufferIndex,
    uint32_t captureBufferAddress
)
{
    uint32_t value;
    
    if (bufferIndex == 0)
    {
        /* Set Capture Buffer 0 */
        value = FIELD_SET(0, ZV0_CAPTURE_BUF0_ADDRESS, STATUS, PENDING) |
                FIELD_SET(0, ZV0_CAPTURE_BUF0_ADDRESS, EXT, LOCAL) |
                FIELD_VALUE(0, ZV0_CAPTURE_BUF0_ADDRESS, ADDRESS, captureBufferAddress);
        pokeRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_BUF0_ADDRESS), value);
    }
    else
    {
        /* Set Capture Buffer 1 */
        value = FIELD_SET(0, ZV0_CAPTURE_BUF1_ADDRESS, STATUS, PENDING) |
                FIELD_SET(0, ZV0_CAPTURE_BUF1_ADDRESS, EXT, LOCAL) |
                FIELD_VALUE(0, ZV0_CAPTURE_BUF1_ADDRESS, ADDRESS, captureBufferAddress);
        pokeRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_BUF1_ADDRESS), value);
    }
}

/*
 *  setZVPortDataSize
 *      This function set the data size and open the path.
 *
 *  Input:
 *      videoSize   - The data bus size (either 8 bit or 16 bit)
 *      lower8Bit   - Flag to indicate of using the lower 8 bit
 */
void setZVPortDataSize(            
    video_data_size_t videoSize,            /* Capture Size, 0: 16-bit, 1: 8-bit */
    unsigned char lower8Bit
)
{
    uint32_t value;
    
    /* Enable the lower 8-bit data path */
    if ((videoSize == DATA_SIZE_16_BIT) || (lower8Bit == 0))
    {
        value = peekRegisterDWord(GPIO_MUX);
        value = FIELD_SET(value, GPIO_MUX, 7, GPIO_ZVPORT);
        value = FIELD_SET(value, GPIO_MUX, 6, GPIO_ZVPORT);
        value = FIELD_SET(value, GPIO_MUX, 5, GPIO_ZVPORT);
        value = FIELD_SET(value, GPIO_MUX, 4, GPIO_ZVPORT);
        value = FIELD_SET(value, GPIO_MUX, 3, GPIO_ZVPORT);
        value = FIELD_SET(value, GPIO_MUX, 2, GPIO_ZVPORT);
        value = FIELD_SET(value, GPIO_MUX, 1, GPIO_ZVPORT);
        value = FIELD_SET(value, GPIO_MUX, 0, GPIO_ZVPORT);
        pokeRegisterDWord(GPIO_MUX, value);
    }
    
    /* Enable the upper 8-bit data path */
    if ((videoSize == DATA_SIZE_16_BIT) || (lower8Bit == 1))
    {
        value = peekRegisterDWord(GPIO_MUX);
        value = FIELD_SET(value, GPIO_MUX, 15, GPIO_ZVPORT);
        value = FIELD_SET(value, GPIO_MUX, 14, GPIO_ZVPORT);
        value = FIELD_SET(value, GPIO_MUX, 13, GPIO_ZVPORT);
        value = FIELD_SET(value, GPIO_MUX, 12, GPIO_ZVPORT);
        value = FIELD_SET(value, GPIO_MUX, 11, GPIO_ZVPORT);
        value = FIELD_SET(value, GPIO_MUX, 10, GPIO_ZVPORT);
        value = FIELD_SET(value, GPIO_MUX, 9,  GPIO_ZVPORT) ;
        value = FIELD_SET(value, GPIO_MUX, 8,  GPIO_ZVPORT) ;
        pokeRegisterDWord(GPIO_MUX, value);
    }
    
    /* Set the capture size bit, either to use 16-bit or 8-bit */
    value = peekRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL)); 
    if (videoSize == DATA_SIZE_16_BIT)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, CS, 16);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, CS, 8);
    pokeRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL), value);
}

/*
 *  startZVPortCapture
 *      This function starts the capture.
 */
void startZVPortCapture()
{
    uint32_t value;
    
    /* Enable capture */
    value = peekRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL));
    value = FIELD_SET(value, ZV0_CAPTURE_CTRL, CAP, ENABLE);
    pokeRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL), value);
}

/*
 *  stopZVPortCapture
 *      This function stops the capture.
 */
void stopZVPortCapture()
{
    uint32_t value;
    
    /* Disable capture */
    value = peekRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL));
    value = FIELD_SET(value, ZV0_CAPTURE_CTRL, CAP, DISABLE);   
    pokeRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL), value);
}

/*
 *  setZVPortCaptureClip
 *      This function sets the clipping on the video source.
 *
 *  Input:
 *      xClip       - Number of pixel to skip after HRef
 *      yEvenClip   - Number of lines on the even field to skip after VSync
 *      yOddclip    - Number of lines on the odd field to skip after VSync
 */
void setZVPortCaptureClip(
    uint32_t xClip,
    uint32_t yEvenClip,
    uint32_t yOddclip
)
{
    pokeRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CLIP),
        FIELD_VALUE(0, ZV0_CAPTURE_CLIP, YCLIP_EVEN_FIELD, yEvenClip) |
        FIELD_VALUE(0, ZV0_CAPTURE_CLIP, XCLIP, xClip));

#ifndef VALIDATION_CHIP
    pokeRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_BUF_OFFSET),
        FIELD_VALUE(peekRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_BUF_OFFSET)), 
                    ZV0_CAPTURE_BUF_OFFSET, YCLIP_ODD_FIELD, yOddclip));
#endif
}

/*
 *  setZVPortCaptureSize
 *      This function sets the video source capture size.
 *
 *  Input:
 *      sourceHeight    - Number of lines to capture
 *      sourceWidth     - Number of pixels to capture
 */
void setZVPortCaptureSize(
    unsigned short sourceHeight, 
    unsigned short sourceWidth
)
{
    /* Set Capture Size */
    pokeRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_SIZE),
        FIELD_VALUE(0, ZV0_CAPTURE_SIZE, HEIGHT, sourceHeight) |
        FIELD_VALUE(0, ZV0_CAPTURE_SIZE, WIDTH, sourceWidth));
}

/*
 *  enableZVPortHalfShrink
 *      This function enable or disable the half shrink feature.
 * 
 *  Input:
 *      horizontalHalfShrink    - Flag for the horizontal half shrink
 *      verticalHalfShrink      - Flag for the vertical half shrink
 */
void enableZVPortHalfShrink(
    unsigned char  halfHorizontalShrink,    /* Horizontal 1/2 shink, 0: Disable, 1: Enable */
    unsigned char  halfVerticalShrink       /* Vertical 1/2 shink, 0: Disable, 1: Enable */
)
{
    uint32_t value;

    value = peekRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL));
    
    /* Enable/Disable 1/2 Horizontal Shrink*/
    if (halfHorizontalShrink == 0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, HSK, DISABLE);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, HSK, ENABLE);

    /* Enable/Disable 1/2 Vertical Shrink */
    if (halfVerticalShrink == 0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, VSK, DISABLE);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, VSK, ENABLE);

    pokeRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL), value);        
}

/*
 *  setZVPortConstants
 *      This function sets the ZV Port constants.
 *
 *  Input:
 *      yConstant       - Y Constant Value
 *      redConstant     - Red Constant Value
 *      greenConstant   - Green Constant Value
 *      blueConstant    - Blue Constant Value
 */
void setZVPortConstants(
    unsigned char  yConstant,               /* Y Adjustment */
    unsigned char  redConstant,             /* Red Conversion constant */
    unsigned char  greenConstant,           /* Green Conversion constant */
    unsigned char  blueConstant             /* Blue Conversion constant */
)
{
    /* Set Capture Constant */
    pokeRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_YRGB_CONST),
        FIELD_VALUE(0, ZV0_CAPTURE_YRGB_CONST, Y, yConstant) |
        FIELD_VALUE(0, ZV0_CAPTURE_YRGB_CONST, R, redConstant) |
        FIELD_VALUE(0, ZV0_CAPTURE_YRGB_CONST, G, greenConstant) |
        FIELD_VALUE(0, ZV0_CAPTURE_YRGB_CONST, B, blueConstant));
}

/*
 *  setZVPortCaptureMethod
 *      This function sets the capture method used in the de-interlace.
 *
 *  Input:
 *      captureMethod   - Capture Method used (0 = BOB, 1 = WEAVE, 2 = Progressive)
 */ 
void setZVPortCaptureMethod(
    uint32_t captureMethod
)
{
    uint32_t value;
    
    value = peekRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL));
    
    /* Select the capture method, either BOB or WEAVE */
    if (captureMethod == 0)         /* BOB */
    {
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, BOB, ENABLE);
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, WEAVE, DISABLE);
    }    
    else if (captureMethod == 1)    /* WEAVE */
    {
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, BOB, DISABLE);
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, WEAVE, ENABLE);
    }
    else                            /* Progressive */
    {
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, BOB, DISABLE);
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, WEAVE, DISABLE);
    }
    
    pokeRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL), value);
}

/*
 *  enableZVPortHorzAverage
 *      This function enables/disables the horizontal averaging feature.
 *
 *  Input:
 *      enable  - Flag to enable or disable the horizontal averaing feature.
 */ 
void enableZVPortHorzAverage(
    uint32_t enable
)
{
    uint32_t value;
    
    value = peekRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL));
    
    /* Enable/Disable horizontal Averaging */
    if (enable == 0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, HA, DISABLE);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, HA, ENABLE);
        
    pokeRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL), value);
}

/*
 *  initCapture
 *      This function initialize the capture control.
 *
 *  Input:
 *      captureParam    - Capture Parameter
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */ 
char initZVPort(
    capture_t *pCaptureParam
)
{
    uint32_t value = 0;
        
    /* Enable/Disable ITU 656 input data. ITU-656 has the sync embedded inside the data */
    if (pCaptureParam->inputITU656 == 0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, 656, DISABLE);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, 656, ENABLE);

    /* Convert the Input data format from YUV to RGB */
    if (pCaptureParam->convertYUVtoRGB == 0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, RGB, DISABLE);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, RGB, ENABLE);

    /* Capture continuously or using the S-bit (reflected in capture buffer address bit 31. */
    if (pCaptureParam->captureControl == 0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, CC, CONTINUE);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, CC, CONDITION);

    /* Enable/disable double buffer mechanism */
    if (pCaptureParam->doubleBuffer == 0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, DB, DISABLE);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, DB, ENABLE);

    /* Enable/Disable Field Swap */
    if (pCaptureParam->fieldSwap == 0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, FS, DISABLE);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, FS, ENABLE);

    /* Set the capture format (YUV or RGB) */
    if (pCaptureParam->captureFormat == 0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, CF, YUV);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, CF, RGB);
        
    /* Enable/Disable byte swap */
    if (pCaptureParam->byteSwap == 0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, BS, DISABLE);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, BS, ENABLE);

    /* Enable/Disable UV Swap */
    if (pCaptureParam->uvSwap==0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, UVS, DISABLE);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, UVS, ENABLE);

    /* Set the clock polarity */
    if (pCaptureParam->clockPolarity == 0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, CP, HIGH);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, CP, LOW);

    /* Set the HRef Phase */
    if (pCaptureParam->hRefPhase == 0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, HP, HIGH);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, HP, LOW);
        
    /* Set the VSync Phase */
    if (pCaptureParam->vsyncPhase == 0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, VP, HIGH);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, VP, LOW);

    /* Set the field detection, either on rising edge or falling edge of the clock */
    if (pCaptureParam->fieldDetect == 0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, FD, RISING);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, FD, FALLING);

    /* Enable/Disable horizontal Averaging */
    if (pCaptureParam->horizontalAveraging == 0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, HA, DISABLE);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, HA, ENABLE);

    /* Set the HRef adjustment, either normal or use one clock delay */
    if (pCaptureParam->delayHRef == 0)
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, ADJ, NORMAL);
    else
        value = FIELD_SET(value, ZV0_CAPTURE_CTRL, ADJ, DELAY);
        
    pokeRegisterDWord(getZVRegisterOffset(ZV0_CAPTURE_CTRL), value);
    
    /* Enable ZV Port 1 Clock when ZV Port 1 is enabled */
    if (g_ucCurrentZVDevice == 1)
    {
        value = FIELD_SET(peekRegisterDWord(GPIO_MUX), GPIO_MUX, 16, GPIO_ZVPORT);
        pokeRegisterDWord(GPIO_MUX, value);
    }
    
    /* Select the capture method, either BOB, WEAVE, or progressive */
    setZVPortCaptureMethod(pCaptureParam->captureMethod);

    return 0;
}

/*
 *   capturePanelOutput
 *       This function enables the capture panel output. Only available on the
 *       ZV Port index 1.
 */
void setZVPortCapturePanelOutput(
    uint32_t enablePanelCapture
)
{
    uint32_t value;
    
    /* Only ZV Device 1 supports this feature */
    if (g_ucCurrentZVDevice == 1)
    {
        if (enablePanelCapture == 1)
        {
            value = FIELD_SET(peekRegisterDWord(ZV1_CAPTURE_CTRL), ZV1_CAPTURE_CTRL, PANEL, ENABLE);
            pokeRegisterDWord(ZV1_CAPTURE_CTRL, value);
        }
        else
        {
            value = FIELD_SET(peekRegisterDWord(ZV1_CAPTURE_CTRL), ZV1_CAPTURE_CTRL, PANEL, DISABLE);
            pokeRegisterDWord(ZV1_CAPTURE_CTRL, value);
        }
    }
}

/* 
 *  checkZVPortIntrStatus
 *      This function is used to check the ZVPort Interrupt status for polling purpose
 */
uint32_t checkZVPortIntrStatus(
    zv_port_ctrl_t zvPortControl
)
{
    uint32_t value;

    value = peekRegisterDWord(RAW_INT);
    if (zvPortControl == ZVPORT1)
    {
        if (FIELD_GET(value, RAW_INT, ZVPORT1_VSYNC) == RAW_INT_ZVPORT1_VSYNC_ACTIVE)
            return 1;
        else
            return 0;
    }
    else
    {
        if (FIELD_GET(value, RAW_INT, ZVPORT0_VSYNC) == RAW_INT_ZVPORT0_VSYNC_ACTIVE)
            return 1;
        else
            return 0;
    }
}

/* 
 *  clearZVPortIntrStatus
 *      This function is clears ZVPort Interrupt status.
 */
void clearZVPortIntrStatus(
    zv_port_ctrl_t zvPortControl
)
{
    uint32_t value;
    
    if (zvPortControl == ZVPORT1)
        value = FIELD_SET(0, RAW_INT, ZVPORT1_VSYNC, CLEAR);
    else
        value = FIELD_SET(0, RAW_INT, ZVPORT0_VSYNC, CLEAR);
    pokeRegisterDWord(RAW_INT, value);
}

/**********************************************************************
 *
 * ZV Port 0 and 1 Interrupt Management
 *
 **********************************************************************/
typedef struct _zv_port_interrupt_t
{
    struct _zv_port_interrupt_t *next;
    void (*handler)(void);
}
zv_port_interrupt_t;

static zv_port_interrupt_t *zvPort0IntHandlers = ((zv_port_interrupt_t *)0);
static zv_port_interrupt_t *zvPort1IntHandlers = ((zv_port_interrupt_t *)0);

/* ZV Port 0 Interrupt Service Routine */
void zvPort0ISR(
    uint32_t status
)
{
    zv_port_interrupt_t *pfnHandler;
    
    if (FIELD_GET(status, INT_STATUS, ZVPORT0_VSYNC) == INT_STATUS_ZVPORT0_VSYNC_ACTIVE)
    {
        /* Walk all registered handlers for handlers that support this interrupt status */
        for (pfnHandler = zvPort0IntHandlers; pfnHandler != ((zv_port_interrupt_t *)0); pfnHandler = pfnHandler->next)
            pfnHandler->handler();
            
        /* Clear the ZV Port 0 Interrupt */
        pokeRegisterDWord(RAW_INT, FIELD_SET(0, RAW_INT, ZVPORT0_VSYNC, CLEAR));
    }            
}

/* CRT VSync Interrupt Service Routine */
void zvPort1ISR(
    uint32_t status
)
{
    zv_port_interrupt_t *pfnHandler;
    
    if (FIELD_GET(status, INT_STATUS, ZVPORT1_VSYNC) == INT_STATUS_ZVPORT1_VSYNC_ACTIVE)
    {
        /* Walk all registered handlers for handlers that support this interrupt status */
        for (pfnHandler = zvPort1IntHandlers; pfnHandler != ((zv_port_interrupt_t *)0); pfnHandler = pfnHandler->next)
            pfnHandler->handler();
            
        /* Clear the ZV Port 1 Interrupt */
        pokeRegisterDWord(RAW_INT, FIELD_SET(0, RAW_INT, ZVPORT1_VSYNC, CLEAR));
    }            
}

/*  
 *  hookZVPortInterrupt
 *      Register ZV Port Interrupt function.
 *
 *  Note:
 *      Somehow the ZVPort 1 interrupt mask can not be enabled. Therefore, this interrupt
 */
unsigned short hookZVPortInterrupt(
    zv_port_ctrl_t zvPortControl,
    void (*handler)(void)
)
{
    zv_port_interrupt_t *pfnNewHandler, *pfnHandler, *pfnInterruptHandler;
    unsigned short returnValue = 0;

    /* Allocate a new interrupt structure */
    pfnNewHandler = (zv_port_interrupt_t *)malloc(sizeof(zv_port_interrupt_t));
    if (pfnNewHandler == ((zv_port_interrupt_t *)0))
    {
        /* No more memory */
        return (-1);
    }
    
    /* Get the right pointer to the interrupt handler */
    if (zvPortControl == ZVPORT1)
        pfnInterruptHandler = zvPort1IntHandlers;
    else
        pfnInterruptHandler = zvPort0IntHandlers;

    /* Make sure that it has not been register more than once */
    for (pfnHandler = pfnInterruptHandler; pfnHandler != ((zv_port_interrupt_t *)0); pfnHandler = pfnHandler->next)
    {
        if (pfnHandler->handler == handler)
            return (-1);                   /* Duplicate entry */
    }
        
    /* If this is the first interrupt handler, register this ZV Port ISR */
    if (pfnInterruptHandler == ((zv_port_interrupt_t *)0))
    {
        returnValue = registerHandler((zvPortControl == ZVPORT1) ? zvPort1ISR : zvPort0ISR, 
                                      (zvPortControl == ZVPORT1) ? 
                                          FIELD_SET(0, INT_MASK, ZVPORT1_VSYNC, ENABLE) :
                                          FIELD_SET(0, INT_MASK, ZVPORT0_VSYNC, ENABLE));
    }

    if (returnValue == 0)
    {
        /* Fill interrupt structure. */
        pfnNewHandler->next = pfnInterruptHandler;
        pfnNewHandler->handler = handler;
        pfnInterruptHandler = pfnNewHandler;
    }
    
    /* Update the actual handler */
    if (zvPortControl == ZVPORT0)
        zvPort0IntHandlers = pfnInterruptHandler;
    else
        zvPort1IntHandlers = pfnInterruptHandler;
    
    return returnValue;
}

/* Unregister a registered panel VSync interrupt handler */
unsigned short unhookZVPortInterrupt(
    zv_port_ctrl_t zvPortControl,
    void (*handler)(void)
)
{
    zv_port_interrupt_t *pfnHandler, *prev, *pfnInterruptHandler;
    uint32_t mask;
    
    if (zvPortControl == ZVPORT1)
        pfnInterruptHandler = zvPort1IntHandlers;
    else
        pfnInterruptHandler = zvPort0IntHandlers;

    /* Find the requested handle to unregister */
    for (pfnHandler = pfnInterruptHandler, prev = ((zv_port_interrupt_t *)0); 
         pfnHandler != ((zv_port_interrupt_t *)0); 
         prev = pfnHandler, pfnHandler = pfnHandler->next)
    {
        if (pfnHandler->handler == handler)
        {
            /* Remove the interrupt handler */
            if (prev == ((zv_port_interrupt_t *)0))
            {
                pfnInterruptHandler = pfnHandler->next;
                
                /* Update the actual handler */
                if (zvPortControl == ZVPORT1)
                    zvPort1IntHandlers = pfnInterruptHandler;
                else
                    zvPort0IntHandlers = pfnInterruptHandler;
            }
            else
            {
                prev->next = pfnHandler->next;
            }
            free(pfnHandler);
            
            /* If this was the last interrupt handler, remove the IRQ handler */
            if (pfnInterruptHandler == ((zv_port_interrupt_t *)0))
            {
                unregisterHandler((zvPortControl == ZVPORT1) ? zvPort1ISR : zvPort0ISR);
            }
            
            /* Success */
            return (0);
        }
    }

    /* Oops, handler is not registered */
    return (-1);
}
