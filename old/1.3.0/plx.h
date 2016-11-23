#ifndef  PLXf_INC
#define  PLXf_INC


#include	"Usb3382.h"
#include	"Rpcie.h"

#define PORT3_BUS 0
#define GPEP_FIFOS_PORT3_BAR2 ((PORT3_BUS << 28)|(0<<24)|(2<<20))
#define GPEP_FIFOS_PORT3_BAR3 ((PORT3_BUS << 28)|(0<<24)|(3<<20))
#define PORT2_BUS 1
#define PORT0_BUS 2

#define ADDRESS_RANGE_PER_USB_FIFO (1<<16)
#define SIZEOF_USB_HOST_BUFFER ADDRESS_RANGE_PER_USB_FIFO

#define USB_VENDOR_ID_NETCHIP 0x0525
#define PLX_3380 0x1820


#define REG_WRITE_CSROUT_SIZE 8
#define REG_READ_CSROUT_SIZE 4
#define REG_WRITE_PCIOUT_SIZE 12
#define REG_READ_PCIOUT_SIZE 8

#define WRITE_PCIMM_ASYNC 0x80000000
#define WRITE_PCIMM_MULTIDWORD 0x40000000
#define PLX_3380_CONSTANT_ADDR 1

#define MB(x)	(x << 20)
#define KB(x) ((x)<<10)

int NcRpci_StartDevice(struct ufb_data * ufb);
int CsrUsbc_ReadUlong(struct ufb_data * ufb,uint16_t address,void * data);
int CsrUsbc_WriteUlong(struct ufb_data * ufb,uint16_t address,uint32_t data);
int CsrConfig_ReadUlong(struct ufb_data * ufb,uint16_t address,void * data);
int CsrConfig_WriteUlong(struct ufb_data * ufb,uint16_t address,uint32_t data);
uint32_t PciConfig_ReadUlongEx(struct ufb_data * ufb,PREMOTE_CONFIGURATION_REGISTER config,uint16_t Register);
void PciConfig_WriteUlongEx(struct ufb_data * ufb,
								PREMOTE_CONFIGURATION_REGISTER Config,
								uint16_t Register,
								uint32_t Value);


/*
 * single function version write rpcie memory routine
 * can only write A dword per transfer 
 * can only use synchronouse method 
 * */
void PciMm_WriteUlong(struct ufb_data *,uint32_t,uint32_t);

/* full function version write rpcie memory routine
 * can write max up to 32DWORD per transfer
 * can let user tell use async or sync method
 * */
void PciMm_WriteUlong_full(struct ufb_data *,uint32_t,uint32_t *,uint32_t);
uint32_t PciMm_ReadUlong(struct ufb_data * ufb,uint32_t reg );
int PciMm_WriteDma(struct ufb_data * ufb,uint32_t reg,const void * src,int size,int flag);
int PciMm_WriteDma_dataport(struct ufb_data * ufb,
							uint32_t reg,
							uint32_t data);
int ConstructDevice(struct ufb_data * ufb);
void NC3380_flushDma(struct ufb_data * ufb);
int PciMm_DmaStage1(struct ufb_data * ufb,void * src,int cnt, int cutbpp);
int PciMm_DmaStage0(struct ufb_data * ufb,uint32_t reg,int cnt);
#endif   /* ----- #ifndef PLXf_INC  ----- */
