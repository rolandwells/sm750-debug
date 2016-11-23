/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  AT25F512.H --- Voyager GX SDK 
*  This file contains the definitions for the AT25F512.
* 
*******************************************************************/

#ifndef _AT25F512_H_
#define _AT25F512_H_

/* AT25F512A Commands */
#define AT25F512A_SET_STATUS        0x0001
#define AT25F512A_PROGRAM           0x0002
#define AT25F512A_READ_DATA         0x0003
#define AT25F512A_WRITE_DISABLE     0x0004
#define AT25F512A_GET_STATUS        0x0005
#define AT25F512A_WRITE_ENABLE      0x0006
#define AT25F512A_GET_DEVICE_ID     0x0015
#define AT25F512A_ERASE_SECTOR      0x0052
#define AT25F512A_ERASE_CHIP        0x0062

#define BLANKBYTE                   0xff

#endif  /* _AT25F512_H_ */
