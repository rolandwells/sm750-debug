/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  gpioint.h --- SMI DDK 
*  This file contains the definitions for setting up the GPIO interrupt.
* 
*******************************************************************/
#ifndef _GPIOINT_H_
#define _GPIOINT_H_

typedef enum _gpio_intr_active_state_t
{
    GPIO_INTR_ACTIVE_LOW = 0,
    GPIO_INTR_ACTIVE_HIGH
}
gpio_intr_active_state_t;

typedef enum _gpio_intr_trigger_t
{
    GPIO_INTR_EDGE_TRIGGERED = 0,
    GPIO_INTR_LEVEL_TRIGGERED
}
gpio_intr_trigger_t;

/*  
 *  hookGPIOInterrupt
 *      Register GPIO Interrupt function.
 */
long hookGPIOInterrupt(
    unsigned long gpioInterruptPin,         /* GPIO Interrupt Pin to be setup as Interrupt */
    gpio_intr_trigger_t triggerType,        /* Trigger Type, either edge-triggered or level-triggered. */
    gpio_intr_active_state_t activeState,   /* Interrupt Active State */
    void (*handler)(void)
);

/*  
 *  unhookGPIOInterrupt
 *      unregister GPIO Interrupt function.
 */
long unhookGPIOInterrupt(
    unsigned char gpioInterruptPin,
    void (*handler)(void)
);

#endif /* _GPIOINT_H_ */
