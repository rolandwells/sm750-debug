/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  RegDMA.h --- Voyager GX SDK 
*  This file contains the definitions for the DMA registers.
* 
*******************************************************************/

#if 0
/* This DMA is not supported in the SM750. */
#define DMA_0_SDRAM                                     0x0D0000
#define DMA_0_SDRAM_EXT                                 27:27
#define DMA_0_SDRAM_EXT_LOCAL                           0
#define DMA_0_SDRAM_EXT_EXTERNAL                        1
#define DMA_0_SDRAM_CS                                  26:26
#define DMA_0_SDRAM_CS_0                                0
#define DMA_0_SDRAM_CS_1                                1
#define DMA_0_SDRAM_ADDRESS                             25:0

#define DMA_0_SRAM                                      0x0D0004
#define DMA_0_SRAM_ADDRESS                              15:0

#define DMA_0_SIZE_CONTROL                              0x0D0008
#define DMA_0_SIZE_CONTROL_STATUS                       31:31
#define DMA_0_SIZE_CONTROL_STATUS_IDLE                  0
#define DMA_0_SIZE_CONTROL_STATUS_ACTIVE                1
#define DMA_0_SIZE_CONTROL_DIR                          30:30
#define DMA_0_SIZE_CONTROL_DIR_TO_SRAM                  0
#define DMA_0_SIZE_CONTROL_DIR_FROM_SRAM                1
#define DMA_0_SIZE_CONTROL_SIZE                         15:0
#endif

#define DMA_1_SOURCE                                    0x0D0010
#define DMA_1_SOURCE_ADDRESS_EXT                        27:27
#define DMA_1_SOURCE_ADDRESS_EXT_LOCAL                  0
#define DMA_1_SOURCE_ADDRESS_EXT_EXTERNAL               1
#define DMA_1_SOURCE_ADDRESS_CS                         26:26
#define DMA_1_SOURCE_ADDRESS_CS_0                       0
#define DMA_1_SOURCE_ADDRESS_CS_1                       1
#define DMA_1_SOURCE_ADDRESS                            25:0

#define DMA_1_DESTINATION                               0x0D0014
#define DMA_1_DESTINATION_ADDRESS_EXT                   27:27
#define DMA_1_DESTINATION_ADDRESS_EXT_LOCAL             0
#define DMA_1_DESTINATION_ADDRESS_EXT_EXTERNAL          1
#define DMA_1_DESTINATION_ADDRESS_CS                    26:26
#define DMA_1_DESTINATION_ADDRESS_CS_0                  0
#define DMA_1_DESTINATION_ADDRESS_CS_1                  1
#define DMA_1_DESTINATION_ADDRESS                       25:0

#define DMA_1_SIZE_CONTROL                              0x0D0018
#define DMA_1_SIZE_CONTROL_STATUS                       31:31
#define DMA_1_SIZE_CONTROL_STATUS_IDLE                  0
#define DMA_1_SIZE_CONTROL_STATUS_ACTIVE                1
#define DMA_1_SIZE_CONTROL_SIZE                         23:0

#define DMA_ABORT_INTERRUPT                             0x0D0020
#define DMA_ABORT_INTERRUPT_ABORT_1                     5:5
#define DMA_ABORT_INTERRUPT_ABORT_1_ENABLE              0
#define DMA_ABORT_INTERRUPT_ABORT_1_ABORT               1
#define DMA_ABORT_INTERRUPT_ABORT_0                     4:4
#define DMA_ABORT_INTERRUPT_ABORT_0_ENABLE              0
#define DMA_ABORT_INTERRUPT_ABORT_0_ABORT               1
#define DMA_ABORT_INTERRUPT_INT_1                       1:1
#define DMA_ABORT_INTERRUPT_INT_1_CLEAR                 0
#define DMA_ABORT_INTERRUPT_INT_1_FINISHED              1
#define DMA_ABORT_INTERRUPT_INT_0                       0:0
#define DMA_ABORT_INTERRUPT_INT_0_CLEAR                 0
#define DMA_ABORT_INTERRUPT_INT_0_FINISHED              1
