/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  CHIP.C --- SMI DDK 
*  This file contains the source code for the mode table.
* 
*******************************************************************/
#include "defs.h"
#include "hardware.h"
#include "power.h"
#include "clock.h"
#include "chip.h"
#include "os.h"

#include "ddkdebug.h"

#define MB(x) (x*0x100000) /* Don't use this macro if x is fraction number */

/* Memory Clock Default Value. It can be calculated by dividing 
   the chip clock by certain valid values, which are: 1, 2, 3, and 4 */
#define DEFAULT_MEMORY_CLK          290

static logical_chip_type_t gLogicalChipType = SM_UNKNOWN;

/*
 * This function returns frame buffer memory size in Byte units.
 */
_X_EXPORT uint32_t getFrameBufSize()
{
    uint32_t sizeSymbol, memSize;

    sizeSymbol = FIELD_GET(peekRegisterDWord(MISC_CTRL), MISC_CTRL, LOCALMEM_SIZE);

    switch(sizeSymbol)
    {
        case MISC_CTRL_LOCALMEM_SIZE_8M:  memSize = MB(8);  break; /* 8  Mega byte */
        case MISC_CTRL_LOCALMEM_SIZE_16M: memSize = MB(16); break; /* 16 Mega byte */
        case MISC_CTRL_LOCALMEM_SIZE_32M: memSize = MB(32); break; /* 32 Mega byte */
        case MISC_CTRL_LOCALMEM_SIZE_64M: memSize = MB(64); break; /* 64 Mega byte */
        default:                          memSize = MB(0);  break; /* 0  Mege byte */
    }

    return memSize;
}

/*
 * This function gets the Frame buffer location.
 *
 * Return:
 *      0x00    - 32-bit
 *      0x01    - 64-bit
 */
unsigned char getMemoryBusWidth()
{
    /* Check Memory Bus */
    if (FIELD_GET(peekRegisterDWord(MISC_CTRL), MISC_CTRL, LOCALMEM_BUS_SIZE) == MISC_CTRL_LOCALMEM_BUS_SIZE_64)
        return 0x01;
    else
        return 0x00;
}

/*
 * This function gets the Frame buffer location.
 *
 * Return:
 *      0x01    - Embedded
 *      0x02    - External
 *      0x03    - Both Embedded and External
 */
unsigned char getFrameBufLocation()
{
    unsigned char memoryLocation = 0;
    
    /* Check if the memory is external memory */
    if (FIELD_GET(peekRegisterDWord(MISC_CTRL), MISC_CTRL, EMBEDDED_LOCALMEM) == MISC_CTRL_EMBEDDED_LOCALMEM_ON)
    {
        memoryLocation |= 0x01; /* Internal Memory */
        
        /* External memory with 64-bit data bus */
        if (getMemoryBusWidth() == 0x01)
            memoryLocation |= 0x02;
    }
    else
    {
        memoryLocation |= 0x02; /* External Memory */
        
    }

    return memoryLocation;
}

/*
 * This function returns the logical chip type defined in chip.h
 * It is one of the following: SM501, SM502, SM107, SM718, SM 750 or
 * SM_UNKNOWN.
 */
logical_chip_type_t getChipType()
{
    uint32_t physicalID, physicalRev;
    logical_chip_type_t chip;

    physicalID =  FIELD_GET(peekRegisterDWord(DEVICE_ID), DEVICE_ID, DEVICE_ID);
    physicalRev = FIELD_GET(peekRegisterDWord(DEVICE_ID), DEVICE_ID, REVISION_ID);
    
    DDKDEBUGPRINT((INIT_LEVEL, "Physical ID: %04x, Revision: %04x\n", physicalID, physicalRev));

    if (physicalID == SMI_DEVICE_ID_SM718)
    {
        chip = SM718;
    }
    else if (physicalID == SMI_DEVICE_ID_SM750)
    {
        chip = SM750;
    }
    else
    {
        chip = SM_UNKNOWN;
    }

    return chip;
}

/*
 * Return a char string name of the current chip.
 * It's convenient for application need to display the chip name.
 */
char * getChipTypeString()
{
    char * chipName;

    switch(getChipType())
    {
        case SM718:
            chipName = "SM718";
            break;
        case SM750:
            chipName = "SM750";
            break;
        default:
            chipName = "Unknown";
            break;
    }

    return chipName;
}

/*
 * Initialize a single chip and environment according to input parameters.
 *
 * Input: initchip_param_t structure.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 *
 * Note:
 *      Caller needs to call the detectDevice and setCurrentDevice
 *      to set the device before calling this initChipParamEx.
 */
int32_t initChipParamEx(initchip_param_t * pInitParam)
{
    uint32_t ulReg;
    unsigned short deviceId;

    /* Check if we know this chip */
    if ((gLogicalChipType = getChipType()) == SM_UNKNOWN)
        return -1;

    /* Set power mode.
       Check parameter validity first.
       If calling function didn't set it up properly or set to some
       weird value, always default it to 0.
     */
    if (pInitParam->powerMode > 1) 
        pInitParam->powerMode = 0;
    setPowerMode(pInitParam->powerMode);
    
    /* Set the Main Chip Clock */
    setChipClock(MHz(pInitParam->chipClock));

    /* Set up memory clock. */
    setMemoryClock(MHz(pInitParam->memClock));

    /* Set up master clock */
    setMasterClock(MHz(pInitParam->masterClock));    
    
    /* Reset the memory controller. If the memory controller is not reset in SM750, 
       the system might hang when sw accesses the memory. 
       The memory should be resetted after changing the MXCLK.
     */
    if (pInitParam->resetMemory == 1)
    {
        DDKDEBUGPRINT((INIT_LEVEL, "Resetting Memory\n"));
        
        ulReg = peekRegisterDWord(MISC_CTRL);
        ulReg = FIELD_SET(ulReg, MISC_CTRL, LOCALMEM_RESET, RESET);
        pokeRegisterDWord(MISC_CTRL, ulReg);
    
        ulReg = FIELD_SET(ulReg, MISC_CTRL, LOCALMEM_RESET, NORMAL);
        pokeRegisterDWord(MISC_CTRL, ulReg);    
    }
    
    if (pInitParam->setAllEngOff == 1)
    {
        enable2DEngine(0);

        /* Disable Overlay, if a former application left it on */
        ulReg = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, VIDEO_DISPLAY_CTRL, PLANE, DISABLE); 
        pokeRegisterDWord(VIDEO_DISPLAY_CTRL, ulReg);

        /* Disable video alpha, if a former application left it on */
        ulReg = peekRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, VIDEO_ALPHA_DISPLAY_CTRL, PLANE, DISABLE); 
        pokeRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL, ulReg);

        /* Disable alpha plane, if a former application left it on */
        ulReg = peekRegisterDWord(ALPHA_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, ALPHA_DISPLAY_CTRL, PLANE, DISABLE); 
        pokeRegisterDWord(ALPHA_DISPLAY_CTRL, ulReg);

        /* Disable Primary hardware cursor, if a former application left it on */
        ulReg = peekRegisterDWord(PRIMARY_HWC_ADDRESS);
        ulReg = FIELD_SET(ulReg, PRIMARY_HWC_ADDRESS, ENABLE, DISABLE); 
        pokeRegisterDWord(PRIMARY_HWC_ADDRESS, ulReg);

        /* Disable Secondary hardware cursor, if a former application left it on */
        ulReg = peekRegisterDWord(SECONDARY_HWC_ADDRESS);
        ulReg = FIELD_SET(ulReg, SECONDARY_HWC_ADDRESS, ENABLE, DISABLE); 
        pokeRegisterDWord(SECONDARY_HWC_ADDRESS, ulReg);

        /* Disable ZV Port 0, if a former application left it on */
        ulReg = peekRegisterDWord(ZV0_CAPTURE_CTRL);
        ulReg = FIELD_SET(ulReg, ZV0_CAPTURE_CTRL, CAP, DISABLE); 
        pokeRegisterDWord(ZV0_CAPTURE_CTRL, ulReg);

        /* Disable ZV Port 1, if a former application left it on */
        ulReg = peekRegisterDWord(ZV1_CAPTURE_CTRL);
        ulReg = FIELD_SET(ulReg, ZV1_CAPTURE_CTRL, CAP, DISABLE); 
        pokeRegisterDWord(ZV1_CAPTURE_CTRL, ulReg);
        
        /* Disable ZV Port Power, if a former application left it on */
        enableZVPort(0);
        
        /* Disable i2c */
        enableI2C(0);
        
        /* Disable DMA Channel, if a former application left it on */
        ulReg = peekRegisterDWord(DMA_ABORT_INTERRUPT);
        ulReg = FIELD_SET(ulReg, DMA_ABORT_INTERRUPT, ABORT_1, ABORT);
        pokeRegisterDWord(DMA_ABORT_INTERRUPT, ulReg);
        
        /* Disable DMA Power, if a former application left it on */
        enableDMA(0);
    }

    /* We can add more initialization as needed. */
        
    return 0;
}

/*
 * Initialize every chip and environment according to input parameters. 
 * (Obsolete)
 *
 * Input: initchip_param_t structure.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 */
int32_t initChipParam(initchip_param_t * pInitParam)
{
    unsigned short deviceId, prevDevice;
    unsigned char deviceIndex;
    int32_t result = 0;

    /* Check if any SMI VGX family chip exist and alive */
    deviceId = detectDevices();
    if (deviceId == 0)
        return (-1);

    /* Save the currently set device. */
    prevDevice = getCurrentDevice();   
    
    /* Go through every device and do the initialization. */
    for (deviceIndex = 0; deviceIndex < (unsigned char)getNumOfDevices(); deviceIndex++)
    {
        /* Set the current adapter */
        setCurrentDevice(deviceIndex);
    
        /* Initialize a single chip. */
        if (initChipParamEx(pInitParam) != 0)
            result = -1;
    }
    
    /* Restore the previously set device. */
    setCurrentDevice(prevDevice);
        
    return result;
}

/*
 * Initialize a single chip with default parameters.
 *
 * Input: none.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 *
 * Note:
 *      Caller needs to call the detectDevice and setCurrentDevice
 *      to set the device before calling this initChipEx.
 */
int32_t initChipEx()
{
    initchip_param_t initParam;
    uint32_t memoryClockDividerIndex, masterClockDividerIndex;
    uint32_t totalMemoryClockDivider, totalMasterClockDivider;
    uint32_t chipClock, masterClock, diff;
    uint32_t bestDiff = 0xffffffff; /* biggest 32 bit unsigned number */
    unsigned char *pMasterClockDivider, *pMemoryClockDivider;

    /* Initialize the chip */
    initParam.powerMode = 0;  /* Default to power mode 0 */
    
#if 1
    initParam.memClock = DEFAULT_MEMORY_CLK;
//    initParam.memClock = 308;

    initParam.chipClock = DEFAULT_MEMORY_CLK;
    initParam.masterClock = initParam.chipClock/3;
#else
    /* Maximum DDR memory clock. */
    initParam.memClock = DEFAULT_MEMORY_CLK;       
    if (getMemoryBusWidth() == 0x01)
    {
        /* 64-bit memory bus. Since the memory is DDR, therefore, it is similar
           like 128-bit bus using SDRAM. This means that the optimum engine
           clock is the same as the memory clock. FYI, the engine is 128-bit. 
           NOTE: The maximum chip clock value can be set is 672MHz, therefore,
                 in here, the maximum master clock can be set is 224MHz (672MHz / 3).
                 However, because of the 190MHz limitation, therefore the next
                 available master clock that can be set is 168MHz (672MHz / 4).
         */
        initParam.masterClock = initParam.memClock;
    }
    else
    {
        /* 32-bit memory bus. Since the memory is DDR, therefore, it is similar
           like 64-bit bus using SDRAM. This means that the optimum engine 
           clock is 1/2 of the memory clock. FYI, the engine is 128-bit */
        initParam.masterClock = initParam.memClock / 2;
    }
    
    /* Adjust the master Clock accordingly (Should not exceed the max frequency that
       the engine can run (about 190 MHz) */
    if (initParam.masterClock > (MAXIMUM_MASTER_CLOCK/1000000))
        initParam.masterClock = (MAXIMUM_MASTER_CLOCK/1000000);

    /* Get the master and memory clock divider values table */
    pMemoryClockDivider = getMemoryClockDivider();
    pMasterClockDivider = getMasterClockDivider();

    /* Calculate the chip clock by traversing each possible multiplication of memory
       clock with the divider. */
    for (memoryClockDividerIndex = 0; 
         memoryClockDividerIndex <= getTotalMemoryClockDivider(); 
         memoryClockDividerIndex++)
    {
        chipClock = initParam.memClock * pMemoryClockDivider[memoryClockDividerIndex];

        /* There is a limit of 1GHz for the chip Clock.
           Therefore, considering the maximum DDR memory of 336MHz, the best 
           chip clock value that can be set is 672 MHz (336 MHz * 2).
         */
        if (chipClock > (MAXIMUM_CHIP_CLOCK/1000000))
            break;

        for (masterClockDividerIndex = 0; 
             masterClockDividerIndex < getTotalMasterClockDivider(); 
             masterClockDividerIndex++)
        {
            masterClock = roundedDiv(chipClock, pMasterClockDivider[masterClockDividerIndex]);
    
            /* Check if we found a closer values to the requested clock. */
            diff = absDiff(initParam.masterClock, masterClock);
            if (diff < bestDiff)
            {
                bestDiff = diff;
                initParam.chipClock = chipClock;
            }
        }
    }
#endif

    DDKDEBUGPRINT((INIT_LEVEL, "Chip Clock: %d\n", initParam.chipClock));
    DDKDEBUGPRINT((INIT_LEVEL, "Memory Clock: %d\n", initParam.memClock));
    DDKDEBUGPRINT((INIT_LEVEL, "Master Clock: %d\n", initParam.masterClock));

    initParam.setAllEngOff = 1;
    initParam.resetMemory = 1;
    
    return initChipParamEx(&initParam);
}

/*
 * Initialize all available chips with default parameters.
 *
 * Input: none.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 *
 * Note:
 *      This function initializes all the adapters at once.
 *      To initialize a selected adapters, please use initChipEx
 */
_X_EXPORT int32_t initChip()
{
    initchip_param_t initParam;
    int32_t prevDevice, result = 0;
    unsigned short deviceId;
    unsigned char deviceIndex;
    
    /* Check if any SMI VGX family chip exist and alive */
    deviceId = detectDevices();
    if (deviceId == 0)
        return (-1);
    
    /* Save the currently set device. */
    prevDevice = getCurrentDevice();   
    
    /* Go through every device and do the initialization. */
    for (deviceIndex = 0; deviceIndex < (unsigned char)getNumOfDevices(); deviceIndex++)
    {
        /* Set the current adapter */
        setCurrentDevice(deviceIndex);
    
        /* Initialize a single chip. */
        if (initChipEx() != 0)
            result = -1;
    }
    
    /* Restore the previously set device. */
    setCurrentDevice(prevDevice);

    return result;
}

/*
 * Set Scratch Data
 */
void setScratchData(
    uint32_t dataFlag,
    uint32_t data
)
{
    unsigned char ucValue;
    uint32_t ulValue;

    switch (dataFlag)
    {
        case SWAP_DISPLAY_CTRL_FLAG:
            ucValue = peekRegisterByte(SCRATCH_DATA);
            if (data == 0)
                ucValue &= ~0x01;
            else
                ucValue |= 0x01;
            pokeRegisterByte(SCRATCH_DATA, ucValue);
            break;
        default:
            /* Do nothing */
            break;
    }
}

/*
 * Get Scratch Data
 */
uint32_t getScratchData(
    uint32_t dataFlag
)
{
    unsigned char ucValue;
    
    switch (dataFlag)
    {
        case SWAP_DISPLAY_CTRL_FLAG:
            ucValue = peekRegisterByte(SCRATCH_DATA);
            if (ucValue & 0x01)
                return 1;
            else
                return 0;
            break;
        default:
            /* Do nothing */
            break;
    }
    
    return (-1);
}

