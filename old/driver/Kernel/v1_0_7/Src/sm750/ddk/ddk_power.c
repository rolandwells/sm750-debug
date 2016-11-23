#include "ddk750_common.h"
/*  
  	On hardware reset, power mode 0 is default.
 	powerMode can be 0,1 or 2
 */
void setPowerMode(unsigned long powerMode)
{
    	unsigned long control_value = 0;
    	control_value = peekRegisterDWord(POWER_MODE_CTRL);
	control_value = FIELD_VALUE(control_value,POWER_MODE_CTRL,MODE,powerMode);  
    	pokeRegisterDWord(POWER_MODE_CTRL, control_value);
}

inline unsigned long getPowerMode()
{
	return (FIELD_GET(peekRegisterDWord(POWER_MODE_CTRL), POWER_MODE_CTRL, MODE));
}

void setCurrentGate(unsigned long gate)
{
/*
	if gate == 1 mode1
	else mode0
*/
    	unsigned long gate_reg;
    	unsigned long mode;
    	/* Get current power mode. */
    	mode = getPowerMode();
	gate_reg = (gate==1)?MODE1_GATE:MODE0_GATE;
    	pokeRegisterDWord(gate_reg, gate);
}

void ddk750_setDPMS(DPMS_t state)
{
/* 
	Set DPMS state 
	DPMS is used for CRT head
*/
    	unsigned long value;
    	value = peekRegisterDWord(SYSTEM_CTRL);
	value= FIELD_VALUE(value,SYSTEM_CTRL,DPMS,state);
    	pokeRegisterDWord(SYSTEM_CTRL, value);
}


/* 
 * This function turns on/off the DAC for CRT display control.
 * Input: On or off
 */
void setDAC(disp_state_t state)
{
	ulong value;
	value = (state == DISP_ON)?MISC_CTRL_DAC_POWER_ON:MISC_CTRL_DAC_POWER_OFF;
	pokeRegisterDWord(MISC_CTRL, FIELD_VALUE(peekRegisterDWord(MISC_CTRL),
                                               MISC_CTRL,
                                               DAC_POWER,
                                               value));
}



/* 
 * This function enable/disable the 2D engine.
 */
void enable2DEngine(unsigned long enable)
{
	unsigned long gate;
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
void enableZVPort(unsigned long enable)
{
    unsigned long gate;
    
    /* Enable ZV Port Gate */
    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
    {
        gate = FIELD_SET(gate, CURRENT_GATE, ZVPORT, ON);
#if 0
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
 * This function enable/disable the I2C Engine
 */
void enableI2C(unsigned long enable)
{
    unsigned long gate;
    
    /* Enable I2C Gate */
    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
        gate = FIELD_SET(gate, CURRENT_GATE, I2C, ON);        
    else
        gate = FIELD_SET(gate, CURRENT_GATE, I2C, OFF);
    
    setCurrentGate(gate);
}


/* 
 * This function enable/disable the GPIO Engine
 */
void enableGPIO(unsigned long enable)
{
    unsigned long gate;
    
    /* Enable GPIO Gate */
    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
        gate = FIELD_SET(gate, CURRENT_GATE, GPIO, ON);        
    else
        gate = FIELD_SET(gate, CURRENT_GATE, GPIO, OFF);
    
    setCurrentGate(gate);
}





