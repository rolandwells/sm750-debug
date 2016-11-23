/*******************************************************************
* 
*         Copyright (c) 2008 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  setmode.c --- Silicon Motion DDK 
*  This file shows how to use the library to set modes.
* 
*******************************************************************/

#include "defs.h"
#include "hardware.h"
#include "ddkdebug.h"

#if 0
/* 
 * Original blockCopy from http://www.azillionmonkeys.com/qed/blockcopy.html 
 */
void blockCopy(char * pSrc, char * pDest, unsigned int length);
#pragma aux blockCopy =                         \
"               mov     eax, ecx        "       \
"               sub     ecx, edi        "       \
"               sub     ecx, eax        "       \
"               and     ecx, 3          "       \
"               sub     eax, ecx        "       \
"               jle     short LEndBytes "       \
"           rep movsb                   "       \
"               mov     ecx, eax        "       \
"               and     eax, 3          "       \
"               shr     ecx, 2          "       \
"           rep movsd                   "       \
"LEndBytes:     add     ecx, eax        "       \
"           rep movsb                   "       \
parm [ESI] [EDI] [ECX]                          \
modify [EAX ECX ESI EDI];
#endif

/*
 * Block Copy Byte using burst mode
 */
void blockCopyByte(unsigned char *pSrc, unsigned char *pDest, uint32_t length);
#pragma aux blockCopyByte =                         \
"               rep movsb                   "       \
parm [ESI] [EDI] [ECX]                              \
modify [ECX ESI EDI];

/*
 * Block Copy Word using burst mode
 */
void blockCopyWord(unsigned char *pSrc, unsigned char *pDest, uint32_t length);
#pragma aux blockCopyWord =                         \
"               shr     ecx, 1              "       \
"               rep movsw                   "       \
parm [ESI] [EDI] [ECX]                              \
modify [ECX ESI EDI];

/*
 * Block Copy DWord using burst mode
 */
void blockCopyDWord(unsigned char *pSrc, unsigned char *pDest, uint32_t length);
#pragma aux blockCopyDWord =                        \
"               shr     ecx, 2              "       \
"               rep movsd                   "       \
parm [ESI] [EDI] [ECX]                              \
modify [ECX ESI EDI];

/*
 * This function enables/disables the burst mode
 *
 * Input:   enable      - enable/disable the burst mode
 *                        0 = Disable
 *                        1 = Enable
 *          readSize    - Read Size during burst mode (ignore when burst is disabled)
 *                        0 = 1 32-word
 *                        1 = 2 32-bit words
 *                        2 = 4 32-bit words
 *                        3 = 8 32-bit words
 */
void burstEnable(
    unsigned char enable,
    unsigned char readSize
)
{
    uint32_t value;
    
    /* Read the original value */
    value = peekRegisterDWord(SYSTEM_CTRL);
    
    /* Enable Burst */
    if (enable == 1)
    {
        value = FIELD_SET(value, SYSTEM_CTRL, PCI_BURST, ON);
        value = FIELD_SET(value, SYSTEM_CTRL, PCI_MASTER, ON);
        value = FIELD_SET(value, SYSTEM_CTRL, PCI_BURST_READ, ON);
        
        switch (readSize)
        {
            case 0: 
                value = FIELD_SET(value, SYSTEM_CTRL, PCI_SLAVE_BURST_READ_SIZE, 1);
                break;
            case 1:
                value = FIELD_SET(value, SYSTEM_CTRL, PCI_SLAVE_BURST_READ_SIZE, 2);
                break;
            case 2:
                value = FIELD_SET(value, SYSTEM_CTRL, PCI_SLAVE_BURST_READ_SIZE, 4);
                break;
            case 3:
                value = FIELD_SET(value, SYSTEM_CTRL, PCI_SLAVE_BURST_READ_SIZE, 8);
                break;
        }
    }
    else
    {
        value = peekRegisterDWord(SYSTEM_CTRL);
        value = FIELD_SET(value, SYSTEM_CTRL, PCI_BURST, OFF);
        value = FIELD_SET(value, SYSTEM_CTRL, PCI_MASTER, OFF);
        value = FIELD_SET(value, SYSTEM_CTRL, PCI_BURST_READ, OFF);
        value = FIELD_SET(value, SYSTEM_CTRL, PCI_SLAVE_BURST_READ_SIZE, 1);
    }
    
    /* Write value to the register */
    pokeRegisterDWord(SYSTEM_CTRL, value);
    
    DDKDEBUGPRINT((INIT_LEVEL, "(BURST) System Control: %08x\n", peekRegisterDWord(SYSTEM_CTRL)));
}

/*
 * Block Copy Byte using burst mode
 */
void testStosd(unsigned char *pSrc, unsigned char *pDest, uint32_t length);
#pragma aux testStosd =                             \
"               mov eax, 01234567h          "       \
"               rep stosd                   "       \
parm [ESI] [EDI] [ECX]                              \
modify [EAX ECX ESI EDI];

/*
 * Block Copy Byte using burst mode
 */
void testLodsd(unsigned char *pSrc, unsigned char *pDest, uint32_t length);
#pragma aux testLodsd =                             \
"               rep lodsd                   "       \
parm [ESI] [EDI] [ECX]                              \
modify [EAX ECX ESI EDI];


/*
 * This function reads from the source Memory Address to destination
 * memory address using burst BYTE, WORD, or DWORD mode
 */
void burstCopy(
    unsigned char srcMemSelect,
    uint32_t srcAddress,
    unsigned char dstMemSelect,
    uint32_t dstAddress,
    unsigned char burstMode, 
    uint32_t length
)
{
    void *pSrcAddress, *pDstAddress;
    
    /* Translate the address to the system memory addressing if the source Address is
       in frame buffer */
    if (srcMemSelect == 0)
        pSrcAddress = getPhysicalAddress(srcAddress);
    else
        pSrcAddress = (void *)srcAddress;
        
    /* Translate the address to the system memory addressing if the destination Address is
       in frame buffer */
    if (dstMemSelect == 0)
        pDstAddress = getPhysicalAddress(dstAddress);
    else
        pDstAddress = (void *)dstAddress;
        
    switch (burstMode)
    {
        case 0:
            /* BYTE mode */
            blockCopyByte(pSrcAddress, pDstAddress, length);
            break;
        case 1:
            /* WORD mode */
            blockCopyWord(pSrcAddress, pDstAddress, length);
            break;
        case 2:
            /* DWORD mode */
#if 0            
            testStosd(pSrcAddress, pDstAddress, length);
            testLodsd(pSrcAddress, pDstAddress, length);
#else
            blockCopyDWord(pSrcAddress, pDstAddress, length);
#endif
            break;
    } 
}
