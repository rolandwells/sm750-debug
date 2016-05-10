/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  ssp.c --- Synchronouse Serial Port 
*  This file contains the source code for the SSP0/1.
* 
*******************************************************************/
#include <stdlib.h>
#include "defs.h"
#include "clock.h"
#include "hardware.h"
#include "os.h"
#include "power.h"
#include "intr.h"
#include "gpioint.h"

#include "ddkdebug.h"

typedef struct _gpio_interrupt_t
{
    struct _gpio_interrupt_t *next;
    void (*handler)(void);
}
gpio_interrupt_t;

static gpio_interrupt_t *gpio25IntHandlers = ((gpio_interrupt_t *)0);
static gpio_interrupt_t *gpio26IntHandlers = ((gpio_interrupt_t *)0);
static gpio_interrupt_t *gpio27IntHandlers = ((gpio_interrupt_t *)0);
static gpio_interrupt_t *gpio28IntHandlers = ((gpio_interrupt_t *)0);
static gpio_interrupt_t *gpio29IntHandlers = ((gpio_interrupt_t *)0);
static gpio_interrupt_t *gpio30IntHandlers = ((gpio_interrupt_t *)0);
static gpio_interrupt_t *gpio31IntHandlers = ((gpio_interrupt_t *)0);

/*
 *  gpioClearInterrupt
 *      This function clears the given GPIO pin interrupt signal
 */
void gpioClearInterrupt(
    unsigned long gpioInterruptPin
)
{
    /* Clear the GPIO Interrupt */
    pokeRegisterDWord(GPIO_INTERRUPT_STATUS, 1 << (gpioInterruptPin - 9));
}

/*
 *  gpioInterruptInit
 *      This function initializes the GPIO pin as an interrupt input signal.
 *
 *  Input:
 *          gpioInterruptPin    - GPIO pin that will be used as interrupt
 *          triggerType         - Interrupt trigger type, either edge-trigger or level-trigger.
 *          activeState         - Interrupt active state, either active low or active high.
 */
long gpioInterruptInit(
    unsigned long gpioInterruptPin,         /* GPIO Interrupt Pin to be setup as Interrupt */
    gpio_intr_trigger_t triggerType,        /* Trigger Type, either edge-triggered or level-triggered. */
    gpio_intr_active_state_t activeState    /* Interrupt Active State */
)
{
    unsigned long gpioMuxValue, gpioIntrSetupValue, gpioDataDirectionValue;
    
    /* Only GPIO Pin 25 to 31 supports interrupt */
    if ((gpioInterruptPin < 25) || (gpioInterruptPin > 31))
        return (-1);
    
    /* Enable the GPIO bit. */
    pokeRegisterDWord(GPIO_MUX, peekRegisterDWord(GPIO_MUX) & ~(1 << gpioInterruptPin));
    
    /* Enable the direction. 
       This step might not be necessary, as long as it is setup as interrupt, then 
       the direction will always be input. */
    pokeRegisterDWord(GPIO_DATA_DIRECTION, peekRegisterDWord(GPIO_DATA_DIRECTION) & ~(1 << gpioInterruptPin));
    
    /* Enable the Input as an interrupt */
    gpioIntrSetupValue = peekRegisterDWord(GPIO_INTERRUPT_SETUP);
    gpioIntrSetupValue = gpioIntrSetupValue | (1 << (gpioInterruptPin - 25));
    
    /* Set up the new interrupt active state */
    if (activeState == GPIO_INTR_ACTIVE_LOW)
        gpioIntrSetupValue = gpioIntrSetupValue & ~(1 << (gpioInterruptPin - 17));
    else 
        gpioIntrSetupValue = gpioIntrSetupValue | (1 << (gpioInterruptPin - 17));
    
    /* Set up the new interrupt trigger type */
    if (triggerType == GPIO_INTR_EDGE_TRIGGERED)
        gpioIntrSetupValue = gpioIntrSetupValue & ~(1 << (gpioInterruptPin - 9));
    else
        gpioIntrSetupValue = gpioIntrSetupValue | (1 << (gpioInterruptPin - 9));
        
    pokeRegisterDWord(GPIO_INTERRUPT_SETUP, gpioIntrSetupValue);
    
    /* Clear the GPIO interrupt first */
    gpioClearInterrupt(gpioInterruptPin);

    return 0;    
}

/*
 *  gpioInterruptExit
 *      This function disable the previously setup GPIO as interrupt signal
 *
 *  Input:
 *          gpioInterruptPin    - GPIO pin used for interrupt signal
 */
void gpioInterruptExit(
    unsigned long gpioInterruptPin
)
{
    unsigned long gpioIntrSetupValue;
    
    /* Clear the GPIO interrupt first */
    gpioClearInterrupt(gpioInterruptPin);
    
    gpioIntrSetupValue = peekRegisterDWord(GPIO_INTERRUPT_SETUP);
    gpioIntrSetupValue = gpioIntrSetupValue & ~(1 << (gpioInterruptPin - 25));
}

/*
 * This ISR function calls the function that hooks to this GPIO interrupt 
 */
void gpio25ISR(
    unsigned long status
)
{
    gpio_interrupt_t *pfnHandler;
    
    if (FIELD_GET(status, INT_STATUS, GPIO25) == INT_STATUS_GPIO25_ACTIVE)
    {
        /* Walk all registered handlers for handlers that support this interrupt status 
           The function hooked to this ISR needs to check the condition of the interrupt
           and do accordingly to reset the interrupt. Like reading the data when the
           receive FIFO is full or transmit the data when the transmit FIFO is full.
         */
        for (pfnHandler = gpio25IntHandlers; pfnHandler != ((gpio_interrupt_t *)0); pfnHandler = pfnHandler->next)
            pfnHandler->handler();
            
        /* Clear the GPIO 25 Interrupt */
        gpioClearInterrupt(25);
    }            
}

/*
 * This ISR function calls the function that hooks to this GPIO interrupt 
 */
void gpio26ISR(
    unsigned long status
)
{
    gpio_interrupt_t *pfnHandler;
    
    if (FIELD_GET(status, INT_STATUS, GPIO26) == INT_STATUS_GPIO26_ACTIVE)
    {
        /* Walk all registered handlers for handlers that support this interrupt status 
           The function hooked to this ISR needs to check the condition of the interrupt
           and do accordingly to reset the interrupt. Like reading the data when the
           receive FIFO is full or transmit the data when the transmit FIFO is full.
         */
        for (pfnHandler = gpio26IntHandlers; pfnHandler != ((gpio_interrupt_t *)0); pfnHandler = pfnHandler->next)
            pfnHandler->handler();

        /* Clear the GPIO 26 Interrupt */
        gpioClearInterrupt(26);
    }            
}

/*
 * This ISR function calls the function that hooks to this GPIO interrupt 
 */
void gpio27ISR(
    unsigned long status
)
{
    gpio_interrupt_t *pfnHandler;
    
    if (FIELD_GET(status, INT_STATUS, GPIO27) == INT_STATUS_GPIO27_ACTIVE)
    {
        /* Walk all registered handlers for handlers that support this interrupt status 
           The function hooked to this ISR needs to check the condition of the interrupt
           and do accordingly to reset the interrupt. Like reading the data when the
           receive FIFO is full or transmit the data when the transmit FIFO is full.
         */
        for (pfnHandler = gpio27IntHandlers; pfnHandler != ((gpio_interrupt_t *)0); pfnHandler = pfnHandler->next)
            pfnHandler->handler();
            
        /* Clear the GPIO 27 Interrupt */
        gpioClearInterrupt(27);
    }            
}

/*
 * This ISR function calls the function that hooks to this GPIO interrupt 
 */
void gpio28ISR(
    unsigned long status
)
{
    gpio_interrupt_t *pfnHandler;
    
    if (FIELD_GET(status, INT_STATUS, GPIO28) == INT_STATUS_GPIO28_ACTIVE)
    {
        /* Walk all registered handlers for handlers that support this interrupt status 
           The function hooked to this ISR needs to check the condition of the interrupt
           and do accordingly to reset the interrupt. Like reading the data when the
           receive FIFO is full or transmit the data when the transmit FIFO is full.
         */
        for (pfnHandler = gpio28IntHandlers; pfnHandler != ((gpio_interrupt_t *)0); pfnHandler = pfnHandler->next)
            pfnHandler->handler();
            
        /* Clear the GPIO 28 Interrupt */
        gpioClearInterrupt(28);
    }            
}

/*
 * This ISR function calls the function that hooks to this GPIO interrupt 
 */
void gpio29ISR(
    unsigned long status
)
{
    gpio_interrupt_t *pfnHandler;
    
    if (FIELD_GET(status, INT_STATUS, GPIO29) == INT_STATUS_GPIO29_ACTIVE)
    {
        /* Walk all registered handlers for handlers that support this interrupt status 
           The function hooked to this ISR needs to check the condition of the interrupt
           and do accordingly to reset the interrupt. Like reading the data when the
           receive FIFO is full or transmit the data when the transmit FIFO is full.
         */
        for (pfnHandler = gpio29IntHandlers; pfnHandler != ((gpio_interrupt_t *)0); pfnHandler = pfnHandler->next)
            pfnHandler->handler();
            
        /* Clear the GPIO 29 Interrupt */
        gpioClearInterrupt(29);
    }            
}

/*
 * This ISR function calls the function that hooks to this GPIO interrupt 
 */
void gpio30ISR(
    unsigned long status
)
{
    gpio_interrupt_t *pfnHandler;
    
    if (FIELD_GET(status, INT_STATUS, GPIO30) == INT_STATUS_GPIO30_ACTIVE)
    {
        /* Walk all registered handlers for handlers that support this interrupt status 
           The function hooked to this ISR needs to check the condition of the interrupt
           and do accordingly to reset the interrupt. Like reading the data when the
           receive FIFO is full or transmit the data when the transmit FIFO is full.
         */
        for (pfnHandler = gpio30IntHandlers; pfnHandler != ((gpio_interrupt_t *)0); pfnHandler = pfnHandler->next)
            pfnHandler->handler();
            
        /* Clear the GPIO 30 Interrupt */
        gpioClearInterrupt(30);
    }            
}

/*
 * This ISR function calls the function that hooks to this GPIO interrupt 
 */
void gpio31ISR(
    unsigned long status
)
{
    gpio_interrupt_t *pfnHandler;
    
    if (FIELD_GET(status, INT_STATUS, GPIO31) == INT_STATUS_GPIO31_ACTIVE)
    {
        /* Walk all registered handlers for handlers that support this interrupt status 
           The function hooked to this ISR needs to check the condition of the interrupt
           and do accordingly to reset the interrupt. Like reading the data when the
           receive FIFO is full or transmit the data when the transmit FIFO is full.
         */
        for (pfnHandler = gpio31IntHandlers; pfnHandler != ((gpio_interrupt_t *)0); pfnHandler = pfnHandler->next)
            pfnHandler->handler();
            
        /* Clear the GPIO 31 Interrupt */
        gpioClearInterrupt(31);
    }            
}

/*  
 *  hookGPIOInterrupt
 *      Register GPIO Interrupt function.
 */
long hookGPIOInterrupt(
    unsigned long gpioInterruptPin,         /* GPIO Interrupt Pin to be setup as Interrupt */
    gpio_intr_trigger_t triggerType,        /* Trigger Type, either edge-triggered or level-triggered. */
    gpio_intr_active_state_t activeState,   /* Interrupt Active State */
    void (*handler)(void)
)
{
    gpio_interrupt_t *pfnNewHandler, *pfnHandler, *pfnInterruptHandler;
    unsigned short returnValue = 0;
    unsigned long maskRegister;
    PFNINTRHANDLER pfnGPIOHandler;
    
    /* Initialize the GPIO as an interrupt input signal. */
    if (gpioInterruptInit(gpioInterruptPin, triggerType, activeState) != 0)
        return (-1);

    /* Allocate a new interrupt structure */
    pfnNewHandler = (gpio_interrupt_t *)malloc(sizeof(gpio_interrupt_t));
    if (pfnNewHandler == ((gpio_interrupt_t *)0))
    {
        /* No more memory */
        return (-1);
    }
    
    /* Get the right pointer to the interrupt handler */
    switch (gpioInterruptPin)
    {
        case 31:
            pfnInterruptHandler = gpio31IntHandlers;
            maskRegister = FIELD_SET(0, INT_MASK, GPIO31, ENABLE);
            pfnGPIOHandler = gpio31ISR;
            break;
        case 30:
            pfnInterruptHandler = gpio30IntHandlers;
            maskRegister = FIELD_SET(0, INT_MASK, GPIO30, ENABLE);
            pfnGPIOHandler = gpio30ISR;
            break;
        case 29:
            pfnInterruptHandler = gpio29IntHandlers;
            maskRegister = FIELD_SET(0, INT_MASK, GPIO29, ENABLE);
            pfnGPIOHandler = gpio29ISR;
            break;
        case 28:
            pfnInterruptHandler = gpio28IntHandlers;
            maskRegister = FIELD_SET(0, INT_MASK, GPIO28, ENABLE);
            pfnGPIOHandler = gpio28ISR;
            break;
        case 27:
            pfnInterruptHandler = gpio27IntHandlers;
            maskRegister = FIELD_SET(0, INT_MASK, GPIO27, ENABLE);
            pfnGPIOHandler = gpio27ISR;
            break;
        case 26:
            pfnInterruptHandler = gpio26IntHandlers;
            maskRegister = FIELD_SET(0, INT_MASK, GPIO26, ENABLE);
            pfnGPIOHandler = gpio26ISR;
            break;
        case 25:
            pfnInterruptHandler = gpio25IntHandlers;
            maskRegister = FIELD_SET(0, INT_MASK, GPIO25, ENABLE);
            pfnGPIOHandler = gpio25ISR;
            break;
    }

    /* Make sure that it has not been register more than once */
    for (pfnHandler = pfnInterruptHandler; pfnHandler != ((gpio_interrupt_t *)0); pfnHandler = pfnHandler->next)
    {
        if (pfnHandler->handler == handler)
            return (-1);                   /* Duplicate entry */
    }
        
    /* If this is the first interrupt handler, register this ZV Port ISR */
    if (pfnInterruptHandler == ((gpio_interrupt_t *)0))
        returnValue = registerHandler(pfnGPIOHandler, maskRegister);

    if (returnValue == 0)
    {
        /* Fill interrupt structure. */
        pfnNewHandler->next = pfnInterruptHandler;
        pfnNewHandler->handler = handler;
        pfnInterruptHandler = pfnNewHandler;
    }
    
    /* Update the actual handler */
    switch (gpioInterruptPin)
    {
        case 31:
            gpio31IntHandlers = pfnInterruptHandler;
            break;
        case 30:
            gpio30IntHandlers = pfnInterruptHandler;
            break;
        case 29:
            gpio29IntHandlers = pfnInterruptHandler;
            break;
        case 28:
            gpio28IntHandlers = pfnInterruptHandler;
            break;
        case 27:
            gpio27IntHandlers = pfnInterruptHandler;
            break;
        case 26:
            gpio26IntHandlers = pfnInterruptHandler;
            break;
        case 25:
            gpio25IntHandlers = pfnInterruptHandler;
            break;
    }
    
    return returnValue;
}

/*  
 *  unhookSSPInterrupt
 *      unregister SSP Interrupt function.
 */
long unhookGPIOInterrupt(
    unsigned char gpioInterruptPin,
    void (*handler)(void)
)
{
    gpio_interrupt_t *pfnHandler, *prev, *pfnInterruptHandler;
    unsigned long mask;
    PFNINTRHANDLER pfnGPIOHandler;
    
    switch (gpioInterruptPin)
    {
        case 31:
            pfnInterruptHandler = gpio31IntHandlers;
            pfnGPIOHandler = gpio31ISR;
            break;
        case 30:
            pfnInterruptHandler = gpio30IntHandlers;
            pfnGPIOHandler = gpio30ISR;
            break;
        case 29:
            pfnInterruptHandler = gpio29IntHandlers;
            pfnGPIOHandler = gpio29ISR;
            break;
        case 28:
            pfnInterruptHandler = gpio28IntHandlers;
            pfnGPIOHandler = gpio28ISR;
            break;
        case 27:
            pfnInterruptHandler = gpio27IntHandlers;
            pfnGPIOHandler = gpio27ISR;
            break;
        case 26:
            pfnInterruptHandler = gpio26IntHandlers;
            pfnGPIOHandler = gpio26ISR;
            break;
        case 25:
            pfnInterruptHandler = gpio25IntHandlers;
            pfnGPIOHandler = gpio25ISR;
            break;
    }

    /* Find the requested handle to unregister */
    for (pfnHandler = pfnInterruptHandler, prev = ((gpio_interrupt_t *)0); 
         pfnHandler != ((gpio_interrupt_t *)0); 
         prev = pfnHandler, pfnHandler = pfnHandler->next)
    {
        if (pfnHandler->handler == handler)
        {
            /* Remove the interrupt handler */
            if (prev == ((gpio_interrupt_t *)0))
            {
                pfnInterruptHandler = pfnHandler->next;
                
                /* Update the actual handler */
                switch (gpioInterruptPin)
                {
                    case 31:
                        gpio31IntHandlers = pfnInterruptHandler;
                        break;
                    case 30:
                        gpio30IntHandlers = pfnInterruptHandler;
                        break;
                    case 29:
                        gpio29IntHandlers = pfnInterruptHandler;
                        break;
                    case 28:
                        gpio28IntHandlers = pfnInterruptHandler;
                        break;
                    case 27:
                        gpio27IntHandlers = pfnInterruptHandler;
                        break;
                    case 26:
                        gpio26IntHandlers = pfnInterruptHandler;
                        break;
                    case 25:
                        gpio25IntHandlers = pfnInterruptHandler;
                        break;
                }
            }
            else
            {
                prev->next = pfnHandler->next;
            }
            free(pfnHandler);
            
            /* If this was the last interrupt handler, remove the IRQ handler */
            if (pfnInterruptHandler == ((gpio_interrupt_t *)0))
            {
                /* Remove the ISR from the list */
                unregisterHandler(pfnGPIOHandler);
                
                /* Set the GPIO back to regular GPIO pin. */
                gpioInterruptExit(gpioInterruptPin);
            }
            
            /* Success */
            return (0);
        }
    }

    /* Oops, handler is not registered */
    return (-1);
}
