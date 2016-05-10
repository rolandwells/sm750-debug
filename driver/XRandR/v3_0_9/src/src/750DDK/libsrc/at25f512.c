/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  AT25F512.c --- Voyager GX SDK 
*  This file contains the source code for the SSP/AT25F512 demo.
* 
*******************************************************************/
#include "defs.h"
#include "at25f512.h"
#include "hardware.h"
#include "ssp.h"

/* Chip ID */
#define AT25F512_CHIP_ID            0x1f65

#define DUMMY_DATA                  0xFF
#define AT25F512_WAIT_TIMER         0x10000

/*
 * Detect BIOS Flash Chip
 *
 * Output:
 *      0   - Device found
 *     -1   - Device not found
 */
int32_t sspDetectChip()
{   
    /* Initialize the SSP with 2MHz bitrate??? */
    sspInit(SSP_MODE_MASTER, SSP_FORMAT_MOTOROLA_SPI, SSP_DATA_SIZE_8_BIT, SSP_SPI_MODE_3, 2000000);

    /* Clear Interrupt */
    sspClearInterrupt();

    /* Read Device ID */
#if 1
    if (sspReadDeviceID() != 0)
#else    
    if (sspReadDeviceID() == AT25F512_CHIP_ID)
#endif    
        return 0;       /* Device found */
    else
        return (-1);    /* Device not found*/
} 

/*
 * Enable/Disable Write
 *
 * Input:
 *      enableWrite - Flag to enable/disable write
 *                      0   - disable
 *                      1   - enable
 */
void sspWriteEnable(
    unsigned char enableWrite
)
{
    unsigned short commandBuffer;
    
    /* Send command */
    if (enableWrite == 1)
        commandBuffer = AT25F512A_WRITE_ENABLE;
    else
        commandBuffer = AT25F512A_WRITE_DISABLE;
    sspTransmitData(&commandBuffer, 1);
}

/*
 * Reads the device ID 
 */
unsigned short sspReadDeviceID()
{
    unsigned short commandBuffer[8];
    unsigned short data[2];
    
    /* Clear Receive FIFO */
    sspClearRxFIFO();

    /* Send Read ID Command */
    commandBuffer[0] = AT25F512A_GET_DEVICE_ID;
    commandBuffer[1] = DUMMY_DATA;
    commandBuffer[2] = DUMMY_DATA;
    sspTransmitData(&commandBuffer, 3);

    /* Read the data */
    sspReceiveData((unsigned short *)0, 1);
    sspReceiveData(&data, 2);

    return ((((unsigned short)data[0]) << 8) + data[1]);
}

/*
 * Reads the device Status 
 */
unsigned short sspReadDeviceStatus()
{
    unsigned short commandBuffer[8];
    unsigned short data;
    
    /* Clear Receive FIFO */
    sspClearRxFIFO();

    /* Send CMD */
    commandBuffer[0] = AT25F512A_GET_STATUS;
    commandBuffer[1] = DUMMY_DATA;
    sspTransmitData(&commandBuffer, 2);

    /* Read the data */
    sspReceiveData((unsigned short *)0, 1);
    if (sspReceiveData(&data, 1) == 0)
        data = 0;

    return data;
}

/*
 * Writes the device status
 *
 * Input:
 *      newStatus   - status to be written to the device
 */
void sspWriteDeviceStatus(
    unsigned short newStatus
)
{
    unsigned short commandBuffer[2];

    /* Enable Device Write */
    sspWriteEnable(1);

    /* Send command */
    commandBuffer[0] = AT25F512A_SET_STATUS;
    commandBuffer[1] = newStatus;
    sspTransmitData(&commandBuffer, 2);

    /* Disable Device Write */
    sspWriteEnable(0);

    /* Wait device ready */
    sspWaitDeviceReady();
}

/*
 * Wait for the device to be ready
 *
 * Output:
 *      0   - Device is ready
 *     -1   - Device is not ready
 */
int32_t sspWaitDeviceReady()
{
    uint32_t timeout = AT25F512_WAIT_TIMER;
    
    /* Check the ready status by sending the read status command */
    while (timeout--)
    {
        if ((sspReadDeviceStatus() & 1) == 0)
            break;
    }
    
    if (timeout > 0)
        return 0;
    
    return (-1);
}

/*
 * Reads device data from the given memory address
 *
 * Input:
 *      memoryAddress   - Memory address to be read
 *      pData           - Pointer to a data storage to store the
 *                        data value that have been read.
 *      length          - Length of the data pointer in bytes. 
 *
 * Output:
 *      Total number of data read
 */
uint32_t sspReadDevice(
    uint32_t memoryAddress,
    unsigned char *pData,
    uint32_t length
)
{
    unsigned short commandBuffer[8], receiveData[4];
    unsigned char tempCount, index;
    uint32_t totalDataRead = 0;
    
    /* Clear FIFO */
    sspClearRxFIFO();

    /* Send CMD */
    commandBuffer[0] = AT25F512A_READ_DATA;
    commandBuffer[1] = 0;
    
    /* Clear the rest of the buffer with dummy values. */
    commandBuffer[4] = DUMMY_DATA;
    commandBuffer[5] = DUMMY_DATA;
    commandBuffer[6] = DUMMY_DATA;
    commandBuffer[7] = DUMMY_DATA;
    
    /* Reading the device. Since this driver is using 8-bit data size,
       therefore, the length is not adjusted. For bigger data size,
       please do not forget to adjust the length since the length is
       specified in bytes. */
    while (length > 0)
    {
        /* Set the memory address */
        commandBuffer[2] = (unsigned char) (memoryAddress>>8);
        commandBuffer[3] = (unsigned char) (memoryAddress);
        
        /* Calculate the number of data to be read. */
        tempCount = (length >= 4) ? 4 : length;

        /* Transmit data */
        if (sspTransmitData(&commandBuffer, 4 + tempCount) == 0)
            break;
    
        /* Read the dummy and the actual data */
        sspReceiveData((unsigned short *)0, 4);
        if (sspReceiveData(receiveData, tempCount) == 0)
            break;
        
        /* Copy the data to the output pointer. Depending on the data size,
           the (unsigned char) cast is needed for 8-bit or below. */
        for (index = 0; index < tempCount; index++)
            *pData++ = (unsigned char)receiveData[index];
        
        /* Adjust the memory address */
        memoryAddress += tempCount;
        
        /* Adjust the dataLength */
        length -= tempCount;
        
        /* Adjust the total read data. */
        totalDataRead += tempCount;
    }
        
    return totalDataRead;
}

/*
 * Program the device
 *
 * Input:
 *      memoryAddress   - Memory Starting Address to be programmed
 *      pData           - Pointer to a data to be programmed
 *      length          - Length of the data to be written to memory in byte
 *
 * Output:
 *      0   - Success
 *     -1   - Fail
 */
int32_t sspProgramDevice(
    uint32_t memoryAddress,
    unsigned char *pData, 
    uint32_t length
)
{
    unsigned short commandBuffer[8];
    unsigned char count, tempCount;
    
    /* Send CMD */
    commandBuffer[0] = AT25F512A_PROGRAM;
    commandBuffer[1] = 0;
    
    /* Programming the device */
    while (length > 0)
    {
        /* Set the memory address */
        commandBuffer[2] = (unsigned char) (memoryAddress>>8);
        commandBuffer[3] = (unsigned char) (memoryAddress);
        
        /* Setup the data. Do not forget to cast the data
           as unsigned short when the ssp data size is 8-bit
           or less. */
        tempCount = (length >= 4) ? 4 : length;
        for (count = 0; count < tempCount; count++)
        {
            commandBuffer[4 + count] = (unsigned short)(*pData);
            pData++;
        }
        
        /* Write Enable */
        sspWriteEnable(1);

        /* Transmit data */
        if (sspTransmitData(&commandBuffer, 4 + tempCount) == 0)
            return (-1);

        /* Wait Device Ready */
        if (sspWaitDeviceReady() != 0)
            return (-1);
        
        /* Adjust the memory address */
        memoryAddress += tempCount;
        
        /* Adjust the dataLength */
        length -= tempCount;
    }
    
    return 0;
}

/*
 * Erases a sector
 *
 * Input:
 *      sectorIndex - Sector to be erased
 */
void sspEraseSector(
    unsigned char sectorIndex
)
{
    unsigned short commandBuffer[8];

    /* Write Enable */
    sspWriteEnable(1);

    /* Send CMD */
    commandBuffer[0] = AT25F512A_ERASE_SECTOR;
    commandBuffer[1] = 0;
    commandBuffer[2] = sectorIndex << 7;
    commandBuffer[3] = 0;
    sspTransmitData(&commandBuffer, 4);

    /* Wait Device Ready */
    sspWaitDeviceReady();
}

/*
 * Erases the whole data on the chip
 */
void sspEraseChip()
{
    unsigned short commandBuffer;

    /* Write Enable */
    sspWriteEnable(1);

    /* Send CMD */
    commandBuffer = AT25F512A_ERASE_CHIP;
    sspTransmitData(&commandBuffer, 1);
    
    /* Wait Device Ready */
    sspWaitDeviceReady();
}

/*
 * Get the protect status
 *
 * Input:
 *      pBlockProtectStatus - Pointer to a variable to store the block protect status flag
 *      pWriteProtectStatus - Pointer to a variable to store the hardware protect status flag
 */
void sspGetProtectStatus(
    unsigned char *pBlockProtectStatus,
    unsigned char *pWriteProtectStatus
)
{
    unsigned short deviceStatus;
    
    deviceStatus = sspReadDeviceStatus();
    
    if (pBlockProtectStatus != (unsigned char *)0)
        *pBlockProtectStatus = ((deviceStatus & 0x0008) == 0) ? 0 : 1;
        
    if (pWriteProtectStatus != (unsigned char *)0)
        *pWriteProtectStatus = ((deviceStatus & 0x0004) == 0) ? 0 : 1;
}

/*
 * Writes the device status
 *
 * Input:
 *      blockWrProtectEnable    - Flag to enable/disable block write protect.
 *      hwWrProtectEnable       - Flag to enable/disable hardware write protect.
 */
void sspSetProtectStatus(
    unsigned char blockWrProtectEnable, 
    unsigned char hwWrProtectEnable
)
{
    unsigned short status = 0;
    
    if (blockWrProtectEnable == 1)
        status |= 0x0004;
    
    if (hwWrProtectEnable == 1)
        status |= 0x0080;

    sspWriteDeviceStatus(status);
}
