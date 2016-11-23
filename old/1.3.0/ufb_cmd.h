#ifndef  UFB_CMD_INC
#define  UFB_CMD_INC

/* kernel fbmem use 0x46?? as the cmd number,we use 0x55?? */
#define UFB_CMD_MODE 0x5500
#define UFB_CMD_UPDATE 0x5503
#define UFB_CMD_UPDATE_BLOCK 0x5513
#define UFB_CMD_UPDATESUB_BLOCK 0x5523
#define UFB_CMD_TRANSF 0x5506
#define UFB_CMD_PEEK 0x5504
#define UFB_CMD_VM_PEEK 0x5512
#define UFB_CMD_POKE 0x5505
#define UFB_CMD_VIDEO 0x5508
#define UFB_CMD_KERNEL_TO_VM 0x5510
#define UFB_CMD_CONVERT_32_16 0x5509
#define UFB_CMD_EDID_GET 0X5511
struct _region
{
	int x1;
	int y1;
	int x2;
	int y2;
	int bypass;
};

struct regionlist
{
	struct _region * boxes;
	int cnt;
	struct _region boundBox;
};

struct cmd_io_setmode
{
	char * modestr;
	int length;
};

struct x_io_reg_parameter
{
	int offset;
	uint32_t value;
};

struct x_io_transf{
	int offset1;/* offset from buf_vfb */
	int offset2;/* offset from video memory start address of remot usb-side device */
	int total;
};

struct x_io_damageArea{
	int x;
	int y;
	int width;
	int height;
	int base;
	int pitch;
	int bpp;
	int actualRect[4];/*x,y,w,h*/
	int csc_playing;
	uint32_t colorKey;
	int csc_update;
	int gui_update;
};

struct x_io_csc{
	/* registers(from DCR_c8 to DCR_f8) value need set */
	uint32_t value[13];
	uint32_t control;
};

struct csc_regs{
	uint32_t value[14];	
};

struct csc_opt{
	int nOpt;
	struct csc_regs * pRegs;
};

struct ovl_opt{

};

struct mvYuv_opt{
	uint32_t offset_sm;
	uint32_t offset_vm;
	int count;
};

struct x_io_video_opt{
	struct mvYuv_opt yuv;
	struct csc_opt csc;
	struct ovl_opt ovl;
	int overlay;/* none-zero means use overlay video */
};


#if 0
struct x_io_modeParm
{
    /* Horizontal timing. */
    unsigned long horizontal_total;
    unsigned long horizontal_display_end;
    unsigned long horizontal_sync_start;
    unsigned long horizontal_sync_width;
  	int horizontal_sync_polarity;

    /* Vertical timing. */
    unsigned long vertical_total;
    unsigned long vertical_display_end;
    unsigned long vertical_sync_start;
    unsigned long vertical_sync_height;
    int vertical_sync_polarity;

    /* Refresh timing. */
    unsigned long pixel_clock;
    unsigned long horizontal_frequency;
    unsigned long vertical_frequency;
    
    /* Clock Phase. This clock phase only applies to Panel. */
    int clock_phase_polarity;

	/* misc */
	int bpp;
	int pitch;
	int base;
};

#endif
#endif   /* ----- #ifndef UFB_CMD_INC  ----- */
