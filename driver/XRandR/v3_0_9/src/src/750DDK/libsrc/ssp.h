/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  SSP.H --- SMI DDK 
*  This file contains the definitions for the mode tables.
* 
*******************************************************************/
#ifndef _SSP_H_
#define _SSP_H_

/* ZV Port selection */
typedef enum _ssp_ctrl_t
{
    SSP0 = 0,
    SSP1
}
ssp_ctrl_t;

/* SSP Mode type (Master/Slave device) */
typedef enum _ssp_mode_t
{
    SSP_MODE_MASTER = 0,
    SSP_MODE_SLAVE
}
ssp_mode_t;

/* SSP Format types */
typedef enum _ssp_format_t
{
    SSP_FORMAT_MOTOROLA_SPI = 0,
    SSP_FORMAT_TEXAS_INSTRUMENT,
    SSP_FORMAT_NATIONAL_MICROWIRE
}
ssp_format_t;

/* Motorola SPI Mode */
typedef enum _ssp_spi_mode_t
{
    SSP_SPI_MODE_0 = 0,         /* Phase = rising edge , Polarity = rising edge.  */
    SSP_SPI_MODE_1,             /* Phase = rising edge , Polarity = falling edge. */
    SSP_SPI_MODE_2,             /* Phase = falling edge, Polarity = rising edge.  */
    SSP_SPI_MODE_3              /* Phase = falling edge, Polarity = falling edge. */
}
ssp_spi_mode_t;

/* Data size */
typedef enum _ssp_data_size_t
{
    SSP_DATA_SIZE_4_BIT = 3,
    SSP_DATA_SIZE_5_BIT,
    SSP_DATA_SIZE_6_BIT,
    SSP_DATA_SIZE_7_BIT,
    SSP_DATA_SIZE_8_BIT,
    SSP_DATA_SIZE_9_BIT,
    SSP_DATA_SIZE_10_BIT,
    SSP_DATA_SIZE_11_BIT,
    SSP_DATA_SIZE_12_BIT,
    SSP_DATA_SIZE_13_BIT,
    SSP_DATA_SIZE_14_BIT,
    SSP_DATA_SIZE_15_BIT,
    SSP_DATA_SIZE_16_BIT
}
ssp_data_size_t;

/*************************************************************************
 *
 *  SM750 SSP Interface API
 *
 *************************************************************************/

/*
 * This function get the current index of the SSP
 *
 * Return:
 *      The index of the SSP that is set. 
 */
unsigned char getCurrentSSP();

/*
 * This function get the current index of the SSP
 *
 * Return:
 *      The index of the SSP that is set. 
 */
void setCurrentSSP(
    unsigned char sspIndex
);

/*
 * This function starts the SSP module
 */
void sspStart();

/*
 * This function stops the SSP module
 */
void sspStop();

/*
 * This function checks the SSP busy status.
 *
 * Return:
 *      0   - Not Busy
 *      1   - Busy
 */
unsigned char sspIsBusy();

/*
 * This function gets the SSP Transmit FIFO Status.
 *
 * Return:
 *      0   - Empty
 *      1   - Not Empty
 *      2   - Full
 */
unsigned char sspGetTxFIFOStatus();

/*
 * This function gets the SSP Receive FIFO Status.
 *
 * Return:
 *      0   - Empty
 *      1   - Not Empty
 *      2   - Full
 */
unsigned char sspGetRxFIFOStatus();

/*
 * This function transmits the data to the Transmitter FIFO.
 *
 * Parameters:
 *      pData   - Pointer to the unsigned short array or single variable
 *                that contains data to be transmitted.
 *      numData - Total number of data that should be transmitted.
 *
 * Return:
 *      - Number of data that is actually transmitted.
 */ 
unsigned char sspTransmitData(
    unsigned short *pData, 
    unsigned char numData
);

/*
 * This function gets the data from the Receive FIFO.
 *
 * Parameters:
 *      pData       - Pointer to the unsigned short array or single variable
 *                    to store the data that is received by the Rx FIFO
 *      totalData   - Total number of data that should be retrieved.
 *
 * Return:
 *      - Number of data that is actually retrieved.
 */
unsigned char sspReceiveData(
    unsigned short *pData, 
    unsigned char totalData
);

/* 
 * This function clears the Receive FIFO
 */
void sspClearRxFIFO();

/*
 * This function clears the interrupt status 
 */
void sspClearInterrupt();

/* 
 * This function initializes the SSP
 *
 * Parameter:
 *      masterMode  - SSP Mode
 *                    * Master Device
 *                    * Slave Device
 *      sspFormat   - SSP Format:
 *                    * Motorola SPI format
 *                    * Texas Instruments serial format
 *                    * National Microwire format
 *      dataSize    - Data Size (4 - 16 bits)
 *      spiMode     - Motorola SPI Mode (ignore this parameter for other format)
 *                    * mode0 (PHASE 0, Rising Edge)
 *                    * mode1 (PHASE 0, Falling Edge)
 *                    * mode2 (PHASE 1, Rising Edge)
 *                    * mode3 (PHASE 1, Falling Edge)
 *      bitRate     - The desired SSP bit rate to be set
 *
 * Return:
 *      The actual bit rate that is set.
 */
int32_t sspInit(
    ssp_mode_t masterMode,
    ssp_format_t sspFormat,
    ssp_data_size_t dataSize,
    ssp_spi_mode_t spiMode,
    uint32_t bitRate
);

/* 
 * This function closes the SSP interface
 */
void sspClose();

/*  
 *  hookSSPInterrupt
 *      Register SSP Interrupt function.
 */
unsigned short hookSSPInterrupt(
    ssp_ctrl_t sspControl,
    void (*handler)(void)
);

/*  
 *  unhookSSPInterrupt
 *      unregister SSP Interrupt function.
 */
unsigned short unhookSSPInterrupt(
    ssp_ctrl_t sspControl,
    void (*handler)(void)
);

/*************************************************************************
 *
 *  DRIVER SPECIFIC IMPLEMENTATION API
 *
 *************************************************************************/

/*
 * Detect BIOS Flash Chip
 *
 * Output:
 *      0   - Device found
 *     -1   - Device not found
 */
int32_t sspDetectChip();

/*
 * Reads the device ID 
 */
unsigned short sspReadDeviceID();

/*
 * Wait for the device to be ready
 *
 * Output:
 *      0   - Device is ready
 *     -1   - Device is not ready
 */
int32_t sspWaitDeviceReady();

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
);

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
);

/*
 * Erases a sector
 *
 * Input:
 *      sectorIndex - Sector to be erased
 */
void sspEraseSector(
    unsigned char sectorIndex
);

/*
 * Erases the whole data on the chip
 */
void sspEraseChip();

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
);

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
);

#endif /* _SSP_H_ */

