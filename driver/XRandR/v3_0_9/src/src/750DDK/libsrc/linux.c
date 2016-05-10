/*******************************************************************
* 
*         Copyright (c) 2008 by Silicon Motion, Inc. (SMI)
* 
*  LINUX.C --- VGX family DDK
*
*  This file contains LINUX dependent codes, which works only 
*  with PCI boards in x86 systems.
*
*  Don't use this file for other OS, non-x86 system, and non-PCI.
* 
*******************************************************************/

#include <fcntl.h>
#include <pci/pci.h>
#include <pci/types.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>

/* Global variable to save all the SMI devices */
static struct pci_dev *g_pCurrentDevice = (struct pci_dev *)0;
static struct pci_access *g_pAccess;
static struct pci_filter g_filter;

/*
 * This function maps a physical address into logical address.
 * Return: NULL address pointer if fail
 *         A Logical address pointer if success.
 */
void *mapPhysicalAddress(
    void *phyAddr,            /* 32 bit physical address */
    unsigned long size        /* Memory size to be mapped */
)
{
    unsigned long address;
    int fileDescriptor;
        
    fileDescriptor = open("/dev/mem", O_RDWR);
    if (fileDescriptor == -1)
        return ((void *)0);
        
    address = (unsigned long) mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, (unsigned long)phyAddr);

    if ((void *) address == MAP_FAILED)
        return ((void *)0);
    
    return ((void *) address);
}

/* Initialize the PCI */
long initPCI(
    unsigned short vendorId, 
    unsigned short deviceId, 
    unsigned short deviceNum
)
{
    unsigned short deviceIndex;
    
    /* Get the pci_access structure */
    g_pAccess = pci_alloc();

    /* Initialize the PCI library */
    pci_init(g_pAccess);

    /* Set all options you want -- here we stick with the defaults */    
    pci_filter_init(g_pAccess, &g_filter);
  	g_filter.vendor = vendorId;
  	g_filter.device = deviceId;
  	
    /* Get the list of devices */
    pci_scan_bus(g_pAccess);
    for(g_pCurrentDevice = g_pAccess->devices, deviceIndex = 0; 
        g_pCurrentDevice; 
        g_pCurrentDevice = g_pCurrentDevice->next)
    {
        if (pci_filter_match(&g_filter, g_pCurrentDevice))
        {
            if (deviceIndex == deviceNum)
            {
                pci_fill_info(g_pCurrentDevice, PCI_FILL_IDENT | PCI_FILL_IRQ | PCI_FILL_BASES | PCI_FILL_ROM_BASE | PCI_FILL_SIZES);
                return 0;
            }
            
            /* Increment the device index */
            deviceIndex++;
        }
    }

    return (-1);
}

/*
 * This function reads a DWord value from the PCI configuration space
 * of a specific PCI device.
 *
 * Inputs are the Vendor and device ID of the device in question.
 * Return: a DWord value.
 */
unsigned long readPCIDword(
    unsigned short vendorId,    /* PCI vendor ID */
    unsigned short deviceId,    /* PCI device ID */
    unsigned short deviceNum,   /* If more than one device in system, device number are ordered as 0, 1, 2,... */
    unsigned short offset       /* Offset in configuration space to be read */
)
{
    if (initPCI(vendorId, deviceId, deviceNum) == 0)
    {
        return ((unsigned long) pci_read_long(g_pCurrentDevice, offset));
    }
    
    return 0;
}

/*
 * This function reads a Word value from the PCI configuration space
 * of a specific device.
 *
 * Inputs are the Vendor and device ID of the device in question.
 * Return: a WORD value.
 */
unsigned short readPCIWord(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum, /* If more than one device in system, device number are ordered as 0, 1, 2,... */
    unsigned short offset     /* Offset in configuration space to be read */
)
{
    if (initPCI(vendorId, deviceId, deviceNum) == 0)
    {
        return ((unsigned short) pci_read_word(g_pCurrentDevice, offset));
    }
    
    return 0;
}

/*
 * This function reads a byte value from the PCI configuration space
 * of a specific device.
 *
 * Inputs are the Vendor and device ID of the device in question.
 * Return: a BYTE value.
 */
unsigned char readPCIByte(
    unsigned short vendorId, /* PCI Vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum, /* If more than one device in system, device number are ordered as 0, 1, 2,... */
    unsigned short offset     /* Offset in configuration space to be read */
)
{
    if (initPCI(vendorId, deviceId, deviceNum) == 0)
    {
        return ((unsigned char) pci_read_byte(g_pCurrentDevice, offset));
    }
    
    return 0;
}

/*
 * This function writes a byte value to the PCI configuration space
 * of a specific device.
 *
 * Inputs are the Vendor and device ID of the device in question.
 * Return:  0 = Success.
 *         -1 = Fail.
 */
long writePCIByte(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum, /* If more than one device in system, device number are ordered as 0, 1, 2,... */
    unsigned short offset,      /* Offset in configuration space to be written */
    unsigned char value       /* To be written BYTE value */
)
{
    if (initPCI(vendorId, deviceId, deviceNum) == 0)
        return (pci_write_byte(g_pCurrentDevice, offset, value));
    
    return (-1);
}

/*******************************************************************
 * Interrupt implementation support from the OS
 * 
 * This implementation is used for handling the SM50x interrupt.
 *******************************************************************/
short enableTimerInterrupt(
    unsigned char enable
)
{
    struct itimerval tick;
        
    /* Initialize struct */
    memset((void *)&tick, 0, sizeof(tick));

    if (enable != 0)
    {
        signal(SIGALRM, irqHandler);
        
        /* Timeout to run function first time */
        tick.it_value.tv_sec = 0;               /* sec */
        tick.it_value.tv_usec = 5880/*16666*/;           /* micro sec. */

        /* Interval time to run function */
        tick.it_interval.tv_sec = 0;
        tick.it_interval.tv_usec = 5880/*16666*/;
    }

    /* Set timer, ITIMER_REAL : real-time to decrease timer, send SIGALRM when timeout */
    if (setitimer(ITIMER_REAL, &tick, NULL))
        return (-1);
        
    return (0);        
}

/* 
 * Register an interrupt handler (ISR) to the interrupt vector table associated
 * with the given irq number.
 *
 * Input:
 *          irqNumber   - IRQ Number
 *          pfnHandler  - Pointer to the ISR function
 *
 * Output:
 *           0  - Success
 *          -1  - Fail
 */
short registerInterrupt(
    unsigned char irqNumber, 
    void (interrupt far *pfnHandler)()
)
{
    return enableTimerInterrupt(1);
}

/* 
 * Unregister an interrupt handler from the interrupt vector table
 *
 * Input:
 *          irqNumber   - IRQ Number
 */
short unregisterInterrupt(
    unsigned char irqNumber
)
{
    return enableTimerInterrupt(0);
}

/* 
 * Signal the End Of Interrupt to the system and chain the interrupt
 * if necessary.
 *
 * Input:
 *          irqNumber   - IRQ Number
 */
void interruptEOI(
    unsigned char irqNumber
)
{
}

