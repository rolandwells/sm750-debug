/*******************************************************************
* 
*         Copyright (c) 2008 by Silicon Motion, Inc. (SMI)
* 
*  WATCOM.C --- VGX family DDK
*
*  This file contains WATCOM DOS Extender dependent codes, which
*  works only with PCI boards in x86 systems.
*
*  Don't use this file for other OS, non-x86 system, and non-PCI.
* 
*******************************************************************/
#include <conio.h>
#include <dos.h>
#include <i86.h>
#include <stdlib.h>

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
    union REGS regs;
    unsigned long address = (unsigned long)phyAddr;

    /* Map the physical to linear addressing */
    regs.w.ax = 0x0800;
    regs.w.bx = (unsigned short) (address >> 16);
    regs.w.cx = (unsigned short) (address);
    regs.w.si = (unsigned short) (size >> 16);
    regs.w.di = (unsigned short) (size);
    int386(0x31, &regs, &regs);
    if (regs.w.cflag & INTR_CF)
    {
      return ((void *)0);
    }

    address = (((unsigned long) regs.w.bx) << 16) +
                (unsigned long) regs.w.cx;

    return ((void *) address);
}

/*
 * This function unmaps a linear logical address obtained by mapPhysicalAddress.
 * Return:
 *      0   - Success
 *     -1   - Fail
 */
long unmapPhysicalAddress(
    void *linearAddr          /* 32 bit linear address */
)
{
    union REGS regs;
    unsigned long address = (unsigned long)linearAddr;

    /* Map the physical to linear addressing */
    regs.w.ax = 0x0801;
    regs.w.bx = (unsigned short) (address >> 16);
    regs.w.cx = (unsigned short) (address);
    int386(0x31, &regs, &regs);
    if (regs.w.cflag & INTR_CF)
        return (-1);

    return 0;
}


/*
 * This function reads a DWord value from the PCI configuration space
 * of a specific PCI device.
 *
 * Inputs are the Vendor and device ID of the device in question.
 * Return: a DWord value.
 */
unsigned long readPCIDword(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum, /* If more than one device in system, device number are ordered as 0, 1, 2,... */
    unsigned short offset     /* Offset in configuration space to be read */
)
{
    union REGS regs;
    unsigned long base;

    /* Find the PCI card. */
    regs.w.ax = 0xB102;
    regs.w.cx = deviceId;
    regs.w.dx = vendorId;
    regs.w.si = deviceNum;
    int386(0x1A, &regs, &regs);
    if (regs.h.ah != 0)
    {
        return (-1);
    }

    /* Read the lower 16-bits of the base address. */
    regs.w.ax = 0xB109;
    regs.w.di = offset;
    int386(0x1A, &regs, &regs);
    if (regs.h.ah != 0)
    {
        return (-1);
    }
    base = regs.w.cx;

    /* Read the upper 16-bits of the base address. */
    regs.w.ax = 0xB109;
    regs.w.di = offset + 2;
    int386(0x1A, &regs, &regs);
    if (regs.h.ah != 0)
    {
        return (-1);
    }
    base |= regs.w.cx << 16;

    /* Return the base address. */
    return(base);
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
    union REGS regs;

    /* Find the PCI card. */
    regs.w.ax = 0xB102;
    regs.w.cx = deviceId;
    regs.w.dx = vendorId;
    regs.w.si = deviceNum;
    int386(0x1A, &regs, &regs);
    if (regs.h.ah != 0)
    {
        return(-1);
    }

    /* Read the 16-bits of the register. */
    regs.w.ax = 0xB109;
    regs.w.di = offset;
    int386(0x1A, &regs, &regs);
    if (regs.h.ah != 0)
    {
        return(-1);
    }

    /* Return the register value. */
    return(regs.w.cx);
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
    union REGS regs;

    /* Find the PCI card. */
    regs.w.ax = 0xB102;
    regs.w.cx = deviceId;
    regs.w.dx = vendorId;
    regs.w.si = deviceNum;
    int386(0x1A, &regs, &regs);
    if (regs.h.ah != 0)
    {
        return(-1);
    }

    /* Read the 8-bits of the register. */
    regs.w.ax = 0xB108;
    regs.w.di = offset;
    int386(0x1A, &regs, &regs);
    if (regs.h.ah != 0)
    {
        return(-1);
    }

    /* Return the register value. */
    return(regs.h.cl);
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
    unsigned short offset,   /* Offset in configuration space to be written */
    unsigned char value      /* To be written BYTE value */
)
{
    union REGS regs;

    /* Find the PCI card. */
    regs.w.ax = 0xB102;
    regs.w.cx = deviceId;
    regs.w.dx = vendorId;
    regs.w.si = deviceNum;
    int386(0x1A, &regs, &regs);

    if (regs.h.ah == 0)
    {
        /* Write the 8-bits of the register. */
        regs.w.ax = 0xB10B;
        regs.w.di = offset;
        regs.h.cl = value;
        int386(0x1A, &regs, &regs);
        return 0;
    }       

    return -1;
}

/*
 * This function writes a word value to the PCI configuration space
 * of a specific device.
 *
 * Inputs are the Vendor and device ID of the device in question.
 * Return:  0 = Success.
 *         -1 = Fail.
 */
long writePCIWord(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum, /* If more than one device in system, device number are ordered as 0, 1, 2,... */
    unsigned short offset,   /* Offset in configuration space to be written */
    unsigned short value     /* To be written BYTE value */
)
{
    union REGS regs;

    /* Find the PCI card. */
    regs.w.ax = 0xB102;
    regs.w.cx = deviceId;
    regs.w.dx = vendorId;
    regs.w.si = deviceNum;
    int386(0x1A, &regs, &regs);

    if (regs.h.ah == 0)
    {
        /* Write the 16-bits of the register. */
        regs.w.ax = 0xB10C;
        regs.w.di = offset;
        regs.w.cx = value;
        int386(0x1A, &regs, &regs);
        return 0;
    }       

    return -1;
}

/*
 * This function writes a dword value to the PCI configuration space
 * of a specific device.
 *
 * Inputs are the Vendor and device ID of the device in question.
 * Return:  0 = Success.
 *         -1 = Fail.
 */
long writePCIDword(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum, /* If more than one device in system, device number are ordered as 0, 1, 2,... */
    unsigned short offset,   /* Offset in configuration space to be written */
    unsigned long value     /* To be written BYTE value */
)
{
    union REGS regs;

    /* Find the PCI card. */
    regs.w.ax = 0xB102;
    regs.w.cx = deviceId;
    regs.w.dx = vendorId;
    regs.w.si = deviceNum;
    int386(0x1A, &regs, &regs);

    if (regs.h.ah == 0)
    {
        /* Write the lower 16-bits of the register. */
        regs.w.ax = 0xB10C;
        regs.w.di = offset;
        regs.w.cx = (unsigned short) value;
        int386(0x1A, &regs, &regs);
        
        /* Write the upper 16-bits of the register. */
        regs.w.ax = 0xB10C;
        regs.w.di = offset + 2;
        regs.w.cx = (unsigned short) (value >> 16);
        int386(0x1A, &regs, &regs);
        return 0;
    }       

    return -1;
}

/*
 * Read the IO Port.
 */
unsigned char readIO(
    unsigned short ioPort       /* IO Port */
)
{
    return inp(ioPort);
}

/*
 * Write to the IO Port.
 */
void writeIO(
    unsigned short ioPort,      /* IO Port */
    unsigned char value         /* Value to be written to the port */
)
{
    outp(ioPort, value);
}

/*******************************************************************
 * Time related functions
 * 
 * This implementation can be used for performance analysis or delay
 * function.
 *******************************************************************/

/* Tick count will be reset after midnight 24 hours. Therefore, when the tick count counter
   reset, it means that it has passed the 24 hours boundary. 
   A Timer tick occurs every 1,193,180 / 65536 (= ~18.20648) times per second. 
   The maximum tick count is calculated as follows: (24*3600*1000/54.9254) = ~1573040
 */
unsigned long getSystemTickCount()
{
    union REGS regs;
    unsigned long tickCount;

    /* Get the tick count. */
    regs.h.ah = 0x00;
    int386(0x1A, &regs, &regs);
    
    tickCount = (unsigned long)(regs.w.cx << 16) + (unsigned long)regs.w.dx;
    
    return tickCount;
}

/* Get current time in milliseconds. */
unsigned long getCurrentTime()
{
    union REGS regs;
    unsigned long milliseconds;

    /* Get the tick count. */
    regs.h.ah = 0x2C;
    int386(0x21, &regs, &regs);
    
    milliseconds = (((unsigned long)regs.h.ch * 3600 + 
                     (unsigned long)regs.h.cl * 60 +
                     (unsigned long)regs.h.dh) * 1000) +
                   (unsigned long)regs.h.dl * 10;
    
    return milliseconds;
}

/*******************************************************************
 * Interrupt implementation support from the OS
 * 
 * This implementation is used for handling the SM50x interrupt.
 *******************************************************************/

#define IRQ_STACK_SIZE      512

static void (interrupt far *pfnOldInterrupt)() = (void (interrupt far *))0;

void int_intercept(
    int intrNumber, 
    void (interrupt far *pfnHandler)(),
    unsigned short stackSize
)
{
	/* Get the interrupt number previous ISR */
	pfnOldInterrupt = _dos_getvect(intrNumber);

	/* Install the VGX interrupt handler to the designated 
       interrupt number */
	_dos_setvect(intrNumber, pfnHandler);
}

void int_restore(
    int intrNumber
)
{
    /* Restore the actual protected mode ISR of the original interrupt */
    _dos_setvect(intrNumber, pfnOldInterrupt);
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
    /* Return error if the handler does not exist. */
    if (irqNumber == (unsigned char)(-1))
        return (-1);
    
    if (irqNumber < 8)
    {
        /* IRQ0-IRQ7 occupy vectors INT08-INT0F */
        int_intercept(0x08 + irqNumber, pfnHandler, IRQ_STACK_SIZE);

        /* Enable the interrupt in Interrupt Mask register */
        outp(0x21, inp(0x21) & ~(0x01 << irqNumber));
    }
    else
    {
        /* IRQ8-IRQ15 occupy vectors INT70-INT77 */
        int_intercept(0x70 + (irqNumber - 8), pfnHandler, IRQ_STACK_SIZE);

        /* Enable the interrupt in Interrupt Mask register */
        outp(0xA1, inp(0xA1) & ~(0x01 << (irqNumber - 8)));
    }
    
    return 0;
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
    if (irqNumber < 8)
    {
        int_restore(0x08 + irqNumber);
    }
    else
    {
        int_restore(0x70 + (irqNumber - 8));
    }
    
    return 0;
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
    if (irqNumber >= 8)
    {
        /* Send EOI (End of Interrupt) Command to 8259 PIC 2
           Note: If using PIC 2, both EOI have to be sent to PIC 1
           and PIC 2.
         */
        outp(0xA0, 0x20);
    }

    /* Send EOI (End of Interrupt) Command to 8259 PIC 1 */
    outp(0x20, 0x20);
    
#if 0
    /* Chain to previous handler. Somehow, the chain does not seem to work.
       Need further investigation if necessary to chain the interrupt. 
       It might not be needed since we only have one process in DOS. */
    if (pfnOldInterrupt)
        _chain_intr (pfnOldInterrupt);
#endif
}

/*******************************************************************
 * Timer Interrupt implementation
 *******************************************************************/

/* Interrupt structure */
typedef struct _timer_interrupt_t
{
    struct _timer_interrupt_t *next;
    unsigned long totalTicks;
    unsigned long ticksCount;
    void (*handler)(void);
}
timer_interrupt_t;

#define TIMER_INTERRUPT         0x08

static timer_interrupt_t *timer_int_handlers = (timer_interrupt_t *)0;
static void (interrupt far *pfnOldTimerInterrupt)() = (void (interrupt far *))0;
 
void interrupt timerHandler()
{
    timer_interrupt_t *p;

    /* Walk all registered handlers for handlers */
    for (p = timer_int_handlers; p != ((timer_interrupt_t *)0); p = p->next)
    {
        /* Increment the tick count for each handlers */
        p->ticksCount++;
        
        /* Call the handler and reset the tickCounts when the function has waited
           for totalTicks number of tick count */
        if (p->ticksCount == p->totalTicks)
        {
            /* Call the function */
            p->handler();
            
            /* Reset the tick count */
            p->ticksCount = 0;
        }
    }

    /* Send EOI to PIC 1 */
    outp(0x20, 0x20);

#if 0
    /* Chain to previous handler. Somehow, the chain does not seem to work.
       Need further investigation if necessary to chain the interrupt.
       It might not be needed in DOS since we only have one process. */
    if (pfnOldTimerInterrupt)
        _chain_intr (pfnOldTimerInterrupt);
#endif
}

/* 
 * Register interrupt handler
 *
 * Output:
 *      0   - Success
 *      -1  - Out of memory
 *
 * Note:
 *      The interrupt is happens every 55ms, or about 18.2 ticks per second. 
 */
short registerTimerHandler(
    void (*handler)(void),
    unsigned long totalTicks        /* Total number of ticks to wait */
) 
{
    timer_interrupt_t *p;

    /* Allocate a new interrupt structure */
    p = (timer_interrupt_t *) malloc(sizeof(timer_interrupt_t));
    if (p == ((timer_interrupt_t *)0))
    {
        /* No more memory */
        return(-1);
    }

    /* If this is the first interrupt handler, intercept the correct IRQ */
    if (timer_int_handlers == ((timer_interrupt_t *)0))
    {
        /* Get the previous ISR for the timer interrupt */
	    pfnOldTimerInterrupt = _dos_getvect(TIMER_INTERRUPT);

	    /* Install the new timer interrupt handler */
	    _dos_setvect (TIMER_INTERRUPT, timerHandler);

        /* Enable the interrupt in Interrupt Mask register */
        outp(0x21, inp(0x21) & ~0x01);
    }


    /* Fill interrupt structure */
    p->next = timer_int_handlers;
    p->totalTicks = totalTicks;
    p->ticksCount = 0;
    p->handler = handler;
    timer_int_handlers = p;

    return 0;
}

/* 
 * Unregister a registered interrupt handler
 *
 * Output:
 *      0   - Success
 *      -1  - Handler is not found 
 */
short unregisterTimerHandler(
    void (*handler)(void)       /* Interrupt function to be unregistered from the SM50x interrupt */
)
{
    timer_interrupt_t *p, *prev;

    /* Find the requested handle to unregister */
    for (p = timer_int_handlers, prev = ((timer_interrupt_t *)0); p != ((timer_interrupt_t *)0); prev = p, p = p->next)
    {
        if (p->handler == handler)
        {
            /* Remove the interrupt handler */
            if (prev == ((timer_interrupt_t *)0))
            {
                timer_int_handlers = p->next;
            }
            else
            {
                prev->next = p->next;
            }
            free(p);

            /* If this was the last interrupt handler, remove the IRQ handler */
            if (timer_int_handlers == ((timer_interrupt_t *)0))
            {
                /* Restore the actual protected mode ISR of the original timer interrupt */
                _dos_setvect(TIMER_INTERRUPT, pfnOldTimerInterrupt);
            }

            /* Success */
            return 0;
        }
    }

    /* Oops, handler is not registered */
    return -1;
}

/*******************************************************************
 * COM Port implementation
 * 
 * This implementation is used by Debug module to send any 
 * debugging messages to other system through serial port.
 *******************************************************************/
#include <BIOS.H>

static unsigned char gComPortIndex = 0;

/* 
 * Initialize the serial port.
 *
 * Parameters:
 *      comPortIndex    - Serial Port Index
 *      baudRate        - The communication speed to be set
 *      dataSize        - Number of bits per characters
 *      parity          - Error checking
 *      stopBit         - Number of bits used as the end of each character
 *      flowCtrl        - Serial port Flow Control (handshake)
 *
 * Returns:
 *       0  - Success
 *      -1  - Fail
 */
long comInit(
    unsigned char comPortIndex,
    baud_rate_t baudRate,
    data_size_t dataSize,
    parity_t parity,
    stop_bit_t stopBit,
    flow_ctrl_t flowCtrl
)
{
    unsigned short dcb = 0;

    /* Process the baud Rate */
    switch (baudRate)
    {
        case COM_2400:
            dcb |= _COM_2400;
            break;
        case COM_4800:
            dcb |= _COM_4800;
            break;
        case COM_9600:
            dcb |= _COM_9600;
            break;
        default:
            return -1;
    }
    
    /* Process the data size */
    switch (dataSize)
    {
        case DATA_SIZE_8:
            dcb |= _COM_CHR8;
            break;
        case DATA_SIZE_7:
            dcb |= _COM_CHR7;
            break;
        default:
            return -1;
    }
   
    /* Process the parity */
    switch (parity)
    {
        case PARITY_NONE:
            dcb |= _COM_NOPARITY;
            break;
        case PARITY_ODD:
            dcb |= _COM_ODDPARITY;
            break;
        case PARITY_EVEN:
            dcb |= _COM_EVENPARITY;
            break;
        case PARITY_SPACE:
            dcb |= _COM_SPACEPARITY;
            break;            
        default:
            return -1;
    }
    
    /* Process the stop bit */
    switch (stopBit)
    {
        case STOP_BIT_1:
            dcb |= _COM_STOP1;
            break;
        case STOP_BIT_2:
            dcb |= _COM_STOP2;
            break;
        default:
            return -1;
    }
    
    /* NOTE: There is no flow control supported in _bios_serialcom. 
       The option is provided for other OS implementation */
    
    /* Save the port index */
    gComPortIndex = comPortIndex;
    
    /* Call the BIOS to initialize the COM Port */
    _bios_serialcom(_COM_INIT, comPortIndex, dcb);
    
    return 0;
}

/* 
 * Send data out to through the serial port.
 *
 * Parameters:
 *      pszPrintBuffer  - Pointer to a buffer which data to be sent out
 *      length          - Number of characters to be sent out.
 *
 * Returns:
 *      Number of characters that are actually sent out.
 */
unsigned long comWrite(
    char* pszPrintBuffer,
    unsigned long length
)
{
    unsigned long index, byteSent = 0;
    
    for (index = 0; index < length; index++)
    {
        /* Send the character one by one to the Serial port */    
        if (_bios_serialcom(_COM_SEND, gComPortIndex, pszPrintBuffer[index]) == 1)
            byteSent++;
    }
    
    return byteSent;
}

/* 
 * Close the serial communication port.
 */
void comClose()
{
    /* Currently is not used */
}
