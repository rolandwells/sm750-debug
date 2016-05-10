/*******************************************************************
* 
*         Copyright (c) 2009 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  dviint.c --- SM750 DDK 
*  This file contains the API function for DVI display.
* 
*******************************************************************/
#include <stdlib.h>
#include "defs.h"
#include "dvi.h"
#include "gpioint.h"

#include "ddkdebug.h"

//#define MAXIMUM_HOOKED_ISR                  5

/* GPIO Pin used for the DVI Hot Plug detection. */
#define DVI_HOT_PLUG_GPIO_PIN               25

typedef struct _dvi_hot_plug_interrupt_t
{
    struct _dvi_hot_plug_interrupt_t *next;
    void (*handler)(void);
}
dvi_hot_plug_interrupt_t;

#if 1
static dvi_hot_plug_interrupt_t *g_pDVIHotPlugIntHandlers = ((dvi_hot_plug_interrupt_t *)0);   
#else
/* Maybe for later use */
static dvi_hot_plug_interrupt_t g_pDVIHotPlugIntHandlers[MAX_SMI_DEVICE][MAXIMUM_HOOKED_ISR] = 
{
    { 
        { ((dvi_hot_plug_interrupt_t *)0) }, 
        { ((dvi_hot_plug_interrupt_t *)0) }, 
        { ((dvi_hot_plug_interrupt_t *)0) }, 
        { ((dvi_hot_plug_interrupt_t *)0) }
    },
    { 
        { ((dvi_hot_plug_interrupt_t *)0) }, 
        { ((dvi_hot_plug_interrupt_t *)0) }, 
        { ((dvi_hot_plug_interrupt_t *)0) }, 
        { ((dvi_hot_plug_interrupt_t *)0) } 
    },
    { 
        { ((dvi_hot_plug_interrupt_t *)0) }, 
        { ((dvi_hot_plug_interrupt_t *)0) }, 
        { ((dvi_hot_plug_interrupt_t *)0) }, 
        { ((dvi_hot_plug_interrupt_t *)0) } 
    },
    { 
        { ((dvi_hot_plug_interrupt_t *)0) }, 
        { ((dvi_hot_plug_interrupt_t *)0) }, 
        { ((dvi_hot_plug_interrupt_t *)0) }, 
        { ((dvi_hot_plug_interrupt_t *)0) } 
    },
};
#endif

void hotPlugISR()
{
    dvi_hot_plug_interrupt_t *pfnHandler;
    unsigned char detectReg;
    dvi_ctrl_device_t *pDviCtrlDevice;
    
    pDviCtrlDevice = getDviCtrl();

    if ((pDviCtrlDevice != (dvi_ctrl_device_t *)0) &&
        (pDviCtrlDevice->pfnCheckInterrupt != (PFN_DVICTRL_CHECKINTERRUPT) 0) &&
        (pDviCtrlDevice->pfnClearInterrupt != (PFN_DVICTRL_CLEARINTERRUPT) 0))
    {
        if (pDviCtrlDevice->pfnCheckInterrupt() > 0)
        {
            /* Walk all registered handlers for handlers that support this interrupt status */
            for (pfnHandler = g_pDVIHotPlugIntHandlers; pfnHandler != ((dvi_hot_plug_interrupt_t *)0); pfnHandler = pfnHandler->next)
                pfnHandler->handler();
            
            /* Clear the MDI Interrupt */
            pDviCtrlDevice->pfnClearInterrupt();
        }
    }
}

/*  
 *  hookDVIHotPlugInterrupt
 *      Register DVI Hot Plug Interrupt function.
 */
long hookDVIHotPlugInterrupt(
    void (*handler)(void)
)
{
    dvi_hot_plug_interrupt_t *pfnNewHandler, *pfnHandler;
    unsigned short returnValue = 0;
    dvi_ctrl_device_t *pDviCtrlDevice;
    
    pDviCtrlDevice = getDviCtrl();

    /* Allocate a new interrupt structure */
    pfnNewHandler = (dvi_hot_plug_interrupt_t *)malloc(sizeof(dvi_hot_plug_interrupt_t));
    if (pfnNewHandler == ((dvi_hot_plug_interrupt_t *)0))
    {
        /* No more memory */
        return (-1);
    }

    /* Make sure that it has not been registered more than once */
    for (pfnHandler = g_pDVIHotPlugIntHandlers; pfnHandler != ((dvi_hot_plug_interrupt_t *)0); pfnHandler = pfnHandler->next)
    {
        if (pfnHandler->handler == handler)
            return (-1);                   /* Duplicate entry */
    }
        
    /* If this is the first interrupt handler, register this Hot plug ISR */
    if (g_pDVIHotPlugIntHandlers == ((dvi_hot_plug_interrupt_t *)0))
    {
        if ((pDviCtrlDevice != (dvi_ctrl_device_t *)0) &&
            (pDviCtrlDevice->pfnClearInterrupt != (PFN_DVICTRL_CLEARINTERRUPT) 0))
        {
            /* Clear the interrupt first. */
            pDviCtrlDevice->pfnClearInterrupt();
    
            /* Hook to the GPIO setup for this DVI Hot plug. */
            returnValue = hookGPIOInterrupt(DVI_HOT_PLUG_GPIO_PIN, GPIO_INTR_EDGE_TRIGGERED, GPIO_INTR_ACTIVE_LOW, hotPlugISR);
        }
    }

    if (returnValue == 0)
    {
        /* Fill interrupt structure. */
        pfnNewHandler->next = g_pDVIHotPlugIntHandlers;
        pfnNewHandler->handler = handler;
        g_pDVIHotPlugIntHandlers = pfnNewHandler;
    }
    
    return returnValue;
}

/*  
 *  unhookDVIHotPlugInterrupt
 *      unregister DVI Hot Plug Interrupt function.
 */
long unhookDVIHotPlugInterrupt(
    void (*handler)(void)
)
{
    dvi_hot_plug_interrupt_t *pfnHandler, *prev;
    unsigned long mask;
    dvi_ctrl_device_t *pDviCtrlDevice;
    
    pDviCtrlDevice = getDviCtrl();

    /* Find the requested handle to unregister */
    for (pfnHandler = g_pDVIHotPlugIntHandlers, prev = ((dvi_hot_plug_interrupt_t *)0); 
         pfnHandler != ((dvi_hot_plug_interrupt_t *)0); 
         prev = pfnHandler, pfnHandler = pfnHandler->next)
    {
        if (pfnHandler->handler == handler)
        {
            /* Remove the interrupt handler */
            if (prev == ((dvi_hot_plug_interrupt_t *)0))
                g_pDVIHotPlugIntHandlers = pfnHandler->next;
            else
                prev->next = pfnHandler->next;

            free(pfnHandler);
            
            /* If this was the last interrupt handler, remove the IRQ handler */
            if (g_pDVIHotPlugIntHandlers == ((dvi_hot_plug_interrupt_t *)0))
            {
                if ((pDviCtrlDevice != (dvi_ctrl_device_t *)0) &&
                    (pDviCtrlDevice->pfnClearInterrupt != (PFN_DVICTRL_CLEARINTERRUPT) 0))
                {
                    /* Clear the interrupt first. */
                    pDviCtrlDevice->pfnClearInterrupt();
                }
                    
                /* Unhook the GPIO Interrupt */
                unhookGPIOInterrupt(DVI_HOT_PLUG_GPIO_PIN, hotPlugISR);
            }
            
            /* Success */
            return (0);
        }
    }

    /* Oops, handler is not registered */
    return (-1);
}


