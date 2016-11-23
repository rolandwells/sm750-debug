#include <stdint.h>
#include <xf86str.h>
#ifndef _CSC_H_
#define _CSC_H_

/* Wrap Around */
typedef enum _csc_wrap_around_t
{
    CSC_WRAP_DISABLE = 0,
    CSC_WRAP_ENABLE
}
csc_wrap_around_t;

/* Source Pixel format */
typedef enum _csc_src_pixel_format_t
{
    SRC_YUV422 = 0,
    SRC_YUV420I,
    SRC_YUV420,
    SRC_RGB565,
    SRC_RGBx888
}
csc_src_pixel_format_t;

/* Destination pixel format */
typedef enum _csc_dst_pixel_format_t
{
    DST_RGB565 = 0,
    DST_RGBx888
}
csc_dst_pixel_format_t;

/* Byte order for YUV422 and YUV420I */
typedef enum _csc_byte_order_t
{
    YUYV_UVUV = 0,
    UYVY_VUVU,
}
csc_byte_order_t;

typedef enum _csc_linear_filter_t
{
    CSC_FILTER_DISABLE = 0,
    CSC_FILTER_ENABLE
}
csc_linear_filter_t;

/* CSC engine status */
typedef enum _csc_status_t
{
    CSC_STATUS_STOP = 0,
    CSC_STATUS_RUN
}
csc_status_t;

/*****************************************************************************
 * Function Prototype
 *****************************************************************************/

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
);

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
);

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
);

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
    csc_byte_order_t byteOrder          /* Byte order for YUV422 and YUV420I only. Other format, write 0 */
);

/* 
 * Set the destination base address
 *
 * Input:
 *      baseAddress - Destination base address
 */
void cscSetDstBase(
    uint32_t baseAddress           /* Destination base address */
);

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
);

/* 
 * Set the linear filter control 
 */
void cscSetLinearFilterCtrl(
    csc_linear_filter_t hFilterCtrl,
    csc_linear_filter_t vFilterCtrl
);

/* 
 * Set CSC constant
 */
void cscSetConstants(
    unsigned char  yConstant,               /* Y Adjustment */
    unsigned char  redConstant,             /* Red Conversion constant */
    unsigned char  greenConstant,           /* Green Conversion constant */
    unsigned char  blueConstant             /* Blue Conversion constant */
);

/*
 * Start CSC Engine
 */
_X_EXPORT void cscStart();

/*
 * Stop CSC Engine
 */
void cscStop();

/*
 * Get CSC Engine status
 */
csc_status_t cscGetStatus();

#if 0   /* Has a bug in the hardware. Software needs to clip on the source
           side instead. 
         */
/*
 *  cscClipDstWindow
 *      This function clips the CSC window.
 *
 *  Input:
 *      top         - Number of video lines to skip on the top of the CSC window. 
 *      left        - Number of pixels to skip on the left of the CSC window.
 *      bottom      - Number of lines to skip on the bottom of the CSC window.
 *      right       - Number of pixels to skip on the right of the CSC window.
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
);
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
);

#endif  /* _CSC_H_ */
