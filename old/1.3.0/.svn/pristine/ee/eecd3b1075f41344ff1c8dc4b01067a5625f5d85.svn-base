#ifndef  USBFB_INC
#define  USBFB_INC

#define TRUE 1
#define FALSE 0

#include "ufb_cmd.h"
#define REG_CNT_SM750_2D	(((0x100050 - 0x100000)>>2) + 1)
#define REG_CNT_SM750_CSC   (((0x1000fc - 0x1000c8)>>2) + 1)
#define UFB_STAT_PLUGOUT 0xff
#define UFB_MONITOR_CHECK_INTERVAL 1000

/* urb_man designed to manage synchronouse urb transmite */
struct urb_man{
	bool ongoing;/* none-zero means no urb transferring on this urb */
	spinlock_t err_lock;
	struct mutex io_mutex;/* used for preventing multi-thread accessing */
	struct completion completion; /* used for wakeup the waiting thread */
	void * priv;
	int error;
};


/* notice: all register access with below struct should be garunteed not 
 * a volatile one, (e.g. 2d status register) */
struct reg_cache_attr{
	uint32_t ignoreBits;/* any bit 1 in mask means this bit is reserved and the value should be ignored */
	uint32_t lastValue;/* current value of this register,first time read from hardware,do & with ~ignore */
};

enum digital_output{
	output_hdmi = 0,
	output_dvid,
	output_analog,
};

struct Xdraw_task{
	struct x_io_damageArea area;
	struct work_struct work;
	void * priv;//point to fb_info
};

struct Xdraw_buffer{
	int ptr_w;
	int ptr_r;
	struct Xdraw_task task[16];
};

typedef struct {
    unsigned char displayPath;
    unsigned char *pEDIDBuffer;
    unsigned long bufferSize;
    unsigned char edidExtNo;
}edid_data,*edid_data_p;

struct ufb_data {
	struct usb_device * udev;
	struct device * gdev; /* &udev->dev */
	struct usb_interface * interface;
	struct kref kref;
	/* driver need two urb struct */
	struct urb * urbs[2];
	/* communication members*/
	struct urb_man uCsr_man;
	struct urb_man uRpcie_man;
	struct urb_man uDma_man;
	/* sm750 2d register cache */
	struct reg_cache_attr reg2d[REG_CNT_SM750_2D];
	/* monitor connection status detection flags */
	struct workqueue_struct * wq_monitor;  
	struct delayed_work w_monitor;
	uint32_t flag_monitor;
	int outputs;
	enum digital_output output0;

	/* X drawing structures */
	struct workqueue_struct * wq_Xdraw;
	struct Xdraw_buffer buf_Xdraw;
	spinlock_t sl_Xdraw;
	
	/* this section of memory located in system memory
	 * and it represent the total video memory (mirror) of the remote
	 * side of USB,it should be exactly the same value with the video memory
	 * of remote side device 
	 * we prefer the size of buf_vfb not below 16 mega bytes,for sake of dual view
	 * application 
	 * */
	char * buf_vfb;
	/* temporary used buffer and physically continouse */
	char * buf_dma;
	/* asynchronouse transfer buffer that contains URB struct and tranferring data */
	char * buf_async;
	char * sbigbuf;
	/* from this offset,video memory is not seen by OS,they are used privately
	 * and the size of this section video memory is 8 mega bytes
	 * */
	int priv_vm_offset;
	/* total memory size */
	int memsize;	
	/* actual memory size */
	int act_size;
	/* */
	struct mutex io_mutex;	
	struct mutex dma_mutex;
	struct fb_info * fbinfo[2];
	atomic_t fbcount;
	int status;
};


struct boxRect {
	uint16_t x1;
	uint16_t y1;
	uint16_t x2;
	uint16_t y2;
};

#define PFX "[ufb] "
#define war_msg(fmt,args...) printk(KERN_WARNING PFX fmt, ## args)
#define inf_msg(fmt,args...) printk(KERN_INFO PFX fmt, ## args)
#define error(fmt,args...)	inf_msg(" error,%s: "fmt "\n",__func__, ## args)
//#define DEBUG 2
#undef DEBUG
#if (DEBUG == 1)
/* debug level == 1 */
#warning "debug=1 build"
#define dbg_msg(fmt,args...) printk(KERN_DEBUG PFX fmt, ## args)
#define ENTER()	printk(KERN_DEBUG PFX "%*c %s\n",smi_indent++,'>',__func__)
#define LEAVE(...)	\
	do{				\
	printk(KERN_DEBUG PFX "%*c %s\n",--smi_indent,'<',__func__); \
	return __VA_ARGS__; \
	}while(0)

#elif (DEBUG == 2)
/* debug level == 2*/
#warning "debug=2 build"
#define dbg_msg(fmt,args...) printk(KERN_ERR PFX fmt, ## args)
#define ENTER()	printk(KERN_ERR PFX "%*c %s\n",smi_indent++,'>',__func__)

#define LEAVE(...)	\
	do{				\
	printk(KERN_ERR PFX "%*c %s\n",--smi_indent,'<',__func__); \
	return __VA_ARGS__; \
	}while(0)

#ifdef inf_msg
#undef inf_msg

#endif
#define inf_msg(fmt,args...) printk(KERN_ERR PFX fmt, ## args)
#else
/* no debug */
//#warning "no debug build"
#define dbg_msg(...)
#define ENTER()
#define LEAVE(...) return __VA_ARGS__

#endif

#define USB_WAIT_JIFF 100 /* 100 jiff equal 0.4 second*/


#if (RIGOROUS == 2)
	/* implment me */

#elif (RIGOROUS == 1)
	/* use spin lock to protect data */
#define GET_URB_TRANSFER_STATE(var,man) \
	do{ \
	struct urb_man * pman; \
	pman = (man); \
	spin_lock(&pman->err_lock); \
	var = pman->ongoing; \
	spin_unlock(&pman->err_lock); \
	}while(0);

#define SET_URB_TRANSFER_STATE(var,man) \
	do{ \
	struct urb_man * pman; \
	pman = (man); \
	spin_lock(&pman->err_lock); \
	pman->ongoing = (var); \
	spin_unlock(&pman->err_lock); \
	}while(0);
#else
	/* no protect,cuz we only get or set one var*/
#define GET_URB_TRANSFER_STATE(var,man) \
	do{ \
		struct urb_man * pman; \
		pman = (man); \
		var = pman->ongoing;\
	}while(0);


#define SET_URB_TRANSFER_STATE(var,man) \
	do{ \
	struct urb_man * pman; \
	pman = (man); \
	pman->ongoing = (var); \
	}while(0);
	
#endif


#define GET_URB_TRANSFER_STATE_IRQ(var,man,flags) \
	do{ \
	struct urb_man * pman; \
	pman = (man); \
	spin_lock_irqsave(pman->err_lock,flags); \
	var = pman->ongoing; \
	spin_unlock_irqrestore(pman->err_lock,flags); \
	}while(0);

#define SET_URB_TRANSFER_STATE_IRQ(var,man,flags) \
	do{ \
	struct urb_man * pman; \
	pman = (man); \
	spin_lock_irqsave(pman->err_lock,flags); \
	man->ongoing = (var); \
	spin_unlock_irqrestore(pman->err_lock,flags); \
	}while(0);
	
#define SMI_VMEM_BAR0 ((PORT0_BUS<<28)|(0<<24)|(0<<20)) //BUS,DEVICE,BAR
#define SMI_MMIO_BAR1 ((PORT0_BUS<<28)|(64<<20)) //VIDEO MEM TAKES 64MB

#define SMI_READ_MMIO(ufb,Reg) PciMm_ReadUlong((ufb),(Reg)+SMI_MMIO_BAR1)


#define SMI_WRITE_MMIO(ufb,Reg,Val) PciMm_WriteUlong((ufb),(Reg)+SMI_MMIO_BAR1,(Val))

#define SMI_WRITE_VMEM(ufb,Reg,Val) PciMm_WriteUlong((ufb),(Reg)+SMI_VMEM_BAR0,(Val))
#define SMI_READ_VMEM(ufb,Reg) PciMm_ReadUlong((ufb),(Reg)+SMI_VMEM_BAR0)

#define SMI_WRITE_VMEM_MULTI(ufb,reg,val,flag) PciMm_WriteUlong_full((ufb),(reg)+SMI_VMEM_BAR0,(val),(flag))

#define MAX_DWORD 32

extern int smi_indent;
#endif   /* ----- #ifndef USBFB_INC  ----- */
