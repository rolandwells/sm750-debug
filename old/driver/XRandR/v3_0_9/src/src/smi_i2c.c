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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86.h"
#include "xf86_OSproc.h"
#include "compiler.h"
#include "xf86Pci.h"
#include "xf86PciInfo.h"
#include "vgaHW.h"

#include "smi.h"
#include "smi750.h"
#include "750ddk.h"


static void SMI_I2CPutBits_750panel(I2CBusPtr,int,int);
static void SMI_I2CPutBits_750crt(I2CBusPtr,int,int);
static void SMI_I2CGetBits_750panel(I2CBusPtr,int*,int*);
static void SMI_I2CGetBits_750crt(I2CBusPtr,int*,int*);

void (*pfn_I2CPutBits_750[])(I2CBusPtr,int,int)=
{SMI_I2CPutBits_750panel,SMI_I2CPutBits_750crt};

void (*pfn_I2CGetBits_750[])(I2CBusPtr,int*,int*)=
{SMI_I2CGetBits_750panel,SMI_I2CGetBits_750crt};

#define GPIO_DATA_31_30	31:30
#define GPIO_DATA_DIRECTION_31_30	31:30
#define GPIO_DATA_18_17 18:17
#define GPIO_DATA_DIRECTION_18_17	18:17

int __wait(void)
{
	int i=0,j=0;
	while(i++<1000){
		j+=i;
	}
	return j;
}
#if 0
static void SMI_I2CPutBits_750panel(I2CBusPtr bus,int clock,int data)
{	
	int val = 0,reg;
	SMIPtr pSmi = SMIPTR(xf86Screens[bus->scrnIndex]);	
	reg = PEEK32(GPIO_DATA);

	if(clock)	{val |= 1;}	
	if(data)	{val |= 2;}
	
	POKE32(GPIO_DATA_DIRECTION,
		FIELD_VALUE(PEEK32(GPIO_DATA_DIRECTION),GPIO_DATA_DIRECTION,31_30,3));
	
	POKE32(GPIO_DATA,FIELD_VALUE(reg,GPIO_DATA,31_30,val));
}

static void SMI_I2CGetBits_750panel(I2CBusPtr bus,int* clock,int* data)
{	
	int val;
	SMIPtr pSmi = SMIPTR(xf86Screens[bus->scrnIndex]);	

	POKE32(GPIO_DATA_DIRECTION,
		FIELD_VALUE(PEEK32(GPIO_DATA_DIRECTION),GPIO_DATA_DIRECTION,31_30,0));	
		
	val = FIELD_GET(PEEK32(GPIO_DATA),GPIO_DATA,31_30);
	
	*clock = val & 1;
	*data = val & 2;	
}

static void SMI_I2CPutBits_750crt(I2CBusPtr bus,int clock,int data)
{
	int val = 0,reg;
	SMIPtr pSmi = SMIPTR(xf86Screens[bus->scrnIndex]);	

	POKE32(GPIO_DATA_DIRECTION,
		FIELD_VALUE(PEEK32(GPIO_DATA_DIRECTION),GPIO_DATA_DIRECTION,18_17,0));
	reg = PEEK32(GPIO_DATA);

	if(clock) {val |= 1;}
	if(data) {val |= 2;}

	POKE32(GPIO_DATA_DIRECTION,
		FIELD_VALUE(PEEK32(GPIO_DATA_DIRECTION),GPIO_DATA_DIRECTION,18_17,3));
	POKE32(GPIO_DATA,FIELD_VALUE(reg,GPIO_DATA,18_17,val));

	__wait();
}

static void SMI_I2CGetBits_750crt(I2CBusPtr bus,int* clock,int* data)
{
	int val;
	SMIPtr pSmi = SMIPTR(xf86Screens[bus->scrnIndex]);	
	
	POKE32(GPIO_DATA_DIRECTION,
		FIELD_VALUE(PEEK32(GPIO_DATA_DIRECTION),GPIO_DATA_DIRECTION,18_17,0));
	
	val = FIELD_GET(PEEK32(GPIO_DATA),GPIO_DATA,18_17);
	
	*clock = val & 1;
	*data = val & 2;

	__wait();
}


#else

static void swI2CSCL(unsigned char value,int cline)
{
    uint32_t ulGPIOData;
    uint32_t ulGPIODirection;

    ulGPIODirection = PEEK32(GPIO_DATA_DIRECTION);
    if (value)      /* High */
    {
        /* Set direction as input. This will automatically pull the signal up. */
        ulGPIODirection &= ~(1 << cline);	
        POKE32(GPIO_DATA_DIRECTION, ulGPIODirection);
    }
    else            /* Low */
    {
        /* Set the signal down */
        ulGPIOData = PEEK32(GPIO_DATA);
        ulGPIOData &= ~(1 << cline);
        POKE32(GPIO_DATA, ulGPIOData);

        /* Set direction as output */
        ulGPIODirection |= (1 << cline);
        POKE32(GPIO_DATA_DIRECTION, ulGPIODirection);
    }
}

static void swI2CSDA(unsigned char value,int dline)
{
    uint32_t ulGPIOData;
    uint32_t ulGPIODirection;

    ulGPIODirection = PEEK32(GPIO_DATA_DIRECTION);
    if (value)      /* High */
    {
        /* Set direction as input. This will automatically pull the signal up. */
        ulGPIODirection &= ~(1 << dline);	
        POKE32(GPIO_DATA_DIRECTION, ulGPIODirection);
    }
    else            /* Low */
    {
        /* Set the signal down */
        ulGPIOData = PEEK32(GPIO_DATA);
        ulGPIOData &= ~(1 << dline);
        POKE32(GPIO_DATA, ulGPIOData);

        /* Set direction as output */
        ulGPIODirection |= (1 << dline);		
        POKE32(GPIO_DATA_DIRECTION, ulGPIODirection);
    }
}

static unsigned char swI2CReadSDA(int dline)
{
    unsigned char data;
    uint32_t ulGPIODirection;
    uint32_t ulGPIOData;

    /* Make sure that the direction is input (High) */
    ulGPIODirection = PEEK32(GPIO_DATA_DIRECTION);
    if ((ulGPIODirection & (1 << dline)) != (~(1 << dline)))
    {
        ulGPIODirection &= ~(1 << DEFAULT_I2C_SDA);
        POKE32(GPIO_DATA_DIRECTION, ulGPIODirection);
    }

    /* Now read the SDA line */
    ulGPIOData = PEEK32(GPIO_DATA);
    if (ulGPIOData & (1 << dline)) 
        return 1;
    else 
        return 0;
}

static unsigned char swI2CReadSCL(int cline)
{
    unsigned char data;
    uint32_t ulGPIODirection;
    uint32_t ulGPIOData;

    /* Make sure that the direction is input (High) */
    ulGPIODirection = PEEK32(GPIO_DATA_DIRECTION);
    if ((ulGPIODirection & (1 << cline)) != (~(1 << cline)))
    {
        ulGPIODirection &= ~(1 << DEFAULT_I2C_SDA);
        POKE32(GPIO_DATA_DIRECTION, ulGPIODirection);
    }

    /* Now read the SDA line */
    ulGPIOData = PEEK32(GPIO_DATA);
    if (ulGPIOData & (1 << cline)) 
        return 1;
    else 
        return 0;
}



static void SMI_I2CPutBits_750panel(I2CBusPtr bus,int clock,int data)
{	
	SMIPtr pSmi = SMIPTR(xf86Screens[bus->scrnIndex]);	
	swI2CSCL(clock,30);
	swI2CSDA(data,31);	
	__wait();
}

static void SMI_I2CGetBits_750panel(I2CBusPtr bus,int* clock,int* data)
{
	SMIPtr pSmi = SMIPTR(xf86Screens[bus->scrnIndex]);
	*data = swI2CReadSDA(31);	
	*clock = swI2CReadSCL(30);
	__wait();
}

static void SMI_I2CPutBits_750crt(I2CBusPtr bus,int clock,int data)
{	
	SMIPtr pSmi = SMIPTR(xf86Screens[bus->scrnIndex]);	
	swI2CSCL(clock,17);
	swI2CSDA(data,18);	
	__wait();
}

static void SMI_I2CGetBits_750crt(I2CBusPtr bus,int* clock,int* data)
{
	SMIPtr pSmi = SMIPTR(xf86Screens[bus->scrnIndex]);
	*data = swI2CReadSDA(18);	
	*clock = swI2CReadSCL(17);
	__wait();
}


#endif



static void
SMI_I2CPutBits(I2CBusPtr b, int clock,  int data)
{
    SMIPtr pSmi = SMIPTR(xf86Screens[b->scrnIndex]);
    unsigned char reg = 0x30;    
    int crtcIndex = *(int *)b->DriverPrivate.ptr;

    if (clock) reg |= 0x01;
    if (data)  reg |= 0x02;

    if(crtcIndex == 0 ){
        VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x72, reg);
    }else{
        VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x73, reg);    
    }     

    
#ifdef SMI_RECORD
    if(crtcIndex == 0){
    record_append("P",1);
    record_append(clock?"1":"0",1);
    record_append(data?"1":"0",1);
    record_append("\n",1);
    }
#endif    
}

static void
SMI_I2CGetBits(I2CBusPtr b, int *clock, int *data)
{
    SMIPtr pSmi = SMIPTR(xf86Screens[b->scrnIndex]);
    unsigned char reg;
    int crtcIndex = *(int *)b->DriverPrivate.ptr;
    
    if(crtcIndex == 0 ){
        reg = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x72);        
    }else{
        reg = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x73);
    }

    *clock = reg & 0x04;
    *data  = reg & 0x08;

#ifdef SMI_RECORD
    if(crtcIndex == 0){
    record_append("G",1);
    record_append(*clock?"1":"0",1);
    record_append(*data?"1":"0",1);
    record_append("\n",1);
    }
#endif    
    
}

Bool
SMI_I2CInit(ScrnInfoPtr pScrn)
{    
    SMIPtr pSmi = SMIPTR(pScrn);
    int cnt = pSmi->DualView?2:1;
    int index;
    I2CBusPtr ptr[2] = {NULL,NULL};
    static char * sm712_name[] = {"I2C Bus crt","I2C Bus lcd"};
	static char * sm750_name[] = {"I2C Bus PanelPath","I2C Bus CrtPath"};	
    ENTER();
    
    index= 0 ;        
    while(index < cnt)
    {       
        if(ptr[index] == NULL )
        {
        	I2CBusPtr I2CPtr = xf86CreateI2CBusRec();
        	if (I2CPtr == NULL)
        	    return FALSE;

			I2CPtr->scrnIndex  = pScrn->scrnIndex;
			
			if(IS_SM712(pSmi)){
	        	I2CPtr->BusName  = sm712_name[index];
	        	I2CPtr->I2CPutBits = SMI_I2CPutBits;
	        	I2CPtr->I2CGetBits = SMI_I2CGetBits;
				
			}else if(IS_SM750(pSmi)){
				I2CPtr->BusName  = sm750_name[index];
				I2CPtr->I2CPutBits = pfn_I2CPutBits_750[index];
				I2CPtr->I2CGetBits = pfn_I2CGetBits_750[index];				
			}else{
				XERR("Do not support I2C on %04x\n",pSmi->Chipset);
				goto error;
			}
			        	
        	I2CPtr->DriverPrivate.ptr = (void *)xalloc(sizeof(int));

        	if (!xf86I2CBusInit(I2CPtr)){				
        	    xfree(I2CPtr->DriverPrivate.ptr);
error:
        	    xf86DestroyI2CBusRec(I2CPtr, TRUE, TRUE);
        	    LEAVE(FALSE);
        	}        	
     
        	ptr[index] = I2CPtr;
			/* 	for sm712: 0 means CRT HEAD i2c bus
				for sm750: 0 means PANEL HEAD i2c bus, 
				note that head is not pll
			*/
        	*((int *)I2CPtr->DriverPrivate.ptr) = index;
        }
        index++;
    }

    pSmi->I2C_primary = ptr[0];
    pSmi->I2C_secondary = ptr[1];

	if(IS_SM750(pSmi))
	{
		/* set mux for the i2c scl/sda*/
		int val = PEEK32(GPIO_MUX);
		val = FIELD_SET(val,GPIO_MUX,31,GPIO);
		val = FIELD_SET(val,GPIO_MUX,30,GPIO);
		val = FIELD_SET(val,GPIO_MUX,18,GPIO);
		val = FIELD_SET(val,GPIO_MUX,17,GPIO);			
		POKE32(GPIO_MUX,val);
		
		/* enable GPIO gate*/
		enableGPIO(1);
	}	

    LEAVE(TRUE);
}

