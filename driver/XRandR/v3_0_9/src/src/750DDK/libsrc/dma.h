/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  DMA.H --- Voyager GX SDK 
*  This file contains the definitions for the DMA functions.
* 
*******************************************************************/
#include <stdint.h>
#ifndef _DMA_H_
#define _DMA_H_

#define MAX_DMA_SDRAM_LEN          0x1000000

/*
 * This function uses DMA channel 1 to transfer data between SDRAM and SDRAM.
 * 
 * Three possibility:
 * 1) Video memory to system memory.
 * 2) Video memory to video memory.
 * 3) System memory to video memory.
 *
 * Input: 
 *      srcAddr     - Source address (either system or video memory)
 *      srcExt      - Source memory address selection, 0 = video and 1 = system
 *      destAddr    - Destination address (either system or video memory)
 *      destExt     - Destination memory address selection, 0 = video and 1 = system
 *      length      - Transfer length in BYTE (max = 16M)
 *
 * Return: 
 *       0  - Success
 *      -1  - Fail
 */
int32_t dmaChannel(
    uint32_t srcAddr,
    unsigned char srcExt,
    uint32_t destAddr,
    unsigned char destExt,
    uint32_t length
);

#endif /* _DMA_H_ */
