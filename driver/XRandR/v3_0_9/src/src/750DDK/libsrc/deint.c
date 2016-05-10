/*
 * Voyager GX SDK
 *
 * deint.c
 *
 * This file contains the source code for the drawing engine interrupt (2D and CSC).
 *
 */
#include <malloc.h>
#include "defs.h"
#include "hardware.h"
#include "os.h"

#include "ddkdebug.h"

typedef struct _de_interrupt_t
{
    struct _de_interrupt_t *next;
    void (*handler)(void);
    uint32_t deMask;               /* 0 = 2D Engine, 1 = CSC */
}
de_interrupt_t;

static de_interrupt_t *pDEIntHandlers = ((de_interrupt_t *)0);

/* Drawing Engine Interrupt Service Routine */
void deISR(
    uint32_t status
)
{
    de_interrupt_t *pfnHandler;
    uint32_t deStatus;
   
    if (FIELD_GET(status, INT_STATUS, DE) == INT_STATUS_DE_ACTIVE)
    {
        deStatus = peekRegisterDWord(DE_STATUS);
        
        /* Walk all registered handlers for handlers that support this interrupt status */
        for (pfnHandler = pDEIntHandlers; pfnHandler != ((de_interrupt_t *)0); pfnHandler = pfnHandler->next)
        {
            if (deStatus & pfnHandler->deMask)
                pfnHandler->handler();
        }
            
        if (FIELD_GET(deStatus, DE_STATUS, 2D) == DE_STATUS_2D_ACTIVE) 
        {
            pokeRegisterDWord(DE_STATUS, 
                FIELD_SET(peekRegisterDWord(DE_STATUS), DE_STATUS, 2D, CLEAR));
        }
        
        if (FIELD_GET(deStatus, DE_STATUS, CSC) == DE_STATUS_CSC_ACTIVE)
        {
            pokeRegisterDWord(DE_STATUS, 
                FIELD_SET(peekRegisterDWord(DE_STATUS), DE_STATUS, CSC, CLEAR));
        }
    }            
}

/*
 * This is the main interrupt hook for drawing engine.
 * It hooks 2D and CSC engine. 
 * Note:
 *      To hook 2D Engine, use hook2DInterrupt.
 *      To hook CSC Engine, use hookCSCInterrupt.
 */
int32_t hookDEInterrupt(
    uint32_t deMask,
    void (*handler)(void)
)
{
    de_interrupt_t *pfnNewHandler, *pfnHandler;
    unsigned short returnValue = 0;

    /* Allocate a new interrupt structure */
    pfnNewHandler = (de_interrupt_t *)malloc(sizeof(de_interrupt_t));
    if (pfnNewHandler == ((de_interrupt_t *)0))
    {
        /* No more memory */
        return (-1);
    }

    /* Make sure that it has not been register more than once */
    for (pfnHandler = pDEIntHandlers; pfnHandler != ((de_interrupt_t *)0); pfnHandler = pfnHandler->next)
    {
        if (pfnHandler->handler == handler)
            return (-1);                   /* Duplicate entry */
    }
        
    /* If this is the first interrupt handler, register this panel VSync ISR */
    if (pDEIntHandlers == ((de_interrupt_t *)0))
        returnValue = registerHandler(deISR, FIELD_SET(0, INT_MASK, DE, ENABLE));

    if (returnValue == 0)
    {
        /* Fill interrupt structure. */
        pfnNewHandler->next = pDEIntHandlers;
        pfnNewHandler->handler = handler;
        pfnNewHandler->deMask = (deMask == 0) ?
                                    FIELD_SET(0, DE_STATUS, 2D, ACTIVE) :
                                    FIELD_SET(0, DE_STATUS, CSC, ACTIVE);
        pDEIntHandlers = pfnNewHandler;
    }
   
    return returnValue;
}

/*
 * This function un-register 2D Interrupt handler.
 */
int32_t unhookDEInterrupt(
    uint32_t deMask,
    void (*handler)(void)
)
{
    de_interrupt_t *pfnHandler, *prev;
    uint32_t mask;

    if (deMask == 0)
        mask = FIELD_SET(0, DE_STATUS, 2D, ACTIVE);
    else
        mask = FIELD_SET(0, DE_STATUS, CSC, ACTIVE);

    /* Find the requested handle to unregister */
    for (pfnHandler = pDEIntHandlers, prev = ((de_interrupt_t *)0); 
         pfnHandler != ((de_interrupt_t *)0); 
         prev = pfnHandler, pfnHandler = pfnHandler->next)
    {
        if ((pfnHandler->deMask == mask) && (pfnHandler->handler == handler))
        {
            /* Remove the interrupt handler */
            if (prev == ((de_interrupt_t *)0))
                pDEIntHandlers = pfnHandler->next;
            else
                prev->next = pfnHandler->next;

            free(pfnHandler);
            
            /* If this was the last interrupt handler, remove the IRQ handler */
            if (pDEIntHandlers == ((de_interrupt_t *)0))
                unregisterHandler(deISR);
            
            /* Success */
            return (0);
        }
    }

    /* Oops, handler is not registered */
    return (-1);
}

/*
 * This function register 2D Interrupt handler.
 */
int32_t hook2DInterrupt(
    void (*handler)(void)
)
{
    return (hookDEInterrupt(0, handler));
}

/*
 * This function un-register 2D Interrupt handler.
 */
int32_t unhook2DInterrupt(
    void (*handler)(void)
)
{
    return (unhookDEInterrupt(0, handler));
}

/*
 * This function register CSC Interrupt handler.
 */
int32_t hookCSCInterrupt(
    void (*handler)(void)
)
{
    return (hookDEInterrupt(1, handler));
}

/*
 * This function un-register CSC Interrupt handler.
 */
int32_t unhookCSCInterrupt(
    void (*handler)(void)
)
{
    return (unhookDEInterrupt(1, handler));
}
