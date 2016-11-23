#ifndef REMOTEPCI_H
#define REMOTEPCI_H

#include	"Usb3382.h"


typedef struct _RPCI_OUTPUT_STRUCT_DRIVER_VERSION
{   
    uint8_t       MajorVersion;
    uint8_t       Year;
    uint8_t       Month;
    uint8_t       Day;
} RPCI_OUTPUT_STRUCT_DRIVER_VERSION, *PRPCI_OUTPUT_STRUCT_DRIVER_VERSION;

#define USBEP_GPEP0_OUT         0x02
#define USBEP_GPEP0_IN          0x82
#define USBEP_GPEP1_OUT         0x04
#define USBEP_GPEP1_IN          0x84
#define USBEP_GPEP2_OUT         0x06
#define USBEP_GPEP2_IN          0x86
#define USBEP_GPEP3_OUT         0x08
#define USBEP_GPEP3_IN          0x88

typedef struct REMOTE_CONFIGURATION_REGISTER_
{  
    uint8_t Bus;
    uint8_t Function;
    uint8_t Device;
    int Type;
    int Port;

} REMOTE_CONFIGURATION_REGISTER, * PREMOTE_CONFIGURATION_REGISTER;

#define DED_EP_RCIN     0x8c
#define DED_EP_CSROUT   0x0d
#define DED_EP_CSRIN    0x8d
#define DED_EP_PCIOUT   0x0e
#define DED_EP_PCIIN    0x8e
#define DED_EP_STATIN   0x8f

typedef enum _NC_CFG_SPACE
{   
    PCI_CONFIG = ((1<<CSR_START)  | (0<<4)),            
    MEMORY_MAPPED_CONFIG = ((1<<CSR_START)  | (1<<4)),  
    CPU_8051_RAM = ((1<<CSR_START)  | (2<<4))           
} NC_CFG_SPACE;

#pragma pack (push)
#pragma pack (1)
typedef struct _NC_CSROUT_PACKET
{   
    uint16_t SpaceSelByteEnables;
    uint16_t Address;
    uint32_t Data;
} NC_CSROUT_PACKET, *PNC_CSROUT_PACKET;
#pragma pack (pop)

#pragma pack (push)
#pragma pack (2)
typedef struct _NC_PCIOUT_PACKET
{
    uint32_t PciMstCtl;
    uint32_t PciMstAddr;
    uint32_t PciMstData;
} NC_PCIOUT_PACKET, *PNC_PCIOUT_PACKET;

#pragma pack (pop)

typedef struct _NC_PCI_OR_CSR_OUT_PACKET
{
    uint32_t Transparent[3]; 
} NC_PCI_OR_CSR_OUT_PACKET, *PNC_PCI_OR_CSR_OUT_PACKET;

#pragma pack (push)
#pragma pack (2)
typedef struct _STATIN_PACKET
{   
    uint32_t IrqStat1;
    uint16_t EpIrqStat;
} STATIN_PACKET, *PSTATIN_PACKET;

#pragma pack (pop)

typedef struct _NC_DMA
{
    void* ClientContext;
    uint32_t RemotePciAddress;
    uint32_t DmaControl;
} NC_DMA, *PNC_DMA;



#endif  
