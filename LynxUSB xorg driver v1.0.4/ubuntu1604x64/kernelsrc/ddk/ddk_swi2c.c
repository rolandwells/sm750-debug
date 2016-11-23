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
#include	<linux/version.h>
#include	<linux/kernel.h>
#include	<linux/errno.h>
#include	<linux/init.h>
#include	<linux/slab.h>
#include	<linux/module.h>
#include	<linux/kref.h>
#include	<linux/uaccess.h>
#include	<linux/usb.h>
#include	<linux/mutex.h>
#include	<linux/fb.h>
#include	<linux/ioport.h>
#include	<linux/vmalloc.h>
#include	<linux/pagemap.h>
#include	<linux/screen_info.h>
#include 	<linux/console.h>
#include	<linux/timer.h>
#include	<linux/delay.h>
//#include	<linux/atomic.h>
#include	<linux/workqueue.h>
#include	<linux/spinlock.h>
#include	<linux/i2c.h>
#include	<linux/i2c-algo-bit.h>
#include	"../usbfb.h"
#include "ddk750_common.h"
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
static unsigned long g_i2cClkGPIOMuxReg = GPIO_MUX;
static unsigned long g_i2cClkGPIODataReg = GPIO_DATA;
static unsigned long g_i2cClkGPIODataDirReg = GPIO_DATA_DIRECTION;

/* i2c Data GPIO Register usage */
static unsigned long g_i2cDataGPIOMuxReg = GPIO_MUX;
static unsigned long g_i2cDataGPIODataReg = GPIO_DATA;
static unsigned long g_i2cDataGPIODataDirReg = GPIO_DATA_DIRECTION;

/*
 *  This function puts a delay between command
 */        
static void swI2CWait(void)
{
    int i, Temp;

    for(i=0; i<600; i++)
    {
        Temp = i;
        Temp += i;
    }
}

/*
 *  This function set/reset the SCL GPIO pin
 *
 *  Parameters:
 *      value	- Bit value to set to the SCL or SDA (0 = low, 1 = high)
 */ 
void swI2CSCL(void * ufb,unsigned char value)
{
    unsigned long ulGPIOData;
    unsigned long ulGPIODirection;

    ulGPIODirection = peek32(ufb,g_i2cClkGPIODataDirReg);
    if (value)      /* High */
    {
        /* Set direction as input. This will automatically pull the signal up. */
        ulGPIODirection &= ~(1 << g_i2cClockGPIO);	
        poke32(ufb,g_i2cClkGPIODataDirReg, ulGPIODirection);
    }
    else            /* Low */
    {
        /* Set the signal down */
        ulGPIOData = peek32(ufb,g_i2cClkGPIODataReg);
        ulGPIOData &= ~(1 << g_i2cClockGPIO);
        poke32(ufb,g_i2cClkGPIODataReg, ulGPIOData);

        /* Set direction as output */
        ulGPIODirection |= (1 << g_i2cClockGPIO);		
        poke32(ufb,g_i2cClkGPIODataDirReg, ulGPIODirection);
    }
}

/*
 *  This function set/reset the SDA GPIO pin
 *
 *  Parameters:
 *      value	- Bit value to set to the SCL or SDA (0 = low, 1 = high)
 */
void swI2CSDA(void * ufb,unsigned char value)
{
    unsigned long ulGPIOData;
    unsigned long ulGPIODirection;

    ulGPIODirection = peek32(ufb,g_i2cDataGPIODataDirReg);
    if (value)      /* High */
    {
        /* Set direction as input. This will automatically pull the signal up. */
        ulGPIODirection &= ~(1 << g_i2cDataGPIO);	
        poke32(ufb,g_i2cDataGPIODataDirReg, ulGPIODirection);
    }
    else            /* Low */
    {
        /* Set the signal down */
        ulGPIOData = peek32(ufb,g_i2cDataGPIODataReg);
        ulGPIOData &= ~(1 << g_i2cDataGPIO);
        poke32(ufb,g_i2cDataGPIODataReg, ulGPIOData);

        /* Set direction as output */
        ulGPIODirection |= (1 << g_i2cDataGPIO);		
        poke32(ufb,g_i2cDataGPIODataDirReg, ulGPIODirection);
    }
}

/*
 *  This function read the data from the SDA GPIO pin
 *
 *  Return Value:
 *      The SDA data bit sent by the Slave
 */
unsigned char swI2CReadSDA(void * ufb)
{
//    unsigned char data;
    unsigned long ulGPIODirection;
    unsigned long ulGPIOData;

    /* Make sure that the direction is input (High) */
    ulGPIODirection = peek32(ufb,g_i2cDataGPIODataDirReg);
    if ((ulGPIODirection & (1 << g_i2cDataGPIO)) != (~(1 << g_i2cDataGPIO)))
    {
        ulGPIODirection &= ~(1 << g_i2cDataGPIO);
        poke32(ufb,g_i2cDataGPIODataDirReg, ulGPIODirection);
    }

    /* Now read the SDA line */
    ulGPIOData = peek32(ufb,g_i2cDataGPIODataReg);
    if (ulGPIOData & (1 << g_i2cDataGPIO)) 
        return 1;
    else 
        return 0;
}

unsigned char swI2CReadSCL(void * ufb)
{
//    unsigned char data;
    unsigned long ulGPIODirection;
    unsigned long ulGPIOData;

    /* Make sure that the direction is input (High) */
    ulGPIODirection = peek32(ufb,g_i2cClkGPIODataDirReg);
    if ((ulGPIODirection & (1 << g_i2cClockGPIO)) != (~(1 << g_i2cClockGPIO)))
    {
        ulGPIODirection &= ~(1 << g_i2cClockGPIO);
        poke32(ufb,g_i2cClkGPIODataDirReg, ulGPIODirection);
    }

    /* Now read the SDA line */
    ulGPIOData = peek32(ufb,g_i2cClkGPIODataReg);
    if (ulGPIOData & (1 << g_i2cClockGPIO)) 
        return 1;
    else 
        return 0;
}
//#pragma optimize( "", off )

/*
 *  This function sends ACK signal
 */
static void swI2CAck(void)
{
    return;  /* Single byte read is ok without it. */
}

/*
 *  This function sends the start command to the slave device
 */
void swI2CStart(void * ufb)
{
    /* Start I2C */
    swI2CSDA(ufb,1);
    swI2CSCL(ufb,1);
    swI2CSDA(ufb,0);
}

/*
 *  This function sends the stop command to the slave device
 */
void swI2CStop(void * ufb)
{
    /* Stop the I2C */
    swI2CSCL(ufb,1);
    swI2CSDA(ufb,0);
    swI2CSDA(ufb,1);
}

/*
 *  This function writes one byte to the slave device
 *
 *  Parameters:
 *      data    - Data to be write to the slave device
 *
 *  Return Value:
 *       0   - Success
 *      -1   - Fail to write byte
 */
long swI2CWriteByte(void * ufb,unsigned char data) 
{
    unsigned char value = data;
    int i;

    /* Sending the data bit by bit */
    for (i=0; i<8; i++)
    {
        /* Set SCL to low */
        swI2CSCL(ufb,0);

        /* Send data bit */
        if ((value & 0x80) != 0)
            swI2CSDA(ufb,1);
        else
            swI2CSDA(ufb,0);

        swI2CWait();

        /* Toggle clk line to one */
        swI2CSCL(ufb,1);

        /* Shift byte to be sent */
        value = value << 1;
    }

    /* Set the SCL Low and SDA High (prepare to get input) */
    swI2CSCL(ufb,0);
    swI2CSDA(ufb,1);

    /* Set the SCL High for ack */
    swI2CWait();
    swI2CSCL(ufb,1);

    /* Read SDA, until SDA==0 */
    for(i=0; i<0xff; i++) 
    {
        swI2CWait();
        swI2CWait();
        if (!swI2CReadSDA(ufb))
            break;
    }

    /* Set the SCL Low and SDA High */
    swI2CSCL(ufb,0);
    swI2CSDA(ufb,1);

    if (i<0xff)
        return 0;
    else
        return (-1);
}

/*
 *  This function reads one byte from the slave device
 *
 *  Parameters:
 *      ack	- Flag to indicate either to send the acknowledge
 *            message to the slave device or not
 *
 *  Return Value:
 *      One byte data read from the Slave device
 */
unsigned char swI2CReadByte(void * ufb,unsigned char ack)
{
    int i;
    unsigned char data = 0;

    for(i=7; i>=0; i--)
    {
        /* Set the SCL to Low and SDA to High (Input) */
        swI2CSCL(ufb,0);
        swI2CSDA(ufb,1);
        swI2CWait();

        /* Set the SCL High */
        swI2CSCL(ufb,1);
        swI2CWait();

        /* Read data bits from SDA */
        data |= (swI2CReadSDA(ufb) << i);
    }

    if (ack)
        swI2CAck();

    /* Set the SCL Low and SDA High */
    swI2CSCL(ufb,0);
    swI2CSDA(ufb,1);

    return data;
}
//#pragma optimize( "", on )

/*
 * This function initializes the i2c attributes and bus
 *
 * Parameters:
 *      i2cClkGPIO  - The GPIO pin to be used as i2c SCL
 *      i2cDataGPIO - The GPIO pin to be used as i2c SDA
 *
 * Return Value:
 *      -1   - Fail to initialize the i2c
 *       0   - Success
 */
long swI2CInit(
		void * ufb,
    unsigned char i2cClkGPIO, 
    unsigned char i2cDataGPIO
)
{
    int i;
#if 0
    unsigned long value;
    unsigned long gate;
#endif
    
    /* Return 0 if the GPIO pins to be used is out of range. The range is only from [0..63] */
    if ((i2cClkGPIO > 31) || (i2cDataGPIO > 31))
        return (-1);
    
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
    poke32(ufb,g_i2cClkGPIOMuxReg, 
                      peek32(ufb,g_i2cClkGPIOMuxReg) & ~(1 << g_i2cClockGPIO));
    poke32(ufb,g_i2cDataGPIOMuxReg, 
                      peek32(ufb,g_i2cDataGPIOMuxReg) & ~(1 << g_i2cDataGPIO));

    /* Enable GPIO power */
    enableGPIO(ufb,1);

    /* Clear the i2c lines. */
    for(i=0; i<9; i++) 
        swI2CStop(ufb);
    
    return 0;
}

/*
 *  This function reads the slave device's register
 *
 *  Parameters:
 *      deviceAddress   - i2c Slave device address which register
 *                        to be read from
 *      registerIndex   - Slave device's register to be read
 *
 *  Return Value:
 *      Register value
 */
unsigned char swI2CReadReg(
		void * ufb,
		unsigned char deviceAddress, 
		unsigned char registerIndex
)
{
    unsigned char data;

    /* Send the Start signal */
    swI2CStart(ufb);

    /* Send the device address */
    swI2CWriteByte(ufb,deviceAddress);                                                  

    /* Send the register index */
    swI2CWriteByte(ufb,registerIndex);               

    /* Get the bus again and get the data from the device read address */
    swI2CStart(ufb);
    swI2CWriteByte(ufb,deviceAddress + 1);
    data = swI2CReadByte(ufb,1);

    /* Stop swI2C and release the bus */
    swI2CStop(ufb);

    return data;
}

/*
 *  This function writes a value to the slave device's register
 *
 *  Parameters:
 *      deviceAddress   - i2c Slave device address which register
 *                        to be written
 *      registerIndex   - Slave device's register to be written
 *      data            - Data to be written to the register
 *
 *  Result:
 *          0   - Success
 *         -1   - Fail
 */
long swI2CWriteReg(
		void * ufb,
		unsigned char deviceAddress, 
		unsigned char registerIndex, 
		unsigned char data
)
{
    long returnValue = 0;
    
    /* Send the Start signal */
    swI2CStart(ufb);

    /* Send the device address and read the data. All should return success
       in order for the writing processed to be successful
     */
    if ((swI2CWriteByte(ufb,deviceAddress) != 0) ||
        (swI2CWriteByte(ufb,registerIndex) != 0) ||
        (swI2CWriteByte(ufb,data) != 0))
    {
        returnValue = -1;
    }
    
    /* Stop i2c and release the bus */
    swI2CStop(ufb);

    return returnValue;
}
