/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  ssp.c --- Synchronouse Serial Port 
*  This file contains the source code for the SSP0/1.
* 
*******************************************************************/
#include <stdlib.h>
#include "defs.h"
#include "clock.h"
#include "hardware.h"
#include "os.h"
#include "power.h"
#include "ssp.h"

#include "ddkdebug.h"

/* Different register offset location between the first and the second SSP */
#define SSP_OFFSET_GAP              0x100

/* Transmit Timeout */
#define TX_TIMEOUT                  1000

typedef struct _ssp_interrupt_t
{
    struct _ssp_interrupt_t *next;
    uint32_t sspIntMask;
    void (*handler)(void);
}
ssp_interrupt_t;

static ssp_interrupt_t *ssp0IntHandlers = ((ssp_interrupt_t *)0);
static ssp_interrupt_t *ssp1IntHandlers = ((ssp_interrupt_t *)0);

static unsigned char gCurrentSSPIndex = 0;

/*
 *  getSSPRegisterOffset
 *      This function gets the offset of the current SSP register.
 *
 *  Input:
 *      Register to be looked at.
 */
static uint32_t getSSPRegisterOffset(
    uint32_t ulRegIndex
)
{
    uint32_t ulRegOffset;
    
    /* Get the register offset */
    if (gCurrentSSPIndex == 0)
        ulRegOffset = 0;
    else
        ulRegOffset = SSP_OFFSET_GAP;
        
    return (ulRegIndex + ulRegOffset);
}

/*
 * This function get the current index of the SSP
 *
 * Return:
 *      The index of the SSP that is set. 
 */
unsigned char getCurrentSSP()
{
    return gCurrentSSPIndex;
}

/*
 * This function get the current index of the SSP
 *
 * Return:
 *      The index of the SSP that is set. 
 */
void setCurrentSSP(
    unsigned char sspIndex
)
{
    if (sspIndex < 2)
        gCurrentSSPIndex = sspIndex;
}

/*
 * This function starts the SSP module
 */
void sspStart()
{
    unsigned char value;
    
    value = peekRegisterByte(getSSPRegisterOffset(SSP_0_CONTROL_1));
    pokeRegisterByte(getSSPRegisterOffset(SSP_0_CONTROL_1), 
                     FIELD_SET(value, SSP_0_CONTROL_1, STATUS, ENABLE));
}

/*
 * This function stops the SSP module
 */
void sspStop()
{
    unsigned char value;
    
    value = peekRegisterByte(getSSPRegisterOffset(SSP_0_CONTROL_1));
    pokeRegisterByte(getSSPRegisterOffset(SSP_0_CONTROL_1), 
                     FIELD_SET(value, SSP_0_CONTROL_1, STATUS, DISABLE));
}

/*
 * This function checks the SSP busy status.
 *
 * Return:
 *      0   - Not Busy
 *      1   - Busy
 */
unsigned char sspIsBusy()
{
    return ((FIELD_GET(peekRegisterByte(getSSPRegisterOffset(SSP_0_STATUS)),
                      SSP_0_STATUS,
                      STATUS) == SSP_0_STATUS_STATUS_BUSY) ? 1 : 0);
}

/*
 * This function gets the SSP Transmit FIFO Status.
 *
 * Return:
 *      0   - Empty
 *      1   - Not Empty
 *      2   - Full
 */
unsigned char sspGetTxFIFOStatus()
{
    unsigned char txStatus;
    
    /* Get the status of the TX FIFO */
    txStatus = FIELD_GET(peekRegisterByte(getSSPRegisterOffset(SSP_0_STATUS)), 
                         SSP_0_STATUS, 
                         TRANSMIT_FIFO);
    
    switch (txStatus)
    {
        default:
        case SSP_0_STATUS_TRANSMIT_FIFO_EMPTY:
            return 0;
        case SSP_0_STATUS_TRANSMIT_FIFO_NOT_FULL:
            return 1;
        case SSP_0_STATUS_TRANSMIT_FIFO_FULL:
            return 2;
    }
}

/*
 * This function gets the SSP Receive FIFO Status.
 *
 * Return:
 *      0   - Empty
 *      1   - Not Empty
 *      2   - Full
 */
unsigned char sspGetRxFIFOStatus()
{
   unsigned char rxStatus;
    
    /* Get the status of the TX FIFO */
    rxStatus = FIELD_GET(peekRegisterByte(getSSPRegisterOffset(SSP_0_STATUS)), 
                         SSP_0_STATUS, 
                         RECEIVE_FIFO);
    
    switch (rxStatus)
    {
        default:
        case SSP_0_STATUS_RECEIVE_FIFO_EMPTY:
            return 0;
        case SSP_0_STATUS_RECEIVE_FIFO_NOT_EMPTY:
            return 1;
        case SSP_0_STATUS_RECEIVE_FIFO_FULL:
            return 2;
    }
}

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
 *        The value is 0 when it fails to transmit.
 */ 
unsigned char sspTransmitData(
    unsigned short *pData, 
    unsigned char numData
)
{
    unsigned char totalData, index;
    uint32_t timeout = TX_TIMEOUT;

    /* Transmit data every 8 at a times since the FIFO depth is 8. */
    while (numData)
    {
        totalData = (numData < 8) ? numData : 8;
    
        sspStop();
    
        for (index = 0; index < totalData; index++)
            pokeRegisterWord(getSSPRegisterOffset(SSP_0_DATA), pData[index]);
        
        sspStart();
        
        /* Adjust pointer and data counter */
        pData+=totalData;
        numData-=totalData;
    }    
    
    /* Check for its completion */
    timeout = TX_TIMEOUT;
    while (timeout--)
    {
        /* Check until it is not busy and the transmit FIFO is empty */
        if ((sspIsBusy() == 0) && (sspGetTxFIFOStatus() == 0))
            break;
    }
    
    if (timeout == 0)
    {
        DDKDEBUGPRINT((ERROR_LEVEL, "SSP Transmit Error.\n"));
        totalData = 0;
    }
        
    return totalData;
}

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
)
{
    unsigned char dataCount = 0;
    unsigned short value;
    
    /* Get the data with the maximum number of the specified numData. */
    while (1)
    {
        /* Check if the total number of data requested are already read.
           A totalData value of (-1) means that the data will be read until
           there is no data in the Receive FIFO. */
        if ((totalData != (unsigned char)(-1)) && (dataCount >= totalData))
            break;
            
        /* Get the data only when the Receive FIFO status is not empty.*/
        if (sspGetRxFIFOStatus() == 0)
            break;

        value = peekRegisterWord(getSSPRegisterOffset(SSP_0_DATA));
        if (pData != (unsigned short *)0)
        {
            *pData = value;
            pData++;
        }
        
        /* Increment the data count */
        dataCount++;
    }
    
    return dataCount;
}

/* 
 * This function clears the Receive FIFO
 */
void sspClearRxFIFO()
{
    /* Start the SSP */
    sspStart();
    
    /* Clear the FIFO */
    sspReceiveData((unsigned short *)0, (unsigned char) (-1));
    
    /* Stop SSP */
    sspStop();
}

/*
 * This function clears the interrupt status 
 */
void sspClearInterrupt()
{
    /* Clear the interrupt status*/
    pokeRegisterByte(getSSPRegisterOffset(SSP_0_INTERRUPT_STATUS), 0);
}

/* 
 * This function sets the SSP Bit Rate
 *
 * Parameter:
 *      desiredBitRate  - Desired bit rate to be set
 *
 * Return:
 *      The actual bit rate that is set.
 */
uint32_t sspCalculateBitRate(
    uint32_t desiredBitRate,
    unsigned char *pSerialClockRate,
    unsigned char *pClockPrescale
)
{
    uint32_t inputClock, difference, calculatedBitRate;
    uint32_t bestDifference = 0xffffffff;
    uint32_t serialClockRate, clockPrescale, bestSCR, bestPrescale;
    uint32_t value;

    /* Get the SSP input clock, which is the master clock */
    inputClock = getMasterClock();
    
    /* Calculate the best possible value. 
       The clock divider / clock prescaler is an even value from 2 to 254. */
    for (clockPrescale = 2; clockPrescale < 256; clockPrescale+=2)
    {
        /* The serial clock rate  is the value between 0 to 255. */
        for (serialClockRate = 0; serialClockRate < 256; serialClockRate++)
        {
            calculatedBitRate = inputClock / (clockPrescale * (1 + serialClockRate));
            
            difference = absDiff(calculatedBitRate, desiredBitRate); 
             
            /* If the difference is less than the current, use it. */
            if (difference < bestDifference)
            {
                /* Store best difference. */
                bestDifference = difference;

                /*  Store the frequency clock values based on the best difference. */
                bestPrescale = clockPrescale;
                bestSCR = serialClockRate;
            }
        }
    }
    
    if (pSerialClockRate != (unsigned char *) 0)
        *pSerialClockRate = bestSCR;
    
    if (pClockPrescale != (unsigned char *) 0)
        *pClockPrescale = bestPrescale;
    
    return (inputClock / (bestPrescale * (1 + bestSCR))); 
}

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
)
{
    uint32_t value = 0;
    unsigned char scrValue, prescaleValue;
    
    /***************************************
     * 1. Check all the parameters validity
     ***************************************/
     
    /* Check the ssp format to be set */
    if (sspFormat > SSP_FORMAT_NATIONAL_MICROWIRE)
    {
        DDKDEBUGPRINT((ERROR_LEVEL, "Unsupported SSP Format.\n"));
        return -1;
    }
        
    /* Check the data size to be set */
    if (dataSize > SSP_DATA_SIZE_16_BIT)
    {
        DDKDEBUGPRINT((ERROR_LEVEL, "Unsupported SSP data size.\n"));
        return -1;
    }
        
    /* Check the Motorola SPI mode */
    if ((sspFormat == SSP_FORMAT_MOTOROLA_SPI) && (spiMode > SSP_SPI_MODE_3))
    {
        DDKDEBUGPRINT((ERROR_LEVEL, "Unsupported SPI mode.\n"));
        return -1;
    }
    
    /***************************************
     * 2. Enable the SSP Clock and Power
     ***************************************/
    enableSSP(1);
    
    /***************************************
     * 3. Set SSP Control 0 Register
     ***************************************/
     
    /* Calculate the bit Rate */
    sspCalculateBitRate(bitRate, &scrValue, &prescaleValue);
    
    /* Set the serial clock rate (SCR) */
    value = FIELD_VALUE(0, SSP_0_CONTROL_0, CLOCK_RATE, scrValue);
    
    /* Set the Frame format and/or SCLKOUT phase and polarity */
    switch(sspFormat)
    {
        case SSP_FORMAT_MOTOROLA_SPI:
            /* Motorola SPI Frame Format */
            value = FIELD_SET(value, SSP_0_CONTROL_0, FRAME_FORMAT, MOTOROLA);
            
            /* Set the Serial clock out phase and polarity. This setting only
               applies to Motorola SPI frame format */
            switch(spiMode)
            {
                case SSP_SPI_MODE_0:
                    /* Mode 0: Phase = 0, Polarity = Rising edge */
                    value = FIELD_SET(value, SSP_0_CONTROL_0, SCLKOUT_PHASE, 0);
                    value = FIELD_SET(value, SSP_0_CONTROL_0, SCLKOUT_POLARITY, RISING);
                    break;
                case SSP_SPI_MODE_1:
                    /* Mode 1: Phase = 0, Polarity = Falling edge */
                    value = FIELD_SET(value, SSP_0_CONTROL_0, SCLKOUT_PHASE, 0);
                    value = FIELD_SET(value, SSP_0_CONTROL_0, SCLKOUT_POLARITY, FALLING);
                    break;
                case SSP_SPI_MODE_2:
                    /* Mode 2: Phase = 1, Polarity = Rising edge */
                    value = FIELD_SET(value, SSP_0_CONTROL_0, SCLKOUT_PHASE, 1);
                    value = FIELD_SET(value, SSP_0_CONTROL_0, SCLKOUT_POLARITY, RISING);
                    break;
                case SSP_SPI_MODE_3:
                    /* Mode 3: Phase = 1, Polarity = Falling edge */
                    value = FIELD_SET(value, SSP_0_CONTROL_0, SCLKOUT_PHASE, 1);
                    value = FIELD_SET(value, SSP_0_CONTROL_0, SCLKOUT_POLARITY, FALLING);
                    break; 
            }
            break;
        case SSP_FORMAT_TEXAS_INSTRUMENT:
            /* Texas Instruments serial frame format */
            value = FIELD_SET(value, SSP_0_CONTROL_0, FRAME_FORMAT, TI);
            break;
        case SSP_FORMAT_NATIONAL_MICROWIRE:
            /* National Microwire frame format */
            value = FIELD_SET(value, SSP_0_CONTROL_0, FRAME_FORMAT, NATIONAL);
            break;
    }
    
    /* Set the data size. */
    value = FIELD_VALUE(value, SSP_0_CONTROL_0, DATA_SIZE, dataSize);
    pokeRegisterDWord(SSP_0_CONTROL_0, value);
    
    /***************************************
     * 4. Set SSP Control 1 Register
     ***************************************/
     
    /* Set the SSP mode (master or slave) */
    if (masterMode == SSP_MODE_MASTER)
        value = FIELD_SET(0, SSP_0_CONTROL_1, MODE_SELECT, MASTER);
    else
        value = FIELD_SET(0, SSP_0_CONTROL_1, MODE_SELECT, SLAVE);

    pokeRegisterByte(SSP_0_CONTROL_1, value);
    
    /***************************************
     * 5. Set SSP Clock Prescale Register
     ***************************************/
    
    /* Set the clock prescale */
    value = FIELD_VALUE(0, SSP_0_CLOCK_PRESCALE, DIVISOR, prescaleValue);
    pokeRegisterDWord(getSSPRegisterOffset(SSP_0_CLOCK_PRESCALE), value);
    
    /***********************************************
     * 6. Set the corresponding GPIO pin as SSP pin
     ***********************************************/    

    /* Enable the SSP pins */
    value = peekRegisterDWord(GPIO_MUX);
    if (gCurrentSSPIndex == 0)
    {
        /* Select SSP 0 Pins */
        pokeRegisterDWord(GPIO_MUX,                                            
            FIELD_SET(value, GPIO_MUX, 24, SSP0) |              
            FIELD_SET(value, GPIO_MUX, 23, SSP0) |              
            FIELD_SET(value, GPIO_MUX, 22, SSP0) |              
            FIELD_SET(value, GPIO_MUX, 21, SSP0) |              
            FIELD_SET(value, GPIO_MUX, 20, SSP0));
    }
    else
    {
        /* Select SSP 1 Pins */
        pokeRegisterDWord(GPIO_MUX,                                            
            FIELD_SET(value, GPIO_MUX, 29, SSP1) |              
            FIELD_SET(value, GPIO_MUX, 28, SSP1) |              
            FIELD_SET(value, GPIO_MUX, 27, SSP1) |              
            FIELD_SET(value, GPIO_MUX, 26, SSP1) |              
            FIELD_SET(value, GPIO_MUX, 25, SSP1) );
    }
    
    /* Success */
    return 0;
}

/* 
 * This function closes the SSP interface
 */
void sspClose()
{
    uint32_t value;
    
    /* Disable SSP Interface. */
    pokeRegisterByte(getSSPRegisterOffset(SSP_0_CONTROL_1), 0);
    
    value = peekRegisterDWord(GPIO_MUX);
    if (gCurrentSSPIndex == 0)
    {
        /* Select SSP 0 Pins */
        pokeRegisterDWord(GPIO_MUX,                                            
            FIELD_SET(value, GPIO_MUX, 24, GPIO) |              
            FIELD_SET(value, GPIO_MUX, 23, GPIO) |              
            FIELD_SET(value, GPIO_MUX, 22, GPIO) |              
            FIELD_SET(value, GPIO_MUX, 21, GPIO) |              
            FIELD_SET(value, GPIO_MUX, 20, GPIO));
    }
    else
    {
        /* Select SSP 1 Pins */
        pokeRegisterDWord(GPIO_MUX,                                            
            FIELD_SET(value, GPIO_MUX, 29, GPIO) |              
            FIELD_SET(value, GPIO_MUX, 28, GPIO) |              
            FIELD_SET(value, GPIO_MUX, 27, GPIO) |              
            FIELD_SET(value, GPIO_MUX, 26, GPIO) |              
            FIELD_SET(value, GPIO_MUX, 25, GPIO) );
    } 
}

/*
 * This ISR function calls the function that hooks to this SSP interrupt 
 */
void ssp0ISR(
    uint32_t status
)
{
    ssp_interrupt_t *pfnHandler;
    
    if (FIELD_GET(status, INT_STATUS, SSP0) == INT_STATUS_SSP0_ACTIVE)
    {
        /* Walk all registered handlers for handlers that support this interrupt status 
           The function hooked to this ISR needs to check the condition of the interrupt
           and do accordingly to reset the interrupt. Like reading the data when the
           receive FIFO is full or transmit the data when the transmit FIFO is full.
         */
        for (pfnHandler = ssp0IntHandlers; pfnHandler != ((ssp_interrupt_t *)0); pfnHandler = pfnHandler->next)
            pfnHandler->handler();            
    }            
}

/*
 * This ISR function calls the function that hooks to this SSP interrupt 
 */
void ssp1ISR(
    uint32_t status
)
{
    ssp_interrupt_t *pfnHandler;
    
    if (FIELD_GET(status, INT_STATUS, SSP1) == INT_STATUS_SSP1_ACTIVE)
    {
        /* Walk all registered handlers for handlers that support this interrupt status 
           The function hooked to this ISR needs to check the condition of the interrupt
           and do accordingly to reset the interrupt. Like reading the data when the
           receive FIFO is full or transmit the data when the transmit FIFO is full.
         */
        for (pfnHandler = ssp1IntHandlers; pfnHandler != ((ssp_interrupt_t *)0); pfnHandler = pfnHandler->next)
            pfnHandler->handler();
    }            
}

/*  
 *  hookSSPInterrupt
 *      Register SSP Interrupt function.
 */
unsigned short hookSSPInterrupt(
    ssp_ctrl_t sspControl,
    void (*handler)(void)
)
{
    ssp_interrupt_t *pfnNewHandler, *pfnHandler, *pfnInterruptHandler;
    unsigned short returnValue = 0;

    /* Allocate a new interrupt structure */
    pfnNewHandler = (ssp_interrupt_t *)malloc(sizeof(ssp_interrupt_t));
    if (pfnNewHandler == ((ssp_interrupt_t *)0))
    {
        /* No more memory */
        return (-1);
    }
    
    /* Get the right pointer to the interrupt handler */
    if (sspControl == SSP1)
        pfnInterruptHandler = ssp1IntHandlers;
    else
        pfnInterruptHandler = ssp0IntHandlers;

    /* Make sure that it has not been register more than once */
    for (pfnHandler = pfnInterruptHandler; pfnHandler != ((ssp_interrupt_t *)0); pfnHandler = pfnHandler->next)
    {
        if (pfnHandler->handler == handler)
            return (-1);                   /* Duplicate entry */
    }
        
    /* If this is the first interrupt handler, register this ZV Port ISR */
    if (pfnInterruptHandler == ((ssp_interrupt_t *)0))
    {
        returnValue = registerHandler((sspControl == SSP1) ? ssp1ISR : ssp0ISR, 
                                      (sspControl == SSP1) ? 
                                          FIELD_SET(0, INT_MASK, SSP1, ENABLE) :
                                          FIELD_SET(0, INT_MASK, SSP0, ENABLE));
    }

    if (returnValue == 0)
    {
        /* Fill interrupt structure. */
        pfnNewHandler->next = pfnInterruptHandler;
        pfnNewHandler->handler = handler;
        pfnInterruptHandler = pfnNewHandler;
    }
    
    /* Update the actual handler */
    if (sspControl == SSP1)
        ssp1IntHandlers = pfnInterruptHandler;
    else
        ssp0IntHandlers = pfnInterruptHandler;
    
    return returnValue;
}

/*  
 *  unhookSSPInterrupt
 *      unregister SSP Interrupt function.
 */
unsigned short unhookSSPInterrupt(
    ssp_ctrl_t sspControl,
    void (*handler)(void)
)
{
    ssp_interrupt_t *pfnHandler, *prev, *pfnInterruptHandler;
    uint32_t mask;
    
    if (sspControl == SSP1)
        pfnInterruptHandler = ssp1IntHandlers;
    else
        pfnInterruptHandler = ssp0IntHandlers;

    /* Find the requested handle to unregister */
    for (pfnHandler = pfnInterruptHandler, prev = ((ssp_interrupt_t *)0); 
         pfnHandler != ((ssp_interrupt_t *)0); 
         prev = pfnHandler, pfnHandler = pfnHandler->next)
    {
        if (pfnHandler->handler == handler)
        {
            /* Remove the interrupt handler */
            if (prev == ((ssp_interrupt_t *)0))
            {
                pfnInterruptHandler = pfnHandler->next;
                
                /* Update the actual handler */
                if (sspControl == SSP1)
                    ssp1IntHandlers = pfnInterruptHandler;
                else
                    ssp0IntHandlers = pfnInterruptHandler;
            }
            else
            {
                prev->next = pfnHandler->next;
            }
            free(pfnHandler);
            
            /* If this was the last interrupt handler, remove the IRQ handler */
            if (pfnInterruptHandler == ((ssp_interrupt_t *)0))
            {
                unregisterHandler((sspControl == SSP1) ? ssp1ISR : ssp0ISR);
            }
            
            /* Success */
            return (0);
        }
    }

    /* Oops, handler is not registered */
    return (-1);
}
