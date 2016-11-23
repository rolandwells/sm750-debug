/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  DMA.C --- Voyager GX SDK 
*  This file contains the definitions for the DMA functions.
* 
*******************************************************************/
#include "defs.h"
#include "dma.h"
#include "hardware.h"
#include "power.h"
#include "ddkdebug.h"

/*
 * This function uses DMA channel 1 to transfer data between SDRAM and SDRAM.
 * 
 * Three possibility:
 * 1) Video memory to system memory.
 * 2) Video memory to video memory.
 * 3) System memory to video memory.
 *
 * Input: 
 *      srcAddr     - Source address (either system or video memory)
 *      srcExt      - Source memory address selection, 0 = video and 1 = system
 *      destAddr    - Destination address (either system or video memory)
 *      destExt     - Destination memory address selection, 0 = video and 1 = system
 *      length      - Transfer length in BYTE (max = 16M)
 *
 * Return: 
 *       0  - Success
 *      -1  - Fail
 *
 * Note:
 *      1. Accessing the system memory in PCI device requires SM50x to act as the 
 *         PCI bus master. Therefore, the PCI Master and PCI Clock run has to be 
 *         started and enabled. These two bits are located in the System Control 
 *         Register (0x000000).
 *      2. The source and the destination address is limited to 2^26 (64MB), 
 *         which means that it can not access the whole 4GB system memory space
 *         without setting the base address. This base address is located in 
 *         the PCI Master Base register (0x000050).
 */
int32_t dmaChannel(
    uint32_t srcAddr,
    unsigned char srcExt,
    uint32_t destAddr,
    unsigned char destExt,
    uint32_t length
)
{  
    uint32_t value, timeout;
    uint32_t oldSystemControl, pciMasterBaseAddress;
    int32_t returnValue = 0;

    /* Sanity check */
    if (length > MAX_DMA_SDRAM_LEN)
    {
        DDKDEBUGPRINT((ERROR_LEVEL, "Transfer Length exceeds 16 MB.\n"));
        return -1;
    }
    
    /* The SDRAM Address, SRAM address, and the length have to be 128-bit aligned. 
       Return error if not aligned. */
    if ((srcAddr & 0x0F) | (destAddr & 0x0F) | (length & 0x0F))
    {
        DDKDEBUGPRINT((ERROR_LEVEL, "The source address, destination address, and length have to be 128-bit aligned.\n"));
        return -1;
    }
        
    /* DMA is not supported between System to System. */
    if ((srcExt == 1) && (destExt == 1))
    {
        DDKDEBUGPRINT((ERROR_LEVEL, "System to System DMA transfer is not supported\n"));
        return (-1);
    }
    
    /* Enable DMA engine */    
    enableDMA(1);
        
    /* Abort previous DMA and enable it back */
    value = peekRegisterDWord(DMA_ABORT_INTERRUPT);
    value = FIELD_SET(value, DMA_ABORT_INTERRUPT, ABORT_1, ABORT);
    pokeRegisterDWord(DMA_ABORT_INTERRUPT, value);
    value = FIELD_SET(value, DMA_ABORT_INTERRUPT, ABORT_1, ENABLE);
    pokeRegisterDWord(DMA_ABORT_INTERRUPT, value);    
            
    /* Save the system Control register value */
    oldSystemControl = peekRegisterDWord(SYSTEM_CTRL);
    
    /* Update the system control register if necessary. The PCI Master and clock run
       need to be started and enabled when accessing system memory. */
    if ((srcExt == 1) || (destExt == 1))
    {
        pokeRegisterDWord(SYSTEM_CTRL, FIELD_SET(oldSystemControl, SYSTEM_CTRL, PCI_MASTER, ON));
        DDKDEBUGPRINT((DMA_LEVEL, "PCI Control in System Control Register: %x\n", peekRegisterDWord(SYSTEM_CTRL)));
    }

    /* Set Source DMA Address */
    value = FIELD_VALUE(0, DMA_1_SOURCE, ADDRESS, srcAddr);
    if (srcExt == 1)
    {
        /* Set the source PCI Master base address.  */
#if 1 /*def VALIDATION_CHIP*/
        pciMasterBaseAddress = ((srcAddr & 0xFFF00000) >> 23) << 3;
        
        /* This errata only applies to System Memory. For local to local, no correction is
           needed. */
        value = ((value & 0x00700000) << 3) + (value & 0x000FFFFF);
#else        
        pciMasterBaseAddress = (srcAddr & 0xFF000000) >> 24;
#endif
        pokeRegisterDWord(PCI_MASTER_BASE, FIELD_VALUE(0, PCI_MASTER_BASE, ADDRESS, pciMasterBaseAddress));
        DDKDEBUGPRINT((DMA_LEVEL, "Actual Source PCI_MASTER_BASE Address: %x\n", peekRegisterDWord(PCI_MASTER_BASE)));

        /* Set the external flag */                    
        value = FIELD_SET(value, DMA_1_SOURCE, ADDRESS_EXT, EXTERNAL);
    }
    pokeRegisterDWord(DMA_1_SOURCE, value);
    DDKDEBUGPRINT((DMA_LEVEL, "DMA_1_SOURCE Address: %x\n", peekRegisterDWord(DMA_1_SOURCE)));

    /* Set DMA Destination address */
    value = FIELD_VALUE(0, DMA_1_DESTINATION, ADDRESS, destAddr);
    if (destExt == 1)
    {
        /* Set the destination PCI Master base address. */
#if 1 /*def VALIDATION_CHIP*/
        pciMasterBaseAddress = ((destAddr & 0xFFF00000) >> 23) << 3;
        
        /* This errata only applies to System Memory. For local to local, no correction is
           needed. */
        value = ((value & 0x00700000) << 3) + (value & 0x000FFFFF);
#else
        pciMasterBaseAddress = (destAddr & 0xFF000000) >> 24;
#endif
        pokeRegisterDWord(PCI_MASTER_BASE, FIELD_VALUE(0, PCI_MASTER_BASE, ADDRESS, pciMasterBaseAddress));
        DDKDEBUGPRINT((DMA_LEVEL, "Actual Destination PCI_MASTER_BASE Address: %x\n", peekRegisterDWord(PCI_MASTER_BASE)));
        
        value = FIELD_SET(value, DMA_1_DESTINATION, ADDRESS_EXT, EXTERNAL);
    }
    pokeRegisterDWord(DMA_1_DESTINATION, value);
    DDKDEBUGPRINT((DMA_LEVEL, "DMA_1_DESTINATION Address: %x\n", peekRegisterDWord(DMA_1_DESTINATION)));

    /* Set DMA Size to be transferred. */
    value = FIELD_VALUE(0, DMA_1_SIZE_CONTROL, SIZE, length);
    pokeRegisterDWord(DMA_1_SIZE_CONTROL, value);
    DDKDEBUGPRINT((DMA_LEVEL, "DMA_1_SIZE_CONTROL Address: %x\n", peekRegisterDWord(DMA_1_SIZE_CONTROL)));

    /* Start the DMA process */
    value = FIELD_SET(value, DMA_1_SIZE_CONTROL, STATUS, ACTIVE);
    pokeRegisterDWord(DMA_1_SIZE_CONTROL, value);

    /* Wait until the DMA has finished. */
    timeout = 0x1000000; 
    do
    {
        value = peekRegisterDWord(DMA_1_SIZE_CONTROL);
        value = FIELD_GET(value, DMA_1_SIZE_CONTROL, STATUS);
        timeout--;
    } while ((value == DMA_1_SIZE_CONTROL_STATUS_ACTIVE) && (timeout));
    
    /* Timeout. Something wrong with the DMA engine */
    if (timeout == 0)
    {
        DDKDEBUGPRINT((ERROR_LEVEL, "Timeout waiting for DMA finish.\n"));
        returnValue = -1;
    }
   
    /* Clear DMA Interrupt Status */
    value = peekRegisterDWord(DMA_ABORT_INTERRUPT);
    value = FIELD_SET(value, DMA_ABORT_INTERRUPT, INT_1, CLEAR);
    pokeRegisterDWord(DMA_ABORT_INTERRUPT, value);
    
    /* Disable the DMA engine after finish. */
    enableDMA(0);

    /* Restore back the system control register if it is not the same 
       value as when it enters the function. */
    if (oldSystemControl != peekRegisterDWord(SYSTEM_CTRL))
        pokeRegisterDWord(SYSTEM_CTRL, oldSystemControl);
    
    return (returnValue);
}     

