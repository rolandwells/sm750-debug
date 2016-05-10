/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  RegSSP.h --- Voyager GX SDK 
*  This file contains the definitions for the Synchroneous Serial Port registers.
* 
*******************************************************************/
/* SSP 0 */

#define SSP_0_CONTROL_0                                 0x020000
#define SSP_0_CONTROL_0_CLOCK_RATE                      15:8
#define SSP_0_CONTROL_0_SCLKOUT_PHASE                   7:7
#define SSP_0_CONTROL_0_SCLKOUT_PHASE_0                 0
#define SSP_0_CONTROL_0_SCLKOUT_PHASE_1                 1
#define SSP_0_CONTROL_0_SCLKOUT_POLARITY                6:6
#define SSP_0_CONTROL_0_SCLKOUT_POLARITY_RISING         0
#define SSP_0_CONTROL_0_SCLKOUT_POLARITY_FALLING        1
#define SSP_0_CONTROL_0_FRAME_FORMAT                    5:4
#define SSP_0_CONTROL_0_FRAME_FORMAT_MOTOROLA           0
#define SSP_0_CONTROL_0_FRAME_FORMAT_TI                 1
#define SSP_0_CONTROL_0_FRAME_FORMAT_NATIONAL           2
#define SSP_0_CONTROL_0_DATA_SIZE                       3:0
#define SSP_0_CONTROL_0_DATA_SIZE_4                     3
#define SSP_0_CONTROL_0_DATA_SIZE_5                     4
#define SSP_0_CONTROL_0_DATA_SIZE_6                     5
#define SSP_0_CONTROL_0_DATA_SIZE_7                     6
#define SSP_0_CONTROL_0_DATA_SIZE_8                     7
#define SSP_0_CONTROL_0_DATA_SIZE_9                     8
#define SSP_0_CONTROL_0_DATA_SIZE_10                    9
#define SSP_0_CONTROL_0_DATA_SIZE_11                    10
#define SSP_0_CONTROL_0_DATA_SIZE_12                    11
#define SSP_0_CONTROL_0_DATA_SIZE_13                    12
#define SSP_0_CONTROL_0_DATA_SIZE_14                    13
#define SSP_0_CONTROL_0_DATA_SIZE_15                    14
#define SSP_0_CONTROL_0_DATA_SIZE_16                    15

#define SSP_0_CONTROL_1                                 0x020004
#define SSP_0_CONTROL_1_SLAVE_OUTPUT                    6:6
#define SSP_0_CONTROL_1_SLAVE_OUTPUT_ENABLE             0
#define SSP_0_CONTROL_1_SLAVE_OUTPUT_DISABLE            1
#define SSP_0_CONTROL_1_MODE_SELECT                     5:5
#define SSP_0_CONTROL_1_MODE_SELECT_MASTER              0
#define SSP_0_CONTROL_1_MODE_SELECT_SLAVE               1
#define SSP_0_CONTROL_1_STATUS                          4:4
#define SSP_0_CONTROL_1_STATUS_DISABLE                  0
#define SSP_0_CONTROL_1_STATUS_ENABLE                   1
#define SSP_0_CONTROL_1_LOOP_BACK                       3:3
#define SSP_0_CONTROL_1_LOOP_BACK_DISABLE               0
#define SSP_0_CONTROL_1_LOOP_BACK_ENABLE                1
#define SSP_0_CONTROL_1_OVERRUN_INTERRUPT               2:2
#define SSP_0_CONTROL_1_OVERRUN_INTERRUPT_DISABLE       0
#define SSP_0_CONTROL_1_OVERRUN_INTERRUPT_ENABLE        1
#define SSP_0_CONTROL_1_TRANSMIT_INTERRUPT              1:1
#define SSP_0_CONTROL_1_TRANSMIT_INTERRUPT_DISABLE      0
#define SSP_0_CONTROL_1_TRANSMIT_INTERRUPT_ENABLE       1
#define SSP_0_CONTROL_1_RECEIVE_INTERRUPT               0:0
#define SSP_0_CONTROL_1_RECEIVE_INTERRUPT_DISABLE       0
#define SSP_0_CONTROL_1_RECEIVE_INTERRUPT_ENABLE        1

#define SSP_0_DATA                                      0x020008
#define SSP_0_DATA_DATA                                 15:0

#define SSP_0_STATUS                                    0x02000C
#define SSP_0_STATUS_STATUS                             4:4
#define SSP_0_STATUS_STATUS_IDLE                        0
#define SSP_0_STATUS_STATUS_BUSY                        1
#define SSP_0_STATUS_RECEIVE_FIFO                       3:2
#define SSP_0_STATUS_RECEIVE_FIFO_EMPTY                 0
#define SSP_0_STATUS_RECEIVE_FIFO_NOT_EMPTY             1
#define SSP_0_STATUS_RECEIVE_FIFO_FULL                  3
#define SSP_0_STATUS_TRANSMIT_FIFO                      1:0
#define SSP_0_STATUS_TRANSMIT_FIFO_FULL                 0
#define SSP_0_STATUS_TRANSMIT_FIFO_NOT_FULL             2
#define SSP_0_STATUS_TRANSMIT_FIFO_EMPTY                3

#define SSP_0_CLOCK_PRESCALE                            0x020010
#define SSP_0_CLOCK_PRESCALE_DIVISOR                    7:0

#define SSP_0_INTERRUPT_STATUS                          0x020014
#define SSP_0_INTERRUPT_STATUS_OVERRUN                  2:2
#define SSP_0_INTERRUPT_STATUS_OVERRUN_NOT_ACTIVE       0
#define SSP_0_INTERRUPT_STATUS_OVERRUN_ACTIVE           1
#define SSP_0_INTERRUPT_STATUS_OVERRUN_CLEAR            1
#define SSP_0_INTERRUPT_STATUS_TRANSMIT                 1:1
#define SSP_0_INTERRUPT_STATUS_TRANSMIT_NOT_ACTIVE      0
#define SSP_0_INTERRUPT_STATUS_TRANSMIT_ACTIVE          1
#define SSP_0_INTERRUPT_STATUS_RECEIVE                  0:0
#define SSP_0_INTERRUPT_STATUS_RECEIVE_NOT_ACTIVE       0
#define SSP_0_INTERRUPT_STATUS_RECEIVE_ACTIVE           1

/* SSP 1 */

#define SSP_1_CONTROL_0                                 0x020100
#define SSP_1_CONTROL_0_CLOCK_RATE                      15:8
#define SSP_1_CONTROL_0_SCLKOUT_PHASE                   7:7
#define SSP_1_CONTROL_0_SCLKOUT_PHASE_0                 0
#define SSP_1_CONTROL_0_SCLKOUT_PHASE_1                 1
#define SSP_1_CONTROL_0_SCLKOUT_POLARITY                6:6
#define SSP_1_CONTROL_0_SCLKOUT_POLARITY_RISING         0
#define SSP_1_CONTROL_0_SCLKOUT_POLARITY_FALLING        1
#define SSP_1_CONTROL_0_FRAME_FORMAT                    5:4
#define SSP_1_CONTROL_0_FRAME_FORMAT_MOTOROLA           0
#define SSP_1_CONTROL_0_FRAME_FORMAT_TI                 1
#define SSP_1_CONTROL_0_FRAME_FORMAT_NATIONAL           2
#define SSP_1_CONTROL_0_DATA_SIZE                       3:0
#define SSP_1_CONTROL_0_DATA_SIZE_4                     3
#define SSP_1_CONTROL_0_DATA_SIZE_5                     4
#define SSP_1_CONTROL_0_DATA_SIZE_6                     5
#define SSP_1_CONTROL_0_DATA_SIZE_7                     6
#define SSP_1_CONTROL_0_DATA_SIZE_8                     7
#define SSP_1_CONTROL_0_DATA_SIZE_9                     8
#define SSP_1_CONTROL_0_DATA_SIZE_10                    9
#define SSP_1_CONTROL_0_DATA_SIZE_11                    10
#define SSP_1_CONTROL_0_DATA_SIZE_12                    11
#define SSP_1_CONTROL_0_DATA_SIZE_13                    12
#define SSP_1_CONTROL_0_DATA_SIZE_14                    13
#define SSP_1_CONTROL_0_DATA_SIZE_15                    14
#define SSP_1_CONTROL_0_DATA_SIZE_16                    15

#define SSP_1_CONTROL_1                                 0x020104
#define SSP_1_CONTROL_1_SLAVE_OUTPUT                    6:6
#define SSP_1_CONTROL_1_SLAVE_OUTPUT_ENABLE             0
#define SSP_1_CONTROL_1_SLAVE_OUTPUT_DISABLE            1
#define SSP_1_CONTROL_1_MODE_SELECT                     5:5
#define SSP_1_CONTROL_1_MODE_SELECT_MASTER              0
#define SSP_1_CONTROL_1_MODE_SELECT_SLAVE               1
#define SSP_1_CONTROL_1_STATUS                          4:4
#define SSP_1_CONTROL_1_STATUS_DISABLE                  0
#define SSP_1_CONTROL_1_STATUS_ENABLE                   1
#define SSP_1_CONTROL_1_LOOP_BACK                       3:3
#define SSP_1_CONTROL_1_LOOP_BACK_DISABLE               0
#define SSP_1_CONTROL_1_LOOP_BACK_ENABLE                1
#define SSP_1_CONTROL_1_OVERRUN_INTERRUPT               2:2
#define SSP_1_CONTROL_1_OVERRUN_INTERRUPT_DISABLE       0
#define SSP_1_CONTROL_1_OVERRUN_INTERRUPT_ENABLE        1
#define SSP_1_CONTROL_1_TRANSMIT_INTERRUPT              1:1
#define SSP_1_CONTROL_1_TRANSMIT_INTERRUPT_DISABLE      0
#define SSP_1_CONTROL_1_TRANSMIT_INTERRUPT_ENABLE       1
#define SSP_1_CONTROL_1_RECEIVE_INTERRUPT               0:0
#define SSP_1_CONTROL_1_RECEIVE_INTERRUPT_DISABLE       0
#define SSP_1_CONTROL_1_RECEIVE_INTERRUPT_ENABLE        1

#define SSP_1_DATA                                      0x020108
#define SSP_1_DATA_DATA                                 15:0

#define SSP_1_STATUS                                    0x02010C
#define SSP_1_STATUS_STATUS                             4:4
#define SSP_1_STATUS_STATUS_IDLE                        0
#define SSP_1_STATUS_STATUS_BUSY                        1
#define SSP_1_STATUS_RECEIVE_FIFO                       3:2
#define SSP_1_STATUS_RECEIVE_FIFO_EMPTY                 0
#define SSP_1_STATUS_RECEIVE_FIFO_NOT_EMPTY             1
#define SSP_1_STATUS_RECEIVE_FIFO_FULL                  3
#define SSP_1_STATUS_TRANSMIT_FIFO                      1:0
#define SSP_1_STATUS_TRANSMIT_FIFO_FULL                 0
#define SSP_1_STATUS_TRANSMIT_FIFO_NOT_FULL             2
#define SSP_1_STATUS_TRANSMIT_FIFO_EMPTY                3

#define SSP_1_CLOCK_PRESCALE                            0x020110
#define SSP_1_CLOCK_PRESCALE_DIVISOR                    7:0

#define SSP_1_INTERRUPT_STATUS                          0x020114
#define SSP_1_INTERRUPT_STATUS_OVERRUN                  2:2
#define SSP_1_INTERRUPT_STATUS_OVERRUN_NOT_ACTIVE       0
#define SSP_1_INTERRUPT_STATUS_OVERRUN_ACTIVE           1
#define SSP_1_INTERRUPT_STATUS_OVERRUN_CLEAR            1
#define SSP_1_INTERRUPT_STATUS_TRANSMIT                 1:1
#define SSP_1_INTERRUPT_STATUS_TRANSMIT_NOT_ACTIVE      0
#define SSP_1_INTERRUPT_STATUS_TRANSMIT_ACTIVE          1
#define SSP_1_INTERRUPT_STATUS_RECEIVE                  0:0
#define SSP_1_INTERRUPT_STATUS_RECEIVE_NOT_ACTIVE       0
#define SSP_1_INTERRUPT_STATUS_RECEIVE_ACTIVE           1
