/*******************************************************************
* 
*         Copyright (c) 2008 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  burst.h --- Voyager GX SDK 
*  This file contains the definitions for the burst functions.
* 
*******************************************************************/
#ifndef _BURST_H_
#define _BURST_H_

/*
 * This function enables/disables the burst mode
 */
void burstEnable(
    unsigned char enable,
    unsigned char readSize
);

/*
 * This function reads from the source Memory Address to destination
 * memory address using burst BYTE, WORD, or DWORD mode.
 *
 * Input:   srcMemSelect    - Source Memory Selection
 *                            0 = Local Memory
 *                            1 = System Memory
 *          srcAddress      - Source Address of the Local or System Memory
 *          dstMemSelect    - Destination Memory Selection
 *                            0 = Local Memory
 *                            1 = System Memory
 *          dstAddress      - Destination Address of the Local or System Memory
 *          burstMode       - Burst Mode
 *                            0 = BYTE mode
 *                            1 = WORD mode
 *                            2 = DWORD mode
 */
void burstCopy(
    unsigned char srcMemSelect,
    uint32_t srcAddress,
    unsigned char dstMemSelect,
    uint32_t dstAddress,
    unsigned char burstMode, 
    uint32_t length
);

#endif  /* BURST_H_ */
