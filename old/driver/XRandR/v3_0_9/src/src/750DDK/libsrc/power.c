/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  power.c --- Voyager GX SDK 
*  This file contains the source code for the power functions.
* 
*******************************************************************/
#include "defs.h"
#include "hardware.h"
#include "power.h"

/* Set DPMS state */
_X_EXPORT void setDPMS(DPMS_t state)
{
    uint32_t value;

    value = peekRegisterDWord(SYSTEM_CTRL);
    switch (state)
    {
       case DPMS_ON:
        value = FIELD_SET(value, SYSTEM_CTRL, DPMS, VPHP);
        break;

       case DPMS_STANDBY:
        value = FIELD_SET(value, SYSTEM_CTRL, DPMS, VPHN);
        break;

       case DPMS_SUSPEND:
        value = FIELD_SET(value, SYSTEM_CTRL, DPMS, VNHP);
        break;

       case DPMS_OFF:
        value = FIELD_SET(value, SYSTEM_CTRL, DPMS, VNHN);
        break;
    }

    pokeRegisterDWord(SYSTEM_CTRL, value);
}

/*
 * This function gets the power mode, one of three modes: 0, 1 or Sleep.
 * On hardware reset, power mode 0 is default.
 */
uint32_t getPowerMode()
{
    return (FIELD_GET(peekRegisterDWord(POWER_MODE_CTRL), POWER_MODE_CTRL, MODE));
}

/*
 * SM50x can operate in one of three modes: 0, 1 or Sleep.
 * On hardware reset, power mode 0 is default.
 */
void setPowerMode(uint32_t powerMode)
{
    uint32_t control_value = 0;

    control_value = peekRegisterDWord(POWER_MODE_CTRL);

    switch (powerMode)
    {
        case POWER_MODE_CTRL_MODE_MODE0:
            control_value = FIELD_SET(control_value, POWER_MODE_CTRL, MODE, MODE0);
            break;

        case POWER_MODE_CTRL_MODE_MODE1:
            control_value = FIELD_SET(control_value, POWER_MODE_CTRL, MODE, MODE1);
            break;

        case POWER_MODE_CTRL_MODE_SLEEP:
            control_value = FIELD_SET(control_value, POWER_MODE_CTRL, MODE, SLEEP);
            break;

        default:
            break;
    }

    /* Set up other fields in Power Control Register */
    if (powerMode == POWER_MODE_CTRL_MODE_SLEEP)
    {
        control_value =
#ifdef VALIDATION_CHIP
            FIELD_SET(  control_value, POWER_MODE_CTRL, 336CLK, OFF) |
#endif
            FIELD_SET(  control_value, POWER_MODE_CTRL, OSC_INPUT,  OFF);
    }
    else
    {
        control_value =
#ifdef VALIDATION_CHIP
            FIELD_SET(  control_value, POWER_MODE_CTRL, 336CLK, ON) |
#endif
            FIELD_SET(  control_value, POWER_MODE_CTRL, OSC_INPUT,  ON);
    }
    
    /* Program new power mode. */
    pokeRegisterDWord(POWER_MODE_CTRL, control_value);
}

void setCurrentGate(uint32_t gate)
{
    uint32_t gate_reg;
    uint32_t mode;

    /* Get current power mode. */
    mode = getPowerMode();

    switch (mode)
    {
        case POWER_MODE_CTRL_MODE_MODE0:
            gate_reg = MODE0_GATE;
            break;

        case POWER_MODE_CTRL_MODE_MODE1:
            gate_reg = MODE1_GATE;
            break;

        default:
            gate_reg = MODE0_GATE;
            break;
    }
    pokeRegisterDWord(gate_reg, gate);
}

/* 
 * This function enable/disable the 2D engine.
 */
void enable2DEngine(uint32_t enable)
{
    uint32_t gate;

    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
    {
        gate = FIELD_SET(gate, CURRENT_GATE, DE,  ON);
        gate = FIELD_SET(gate, CURRENT_GATE, CSC, ON);
    }
    else
    {
        gate = FIELD_SET(gate, CURRENT_GATE, DE,  OFF);
        gate = FIELD_SET(gate, CURRENT_GATE, CSC, OFF);
    }

    setCurrentGate(gate);
}

/* 
 * This function enable/disable the ZV Port.
 */
void enableZVPort(uint32_t enable)
{
    uint32_t gate;
    
    /* Enable ZV Port Gate */
    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
    {
        gate = FIELD_SET(gate, CURRENT_GATE, ZVPORT, ON);
#if 1
        /* Using Software I2C */
        gate = FIELD_SET(gate, CURRENT_GATE, GPIO, ON);
#else
        /* Using Hardware I2C */
        gate = FIELD_SET(gate, CURRENT_GATE, I2C,    ON);        
#endif
    }
    else
    {
        /* Disable ZV Port Gate. There is no way to know whether the GPIO pins are being used
           or not. Therefore, do not disable the GPIO gate. */
        gate = FIELD_SET(gate, CURRENT_GATE, ZVPORT, OFF);
    }
    
    setCurrentGate(gate);
}

/* 
 * This function enable/disable the SSP.
 */
void enableSSP(uint32_t enable)
{
    uint32_t gate;
    
    /* Enable SSP Gate */
    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
        gate = FIELD_SET(gate, CURRENT_GATE, SSP, ON);        
    else
        gate = FIELD_SET(gate, CURRENT_GATE, SSP, OFF);
    
    setCurrentGate(gate);
}

/* 
 * This function enable/disable the DMA Engine
 */
void enableDMA(uint32_t enable)
{
    uint32_t gate;
    
    /* Enable DMA Gate */
    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
        gate = FIELD_SET(gate, CURRENT_GATE, DMA, ON);        
    else
        gate = FIELD_SET(gate, CURRENT_GATE, DMA, OFF);
    
    setCurrentGate(gate);
}

/* 
 * This function enable/disable the GPIO Engine
 */
_X_EXPORT void enableGPIO(uint32_t enable)
{
    uint32_t gate;
    
    /* Enable GPIO Gate */
    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
        gate = FIELD_SET(gate, CURRENT_GATE, GPIO, ON);        
    else
        gate = FIELD_SET(gate, CURRENT_GATE, GPIO, OFF);
    
    setCurrentGate(gate);
}

/* 
 * This function enable/disable the PWM Engine
 */
void enablePWM(uint32_t enable)
{
    uint32_t gate;
    
    /* Enable PWM Gate */
    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
        gate = FIELD_SET(gate, CURRENT_GATE, PWM, ON);        
    else
        gate = FIELD_SET(gate, CURRENT_GATE, PWM, OFF);
    
    setCurrentGate(gate);
}

/* 
 * This function enable/disable the I2C Engine
 */
void enableI2C(uint32_t enable)
{
    uint32_t gate;
    
    /* Enable I2C Gate */
    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
        gate = FIELD_SET(gate, CURRENT_GATE, I2C, ON);        
    else
        gate = FIELD_SET(gate, CURRENT_GATE, I2C, OFF);
    
    setCurrentGate(gate);
}
