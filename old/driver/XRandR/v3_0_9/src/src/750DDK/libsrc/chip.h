/*******************************************************************
* 
*         Copyright (c) 2008 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  CHIP.H --- SMI DDK 
*  This file contains the source code for the mode table.
* 
*******************************************************************/
#include <stdint.h>
#include <xf86str.h>
#ifndef _CHIP_H_
#define _CHIP_H_

/* Definition of Scratch Data Flag. Currently, only one flag is defined. */
#define SWAP_DISPLAY_CTRL_FLAG                  0x00000001

/* This is all the chips recognized by this library */
typedef enum _logical_chip_type_t
{
    SM_UNKNOWN,
    SM718,
    SM750,
}
logical_chip_type_t;

/* input struct to initChipParam() function */
typedef struct _initchip_param_t
{
    unsigned short powerMode;    /* Use power mode 0 or 1 */
    unsigned short chipClock;    /* Speed of main chip clock in MHz unit
                                    0 = keep the current clock setting
                                    Others = the new main chip clock
                                  */
    unsigned short memClock;     /* Speed of memory clock in MHz unit
                                    0 = keep the current clock setting
                                    Others = the new memory clock
                                  */
    unsigned short masterClock;  /* Speed of master clock in MHz unit 
                                    0 = keep the current clock setting
                                    Others = the new master clock
                                  */
    unsigned short setAllEngOff; /* 0 = leave all engine state untouched.
                                    1 = make sure they are off: 2D, Overlay,
                                    video alpha, alpha, hardware cursors
                                 */
    unsigned char resetMemory;   /* 0 = Do not reset the memory controller
                                    1 = Reset the memory controller
                                  */

    /* More initialization parameter can be added if needed */
}
initchip_param_t;

/*
 * This function returns frame buffer memory size in Byte units.
 */
_X_EXPORT uint32_t getFrameBufSize(void);

/*
 * This function gets the Frame buffer location.
 */
unsigned char getFrameBufLocation();

/*
 * This function returns the logical chip type defined in chip.h
 * It is one of the following: SM501, SM502, SM107, SM718, SM 750 or
 * SM_UNKNOWN.
 */
logical_chip_type_t getChipType(void);

/*
 * Return a char string name of the current chip.
 * It's convenient for application need to display the chip name.
 */
char * getChipTypeString(void);

/*
 * Initialize a single chip and environment according to input parameters.
 *
 * Input: initchip_param_t structure.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 *
 * Note:
 *      Caller needs to call the detectDevice and setCurrentDevice
 *      to set the device before calling this initChipParamEx.
 */
int32_t initChipParamEx(initchip_param_t * pInitParam);

/*
 * Initialize every chip and environment according to input parameters. 
 * (Obsolete)
 *
 * Input: initchip_param_t structure.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 */
int32_t initChipParam(initchip_param_t * pInitParam);

/*
 * Initialize a single chip with default parameters.
 *
 * Input: none.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 *
 * Note:
 *      Caller needs to call the detectDevice and setCurrentDevice
 *      to set the device before calling this initChipEx.
 */
int32_t initChipEx();

/*
 * Initialize the chip with default parameters.
 *
 * Input: none.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 */
_X_EXPORT int32_t initChip(void);

/*******************************************************************
 * Scratch Data implementation (To be used by DDK library only)
 *******************************************************************/
 
/*
 * Set Scratch Data
 */
void setScratchData(
    uint32_t dataFlag,
    uint32_t data
);

/*
 * Get Scratch Data
 */
uint32_t getScratchData(
    uint32_t dataFlag
);

#endif /* _CHIP_H_ */

