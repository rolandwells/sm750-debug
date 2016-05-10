/*******************************************************************
* 
*         Copyright (c) 2009 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  edid.c --- SM750 DDK 
*  This file contains functions to interpret the EDID structure.
* 
*******************************************************************/
//#include <math.h>
//#include "defs.h"
#include "hardware.h"
//#include "helper.h"
#include "hwi2c.h"
#include "swi2c.h"
#include "edid.h"
//#include "ddkdebug.h"

/* Enable this one to print the VDIF timing when debug is enabled. */
//#define ENABLE_DEBUG_PRINT_VDIF

/****************************************************************
 * Configuration setting
 ****************************************************************/
/* I2C Address of each Monitor. Currently, there is only one i2c bus and
   both display devices will responds to 0xA0 address with analog CRT as the first priority
   The second version of evaluation board will separate the i2c bus, so that each i2c bus
   corresponds to one display devices.
   The new monitor devices (at least the tested DVI monitor) also corresponds to 0xA2, 
   which is temporarily used in this DDK. */
#define EDID_DEVICE_I2C_ADDRESS             0xA0

/* GPIO used for the I2C on the PANEL_PATH size  */
#define EDID_PANEL_I2C_SCL                  DEFAULT_I2C_SCL
#define EDID_PANEL_I2C_SDA                  DEFAULT_I2C_SDA

/* GPIO used for the I2C on the CRT_PATH size.
   These GPIO pins only available in the Evaluation Board version 2.2.
   Need to find out which pins are used for the CRT_PATH i2c. */
   
/* according to hardware spec of v2.2 750 demobord, crt scl,sda is 17,18 */
#define EDID_CRT_I2C_SCL                    17
#define EDID_CRT_I2C_SDA                    18

#define TOTAL_EDID_REGISTERS                128

/*
 *  edidGetVersion
 *      This function gets the EDID version
 *
 *  Input:
 *      pEDIDBuffer - Buffer that contains the EDID structure of the monitor
 *      pRevision   - Revision of the EDIE (if exist)
 *
 *  Output:
 *      Revision number of the given EDID buffer.
 */
unsigned char edidGetVersion(
    unsigned char *pEDIDBuffer,
    unsigned char *pRevision
)
{
    unsigned char version;
    
    if (pEDIDBuffer != (unsigned char *)0)
    {
        /* Check the header */
        if ((pEDIDBuffer[0] == 0x00) && (pEDIDBuffer[1] == 0xFF) && (pEDIDBuffer[2] == 0xFF) &&
            (pEDIDBuffer[3] == 0xFF) && (pEDIDBuffer[4] == 0xFF) && (pEDIDBuffer[5] == 0xFF) &&
            (pEDIDBuffer[6] == 0xFF) && (pEDIDBuffer[7] == 0x00))
        {
            /* 
             * EDID Structure Version 1.
             */
        
            /* Read the version field from the buffer. It should be 1 */
            version  = pEDIDBuffer[18];
        
            if (version == 1)
            {
                /* Copy the revision first */
                if (pRevision != (unsigned char *)0)
                    *pRevision = pEDIDBuffer[19];
                    
                return version;
            }
        }
        else
        {
            /* 
             * EDID Structure Version 2 
             */
             
            /* Read the version and revision field from the buffer. */
            version = pEDIDBuffer[0];
        
            if ((version >> 4) == 2)
            {
                /* Copy the revision */
                if (pRevision != (unsigned char *)0)
                    *pRevision = version & 0x0F;
                
                return (version >> 4);
            }
        }
    }
    
    return 0;    
}



#define EDID_TOTAL_RETRY_COUNTER            4
/*
 *  edidReadMonitor
 *      This function reads the EDID structure from the attached monitor
 *
 *  Input:
 *      displayPath - Display device which EDID to be read from.
 *      pEDIDBuffer - Buffer that contains the EDID structure of the monitor
 *      bufferSize  - The EDID Buffer size index (usually 128-bytes)
 *      edidExtNo   - Extension Index of the EDID Structure to be read
 *      sclGpio     - GPIO pin used as the I2C Clock (SCL)
 *      sdaGpio     - GPIO pin used as the I2C Data (SDA)
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
_X_EXPORT int32_t edidReadMonitorEx(
    disp_path_t displayPath,
    unsigned char *pEDIDBuffer,
    uint32_t bufferSize,
    unsigned char edidExtNo,
    unsigned char sclGpio,
    unsigned char sdaGpio
)
{
    unsigned char value, retry, edidVersion, edidRevision;
    unsigned char edidBuffer[256];
    uint32_t offset;
    
    /* Initialize the i2c bus */
#if USE_HW_I2C == 1
	hwI2CInit(1);//use fast mode
#else
    swI2CInit(sclGpio, sdaGpio);
#endif

    for (retry = 0; retry < EDID_TOTAL_RETRY_COUNTER; retry++)
    {
            
        /* Read the EDID from the monitor. */
        for (offset = 0; offset < TOTAL_EDID_REGISTERS; offset++){
#if USE_HW_I2C == 1
			edidBuffer[offset] = hwI2CReadReg(EDID_DEVICE_I2C_ADDRESS, (unsigned char)offset);
#else
            edidBuffer[offset] = swI2CReadReg(EDID_DEVICE_I2C_ADDRESS, (unsigned char)offset);
#endif
    	}
            
        /* Check if the EDID is valid. */
        edidVersion = edidGetVersion((unsigned char *)&edidBuffer, (unsigned char *)&edidRevision);
        if (edidVersion != 0)
            break;
    }
    
    /* 
     *  The monitor might not be DDC2B compliance. Therefore, need to use DDC1 protocol,
     *  which uses the Vertical Sync to clock in the EDID data.
     *  Currently this function return error. DDC1 protocol can be added later.  
     */
    if (retry == EDID_TOTAL_RETRY_COUNTER)
    {
        /* DDC1 uses the SDA line to transmit 9 bit data per byte. The last bit is
         * only an acknowledge flag, which could be high or low. However, SCL line
         * is not used. Instead the data is clock-in using vertical sync.
         */
        return (-1);
    }
    
    /* Copy the data to the given buffer */
    if (pEDIDBuffer != (unsigned char *)0)
    {
        for (offset = 0; offset < bufferSize; offset++)
            pEDIDBuffer[offset] = edidBuffer[offset];
    }
    return 0;
}

_X_EXPORT int32_t edidReadMonitorEx_software(
    disp_path_t displayPath,
    unsigned char *pEDIDBuffer,
    uint32_t bufferSize,
    unsigned char edidExtNo,
    unsigned char sclGpio,
    unsigned char sdaGpio,
    swI2C_wait_proc function
)
{
    unsigned char value, retry, edidVersion, edidRevision;
    unsigned char edidBuffer[256];
    uint32_t offset;
    
    /* Initialize the i2c bus */
    swI2CInit(sclGpio, sdaGpio,function);
    for (retry = 0; retry < EDID_TOTAL_RETRY_COUNTER; retry++)
    {
            
        /* Read the EDID from the monitor. */
        for (offset = 0; offset < TOTAL_EDID_REGISTERS; offset++){
            edidBuffer[offset] = swI2CReadReg(EDID_DEVICE_I2C_ADDRESS, (unsigned char)offset);
    	}
            
        /* Check if the EDID is valid. */
        edidVersion = edidGetVersion((unsigned char *)&edidBuffer, (unsigned char *)&edidRevision);
        if (edidVersion != 0)
            break;
    }
    
    /* 
     *  The monitor might not be DDC2B compliance. Therefore, need to use DDC1 protocol,
     *  which uses the Vertical Sync to clock in the EDID data.
     *  Currently this function return error. DDC1 protocol can be added later.  
     */
    if (retry == EDID_TOTAL_RETRY_COUNTER)
    {
        /* DDC1 uses the SDA line to transmit 9 bit data per byte. The last bit is
         * only an acknowledge flag, which could be high or low. However, SCL line
         * is not used. Instead the data is clock-in using vertical sync.
         */
        return (-1);
    }
    
    /* Copy the data to the given buffer */
    if (pEDIDBuffer != (unsigned char *)0)
    {
        for (offset = 0; offset < bufferSize; offset++)
            pEDIDBuffer[offset] = edidBuffer[offset];
    }
    return 0;
}


