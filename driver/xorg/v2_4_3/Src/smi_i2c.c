/* Header:   //Mercury/Projects/archives/XFree86/4.0/smi_i2c.c-arc   1.10   27 Nov 2000 15:47:58   Frido  $ */

/*
Copyright (C) 1994-2000 The XFree86 Project, Inc.  All Rights Reserved.
Copyright (C) 2000 Silicon Motion, Inc.  All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the names of the XFree86 Project and
Silicon Motion shall not be used in advertising or otherwise to promote the
sale, use or other dealings in this Software without prior written
authorization from the XFree86 Project and Silicon Motion.
*/
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/siliconmotion/smi_i2c.c,v 1.2 2001/12/20 21:35:39 eich Exp $ */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "xf86.h"
#include "xf86_OSproc.h"
#include "smi.h"
#ifndef XSERVER_LIBPCIACCESS
//#if  XORG_VERSION_CURRENT <=  XORG_VERSION_NUMERIC(7,1,1,0,0)
#include "xf86_ansic.h"
#endif
#include "compiler.h"
#include "xf86Pci.h"
#include "xf86PciInfo.h"
#include "vgaHW.h"
#include "smi.h"
#include "smi_750.h"



#undef VERBLEV
#undef DEBUG_PROC
#undef DEBUG
#define VERBLEV 2
#define DEBUG_PROC(PROCNAME)


#if 1
//#define DEBUG(arg)

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

/* Default i2c CLK and Data GPIO. These are the default i2c pins */
#define DEFAULT_I2C_SCL                     30
#define DEFAULT_I2C_SDA                     31
#define SECOND_I2C_SCL				17
#define SECOND_I2C_SDA				18

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
static uint32_t g_i2cClkGPIOMuxReg;
static uint32_t g_i2cClkGPIODataReg;
static uint32_t g_i2cClkGPIODataDirReg;
static char * g_mmiobase;

/* i2c Clock GPIO Register usage */
static uint32_t g_i2cDataGPIOMuxReg;
static uint32_t g_i2cDataGPIODataReg;
static uint32_t g_i2cDataGPIODataDirReg;

static void SMI_I2CPutBits_750panel(I2CBusPtr,int,int);
static void SMI_I2CPutBits_750crt(I2CBusPtr,int,int);
static void SMI_I2CGetBits_750panel(I2CBusPtr,int*,int*);
static void SMI_I2CGetBits_750crt(I2CBusPtr,int*,int*);

void (*pfn_I2CPutBits_750[])(I2CBusPtr,int,int)=
{SMI_I2CPutBits_750panel,SMI_I2CPutBits_750crt};

void (*pfn_I2CGetBits_750[])(I2CBusPtr,int*,int*)=
{SMI_I2CGetBits_750panel,SMI_I2CGetBits_750crt};

#define 	peekRegisterDWord(I) 	(*(volatile unsigned int *)(g_mmiobase + I))
#define 	pokeRegisterDWord(I,D)	(*(volatile unsigned int *)(g_mmiobase + I)) = (D)

#define 	peekRegisterByte(I) 		(*(volatile unsigned char  *)(g_mmiobase + I))
#define   pokeRegisterByte(I, D)     (*(volatile unsigned char *)(g_mmiobase + I)) = (D)

/*******************************************************************************
    swI2CWait
        This function puts a delay between command

    Parameters:
        None

    Return Value:
        None
 *******************************************************************************/        
static void swI2CWait(int time)
{
    int i, Temp;

    for(i=0; i<time; i++)
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
     swI2CWait(600);
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
	swI2CWait(600);
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
		swI2CWait(600);

    if (ulGPIOData & (1 << g_i2cDataGPIO)) 
        return 1;
    else 
        return 0;
	
}

static unsigned char swI2CReadSCL()
{
    unsigned char data;
    uint32_t ulGPIODirection;
    uint32_t ulGPIOData;

    /* Make sure that the direction is input (High) */
    ulGPIODirection = peekRegisterDWord(g_i2cDataGPIODataDirReg);
    if ((ulGPIODirection & (1 << g_i2cClockGPIO)) != (~(1 << g_i2cClockGPIO)))
    {
        ulGPIODirection &= ~(1 << DEFAULT_I2C_SDA);
        pokeRegisterDWord(GPIO_DATA_DIRECTION, ulGPIODirection);
    }

    /* Now read the SDA line */
    ulGPIOData = peekRegisterDWord(GPIO_DATA);
    if (ulGPIOData & (1 << g_i2cClockGPIO)) 
        return 1;
    else 
        return 0;
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
 #endif

static void
SMI_I2CPutBits(I2CBusPtr b, int clock,  int data)
{
	SMIPtr pSmi = SMIPTR(xf86Screens[b->scrnIndex]);
	unsigned int reg = 0x30;


	if (clock) reg |= 0x01;
	if (data)  reg |= 0x02;

	VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x72, reg);

}

static void
SMI_I2CGetBits(I2CBusPtr b, int *clock, int *data)
{
	SMIPtr pSmi = SMIPTR(xf86Screens[b->scrnIndex]);
	unsigned int reg = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x72);
	

	*clock = reg & 0x04;
	*data  = reg & 0x08;

}

void
SMI_750_swI2CInit(ScrnInfoPtr pScrn, int is_crt)
	
{
	    int i;
	    uint32_t value;
	    uint32_t gate;
	    SMIPtr pSmi = SMIPTR(pScrn);
	
	    /* Return 0 if the GPIO pins to be used is out of range. The range is only from [0..63] */

	    /* Initialize the GPIO pin for the i2c Clock Register */
	    g_i2cClkGPIOMuxReg = GPIO_MUX;   
	    g_i2cClkGPIODataReg = GPIO_DATA;    
	    g_i2cClkGPIODataDirReg = GPIO_DATA_DIRECTION;
	    g_i2cClockGPIO = is_crt?SECOND_I2C_SCL:DEFAULT_I2C_SCL;
	    g_i2cDataGPIO = is_crt?SECOND_I2C_SDA:DEFAULT_I2C_SDA;

	    
	    /* Initialize the Clock GPIO Offset */
	    
	    /* Initialize the GPIO pin for the i2c Data Register */
	    g_i2cDataGPIOMuxReg = GPIO_MUX;    
	    g_i2cDataGPIODataReg = GPIO_DATA;    
	    g_i2cDataGPIODataDirReg = GPIO_DATA_DIRECTION;
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
}

static void SMI_I2CPutBits_750panel(I2CBusPtr bus,int clock,int data)
{	
	SMIPtr pSmi = SMIPTR(xf86Screens[bus->scrnIndex]);	
	swI2CSCL(clock);
	swI2CSDA(data);	
	swI2CWait(1000);
}

static void SMI_I2CGetBits_750panel(I2CBusPtr bus,int* clock,int* data)
{
	SMIPtr pSmi = SMIPTR(xf86Screens[bus->scrnIndex]);
	*data = swI2CReadSDA();	
	*clock = swI2CReadSCL();
	swI2CWait(1000);
}

static void SMI_I2CPutBits_750crt(I2CBusPtr bus,int clock,int data)
{	
	SMIPtr pSmi = SMIPTR(xf86Screens[bus->scrnIndex]);	
	swI2CSCL(clock);
	swI2CSDA(data);	
	swI2CWait(1000);
}

static void SMI_I2CGetBits_750crt(I2CBusPtr bus,int* clock,int* data)
{
	SMIPtr pSmi = SMIPTR(xf86Screens[bus->scrnIndex]);
	*data = swI2CReadSDA();	
	*clock = swI2CReadSCL();
	swI2CWait(1000);
}

Bool
SMI_I2CInit(ScrnInfoPtr pScrn)
{
	SMIPtr pSmi = SMIPTR(pScrn);

	    /* Initialize the Data GPIO Offset */

	if (pSmi->I2C == NULL)
	{
		I2CBusPtr I2CPtr = xf86CreateI2CBusRec();
		if (I2CPtr == NULL)
		{
			return(FALSE);
		}

		I2CPtr->BusName    = "I2C bus";
		I2CPtr->scrnIndex  = pScrn->scrnIndex;
		I2CPtr->I2CPutBits = SMI_I2CPutBits;
		I2CPtr->I2CGetBits = SMI_I2CGetBits;

		if (!xf86I2CBusInit(I2CPtr))
		{
			xf86DestroyI2CBusRec(I2CPtr, TRUE, TRUE);
			return(FALSE);
		}

		pSmi->I2C = I2CPtr;
	}

	return(TRUE);
}

void enableGPIO(uint32_t enable)
{
    uint32_t gate;
    ENTER();
    
    /* Enable GPIO Gate */
    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
        gate = FIELD_SET(gate, CURRENT_GATE, GPIO, ON);        
    else
        gate = FIELD_SET(gate, CURRENT_GATE, GPIO, OFF);
    
    setCurrentGate(gate);
    LEAVE();
}

Bool
SMI750_I2cInit(ScrnInfoPtr pScrn)
{    
    SMIPtr pSmi = SMIPTR(pScrn);
    g_mmiobase = pSmi->MapBase;
	#if 1

    SMIRegPtr	pSMIEnt = SMIEntPriv (pScrn);
    //int cnt = pSMIEnt->DualHead?2:1;
    int cnt = pSmi->IsPrimary;
    int index;
    I2CBusPtr ptr[2] = {NULL,NULL};
    static char * sm750_name[] = {"I2C Bus PanelPath","I2C Bus CrtPath"};	
    ENTER();
    index= 0 ;        
    while(index <= cnt)
    {       
        if(ptr[index] == NULL )
        {
        	I2CBusPtr I2CPtr = xf86CreateI2CBusRec();
        	if (I2CPtr == NULL)
        	    LEAVE(FALSE);
			I2CPtr->scrnIndex  = pScrn->scrnIndex;
			
			I2CPtr->BusName  = sm750_name[index];
			I2CPtr->I2CPutBits = pfn_I2CPutBits_750[index];
			I2CPtr->I2CGetBits = pfn_I2CGetBits_750[index];				
			
			        	
        	I2CPtr->DriverPrivate.ptr = (void *)xalloc(sizeof(int));

        	if (!xf86I2CBusInit(I2CPtr)){				
        	    xfree(I2CPtr->DriverPrivate.ptr);
error:
        	    xf86DestroyI2CBusRec(I2CPtr, TRUE, TRUE);
        	    LEAVE(FALSE);
        	}        	
        	ptr[index] = I2CPtr;
        	*((int *)I2CPtr->DriverPrivate.ptr) = index;
        }
        index++;
    }
    pSmi->I2C =  ptr[0];
    pSmi->I2C_primary = ptr[0];
    pSmi->I2C_secondary = ptr[1];

	/* set mux for the i2c scl/sda*/
	int val = PEEK32(GPIO_MUX);
	val = FIELD_SET(val,GPIO_MUX,31,GPIO);
	val = FIELD_SET(val,GPIO_MUX,30,GPIO);
	val = FIELD_SET(val,GPIO_MUX,18,GPIO);
	val = FIELD_SET(val,GPIO_MUX,17,GPIO);			
	POKE32(GPIO_MUX,val);
	XERR("you should see me!!\n");		
	/* enable GPIO gate*/
	enableGPIO(1);
    LEAVE(TRUE);
#endif 
}

#if 1
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

        swI2CWait(600);

        /* Toggle clk line to one */
        swI2CSCL(1);

        /* Shift byte to be sent */
        value = value << 1;
    }

    /* Set the SCL Low and SDA High (prepare to get input) */
    swI2CSCL(0);
    swI2CSDA(1);

    /* Set the SCL High for ack */
    swI2CWait(600);
    swI2CSCL(1);

    /* Read SDA, until SDA==0 */
    for(i=0; i<0xff; i++) 
    {
        swI2CWait(600);
        swI2CWait(600);
        if (!swI2CReadSDA())
            break;
    }

    /* Set the SCL Low and SDA High */
    swI2CSCL(0);
    swI2CSDA(1);

    return (i<0xff);
}

uint32_t getPowerMode()
{
    return (FIELD_GET(peekRegisterDWord(POWER_MODE_CTRL), POWER_MODE_CTRL, MODE));
}

void setCurrentGate(uint32_t gate) {
    uint32_t gate_reg;
    uint32_t mode;

	ENTER();

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
	LEAVE();
}

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
        swI2CWait(600);

        /* Set the SCL High */
        swI2CSCL(1);
        swI2CWait(600);

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

#define I2C_RESET                                       0x010042
#define MAX_HWI2C_FIFO                  		16
#define HWI2C_WAIT_TIMEOUT              0xF0000

long hwI2CWaitTXDone()
{
    unsigned long timeout;

    /* Wait until the transfer is completed. */
    timeout = HWI2C_WAIT_TIMEOUT;
	while ((FIELD_GET(peekRegisterByte(I2C_STATUS), I2C_STATUS, TX) != I2C_STATUS_TX_COMPLETED) &&
           (timeout != 0))
		timeout--;
    
	if (timeout == 0)
	    return (-1);

    return 0;
}



long hwI2CInit(
    unsigned char busSpeedMode
)
{
    unsigned long value;
    
    /* Enable GPIO 30 & 31 as IIC clock & data */
	value = peekRegisterDWord(GPIO_MUX);
    value = FIELD_SET(value, GPIO_MUX, 30, I2C) |
			FIELD_SET(0, GPIO_MUX, 31, I2C);
	pokeRegisterDWord(GPIO_MUX, value);
              
    /* Enable Hardware I2C power.
       TODO: Check if we need to enable GPIO power?
     */
    enableI2C(1);
    
    /* Enable the I2C Controller and set the bus speed mode */
    value = peekRegisterByte(I2C_CTRL);
    if (busSpeedMode == 0)
        value = FIELD_SET(value, I2C_CTRL, MODE, STANDARD);
    else
        value = FIELD_SET(value, I2C_CTRL, MODE, FAST);        
    value = FIELD_SET(value, I2C_CTRL, EN, ENABLE);
    pokeRegisterByte(I2C_CTRL, value);
    
    return 0;
}

unsigned long hwI2CWriteData(
    unsigned char deviceAddress,
    unsigned long length,
    unsigned char *pBuffer
)
{
    unsigned char value, count, i;
    unsigned long timeout, totalBytes = 0;
    
    /* Set the Device Address */
    pokeRegisterByte(I2C_SLAVE_ADDRESS, deviceAddress & ~0x01);
    
    /* Write data.
     * Note:
     *      Only 16 byte can be accessed per i2c start instruction.
     */
    do
    {
        /* Reset I2C by writing 0 to I2C_RESET register to clear the previous status. */
        pokeRegisterByte(I2C_RESET, 0);
        
        /* Set the number of bytes to be written */
        if (length < MAX_HWI2C_FIFO)
            count = length - 1;
        else
            count = MAX_HWI2C_FIFO - 1;
        pokeRegisterByte(I2C_BYTE_COUNT, count);
        
        /* Move the data to the I2C data register */
	    for (i = 0; i <= count; i++)
            pokeRegisterByte(I2C_DATA0 + i, *pBuffer++);

        /* Start the I2C */
        pokeRegisterByte(I2C_CTRL, FIELD_SET(peekRegisterByte(I2C_CTRL), I2C_CTRL, CTRL, START));
        
        /* Wait until the transfer is completed. */
        if (hwI2CWaitTXDone() != 0)
            break;
    
        /* Substract length */
        length -= (count + 1);
        
        /* Total byte written */
        totalBytes += (count + 1);
        
    } while (length > 0);
            
    return totalBytes;
}


unsigned long hwI2CReadData(
    unsigned char deviceAddress,
    unsigned long length,
    unsigned char *pBuffer
)
{
    unsigned char value, count, i;
    unsigned long totalBytes = 0; 
    
    /* Set the Device Address */
    pokeRegisterByte(I2C_SLAVE_ADDRESS, deviceAddress | 0x01);
    
    /* Read data and save them to the buffer.
     * Note:
     *      Only 16 byte can be accessed per i2c start instruction.
     */
    do
    {
        /* Reset I2C by writing 0 to I2C_RESET register to clear all the status. */
        pokeRegisterByte(I2C_RESET, 0);
        
        /* Set the number of bytes to be read */
        if (length <= MAX_HWI2C_FIFO)
            count = length - 1;
        else
            count = MAX_HWI2C_FIFO - 1;
        pokeRegisterByte(I2C_BYTE_COUNT, count);

        /* Start the I2C */
        pokeRegisterByte(I2C_CTRL, FIELD_SET(peekRegisterByte(I2C_CTRL), I2C_CTRL, CTRL, START));
        
        /* Wait until transaction done. */
        if (hwI2CWaitTXDone() != 0)
            break;

        /* Save the data to the given buffer */
        for (i = 0; i <= count; i++)
		    *pBuffer++ = peekRegisterByte(I2C_DATA0 + i);

        /* Substract length by 16 */
        length -= (count + 1);
    
        /* Number of bytes read. */
        totalBytes += (count + 1); 
        
    } while (length > 0);
    
    return totalBytes;
}





unsigned char hwI2CReadReg(
    unsigned char deviceAddress, 
    unsigned char registerIndex
)
{
    unsigned char value = (0xFF);

    if (hwI2CWriteData(deviceAddress, 1, &registerIndex) == 1)
        hwI2CReadData(deviceAddress, 1, &value);

    return value;
}






#endif
