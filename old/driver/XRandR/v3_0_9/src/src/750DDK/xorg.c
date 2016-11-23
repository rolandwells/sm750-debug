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
#include <stdint.h>

#include "defs.h"
#include "hardware.h"
#include "os.h"

#include "clock.h"
#include "mode.h"


#include <stdint.h>
#include <mode.h>
#include <xf86str.h>

#include "xf86_OSproc.h"
#include <X11/X.h>
#include <X11/Xos.h>
#include <X11/Xproto.h>
#include "xf86i2c.h"




/* Global variable to save all the SMI devices */
static struct pci_dev *g_pCurrentDevice = (struct pci_dev *)0;
static struct pci_access *g_pAccess;
static struct pci_filter g_filter;

static unsigned long SM750_MMIO_Base = 0;
static unsigned long SM750_Memory_Base = 0;



/*
 * This function maps a physical address into logical address.
 * Return: NULL address pointer if fail
 *         A Logical address pointer if success.
 */
void *mapPhysicalAddress(
    void *phyAddr,            /* 32 bit physical address */
//    uint32_t size        /* Memory size to be mapped */
    unsigned long size
)
{
    unsigned long address;
    int fileDescriptor;
        
    fileDescriptor = open("/dev/mem", O_RDWR);
    if (fileDescriptor == -1)
        return ((void *)0);
        
    address = (unsigned long) mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, (uint32_t)phyAddr);

    if ((void *) address == MAP_FAILED)
        return ((void *)0);
    
    return ((void *) address);
}

/* Initialize the PCI */
int32_t initPCI(
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
        return ((uint32_t) pci_read_int32_t(g_pCurrentDevice, offset));
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
    if ((control & 0x1E) != 0x1E)
        writePCIByte(vendorId, deviceId, deviceNum, 0x04, control | 0x1E);

    return 0;
}

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
	//Now only support one sm750
	if((deviceId==SMI_DEVICE_ID_SM750) &&
	    (deviceNum == 0))
   		 return (SM750_MMIO_Base);
	else 
		return NULL;
}

_X_EXPORT void setMmioBase(
	unsigned long address
)
{
    SM750_MMIO_Base = address;
}


/* 
 * Get a pointer to the base of video memory (or frame buffer).
 *
 * Return: A pointer to base of video memory.
 *         NULL pointer if fail.
 */
void *getMemoryBase(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum /* If more than one device in system, device number are ordered as 0, 1, 2,... */
)
{
	//Now only support one sm750
	if((deviceId==SMI_DEVICE_ID_SM750) &&
	    (deviceNum == 0))
   		 return (SM750_Memory_Base);
	else 
		 return NULL;
}

_X_EXPORT void setMemoryBase(
	unsigned long address
)
{
    //SM750_Memory_Base = address;
}

/*
 * Get the IRQ number of a device from system.
 */
unsigned char getDeviceIRQ(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum /* If more than one device in system, device number are ordered as 0, 1, 2,... */
)
{
    return(readPCIByte(vendorId, deviceId, deviceNum, 0x3C));
}

/*******************************************************************
 * Interrupt implementation
 * 
 * This implementation is used for handling the SM50x interrupt.
 *******************************************************************/
/* Interrupt structure */
typedef struct _interrupt_t
{
    struct _interrupt_t *next;
    uint32_t mask;
    void (*handler)(uint32_t status);
}
interrupt_t;

static interrupt_t *int_handlers = (interrupt_t *)0;

short enableTimerInterrupt(unsigned char enable);

/* 
 * IRQ Handler. 
 */
 
/* below implement is done in intr.c */
#if 0	
void irqHandler()
{
    uint32_t status;
    interrupt_t *p;

    /* Disable Timer Interrupt */
//    enableTimerInterrupt(0);

    /* Get interrupt status */
    status = peekRegisterDWord(INT_STATUS);

    /* Walk all registered handlers for handlers that support this interrupt */
    /* status */
    for (p = int_handlers; p != ((interrupt_t *)0); p = p->next)
    {
        p->handler(status);
    }
    
    /* Enable Timer Interrupt back */
//    enableTimerInterrupt(1);
}
#endif

void irqHandler(void);


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

/* below implement(s) were done in intr.c since ddk 1.2  */

#if 0
/* 
 * Count number of interrupt handler that use a specific given mask 
 */
short countMask(
    uint32_t mask
)
{
    interrupt_t *p;
    short count = 0;

    /* Loop through all registered interrupt handlers */
    for (p = int_handlers; p != ((interrupt_t *)0); p = p->next)
    {
        /* Up the count if this interrupt handlers supports the mask */
        if (p->mask & mask)
        {
            count++;
        }
    }

    /* Return count */
    return(count);
}

/* 
 * Change interrupt mask 
 */
void setIntMask(
    uint32_t mask_on, 
    uint32_t mask_off
)
{
    uint32_t mask;

    /* Get current interrupt mask */
    mask = peekRegisterDWord(INT_MASK);

    /* Enable new masks and disable old masks */
    mask = mask | mask_on;
    mask = mask & ~mask_off;

    /* Program new interrupt mask */
    pokeRegisterDWord(INT_MASK, mask);
}

/* 
 * Register interrupt handler
 *
 * Output:
 *      0   - Success
 *      -1  - Out of memory
 * 
 */
short registerHandler(
    void (*handler)(uint32_t status), 
    uint32_t mask
)
{
    interrupt_t *p;

    /* Allocate a new interrupt structure */
    p = (interrupt_t *) malloc(sizeof(interrupt_t));
    if (p == ((interrupt_t *)0))
    {
        /* No more memory */
        return(-1);
    }

    /* If this is the first interrupt handler, intercept the correct IRQ */
    if (int_handlers == ((interrupt_t *)0))
    {
        if (enableTimerInterrupt(1) != 0)
            return (-1);
    }

    /* Fill interrupt structure */
    p->next = int_handlers;
    p->handler = handler;
    p->mask = mask;
    int_handlers = p;

    return 0;
}

/* 
 * Unregister a registered interrupt handler
 *
 * Output:
 *      0   - Success
 *      -1  - Handler is not found 
 */
short unregisterHandler(
    void (*handler)(uint32_t status)
)
{
    interrupt_t *p, *prev;
    uint32_t mask;
    struct itimerval tick;

    /* Find the requested handle to unregister */
    for (p = int_handlers, prev = ((interrupt_t *)0); p != ((interrupt_t *)0); prev = p, p = p->next)
    {
        if (p->handler == handler)
        {
            /* Remove the interrupt handler */
            if (prev == ((interrupt_t *)0))
            {
                int_handlers = p->next;
            }
            else
            {
                prev->next = p->next;
            }
            free(p);

            /* If this was the last interrupt handler, remove the IRQ handler */
            if (int_handlers == ((interrupt_t *)0))
            {
                if (enableTimerInterrupt(0) != 0)
                    return (-1);
            }

            /* Success */
            return 0;
        }
    }

    /* Oops, handler is not registered */
    return -1;
}

#endif

#if 0
typedef struct _DisplayModeRec {
    struct _DisplayModeRec *	prev;
    struct _DisplayModeRec *	next;
    char *			name;		/* identifier for the mode */
    ModeStatus			status;
    int				type;

    /* These are the values that the user sees/provides */
    int				Clock;		/* pixel clock freq (kHz) */
    int				HDisplay;	/* horizontal timing */
    int				HSyncStart;
    int				HSyncEnd;
    int				HTotal;
    int				HSkew;
    int				VDisplay;	/* vertical timing */
    int				VSyncStart;
    int				VSyncEnd;
    int				VTotal;
    int				VScan;
    int				Flags;

  /* These are the values the hardware uses */
    int				ClockIndex;
    int				SynthClock;	/* Actual clock freq to
					  	 * be programmed  (kHz) */
    int				CrtcHDisplay;
    int				CrtcHBlankStart;
    int				CrtcHSyncStart;
    int				CrtcHSyncEnd;
    int				CrtcHBlankEnd;
    int				CrtcHTotal;
    int				CrtcHSkew;
    int				CrtcVDisplay;
    int				CrtcVBlankStart;
    int				CrtcVSyncStart;
    int				CrtcVSyncEnd;
    int				CrtcVBlankEnd;
    int				CrtcVTotal;
    Bool			CrtcHAdjusted;
    Bool			CrtcVAdjusted;
    int				PrivSize;
    INT32 *			Private;
    int				PrivFlags;

    float			HSync, VRefresh;
} DisplayModeRec, *DisplayModePtr;
#endif

static int32_t xorg_setCustomMode(logicalMode_t * pLogicalMode,mode_parameter_t * pVesaMode)
{
    pll_value_t pll;
    uint32_t uiActualPixelClk,uiTemp;    
    

    pll.inputFreq = DEFAULT_INPUT_CLOCK;
    pll.clockType = getClockType(pLogicalMode->dispCtrl);
    uiActualPixelClk = calcPllValue(pVesaMode->pixel_clock,&pll);

    /* don't wonder why base and pith be zero , they are finally reset in other routines */    
    programModeRegisters(pVesaMode,pLogicalMode->bpp,0,0,&pll);
    return 0;
}

/* _xorg_setMode will not tak care of expansion situation  */
_X_EXPORT unsigned int xorg_setMode(logicalMode_t * pLogicalMode,DisplayModePtr adjusted_mode)
{
    mode_parameter_t vesaMode; /* physical parameters for the mode */
    vesaMode.pixel_clock = adjusted_mode->Clock * 1000;      
    vesaMode.horizontal_total = adjusted_mode->HTotal;
    vesaMode.horizontal_display_end = adjusted_mode->HDisplay;
    vesaMode.horizontal_sync_start = adjusted_mode->HSyncStart;
    vesaMode.horizontal_sync_width = adjusted_mode->HSyncEnd - adjusted_mode->HSyncStart;
    vesaMode.horizontal_sync_polarity = (adjusted_mode->Flags & V_PHSYNC)?POS:NEG;
    
    vesaMode.vertical_total = adjusted_mode->VTotal;
    vesaMode.vertical_display_end = adjusted_mode->VDisplay;
    vesaMode.vertical_sync_start = adjusted_mode->VSyncStart;
    vesaMode.vertical_sync_height = adjusted_mode->VSyncEnd - adjusted_mode->VSyncStart;
    vesaMode.vertical_sync_polarity = (adjusted_mode->Flags & V_PVSYNC)?POS:NEG;
    return xorg_setCustomMode(pLogicalMode, &vesaMode);
}



_X_EXPORT void xorg_I2CUDelay(int usec)
{
  struct timeval begin, cur;
  long d_secs, d_usecs;
  long diff;

  if (usec > 0) {
    X_GETTIMEOFDAY(&begin);
    do {
      /* It would be nice to use {xf86}usleep, 
       * but usleep (1) takes >10000 usec !
       */
      X_GETTIMEOFDAY(&cur);
      d_secs  = (cur.tv_sec - begin.tv_sec);
      d_usecs = (cur.tv_usec - begin.tv_usec);
      diff = d_secs*1000000 + d_usecs;
    } while (diff>=0 && diff< (usec + 1));
  }
}


