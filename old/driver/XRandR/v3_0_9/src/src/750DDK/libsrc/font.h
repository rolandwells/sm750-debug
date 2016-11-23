/*
 * Voyager GX SDK
 *
 * $Workfile:   Font.h  $
 *
 */

#include <stdint.h>
#ifndef _FONT_H_
#define _FONT_H_

#define FONT8X8_CHAR_SIZE 8               /* Number of bytes to store one character */
extern unsigned char Font8x8[];           /* Pointer to 8x8 font table */
extern uint32_t FONT8X8_TOTAL_CHAR;  /* Total number of characters in the talbe */

#define FONT8X16_CHAR_SIZE 16             /* Number of bytes to store one character */
extern unsigned char Font8x16[];          /* Pointer to 8x16 font table */
extern uint32_t FONT8X16_TOTAL_CHAR; /* Total number of characters in the talbe */

#define FONT16X32_CHAR_SIZE 64            /* Number of bytes to store one character */
extern unsigned short Font16x32[];        /* Pointer to 16x32 font table */
extern uint32_t FONT16X32_TOTAL_CHAR;/* Total number of characters in the talbe */

#define FONT32X32_CHAR_SIZE 128           /* Number of bytes to store one character */
extern unsigned short Font32x32[];
extern uint32_t FONT32X32_TOTAL_CHAR;

#endif //_FONT_H_
