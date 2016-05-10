/*******************************************************************
* 
*         Copyright (c) 2008 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  CAPTURE.H --- SMI DDK 
*  This file contains the source code for the capture device
* 
*******************************************************************/
#include <stdint.h>
#ifndef _CAPTURE_H_
#define _CAPTURE_H_

/* Enable or disable capture */
typedef enum _zv_ctrl_t
{
    CAPTURE_DISABLE = 0,
    CAPTURE_ENABLE
}
zv_ctrl_t;

/* Capture source: panel or normal. Only apply to ZV Port 1 */
typedef enum _zv_capture_source_t
{
    ZV_CAPTURE_NORMAL,
    ZV_CAPTURE_PANEL
}
zv_capture_source_t;

/* This is the available option for the Sync Decoding type for ITU-601/ITU-656 control */
typedef enum _video_sync_type_t
{
    SYNC_EMBEDDED = 0,
    SYNC_INDEPENDENT = 1
}
video_sync_type_t;

/* Video Data size */
typedef enum _video_data_size_t
{
    DATA_SIZE_8_BIT = 0,
    DATA_SIZE_16_BIT = 1
}
video_data_size_t;

/* ZV Port selection */
typedef enum _zv_port_ctrl_t
{
    ZVPORT0 = 0,
    ZVPORT1
}
zv_port_ctrl_t;

typedef struct _capture_t
{
    unsigned char  inputITU656;             /* CCRI 656 Input, 0: Disable, 1: Enable */
    unsigned char  convertYUVtoRGB;         /* YUV to RGB Conversion, 0: Disable, 1: Enable */
    unsigned char  captureControl;          /* Capture Control, 0: Continuous, 1: By S-bit */
    unsigned char  doubleBuffer;            /* Double Buffer, 0: Disable, 1: Enable */
    unsigned char  captureMethod;           /* Capture Method. 0 = BOB, 1 = WEAVE, 2 = Progressive */
    unsigned char  fieldSwap;               /* Field Swap, 0: Disable, 1: Enable */
    unsigned char  captureFormat;           /* Capture Format, 0: YUV, 1: RGB */
    unsigned char  byteSwap;                /* Byte Swap, 0: Disable, 1: Enable */
    unsigned char  uvSwap;                  /* UV Swap, 0: Disable, 1: Enable */
    unsigned char  clockPolarity;           /* CLK Polarity, 0: Active H, 1: Active L */
    unsigned char  hRefPhase;               /* Href Phase, 0: Active H, 1: Active L */
    unsigned char  vsyncPhase;              /* VSync Phase, 0: Active H, 1: Active L */
    unsigned char  fieldDetect;             /* Field Detect Method, 0: VS rising_edge, 1: VS falling_edge */
    unsigned char  horizontalAveraging;     /* Horizontal Averaging, 0: Disable, 1: Enable */
    unsigned char  delayHRef;               /* Delay Href, 0: No_Delay, 1: Delay 1 clock */
}
capture_t;

/* Connection detection */
#define SOURCE_NOT_CONNECTED                            0
#define SOURCE_CONNECTED                                1

/* Video Format definition */
#define VIDEO_FORMAT_UNKNOWN                            0
#define VIDEO_FORMAT_BLACK_WHITE                        1
#define VIDEO_FORMAT_NTSC                               2
#define VIDEO_FORMAT_PAL                                3
#define VIDEO_FORMAT_SECAM                              4


/**************************************************************************** 
    Function prototypes for the external video source chip that is connected 
    to VGX ZV, such as Philips SAA7118 video decoder. All other video decoder
    should follow this API for compatibility with this DDK.
 ****************************************************************************/

/*
 *  setCurrentVideoDecoder
 *      This function sets the current capture Device.
 *
 *  Input:
 *      ucDeviceNumber - The device number to be set/accessed 
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
char setCurrentVideoDecoder(
    unsigned char ucDeviceNumber
);

/*
 *  getCurrentVideoDecoder
 *      This function gets the current capture Device.
 *
 *  Output:
 *      The current device index.
 *      0xFF    - Device has not been initialized.
 */
unsigned char getCurrentVideoDecoder();

/*
 *   getTotalVideoDecoder 
 *       This function gets the total number of capture devices that has been
 *       initialized and detected.
 *       
 *   Output:
 *       Total number of initialized capture devices.      
 */
unsigned char getTotalVideoDecoder();

/*
 *  detectVideoDecoderInput
 *      This function detects wheteher there is a video input coming to the
 *      video decoder. 
 *
 *  Output:
 *      0 - No Video Source is not detected
 *      1 - Video Source is detected        
 */
unsigned char detectVideoDecoderInput();

/*
 *  checkVideoDecoderColorSystem
 *      This function checks the video format. 
 *
 *  Output:
 *      VIDEO_FORMAT_BLACK_WHITE    - B/W Format
 *      VIDEO_FORMAT_NTSC           - NTSC Format
 *      VIDEO_FORMAT_PAL            - PAL Format
 *      VIDEO_FORMAT_SECAM          - SECAM Format
 */
unsigned char checkVideoDecoderColorSystem();

/*
 *  configureVideoDecoder
 *      This function configures the video decoder.
 *  
 *  Input:
 *      dataSize        - Data bus where the data is transferred. Either 8_BIT
 *                        or 16_BIT
 *      syncType        - Sync type, either embedded in the data (ITU-656)
 *                        or separate (ITU-601)
 *      inputMode       - Input mode
 *                              MODE_00..MODE_05    --> CVBS Modes 1
 *                              MODE_06..MODE_0D    --> Y+C Modes 1
 *                              MODE_0E..MODE_15    --> CVBS Modes 2
 *                              MODE_16..MODE_1D    --> Y+C Modes2
 *                              MODE_1E..MODE_1F    --> CVBS Modes 3
 *                              MODE_20..MODE_21    --> Y-Pb-Pr Modes
 *                              MODE_22..MODE_2D    --> Reserved
 *                              MODE_2E..MODE_2F    --> Y-Pb-Pr Modes (Continue)
 *                              MODE_30..MODE_3F    --> RGB Modes
 *                              MODE_00..MODE_15    --> VSB Modes^
 *       
 *                          Notes:
 *                              ^ Overlapped modes. Need to set REFA(0x23[4]) = 1, 
 *                                GAFIX(0x03[2]) = 1, and DOSL(0x88[7:6]) = 0/1/2/3.
 *      invertFieldID   - Invert Input Field ID:
 *                          0   - Do not invert ID
 *                          1   - Invert the Field ID
 *      horizontalOffset- Horizontal input window start in horizontal direction. 
 *      verticalOffset  - Vertical input window start in vertical direction.
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
uint32_t configureVideoDecoder(
    video_data_size_t dataSize,
    video_sync_type_t syncType,
    unsigned char inputMode,
    unsigned char invertFieldID,
    unsigned short horizontalOffset,
    unsigned short verticalOffset
);

/*
 *  initVideoDecoder
 *      This function initialize the Video Capture Chip. 
 *
 *  Output:
 *      Total number of chip being detected.
 */
unsigned char initVideoDecoder();

/*
 *  setVideoDecoderSyncType
 *      This function sets the sync type, either embedded in the data (ITU-656)
 *      or separate (ITU-601)
 *
 *  Input:
 *      SYNC_EMBEDDED    - The sync signal is embedded in the data.
 *      SYNC_INDEPENDENT - The sync signal is independent from the data.
 */
void setVideoDecoderSyncType(
    video_sync_type_t syncType
);

/*
 *  setVideoDecoderDataSize
 *      This function sets the video data bus (8-bit or 16-bit)
 *
 *  Input:
 *      dataSize    - Data bus where the data is transferred. Either 8_BIT
 *                    or 16_BIT
 */
void setVideoDecoderDataSize(
    video_data_size_t dataSize
);

/**************************************************************************** 
    Function prototypes for controlling VGX ZV ports.
 ****************************************************************************/

/* Get the total available ZV Port */
unsigned char getTotalZVPort();

/* Set the Current ZV Port Index (either 0 or 1) */
char setCurrentZVPort(
    unsigned char ucZVDevice
);

/* Get the current ZV Port Index */
unsigned char getCurrentZVPort();

/* isCaptureInterlace
 *      This unction checks whether the capture video source is interlaced or progressive
 *
 * Output:
 *      0   - Progressive
 *      1   - Interlace
 */
unsigned char isCaptureInterlace();

/* getCurrentBufferCapture
 *      This unction gets the current buffer captured
 *
 * Output:
 *      0   - Buffer 0
 *      1   - Buffer 1
 */
unsigned char getCurrentBufferCapture();

/* getCurrentBufferFieldStatus
 *      This function gets the current buffer Field Status
 *
 * Output:
 *      0   - Even field
 *      1   - Odd Field
 */
unsigned char getCurrentBufferFieldStatus();

/*
 *  initCapture
 *      This function initialize the capture control.
 *
 *  Input:
 *      captureParam    - Capture Parameter
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */ 
char initZVPort(
    capture_t *pCaptureParam
);

/* Set the capture size */
void setZVPortCaptureSize(
    unsigned short sourceHeight,            /* Number of source line to capture */ 
    unsigned short sourceWidth              /* Number of source pixel to capture */
);

/* Set the capture data size */
void setZVPortDataSize(            
    video_data_size_t videoSize,            /* Capture Size, 0: 16-bit, 1: 8-bit */
    unsigned char lower8Bit                 /* Enable the upper GPIO pins [8:15] (=0)
                                               or lower GPIO pins[0:7] (=1)
                                               This parameter only valid in 8 bit mode.
                                               16-bit mode always enable all the lower
                                               and upper GPIO bits */
);

/*
 *  setZVPortCaptureClip
 *      This function sets the clipping on the video source.
 *
 *  Input:
 *      xClip       - Number of pixel to skip after HRef
 *      yEvenClip   - Number of lines on the even field to skip after VSync
 *      yOddclip    - Number of lines on the odd field to skip after VSync
 */
void setZVPortCaptureClip(
    uint32_t xClip,
    uint32_t yEvenClip,
    uint32_t yOddclip
);

/* Enable the ZV Port */
void setZVPortCapture(
    zv_ctrl_t zvCtrl                        /* Flag to enable(=1) or disable(=0) the capturing */
);

/*
 *  getZVPortCaptureBuffer
 *      This function gets the capture buffer
 *
 *  Input:
 *      bufferIndex             - The index of the capture buffer address which status 
 *                                to be retrieved (0 = buffer 0, 1 = buffer 1)
 *      pCaptureBuffer0Address  - Pointer to a variable to store the capture buffer address.
 *      pBufferOffset           - Pointer to a variable to store the number of 128-bit aligned 
 *                                bytes per line of the capture buffer
 *
 *  Return:
 *      Status of the captured buffer
 *          0   - Flip not pending
 *          1   - Flip pending
 */
unsigned char getZVPortCaptureBuffer(
    unsigned char bufferIndex,
    uint32_t *pCaptureBufferAddress,
    unsigned short *pBufferOffset
);

/*
 *  setZVPortCaptureOffset
 *      This function sets the current capture buffer offset.
 *
 *  Input:
 *      bufferOffset            - Number of 128-bit aligned bytes per line of the capture buffer
 *                                Set it to 0 when there is no modification necessary.
 */
void setZVPortCaptureOffset(
    unsigned short bufferOffset
);

/*
 *  setZVPortCaptureBuffer
 *      This function sets the current capture buffer.
 *
 *  Input:
 *      bufferIndex             - The index of the capture buffer address which to be 
 *                                set (0 = buffer 0, 1 = buffer 1)
 *      captureBufferAddress    - The capture buffer address
 */
void setZVPortCaptureBuffer(
    unsigned char bufferIndex,
    uint32_t captureBufferAddress
);

/* Enable the Half Shrink feature */
void enableZVPortHalfShrink(
    unsigned char  halfHorizontalShrink,    /* Horizontal 1/2 shink, 0: Disable, 1: Enable */
    unsigned char  halfVerticalShrink       /* Vertical 1/2 shink, 0: Disable, 1: Enable */
);

/* Set the ZV Port constants */
void setZVPortConstants(
    unsigned char  yConstant,               /* Y Adjustment */
    unsigned char  redConstant,             /* Red Conversion constant */
    unsigned char  greenConstant,           /* Green Conversion constant */
    unsigned char  blueConstant             /* Blue Conversion constant */
);

/*
 *  enableZVPortHorzAverage
 *      This function enables/disables the horizontal averaging feature.
 *
 *  Input:
 *      enable  - Flag to enable or disable the horizontal averaing feature.
 */ 
void enableZVPortHorzAverage(
    uint32_t enable
);

void setZVPortCapturePanelOutput(
    uint32_t enablePanelCapture
);

/*
 *  startZVPortCapture
 *      This function starts the capture.
 */
void startZVPortCapture();

/*
 *  stopZVPortCapture
 *      This function stops the capture.
 */
void stopZVPortCapture();

/* 
 *  checkZVPortIntrStatus
 *      This function is used to check the ZVPort Interrupt status for polling purpose
 */
uint32_t checkZVPortIntrStatus(
    zv_port_ctrl_t zvPortControl
);

/* 
 *  clearZVPortIntrStatus
 *      This function is clears ZVPort Interrupt status.
 */
void clearZVPortIntrStatus(
    zv_port_ctrl_t zvPortControl
);

/* 
 * hookZVPortInterrupt
 *      This function registers ZV Port Interrupt function 
 */
unsigned short hookZVPortInterrupt(
    zv_port_ctrl_t zvPortControl,
    void (*handler)(void)
);

/* 
 * unhookZVPortInterrupt
 *      This function unregister a registered panel VSync interrupt handler 
 */
unsigned short unhookZVPortInterrupt(
    zv_port_ctrl_t zvPortControl,
    void (*handler)(void)
);

#endif  /* _CAPTURE_H_ */
