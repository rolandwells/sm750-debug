/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  CSC.C --- Voyager GX DDK 
*  This file contains source code for the Voyager Family Color
*  Space Conversion Engine
* 
*******************************************************************/

#include "defs.h"
#include "csc.h"
#include "hardware.h"
#include "power.h"

#define CSC_SCALE_CONSTANT          (1 << 13)

static uint32_t gTopAdjustment = 0;
static uint32_t gLeftAdjustment = 0;
static uint32_t gBottomAdjustment = 0;
static uint32_t gRightAdjustment = 0;

static uint32_t gActualDstWidth = 0;
static uint32_t gActualDstHeight = 0;
     
/*
 * Get CSC Engine status
 */
csc_status_t cscGetStatus()
{
    uint32_t value;
    
    value = peekRegisterDWord(CSC_CONTROL);
    
    if (FIELD_GET(value, CSC_CONTROL, STATUS) == CSC_CONTROL_STATUS_STOP)
        return CSC_STATUS_STOP;
        
    return CSC_STATUS_RUN;
}

/*
 * Stop CSC engine by stopping the current CSC operation.
 */
void cscStop()
{
    pokeRegisterDWord(CSC_CONTROL, peekRegisterDWord(CSC_CONTROL) |
                      FIELD_SET(0, CSC_CONTROL, STATUS, STOP));
}

/*
 * Start CSC Engine
 */
_X_EXPORT void cscStart()
{
    pokeRegisterDWord(CSC_CONTROL, peekRegisterDWord(CSC_CONTROL) |
                      FIELD_SET(0, CSC_CONTROL, STATUS, START));
}

/* 
 * Set CSC constants. This is only used in YUV to RGB conversion?
 *
 * Input:
 *      yConstant       - Y Adjustment
 *      redConstant     - Red Conversion constant
 *      greenConstant   - Green Conversion constant
 *      blueConstant    - Blue Conversion constant
 */
void cscSetConstants(
    unsigned char  yConstant,               /* Y Adjustment */
    unsigned char  redConstant,             /* Red Conversion constant */
    unsigned char  greenConstant,           /* Green Conversion constant */
    unsigned char  blueConstant             /* Blue Conversion constant */
)
{
    /* Set CSC constant */
    pokeRegisterDWord(CSC_CONSTANTS,
        FIELD_VALUE(0, CSC_CONSTANTS, Y, yConstant) |
        FIELD_VALUE(0, CSC_CONSTANTS, R, redConstant) |
        FIELD_VALUE(0, CSC_CONSTANTS, G, greenConstant) |
        FIELD_VALUE(0, CSC_CONSTANTS, B, blueConstant));
}

/*
 * Set the source dimension
 *
 * Input:
 *      srcWidth    - Source Width
 *      srcHeight   - Source Height
 */
void cscSetSrcDimension(
    uint32_t srcWidth,
    uint32_t srcHeight
)
{
    /* TODO: Might need to modify the source width and source height 
             when top and left CSC window needs to be clipped.
             Reserved for future implementation (as necessary).
     */
     
    /* Set the source dimension register accordingly. */
    pokeRegisterDWord(CSC_SOURCE_DIMENSION, 
                      FIELD_VALUE(0, CSC_SOURCE_DIMENSION, X, srcWidth) |
                      FIELD_VALUE(0, CSC_SOURCE_DIMENSION, Y, srcHeight));
}

/*
 * Get the source dimension
 *
 * Input:
 *      pSrcWidth   - Pointer to a variable to store the current source width
 *      pSrcHeight  - Pointer to a variable to store the current source height
 */
void cscGetSrcDimension(
    uint32_t *pSrcWidth,
    uint32_t *pSrcHeight
)
{
    uint32_t value;
    
    value = peekRegisterDWord(CSC_SOURCE_DIMENSION);
    
    if (pSrcWidth != ((uint32_t *)0))
        *pSrcWidth = FIELD_GET(value, CSC_SOURCE_DIMENSION, X);
    
    if (pSrcHeight != ((uint32_t *)0))
        *pSrcHeight = FIELD_GET(value, CSC_SOURCE_DIMENSION, Y);
}

/*
 * Set the destination dimension
 *
 * Input:
 *      dstWidth    - The width destination
 *      dstHeight   - The height destination
 */
void cscSetDstDimension(
    uint32_t dstWidth,
    uint32_t dstHeight
)
{
    uint32_t width, height, adjustment;
    
    gActualDstWidth = dstWidth;
    gActualDstHeight = dstHeight;
    
    /* Calculate the width to be programmed to the dest dimension register */
    adjustment = gRightAdjustment + gLeftAdjustment;
    width = (gActualDstWidth < adjustment) ? 0 : gActualDstWidth - adjustment;
    
    /* Calculate the height to be programmed to the dest dimension register */
    adjustment = gBottomAdjustment + gTopAdjustment;
    height = (gActualDstHeight < adjustment) ? 0 : gActualDstHeight - adjustment;    
    
    /* Set the Destination Dimension */
    pokeRegisterDWord(CSC_DESTINATION_DIMENSION,
                      FIELD_VALUE(0, CSC_DESTINATION_DIMENSION, X, width) |
                      FIELD_VALUE(0, CSC_DESTINATION_DIMENSION, Y, height));
}

/*
 * Get the destination dimension
 *
 * Input:
 *      pDstWidth   - Pointer to a variable to store the current destination width
 *      pDstHeight  - Pointer to a variable to store the current destination height
 */
void cscGetDstDimension(
    uint32_t *pDstWidth,
    uint32_t *pDstHeight
)
{    
    if (pDstWidth != ((uint32_t *)0))
        *pDstWidth = gActualDstWidth;
    
    if (pDstHeight != ((uint32_t *)0))
        *pDstHeight = gActualDstHeight;
}

/* 
 * Set the scale factor based on the given source and destination dimension
 *
 * Input:
 *      srcWidth    - The source width
 *      srcHeight   - The source height
 *      dstWidth    - The targeted destination width
 *      dstHeight   - The targeted destination height
 */
void cscScale(
    uint32_t srcWidth,
    uint32_t srcHeight,
    uint32_t dstWidth,
    uint32_t dstHeight
)
{
    uint32_t hScaleFactor, vScaleFactor;
    
    /* Calculate the scale factor specified in 3.13 format */
    hScaleFactor = CSC_SCALE_CONSTANT * (srcWidth - 1) / (dstWidth - 1);
    vScaleFactor = CSC_SCALE_CONSTANT * (srcHeight - 1) / (dstHeight - 1);
    
    pokeRegisterDWord(CSC_SCALE_FACTOR,
                      FIELD_VALUE(0, CSC_SCALE_FACTOR, HORIZONTAL, hScaleFactor) |
                      FIELD_VALUE(0, CSC_SCALE_FACTOR, VERTICAL, vScaleFactor));
}

/* 
 * Get the source base address
 *
 * Input:
 *      pYSrcAddress - Pointer to store Y Source Base Address or the RGB base address
 *      pUSrcAddress - Pointer to store U Source Base Address
 *      pVSrcAddress - Pointer to store V Source Base Address
 */
void cscGetSourceBase(
    uint32_t *pYSrcAddress,          /* Y Source Base address */
    uint32_t *pUSrcAddress,          /* U Source Base Address */
    uint32_t *pVSrcAddress           /* V Source Base Address */
)
{
    uint32_t value;
    
    /* Set Y Source Base Address */
    if (pYSrcAddress != (uint32_t *)0)
        *pYSrcAddress = FIELD_GET(peekRegisterDWord(CSC_Y_SOURCE_BASE), CSC_Y_SOURCE_BASE, ADDRESS);
    
    /* Set U Source Base Address */
    if (pUSrcAddress != (uint32_t *)0) 
        *pUSrcAddress = FIELD_GET(peekRegisterDWord(CSC_U_SOURCE_BASE), CSC_U_SOURCE_BASE, ADDRESS);
    
    /* Set V Source Base Address */
    if (pVSrcAddress != (uint32_t *)0) 
        *pVSrcAddress = FIELD_GET(peekRegisterDWord(CSC_V_SOURCE_BASE), CSC_V_SOURCE_BASE, ADDRESS);
}

/* 
 * Set the source base address
 *
 * Input:
 *      ySrcAddress - Y Source Base Address or the RGB base address
 *      uSrcAddress - U Source Base Address
 *      vSrcAddress - V Source Base Address
 */
void cscSetSourceBase(
    uint32_t ySrcAddress,          /* Y Source Base address */
    uint32_t uSrcAddress,          /* U Source Base Address */
    uint32_t vSrcAddress           /* V Source Base Address */
)
{
    uint32_t value;
    
    /* Set Y Source Base Address */
    value = FIELD_SET(0, CSC_Y_SOURCE_BASE, EXT, LOCAL) |
            FIELD_VALUE(0, CSC_Y_SOURCE_BASE, ADDRESS, ySrcAddress);
    pokeRegisterDWord(CSC_Y_SOURCE_BASE, value);
    
    /* Set U Source Base Address */
    value = FIELD_SET(0, CSC_U_SOURCE_BASE, EXT, LOCAL) |
            FIELD_VALUE(0, CSC_U_SOURCE_BASE, ADDRESS, uSrcAddress);
    pokeRegisterDWord(CSC_U_SOURCE_BASE, value);
    
    /* Set V Source Base Address */
    value = FIELD_SET(0, CSC_V_SOURCE_BASE, EXT, LOCAL) |
            FIELD_VALUE(0, CSC_V_SOURCE_BASE, ADDRESS, vSrcAddress);
    pokeRegisterDWord(CSC_V_SOURCE_BASE, value);
}

/* 
 *  Get the Source plane pitch
 *
 *  Input:
 *      pUVPitch    - Pointer to a variable to get the UV Pitch value.
 *
 *  Output:
 *      Y Pitch value
 */
uint32_t cscGetSrcPitch(
    uint32_t *pUVPitch
)
{
    uint32_t value;
    
    /* Get the source pitch in bytes/16 */
    value = peekRegisterDWord(CSC_SOURCE_PITCH);
    
    if (pUVPitch != (uint32_t *)0)
        *pUVPitch = FIELD_GET(value, CSC_SOURCE_PITCH, UV) << 4;

    return (FIELD_GET(value, CSC_SOURCE_PITCH, Y) << 4);
}

/* 
 * Setup Source Display Plane
 */
void cscSetSrcPlane(
    unsigned short xInteger,            /* Integer part of X coordinate in Y plane */            
    unsigned short xFraction,           /* Fraction part of X coordinate in Y plane */
    unsigned short yInteger,            /* Integer part of Y coordinate in Y plane */
    unsigned short yFraction,           /* Fraction part of Y coordinate in Y plane */
    uint32_t width,                /* Width of source in pixel */
    uint32_t height,               /* Height of source in lines */
    uint32_t ySrcAddress,          /* Y Source Base address */
    uint32_t uSrcAddress,          /* U Source Base Address */
    uint32_t vSrcAddress,          /* V Source Base Address */
    uint32_t yPitch,               /* Y plane pitch value in bytes */
    uint32_t uvPitch,              /* UV plane pitch value in bytes */
    csc_src_pixel_format_t srcFormat,   /* Source pixel format */
    csc_byte_order_t byteOrder          /* Byte order for YUV422 and YUV420I only */
)
{
    uint32_t value, dstWidth, dstHeight;
        
    /* Set the X Coordinate into Y-Plane */
    pokeRegisterDWord(CSC_Y_SOURCE_X, 
                      FIELD_VALUE(0, CSC_Y_SOURCE_X, INTEGER, xInteger) |
                      FIELD_VALUE(0, CSC_Y_SOURCE_X, FRACTION, xFraction));
    
    /* Set the X Coordinate into Y-Plane */
    pokeRegisterDWord(CSC_Y_SOURCE_Y, 
                      FIELD_VALUE(0, CSC_Y_SOURCE_Y, INTEGER, yInteger) |
                      FIELD_VALUE(0, CSC_Y_SOURCE_Y, FRACTION, yFraction));
    
    /* Set the Source Dimension */
    cscSetSrcDimension(width, height);
    
    /* Set the source base address */
    cscSetSourceBase(ySrcAddress, uSrcAddress, vSrcAddress);
    
    /* Set the source pitch in bytes/16 */
    pokeRegisterDWord(CSC_SOURCE_PITCH,
                      FIELD_VALUE(0, CSC_SOURCE_PITCH, Y, yPitch >> 4) |
                      FIELD_VALUE(0, CSC_SOURCE_PITCH, UV, uvPitch >> 4));
                      
    /* Set the source display pixel format */
    value = peekRegisterDWord(CSC_CONTROL);
    switch(srcFormat)
    {
       default:
       case SRC_YUV422:
           value = FIELD_SET(value, CSC_CONTROL, SOURCE_FORMAT, YUV422);
           
            /* Byte order */
            if (byteOrder == YUYV_UVUV)
                value = FIELD_SET(value, CSC_CONTROL, BYTE_ORDER, YUYV);
            else
                value = FIELD_SET(value, CSC_CONTROL, BYTE_ORDER, UYVY);
            break;
        case SRC_YUV420I:
            value = FIELD_SET(value, CSC_CONTROL, SOURCE_FORMAT, YUV420I);
            
            /* Byte Order */
            if (byteOrder == YUYV_UVUV)
                value = FIELD_SET(value, CSC_CONTROL, BYTE_ORDER, YUYV);
            else
                value = FIELD_SET(value, CSC_CONTROL, BYTE_ORDER, UYVY);
            break;
        case SRC_YUV420:
            value = FIELD_SET(value, CSC_CONTROL, SOURCE_FORMAT, YUV420);
            break;
        case SRC_RGB565:
            value = FIELD_SET(value, CSC_CONTROL, SOURCE_FORMAT, RGB565);
            break;
        case SRC_RGBx888:
            value = FIELD_SET(value, CSC_CONTROL, SOURCE_FORMAT, RGB8888);
            break;
    }
    pokeRegisterDWord(CSC_CONTROL, value);
    
    /* Calculate Scale */
    cscGetDstDimension(&dstWidth, &dstHeight);
    cscScale(width, height, dstWidth, dstHeight);
}

/* 
 * Set the destination base address
 *
 * Input:
 *      baseAddress - Destination base address
 */
void cscSetDstBase(
    uint32_t baseAddress           /* Destination base address */
)
{
    uint32_t value;
    
    /* Set the Destination base address */
    value = FIELD_SET(0, CSC_DESTINATION_BASE, EXT, LOCAL) |
            FIELD_VALUE(0, CSC_DESTINATION_BASE, ADDRESS, baseAddress);
    pokeRegisterDWord(CSC_DESTINATION_BASE, value);
}

/*
 * Setup Destination Display Plane
 */
void cscSetDstPlane(
    uint32_t x,                    /* X Coordinate of the destination */
    uint32_t y,                    /* Y Coordinate of the destination */
    uint32_t width,                /* Width of the destination display */
    uint32_t height,               /* Height of the destination display */
    uint32_t baseAddress,          /* Destination base address */
    uint32_t hPitch,               /* Horizontal pitch of destination in bytes */
    uint32_t vPitch,               /* Vertical pitch of the destination in pixels */
    csc_wrap_around_t wrapCtrl,         /* Wrap around flag */
    csc_dst_pixel_format_t dstFormat    /* Destination pixel format */
)
{
    uint32_t value, srcWidth, srcHeight;
    
    /* Set the XY Coordinate of the Destination */
    value = FIELD_VALUE(0, CSC_DESTINATION, X, x) |
            FIELD_VALUE(0, CSC_DESTINATION, Y, y);
    
    /* Set the wrap around control */                      
    value = (wrapCtrl == CSC_WRAP_ENABLE) ?
                FIELD_SET(value, CSC_DESTINATION, WRAP, ENABLE) :
                FIELD_SET(value, CSC_DESTINATION, WRAP, DISABLE);
    pokeRegisterDWord(CSC_DESTINATION, value);
    
    /* Set the Destination Dimension */
    cscSetDstDimension(width, height);
                      
    /* Set the Destination base address */
    cscSetDstBase(baseAddress);

    /* Set the destination pitch value */
    pokeRegisterDWord(CSC_DESTINATION_PITCH,
                      FIELD_VALUE(0, CSC_DESTINATION_PITCH, X, hPitch >> 4) |
                      FIELD_VALUE(0, CSC_DESTINATION_PITCH, Y, vPitch));
    
    /* Set the source display pixel format */
    value = peekRegisterDWord(CSC_CONTROL);
    switch(dstFormat)
    {
        default:
        case DST_RGB565:
            value = FIELD_SET(value, CSC_CONTROL, DESTINATION_FORMAT, RGB565);
            break;
        case DST_RGBx888:
            value = FIELD_SET(value, CSC_CONTROL, DESTINATION_FORMAT, RGB8888);
            break;
    }
    pokeRegisterDWord(CSC_CONTROL, value);
    
    /* Calculate Scale */
    cscGetSrcDimension(&srcWidth, &srcHeight);
    cscScale(srcWidth, srcHeight, width, height);                       
}

/* 
 * Set the linear filter control or also known interpolation
 *
 * Input:
 *      hFilterCtrl - Flag to enable/disable the Horizontal interpolation
 *      vFilterCtrl - Flag to enable/disable the vertical interpolation
 */
void cscSetLinearFilterCtrl(
    csc_linear_filter_t hFilterCtrl,
    csc_linear_filter_t vFilterCtrl
)
{
    uint32_t value;
    
    value = peekRegisterDWord(CSC_CONTROL);
    
    /* Enable/Disable the horizontal filter control */
    if (hFilterCtrl == CSC_FILTER_DISABLE)
        value = FIELD_SET(value, CSC_CONTROL, HORIZONTAL_FILTER, DISABLE);
    else
        value = FIELD_SET(value, CSC_CONTROL, HORIZONTAL_FILTER, ENABLE);
    
    /* Enable/Disable the vertical filter control */    
    if (vFilterCtrl == CSC_FILTER_DISABLE)
        value = FIELD_SET(value, CSC_CONTROL, VERTICAL_FILTER, DISABLE);
    else
        value = FIELD_SET(value, CSC_CONTROL, VERTICAL_FILTER, ENABLE);
        
    pokeRegisterDWord(CSC_CONTROL, value);        
}

#if 0   /* Has a bug in the hardware. Software needs to clip on the source
           side instead. 
         */
/*
 *  cscClipDstWindow
 *      This function clips the CSC window.
 *
 *  Input:
 *      left        - Number of pixels to skip on the left of the CSC window. 
 *      top         - Number of video lines to skip on the top of the CSC window. 
 *      right       - Number of pixels to skip on the right of the CSC window.
 *      bottom      - Number of lines to skip on the bottom of the CSC window.
 *
 *  Note:
 *      - There is no registers dedicated for this CSC engine window dimension. 
 *        This function is only manipulating the other registers to simulate the 
 *        window. This function can be used to clip the csc window that pass the 
 *        screen boundary.
 *      - Currently, top and left clipping is not supported. The parameters are
 *        provided here for future development.
 */
void cscClipDstWindow(
    uint32_t left,
    uint32_t top,
    uint32_t right,
    uint32_t bottom
)
{
    uint32_t value, dstWidth, dstHeight;

    /* Since top and left is not currently implemented, therefore, 
       set the variables to 0 */
    gLeftAdjustment = 0;
    gTopAdjustment = 0;    
    gRightAdjustment = right;
    gBottomAdjustment = bottom;
    
    /* Get the destination dimension */
    cscGetDstDimension(&dstWidth, &dstHeight);
    
    /* Set the destination dimension */
    cscSetDstDimension(dstWidth, dstHeight);
}
#endif

/*
 * CSC Convert.
 * 
 * To convert the source plane to the destination plane based on the given parameter.
 */
void cscConvert(
    unsigned short sxInteger,           /* Integer part of X coordinate in Y plane */            
    unsigned short sxFraction,          /* Fraction part of X coordinate in Y plane 
                                           (not used in RGB Space) */
    unsigned short syInteger,           /* Integer part of Y coordinate in Y plane */
    unsigned short syFraction,          /* Fraction part of Y coordinate in Y plane 
                                           (not used in RGB Space)*/
    uint32_t sWidth,               /* Width of source in pixel */
    uint32_t sHeight,              /* Height of source in lines */
    uint32_t sYAddress,            /* Y Source Base address or the RGB Base address */
    uint32_t sUAddress,            /* U Source Base Address (not used in RGB Space) */
    uint32_t sVAddress,            /* V Source Base Address (not used in RGB Space) */
    uint32_t sYPitch,              /* Y plane pitch value in bytes */
    uint32_t sUVPitch,             /* UV plane pitch value in bytes (not used in 
                                           RGB Space) */
    csc_src_pixel_format_t sFormat,     /* Source pixel format */
    csc_byte_order_t sByteOrder,        /* Byte order for YUV422 and YUV420I only 
                                           (not used in RGB Space) */
    uint32_t dx,                   /* X Coordinate of the destination */
    uint32_t dy,                   /* Y Coordinate of the destination */
    uint32_t dWidth,               /* Width of the destination display */
    uint32_t dHeight,              /* Height of the destination display */
    uint32_t dBase,                /* Destination base address */
    uint32_t dHorzPitch,           /* Horizontal pitch of destination in bytes */
    uint32_t dVertPitch,           /* Vertical pitch of the destination in pixels */
    csc_wrap_around_t dWrapCtrl,        /* Wrap around flag */
    csc_dst_pixel_format_t dFormat,     /* Destination pixel format */
    csc_linear_filter_t hFilterCtrl,    /* Horizontal Linear Filter Enable */
    csc_linear_filter_t vFilterCtrl,    /* Vertical Linear Filter Enable */
    unsigned char  yConstant,           /* Y Adjustment */
    unsigned char  redConstant,         /* Red Conversion constant */
    unsigned char  greenConstant,       /* Green Conversion constant */
    unsigned char  blueConstant         /* Blue Conversion constant */
)
{
    /* Set the constant YRGB value */               
    cscSetConstants(yConstant, redConstant, greenConstant, blueConstant);
    
    /* Set the source plane attributes */            
    cscSetSrcPlane(sxInteger, sxFraction, syInteger, syFraction, 
                   sWidth, sHeight, 
                   sYAddress, sUAddress, sVAddress,
                   sYPitch, sUVPitch,
                   sFormat, 
                   sByteOrder);
    
    /* Set the destination plane attributes */               
    cscSetDstPlane(dx, dy,
                   dWidth, dHeight,
                   dBase,
                   dHorzPitch, dVertPitch,
                   dWrapCtrl,
                   dFormat);
                   
    /* Enable the linear filter */
    cscSetLinearFilterCtrl(hFilterCtrl, vFilterCtrl);
    
    /* Start the CSC engine */               
    cscStart();
}

