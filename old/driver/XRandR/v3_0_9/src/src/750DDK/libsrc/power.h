/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  power.h --- Voyager GX SDK 
*  This file contains the definitions for the power functions.
* 
*******************************************************************/
#ifndef _POWER_H_
#define _POWER_H_

typedef enum _DPMS_t
{
    DPMS_ON,
    DPMS_STANDBY,
    DPMS_SUSPEND,
    DPMS_OFF
}
DPMS_t;

/*
 * This function sets the DPMS state 
 */
_X_EXPORT void setDPMS(DPMS_t state);

/* 
 * This function gets the current power mode 
 */
uint32_t getPowerMode();

/* 
 * This function sets the current power mode
 */
void setPowerMode(uint32_t powerMode);

/* 
 * This function sets current gate 
 */
void setCurrentGate(uint32_t gate);

/* 
 * This function enable/disable the 2D engine.
 */
void enable2DEngine(uint32_t enable);

/* 
 * This function enable/disable the ZV Port 
 */
void enableZVPort(uint32_t enable);

/* 
 * This function enable/disable the DMA Engine
 */
void enableDMA(uint32_t enable);

/* 
 * This function enable/disable the GPIO Engine
 */
void enableGPIO(uint32_t enable);

/* 
 * This function enable/disable the PWM Engine
 */
void enablePWM(uint32_t enable);

/* 
 * This function enable/disable the I2C Engine
 */
void enableI2C(uint32_t enable);

/* 
 * This function enable/disable the SSP.
 */
void enableSSP(uint32_t enable);

#endif /* _POWER_H_ */
