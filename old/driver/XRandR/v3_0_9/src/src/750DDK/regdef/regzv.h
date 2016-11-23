/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  RegZV.h --- Voyager GX SDK 
*  This file contains the definitions for the ZV registers.
* 
*******************************************************************/

/* ZV0 */

#define ZV0_CAPTURE_CTRL                                0x090000
#define ZV0_CAPTURE_CTRL_FIELD_INPUT                    27:27
#define ZV0_CAPTURE_CTRL_FIELD_INPUT_EVEN_FIELD         0
#define ZV0_CAPTURE_CTRL_FIELD_INPUT_ODD_FIELD          1
#define ZV0_CAPTURE_CTRL_SCAN                           26:26
#define ZV0_CAPTURE_CTRL_SCAN_PROGRESSIVE               0
#define ZV0_CAPTURE_CTRL_SCAN_INTERLACE                 1
#define ZV0_CAPTURE_CTRL_CURRENT_BUFFER                 25:25
#define ZV0_CAPTURE_CTRL_CURRENT_BUFFER_0               0
#define ZV0_CAPTURE_CTRL_CURRENT_BUFFER_1               1
#define ZV0_CAPTURE_CTRL_VERTICAL_SYNC                  24:24
#define ZV0_CAPTURE_CTRL_VERTICAL_SYNC_INACTIVE         0
#define ZV0_CAPTURE_CTRL_VERTICAL_SYNC_ACTIVE           1
#define ZV0_CAPTURE_CTRL_ADJ                            19:19
#define ZV0_CAPTURE_CTRL_ADJ_NORMAL                     0
#define ZV0_CAPTURE_CTRL_ADJ_DELAY                      1
#define ZV0_CAPTURE_CTRL_HA                             18:18
#define ZV0_CAPTURE_CTRL_HA_DISABLE                     0
#define ZV0_CAPTURE_CTRL_HA_ENABLE                      1
#define ZV0_CAPTURE_CTRL_VSK                            17:17
#define ZV0_CAPTURE_CTRL_VSK_DISABLE                    0
#define ZV0_CAPTURE_CTRL_VSK_ENABLE                     1
#define ZV0_CAPTURE_CTRL_HSK                            16:16
#define ZV0_CAPTURE_CTRL_HSK_DISABLE                    0
#define ZV0_CAPTURE_CTRL_HSK_ENABLE                     1
#define ZV0_CAPTURE_CTRL_FD                             15:15
#define ZV0_CAPTURE_CTRL_FD_RISING                      0
#define ZV0_CAPTURE_CTRL_FD_FALLING                     1
#define ZV0_CAPTURE_CTRL_VP                             14:14
#define ZV0_CAPTURE_CTRL_VP_HIGH                        0
#define ZV0_CAPTURE_CTRL_VP_LOW                         1
#define ZV0_CAPTURE_CTRL_HP                             13:13
#define ZV0_CAPTURE_CTRL_HP_HIGH                        0
#define ZV0_CAPTURE_CTRL_HP_LOW                         1
#define ZV0_CAPTURE_CTRL_CP                             12:12
#define ZV0_CAPTURE_CTRL_CP_HIGH                        0
#define ZV0_CAPTURE_CTRL_CP_LOW                         1
#define ZV0_CAPTURE_CTRL_UVS                            11:11
#define ZV0_CAPTURE_CTRL_UVS_DISABLE                    0
#define ZV0_CAPTURE_CTRL_UVS_ENABLE                     1
#define ZV0_CAPTURE_CTRL_BS                             10:10
#define ZV0_CAPTURE_CTRL_BS_DISABLE                     0
#define ZV0_CAPTURE_CTRL_BS_ENABLE                      1
#define ZV0_CAPTURE_CTRL_CS                             9:9
#define ZV0_CAPTURE_CTRL_CS_16                          0
#define ZV0_CAPTURE_CTRL_CS_8                           1
#define ZV0_CAPTURE_CTRL_CF                             8:8
#define ZV0_CAPTURE_CTRL_CF_YUV                         0
#define ZV0_CAPTURE_CTRL_CF_RGB                         1
#define ZV0_CAPTURE_CTRL_FS                             7:7
#define ZV0_CAPTURE_CTRL_FS_DISABLE                     0
#define ZV0_CAPTURE_CTRL_FS_ENABLE                      1
#define ZV0_CAPTURE_CTRL_WEAVE                          6:6
#define ZV0_CAPTURE_CTRL_WEAVE_DISABLE                  0
#define ZV0_CAPTURE_CTRL_WEAVE_ENABLE                   1
#define ZV0_CAPTURE_CTRL_BOB                            5:5
#define ZV0_CAPTURE_CTRL_BOB_DISABLE                    0
#define ZV0_CAPTURE_CTRL_BOB_ENABLE                     1
#define ZV0_CAPTURE_CTRL_DB                             4:4
#define ZV0_CAPTURE_CTRL_DB_DISABLE                     0
#define ZV0_CAPTURE_CTRL_DB_ENABLE                      1
#define ZV0_CAPTURE_CTRL_CC                             3:3
#define ZV0_CAPTURE_CTRL_CC_CONTINUE                    0
#define ZV0_CAPTURE_CTRL_CC_CONDITION                   1
#define ZV0_CAPTURE_CTRL_RGB                            2:2
#define ZV0_CAPTURE_CTRL_RGB_DISABLE                    0
#define ZV0_CAPTURE_CTRL_RGB_ENABLE                     1
#define ZV0_CAPTURE_CTRL_656                            1:1
#define ZV0_CAPTURE_CTRL_656_DISABLE                    0
#define ZV0_CAPTURE_CTRL_656_ENABLE                     1
#define ZV0_CAPTURE_CTRL_CAP                            0:0
#define ZV0_CAPTURE_CTRL_CAP_DISABLE                    0
#define ZV0_CAPTURE_CTRL_CAP_ENABLE                     1

#define ZV0_CAPTURE_CLIP                                0x090004
#define ZV0_CAPTURE_CLIP_YCLIP_EVEN_FIELD                25:16
#define ZV0_CAPTURE_CLIP_YCLIP                          25:16
#define ZV0_CAPTURE_CLIP_XCLIP                          9:0

#define ZV0_CAPTURE_SIZE                                0x090008
#define ZV0_CAPTURE_SIZE_HEIGHT                         26:16
#define ZV0_CAPTURE_SIZE_WIDTH                          10:0   

#define ZV0_CAPTURE_BUF0_ADDRESS                        0x09000C
#define ZV0_CAPTURE_BUF0_ADDRESS_STATUS                 31:31
#define ZV0_CAPTURE_BUF0_ADDRESS_STATUS_CURRENT         0
#define ZV0_CAPTURE_BUF0_ADDRESS_STATUS_PENDING         1
#define ZV0_CAPTURE_BUF0_ADDRESS_EXT                    27:27
#define ZV0_CAPTURE_BUF0_ADDRESS_EXT_LOCAL              0
#define ZV0_CAPTURE_BUF0_ADDRESS_EXT_EXTERNAL           1
#define ZV0_CAPTURE_BUF0_ADDRESS_CS                     26:26
#define ZV0_CAPTURE_BUF0_ADDRESS_CS_0                   0
#define ZV0_CAPTURE_BUF0_ADDRESS_CS_1                   1
#define ZV0_CAPTURE_BUF0_ADDRESS_ADDRESS                25:0

#define ZV0_CAPTURE_BUF1_ADDRESS                        0x090010
#define ZV0_CAPTURE_BUF1_ADDRESS_STATUS                 31:31
#define ZV0_CAPTURE_BUF1_ADDRESS_STATUS_CURRENT         0
#define ZV0_CAPTURE_BUF1_ADDRESS_STATUS_PENDING         1
#define ZV0_CAPTURE_BUF1_ADDRESS_EXT                    27:27
#define ZV0_CAPTURE_BUF1_ADDRESS_EXT_LOCAL              0
#define ZV0_CAPTURE_BUF1_ADDRESS_EXT_EXTERNAL           1
#define ZV0_CAPTURE_BUF1_ADDRESS_CS                     26:26
#define ZV0_CAPTURE_BUF1_ADDRESS_CS_0                   0
#define ZV0_CAPTURE_BUF1_ADDRESS_CS_1                   1
#define ZV0_CAPTURE_BUF1_ADDRESS_ADDRESS                25:0

#define ZV0_CAPTURE_BUF_OFFSET                          0x090014
#ifndef VALIDATION_CHIP
    #define ZV0_CAPTURE_BUF_OFFSET_YCLIP_ODD_FIELD      25:16
#endif
#define ZV0_CAPTURE_BUF_OFFSET_OFFSET                   15:0

#define ZV0_CAPTURE_FIFO_CTRL                           0x090018
#define ZV0_CAPTURE_FIFO_CTRL_FIFO                      2:0
#define ZV0_CAPTURE_FIFO_CTRL_FIFO_0                    0
#define ZV0_CAPTURE_FIFO_CTRL_FIFO_1                    1
#define ZV0_CAPTURE_FIFO_CTRL_FIFO_2                    2
#define ZV0_CAPTURE_FIFO_CTRL_FIFO_3                    3
#define ZV0_CAPTURE_FIFO_CTRL_FIFO_4                    4
#define ZV0_CAPTURE_FIFO_CTRL_FIFO_5                    5
#define ZV0_CAPTURE_FIFO_CTRL_FIFO_6                    6
#define ZV0_CAPTURE_FIFO_CTRL_FIFO_7                    7

#define ZV0_CAPTURE_YRGB_CONST                          0x09001C
#define ZV0_CAPTURE_YRGB_CONST_Y                        31:24
#define ZV0_CAPTURE_YRGB_CONST_R                        23:16
#define ZV0_CAPTURE_YRGB_CONST_G                        15:8
#define ZV0_CAPTURE_YRGB_CONST_B                        7:0

#define ZV0_CAPTURE_LINE_COMP                           0x090020
#define ZV0_CAPTURE_LINE_COMP_LC                        10:0

/* ZV1 */

#define ZV1_CAPTURE_CTRL                                0x098000
#define ZV1_CAPTURE_CTRL_FIELD_INPUT                    27:27
#define ZV1_CAPTURE_CTRL_FIELD_INPUT_EVEN_FIELD         0
#define ZV1_CAPTURE_CTRL_FIELD_INPUT_ODD_FIELD          0
#define ZV1_CAPTURE_CTRL_SCAN                           26:26
#define ZV1_CAPTURE_CTRL_SCAN_PROGRESSIVE               0
#define ZV1_CAPTURE_CTRL_SCAN_INTERLACE                 1
#define ZV1_CAPTURE_CTRL_CURRENT_BUFFER                 25:25
#define ZV1_CAPTURE_CTRL_CURRENT_BUFFER_0               0
#define ZV1_CAPTURE_CTRL_CURRENT_BUFFER_1               1
#define ZV1_CAPTURE_CTRL_VERTICAL_SYNC                  24:24
#define ZV1_CAPTURE_CTRL_VERTICAL_SYNC_INACTIVE         0
#define ZV1_CAPTURE_CTRL_VERTICAL_SYNC_ACTIVE           1
#define ZV1_CAPTURE_CTRL_PANEL                          20:20
#define ZV1_CAPTURE_CTRL_PANEL_DISABLE                  0
#define ZV1_CAPTURE_CTRL_PANEL_ENABLE                   1
#define ZV1_CAPTURE_CTRL_ADJ                            19:19
#define ZV1_CAPTURE_CTRL_ADJ_NORMAL                     0
#define ZV1_CAPTURE_CTRL_ADJ_DELAY                      1
#define ZV1_CAPTURE_CTRL_HA                             18:18
#define ZV1_CAPTURE_CTRL_HA_DISABLE                     0
#define ZV1_CAPTURE_CTRL_HA_ENABLE                      1
#define ZV1_CAPTURE_CTRL_VSK                            17:17
#define ZV1_CAPTURE_CTRL_VSK_DISABLE                    0
#define ZV1_CAPTURE_CTRL_VSK_ENABLE                     1
#define ZV1_CAPTURE_CTRL_HSK                            16:16
#define ZV1_CAPTURE_CTRL_HSK_DISABLE                    0
#define ZV1_CAPTURE_CTRL_HSK_ENABLE                     1
#define ZV1_CAPTURE_CTRL_FD                             15:15
#define ZV1_CAPTURE_CTRL_FD_RISING                      0
#define ZV1_CAPTURE_CTRL_FD_FALLING                     1
#define ZV1_CAPTURE_CTRL_VP                             14:14
#define ZV1_CAPTURE_CTRL_VP_HIGH                        0
#define ZV1_CAPTURE_CTRL_VP_LOW                         1
#define ZV1_CAPTURE_CTRL_HP                             13:13
#define ZV1_CAPTURE_CTRL_HP_HIGH                        0
#define ZV1_CAPTURE_CTRL_HP_LOW                         1
#define ZV1_CAPTURE_CTRL_CP                             12:12
#define ZV1_CAPTURE_CTRL_CP_HIGH                        0
#define ZV1_CAPTURE_CTRL_CP_LOW                         1
#define ZV1_CAPTURE_CTRL_UVS                            11:11
#define ZV1_CAPTURE_CTRL_UVS_DISABLE                    0
#define ZV1_CAPTURE_CTRL_UVS_ENABLE                     1
#define ZV1_CAPTURE_CTRL_BS                             10:10
#define ZV1_CAPTURE_CTRL_BS_DISABLE                     0
#define ZV1_CAPTURE_CTRL_BS_ENABLE                      1
#define ZV1_CAPTURE_CTRL_CS                             9:9
#define ZV1_CAPTURE_CTRL_CS_16                          0
#define ZV1_CAPTURE_CTRL_CS_8                           1
#define ZV1_CAPTURE_CTRL_CF                             8:8
#define ZV1_CAPTURE_CTRL_CF_YUV                         0
#define ZV1_CAPTURE_CTRL_CF_RGB                         1
#define ZV1_CAPTURE_CTRL_FS                             7:7
#define ZV1_CAPTURE_CTRL_FS_DISABLE                     0
#define ZV1_CAPTURE_CTRL_FS_ENABLE                      1
#define ZV1_CAPTURE_CTRL_WEAVE                          6:6
#define ZV1_CAPTURE_CTRL_WEAVE_DISABLE                  0
#define ZV1_CAPTURE_CTRL_WEAVE_ENABLE                   1
#define ZV1_CAPTURE_CTRL_BOB                            5:5
#define ZV1_CAPTURE_CTRL_BOB_DISABLE                    0
#define ZV1_CAPTURE_CTRL_BOB_ENABLE                     1
#define ZV1_CAPTURE_CTRL_DB                             4:4
#define ZV1_CAPTURE_CTRL_DB_DISABLE                     0
#define ZV1_CAPTURE_CTRL_DB_ENABLE                      1
#define ZV1_CAPTURE_CTRL_CC                             3:3
#define ZV1_CAPTURE_CTRL_CC_CONTINUE                    0
#define ZV1_CAPTURE_CTRL_CC_CONDITION                   1
#define ZV1_CAPTURE_CTRL_RGB                            2:2
#define ZV1_CAPTURE_CTRL_RGB_DISABLE                    0
#define ZV1_CAPTURE_CTRL_RGB_ENABLE                     1
#define ZV1_CAPTURE_CTRL_656                            1:1
#define ZV1_CAPTURE_CTRL_656_DISABLE                    0
#define ZV1_CAPTURE_CTRL_656_ENABLE                     1
#define ZV1_CAPTURE_CTRL_CAP                            0:0
#define ZV1_CAPTURE_CTRL_CAP_DISABLE                    0
#define ZV1_CAPTURE_CTRL_CAP_ENABLE                     1

#define ZV1_CAPTURE_CLIP                                0x098004
#define ZV1_CAPTURE_CLIP_YCLIP                          25:16
#define ZV1_CAPTURE_CLIP_XCLIP                          9:0

#define ZV1_CAPTURE_SIZE                                0x098008
#define ZV1_CAPTURE_SIZE_HEIGHT                         26:16   
#define ZV1_CAPTURE_SIZE_WIDTH                          10:0   

#define ZV1_CAPTURE_BUF0_ADDRESS                        0x09800C
#define ZV1_CAPTURE_BUF0_ADDRESS_STATUS                 31:31
#define ZV1_CAPTURE_BUF0_ADDRESS_STATUS_CURRENT         0
#define ZV1_CAPTURE_BUF0_ADDRESS_STATUS_PENDING         1
#define ZV1_CAPTURE_BUF0_ADDRESS_EXT                    27:27
#define ZV1_CAPTURE_BUF0_ADDRESS_EXT_LOCAL              0
#define ZV1_CAPTURE_BUF0_ADDRESS_EXT_EXTERNAL           1
#define ZV1_CAPTURE_BUF0_ADDRESS_CS                     26:26
#define ZV1_CAPTURE_BUF0_ADDRESS_CS_0                   0
#define ZV1_CAPTURE_BUF0_ADDRESS_CS_1                   1
#define ZV1_CAPTURE_BUF0_ADDRESS_ADDRESS                25:0

#define ZV1_CAPTURE_BUF1_ADDRESS                        0x098010
#define ZV1_CAPTURE_BUF1_ADDRESS_STATUS                 31:31
#define ZV1_CAPTURE_BUF1_ADDRESS_STATUS_CURRENT         0
#define ZV1_CAPTURE_BUF1_ADDRESS_STATUS_PENDING         1
#define ZV1_CAPTURE_BUF1_ADDRESS_EXT                    27:27
#define ZV1_CAPTURE_BUF1_ADDRESS_EXT_LOCAL              0
#define ZV1_CAPTURE_BUF1_ADDRESS_EXT_EXTERNAL           1
#define ZV1_CAPTURE_BUF1_ADDRESS_CS                     26:26
#define ZV1_CAPTURE_BUF1_ADDRESS_CS_0                   0
#define ZV1_CAPTURE_BUF1_ADDRESS_CS_1                   1
#define ZV1_CAPTURE_BUF1_ADDRESS_ADDRESS                25:0

#define ZV1_CAPTURE_BUF_OFFSET                          0x098014
#define ZV1_CAPTURE_BUF_OFFSET_OFFSET                   15:0

#define ZV1_CAPTURE_FIFO_CTRL                           0x098018
#define ZV1_CAPTURE_FIFO_CTRL_FIFO                      2:0
#define ZV1_CAPTURE_FIFO_CTRL_FIFO_0                    0
#define ZV1_CAPTURE_FIFO_CTRL_FIFO_1                    1
#define ZV1_CAPTURE_FIFO_CTRL_FIFO_2                    2
#define ZV1_CAPTURE_FIFO_CTRL_FIFO_3                    3
#define ZV1_CAPTURE_FIFO_CTRL_FIFO_4                    4
#define ZV1_CAPTURE_FIFO_CTRL_FIFO_5                    5
#define ZV1_CAPTURE_FIFO_CTRL_FIFO_6                    6
#define ZV1_CAPTURE_FIFO_CTRL_FIFO_7                    7

#define ZV1_CAPTURE_YRGB_CONST                          0x09801C
#define ZV1_CAPTURE_YRGB_CONST_Y                        31:24
#define ZV1_CAPTURE_YRGB_CONST_R                        23:16
#define ZV1_CAPTURE_YRGB_CONST_G                        15:8
#define ZV1_CAPTURE_YRGB_CONST_B                        7:0


