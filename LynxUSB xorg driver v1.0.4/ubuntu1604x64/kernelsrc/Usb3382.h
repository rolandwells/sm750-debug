/******************************************************************************

Copyright (C) 2010, PLX Technology, Inc. (http://www.PlxTech.com)

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

USB3382.H
 
This file embodies register and bit field constants for the PLX USB3382 
USB to PCIe bridge chip and its variants
 - Some details extracted from PLX databooks

******************************************************************************/

////////////////////////////////////////////////////////////////////////////////////
// USB3382 register and bit field definitions:
//  - Usage tip: Apply the shift operator ('<<') to build bit masks where needed 
//    in your code. Example:
//       if (RegisterValue & (1<<BIT_FIELD_NAME)) {DoSomething();}
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
#ifndef USB3382_H
#define USB3382_H

////////////////////////////////////////////////////////////////////////////////////
// USB3382 chip revisions
//  - See CHIPREV register
////////////////////////////////////////////////////////////////////////////////////
#define CHIPREV_NET2282_REV_1               0x00000100                  // NET2282 Rev-1 (first silicon)
#define CHIPREV_NET2282_REV_1A              0x00000110                  // NET2282 Rev-1A (mask change)
#define CHIPREV_USB3382_REV_AA              0x00000110                  // USB3382 Rev-AA (first silicon)

////////////////////////////////////////////////////////////////////////////////////
#define CHIPREV_REQUIRED                    CHIPREV_USB3382_REV_AA      // This build expects this CHIPREV

////////////////////////////////////////////////////////////////////////////////////
// USB3382 configuration ports:
//  - USB3382 configuration ports apply when chip is used in Root Complex mode (RC).
//  - USB3382 ports are accessed via CSROUT and CSRIN dedicated endpoints. The 
//    CSRCTL member of a CSROUT transfer must be set properly to access ports
//  - There are four ports, 0 through 3. Offsets to each set of configuration port
//    are 0x0000, 0x1000, 0x2000 and 0x30000 respectively. 
//  - Each USB3382 port maps to a type 0 or type 1 PCI header. Each port has 
//    different application and connection. For instance port 3 is the USB
//    controller; port 2 is a P-to-P bridge facing down to ports 0 and 1. Port 0
//    connects to your external ('remote') PCI device. Port 2 must be configured 
//    before configuring ports 0 and 1. (Port 1 is rarely used.)
//  - See USB3382 databook: CSRCTL register: CSR Space Select, PCI Express 
//    Configuration register (under section 17.7), 16.4.3 Device-Specific Memory-
//    Mapped Configuration Mechanism and related sections.

////////////////////////////////////////////////////////////////////////////////////
// USB3382 Port 0:
//  - PCIe type 1.
//  - Faces up to Port 2
//  - In RC mode, your external PCIe adapter connects to Port 0
#define PORT0_BASE                                          0x0000

////////////////////////////////////////////////////////////////////////////////////
// USB3382 Port 1:
//  - PCIe type 1.
//  - In RC mode, Port 1 is rarely used.
#define PORT1_BASE                                          0x1000

////////////////////////////////////////////////////////////////////////////////////
// USB3382 Port 2:
//  - PCIe type 1.
//  - In RC mode, Port 2 faces down to Ports 0 and 1.
//  - Port 2 is a virtual PCI-to-PCI bridge, internal to the USB3382.
#define PORT2_BASE                                          0x2000

////////////////////////////////////////////////////////////////////////////////////
// USB3382 Port 3, USB controller:
//  - Type 0 (PCIe endpoint). In RC mode, Port 3 has special capabilities.
//  - In RC mode, Port 3 faces up to USB host.
//  - PCIe agents (e.g. remote PCIe adapters) connected to Port 0 can 
//    access USB3382 BARs:
//     - BAR0: USBC, GPEP, DMAC registers, etc. (Rarely acccessed by 
//       remote PCIe adapters.)
//     - BAR1: On-chip 8051 memory. (Can also be used by remote a adapter
//       for storing remote DMAC's scatter/gather descriptors. See USB3382
//       databook Internal RAM Layout under 8.2.2 Endpoint FIFO Configuration)
//     - BAR2, BAR3: USB FIFOs. Large address spaces map to chip's USB IN
//       and OUT FIFOs. Remote adapter's DMACs use Port 3's BAR2 and BAR3 
//       addresses to efficiently transfer large amounts of data to and from
//       a USB host. See USB3382 databook BAR2CTL (under 17.5 USB Controller 
//       Device-Specific Registers) and related sections for important information.
#define PORT3_BASE                                          0x3000

////////////////////////////////////////////////////////////////////////////////////
// PCI Type 0 Configuration Registers
//  - Defined in PCI 2.3 specification
////////////////////////////////////////////////////////////////////////////////////

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIVENDID                                           0x000

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIDEVID                                            0x002

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCICMD                                              0x004
#define     IO_SPACE_ENABLE                                         0
#define     MEMORY_SPACE_ENABLE                                     1
#define     BUS_MASTER_ENABLE                                       2
#define     SPECIAL_CYCLES                                          3
#define     MEMORY_WRITE_AND_INVALIDATE_ENABLE                      4
#define     VGA_PALETTE_SNOOP                                       5
#define     PARITY_CHECKING_ENABLE                                  6
#define     ADDRESS_STEPPING_ENABLE                                 7
#define     SERR_ENABLE                                             8
#define     FAST_BACK_TO_BACK_ENABLE                                9
#define     INTERRUPT_DISABLE                                       10

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCISTAT                                             0x006
#define     INTERRUPT_STATUS                                        3
#define     CAPABILITY_VALID                                        4
#define     USER_DEFINABLE                                          6
#define     FAST_BACK_TO_BACK                                       7
#define     DATA_PARITY_ERROR_DETECTED                              8
#define     DEVSEL_TIMING                                           9
#define     TARGET_ABORT_ASSERTED                                   11
#define     TARGET_ABORT_RECEIVED                                   12
#define     MASTER_ABORT_RECEIVED                                   13
#define     SERR_ASSERTED                                           14
#define     PARITY_ERROR_DETECTED                                   15

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIDEVREV                                           0x008

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCICLASS                                            0x009
#define     INTERFACE                                               0
#define     SUB_CLASS                                               8
#define     BASE_CLASS                                              16

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCICACHESIZE                                        0x00c

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCILATENCY                                          0x00d

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIHEADER                                           0x00e

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIBIST                                             0x00f

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIBASE0                                            0x010
#define     SPACE_TYPE                                              0
#define     ADDRESS_TYPE                                            1
#define     PREFETCH_ENABLE                                         3

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIBASE1                                            0x014

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIBASE2                                            0x018

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIBASE3                                            0x01c

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIBASE4                                            0x020

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIBASE5                                            0x024

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define CARDBUS                                             0x028

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCISUBVID                                           0x02c

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCISUBID                                            0x02e

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIROMBASE                                          0x030

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCICAPPTR                                           0x034

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIINTLINE                                          0x03c

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIINTPIN                                           0x03d

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIMINGNT                                           0x03e

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIMINLAT                                           0x03f

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PWRMNGID                                            0x040

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PWRMNGNEXT                                          0x041

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PWRMNGCAP                                           0x042
#define     PME_VERSION                                             0
#define     PME_CLOCK                                               3
#define     DEVICE_SPECIFIC_INITIALIZATION                          5
#define     D1_SUPPORT                                              9
#define     D2_SUPPORT                                              10
#define     PME_SUPPORT                                             11

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PWRMNGCSR                                           0x044
#define     POWER_STATE                                             0
#define         D0                                                          0
#define         D1                                                          1
#define         D2                                                          2
#define         D3HOT                                                       3
#define     PME_ENABLE                                              8
#define     DATA_SELECT                                             9
#define     DATA_SCALE                                              13
#define     PME_STATUS                                              15

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit ///Val ////
#define PWRMNGBRIDGE                                        0x046

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PWRMNGDATA                                          0x047


////////////////////////////////////////////////////////////////////////////////////
// PCI Type 1 Configuration Registers
//  - Define only those registers that are different from type 0
////////////////////////////////////////////////////////////////////////////////////


/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PRIMARYBUSNUMBER                                  0x018

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define SECONDARYBUSNUMBER                                0x019

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define SUBORDINATEBUSNUMBER                              0x01a

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define SECONDARYLATENCYTIMER                             0x01b

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
// I/O Base, I/O Limit, Secondary Status
// Registers merged as one ULONG:
//  - IOBASE: 7:4 I/O base * 0x1000; 3:0 Decode type
//  - IOLIMIT: 15:12 (I/O limit * 0x1000) + 0xfff; 11:8 Decode type
//  - SECONDARYSTATUS: 31:16
#define IOBASE                                            0x01c

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define IOLIMIT                                           0x01d

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define SECONDARYSTATUS                                   0x01e

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
// Registers merged as one ULONG:
//  - MEMORYBASE: 15:4 Memory base * 0x100000; 3:0 Decode type
//  - MEMORYLIMIT: 31:20 (Memory limit * 0x100000) + 0xfffff; 19:16 Decode type
#define MEMORYBASE                                        0x020

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define MEMORYLIMIT                                       0x022

//XXXXXXXXXXXXXX TODO: Add more type 1 registers.

////////////////////////////////////////////////////////////////////////////////////
// USB Controller Device-Specific Registers
////////////////////////////////////////////////////////////////////////////////////

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DEVINIT                                             0x000
#define     M8051_RESET                                             0
#define     USB_SOFT_RESET                                          1
#define     PCI_SOFT_RESET                                          2
#define     CONFIGURATION_SOFT_RESET                                3
#define     FIFO_SOFT_RESET                                         4
#define     PCI_ENABLE                                              5
#define     PCI_ID                                                  6
#define     FORCE_PCI_RESET                                         7
#define     LOCAL_CLOCK_FREQUENCY                                   8

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCICTL                                              0x00C
#define     PCIBAR0_ENABLE                                          0
#define     PCIBAR1_ENABLE                                          1

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIIRQENB0                                          0x010
#define     ENDPOINT_0_INTERRUPT                                    0
#define     GPEP0_OUT_INTERRUPT                                     1
#define     GPEP1_OUT_INTERRUPT                                     2
#define     GPEP2_OUT_INTERRUPT                                     3
#define     GPEP3_OUT_INTERRUPT                                     4
#define     SETUP_PACKET_INTERRUPT                                  7
#define     USB_TO_PCIE_TLP_DRAINED_ON_PORT_0_INTERRUPT             8
#define     USB_TO_PCIE_TLP_DRAINED_ON_PORT_1_INTERRUPT             9
#define     USB_CONFIGURATION_RETRY_INTERRUPT                       10
#define     GPEP0_IN_INTERRUPT                                      17
#define     GPEP1_IN_INTERRUPT                                      18
#define     GPEP2_IN_INTERRUPT                                      19
#define     GPEP3_IN_INTERRUPT                                      20


/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIIRQENB1                                          0x014
#define     SOF_INTERRUPT                                           0
#define     RESUME_INTERRUPT                                        1
#define     SUSPEND_REQUEST_CHANGE_INTERRUPT                        2
#define     SUSPEND_REQUEST_INTERRUPT                               3
#define     ROOT_PORT_RESET_INTERRUPT                               4
#define     CONTROL_STATUS_INTERRUPT                                6
#define     VBUS_INTERRUPT                                          7
#define     EEPROM_DONE_INTERRUPT                                   8
#define     DMA_CHANNEL_0_INTERRUPT                                 9
#define     DMA_CHANNEL_1_INTERRUPT                                 10
#define     DMA_CHANNEL_2_INTERRUPT                                 11
#define     DMA_CHANNEL_3_INTERRUPT                                 12
#define     GPIO_INTERRUPT                                          13
#define     SOF_DOWNCOUNT_INTERRUPT                                 14
#define     PCI_MASTER_CYCLE_DONE_INTERRUPT                         16
#define     PCIE_HOT_PLUG_PCI_INTERRUPT                             18
#define     PCI_TARGET_ABORT_RECEIVED_INTERRUPT                     19
#define     PCI_MASTER_ABORT_RECEIVED_INTERRUPT                     20
#define     PCI_PARITY_ERROR_INTERRUPT                              25
#define     POWER_STATE_CHANGE_INTERRUPT                            27
#define     PCIE_DL_DOWN_STATE_CHANGE_INTERRUPT                     28
#define     PCIE_HOT_RESET_INTERRUPT                                29
#define     PCIE_ENDPOINT_POWER_MANAGEMENT_INTERRUPT                30
#define     GLOBAL_PCI_INTERRUPT                                    31

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define CPUIRQENB0                                          0x018
#define     ENDPOINT_0_INTERRUPT                                    0
#define     GPEP0_OUT_INTERRUPT                                     1
#define     GPEP1_OUT_INTERRUPT                                     2
#define     GPEP2_OUT_INTERRUPT                                     3
#define     GPEP3_OUT_INTERRUPT                                     4
#define     SETUP_PACKET_INTERRUPT                                  7
#define     USB_TO_PCIE_TLP_DRAINED_ON_PORT_0_INTERRUPT             8
#define     USB_TO_PCIE_TLP_DRAINED_ON_PORT_1_INTERRUPT             9
#define     USB_CONFIGURATION_RETRY_INTERRUPT                       10
#define     GPEP0_IN_INTERRUPT                                      17
#define     GPEP1_IN_INTERRUPT                                      18
#define     GPEP2_IN_INTERRUPT                                      19
#define     GPEP3_IN_INTERRUPT                                      20

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define CPUIRQENB1                                          0x01c
#define     SOF_INTERRUPT                                           0
#define     RESUME_INTERRUPT                                        1
#define     SUSPEND_REQUEST_CHANGE_INTERRUPT                        2
#define     SUSPEND_REQUEST_INTERRUPT                               3
#define     ROOT_PORT_RESET_INTERRUPT                               4
#define     CONTROL_STATUS_INTERRUPT                                6
#define     VBUS_INTERRUPT                                          7
#define     EEPROM_DONE_INTERRUPT                                   8
#define     DMA_CHANNEL_0_INTERRUPT                                 9
#define     DMA_CHANNEL_1_INTERRUPT                                 10
#define     DMA_CHANNEL_2_INTERRUPT                                 11
#define     DMA_CHANNEL_3_INTERRUPT                                 12
#define     GPIO_INTERRUPT                                          13
#define     SOF_DOWNCOUNT_INTERRUPT                                 14
#define     PCI_MASTER_CYCLE_DONE_INTERRUPT                         16
#define     PCIE_HOT_PLUG_PCI_INTERRUPT                             18
#define     PCI_TARGET_ABORT_RECEIVED_INTERRUPT                     19
#define     PCI_MASTER_ABORT_RECEIVED_INTERRUPT                     20
#define     CORRECTABLE_ERROR_MESSAGE_RECEIVED_INTERRUPT            21
#define     NON_FATAL_ERROR_MESSAGE_RECEIVED_INTERRUPT              22
#define     FATAL_ERROR_MESSAGE_RECEIVED_INTERRUPT                  23
#define     PCI_INTA_INTERRUPT                                      24
#define     PCI_PARITY_ERROR_INTERRUPT                              25
#define     POWER_STATE_CHANGE_INTERRUPT                            27
#define     PCIE_DL_DOWN_STATE_CHANGE_INTERRUPT                     28
#define     PCIE_HOT_RESET_INTERRUPT                                29
#define     PCIE_ENDPOINT_POWER_MANAGEMENT_INTERRUPT                30
#define     GLOBAL_CPU_INTERRUPT                                    31

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define USBIRQENB0                                          0x020
#define     EP_0_INTERRUPT                                          0
#define     GPEP0_OUT_INTERRUPT                                     1
#define     GPEP1_OUT_INTERRUPT                                     2
#define     GPEP2_OUT_INTERRUPT                                     3
#define     GPEP3_OUT_INTERRUPT                                     4
#define     SETUP_PACKET_INTERRUPT                                  7
#define     USB_TO_PCIE_TLP_DRAINED_ON_PORT_0                       8
#define     USB_TO_PCIE_TLP_DRAINED_ON_PORT_1                       9
#define     USB_CONFIGURATION_RETRY_INTERRUPT_STATUS                10
#define     GPEP0_IN_INTERRUPT                                      17
#define     GPEP1_IN_INTERRUPT                                      18
#define     GPEP2_IN_INTERRUPT                                      19
#define     GPEP3_IN_INTERRUPT                                      20

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define USBIRQENB1                                          0x024
#define     SOF_INTERRUPT                                           0
#define     RESUME_INTERRUPT                                        1
#define     SUSPEND_REQUEST_CHANGE_INTERRUPT                        2
#define     SUSPEND_REQUEST_INTERRUPT                               3
#define     ROOT_PORT_RESET_INTERRUPT                               4
#define     CONTROL_STATUS_INTERRUPT                                6
#define     VBUS_INTERRUPT                                          7
#define     EEPROM_DONE_INTERRUPT                                   8
#define     DMA_CHANNEL_0_INTERRUPT                                 9
#define     DMA_CHANNEL_1_INTERRUPT                                 10
#define     DMA_CHANNEL_2_INTERRUPT                                 11
#define     DMA_CHANNEL_3_INTERRUPT                                 12
#define     GPIO_INTERRUPT                                          13
#define     SOF_DOWNCOUNT_INTERRUPT                                 14
#define     PCI_MASTER_CYCLE_DONE_INTERRUPT                         16
#define     PCIE_HOT_PLUG_PCI_INTERRUPT                             18
#define     PCI_TARGET_ABORT_RECEIVED_INTERRUPT                     19
#define     PCI_MASTER_ABORT_RECEIVED_INTERRUPT                     20
#define     CORRECTABLE_ERROR_MESSAGE_RECEIVED_INTERRUPT            21
#define     NON_FATAL_ERROR_MESSAGE_RECEIVED_INTERRUPT              22
#define     FATAL_ERROR_MESSAGE_RECEIVED_INTERRUPT                  23
#define     PCI_INTA_INTERRUPT                                      24
#define     PCI_PARITY_ERROR_INTERRUPT                              25
#define     POWER_STATE_CHANGE_INTERRUPT                            27
#define     PCIE_DL_DOWN_STATE_CHANGE_INTERRUPT                     28
#define     PCIE_HOT_RESET_INTERRUPT                                29
#define     PCIE_ENDPOINT_POWER_MANAGEMENT_INTERRUPT                30
#define     GLOBAL_USB_INTERRUPT                                    31

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define IRQSTAT0                                            0x028
#define     EP_0_INTERRUPT                                          0
#define     GPEP0_OUT_INTERRUPT                                     1
#define     GPEP1_OUT_INTERRUPT                                     2
#define     GPEP2_OUT_INTERRUPT                                     3
#define     GPEP3_OUT_INTERRUPT                                     4
#define     SETUP_PACKET_INTERRUPT                                  7
#define     USB_TO_PCIE_TLP_DRAINED_ON_PORT_0                       8
#define     USB_TO_PCIE_TLP_DRAINED_ON_PORT_1                       9
#define     USB_CONFIGURATION_RETRY_INTERRUPT_STATUS                10
#define     INTA_ASSERTED                                           12      // TRUE: INTA# asserted by USB3382
#define     GPEP0_IN_INTERRUPT                                      17
#define     GPEP1_IN_INTERRUPT                                      18
#define     GPEP2_IN_INTERRUPT                                      19
#define     GPEP3_IN_INTERRUPT                                      20

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define IRQSTAT1                                            0x02c
#define     SOF_INTERRUPT                                           0
#define     RESUME_INTERRUPT                                        1
#define     SUSPEND_REQUEST_CHANGE_INTERRUPT                        2
#define     SUSPEND_REQUEST_INTERRUPT                               3
#define     ROOT_PORT_RESET_INTERRUPT                               4
#define     CONTROL_STATUS_INTERRUPT                                6
#define     VBUS_INTERRUPT                                          7
#define     EEPROM_DONE_INTERRUPT                                   8
#define     DMA_CHANNEL_0_INTERRUPT                                 9
#define     DMA_CHANNEL_1_INTERRUPT                                 10
#define     DMA_CHANNEL_2_INTERRUPT                                 11
#define     DMA_CHANNEL_3_INTERRUPT                                 12
#define     GPIO_INTERRUPT                                          13
#define     SOF_DOWNCOUNT_INTERRUPT                                 14
#define     PCI_MASTER_CYCLE_DONE_INTERRUPT                         16
#define     PCIE_HOT_PLUG_PCI_INTERRUPT                             18
#define     PCI_TARGET_ABORT_RECEIVED_INTERRUPT                     19
#define     PCI_MASTER_ABORT_RECEIVED_INTERRUPT                     20
#define     CORRECTABLE_ERROR_MESSAGE_RECEIVED_INTERRUPT            21
#define     NON_FATAL_ERROR_MESSAGE_RECEIVED_INTERRUPT              22
#define     FATAL_ERROR_MESSAGE_RECEIVED_INTERRUPT                  23
#define     PCI_INTA_INTERRUPT                                      24
#define     PCI_PARITY_ERROR_INTERRUPT                              25
#define     POWER_STATE_CHANGE_INTERRUPT                            27
#define     PCIE_DL_DOWN_STATE_CHANGE_INTERRUPT                     28
#define     PCIE_HOT_RESET_INTERRUPT                                29
#define     PCIE_ENDPOINT_POWER_MANAGEMENT_INTERRUPT                30

////////////////////////////////////////////////////////////////////////////////////
// Indexed Register Address:
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define IDXADDR                                             0x030

////////////////////////////////////////////////////////////////////////////////////
// Indexed Register Data:
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define IDXDATA                                             0x034

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define FIFOCTL                                             0x038
#define     FIFO_CONFIGURATION_SELECT                               0
#define         GPEP_3210_1K                                                0
#define         GPEP_10_2K                                                  1
#define         GPEP_0_2K_GPEP_21_1K                                        2
#define     PCI_BASE2_SELECT                                        2
#define     IGNORE_FIFO_AVAILABILITY                                3
#define     PCI_BAR2_RANGE                                         16

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define BAR2CTL                                             0x03c
#define     QUADRANT0_DIRECTION                                     0
#define     QUADRANT0_GPEP                                          1
#define     QUADRANT1_DIRECTION                                     4
#define     QUADRANT1_GPEP                                          5
#define     QUADRANT2_DIRECTION                                     8
#define     QUADRANT2_GPEP                                          9
#define     QUADRANT3_DIRECTION                                     12
#define     QUADRANT3_GPEP                                          13
#define     PCI_BAR2_RANGE                                         16

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define BAR3CTL                                             0x040
#define     QUADRANT0_DIRECTION                                     0
#define     QUADRANT0_GPEP                                          1
#define     QUADRANT1_DIRECTION                                     4
#define     QUADRANT1_GPEP                                          5
#define     QUADRANT2_DIRECTION                                     8
#define     QUADRANT2_GPEP                                          9
#define     QUADRANT3_DIRECTION                                     12
#define     QUADRANT3_GPEP                                          13
#define     PCI_BAR3_RANGE                                         16

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define GPIOCTRL                                             0x050
#define     GPIO0_DATA                                              0
#define     GPIO1_DATA                                              1
#define     GPIO2_DATA                                              2
#define     GPIO3_DATA                                              3
#define     GPIO0_OUTPUT_ENABLE                                     4
#define     GPIO1_OUTPUT_ENABLE                                     5
#define     GPIO2_OUTPUT_ENABLE                                     6
#define     GPIO3_OUTPUT_ENABLE                                     7
#define     GPIO0_INTERRUPT_ENABLE                                  8
#define     GPIO1_INTERRUPT_ENABLE                                  9
#define     GPIO2_INTERRUPT_ENABLE                                  10
#define     GPIO3_INTERRUPT_ENABLE                                  11
#define     GPIO3_LED_SELECT                                        12
#define     GPIO0_INPUT_DEBOUNCE_ENABLE                             16
#define     GPIO1_INPUT_DEBOUNCE_ENABLE                             17
#define     GPIO2_INPUT_DEBOUNCE_ENABLE                             18
#define     GPIO3_INPUT_DEBOUNCE_ENABLE                             19
#define     GPIO0_PWM_ENABLE                                        24
#define     GPIO1_PWM_ENABLE                                        25
#define     GPIO2_PWM_ENABLE                                        26
#define     GPIO3_PWM_ENABLE                                        27

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define GPIOSTAT                                            0x054
#define     GPIO0_INTERRUPT                                         0
#define     GPIO1_INTERRUPT                                         1
#define     GPIO2_INTERRUPT                                         2
#define     GPIO3_INTERRUPT                                         3

////////////////////////////////////////////////////////////////////////////////////
//XXXXXXXXXXXXX TODO: Define more USB3382 USBC registers.

////////////////////////////////////////////////////////////////////////////////////
// USB Interface Control Registers
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// Standard Response Control:
//  - When a STDRSP bit is set, the USB3382 will 
//    automatically respond to the USB host request
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define STDRSP                                              0x080
#define     GET_DEVICE_STATUS                                       0
#define     GET_INTERFACE_STATUS                                    1
#define     GET_ENDPOINT_STATUS                                     2
#define     GET_DEVICE_DESCRIPTOR                                   3
#define     GET_CONFIGURATION_DESCRIPTOR                            4
#define     GET_SET_CONFIGURATION                                   5
#define     GET_SET_INTERFACE                                       6
#define     GET_STRING_DESCRIPTOR_0                                 8
#define     GET_STRING_DESCRIPTOR_1                                 9
#define     GET_STRING_DESCRIPTOR_2                                 10
#define     DEVICE_SET_CLEAR_DEVICE_REMOTE_WAKEUP                   11
#define     ENDPOINT_SET_CLEAR_HALT                                 12
#define     SET_ADDRESS__                                           13      // SET_ADDRESS already defined elsewhere
#define     GET_DEVICE_QUALIFIER                                    14
#define     GET_OTHER_SPEED_CONFIGURATION                           15
#define     SET_TEST_MODE                                           16
#define     SET_CLR_FUNCTION_SUSPEND                                17
#define     SET_CLR_U1_ENABLE                                       18
#define     SET_CLR_U2_ENABLE                                       19
#define     SET_CLR_LTM_ENABLE                                      20
#define     GET_BOS_DESCRIPTOR                                      21
// TODO: Resolve compiler multiple dec (See UsbStd.h): #define     SET_SEL                                                 22
#define     GET_STRING_DESCRIPTOR_3                                 23
#define     SET_ISOCHRONOUS_DELAY                                   24
#define     STALL_UNSUPPORTED_REQUESTS                              31

/////////////////////////////////////////////////////////////////////////////////////
// USB product and Vendor IDs:
//  - Returned by chip's on-board automatic enumeration controller 
//    when GET_DEVICE_DESCRIPTOR bit in STDRSP is set
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PRODVENDID                                          0x084
#define     VENDOR_ID                                               0
#define     PRODUCT_ID                                              16

/////////////////////////////////////////////////////////////////////////////////////
// Device Release Number:
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define RELNUM                                              0x088

////////////////////////////////////////////////////////////////////////////////////
// USB Control:
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define USBCTL                                              0x08c
#define     SELF_POWERED_STATUS                                     0
#define     REMOTE_WAKEUP_ENABLE                                    1
#define     PCIE_WAKEUP_ENABLE                                      2
#define     USB_DETECT_ENABLE                                       3
#define     REMOTE_WAKEUP_SUPPORT                                   5
#define     SELF_POWERED_USB_DEVICE                                 6
#define     IMMEDIATELY_SUSPEND                                     7
#define     TIMED_DISCONNECT                                        9
#define     VBUS_PIN                                                10
#define     USB_ROOT_PORT_WAKEUP_ENABLE                             11
#define     VENDOR_ID_STRING_ENABLE                                 12
#define     PRODUCT_ID_STRING_ENABLE                                13


////////////////////////////////////////////////////////////////////////////////////
// USB Status:
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define USBSTAT                                             0x090
#define     GENERATE_DEVICE_REMOTE_WAKEUP                           4
#define     GENERATE_RESUME                                         5
#define     FULL_SPEED                                              6
#define     HIGH_SPEED                                              7
#define     SUPER_SPEED                                             8
#define     SUSPEND_STATUS                                          9
#define     REMOTE_WAKEUP_STATUS                                    10
#define     USB_ENHANCED                                            11
#define     HOST_MODE                                               12

/////////////////////////////////////////////////////////////////////////////////////
// USB Transceiver Diagnostic Control:
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define XCVRDIAG                                            0x094
#define     TERMINATION_SELECT                                      0
#define     TRANSCEIVER_SELECT                                      1
#define     TRANSCEIVER_OPERATION_MODE                              2
#define     LINE_STATE                                              16
#define     USB_TEST_MODE                                           24
#define     TRANSCEIVER_LOOPBACK                                    28
#define     FORCE_FULL_SPEED_MODE                                   30
#define     FORCE_HIGH_SPEED_MODE                                   31

/////////////////////////////////////////////////////////////////////////////////////
// Setup Dword 0:
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define SETUPDW0                                            0x098
#define     SETUP_BYTE_0                                            0
#define     SETUP_BYTE_1                                            8
#define     SETUP_BYTE_2                                            16
#define     SETUP_BYTE_3                                            24

/////////////////////////////////////////////////////////////////////////////////////
// Setup Dword 1:
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define SETUPDW1                                            0x09C
#define     SETUP_BYTE_7                                            24
#define     SETUP_BYTE_6                                            16
#define     SETUP_BYTE_5                                            8
#define     SETUP_BYTE_4                                            0

/////////////////////////////////////////////////////////////////////////////////////
// Our USB Address:
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define OURADDR                                             0x0a4
#define     FORCE_IMMEDIATE                                         7
#define     OUR_USB_ADDRESS                                         0

/////////////////////////////////////////////////////////////////////////////////////
// Our USB Configuration:
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define OURCONFIG                                           0x0a8

/////////////////////////////////////////////////////////////////////////////////////
// USB Class:
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define USB_CLASS                                           0x0b4
#define     CLASS__                                                 0               // 7:0 'CLASS' defined elsewhere
#define     SUB_CLASS                                               8               // 15:8
#define     PROTOCOL                                                16              // 23:16

/////////////////////////////////////////////////////////////////////////////////////
// Super-Speed System Exit Latency:
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define SS_SEL                                              0x0b8
#define     U1_SYSTEM_EXIT_LATENCY                                  0               // 11:0
#define     U2_SYSTEM_EXIT_LATENCY                                  12              // 23:12

/////////////////////////////////////////////////////////////////////////////////////
// Super-Speed Device Exit Latency:
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define SS_DEL                                              0x0bc
#define     U1_DEVICE_EXIT_LATENCY                                  0               // 7:0
#define     U2_DEVICE_EXIT_LATENCY                                  8               // 23:8

/////////////////////////////////////////////////////////////////////////////////////
// USB2 Link Power Management:
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define USB2LPM                                             0x0c0
#define     USB_L1_LPM_SUPPORT                                      0
#define     USB_L1_LPM_REMOTE_WAKE_ENABLE                           1
#define     USB_L1_LPM_HIRD                                         2               // 5:2

/////////////////////////////////////////////////////////////////////////////////////
// USB3 Best Effort Latency Tolerance:
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define USB3BELT                                             0x0c4
#define     BEST_EFFORT_LATENCY_TOLERANCE                           0               // 9:0
#define     BELT_MULTIPLIER                                         10              // 11:10

////////////////////////////////////////////////////////////////////////////////////
// USB Control 2:
//  - See also USBCTL
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define USBCTL2                                             0x0c8
#define     SERIAL_NUMBER_STRING_ENABLE                             0
#define     USB2_CORE_ENABLE                                        2
#define     USB3_CORE_ENABLE                                        3
// Name conflict tip: Below, USB spec and chip spec unfortunately use identical 
// names. Added _CONTROL to chip bits to distinguish chip bits from USB values.
#define     FUNCTION_SUSPEND_CONTROL                                4
#define     U1_ENABLE_CONTROL                                       5
#define     U2_ENABLE_CONTROL                                       6
#define     LTM_ENABLE_CONTROL                                      7
#define     PCIOUT_ERDY_CONTROL                                     8


/////////////////////////////////////////////////////////////////////////////////////
// Isochronous Delay
//  - Isochronous Delay. "This delay represents the time from when the host starts 
//    transmitting the first framing symbol of the packet to when the device 
//    receives the first framing symbol of that packet. The range is 0 to 65535 ns."
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define ISODELAY                                            0xd0

////////////////////////////////////////////////////////////////////////////////////
// PCI Express/Configuration Cursor Registers
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// PCI Master Control:
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIMSTCTL                                           0x100
#define     PCIE_FIRST_BYTE_ENABLES                                 0               // 3:0
#define     PCIE_MASTER_COMMAND_SELECT                              4               // 5:4
#define         MEM_READ_OR_WRITE                                           0
#define         IO_READ_OR_WRITE                                            1
#define         CFG_READ_OR_WRITE                                           2
#define         MESSAGE_WRITE                                               3
#define     PCIE_MASTER_START                                       6
#define     PCIE_MASTER_READ_WRITE                                  7
#define     MESSAGE_CODE                                            8               // 15:8
#define     MESSAGE_ROUTING                                         16              // 18:16
#define     MESSAGE_TYPE                                            19
#define     PCIE_PORT                                               20
#define     PCIE_DW_LENGTH                                          24              // 30:24

////////////////////////////////////////////////////////////////////////////////////
// PCI Master Address
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIMSTADDR                                          0x104

////////////////////////////////////////////////////////////////////////////////////
// PCI Master Data
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIMSTDATA                                          0x108

////////////////////////////////////////////////////////////////////////////////////
// PCI Master Status
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIMSTSTAT                                          0x10C
#define     PCI_HOST_MODE                                           0

////////////////////////////////////////////////////////////////////////////////////
// CSR Control
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define CSRCTL                                              0x110
#define     CSR_BYTE_ENABLES                                        0               // 3:0
#define     CSR_SPACE_SELECT                                        4               // 5:4
#define         CONFIGURATION_SPACE                                         0
#define         MEM_SPACE                                                   1
#define         M8051_SPACE                                                 2
#define     CSR_START                                               6
#define     CSR_READ_WRITE                                          7
#define     CSR_ADDRESS                                             16              // 31:16

////////////////////////////////////////////////////////////////////////////////////
// CSR Data
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define CSRDATA                                             0x114                   // 31:0

////////////////////////////////////////////////////////////////////////////////////
// Semaphore
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define SEMAPHORE                                           0x118
#define     SEMAPHORE_                                              0


////////////////////////////////////////////////////////////////////////////////////
// PCI Master Msg
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIMSTMSG                                           0x11C
#define     PCI_MASTER_MSG                                          0               // 31:0

////////////////////////////////////////////////////////////////////////////////////
// DMA Registers:
//  - There are four sets of DMA registers, one for each GPEP.
//  - DMA registers sets start at offset 0x180, and 0x20 bytes apart:
//     Offset to DMA0 register set: 0x180
//     Offset to DMA1 register set: 0x1a0
//     Offset to DMA2 register set: 0x1c0
//     Offset to DMA3 register set: 0x1e0
//  - Macros are provided to simplify accessing DMA register sets
//
// DMA Scatter/Gather descriptors (See USB3382 databook 10.3 Scatter/Gather Mode.)
//  - In an S/G entry, the following descriptor DWORDs associate 
//    with USB3382 DMA registers:
//       DMACOUNT:  First DWORD of descriptor (containing DMA Byte Count and controls)
//       DMAADDR:   Second DWORD (containing PCI Starting Address)
//       DMADESC:   Third DWORD (containing Next Descriptor Address)
////////////////////////////////////////////////////////////////////////////////////

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DMACTL                                              0x180
#define     DMA_ADDRESS_CONSTANT                                    0
#define     DMA_ENABLE                                              1
#define     DMA_FIFO_VALIDATE                                       2           // Terminate current S/G entry with short packet
#define     DMA_PREEMPT_ENABLE                                      3
#define     DMA_OUT_AUTO_START_ENABLE                               4
#define     DMA_REQUESTS_OUTSTANDING                                5           // 7:5
#define     DMA_DESCRIPTOR_MODE                                     16
#define     DMA_VALID_BIT_ENABLE                                    17
#define     DMA_VALID_BIT_POLLING_ENABLE                            18
#define     DESCRIPTOR_POLLING_RATE                                 19
#define         POLL_CONTINUOUS                                             0
#define         POLL_1_USEC                                                 1
#define         POLL_100_USEC                                               2
#define         POLL_1_MSEC                                                 3
#define     DMA_CLEAR_COUNT_ENABLE                                  21
#define     PREFETCH_DISABLE                                        22
#define     PAUSE_MODE                                              23
#define     DMA_DESCRIPTOR_DONE_INTERRUPT_ENABLE                    25
#define     DMA_PAUSE_DONE_INTERRUPT_ENABLE                         26
#define     DMA_ABORT_DONE_INTERRUPT_ENABLE                         27

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DMASTAT                                             0x184
#define     DMA_START                                               0
#define     DMA_ABORT                                               1
#define     DMA_TRANSACTION_DONE_INTERRUPT                          24
#define     DMA_LAST_DESCRIPTOR_DONE_INTERRUPT                      25
#define     DMA_PAUSE_DONE_INTERRUPT                                26
#define     DMA_ABORT_DONE_INTERRUPT                                27
#define     DMA_COMPLETION_SEQUENCE_ERROR_STATUS                    31

///////////////////////////////////////////////////////////////////////////////
// DMACOUNT, DMAADDR, DMADESC exist in CSR registers as well as DMA descriptors:
//  - DMACOUNT: Once the DMA is started, the DMA updates DMACOUNT CSR. Upon DMA 
//    completion, the CPU can read DMACOUNT CSR to do accounting for the number 
//    of bytes processed by the final descriptor.
//  - DMAADDR: Like DMACOUNT, upon DMA completion, the CPU reads DMAADDR CSR to
//    do accounting for the number of bytes processed by the final descriptor.
//  - DMADESC: Before starting the DMA, the CPU sets DMADESC CSR to the first
//    descriptor of a DMA descriptor chain. Upon DMA completion, the CPU can
//    read DMADESC CSR to determine the descriptor upon which the DMA stopped.
///////////////////////////////////////////////////////////////////////////////

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DMACOUNT                                            0x190
#define     DMA_TRANSFER_LENGTH                                      0              // 23:0
#define         DMA_TRANSFER_LENGTH_MASK                                    ((1<<24) - 1)
#define     DMA_OUT_CONTINUE                                        24
#define     DMA_DESCRIPTOR_FIFO_VALIDATE                            27          // TRUE: Validates short packet. Applies to S/G usage
#define     DMA_LAST_DESCRIPTOR                                     28
#define     DMA_DONE_INTERRUPT_ENABLE                               29
#define     DMA_DIRECTION                                           30          // TRUE: USB IN (PCI to USB)
#define     VALID_BIT                                               31

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DMAADDR                                             0x194                   // 31:0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DMADESC                                             0x198
#define     ON_CHIP                                                 0
#define     DESCRIPTOR_PORT_SELECT                                  1
#define     DATA_PORT_SELECT                                        2
#define     NEXT_DESCRIPTOR_ADDRESS                                 4               // 31:4

////////////////////////////////////////////////////////////////////////////////////
// Dedicated Endpoint Registers
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// Dedicated Endpoint Configuration
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DEP_CFG                                             0x200
// DEP_CFG bits are identical to bits defined in EP_CFG:
//  - ENDPOINT_ENABLE
//  - ENDPOINT_TYPE
//  - ENDPOINT_NUMBER

////////////////////////////////////////////////////////////////////////////////////
// Dedicated Endpoint Response 
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DEP_RSP                                             0x204
// DEP_RSP bits are identical to bits defined in EP_RSP:
//  - SET_NAK_PACKETS
//  - SET_ENDPOINT_TOGGLE
//  - SET_ENDPOINT_HALT
//  - CLEAR_NAK_PACKETS
//  - CLEAR_ENDPOINT_TOGGLE
//  - CLEAR_ENDPOINT_HALT

////////////////////////////////////////////////////////////////////////////////////
// EP 0 and GPEPx Registers:
//  - Tip: Each GPEP embodies separate OUT and IN EP registers, however
//    a few are common, notably EP_CFG (and therefore EP_BYTE_COUNT):
//  - See USB3382 databook 17.10 EP 0 and GPEPx Registers
//  - Legacy tip: When USB3382 strapped for Legacy Adapter mode, apply NET2282 
//    mapping (See NET2282 databook 11.10 Configurable Endpoint/FIFO Registers.)
//              EP 0    GPEP0 OUT/IN    GPEP1 OUT/IN    GPEP2 OUT/IN    GPEP3 OUT/IN
//  EP_CFG      300h    320h            340h            360h            380h
//  EP_RSP      304h    324h/3E4h       344h/404h       364h/424h       384h/444h
//  EP_IRQENB   308h    328h/3E8h       348h/408h       368h/428h       388h/448h
//  EP_STAT     30Ch    32Ch/3ECh       34Ch/40Ch       36Ch/42Ch       38Ch/44Ch
//  EP_AVAIL    310h    330h/3F0h       350h/410h       370h/430h       390h/450h
//  EP_DATA     314h    334h/3F4h       354h/414h       374h/434h       394h/454h
//  EP_DATA1    318h    338h/3F8h       358h/418h       378h/438h       398h/458h
//  EP_VAL      31Ch    33Ch            35Ch            37Ch            39Ch
//
// Address offset from base address of a GPEP to its OUT or IN register sets:
//  - PCIe addressing tip: The OUT and IN EPs in a single GPEP are differentiated
//    by offset 0xc0 (e.g. 0x344 for EP_RSP_OUT and 0x404 for EP_RSP_IN)
//  - Tip: GPEP IN register set does NOT include EP_CFG or EP_VAL
//  - Tip: In legacy mode, USB338x maps both IN and OUT EPs to the OUT register set
//  - Usage: EpStat = NC_EP_READ((IsEpIn? GPEP_IN_OFFSET: GPEP_OUT_OFFSET) + EP_STAT_OUT);
//     - Usage tips: Applies to GPEPs, not EP0; not applicable to EP_CFG; use 
//       an EP_XXXX_OUT register variant plus the OUT or IN offest
#define GPEP_OUT_OFFSET                                     0x00
#define GPEP_IN_OFFSET                                      0xc0

////////////////////////////////////////////////////////////////////////////////////
// Endpoint Configuration (combined as follows):
//  - Endpoint Configuration for EP0 
//  - Endpoint Configuration for GPEP endpoints
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_CFG                                              0x300
#define     ENDPOINT_NUMBER                                         0               // 3:0
#define     ENDPOINT_DIRECTION                                      7               // EP0 tip: For EP0, direction automatically changed according to the direction specified in the SETUP packet.
#define     ENDPOINT_TYPE                                           8               // 9:8 (Legacy mode; type applies to IN or OUT EP)
#define     OUT_ENDPOINT_TYPE                                       8               // 9:8 (Enhanced mode)
#define         CONTROL                                                     0
#define         ISOCHRONOUS                                                 1
#define         BULK_                                                       2       // (BULK defined elsewhere)
#define         INTERRUPT                                                   3
#define     ENDPOINT_ENABLE                                         10              // (Legacy mode; enable applies to IN or OUT EP)
#define     OUT_ENDPOINT_ENABLE                                     10              // (Enhanced mode)
#define     BYTE_PACKING_ENABLE                                     11              // (Enhanced mode only)
#define     IN_ENDPOINT_TYPE                                        12              // 13:12 (Enhanced mode only)
#define         CONTROL                                                     0
#define         ISOCHRONOUS                                                 1
#define         BULK_                                                       2       // (BULK defined elsewhere)
#define         INTERRUPT                                                   3
#define     IN_ENDPOINT_ENABLE                                      14              // (Enhanced mode only)
#define     IN_EP_FORMAT                                            15
#define     EP_FIFO_BYTE_COUNT                                      16              // 16:18
#define     SERVICE_INTERVAL                                        19              // 23:19
#define     MAX_BURST_SIZE                                          24              // 27:24
#define     SYNC_TYPE                                               28              // 29:28
#define         NO_SYNCHRONIZATION                                          0
#define         ASYNCHRONOUS                                                1
#define         ADAPTIVE                                                    2
#define         SYNCHRONOUS                                                 3
#define     USAGE_TYPE                                              30              // 31:30
#define         PERIODIC_OR_DATA                                            0       // Interrupt or Isoch
#define         NOTIFICATION_OR_FEEDBACK                                    1       // Interrupt or Isoch
#define         IMPLICIT_FEEDBACK_DATA                                      2       // Isoch

////////////////////////////////////////////////////////////////////////////////////
// Endpoint Byte Count:
//  - Treated as a special 16 bit sub-register in EP_CFG:
//  - Usage example: To terminate an IN transfer with a short packet using PIO
//    set Endpoint Byte Count to 0, 1, 2, 3, or 4 then write the final (32 bit) 
//    word of the transfer to EP_DATA. The USB3382 will automatically write 
//    the word to the FIFO and validate the FIFO with 0, 1, 2, 3 or 4 bytes.
//  - EP_DATA must be written even if Endpoint Byte Count is set to zero.
//  - Endpoint Byte Count defaults to 4, and is restored to 4 after
//    every write to EP_DATA (or FIFO Flush).
// ZLP considerations: 
//  - Care must be taken for terminating transfers that are exact packet 
//    multiples. (i.e. transfers that are an exact multiple of the endpoint
//    MaxPacketSize.) High level protocol architectures may require Zero Length
//    Packets (ZLPs) to follow the final data packet (or may NOT!):
//  - After writing the final data to EP_DATA, software must make sure
//    there is room available in the FIFO for at least 4 bytes before 
//    setting Endpoint Byte Count to zero, then writing EP_DATA
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_BYTE_COUNT                                       0x302
                                                                    
////////////////////////////////////////////////////////////////////////////////////
// Endpoint Response for EP0 and OUT GPEPs
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_RSP_OUT                                          0x304
#define     CLEAR_ENDPOINT_HALT                                     0
#define     CLEAR_ENDPOINT_TOGGLE                                   1
#define     CLEAR_NAK_OUT_PACKETS_MODE                              2
#define     CLEAR_CONTROL_STATUS_PHASE_HANDSHAKE                    3
#define     CLEAR_INTERRUPT_MODE                                    4
#define     CLEAR_ENDPOINT_FORCE_CRC_ERROR                          5
#define     CLEAR_EP_HIDE_STATUS_PHASE                              6
#define     CLEAR_NAK_PACKETS                                       7
#define     SET_ENDPOINT_HALT                                       8
#define     SET_ENDPOINT_TOGGLE                                     9
#define     SET_NAK_OUT_PACKETS_MODE                                10
#define     SET_CONTROL_STATUS_PHASE_HANDSHAKE                      11
#define     SET_INTERRUPT_MODE                                      12
#define     SET_ENDPOINT_FORCE_CRC_ERROR                            13
#define     SET_EP_HIDE_STATUS_PHASE                                14
#define     SET_NAK_PACKETS                                         15

////////////////////////////////////////////////////////////////////////////////////
// Endpoint Interrupt Enable for EP0 and OUT GPEPs
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_IRQENB_OUT                                       0x308
#define     DATA_IN_TOKEN_INTERRUPT_ENABLE                          0
#define     DATA_OUT_PING_TOKEN_INTERRUPT_ENABLE                    1
#define     DATA_PACKET_TRANSMITTED_INTERRUPT_ENABLE                2
#define     DATA_PACKET_RECEIVED_INTERRUPT_ENABLE                   3
#define     SHORT_OUT_PACKET_RECEIVED_INTERRUPT_ENABLE              5       
#define     SHORT_PACKET_OUT_DONE_INTERRUPT_ENABLE                  6
#define     ZLP_INTERRUPT_ENABLE                                    13
#define     DMA_INTERRUPT_STATUS_ENABLE                             14

////////////////////////////////////////////////////////////////////////////////////
// Endpoint Status for EP0 and OUT GPEPs
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_STAT_OUT                                         0x30c
#define     DATA_IN_TOKEN_INTERRUPT                                 0
#define     DATA_OUT_PING_TOKEN_INTERRUPT                           1
#define     DATA_PACKET_TRANSMITTED_INTERRUPT                       2
#define     DATA_PACKET_RECEIVED_INTERRUPT                          3
#define     NAK_PACKETS                                             4
#define     SHORT_OUT_PACKET_RECEIVED_INTERRUPT                     5
#define     SHORT_PACKET_OUT_DONE_INTERRUPT                         6
#define     SHORT_PACKET_TRANSFERRED_STATUS                         7
#define     FIFO_VALIDATE                                           8
#define     FIFO_FLUSH                                              9
#define     FIFO_EMPTY                                              10
#define     FIFO_FULL                                               11
#define     ZLP_INTERRUPT                                           13
#define     DMA_INTERRUPT_STATUS                                    14
#define     USB_OUT_ACK_SENT                                        16
#define     USB_OUT_NAK_SENT                                        17
#define     USB_IN_ACK_RCVD                                         18
#define     USB_IN_NAK_SENT                                         19
#define     USB_STALL_SENT                                          20
#define     TIMEOUT                                                 21
#define     HIGH_BANDWIDTH_OUT_TRANSACTION_PID                      22              // 23:22
#define         DATA0                                                       0
#define         DATA1                                                       1
#define         DATA2                                                       2
#define         MDATA                                                       3
#define     FIFO_VALID_COUNT                                        24              // 28:24

////////////////////////////////////////////////////////////////////////////////////
// Endpoint Available Count for EP0 and OUT GPEPs
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_AVAIL_OUT                                        0x310
#define     ENDPOINT_AVAILABLE_COUNT                                0               // 13:0

////////////////////////////////////////////////////////////////////////////////////
// Endpoint Data for EP0 and OUT GPEPs
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_DATA_OUT                                         0x314

////////////////////////////////////////////////////////////////////////////////////
// Endpoint Data1 for EP0 and OUT GPEPs
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_DATA1_OUT                                        0x318
#define     ENDPOINT_DATA_BYTE_ENABLES                              0
#define     ENDPOINT_DATA_END_OF_PACKET                             4

////////////////////////////////////////////////////////////////////////////////////
// Endpoint Validate for EP0 and OUT GPEPs
#define EP_VAL                                              0x31c
                                                                    
////////////////////////////////////////////////////////////////////////////////////
// Endpoint Response for IN GPEPs
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_RSP_IN                                           (EP_RSP_OUT + GPEP_IN_OFFSET)
#define     CLEAR_ENDPOINT_HALT                                     0
#define     CLEAR_ENDPOINT_TOGGLE                                   1
#define     CLEAR_NAK_OUT_PACKETS_MODE                              2
#define     CLEAR_CONTROL_STATUS_PHASE_HANDSHAKE                    3
#define     CLEAR_INTERRUPT_MODE                                    4
#define     CLEAR_ENDPOINT_FORCE_CRC_ERROR                          5
#define     CLEAR_EP_HIDE_STATUS_PHASE                              6
#define     CLEAR_NAK_PACKETS                                       7
#define     SET_ENDPOINT_HALT                                       8
#define     SET_ENDPOINT_TOGGLE                                     9
#define     SET_NAK_OUT_PACKETS_MODE                                10
#define     SET_CONTROL_STATUS_PHASE_HANDSHAKE                      11
#define     SET_INTERRUPT_MODE                                      12
#define     SET_ENDPOINT_FORCE_CRC_ERROR                            13
#define     SET_EP_HIDE_STATUS_PHASE                                14
#define     SET_NAK_PACKETS                                         15

////////////////////////////////////////////////////////////////////////////////////
// Endpoint Interrupt Enable for IN GPEPs
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_IRQENB_IN                                        (EP_IRQENB_OUT + GPEP_IN_OFFSET)
#define     DATA_IN_TOKEN_INTERRUPT_ENABLE                          0
#define     DATA_OUT_PING_TOKEN_INTERRUPT_ENABLE                    1
#define     DATA_PACKET_TRANSMITTED_INTERRUPT_ENABLE                2
#define     DATA_PACKET_RECEIVED_INTERRUPT_ENABLE                   3
#define     SHORT_OUT_PACKET_RECEIVED_INTERRUPT_ENABLE              5
#define     SHORT_PACKET_OUT_DONE_INTERRUPT_ENABLE                  6
#define     ZLP_INTERRUPT_ENABLE                                    13
#define     DMA_INTERRUPT_STATUS_ENABLE                             14

////////////////////////////////////////////////////////////////////////////////////
// Endpoint Status for IN GPEPs
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_STAT_IN                                          (EP_STAT_OUT + GPEP_IN_OFFSET)
#define     DATA_IN_TOKEN_INTERRUPT                                 0
#define     DATA_OUT_PING_TOKEN_INTERRUPT                           1
#define     DATA_PACKET_TRANSMITTED_INTERRUPT                       2
#define     DATA_PACKET_RECEIVED_INTERRUPT                          3
#define     NAK_PACKETS                                             4
#define     SHORT_OUT_PACKET_RECEIVED_INTERRUPT                     5
#define     SHORT_PACKET_OUT_DONE_INTERRUPT                         6
#define     SHORT_PACKET_TRANSFERRED_STATUS                         7
#define     FIFO_VALIDATE                                           8
#define     FIFO_FLUSH                                              9
#define     FIFO_EMPTY                                              10
#define     FIFO_FULL                                               11
#define     ZLP_INTERRUPT                                           13
#define     DMA_INTERRUPT_STATUS                                    14
#define     USB_OUT_ACK_SENT                                        16
#define     USB_OUT_NAK_SENT                                        17
#define     USB_IN_ACK_RCVD                                         18
#define     USB_IN_NAK_SENT                                         19
#define     USB_STALL_SENT                                          20
#define     TIMEOUT                                                 21
#define     HIGH_BANDWIDTH_OUT_TRANSACTION_PID                      22              // 23:22
#define         DATA0                                                       0
#define         DATA1                                                       1
#define         DATA2                                                       2
#define         MDATA                                                       3
#define     FIFO_VALID_COUNT                                        24              // 28:24

////////////////////////////////////////////////////////////////////////////////////
// Endpoint Available Count for IN GPEPs
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_AVAIL_IN                                         (EP_AVAIL_OUT + GPEP_IN_OFFSET)
#define     ENDPOINT_AVAILABLE_COUNT                                0               // 13:0

////////////////////////////////////////////////////////////////////////////////////
// Endpoint Data for IN GPEPs
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_DATA_IN                                          (EP_DATA_OUT + GPEP_IN_OFFSET)

////////////////////////////////////////////////////////////////////////////////////
// Endpoint Data1 for IN GPEPs
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_DATA1_IN                                         (EP_DATA1_OUT + GPEP_IN_OFFSET)
#define     ENDPOINT_DATA_BYTE_ENABLES                              0
#define     ENDPOINT_DATA_END_OF_PACKET                             4


////////////////////////////////////////////////////////////////////////////////////
// FIFO Registers
//  - Advanced! 
//  - Tip: One set of FIFO management registers applies to all EPs: EP0 and GPEPs.
//  - Usage example for EP0: NETCHIP_READ(EP_FIFO_OUT_RDPTR);
//  - Usage example for GPEPs: NETCHIP_READ(GPEPPAGEOFFSET(GPEP) + EP_FIFO_OUT_RDPTR);
////////////////////////////////////////////////////////////////////////////////////

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_FIFOSIZE                                         0x500
#define     OUT_FIFO_SIZE                                           0               // 2:0 Size: 2^(6 + FIFO_SIZE)
#define     OUT_FIFO_BASE_ADDRESS                                   6               // 14:6 (Range restriction tip: 0000 to 4000)
#define     IN_FIFO_SIZE                                            16              // 18:16 Size: 2^(6 + FIFO_SIZE)
#define     IN_FIFO_BASE_ADDRESS                                    22              // 30:22 (Range restriction tip: 0000 to 1370; 6000 to 7370)
#define         FIFO_SIZE_64                                                0       // These FIFO sizes apply to OUT or IN FIFO size
#define         FIFO_SIZE_128                                               1       //  - FifoSizeBytes = (0x40 * (2 ^ FifoSizeValue))
#define         FIFO_SIZE_256                                               2
#define         FIFO_SIZE_512                                               3
#define         FIFO_SIZE_1024                                              4
#define         FIFO_SIZE_2048                                              5
#define         FIFO_SIZE_4096                                              6

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_FIFO_OUT_WRPTR                                   0x504
#define     OUT_FIFO_GLOBAL_WRITE_POINTER                           0               // 12:0
#define     OUT_FIFO_WORKING_WRITE_POINTER                          16              // 28:16

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_FIFO_OUT_RDPTR                                   0x508
#define     OUT_FIFO_GLOBAL_READ_POINTER                            0               // 15:0
#define     OUT_FIFO_WORKING_READ_POINTER                           16              // 31:16

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_FIFO_IN_WRPTR                                    0x510
#define     IN_FIFO_GLOBAL_WRITE_POINTER                            0               // 15:0
#define     IN_FIFO_WORKING_WRITE_POINTER                           16              // 31:16

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_FIFO_IN_RDPTR                                    0x514
#define     IN_FIFO_GLOBAL_READ_POINTER                             0               // 15:0
#define     IN_FIFO_WORKING_READ_POINTER                            16              // 31:16

////////////////////////////////////////////////////////////////////////////////////
// Power Management Registers
//  - USBPM, USBPM <<XXXXXXX TODO: To be defined>>
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// Link Layer General Control:
//  - For factory use only
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define LL_GENERAL_CONTROL_1                                0x718
#define     FORCE_DL_BIT                                            5
#define     DL_BIT_VALUE_FMW                                        6
#define     RESEND_DPP_ON_LRTY_FMW                                  7
#define     SKP_THRESHOLD_ADJUST_FMW                                8               // 13:8
#define     PM_U1_EN                                                16
#define     PM_U2_EN                                                17
#define     PM_DIR_ENTRY_U1                                         18
#define     PM_DIR_ENTRY_U2                                         19
#define     PM_DIR_ENTRY_U3                                         20
#define     PM_FORCE_LINK_ACCEPT                                    22
#define     PM_DIR_LINK_REJECT                                      23
#define     PM_LGO_COLLISION_SEND_LAU                               24
#define     PM_FORCE_U1_ENTRY                                       25
#define     PM_FORCE_U2_ENTRY                                       26
#define     PM_U1_AUTO_EXIT                                         27
#define     PM_U2_AUTO_EXIT                                         28
#define     PM_U3_AUTO_EXIT                                         29

////////////////////////////////////////////////////////////////////////////////////
// Link Layer General Control:
//  - For factory use only
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define LL_GENERAL_CONTROL_2

////////////////////////////////////////////////////////////////////////////////////
// Link Layer U0 Idle to U1:
//  - For factory use only
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define LL_U0_IDLE_TO_U1                                    0x734

////////////////////////////////////////////////////////////////////////////////////
// Indexed Registers
//  - Indexed registers are 32 bits wide, and accessed by 
//    selecting a register ordinal in IDXADDR then reading 
//    or writing its content via IDXDATA.
////////////////////////////////////////////////////////////////////////////////////

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DIAG                                                0x0
#define     FORCE_TRANSMIT_CRC_ERROR                                0
#define     FORCE_RECEIVE_ERROR                                     2
#define     FAST_TIMES                                              4
#define     ILLEGAL_BYTE_ENABLES                                    5
#define     FORCE_CPU_INTERRUPT                                     8
#define     FORCE_USB_INTERRUPT                                     9
#define     FORCE_PCI_INTERRUPT                                     10
#define     RETRY_COUNTER                                           16

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PKTLEN                                              0x1

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define FRAME                                               0x2

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define CHIPREV                                             0x3

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define UFRAME                                              0x4

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define FRAME_COUNT                                         0x5

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define HS_MAXPOWER                                         0x6

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define FS_MAXPOWER                                         0x7

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define HS_INTPOLL_RATE                                     0x8

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define FS_INTPOLL_RATE                                     0x9

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define HS_NAK_RATE                                         0xA

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define SCRATCH                                             0xB

////////////////////////////////////////////////////////////////////////////////////
// GPEP indexed registers:
//  - All GPEP indexed registers follow the pattern of GPEP0 OUT. Apply math to 
//    index an arbitrary GPEP indexed register. 
// For a GPEP n, where XX is one of HS, FS or SS:
//  - Index to an OUT GPEP n: GPEP0_OUT_XX_MAXPKT + (n * 0x10) + 0x00
//  - Index to an IN GPEP n : GPEP0_OUT_XX_MAXPKT + (n * 0x10) + 0x40
//  - Example: Indexof_Gpep3In_FsMaxPacket = GPEP0_OUT_FS_MAXPKT + (3 * 0x10) + 0x40;
//  - Example: Indexof_GpepN_FsMaxPacket   = GPEP0_OUT_FS_MAXPKT + (GpepN * 0x10) + (DirIn? 0x40: 0x00);
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// High-Speed Max Packet Size
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define GPEP0_OUT_HS_MAXPKT                                 0x20
#define     HIGH_SPEED_MAXIMUM_PACKET_SIZE                          0               // 10:0
#define     ADDITIONAL_TRANSACTION_OPPORTUNITIES                    11              // 12:11

////////////////////////////////////////////////////////////////////////////////////
// Full-Speed Max Packet Size
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define GPEP0_OUT_FS_MAXPKT                                 0x21

////////////////////////////////////////////////////////////////////////////////////
// Super-Speed Max Packet Size
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define GPEP0_OUT_SS_MAXPKT                                 0x22
#define     SUPER_SPEED_MAXIMUM_PACKET_SIZE                         0               // 10:0
#define     MAX_PACKETS_PER_SERVICE_INTERVAL                        11              // 12:11
#define     TOTAL BYTES PER SERVICE INTERVAL                        16              // 31:16

////////////////////////////////////////////////////////////////////////////////////
// CPEP0_OUT, GPEP0_IN, 
// GPEP1_OUT, GPEP1_IN
// GPEP2_OUT, GPEP2_IN
// GPEP3_OUT, GPEP3_IN
//  - See GPEP comments above
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// High-Speed Interrupt Polling Rate for STATIN and RCIN
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define STATIN_HS_INTPOLL_RATE                              0x84

////////////////////////////////////////////////////////////////////////////////////
// Full-Speed Interrupt Polling Rate for STATIN
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define STATIN_FS_INTPOLL_RATE                              0x85

////////////////////////////////////////////////////////////////////////////////////
// Super-Speed Maximum Power
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define SS_MAXPOWER                                         0x86

////////////////////////////////////////////////////////////////////////////////////
// Miscellaneous USB3382 constants
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// Configurable endpoints:
//  - EP0, GPEP0_OUT, GPEP0_IN, GPEP1_OUT, GPEP1_IN, ...
//  - Endpoint numbers map one-to-one to their bit positions in IRQSTAT0
//  - Base address for each endpoint's registers reside at a fixed address
//  - Use macros to get address of a configurable endpoint register:
//     AddressOf_EpStat_For_EpB = GPEPPAGEOFFSET(EPB) + EP_STAT;
#define EP0                                                 EP_0_INTERRUPT
#define GPEP0_OUT                                           GPEP0_OUT_INTERRUPT
#define GPEP1_OUT                                           GPEP1_OUT_INTERRUPT
#define GPEP2_OUT                                           GPEP2_OUT_INTERRUPT
#define GPEP3_OUT                                           GPEP3_OUT_INTERRUPT
/* For future chips with more GPEPs, continue same pattern:
#define GPEP4_OUT                                           GPEP4_OUT_INTERRUPT
#define GPEP5_OUT                                           GPEP5_OUT_INTERRUPT
#define GPEP6_OUT                                           GPEP6_OUT_INTERRUPT
#define GPEP7_OUT                                           GPEP7_OUT_INTERRUPT
#define GPEP8_OUT                                           GPEP8_OUT_INTERRUPT
#define GPEP9_OUT                                           GPEP9_OUT_INTERRUPT
#define GPEP10_OUT                                          GPEP10_OUT_INTERRUPT
#define GPEP11_OUT                                          GPEP11_OUT_INTERRUPT
#define GPEP12_OUT                                          GPEP12_OUT_INTERRUPT
#define GPEP13_OUT                                          GPEP13_OUT_INTERRUPT
#define GPEP14_OUT                                          GPEP14_OUT_INTERRUPT
*/
#define GPEP0_IN                                            GPEP0_IN_INTERRUPT
#define GPEP1_IN                                            GPEP1_IN_INTERRUPT
#define GPEP2_IN                                            GPEP2_IN_INTERRUPT
#define GPEP3_IN                                            GPEP3_IN_INTERRUPT
/* For future chips with more GPEPs, continue same pattern:
#define GPEP4_IN                                            GPEP4_IN_INTERRUPT
#define GPEP5_IN                                            GPEP5_IN_INTERRUPT
#define GPEP6_IN                                            GPEP6_IN_INTERRUPT
#define GPEP7_IN                                            GPEP7_IN_INTERRUPT
#define GPEP8_IN                                            GPEP8_IN_INTERRUPT
#define GPEP9_IN                                            GPEP9_IN_INTERRUPT
#define GPEP10_IN                                           GPEP10_IN_INTERRUPT
#define GPEP11_IN                                           GPEP11_IN_INTERRUPT
#define GPEP12_IN                                           GPEP12_IN_INTERRUPT
#define GPEP13_IN                                           GPEP13_IN_INTERRUPT
#define GPEP14_IN                                           GPEP14_IN_INTERRUPT
*/

////////////////////////////////////////////////////////////////////////////////////
// Dedicated endpoints:
//  - CSROUT, CSRIN, PCIOUT, PCIIN, STATIN, RCIN
//  - Use macros to get PCIe offset of a dedicated endpoint register:
//     Offsetof_DepCfg_For_PciIn = DEDEPPAGEOFFSET(PCIIN) + DEP_CFG;
#define CSROUT                                              0   // 0x200
#define CSRIN                                               1   // 0x210
#define PCIOUT                                              2   // 0x220
#define PCIIN                                               3   // 0x230
#define STATIN                                              4   // 0x240
#define RCIN                                                5   // 0x250

// Helper macros for dedicated endpoints
#define SIZEOF_DEDICATEDEP                                  0x10
#define DEDEPPAGEOFFSET(dep)                              ((dep)*SIZEOF_DEDICATEDEP)
#define FIRSTDEDEPREG                                       DEP_CFG
#define DEDENDPOINT_COUNT                                   6


////////////////////////////////////////////////////////////////////////////////////
#define NUMDMACHANNELREGISTERS                              8
#define FIRSTDMACHANNELREG                                  DMACTL

#define SIZEOF_DMACTLREG                                    0x20
#define DMACHANNELOFFSET(gpep)                              ((gpep)*SIZEOF_DMACTLREG)
#define DMACHANNEL_INT_ENABLE_BIT(ep)                       ((ep)+8)

#define SIZEOF_ENDPOINT                                     0x20    // Number of bytes in a set of Endpoint/FIFO registers
#define NUMEPREGISTERS                                      8
#define GPEPPAGEOFFSET(gpep)                                (((gpep) + 1)*SIZEOF_ENDPOINT)
#define FIRSTEPREG                                          EP_CFG
#define EP_HS_MAXPKT(ep)                                    ((ep)*0x10+0x10)
#define EP_FS_MAXPKT(ep)                                    ((ep)*0x10+0x11)

////////////////////////////////////////////////////////////////////////////////////
// Number of GPEPs, EPs and DMA channels:
//  - Endpoint information (not including EP0)
//  - Number of endpoints on the chip, endpoint dimensions, etc.

// Number of GPEPs on the chip
//  - A GPEP embodies one IN and one OUT data EP:
#define NUMBEROF_GPEPS                                      4

// Number of EPs for each GPEP
//  - A GPEP embodies one IN and one OUT data EP:
#define NUMBEROF_EPS_PER_GPEP                               2

// Number of data EPs on the chip:
//  - All IN plus all OUT EPs on the chip excluding EP0
#define NUMBEROF_DATA_EPS                                   (NUMBEROF_EPS_PER_GPEP * NUMBEROF_GPEPS)

// EP indices: For EP0, the first, and final data EPs
//  - Application: Useful for indexing an array of software-managed EP 
//    structures, that might or might not include EP0
#define INDEXOF_EP0                                         0
#define INDEXOF_FIRST_DATA_EP                               1
#define INDEXOF_FINAL_DATA_EP                               NUMBEROF_DATA_EPS

#define INDEXOF_FIRST_GPEP                                  INDEXOF_FIRST_DATA_EP
#define INDEXOF_FINAL_GPEP                                  NUMBEROF_GPEPS

// Number of DMA controllers
//  - Each GPEP has one DMAC bound to it
#define NUMBEROF_DMACS                                      NUMBEROF_GPEPS

////////////////////////////////////////////////////////////////////////////////////
// Maximum number of USB endpoints available for a single USB configuration:
//  - A USB device application may use up to this many endpoints in one configuration
#define MAX_EPS_PER_USB_CONFIG                              (DIMENSIONS_PER_GPEP * (FINAL_PHYSICAL_GPEP - FIRST_PHYSICAL_GPEP))

////////////////////////////////////////////////////////////////////////////////////
// USB3382 8051 microcontroller
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
#define MAX8051PROGRAMSIZE                                  0x8000

////////////////////////////////////////////////////////////////////////////////////
#endif // USB3382_H

////////////////////////////////////////////////////////////////////////////////////
//  End of file
////////////////////////////////////////////////////////////////////////////////////
