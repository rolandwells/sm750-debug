/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  bitmap.h --- Voyager GX SDK 
*  This file contains the definitions for the bitmap functions.
* 
*******************************************************************/
#include <stdint.h>
#ifndef _BITMAP_H_
#define _BITMAP_H_

#pragma pack(2)
typedef struct _bitmap_file_header_t
{
    unsigned short bitmap_file_type;
    uint32_t bitmap_file_size;
    unsigned short bitmap_file_reserved1;
    unsigned short bitmap_file_reserved2;
    uint32_t bitmap_file_bits_offset;
}
bitmap_file_header_t;
#pragma pack()

typedef struct _bitmap_rgb_t
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char reserved;
}
bitmap_rgb_t;

typedef struct _bitmap_header_t
{
    uint32_t bitmap_size;
    int32_t bitmap_width;
    int32_t bitmap_height;
    unsigned short bitmap_planes;
    unsigned short bitmap_bit_count;
    uint32_t bitmap_compression;
    uint32_t bitmap_image_size;
    int32_t bitmap_x_pels_per_meter;
    int32_t bitmap_y_pels_per_meter;
    uint32_t bitmap_colors_used;
    uint32_t bitmap_colors_imported;
}
bitmap_header_t;

#define BITMAP_RGB                      0
#define BITMAP_RLE8                     1
#define BITMAP_RLE4                     2
#define BITMAP_BITFIELDS                3
#define BITMAP_JPEG                     4
#define BITMAP_PNG                      5

#define BPP_YUV                         -16

short getBitmapInfo(const char *file, bitmap_header_t *bitmap_header);

short loadBitmap(const char *file, bitmap_header_t *bitmap_header,
                 uint32_t address, short bits_per_pixel);
                 
short loadBitmapEx(const char *file, bitmap_header_t *bitmap_header,
                   unsigned char systemMemory,  uint32_t address, short bits_per_pixel);

short loadCursor(const char *file, bitmap_header_t *bitmap_header,
                 uint32_t address);

#if 0   /* For testing purpose */                 
/* 
 * Screen to Screen bitmap conversion
 *
 * Input:
 *      width
 *      height
 *      srcAddress
 *      srcPitch
 *      srcBpp
 *      dstAddress
 *      dstPitch
 *      dstBpp
 *
 * Ouput:
 *      0   - Success
 *      -1  - Fail
 */
short convertBitmapScreen2Screen(
    uint32_t width,
    uint32_t height,
    uint32_t srcAddress,
    uint32_t srcPitch,
    uint32_t srcBpp,
    uint32_t dstAddress,
    uint32_t dstPitch, 
    short dstBpp
);
#endif

#endif /* _BITMAP_H_ */
