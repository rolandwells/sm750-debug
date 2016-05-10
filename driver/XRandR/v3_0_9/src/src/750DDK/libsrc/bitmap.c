/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  bitmap.c --- Voyager GX SDK 
*  This file contains the source code for the bitmap functions.
* 
*******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "bitmap.h"
#include "hardware.h"

/* 
 * Calculate the Y conversion value for the specific RGB value 
 */
unsigned char Y(
    unsigned char red, 
    unsigned char green, 
    unsigned char blue
)
{
    int32_t y;

    y = (16500 + 257 * red + 504 * green + 98 * blue) / 1000;

    if (y < 0) y = 0;
    else if (y > 255) y = 255;

    return (unsigned char) y;
}

/* 
 * Calculate the U conversion value for the specific RGB value 
 */
unsigned char U(
    unsigned char red, 
    unsigned char green, 
    unsigned char blue
)
{
    int32_t u;

    u = (128500 - 148 * red - 291 * green + 439 * blue) / 1000;

    if (u < 0) u = 0;
    else if (u > 255) u = 255;

    return (unsigned char) u;
}

/* 
 * Calculate the V conversion value for the specific RGB value 
 */
unsigned char V(
    unsigned char red, 
    unsigned char green, 
    unsigned char blue
)
{
    int32_t v;

    v = (128500 + 439 * red - 368 * green - 71 * blue) / 1000;

    if (v < 0) v = 0;
    else if (v > 255) v = 255;

    return (unsigned char) v;
}

/* 
 * Find the index of the pixel for the given RGB value
 */
unsigned char findPixel(
    unsigned char red, 
    unsigned char green,
    unsigned char blue
)
{
    unsigned char index;

    if (red == green && green == blue)
    {
        /* Gray color */
        index = 216 + ((red * 1000) / 654 + 5) / 10;
    }
    else
    {
        /* There are 6 even colors each for red, green and blue */
        index = ((red * 10) / 51 + 5) / 10;
        index = index * 6 + ((green * 10) / 51 + 5) / 10;
        index = index * 6 + ((blue * 10) / 51 + 5) / 10;
    }

    return(index);
}

/* 
 * Get the bitmap info.
 *
 * Ouput:
 *      0   - Success
 *      -1  - File not found
 *      -2  - Invalid bitmap file
 *
 * Note:
 *      This function needs to be called first to know the bitmap information before
 *      calling the loadBitmapEx with system memory as the destination.
 */
short getBitmapInfo(
    const char *file, 
    bitmap_header_t *bitmap_header
)
{
    FILE *f;
    bitmap_file_header_t file_header;
    
    /* Open file */
    f = fopen(file, "rb");
    if (f == (FILE *)0)
    {
        return(-1);
    }

    /* Read file header, verify bitmap type, read bitmap header */
    if ((fread(&file_header, sizeof(file_header), 1, f) != 1)
      || (file_header.bitmap_file_type != 'MB')
      || (fread(bitmap_header, sizeof(bitmap_header_t), 1, f) != 1))
    {
        fclose(f);
        return(-2);
    }
    
    /* Close the file */
    fclose(f);
    
    return 0;
}

#if 0
/*
 * For testing purpose 
 */
 
/* 
 * Load the bitmap of the given bitmap file
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
)
{
    int32_t x, y;
    unsigned char red, green, blue, index;
    unsigned short rgb16;
    uint32_t srcOffset, dstOffset;
    
    if (((srcBpp > 0) && (srcBpp <= 8)) || 
        ((dstBpp > 0) && (dstBpp <= 8)))
        return (-1);

    /* Load all lines */
    for (y = 0; y < height; y++)
    {
        srcOffset = 0;
        dstOffset = 0;
        
        /* Load all pixels */
        for (x = 0; x < width; x++)
        {
            switch (srcBpp)
            {
                case 16:
                    rgb16 = peekWord(srcAddress + srcOffset);
                    red = ((rgb16 & 0xF800) >> 8) | ((rgb16 & 0xE000) >> 13);
                    green = ((rgb16 & 0x07E0) >> 3) | ((rgb16 & 0x0600) >> 9);
                    blue = ((rgb16 & 0x001F) << 3) | ((rgb16 & 0x001C) >> 2);
                    srcOffset+=2;
                    break;

                case 32:
                    blue = peekByte(srcAddress + srcOffset + 1);
                    green = peekByte(srcAddress + srcOffset + 2);
                    red = peekByte(srcAddress + srcOffset + 3);
                    index = peekByte(srcAddress + srcOffset + 4);
                    srcOffset+=4;
                    break;

                default:
                    return(-1);
            }
            
            switch (dstBpp)
            {
                case 16:
                    pokeWord(dstAddress + dstOffset, RGB16(red, green, blue));
                    dstOffset += 2;
                    break;

                case 32:
                    pokeDWord(dstAddress + dstOffset, RGB(red, green, blue));
                    dstOffset += 4;
                    break;

                case BPP_YUV:
                    /* Store Y value */
                    pokeByte(dstAddress + dstOffset, Y(red, green, blue));
                    if ((x & 1) == 0)
                    {
                        /* Even pixels: store U and V values */
                        pokeByte(dstAddress + dstOffset + 1, U(red, green, blue));
                        pokeByte(dstAddress + dstOffset + 3, V(red, green, blue));
                    }
                    dstOffset += 2;
                    break;
    
            }
        }

        /* Address next line */
        srcAddress += srcPitch;
        dstAddress += dstPitch;
    }
    
    return 0;
}
#endif

/* 
 * Load the bitmap of the given bitmap file
 *
 * Input:
 *      file            - filename
 *      bitmap_header   - pointer to a bitmap_header_t variable to store the bitmap
 *                        information
 *      systemMemory    - Flag to indicate system Memory or local memory
 *                        0 - Local Memory
 *                        1 - System Memory
 *      address         - Address of the memory. If 0, then it points to the starting
 *                        address in the local memory to load the bitmap. If 1, then
 *                        the address is a system memory.
 *      bits_per_pixel  - The bpp of the destination display.
 *
 * Ouput:
 *      0   - Success
 *      -1  - File not found
 *      -2  - Invalid bitmap file
 *      -3  - Invalid bitmap format 
 */
short loadBitmapEx(
    const char *file, 
    bitmap_header_t *bitmap_header,
    unsigned char systemMemory,
    uint32_t address, 
    short bits_per_pixel
)
{
    FILE *f;
    bitmap_file_header_t file_header;
    int32_t x, y;
    uint32_t pitch, bitmap_pitch;
    uint32_t line;
    unsigned char red, green, blue, index;
    unsigned short rgb16;
    bitmap_rgb_t rgb_lookup[256];
    uint32_t offset;
    short returnValue;
    
    unsigned char *pByteMemory = (unsigned char *)0;

    /* Open file */
    f = fopen(file, "rb");
    if (f == (FILE *)0)
    {
        return(-1);
    }

    /* Read file header, verify bitmap type, read bitmap header */
    if ((fread(&file_header, sizeof(file_header), 1, f) != 1)
      || (file_header.bitmap_file_type != 'MB')
      || (fread(bitmap_header, sizeof(bitmap_header_t), 1, f) != 1))
    {
        fclose(f);
        return(-2);
    }        

    /* Verify bitmap compression and plane count */
    if ((bitmap_header->bitmap_compression != BITMAP_RGB)
      || (bitmap_header->bitmap_planes != 1))
    {
        fclose(f);
        return(-3);
    }

    /* Load color lookup table */
    if ((bitmap_header->bitmap_colors_used == 0)
      && (bitmap_header->bitmap_bit_count <= 8))
    {
        bitmap_header->bitmap_colors_used =
            1 << bitmap_header->bitmap_bit_count;
    }

    if (bitmap_header->bitmap_colors_used > 0)
    {
        fseek(f, sizeof(file_header) + bitmap_header->bitmap_size, SEEK_SET);
        fread(rgb_lookup,
              sizeof(bitmap_rgb_t),
              bitmap_header->bitmap_colors_used,
              f);
    }

    /* Calculate pitches */
    bitmap_pitch = (file_header.bitmap_file_size
                 - file_header.bitmap_file_bits_offset)
                 / bitmap_header->bitmap_height;

    pitch = bitmap_header->bitmap_width * abs(bits_per_pixel) / 8;
    pitch = (pitch + 15) & ~15;

    /* Load all lines */
    for (y = 0; y < abs(bitmap_header->bitmap_height); y++)
    {
        pByteMemory = (unsigned char *)address;
        
        /* Seek to line */
        if (bitmap_header->bitmap_height > 0)
        {
            line = bitmap_header->bitmap_height - y - 1;
        }
        else
        {
            line = y;
        }

        fseek(f,
              file_header.bitmap_file_bits_offset + line * bitmap_pitch,
              SEEK_SET);

        offset = 0;

        /* Load all pixels */
        for (x = 0; x < bitmap_header->bitmap_width; x++)
        {
            switch (bitmap_header->bitmap_bit_count)
            {
                case 4:
                    if (x & 1)
                    {
                        red = rgb_lookup[index & 0x0F].red;
                        green = rgb_lookup[index & 0x0F].green;
                        blue = rgb_lookup[index & 0x0F].blue;
                    }
                    else
                    {
                        fread(&index, 1, 1, f);
                        red = rgb_lookup[index >> 4].red;
                        green = rgb_lookup[index >> 4].green;
                        blue = rgb_lookup[index >> 4].blue;
                    }
                    break;

                case 8:
                    fread(&index, 1, 1, f);
                    red = rgb_lookup[index].red;
                    green = rgb_lookup[index].green;
                    blue = rgb_lookup[index].blue;
                    break;

                case 16:
                    fread(&rgb16, 2, 1, f);
                    red = ((rgb16 & 0xF800) >> 8) | ((rgb16 & 0xE000) >> 13);
                    green = ((rgb16 & 0x07E0) >> 3) | ((rgb16 & 0x0600) >> 9);
                    blue = ((rgb16 & 0x001F) << 3) | ((rgb16 & 0x001C) >> 2);
                    break;

                case 24:
                    fread(&blue, 1, 1, f);
                    fread(&green, 1, 1, f);
                    fread(&red, 1, 1, f);
                    break;

                case 32:
                    fread(&blue, 1, 1, f);
                    fread(&green, 1, 1, f);
                    fread(&red, 1, 1, f);
                    fread(&index, 1, 1, f);
                    break;

                default:
                    return(-3);
            }
          
            if (systemMemory == 1)
            {
                switch (bits_per_pixel)
                {
                    case 8:
                        pByteMemory[offset] = findPixel(red, green, blue);
                        offset += 1;
                        break;
                    case 16:
                        pByteMemory[offset] = (unsigned char)RGB16(red, green, blue);
                        pByteMemory[offset + 1] = (unsigned char)(RGB16(red, green, blue) >> 8);
                        offset += 2;
                        break;
                    case 32:
                        pByteMemory[offset] = (unsigned char)RGB(red, green, blue);
                        pByteMemory[offset + 1] = (unsigned char)(RGB(red, green, blue) >> 8);
                        pByteMemory[offset + 2] = (unsigned char)(RGB(red, green, blue) >> 16);
                        pByteMemory[offset + 3] = (unsigned char)(RGB(red, green, blue) >> 24);
                        offset += 4;
                        break;
                    case BPP_YUV:
                        /* Store Y value */
                        pByteMemory[offset] = Y(red, green, blue);
                        if ((x & 1) == 0)
                        {
                            /* Even pixels: store U and V values */
                            pByteMemory[offset + 1] = U(red, green, blue);
                            pByteMemory[offset + 3] = V(red, green, blue);
                        }
                        offset += 2;
                        break; 
                }
            }
            else
            {
                switch (bits_per_pixel)
                {
                    case 8:
                        pokeByte(address + offset, findPixel(red, green, blue));
                        offset += 1;
                        break;

                    case 16:
                        pokeWord(address + offset, RGB16(red, green, blue));
                        offset += 2;
                        break;

                    case 32:
                        pokeDWord(address + offset, RGB(red, green, blue));
                        offset += 4;
                        break;

                    case BPP_YUV:
                        /* Store Y value */
                        pokeByte(address + offset, Y(red, green, blue));
                        if ((x & 1) == 0)
                        {
                            /* Even pixels: store U and V values */
                            pokeByte(address + offset + 1, U(red, green, blue));
                            pokeByte(address + offset + 3, V(red, green, blue));
                        }
                        offset += 2;
                        break;
        
                }
            }
        }
        
        if (systemMemory == 1)
        {        
            /* Fill remainder of the line with black pixels */
            while (offset < pitch)
            {
                switch (bits_per_pixel)
                {
                    case 8:
                        pByteMemory[offset] = findPixel(0, 0, 0);
                        offset += 1;
                        break;

                    case 16:
                        pByteMemory[offset] = RGB16(0, 0, 0);
                        pByteMemory[offset + 1] = (unsigned char)(RGB16(0, 0, 0) >> 8);
                        offset += 2;
                        break;

                    case 32:
                        pByteMemory[offset] = (unsigned char)RGB(0, 0, 0);
                        pByteMemory[offset + 1] = (unsigned char)(RGB(0, 0, 0) >> 8);
                        pByteMemory[offset + 2] = (unsigned char)(RGB(0, 0, 0) >> 16);
                        pByteMemory[offset + 3] = (unsigned char)(RGB(0, 0, 0) >> 24);
                        offset += 4;
                        break;

                    case BPP_YUV:
                        pByteMemory[offset] = Y(0, 0, 0);
                        if ((x & 1) == 0)
                        {
                            /* Even pixels: store U and V values */
                            pByteMemory[offset + 1] = U(0, 0, 0);
                            pByteMemory[offset + 3] = V(0, 0, 0);
                        }
                        offset += 2;
                        break;
                }
                
                /* Increment x */
                x++;
            }
        }
        else    
        {
            /* Fill remainder of the line with black pixels */
            while (offset < pitch)
            {
                switch (bits_per_pixel)
                {
                    case 8:
                        pokeByte(address + offset, findPixel(0, 0, 0));
                        offset += 1;
                        break;

                    case 16:
                        pokeWord(address + offset, RGB16(0, 0, 0));
                        offset += 2;
                        break;

                    case 32:
                        pokeDWord(address + offset, RGB(0, 0, 0));
                        offset += 4;
                        break;

                    case BPP_YUV:
                        pokeByte(address + offset, Y(0, 0, 0));
                        if ((x & 1) == 0)
                        {
                            /* Even pixels: store U and V values */
                            pokeByte(address + offset + 1, U(0, 0, 0));
                            pokeByte(address + offset + 3, V(0, 0, 0));
                        }
                        offset += 2;
                        break;
                }
                
                /* Increment x */
                x++;
            }
        }

        /* Address next line */
        address += pitch;
    }

    fclose(f);
    return 0;
}

/* 
 * Load the bitmap of the given bitmap file
 *
 * Ouput:
 *      0   - Success
 *      -1  - File not found
 *      -2  - Invalid bitmap file
 *      -3  - Invalid bitmap format 
 */
short loadBitmap(
    const char *file, 
    bitmap_header_t *bitmap_header,
    uint32_t address, 
    short bits_per_pixel
)
{
    return loadBitmapEx(file, bitmap_header, 0, address, bits_per_pixel);
}

/* 
 * Find the match RGB value
 *
 * Ouput:
 *      >= 0    - RGB Value
 *      -1      - Fail to find the RGB Value
 */
char find_match(
    bitmap_rgb_t rgb_cursor[3], 
    unsigned char red,
    unsigned char green, 
    unsigned char blue
)
{
    char i;

    if (red == 0xFF && green == 0xFF && blue == 0xFF)
    {
        return(0);
    }

    for (i = 0; i < 3; i++)
    {
        if (rgb_cursor[i].reserved)
        {
            if ((rgb_cursor[i].red == red)
              && (rgb_cursor[i].green == green)
              && (rgb_cursor[i].blue == blue))
            {
                return(1 + i);
            }
        }
        else
        {
            rgb_cursor[i].red = red;
            rgb_cursor[i].green = green;
            rgb_cursor[i].blue = blue;
            rgb_cursor[i].reserved = 1;

            return(1 + i);
        }
    }

    return(-1);
}

/* 
 * Load Cursor
 *
 * Ouput:
 *      0   - Success
 *      -1  - File not found
 *      -2  - Invalid bitmap file
 *      -3  - Invalid bitmap format 
 */
short loadCursor(
    const char *file, 
    bitmap_header_t *bitmap_header,
    uint32_t address
)
{
    FILE *f;
    bitmap_file_header_t file_header;
    int32_t x, y;
    uint32_t bitmap_pitch;
    uint32_t line;
    unsigned char red, green, blue, index;
    char bit_values;
    unsigned char pixel;
    unsigned short rgb16;
    bitmap_rgb_t rgb_lookup[256];
    bitmap_rgb_t rgb_cursor[3];

    /* Open file */
    f = fopen(file, "rb");
    if (f == (FILE *)0)
    {
        return(-1);
    }

    /* Read file header, verify bitmap type, read bitmap header */
    if ((fread(&file_header, sizeof(file_header), 1, f) != 1)
      || (file_header.bitmap_file_type != 'MB')
      || (fread(bitmap_header, sizeof(bitmap_header_t), 1, f) != 1))
    {
        fclose(f);
        return(-2);
    }

    /* Verify bitmap compression and plane count */
    if ((bitmap_header->bitmap_compression != BITMAP_RGB)
      || (bitmap_header->bitmap_planes != 1))
    {
        fclose(f);
        return(-3);
    }

    /* Load color lookup table */
    if ((bitmap_header->bitmap_colors_used == 0)
      && (bitmap_header->bitmap_bit_count <= 8))
    {
        bitmap_header->bitmap_colors_used =
            1 << bitmap_header->bitmap_bit_count;
    }

    if (bitmap_header->bitmap_colors_used > 0)
    {
        fseek(f, sizeof(file_header) + bitmap_header->bitmap_size, SEEK_SET);
        fread(rgb_lookup,
              sizeof(bitmap_rgb_t),
              bitmap_header->bitmap_colors_used,
              f);
    }

    /* Calculate pitches */
    bitmap_pitch = (file_header.bitmap_file_size
                 - file_header.bitmap_file_bits_offset)
                 / bitmap_header->bitmap_height;

    /* Initialize cursor lookup  table */
    memset(rgb_cursor, 0, sizeof(rgb_cursor));

    /* Load all lines */
    for (y = 0; y < 64; y++)
    {
        if (y < abs(bitmap_header->bitmap_height))
        {
            /* Seek to line */
            if (bitmap_header->bitmap_height > 0)
            {
                line = bitmap_header->bitmap_height - y - 1;
            }
            else
            {
                line = y;
            }

            fseek(f,
                  file_header.bitmap_file_bits_offset + line * bitmap_pitch,
                  SEEK_SET);

            /* Load all pixels */
            for (x = 0; x < 64; x++)
            {
                if (x < bitmap_header->bitmap_width)
                {
                  switch (bitmap_header->bitmap_bit_count)
                  {
                    case 4:
                        if (x & 1)
                        {
                            red = rgb_lookup[index & 0x0F].red;
                            green = rgb_lookup[index & 0x0F].green;
                            blue = rgb_lookup[index & 0x0F].blue;
                        }
                        else
                        {
                            fread(&index, 1, 1, f);
                            red = rgb_lookup[index >> 4].red;
                            green = rgb_lookup[index >> 4].green;
                            blue = rgb_lookup[index >> 4].blue;
                        }
                        break;

                    case 8:
                        fread(&index, 1, 1, f);
                        red = rgb_lookup[index].red;
                        green = rgb_lookup[index].green;
                        blue = rgb_lookup[index].blue;
                        break;

                    case 16:
                        fread(&rgb16, 2, 1, f);
                        red = ((rgb16 & 0xF800) >> 8)
                            | ((rgb16 & 0xE000) >> 13);
                        green = ((rgb16 & 0x07E0) >> 3)
                              | ((rgb16 & 0x0600) >> 9);
                        blue = ((rgb16 & 0x001F) << 3)
                             | ((rgb16 & 0x001C) >> 2);
                        break;

                    case 24:
                        fread(&blue, 1, 1, f);
                        fread(&green, 1, 1, f);
                        fread(&red, 1, 1, f);
                        break;

                    case 32:
                        fread(&blue, 1, 1, f);
                        fread(&green, 1, 1, f);
                        fread(&red, 1, 1, f);
                        fread(&index, 1, 1, f);
                        break;

                    default:
                        return(-3);
                  }

                  /* Match the RGB value */
                  bit_values = find_match(rgb_cursor, red, green, blue);
                  if (bit_values == (char)(-1))
                  {
                        return(-3);
                  }
                }
                else
                {
                    /* Extra pixels, fill with transparent pixels */
                    bit_values = 0;
                }

                /* Copy bits into cursor byte */
                switch (x & 3)
                {
                  case 0:
                    pixel = bit_values;
                    break;

                  case 1:
                    pixel |= bit_values << 2;
                    break;

                  case 2:
                    pixel |= bit_values  << 4;
                    break;

                  case 3:
                    pixel |= bit_values  << 6;
                    pokeByte(address, pixel);
                    address++;
                    break;
                }
            }
        }
        else
        {
            /* Fill remaining lines with transparent pixels */
            for (x = 0; x < 64 * 2 / 8; x++)
            {
                pokeByte(address, 0);
                address++;
            }
        }
    }

    fclose(f);
    return 0;
}
