/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  DISPINT.C --- Voyager GX SDK 
*  This file contains the source code for the panel and CRT VSync
*  interrupt functions.
* 
*******************************************************************/
#include <stdlib.h>
#include "defs.h"
#include "hardware.h"
#include "chip.h"
#include "dispint.h"

typedef struct _vsync_interrupt_t
{
    struct _vsync_interrupt_t *next;
    void (*handler)(void);
}
vsync_interrupt_t;

static vsync_interrupt_t *primaryIntHandlers = ((vsync_interrupt_t *)0);
static vsync_interrupt_t *secondaryIntHandlers = ((vsync_interrupt_t *)0);

/* Primary Output VSync Interrupt Service Routine */
void primaryOutputVSyncISR(
    uint32_t status
)
{
    vsync_interrupt_t *pfnHandler;
   
#ifdef LINUX    /* To be removed once the Linux interrupt can be set up. */
    if (FIELD_GET(peekRegisterDWord(RAW_INT), RAW_INT, PRIMARY_VSYNC) ==
        RAW_INT_PRIMARY_VSYNC_ACTIVE)
#else
    if (FIELD_GET(status, INT_STATUS, PRIMARY_VSYNC) == INT_STATUS_PRIMARY_VSYNC_ACTIVE)
#endif
    {
        /* Walk all registered handlers for handlers that support this interrupt status */
        for (pfnHandler = primaryIntHandlers; pfnHandler != ((vsync_interrupt_t *)0); pfnHandler = pfnHandler->next)
            pfnHandler->handler();
            
        /* Clear the panel VSync Interrupt */
        pokeRegisterDWord(RAW_INT, FIELD_SET(0, RAW_INT, PRIMARY_VSYNC, CLEAR));
    }            
}

/* Secondary output VSync Interrupt Service Routine */
void secondaryOutputVSyncISR(
    uint32_t status
)
{
    vsync_interrupt_t *pfnHandler;
    
#ifdef LINUX    /* To be removed once the Linux interrupt can be set up. */
    if (FIELD_GET(peekRegisterDWord(RAW_INT), RAW_INT, SECONDARY_VSYNC) ==
        RAW_INT_SECONDARY_VSYNC_ACTIVE)
#else
    if (FIELD_GET(status, INT_STATUS, SECONDARY_VSYNC) == INT_STATUS_SECONDARY_VSYNC_ACTIVE)
#endif
    {
        /* Walk all registered handlers for handlers that support this interrupt status */
        for (pfnHandler = secondaryIntHandlers; pfnHandler != ((vsync_interrupt_t *)0); pfnHandler = pfnHandler->next)
            pfnHandler->handler();

        /* Clear the CRT VSync Interrupt */
        pokeRegisterDWord(RAW_INT, FIELD_SET(0, RAW_INT, SECONDARY_VSYNC, CLEAR));            
    }            
}

/* Register Vertical Sync Interrupt function */
short hookVSyncInterrupt(
    disp_control_t dispControl,
    void (*handler)(void)
)
{
    vsync_interrupt_t *pfnNewHandler, *pfnHandler, *pfnInterruptHandler;
    unsigned short returnValue = 0;
    uint32_t interruptMask;

    /* Allocate a memory for new interrupt structure */
    pfnNewHandler = (vsync_interrupt_t *)malloc(sizeof(vsync_interrupt_t));
    if (pfnNewHandler == ((vsync_interrupt_t *)0))
    {
        /* No more memory */
        return (-1);
    }
    
    /* Get the right pointer to the interrupt handler */
    if (dispControl == SECONDARY_CTRL)
        pfnInterruptHandler = secondaryIntHandlers;
    else
        pfnInterruptHandler = primaryIntHandlers;
    
    /* Make sure that it has not been register more than once */
    for (pfnHandler = pfnInterruptHandler; pfnHandler != ((vsync_interrupt_t *)0); pfnHandler = pfnHandler->next)
    {
        if (pfnHandler->handler == handler)
            return (-1);                   /* Duplicate entry */
    }
        
    /* If this is the first interrupt handler, register this VSync ISR */
    if (pfnInterruptHandler == ((vsync_interrupt_t *)0))
    {
        returnValue = registerHandler((dispControl == SECONDARY_CTRL) ? secondaryOutputVSyncISR : primaryOutputVSyncISR, 
                                      (dispControl == SECONDARY_CTRL) ? 
                                          FIELD_SET(0, INT_MASK, SECONDARY_VSYNC, ENABLE) :
                                          FIELD_SET(0, INT_MASK, PRIMARY_VSYNC, ENABLE));
    }

    if (returnValue == 0)
    {
        /* Fill interrupt structure. */
        pfnNewHandler->next = pfnInterruptHandler;
        pfnNewHandler->handler = handler;
        pfnInterruptHandler = pfnNewHandler;
    }
    
    /* Update the actual handler */
    if (dispControl == SECONDARY_CTRL)
        secondaryIntHandlers = pfnInterruptHandler;
    else
        primaryIntHandlers = pfnInterruptHandler;
    
    return returnValue;
}

/* Unregister a registered panel VSync interrupt handler */
short unhookVSyncInterrupt(
    disp_control_t dispControl,
    void (*handler)(void)
)
{
    vsync_interrupt_t *pfnHandler, *prev, *pfnInterruptHandler;
    
    if (dispControl == SECONDARY_CTRL)
        pfnInterruptHandler = secondaryIntHandlers;
    else
        pfnInterruptHandler = primaryIntHandlers;

    /* Find the requested handle to unregister */
    for (pfnHandler = pfnInterruptHandler, prev = ((vsync_interrupt_t *)0); 
         pfnHandler != ((vsync_interrupt_t *)0); 
         prev = pfnHandler, pfnHandler = pfnHandler->next)
    {
        if (pfnHandler->handler == handler)
        {
            /* Remove the interrupt handler */
            if (prev == ((vsync_interrupt_t *)0))
            {
                pfnInterruptHandler = pfnHandler->next;
                
                /* Update the actual handler */
                if (dispControl == SECONDARY_CTRL)
                    secondaryIntHandlers = pfnInterruptHandler;
                else
                    primaryIntHandlers = pfnInterruptHandler;
            }
            else
            {
                prev->next = pfnHandler->next;
            }
            free(pfnHandler);
            
            /* If this was the last interrupt handler, remove the IRQ handler */
            if (pfnInterruptHandler == ((vsync_interrupt_t *)0))
                unregisterHandler((dispControl == SECONDARY_CTRL) ? secondaryOutputVSyncISR : primaryOutputVSyncISR);
            
            /* Success */
            return (0);
        }
    }

    /* Oops, handler is not registered */
    return (-1);
}

