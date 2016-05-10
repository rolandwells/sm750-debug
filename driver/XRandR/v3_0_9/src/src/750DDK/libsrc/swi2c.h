/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  I2C.H --- Voyager GX SDK 
*  This file contains the definitions for SW I2C.
* 
*******************************************************************/
#ifndef _SWI2C_H_
#define _SWI2C_H_

/* Default i2c CLK and Data GPIO. These are the default i2c pins */
#define DEFAULT_I2C_SCL                     30
#define DEFAULT_I2C_SDA                     31

typedef void (*swI2C_wait_proc)(int);
int32_t swI2CInit(
    unsigned char i2cClkGPIO, 
    unsigned char i2cDataGPIO,
    swI2C_wait_proc function
);

unsigned char swI2CReadReg(
    unsigned char deviceAddress, 
    unsigned char registerIndex
);

int32_t swI2CWriteReg(
    unsigned char deviceAddress, 
    unsigned char registerIndex, 
    unsigned char data
);




#endif  /* _SWI2C_H_ */
