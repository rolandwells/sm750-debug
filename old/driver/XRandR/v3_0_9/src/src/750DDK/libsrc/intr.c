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
#include <stdlib.h>
#include "defs.h"
#include "hardware.h"
#include "os.h"

#include "ddkdebug.h"

/* Interrupt structure */
typedef struct _interrupt_t
{
    struct _interrupt_t *next;
    unsigned long mask;
    void (*handler)(unsigned long status);
}
interrupt_t;

static interrupt_t *int_handlers = (interrupt_t *)0;

/* 
 * IRQ Handler.
 */
//void interrupt far irqHandler()
void irqHandler()
{
    unsigned long status, intMask;
    interrupt_t *p;
    
#ifdef USE_IO_PORT
    /* IMPORTANT!!!
            The port index and the MMIO Registers need to be "saved and restored"
            during the interrupt since they will be changed by the ISR.
     */
    unsigned char currentCRTIndex, index;
    unsigned char ioMMIOData[8];

    /* Save the current index in the CRT Register first */
    currentCRTIndex = readIO(0x3D4);
        
    /* Save IO Registers that are used by the MMIO Register including their values
       The register indexes are: 0x80, 0x81, 0x82, 0x85, 0x86, and 0x87. There is
       no need to save index 0x84, since this is the trigger of either read or write.
     */
    for (index = 0; index < 8; index++)
    {
        /* Skip 0x83 (not used) and 0x84 (trigger register) */
        if ((index == 0x03) || (index == 0x04))
            continue;
            
        ioMMIOData[index] = peekIO(CRT_REGISTER, 0x80 + index);
    } 
#endif    
    /* Save the interrupt mask register to be restored after the ISR. */
    intMask = peekRegisterDWord(INT_MASK);
    
    /* Get interrupt status */
    status = peekRegisterDWord(INT_STATUS);
    
    /* Unmask all interrupts to prevent further interrupt. */
    pokeRegisterDWord(INT_MASK, 0);
    
    /* Walk all registered handlers for handlers that support this interrupt */
    /* status */
    for (p = int_handlers; p != ((interrupt_t *)0); p = p->next)
    {
        if (p->mask & status)
        {
            p->handler(status);
        }
    }
    
    /* Restore the interrupt mask again before sending the EOI. */
    pokeRegisterDWord(INT_MASK, intMask);

#ifdef USE_IO_PORT
    /* Restore the IO Registers value that are used by the MMIO Register.
       Please note that this routine should be done after restoring the interrupt
       mask since pokeRegisterDWord is using the IO Port when "USE_IO_PORT" 
       is defined. */
    for (index = 0; index < 8; index++)
    {
        /* Again, skip 0x83 and 0x84 */
        if ((index == 0x03) || (index == 0x04))
            continue;
            
        pokeIO(CRT_REGISTER, 0x80 + index, ioMMIOData[index]);
    }
    
    /* Restore the CRT Register Index */
    writeIO(0x3D4, currentCRTIndex);
#endif
    
    /* End of IRQ */
    interruptEOI(getIRQ());
}

/* 
 * Count number of interrupt handler that use a specific given mask 
 */
short countMask(
    unsigned long mask
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
    unsigned long mask_on, 
    unsigned long mask_off
)
{
    unsigned long mask;

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
    void (*handler)(unsigned long status),      /* Interrupt function to be registered to the SM50x interrupt */ 
    unsigned long mask                          /* SM50x interrupt mask */
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
        /* Register Interrupt */
        if (registerInterrupt(getIRQ(), irqHandler) != 0)
            return (-1);
    }

    /* Fill interrupt structure */
    p->next = int_handlers;
    p->handler = handler;
    p->mask = mask;
    int_handlers = p;

#ifndef LINUX   /* Enable this back once the interrupt can be hooked properly in LINUX */
    /* Enable interrupt mask */
    setIntMask(mask, 0);
#endif

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
    void (*handler)(unsigned long status)       /* Interrupt function to be unregistered from the SM50x interrupt */
)
{
    interrupt_t *p, *prev;
    unsigned long mask;

    /* Find the requested handle to unregister */
    for (p = int_handlers, prev = ((interrupt_t *)0); p != ((interrupt_t *)0); prev = p, p = p->next)
    {
        if (p->handler == handler)
        {
        
#ifndef LINUX   /* Enable this back once the interrupt can be hooked properly in LINUX */
            /* Now, we need to disable the interrupt masks if there are no */
            /* other handlers using that mask */
            for (mask = 1; mask != 0; mask <<= 1)
            {
                /* If the handler was using this mask, and there is nobody else */
                /* claiming this mask, disable it */
                if ((p->mask & mask) && (countMask(mask) == 1))
                {
                    setIntMask(0, mask);
                }
            }
#endif

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
                /* Unregister IRQ */
                if (unregisterInterrupt(getIRQ()) != 0)
                    return (-1);
            }

            /* Success */
            return 0;
        }
    }

    /* Oops, handler is not registered */
    return (-1);
}
