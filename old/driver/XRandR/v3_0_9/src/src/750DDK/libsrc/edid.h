/*******************************************************************
* 
*         Copyright (c) 2009 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  edid.h --- SMI DDK 
*  This file contains the EDID structure and function API to
*  interpret the EDID
* 
*******************************************************************/
#ifndef _SMI_EDID_H_
#define _SMI_EDID_H_


#include "display.h"
//#include "vdif.h"



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
);


#endif  /* _EDID_H_ */


