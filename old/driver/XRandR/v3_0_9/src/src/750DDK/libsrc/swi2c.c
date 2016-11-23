/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  I2C.c --- Voyager GX SDK 
*  This file contains the source code for SW I2C.
* 
*******************************************************************/
#include "defs.h"
#include "hardware.h"
#include "power.h"
#include "swi2c.h"

/*******************************************************************
 * I2C Software Master Driver:   
 * ===========================
 * Each i2c cycle is split into 4 sections. Each of these section marks
 * a point in time where the SCL or SDA may be changed. 
 * 
 * 1 Cycle == |  Section I. |  Section 2. |  Section 3. |  Section 4. |
 *            +-------------+-------------+-------------+-------------+
 *            | SCL set LOW |SCL no change| SCL set HIGH|SCL no change|
 *                 
 *                                          ____________ _____________
 * SCL == XXXX _____________ ____________ /
 *                 
 * I.e. the SCL may only be changed in section 1. and section 3. while
 * the SDA may only be changed in section 2. and section 4. The table
 * below gives the changes for these 2 lines in the varios sections.
 * 
 * Section changes Table:        
 * ======================
 * blank = no change, L = set bit LOW, H = set bit HIGH
 *                       
 *                                | 1.| 2.| 3.| 4.|      
 *                 ---------------+---+---+---+---+      
 *                 Tx Start   SDA |   | H |   | L |      
 *                            SCL | L |   | H |   |      
 *                 ---------------+---+---+---+---+                
 *                 Tx Stop    SDA |   | L |   | H |      
 *                            SCL | L |   | H |   |      
 *                 ---------------+---+---+---+---+                
 *                 Tx bit H   SDA |   | H |   |   |      
 *                            SCL | L |   | H |   |      
 *                 ---------------+---+---+---+---+                
 *                 Tx bit L   SDA |   | L |   |   |      
 *                            SCL | L |   | H |   |      
 *                 ---------------+---+---+---+---+                
 *                                  
 ******************************************************************/

/* GPIO pins used for this I2C. It ranges from 0 to 63. */
static unsigned char g_i2cClockGPIO = DEFAULT_I2C_SCL;
static unsigned char g_i2cDataGPIO = DEFAULT_I2C_SDA;

/*
 *  Below is the variable declaration for the GPIO pin register usage
 *  for the i2c Clock and i2c Data.
 *
 *  Note:
 *      Notice that the GPIO usage for the i2c clock and i2c Data are
 *      separated. This is to make this code flexible enough when 
 *      two separate GPIO pins for the clock and data are located
 *      in two different GPIO register set (worst case).
 */

/* i2c Clock GPIO Register usage */
static uint32_t g_i2cClkGPIOMuxReg = GPIO_MUX;
static uint32_t g_i2cClkGPIODataReg = GPIO_DATA;
static uint32_t g_i2cClkGPIODataDirReg = GPIO_DATA_DIRECTION;

/* i2c Clock GPIO Register usage */
static uint32_t g_i2cDataGPIOMuxReg = GPIO_MUX;
static uint32_t g_i2cDataGPIODataReg = GPIO_DATA;
static uint32_t g_i2cDataGPIODataDirReg = GPIO_DATA_DIRECTION;

static void (*pfn_swI2C_wait)(void) = NULL;



/*******************************************************************************
    swI2CWait
        This function puts a delay between command

    Parameters:
        None

    Return Value:
        None
 *******************************************************************************/        
static void swI2CWait(void)
{
    int i, Temp;

    for(i=0; i<600; i++)
    {
        Temp = i;
        Temp += i;
    }
}

/*******************************************************************************
    swI2CSCL
        This function set/reset the SCL GPIO pin

    Parameters:
        value	- Bit value to set to the SCL or SDA (0 = low, 1 = high)

    Return Value:
        None
 *******************************************************************************/ 
static void swI2CSCL(unsigned char value)
{
    uint32_t ulGPIOData;
    uint32_t ulGPIODirection;

    ulGPIODirection = peekRegisterDWord(g_i2cClkGPIODataDirReg);
    if (value)      /* High */
    {
        /* Set direction as input. This will automatically pull the signal up. */
        ulGPIODirection &= ~(1 << g_i2cClockGPIO);	
        pokeRegisterDWord(g_i2cClkGPIODataDirReg, ulGPIODirection);
    }
    else            /* Low */
    {
        /* Set the signal down */
        ulGPIOData = peekRegisterDWord(g_i2cClkGPIODataReg);
        ulGPIOData &= ~(1 << g_i2cClockGPIO);
        pokeRegisterDWord(g_i2cClkGPIODataReg, ulGPIOData);

        /* Set direction as output */
        ulGPIODirection |= (1 << g_i2cClockGPIO);		
        pokeRegisterDWord(g_i2cClkGPIODataDirReg, ulGPIODirection);
    }
	swI2CWait();
}

/*******************************************************************************
    swI2CSDA
        This function set/reset the SDA GPIO pin

    Parameters:
        value	- Bit value to set to the SCL or SDA (0 = low, 1 = high)

    Return Value:
        None
 *******************************************************************************/
static void swI2CSDA(unsigned char value)
{
    uint32_t ulGPIOData;
    uint32_t ulGPIODirection;

    ulGPIODirection = peekRegisterDWord(g_i2cDataGPIODataDirReg);
    if (value)      /* High */
    {
        /* Set direction as input. This will automatically pull the signal up. */
        ulGPIODirection &= ~(1 << g_i2cDataGPIO);	
        pokeRegisterDWord(g_i2cDataGPIODataDirReg, ulGPIODirection);
    }
    else            /* Low */
    {
        /* Set the signal down */
        ulGPIOData = peekRegisterDWord(g_i2cDataGPIODataReg);
        ulGPIOData &= ~(1 << g_i2cDataGPIO);
        pokeRegisterDWord(g_i2cDataGPIODataReg, ulGPIOData);

        /* Set direction as output */
        ulGPIODirection |= (1 << g_i2cDataGPIO);		
        pokeRegisterDWord(g_i2cDataGPIODataDirReg, ulGPIODirection);
    }
	swI2CWait();
}

/*******************************************************************************
    swI2CReadSDA
        This function read the data from the SDA GPIO pin

    Parameters:
        None

    Return Value:
        The SDA data bit sent by the Slave
 *******************************************************************************/
static unsigned char swI2CReadSDA()
{
    unsigned char data;
    uint32_t ulGPIODirection;
    uint32_t ulGPIOData;

    /* Make sure that the direction is input (High) */
    ulGPIODirection = peekRegisterDWord(g_i2cDataGPIODataDirReg);
    if ((ulGPIODirection & (1 << g_i2cDataGPIO)) != (~(1 << g_i2cDataGPIO)))
    {
        ulGPIODirection &= ~(1 << g_i2cDataGPIO);
        pokeRegisterDWord(g_i2cDataGPIODataDirReg, ulGPIODirection);
    }

    /* Now read the SDA line */
    ulGPIOData = peekRegisterDWord(g_i2cDataGPIODataReg);
    if (ulGPIOData & (1 << g_i2cDataGPIO)) 
        return 1;
    else 
        return 0;
	swI2CWait();
}

#pragma optimize( "", off )

/*******************************************************************************
    swI2CAck
        This function sends ACK signal

    Parameters:
        None

    Return Value:
        None
 *******************************************************************************/
static void swI2CAck(void)
{
    return;  /* Single byte read is ok without it. */
}

/*******************************************************************************
    swI2CStart
        This function sends the start command to the slave device

    Parameters:
        None

    Return Value:
        None
 *******************************************************************************/
void swI2CStart(void)
{
    /* Start I2C */
    swI2CSDA(1);
    swI2CSCL(1);
    swI2CSDA(0);
}

/*******************************************************************************
    swI2CStop
        This function sends the stop command to the slave device

    Parameters:
        None

    Return value:
        None
 *******************************************************************************/
void swI2CStop()
{
    /* Stop the I2C */
    swI2CSCL(1);
    swI2CSDA(0);
    swI2CSDA(1);
}

/*******************************************************************************
    swI2CInit
        This function initializes the i2c attributes and bus

    Parameters:
        i2cClkGPIO  - The GPIO pin to be used as i2c SCL
        i2cDataGPIO - The GPIO pin to be used as i2c SDA

    Return Value:
        -1   - Fail to initialize the i2c
         0   - Success
 *******************************************************************************/
int32_t swI2CInit(unsigned char i2cClkGPIO, unsigned char i2cDataGPIO,swI2C_wait_proc function)
{
    int i;
    uint32_t value;
    uint32_t gate;
    
    /* Return 0 if the GPIO pins to be used is out of range. The range is only from [0..63] */
    if ((i2cClkGPIO > 31) || (i2cDataGPIO > 31))
        return (-1);

	/* initialize the function pointer */
	pfn_swI2C_wait = function;
	if(pfn_swI2C_wait == NULL)
		pfn_swI2C_wait = swI2CWait;	
    
    /* Initialize the GPIO pin for the i2c Clock Register */
    g_i2cClkGPIOMuxReg = GPIO_MUX;   
    g_i2cClkGPIODataReg = GPIO_DATA;    
    g_i2cClkGPIODataDirReg = GPIO_DATA_DIRECTION;
    
    /* Initialize the Clock GPIO Offset */
    g_i2cClockGPIO = i2cClkGPIO;
    
    /* Initialize the GPIO pin for the i2c Data Register */
    g_i2cDataGPIOMuxReg = GPIO_MUX;    
    g_i2cDataGPIODataReg = GPIO_DATA;    
    g_i2cDataGPIODataDirReg = GPIO_DATA_DIRECTION;
    
    /* Initialize the Data GPIO Offset */
    g_i2cDataGPIO = i2cDataGPIO;

    /* Enable the GPIO pins for the i2c Clock and Data (GPIO MUX) */
    pokeRegisterDWord(g_i2cClkGPIOMuxReg, 
                      peekRegisterDWord(g_i2cClkGPIOMuxReg) & ~(1 << g_i2cClockGPIO));
    pokeRegisterDWord(g_i2cDataGPIOMuxReg, 
                      peekRegisterDWord(g_i2cDataGPIOMuxReg) & ~(1 << g_i2cDataGPIO));

    /* Enable GPIO power */
    enableGPIO(1);

    /* Clear the i2c lines. */
    for(i=0; i<9; i++) 
        swI2CStop();

    return 0;
}

/*******************************************************************************
    swI2CWriteByte
        This function writes one byte to the slave device

    Parameters:
        data    - Data to be write to the slave device

    Return Value:
        0   - Fail to write byte
        1   - Success
 *******************************************************************************/
unsigned char swI2CWriteByte(unsigned char data) 
{
    unsigned char value = data;
    int i;

    /* Sending the data bit by bit */
    for (i=0; i<8; i++)
    {
        /* Set SCL to low */
        swI2CSCL(0);

        /* Send data bit */
        if ((value & 0x80) != 0)
            swI2CSDA(1);
        else
            swI2CSDA(0);

        swI2CWait();

        /* Toggle clk line to one */
        swI2CSCL(1);

        /* Shift byte to be sent */
        value = value << 1;
    }

    /* Set the SCL Low and SDA High (prepare to get input) */
    swI2CSCL(0);
    swI2CSDA(1);

    /* Set the SCL High for ack */
    swI2CWait();
    swI2CSCL(1);

    /* Read SDA, until SDA==0 */
    for(i=0; i<0xff; i++) 
    {
        swI2CWait();
        swI2CWait();
        if (!swI2CReadSDA())
            break;
    }

    /* Set the SCL Low and SDA High */
    swI2CSCL(0);
    swI2CSDA(1);

    return (i<0xff);
}

/*******************************************************************************
    swI2CReadByte
        This function reads one byte from the slave device

    Parameters:
        ack	- Flag to indicate either to send the acknowledge
              message to the slave device or not

    Return Value:
        One byte data read from the Slave device
 *******************************************************************************/
unsigned char swI2CReadByte(unsigned char ack)
{
    int i;
    unsigned char data = 0;

    for(i=7; i>=0; i--)
    {
        /* Set the SCL to Low and SDA to High (Input) */
        swI2CSCL(0);
        swI2CSDA(1);
        swI2CWait();

        /* Set the SCL High */
        swI2CSCL(1);
        swI2CWait();

        /* Read data bits from SDA */
        data |= (swI2CReadSDA() << i);
    }

    if (ack)
        swI2CAck();

    /* Set the SCL Low and SDA High */
    swI2CSCL(0);
    swI2CSDA(1);

    return data;
}
#pragma optimize( "", on )

/*******************************************************************************
    swI2CReadReg
    This function reads the slave device's register

    Parameters:
    deviceAddress   - i2c Slave device address which register
                      to be read from
    registerIndex   - Slave device's register to be read

    Return Value:
        Register value
 *******************************************************************************/
unsigned char swI2CReadReg(unsigned char deviceAddress, unsigned char registerIndex)
{
    unsigned char data;

    /* Send the Start signal */
    swI2CStart();

    /* Send the device address */
    swI2CWriteByte(deviceAddress);                                                  

    /* Send the register index */
    swI2CWriteByte(registerIndex);               

    /* Get the bus again and get the data from the device read address */
    swI2CStart();
    swI2CWriteByte(deviceAddress + 1);
    data = swI2CReadByte(1);

    /* Stop swI2C and release the bus */
    swI2CStop();

    return data;
}

/*******************************************************************************
    swI2CWriteReg
        This function write a value to the slave device's register

    Parameters:
        deviceAddress   - i2c Slave device address which register
                          to be written
        registerIndex   - Slave device's register to be written
        data            - Data to be written to the register

 *******************************************************************************/
int32_t swI2CWriteReg(unsigned char deviceAddress, unsigned char registerIndex, unsigned char data)
{
    int32_t returnValue = 0;
    
    /* Send the Start signal */
    swI2CStart();

    /* Send the device address and read the data. All should return success
       in order for the writing processed to be successful
     */
    if (swI2CWriteByte(deviceAddress) && 
        swI2CWriteByte(registerIndex) &&
        swI2CWriteByte(data))
    {
        returnValue = -1;
    }
    
    /* Stop i2c and release the bus */
    swI2CStop();

    return returnValue;
}



