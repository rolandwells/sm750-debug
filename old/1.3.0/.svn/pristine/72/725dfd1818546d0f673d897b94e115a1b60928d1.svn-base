/*
 * =====================================================================================
 *
 *       Filename:  usbacc.c
 *
 *    Description:  this file contains functions used to send and receive urb
 *
 *        Version:  1.0
 *        Created:  01/10/2012 10:29:28 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  monk.liu (ml), monk.liu@siliconmotion.com
 *        Company:  siliconmotion,inc
 *
 * =====================================================================================
 */


#include	<linux/kernel.h>
#include	<linux/errno.h>
#include	<linux/init.h>
#include	<linux/slab.h>
#include	<linux/module.h>
#include	<linux/kref.h>
#include	<linux/uaccess.h>
#include	<linux/usb.h>
#include	<linux/mutex.h>
#include	<linux/vmalloc.h>
#include	<linux/jiffies.h>
#include	<linux/time.h>

#if (DEBUG != 0)
#define DEBUG_TMP DEBUG
#undef DEBUG
#define DEBUG 0
#endif
#include	"usbfb.h"
#include "ufb_drv.h"

#include	"plx.h"

int NcRpci_StartDevice(struct ufb_data * ufb){
	int ret;

	ret = 0;
	ENTER();

	LEAVE(ret);
	return ret;
}

static void OnSendTimedUrbComplete_address(struct urb * urb){
	struct urb_man * man;
	int ret;

	//ENTER();
	man = urb->context;
	ret = urb->status;
	if(ret){
		error("urb transfer failed,error = %d",ret);
	}
	//LEAVE();
}

static void OnSendTimedUrbComplete_done_async(struct urb * urb){
	int ret;
	ENTER();
	ret = urb->status;
	if(ret){
		error("async_urb failed,status:%d",ret);
		LEAVE();
	}
	LEAVE();
}

static void OnSendTimedUrbComplete_done(struct urb * urb){
	struct urb_man * man;
	int ret;

	//ENTER();
	man = urb->context;
	ret = urb->status;
	if(ret){
		printk("%s:error=%d",__func__,ret);
		goto exit;
	}
	
	SET_URB_TRANSFER_STATE(0,man);
	/* wake up the thread stuck on man->completion */
exit:
	complete(&man->completion);
	//LEAVE();
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  RemoteRegWrite
 *  Description:  this function is synchronous,though it will shcedule out for a while
 *  				caller need only consider it a synchronouse routine
 * =====================================================================================
 */
static inline int RemoteRegWrite(struct ufb_data * ufb,
						uint8_t epOut,
						void * outPack,
						struct urb_man * man,
						uint32_t flag)
{
	struct urb * urbOut;
	bool ongoing,async;
	int ret,ct;
	int buflen;
	int e1=0,e2=0;

	//ENTER();
	ret = 0;
	async = flag & WRITE_PCIMM_ASYNC;
	buflen  = flag & 0xffffff;/* buffer length is contained in lower 3bytes of flag */

	if(async){
		/* asynchronouse version */
		urbOut = usb_alloc_urb(0,GFP_KERNEL);
		usb_fill_bulk_urb(urbOut,
				ufb->udev,
				usb_sndbulkpipe(ufb->udev,epOut),
				outPack,
				buflen,
				OnSendTimedUrbComplete_done_async,
				NULL);

		ret = usb_submit_urb(urbOut,GFP_KERNEL);
		if(ret){
			error("async_urb submit failed,error = %d",ret);
			goto exit;
		}
		usb_free_urb(urbOut);
	}else{
		init_completion(&man->completion);
		/* synchronouse version */
		urbOut = ufb->urbs[0];
		usb_fill_bulk_urb(urbOut,
				ufb->udev,
				usb_sndbulkpipe(ufb->udev,epOut),
				outPack,
				buflen,
				OnSendTimedUrbComplete_done,
				man);
		SET_URB_TRANSFER_STATE(1,man);
		ret = usb_submit_urb(urbOut,GFP_KERNEL);
		if(ret){
			printk("er1:%d,e1=%d,e2=%d\n",ret,e1,e2);
			if(ret == -EPIPE){
				usb_clear_halt(ufb->udev,usb_sndbulkpipe(ufb->udev,epOut));
				NC3380_flushDma(ufb);
				goto exit;
			}
			if(ret == -ESHUTDOWN || ret == -ENODEV){
				printk("plugout!1");
				ufb->status = UFB_STAT_PLUGOUT;
			}
			goto exit;
		}
		ct = 0;

		if(ct++ > 5){
			printk("?");
			ret = -ETIMEDOUT;
			goto exit;
		}
		GET_URB_TRANSFER_STATE(ongoing,man);
		if(ongoing)
		{
			ret = wait_for_completion_timeout(&man->completion,USB_WAIT_JIFF);
			e1 = ret;
			e2 = urbOut->status;			
			if(ret)
			{
				ret = urbOut->status;
				switch(urbOut->status)
				{
					case -EPIPE:
						printk("EPIPE\n");
						usb_clear_halt(ufb->udev,usb_sndbulkpipe(ufb->udev,epOut));
						break;
					case -ENODEV:/* device plugged out*/
					case -ESHUTDOWN:
						ufb->status = UFB_STAT_PLUGOUT;
						printk("plugout!2");
						break;
					case -ENOENT:
						printk("ENOENT\n");
						NC3380_flushDma(ufb);
					default:						
						break;
				}				
				goto exit;
			}else{
				ret = -ETIMEDOUT;
				goto exit;
			}
		}
	}
exit:
	if(ret>0)
		ret = 0;
	//LEAVE(ret);
	return ret;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  RemoteRegRead
 *  Description:  this function is synchronous,though it will schedule out for a while
 *  			  caller need only consider it a synchrounous routine
 * =====================================================================================
 */
static int RemoteRegRead(struct ufb_data * ufb,
							uint8_t epOut,
							uint8_t epIn,
							PNC_PCI_OR_CSR_OUT_PACKET outPack,
							void * data,
							struct urb_man * man)
{
	/*
	 * stage 1: send an out dir urb,tell device which register we need reed
	 * stage 2: send an in dir urb,get the register value from device
	 * */
	struct urb * urbOut,*urbIn;
	bool ongoing;
	int ret,rt,ct;

	//ENTER();
	rt = 0;
	ret = 0;
	init_completion(&man->completion);
	urbOut = ufb->urbs[0];
	urbIn = ufb->urbs[1];
	/* filling out urb with known status and information */
	usb_fill_bulk_urb(urbOut,
					ufb->udev,
					usb_sndbulkpipe(ufb->udev,epOut),
					outPack,
					(epOut == DED_EP_PCIOUT)?REG_READ_PCIOUT_SIZE:REG_READ_CSROUT_SIZE,
					OnSendTimedUrbComplete_address,
					man);


	/* filling in urb with known status and information */
	usb_fill_bulk_urb(urbIn,
					ufb->udev,
					usb_rcvbulkpipe(ufb->udev,epIn),
					data,/* register value written to */
					sizeof(uint32_t),
					OnSendTimedUrbComplete_done,
					man);

	man->priv = urbIn;

	/* set flag to claim the authority of urb man */
	SET_URB_TRANSFER_STATE(1,man);

	/* submit our urb, be aware that it's not done,just returned */
	ret = usb_submit_urb(urbOut,GFP_KERNEL);
	if(ret){
		printk("er3:%d",ret);
		if(ret == -ENODEV || ret == -ESHUTDOWN){
			ufb->status = UFB_STAT_PLUGOUT;
			printk("plugout!3");
		}
		goto exit;
	}

	ret = usb_submit_urb(urbIn,GFP_KERNEL);
	if(ret){
		printk("er4:%d",ret);
		if(ret == -ENODEV || ret == -ESHUTDOWN){
			ufb->status = UFB_STAT_PLUGOUT;
			printk("plugout!4");
		}
		goto exit;
	}

	ct = 0;

	if(ct++ > 5){
		ret = -ETIMEDOUT;
		goto exit;
	}
	GET_URB_TRANSFER_STATE(ongoing,man);
	if(ongoing)
	{
		ret = wait_for_completion_timeout(&man->completion,USB_WAIT_JIFF);
		if(ret)
		{
			ret = urbIn->status;
			switch(urbIn->status)
			{
				case -EPIPE:
					printk("EPIPE\n");
					usb_clear_halt(ufb->udev,usb_sndbulkpipe(ufb->udev,epOut));
					break;
				case -ENODEV:/* device plugged out*/
				case -ESHUTDOWN:
					ufb->status = UFB_STAT_PLUGOUT;
					printk("plugout!5");
					break;
				case -ENOENT:
					NC3380_flushDma(ufb);
				default:
					break;
			}
			goto exit;
		}else{
			ret = -ETIMEDOUT;
			goto exit;
		}
	}
exit:
	//LEAVE(ret);
	return ret;
}

static int PciRegRead(struct ufb_data * ufb,PNC_PCIOUT_PACKET PciOutPacket,void * data){
	struct urb_man * man;
	int ret;
	
	//ENTER();
	preempt_disable();
	man = &ufb->uRpcie_man;
	ret = RemoteRegRead(ufb,DED_EP_PCIOUT,DED_EP_PCIIN,
						(PNC_PCI_OR_CSR_OUT_PACKET)PciOutPacket,
						data,
						man);
	preempt_enable();
	//LEAVE(ret);
	return ret;
}

#if 0
static int PciRegWrite_Ex(struct ufb_data *ufb,PNC_PCIOUT_PACKET PciOutPacket,int cnt){
	struct urb_man * man;
	int ret;
	
	ENTER();
	man = &ufb->uRpcie_man;
	ret = mutex_lock_interruptible(&man->io_mutex);
	if(ret < 0){
//		error("signal arrived when in lock state,error = %d",ret);
		LEAVE(ret);
	}

	ret = RemoteRegWrite_Ex(ufb,DED_EP_PCIOUT,
						(PNC_PCI_OR_CSR_OUT_PACKET)PciOutPacket,
						man,cnt);

	mutex_unlock(&man->io_mutex);
	LEAVE(ret);
}

#endif

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  PciRegWrite
 *  Description:  be aware ,flag should contain valid DWORD count of this transfer
 *  			  it should be at least 1.
 * =====================================================================================
 */
static int PciRegWrite(struct ufb_data *ufb,
						PNC_PCIOUT_PACKET PciOutPacket,
						uint32_t flag)
{
	struct urb_man * man;
	int ret,cnt;
	
	//ENTER();
	preempt_disable();
	ret = 0;
	man = &ufb->uRpcie_man;
	
	cnt = flag & 0xffff;
	cnt <<= 2;
	cnt += REG_WRITE_PCIOUT_SIZE - 4;
	
	flag &= 0xffff0000;
	flag |= cnt;

	ret = RemoteRegWrite(ufb,DED_EP_PCIOUT,
			PciOutPacket,
			man,
			flag);

	preempt_enable();
	//LEAVE(ret);
    return ret;
}

static int CsrWrite(struct ufb_data * ufb,PNC_CSROUT_PACKET outPack)
{
	struct urb_man * man;
	int ret;

	//ENTER();
	preempt_disable();
	man = &ufb->uCsr_man;

	ret = RemoteRegWrite(ufb,DED_EP_CSROUT,
						outPack,
						man,
						REG_WRITE_CSROUT_SIZE);

	preempt_enable();
	//LEAVE(ret);
    return ret;
}

static int CsrRead(struct ufb_data * ufb,PNC_CSROUT_PACKET outPack,void* data)
{
	struct urb_man * man;
	int ret;

	//ENTER();
	preempt_disable();
	man = &ufb->uCsr_man;
	
	/*
	 * below section are used for protecting code from several thread
	 * */


	ret = RemoteRegRead(ufb,DED_EP_CSROUT,DED_EP_CSRIN,
						(PNC_PCI_OR_CSR_OUT_PACKET)outPack,
						data,
						man);

	preempt_enable();
	//LEAVE(ret);
    return ret;
}

int CsrUsbc_ReadUlong(struct ufb_data * ufb,uint16_t address,void * data){
	/* read a usb3382 usb control register (Port3,BAR0)
	 * see RPCI header file for more details
	 * */	
	int ret;
	NC_CSROUT_PACKET outpack = {(1<<CSR_READ_WRITE) + MEMORY_MAPPED_CONFIG + 0xf};
	//ENTER();
	outpack.Address = address;
	ret = CsrRead(ufb,&outpack,data);
	//LEAVE(ret);
    return ret;
}

int CsrUsbc_WriteUlong(struct ufb_data * ufb,uint16_t address,uint32_t data){
	int ret;
	NC_CSROUT_PACKET outpack = {MEMORY_MAPPED_CONFIG + 0xf};
	//ENTER();
	outpack.Address = address;
	outpack.Data = data;
	ret = CsrWrite(ufb,&outpack);
	//LEAVE(ret);
	return ret;
}

int CsrConfig_ReadUlong(struct ufb_data * ufb,uint16_t address,void * data){
	int ret;
	NC_CSROUT_PACKET outpack = {(1<<CSR_READ_WRITE) + PCI_CONFIG + 0xf};
	//ENTER();
	outpack.Address = address;
	ret = CsrRead(ufb,&outpack,data);
	//LEAVE(ret);
	return ret;
}

int CsrConfig_WriteUlong(struct ufb_data * ufb,uint16_t address,uint32_t data){
	int ret;
	NC_CSROUT_PACKET outpack;
	//ENTER();
	outpack.SpaceSelByteEnables = PCI_CONFIG + 0xf;
	outpack.Address = address;
	outpack.Data = data;
	ret = CsrWrite(ufb,&outpack);
	//LEAVE(ret);
	return ret;
}



uint32_t PciConfig_ReadUlongEx(struct ufb_data * ufb,PREMOTE_CONFIGURATION_REGISTER config,uint16_t Register){
	uint32_t ret;
	NC_PCIOUT_PACKET PciOutPacket = 
	{
		(0x0f << PCIE_FIRST_BYTE_ENABLES)|
		(CFG_READ_OR_WRITE << PCIE_MASTER_COMMAND_SELECT)|
		(1<<PCIE_MASTER_START)|
		(1<<PCIE_MASTER_READ_WRITE)|
		(0<<PCIE_PORT)|
		(1<<PCIE_DW_LENGTH)|
		0
	};
	//ENTER();

	PciOutPacket.PciMstAddr = 
		(config->Bus<<0)|
		(config->Function<<8)|
		(config->Device<<11)|
		((Register & 0x00000f00)<<8) |
		(((config->Type)?1:0)<<24)|
		((Register & 0x000000fc)<<24)|
		0;

	PciRegRead(ufb,&PciOutPacket,&ret);
	//LEAVE(ret);	
	return ret;
}

void PciConfig_WriteUlongEx(struct ufb_data * ufb,
								PREMOTE_CONFIGURATION_REGISTER Config,
								uint16_t Register,
								uint32_t Value){
	NC_PCIOUT_PACKET PciOutPacket = 
	{
		(0X0F<<PCIE_FIRST_BYTE_ENABLES)|
		(CFG_READ_OR_WRITE<<PCIE_MASTER_COMMAND_SELECT)|
		(1<<PCIE_MASTER_START)|
		(0<<PCIE_MASTER_READ_WRITE)|
		(((Config->Port)?1:0)<<PCIE_PORT)|
		(1<<PCIE_DW_LENGTH)|
		0
	};
	//ENTER();

	PciOutPacket.PciMstAddr = 
		(Config->Bus<<0) |
		(Config->Function<<8)|
		(Config->Device<<11) |
		((Register & 0xf00)<<8)|
		(((Config->Type)?1:0)<<24)|
		((Register & 0xfc)<<24)|
		0;

	PciOutPacket.PciMstData = Value;
	PciRegWrite(ufb,&PciOutPacket,1);
	//LEAVE();
}

void PciMm_WriteUlong(struct ufb_data * ufb,
						uint32_t reg,
						uint32_t data)
{
	NC_PCIOUT_PACKET packet;
	int ret;
	//ENTER();
	packet.PciMstCtl = (15<<PCIE_FIRST_BYTE_ENABLES)|
						(MEM_READ_OR_WRITE<<PCIE_MASTER_COMMAND_SELECT)|
						(1<<PCIE_MASTER_START)|
						(0<<PCIE_MASTER_READ_WRITE)|
						(0<<PCIE_PORT)|
						(1<<PCIE_DW_LENGTH)|
						0;
	packet.PciMstAddr = reg;
	packet.PciMstData = data;
	ret = PciRegWrite(ufb,&packet,1);
	//LEAVE();
}

void PciMm_WriteUlong_full(struct ufb_data * ufb,
							uint32_t reg,
							uint32_t * src,
							uint32_t flag)
{
	uint32_t cnt;
	NC_PCIOUT_PACKET packet;
	//static char * sbigbuf = NULL;
	int quantity;
	//ENTER();

	
	if(ufb->sbigbuf == NULL){
		ufb->sbigbuf = ufb->buf_async;
	}	

	cnt = flag & 0xffff;
	if(!cnt || cnt > 32)
		cnt = 1;//in case cnt zero

	quantity = (cnt + 2) * 4;
	
	packet.PciMstCtl = (15<<PCIE_FIRST_BYTE_ENABLES)|
						(MEM_READ_OR_WRITE<<PCIE_MASTER_COMMAND_SELECT)|
						(1<<PCIE_MASTER_START)|
						(0<<PCIE_MASTER_READ_WRITE)|
						(0<<PCIE_PORT)|
						(cnt<<PCIE_DW_LENGTH)|
						0;
	packet.PciMstAddr = reg;

	if(flag & WRITE_PCIMM_ASYNC){
		if((BUF_ASYNC_SIZE - (uint32_t)(ufb->sbigbuf - ufb->buf_async)) <= quantity){
			ufb->sbigbuf = ufb->buf_async;
		}
		memcpy(ufb->sbigbuf,&packet,8);
		memcpy(ufb->sbigbuf+8,(const void *)src,cnt*4);
		PciRegWrite(ufb,(PNC_PCIOUT_PACKET)ufb->sbigbuf,flag);
		ufb->sbigbuf += quantity;
	}else{
		/* synchonouse case */
		char tmp[256];
		memcpy(tmp,&packet,8);
		memcpy(tmp+8,src,cnt*4);
		PciRegWrite(ufb,(PNC_PCIOUT_PACKET)tmp,flag);
	}
	//LEAVE();
}

uint32_t PciMm_ReadUlong(struct ufb_data * ufb,uint32_t reg ){
	uint32_t ret;
	NC_PCIOUT_PACKET packet;
	//ENTER();
	packet.PciMstCtl = (15<<PCIE_FIRST_BYTE_ENABLES)|
						(MEM_READ_OR_WRITE<<PCIE_MASTER_COMMAND_SELECT)|
						(1<<PCIE_MASTER_START)|
						(1<<PCIE_MASTER_READ_WRITE)|
						(0<<PCIE_PORT)|
						(1<<PCIE_DW_LENGTH)|
						0;
	packet.PciMstAddr = reg;
	PciRegRead(ufb,&packet,&ret);
	//LEAVE(ret);
	return ret;
}


#if 0
/* reg should be the dataport address */
int PciMm_WriteDma_dataport(struct ufb_data * ufb,
							uint32_t reg,
							uint32_t data)
{
#define DMA_MAX_CNT 0xffffff
#define DMA_MAX_TICK 400
	uint32_t dmactl,dmaoffset;
	int ret;
	static int tick = 0;
	static int cnt = DMA_MAX_CNT;
	static uint32_t sbuf[DMA_MAX_TICK];
	ret = 0;

	sbuf[tick++] = data;

	if(cnt == DMA_MAX_CNT){
		dmactl = (1 << DMA_ENABLE)|
//			(1 << DMA_OUT_AUTO_START_ENABLE)|
//			(1<< DMA_FIFO_VALIDATE)|			
			1;

		dmaoffset = DMACHANNELOFFSET(1);
		CsrUsbc_WriteUlong(ufb,dmaoffset+DMAADDR,reg);
		CsrUsbc_WriteUlong(ufb,dmaoffset+DMACOUNT,DMA_MAX_CNT);
		CsrUsbc_WriteUlong(ufb,dmaoffset+DMACTL,dmactl);	
		CsrUsbc_WriteUlong(ufb,dmaoffset+DMASTAT,1<<DMA_START);
		error("kick dma");
	}

	cnt -= 4;
	if(cnt < 4)
		cnt = DMA_MAX_CNT;

	if(tick == DMA_MAX_TICK){
		/* feed data to gpep1 */
		error("kick endpoint 2");
		ret = RemoteRegWrite(ufb,
				2,	
				sbuf,
				&ufb->uDma_man,
				DMA_MAX_TICK*4);
		tick = 0;
	}else{
		error("tick = %d",tick);
	}

	return ret;

}
#endif


void NC3380_flushDma(struct ufb_data * ufb)
{
	/* flush 3380 dma engine stat and buffer */
	int gpepIndex = 1;
	uint32_t epcfg;		
	CsrUsbc_ReadUlong(ufb,GPEPPAGEOFFSET(gpepIndex)+EP_CFG,&epcfg);
	CsrUsbc_WriteUlong(ufb,DMACHANNELOFFSET(gpepIndex)+DMASTAT,(1<<DMA_ABORT));
	CsrUsbc_WriteUlong(ufb,GPEPPAGEOFFSET(gpepIndex)+EP_STAT_OUT,
			(1<<FIFO_FLUSH));
	CsrUsbc_WriteUlong(ufb,GPEPPAGEOFFSET(gpepIndex)+EP_STAT_IN,
			(1<<FIFO_FLUSH));
}


int PciMm_DmaStage0(struct ufb_data * ufb,uint32_t reg,int cnt)
{
	uint32_t dmactl,dmaoffset;
	int ret;
	//ENTER();

	ret = 0;

	/* setting dma register */
	dmactl = (1 << DMA_ENABLE)|
#if 1
		6 << DMA_REQUESTS_OUTSTANDING| /* for usb3.0 performance*/
#endif
		//		(1 << DMA_OUT_AUTO_START_ENABLE)|
		//		(1 << DMA_FIFO_VALIDATE)|
		0;

	/* use gpep 1*/
	dmaoffset = DMACHANNELOFFSET(1);
	ret = CsrUsbc_WriteUlong(ufb,dmaoffset+DMAADDR,reg);
	if(ret < 0)
		//LEAVE(ret);
	    return ret;
	ret = CsrUsbc_WriteUlong(ufb,dmaoffset+DMACOUNT,cnt);
	if(ret < 0)
		//LEAVE(ret);
	    return ret;
	ret = CsrUsbc_WriteUlong(ufb,dmaoffset+DMACTL,dmactl);	
	if(ret < 0)
		//LEAVE(ret);
	    return ret;
	ret = CsrUsbc_WriteUlong(ufb,dmaoffset+DMASTAT,1<<DMA_START);	
	if(ret < 0)
		//LEAVE(ret);
	    return ret;
	//LEAVE(0);
	return 0;
}
//#define GET_INTERVAL
int PciMm_DmaStage1(struct ufb_data * ufb,void * src,int cnt, int cutbpp)
{
	int ret;
 #ifdef GET_INTERVAL
    struct timeval tv1,tv2;
    unsigned long deltv;
    int buflen;
    buflen  = cnt & 0xffffff;/* buffer length is contained in lower 3bytes of flag */
    do_gettimeofday(&tv1);
 #endif

    int buffer_length;
    int k;
    u16 * convert;
    u32 * point_src; 
    u32 src_bytes_pixel;
    
    convert = (u16 *)src;
    point_src = (u32 *)src;
    
    if(cutbpp)
    {
        //convert 32 bpp to 16 bpp
        buffer_length  = cnt & 0xffffff;/* buffer length is contained in lower 3bytes of flag */
        //dbg_msg("converting:%ld\n",buffer_length);
        for (k = 0; k < (buffer_length/4); k++)
        {
            src_bytes_pixel = point_src[k];
            convert[k] = (u16)((src_bytes_pixel & 0x000000F8) >> 3 | (src_bytes_pixel & 0x0000FC00) >> 5 | (src_bytes_pixel & 0x00F80000) >> 8);
        }                
        cnt = (cnt&0xff000000)|(buffer_length/2);
        //dbg_msg("convert done:%ld\n",cnt);
    }        
        
	/* feed data to gpep1 */
	ret = RemoteRegWrite(ufb,
			2,	
			(void*)convert,
			&ufb->uDma_man,
			cnt);
#ifdef GET_INTERVAL
    do_gettimeofday(&tv2);
    deltv = tv2.tv_usec - tv1.tv_usec;
    if(buflen>=(128*1024))
    {
        dbg_msg("The 1st time counter %ld\n",tv1.tv_usec);
        dbg_msg("The 2nd time counter %ld\n",tv2.tv_usec);
        dbg_msg("Transfer %ld bytes via DMA expend %ld microseconds\n",buflen,deltv);
        dbg_msg("DMA speed up to %ld Mbytes/s\n ", ((buflen/(deltv))*1000*1000)/(1024*1024));
    }
#endif
	return ret;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  PciMm_WriteDma
 *  Description:  use dma to transfer data to rpcie device
 *  			note:please make size below 4k
 *  			please assure that @src is physically contigniouse
 * =====================================================================================
 */
int PciMm_WriteDma(struct ufb_data * ufb,uint32_t reg,const void * src,int size,int flag)
{
	uint32_t dmactl,dmaoffset;
	int ret;
	ENTER();

	ret = 0;
//	NC3380_flushDma(ufb);

	/* setting dma register */
	dmactl = (1 << DMA_ENABLE)|
#if 1
		6 << DMA_REQUESTS_OUTSTANDING| /* for usb3.0 performance*/
#endif
		//		(1 << DMA_OUT_AUTO_START_ENABLE)|
		//		(1 << DMA_FIFO_VALIDATE)|
		0;

	if(flag & PLX_3380_CONSTANT_ADDR)
		dmactl|=1;

	/* use gpep 1*/
	dmaoffset = DMACHANNELOFFSET(1);
	ret = CsrUsbc_WriteUlong(ufb,dmaoffset+DMAADDR,reg);
	if(ret < 0)
		LEAVE(ret);
	ret = CsrUsbc_WriteUlong(ufb,dmaoffset+DMACOUNT,size);
	if(ret < 0)
		LEAVE(ret);
	ret = CsrUsbc_WriteUlong(ufb,dmaoffset+DMACTL,dmactl);	
	if(ret < 0)
		LEAVE(ret);
	ret = CsrUsbc_WriteUlong(ufb,dmaoffset+DMASTAT,1<<DMA_START);	
	if(ret < 0)
		LEAVE(ret);

	/* feed data to gpep1 */
	ret = RemoteRegWrite(ufb,
			2,	
			(void*)src,
			&ufb->uDma_man,
			size);
//	NC3380_flushDma(ufb);

	LEAVE(ret);
}


static int EnumeratePlxPcie(struct ufb_data * ufb){
	int ret;
	int time;
	ENTER();
	ret = 0;
	time = 0;

	while(time<1){
		CsrConfig_WriteUlong(
				ufb,
				PORT3_BASE + PCICMD,
				(1<<BUS_MASTER_ENABLE) |
				(1<<MEMORY_SPACE_ENABLE) |
				0);
		time++;
	}
#if 1
	CsrConfig_WriteUlong(
		ufb,
		PORT3_BASE + PCIBASE2,
		GPEP_FIFOS_PORT3_BAR2);

	CsrConfig_WriteUlong(
		ufb,
		PORT2_BASE + PCICMD,
		(1<<MEMORY_SPACE_ENABLE)|
		(1<<BUS_MASTER_ENABLE)|
		0);
 	
	CsrConfig_WriteUlong(
		ufb,
		PORT2_BASE + PRIMARYBUSNUMBER,
		(0x00<<0) |
		(PORT2_BUS << 8) |
		(PORT0_BUS << 16) |
		0);

	CsrConfig_WriteUlong(
		ufb,
		PORT0_BASE + PRIMARYBUSNUMBER,
		(PORT2_BUS << 0)|
		(PORT0_BUS << 8) |
		(PORT0_BUS << 16) |
		0);

	CsrConfig_WriteUlong(
		ufb,
		PORT0_BASE + PCICMD,
		(1<<MEMORY_SPACE_ENABLE)|
		(1<<BUS_MASTER_ENABLE)|
		0);

#endif
	LEAVE(ret);
}


static void	NcRpci_MapNcDma(struct ufb_data * ufb){
	int gpepIndex;

	for(gpepIndex = 0;gpepIndex<NUMBEROF_GPEPS;gpepIndex++){
		uint32_t epcfg;		
		CsrUsbc_ReadUlong(ufb,GPEPPAGEOFFSET(gpepIndex)+EP_CFG,&epcfg);
		CsrUsbc_WriteUlong(ufb,DMACHANNELOFFSET(gpepIndex)+DMASTAT,(1<<DMA_ABORT));
		CsrUsbc_WriteUlong(ufb,GPEPPAGEOFFSET(gpepIndex)+EP_STAT_OUT,
						(1<<FIFO_FLUSH));
		CsrUsbc_WriteUlong(ufb,GPEPPAGEOFFSET(gpepIndex)+EP_STAT_IN,
						(1<<FIFO_FLUSH));
		
	}
}

static void SetRcOutFifoRam(struct ufb_data * ufb,
							int gpep,
							uint32_t outfifosize,
							uint32_t outfifobase)
{
	uint32_t epfifosize;
	ENTER();
	CsrUsbc_ReadUlong(ufb,EP_FIFOSIZE + (GPEPPAGEOFFSET(gpep)),&epfifosize);
	epfifosize &= 0xffff0000;
	epfifosize |= (
			outfifosize|
			outfifobase|
			0);
	CsrUsbc_WriteUlong(ufb,EP_FIFOSIZE + (GPEPPAGEOFFSET(gpep)),epfifosize);


	LEAVE();
}

int ConstructDevice(struct ufb_data * ufb){
	uint32_t cp_cfg;
	int ret;
	REMOTE_CONFIGURATION_REGISTER Config;
	uint32_t reg;

	ENTER();
	ret = EnumeratePlxPcie(ufb);

	CsrUsbc_WriteUlong(ufb,BAR2CTL,(
		(1 << QUADRANT0_DIRECTION) |
		(0 << QUADRANT0_GPEP) |

		(0 << QUADRANT1_DIRECTION) |
		(1 << QUADRANT1_GPEP) |

		(1 << QUADRANT2_DIRECTION) |
		(2 << QUADRANT2_GPEP) |

		(0 << QUADRANT3_DIRECTION) |
		(3 << QUADRANT3_GPEP) |

		(0 - (4*ADDRESS_RANGE_PER_USB_FIFO)) |
		0));

#if 1
	SetRcOutFifoRam(ufb,0,FIFO_SIZE_256,512);
	SetRcOutFifoRam(ufb,1,FIFO_SIZE_4096,512+256);
	SetRcOutFifoRam(ufb,2,FIFO_SIZE_256,512+256+4096);
	SetRcOutFifoRam(ufb,3,FIFO_SIZE_4096,512+256+4096+256);
#else
	SetRcOutFifoRam(ufb,0,FIFO_SIZE_256,512);
	SetRcOutFifoRam(ufb,1,FIFO_SIZE_4096,512+256);
	SetRcOutFifoRam(ufb,2,FIFO_SIZE_256,512+256+4096);
	SetRcOutFifoRam(ufb,3,FIFO_SIZE_4096,512+256+4096+256);
#endif

	NcRpci_MapNcDma(ufb);

	Config.Bus = PORT0_BUS;
	Config.Device = 0;
	Config.Function = 0;
	Config.Port = 0;
	Config.Type = 0;

	reg = PciConfig_ReadUlongEx(ufb,&Config,PCICMD);
	PciConfig_WriteUlongEx(ufb,&Config,PCICMD,
		(reg)|
		(1<<BUS_MASTER_ENABLE)|
		(1<<MEMORY_SPACE_ENABLE)|
		0);		

	/* setting smi pcie device*/	
	PciConfig_WriteUlongEx(ufb,&Config,PCIBASE0,SMI_VMEM_BAR0);
	PciConfig_WriteUlongEx(ufb,&Config,PCIBASE1,SMI_MMIO_BAR1);

#if 1
	CsrUsbc_WriteUlong(ufb,EP_RSP_OUT + GPEPPAGEOFFSET(1),
						1<<CLEAR_NAK_OUT_PACKETS_MODE|
						0);

#endif
//	inf_msg("mmio_0x54 is %08x\n",SMI_READ_MMIO(ufb,0x54));

	CsrUsbc_ReadUlong(ufb,0x320,&cp_cfg);
//	inf_msg("gpep0,%08x\n",cp_cfg);
	CsrUsbc_ReadUlong(ufb,0x340,&cp_cfg);
#if 1
	/* set max burst size to 3,according to PLX for usb3.0 performance*/
	cp_cfg &= 0xf0ffffff;
	cp_cfg |= 0x03000000;
	CsrUsbc_WriteUlong(ufb,0x340,cp_cfg);
#endif
	CsrUsbc_ReadUlong(ufb,0x340,&cp_cfg);
//	inf_msg("gpep1,%08x\n",cp_cfg);
	CsrUsbc_ReadUlong(ufb,0x360,&cp_cfg);
//	inf_msg("gpep2,%08x\n",cp_cfg);
	CsrUsbc_ReadUlong(ufb,0x380,&cp_cfg);
//	inf_msg("gpep3,%08x\n",cp_cfg);

	LEAVE(ret);
}
#ifdef DEBUG_TMP
#undef DEBUG
#define DEBUG DEBUG_TMP
#undef DEBUG_TMP
#endif

