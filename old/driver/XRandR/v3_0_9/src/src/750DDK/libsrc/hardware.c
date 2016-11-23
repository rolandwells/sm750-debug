/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  Hardware.c --- Voyager GX SDK 
*  This file contains the source code for the hardware.
* 
*******************************************************************/
#include "hardware.h"
#include "os.h"

#include "ddkdebug.h"

/* Table of all the device ID's supported by this library */
static unsigned short gwDeviceIdTable[] = {
SMI_DEVICE_ID_SM718,
SMI_DEVICE_ID_SM750,
0};

/* Use Voyager PCI ID as default, but it may be changed by
   detectDevice() if it is called */
static unsigned short gwDeviceId = 0x0750;

/* Total number of devices with the same ID in the system.
   Use 1 as default.
*/
static unsigned short gwNumOfDev = 1;

/* Current device in use.
   If there is only ONE device, it's always device 0.
   If gwNumDev > 1, user need to set device 0, device 1, device 2, etc..
   The purpose is using it control multiple devices of the same ID, if any.
*/
static unsigned short gwCurDev = 0;

static unsigned char *frameBufBase[MAX_SMI_DEVICE] = {0, 0, 0, 0};
static unsigned char *mmioBase[MAX_SMI_DEVICE] = {0, 0, 0, 0};
static unsigned char *frameBufPhysicalBase[MAX_SMI_DEVICE] = {0, 0, 0, 0};

#ifdef USE_A0000
static uint32_t gBankSize = 0;

/*
 * Set the bank size when using A0000h memory addressing .
 */
void setBankSize(
    uint32_t bankSize
)
{
    gBankSize = bankSize;
}
#endif

#if 0
/*
 * Detect if a specific device exist in the system.
 * If exist, enable it on the PCI bus.
 * Return: 0 if device found
 *         -1 if no device.
 */
int32_t enableDevice(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum /* If more than one device in system, device number are ordered as 0, 1, 2,... */
)
{
    unsigned short value;
    unsigned char control;

    /* Detect SMI device */
    value = readPCIWord(vendorId, deviceId, deviceNum, 0x00);
    if (value != vendorId)
        return -1;

    /* Enable the PCI device, if not yet enabled */
    control = readPCIByte(vendorId, deviceId, deviceNum, 0x04);
    
#ifdef USE_IO_PORT
    if (deviceId == SMI_DEVICE_ID_SM750)
    {
        /* Set the Base Address Register 2 (BAR2) to 0xA0000 or 0xB0000 (depending
           on the necessary configuration */
        writePCIDword(vendorId, deviceId, deviceNum, 0x18, 0xA0000);
        
        /* Set the Base Address Register 3 (BAR3) to 0xB8000 */
        writePCIDword(vendorId, deviceId, deviceNum, 0x1C, 0xB8000);
        
        /* Set the Base Address Register 4 (BAR4) to 0x3C0 */
        writePCIDword(vendorId, deviceId, deviceNum, 0x20, 0x3C0);
        
        /* Set the Base Address Register 5 (BAR5) to 0x3D0/0x3B0 (depending on
           the necessary configuration */
        writePCIDword(vendorId, deviceId, deviceNum, 0x24, 0x3D0);        
    }

    /* Enable the IO Port */
    if ((control & 0x1F) != 0x1F)
        writePCIByte(vendorId, deviceId, deviceNum, 0x04, control | 0x1F);
#else
    if (deviceId == SMI_DEVICE_ID_SM750)
    {
        /* In PCI-E, bit 3 and 4 are not used and should be set to zeroes by hardware. */
        if ((control & 0x07) != 0x07)
            writePCIByte(vendorId, deviceId, deviceNum, 0x04, control | 0x07);
    }
    else
    {
        if ((control & 0x1E) != 0x1E)
            writePCIByte(vendorId, deviceId, deviceNum, 0x04, control | 0x1E);
    }
#endif

    return 0;
}
#endif

#if 0
/*
 * Get a pointer to the base of Memory Mapped IO space.
 *
 * Return: A pointer to base of MMIO.
 *         NULL pointer if fail.
 */
void *getMmioBase( 
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum /* If more than one device in system, device number are ordered as 0, 1, 2,... */
)
{
    uint32_t physicalAddr;

    if (enableDevice(vendorId, deviceId, deviceNum) == -1)
        return (void *)0;

    /* Get MMIO physical address */
    physicalAddr = readPCIDword(vendorId, deviceId, deviceNum, 0x14);
        
    DDKDEBUGPRINT((INIT_LEVEL, "Mmio Physical Addr: %08x\n", physicalAddr));
    if (physicalAddr == (-1))
       return (void *)0;
    
    /* Map it into the address space. */
    return (mapPhysicalAddress((void*)physicalAddr, SM750_PCI_ALLOC_MMIO_SIZE));
}
#endif

/* 
 * Get a pointer to the base of physical video memory. This can be used for DMA transfer.
 * In DOS, the physical and the logical address most likely are the same.
 * Return: A pointer to base of physical video memory.
 *         NULL pointer if fail.
 */
void *getPhysicalMemoryBase(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum /* If more than one device in system, device number are ordered as 0, 1, 2,... */
)
{
    uint32_t physicalAddr;

    if (enableDevice(vendorId, deviceId, deviceNum) == -1)
        return (void *)0;
    
#ifdef USE_A0000
    /*
     * Access frame buffer through the 0xA0000 address.
     */
    physicalAddr = 0xA0000;
    
    /* Set to Write Mode 0 */
    pokeIO(GRAPHICS_REGISTER, 0x05, 0x00);
        
    /* Enable address 0xA0000 to 0xAFFFF */
#ifdef BANK_SIZE_128K
    pokeIO(GRAPHICS_REGISTER, 0x06, 0x00);
    setBankSize(128*1024);
#else
    pokeIO(GRAPHICS_REGISTER, 0x06, 0x04);
    setBankSize(64*1024);
#endif
    
    /* Enable writing to corresponding plane. Set it ot 0x0f or 0x04. BIOS set this IO
       port to 0x04 during font loading */
    pokeIO(SEQUENCER_REGISTER, 0x04, 0x04);

#else
    /*
     * Access frame buffer through the PCI linear address.  
     */
     
    /* Get frame buffer physical address */
    physicalAddr = readPCIDword(vendorId, deviceId, deviceNum, 0x10);

    /*
     * In a memory bar (Frame buffer), bit 3 to bit 0 are used as the base
     * address information, therefore, we should mask this out and set them
     * to zero.
     */
    physicalAddr &= ~0x0000000F;
#endif

    DDKDEBUGPRINT((INIT_LEVEL, "Memory Physical Addr: %08x\n", physicalAddr));
    if (physicalAddr == (-1))
       return (void *)0;

    return ((void *)physicalAddr);
}

/* 
 * Get a pointer to the base of video memory (or frame buffer).
 *
 * Return: A pointer to base of video memory.
 *         NULL pointer if fail.
 */
#if 0 
void *getMemoryBase(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum /* If more than one device in system, device number are ordered as 0, 1, 2,... */
)
{
    void *physicalAddr;

    physicalAddr = getPhysicalMemoryBase(vendorId, deviceId, deviceNum);
    if (physicalAddr == (void *)0)
       return (void *)0;

#ifdef USE_A0000
    return physicalAddr;
#else
    /* Map it into the address space */
    return (mapPhysicalAddress(physicalAddr, SM750_PCI_ALLOC_MEMORY_SIZE));
#endif
}

#endif
/*
 * Get the physical address of an offset location in frame buffer.
 * In DOS, the physical and the logical address most likely are the same.
 * Return: A valid address if success.
 *         NULL address if fail.
 */
void *getPhysicalAddress(uint32_t offset /* Offset from base of physical frame buffer */)
{
    if (frameBufPhysicalBase[gwCurDev] == (unsigned char *)0)
    {
        frameBufPhysicalBase[gwCurDev] = (unsigned char *)getPhysicalMemoryBase(SMI_PCI_VENDOR_ID, gwDeviceId, gwCurDev);
        DDKDEBUGPRINT((INIT_LEVEL, "Device: %x, Memory Physical Base Addr: %08x\n", gwCurDev, frameBufPhysicalBase[gwCurDev]));
        if (frameBufPhysicalBase[gwCurDev] == 0)
        {
            return((void *)0);
        }
    }
    
    return(frameBufPhysicalBase[gwCurDev] + offset);
}

/*
 * Get the logical address of an offset location in frame buffer.
 * Return: A valid address if success.
 *         NULL address if fail.
 */
void *getAddress(uint32_t offset /*Offset from base of frame buffer*/)
{
    if (frameBufBase[gwCurDev] == (unsigned char *)0)
    {
        frameBufBase[gwCurDev] = (unsigned char *)getMemoryBase(SMI_PCI_VENDOR_ID, gwDeviceId, gwCurDev);
        DDKDEBUGPRINT((INIT_LEVEL, "Device: %x, Memory Base Addr: %08x\n", gwCurDev, frameBufBase[gwCurDev]));
        if (frameBufBase[gwCurDev] == 0)
        {
            return((void *)0);
        }
    }

#ifdef USE_A0000
    {
        unsigned short bankValue;
        
        /* Calculate the Memory Address high (bank register) and
           the new offset */
        bankValue = (unsigned short)(offset / gBankSize);
        offset = offset % gBankSize;
        
        /* Set the Page Selection Register and also */
        pokeIO(CRT_REGISTER, 0x89, (unsigned char) bankValue);
        pokeIO(CRT_REGISTER, 0x8A, (unsigned char) (bankValue >> 8));
    }
#endif    
    
    return(frameBufBase[gwCurDev] + offset);
}

/*
 * Get the logical address of an offset location in MMIO space.
 * Return: A valid address if success.
 *         NULL address if fail.
 */
void *getMemoryMappedAddress(uint32_t offset /*Offset from base of MMIO*/)
{
    /*if (mmioBase[gwCurDev] == (unsigned char *)0)
    {*/
        mmioBase[gwCurDev] = (unsigned char *)getMmioBase(SMI_PCI_VENDOR_ID, gwDeviceId, gwCurDev);
        DDKDEBUGPRINT((INIT_LEVEL, "Device: %x, MMIO Base Addr: %08x\n", gwCurDev, mmioBase[gwCurDev]));
        if (mmioBase[gwCurDev] == 0)
        {
            return((void *)0);
        }
    /*}*/

    return(mmioBase[gwCurDev] + offset);
}

/*
 * Get the IRQ number of the current device.
 */
unsigned char getIRQ()
{
    return (readPCIByte(SMI_PCI_VENDOR_ID, gwDeviceId, gwCurDev, 0x3C));
}

/*
 * This function detects what SMI chips is in the system.
 *
 * Return: A non-zero device ID if SMI chip is detected.
 *         Zero means NO SMI chip is detected.
 */
unsigned short detectDevices()
{
    int i, j;
    
    /* Clean up the MMIO and Frame Buffer structure first. 
       Otherwise, calling this function twice, will result in different gwDeviceID. 
     */
    for (i = 0; i < MAX_SMI_DEVICE; i++)
    {
        frameBufBase[i] = 0;
        mmioBase[i] = 0;
        frameBufPhysicalBase[i] = 0;
    }

    /* Walk through the device ID table */
    for (i=0; gwDeviceIdTable[i] != 0; i++)
    {
        /* For a specific device, find out how many exist.
          This library supports a max of 4 devices of the same ID.
        */
        gwDeviceId = gwDeviceIdTable[i];
        gwNumOfDev = 0;

        for (j=0; j<MAX_SMI_DEVICE; j++)
        {
            gwCurDev = j;
#ifndef USE_IO_PORT            
            if (getMemoryMappedAddress(0) == (void *)0)
                break;
#endif
			#if 0
			if (getAddress(0) == (void *)0)
                break;
			#endif			

            gwNumOfDev++;
        }

        /* If a device is detected, don't need to try the next */
        if (gwNumOfDev > 0) break;
    }

    gwCurDev = 0; /* Always defaults to device 0 after initialization */

    DDKDEBUGPRINT((INIT_LEVEL, "Selected DeviceId: %x\n", gwDeviceId));

    /* Return the ID of detected device */
    return gwDeviceIdTable[i];
}

/*
 * How many devices of the same ID are there.
 */
unsigned short getNumOfDevices()
{
    return gwNumOfDev;
}
 
/*
 * This function sets up the current accessible device, if more
 * than one of the same ID exist in the system.
 *
 * Note:
 * Single device application don't need to call this function.
 * This function is to control multiple devices.
 */
int32_t setCurrentDevice(unsigned short dev)
{
    /* Error check */
    if ( dev >= gwNumOfDev)
        return -1;

    gwCurDev = dev;
    return 0;
}

/*
 * This function gets the current accessible device index.
 */
unsigned short getCurrentDevice()
{
    return gwCurDev;
}

unsigned char peekByte(uint32_t offset)
{
    void *pAddr;
    
    if ((pAddr = getAddress(offset)) == (void *)0)
        return(0xFF);

    return *(volatile unsigned char *)(pAddr);
}

unsigned short peekWord(uint32_t offset)
{
    void *pAddr;

    if ((pAddr = getAddress(offset)) == (void *)0)
        return(0xFFFF);

    return *(volatile unsigned short *)(pAddr);
}

uint32_t peekDWord(uint32_t offset)
{
    void *pAddr;

    if ((pAddr = getAddress(offset)) == (void *)0)
        return(0xFFFFFFFF);

    return *(volatile uint32_t *)(pAddr);
}

void poke_4_Byte(uint32_t offset, unsigned char *buf)
{
    uint32_t tData; // WR by DW
    void *pAddr;

    tData = ((uint32_t)buf[3]<<24) + ((uint32_t)buf[2]<<16) + ((unsigned int)buf[1]<<8) + buf[0];

    if ((pAddr = getAddress(offset)) != (void *)0)
        *(volatile uint32_t *)(pAddr) = tData;
}
 
void pokeByte(uint32_t offset, unsigned char value)
{
    void *pAddr;

    if ((pAddr = getAddress(offset)) != (void *)0)
        *(volatile unsigned char *)(pAddr) = value;
}

void pokeWord(uint32_t offset, unsigned short value)
{
    void *pAddr;

    if ((pAddr = getAddress(offset)) != (void *)0)
        *(volatile unsigned short *)(pAddr) = value;
}

void pokeDWord(uint32_t offset, uint32_t value)
{
    void *pAddr;

    if ((pAddr = getAddress(offset)) != (void *)0)
        *(volatile uint32_t *)(pAddr) = value;
}

/*
 * Read the IO Port.
 *
 * Input:
 *      regType     - Register Type
 *      ioIndex     - When register type is OTHER_REGISTER, then ioIndex should
 *                    contains the IO Port address. This will function just as
 *                    regular IO port reading.
 */
unsigned char peekIO(
    reg_type_t regType,         /* Register Type */
    unsigned short ioIndex      /* IO Port Index */
)
{
    unsigned char value;
    switch (regType)
    {
        case MISCELLANEOUS_REGISTER:
            value = readIO(0x3CC);
            break;
        case SEQUENCER_REGISTER:
            writeIO(0x3C4, (unsigned char) ioIndex);
            value = readIO(0x3C5);
            break;
        case CRT_REGISTER:
            writeIO(0x3D4, (unsigned char) ioIndex);
            value = readIO(0x3D5);
            break;
        case GRAPHICS_REGISTER:
            writeIO(0x3CE, (unsigned char) ioIndex);
            value = readIO(0x3CF);
            break;
        default:
            value = readIO(ioIndex);
            break;
    }
    
    return (value);
}

/*
 * Write to the IO Port.
 *
 * Input:
 *      regType     - Register Type
 *      ioIndex     - When register type is OTHER_REGISTER, then ioIndex should
 *                    contains the IO Port address. This will function just as
 *                    regular IO Port writing.
 *      value       - The value to be written to the given register type and index.
 */
void pokeIO(
    reg_type_t regType,         /* Register Type */
    unsigned short ioIndex,     /* IO Port index */
    unsigned char value         /* Value to be written to the port */
)
{
    switch (regType)
    {
        case MISCELLANEOUS_REGISTER:
            writeIO(0x3C2, value);
            break;
        case SEQUENCER_REGISTER:
            writeIO(0x3C4, (unsigned char) ioIndex);
            writeIO(0x3C5, value);
            break;
        case CRT_REGISTER:
            writeIO(0x3D4, (unsigned char) ioIndex);
            writeIO(0x3D5, value);
            break;
        case GRAPHICS_REGISTER:
            writeIO(0x3CE, (unsigned char) ioIndex);
            writeIO(0x3CF, value);
            break;
        default:
            writeIO(ioIndex, value);
            break;
    }
}

/* Read/Write MMIO through IO Port helper */
uint32_t ioReadMMIOAddress()
{
    uint32_t value = 0;
    unsigned char index;
    
    for (index = 0x82; index >= 0x80; index--)
        value = (value << 8) + peekIO(CRT_REGISTER, index);
    
    return value;
}

void ioWriteMMIOAddress(uint32_t address)
{
    unsigned char index;
    
    for (index = 0x80; index <= 0x82; index++)
    {
        pokeIO(CRT_REGISTER, index, (unsigned char) address);
        address >>= 8;
    }
}

uint32_t ioReadMMIOData()
{
    uint32_t value = 0;
    unsigned char index;
    
    for (index = 0x87; index >= 0x84; index--)
        value = (value << 8) + peekIO(CRT_REGISTER, index);
    
    return value;
}

void ioWriteMMIOData(uint32_t data)
{
    unsigned char index;
    
    pokeIO(CRT_REGISTER, 0x87, (unsigned char) (data >> 24));
    pokeIO(CRT_REGISTER, 0x86, (unsigned char) (data >> 16));
    pokeIO(CRT_REGISTER, 0x85, (unsigned char) (data >> 8));
    pokeIO(CRT_REGISTER, 0x84, (unsigned char) data);
}

/* REGISTER SPACE */

unsigned char peekRegisterByte(uint32_t offset)
{
#ifdef USE_IO_PORT
    uint32_t regValue;
    ioWriteMMIOAddress(offset & ~0x00000003);
    regValue = ioReadMMIOData();
    
    regValue >>= ((offset & 0x00000003) * 8);
    return (unsigned char) regValue;
#else
    void *pAddr;

    if ((pAddr = getMemoryMappedAddress(offset)) == (void *)0)
        return (0xFF);

    return *(volatile unsigned char *)(pAddr);
#endif
}

unsigned short peekRegisterWord(uint32_t offset)
{
#ifdef USE_IO_PORT
    uint32_t regValue;
    ioWriteMMIOAddress(offset & ~0x00000003);
    regValue = ioReadMMIOData();
    
    regValue >>= ((offset & 0x00000002) * 8);
    return (unsigned short) regValue;
#else
    void *pAddr;

    if ((pAddr = getMemoryMappedAddress(offset)) == (void *)0)
        return (0xFFFF);

    return *(volatile unsigned short *)(pAddr);
#endif
}

_X_EXPORT uint32_t peekRegisterDWord(uint32_t offset)
{
#ifdef USE_IO_PORT
    ioWriteMMIOAddress(offset & ~0x00000003);
    return ioReadMMIOData();
#else
    void *pAddr;

    if ((pAddr = getMemoryMappedAddress(offset)) == (void *)0)
        return (0xFFFFFFFF);

    return *(volatile uint32_t *)(pAddr);
#endif
}

void pokeRegisterByte(uint32_t offset, unsigned char value)
{
#ifdef USE_IO_PORT
    uint32_t regValue, mask, shift;
    
    /* Read the actual register value first */
    regValue = peekRegisterDWord(offset);
    
    /* Overwrite the one that needs to be replaced */
    shift = ((offset & 0x00000003) * 8);
    mask = 0x000000FF << shift;    
    regValue = (regValue & ~mask) + (((uint32_t) value) << shift);
    ioWriteMMIOData(regValue);
#else
    void *pAddr;

    if ((pAddr = getMemoryMappedAddress(offset)) != (void *)0)
        *(volatile unsigned char *)(pAddr) = value;
#endif
}

void pokeRegisterWord(uint32_t offset, unsigned short value)
{
#ifdef USE_IO_PORT
    uint32_t regValue, mask, shift;
    
    /* Read the actual register value first */
    regValue = peekRegisterDWord(offset);
    
    /* Overwrite the one that needs to be replaced */
    shift = ((offset & 0x00000002) * 8);
    mask = 0x0000FFFF << shift;
    regValue = (regValue & ~mask) + (((uint32_t) value) << shift);
    ioWriteMMIOData(regValue);
#else
    void *pAddr;

    if ((pAddr = getMemoryMappedAddress(offset)) != (void *)0)
        *(volatile unsigned short *)(pAddr) = value;
#endif
}

_X_EXPORT void pokeRegisterDWord(uint32_t offset, uint32_t value)
{
#ifdef USE_IO_PORT
    ioWriteMMIOAddress(offset & ~0x00000003);
    ioWriteMMIOData(value);
#else
    void *pAddr;

    if ((pAddr = getMemoryMappedAddress(offset)) != (void *)0)
        *(volatile uint32_t *)(pAddr) = value;
#endif
}

