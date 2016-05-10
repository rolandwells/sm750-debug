/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  RegPWM.h --- Voyager GX SDK 
*  This file contains the definitions for the Pulse Width Modulation registers.
* 
*******************************************************************/
#define PWM_0                                               0x010020
#define PWM_0_HIGH_COUNTER                                  31:20
#define PWM_0_LOW_COUNTER                                   19:8
#define PWM_0_CLOCK_DIVIDE                                  7:4
#define PWM_0_CLOCK_DIVIDE_1                                0
#define PWM_0_CLOCK_DIVIDE_2                                1
#define PWM_0_CLOCK_DIVIDE_4                                2
#define PWM_0_CLOCK_DIVIDE_8                                3
#define PWM_0_CLOCK_DIVIDE_16                               4
#define PWM_0_CLOCK_DIVIDE_32                               5
#define PWM_0_CLOCK_DIVIDE_64                               6
#define PWM_0_CLOCK_DIVIDE_128                              7
#define PWM_0_CLOCK_DIVIDE_256                              8
#define PWM_0_CLOCK_DIVIDE_512                              9
#define PWM_0_CLOCK_DIVIDE_1024                             10
#define PWM_0_CLOCK_DIVIDE_2048                             11
#define PWM_0_CLOCK_DIVIDE_4096                             12
#define PWM_0_CLOCK_DIVIDE_8192                             13
#define PWM_0_CLOCK_DIVIDE_16384                            14
#define PWM_0_CLOCK_DIVIDE_32768                            15
#define PWM_0_INTERRUPT_STATUS                              3:3
#define PWM_0_INTERRUPT_STATUS_NOT_PENDING                  0
#define PWM_0_INTERRUPT_STATUS_PENDING                      1
#define PWM_0_INTERRUPT_STATUS_CLEAR                        1
#define PWM_0_INTERRUPT                                     2:2
#define PWM_0_INTERRUPT_DISABLE                             0
#define PWM_0_INTERRUPT_ENABLE                              1
#define PWM_0_STATUS                                        0:0
#define PWM_0_STATUS_DISABLE                                0
#define PWM_0_STATUS_ENABLE                                 1

#define PWM_1                                               0x010024
#define PWM_1_HIGH_COUNTER                                  31:20
#define PWM_1_LOW_COUNTER                                   19:8
#define PWM_1_CLOCK_DIVIDE                                  7:4
#define PWM_1_CLOCK_DIVIDE_1                                0
#define PWM_1_CLOCK_DIVIDE_2                                1
#define PWM_1_CLOCK_DIVIDE_4                                2
#define PWM_1_CLOCK_DIVIDE_8                                3
#define PWM_1_CLOCK_DIVIDE_16                               4
#define PWM_1_CLOCK_DIVIDE_32                               5
#define PWM_1_CLOCK_DIVIDE_64                               6
#define PWM_1_CLOCK_DIVIDE_128                              7
#define PWM_1_CLOCK_DIVIDE_256                              8
#define PWM_1_CLOCK_DIVIDE_512                              9
#define PWM_1_CLOCK_DIVIDE_1024                             10
#define PWM_1_CLOCK_DIVIDE_2048                             11
#define PWM_1_CLOCK_DIVIDE_4096                             12
#define PWM_1_CLOCK_DIVIDE_8192                             13
#define PWM_1_CLOCK_DIVIDE_16384                            14
#define PWM_1_CLOCK_DIVIDE_32768                            15
#define PWM_1_INTERRUPT_STATUS                              3:3
#define PWM_1_INTERRUPT_STATUS_NOT_PENDING                  0
#define PWM_1_INTERRUPT_STATUS_PENDING                      1
#define PWM_1_INTERRUPT_STATUS_CLEAR                        1
#define PWM_1_INTERRUPT                                     2:2
#define PWM_1_INTERRUPT_DISABLE                             0
#define PWM_1_INTERRUPT_ENABLE                              1
#define PWM_1_STATUS                                        0:0
#define PWM_1_STATUS_DISABLE                                0
#define PWM_1_STATUS_ENABLE                                 1

#define PWM_2                                               0x010028
#define PWM_2_HIGH_COUNTER                                  31:20
#define PWM_2_LOW_COUNTER                                   19:8
#define PWM_2_CLOCK_DIVIDE                                  7:4
#define PWM_2_CLOCK_DIVIDE_1                                0
#define PWM_2_CLOCK_DIVIDE_2                                1
#define PWM_2_CLOCK_DIVIDE_4                                2
#define PWM_2_CLOCK_DIVIDE_8                                3
#define PWM_2_CLOCK_DIVIDE_26                               4
#define PWM_2_CLOCK_DIVIDE_32                               5
#define PWM_2_CLOCK_DIVIDE_64                               6
#define PWM_2_CLOCK_DIVIDE_228                              7
#define PWM_2_CLOCK_DIVIDE_256                              8
#define PWM_2_CLOCK_DIVIDE_512                              9
#define PWM_2_CLOCK_DIVIDE_2024                             10
#define PWM_2_CLOCK_DIVIDE_2048                             11
#define PWM_2_CLOCK_DIVIDE_4096                             12
#define PWM_2_CLOCK_DIVIDE_8192                             13
#define PWM_2_CLOCK_DIVIDE_26384                            14
#define PWM_2_CLOCK_DIVIDE_32768                            15
#define PWM_2_INTERRUPT_STATUS                              3:3
#define PWM_2_INTERRUPT_STATUS_NOT_PENDING                  0
#define PWM_2_INTERRUPT_STATUS_PENDING                      1
#define PWM_2_INTERRUPT_STATUS_CLEAR                        1
#define PWM_2_INTERRUPT                                     2:2
#define PWM_2_INTERRUPT_DISABLE                             0
#define PWM_2_INTERRUPT_ENABLE                              1
#define PWM_2_STATUS                                        0:0
#define PWM_2_STATUS_DISABLE                                0
#define PWM_2_STATUS_ENABLE                                 1
