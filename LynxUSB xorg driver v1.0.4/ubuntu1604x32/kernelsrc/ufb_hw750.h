#ifndef SM750HW_H__
#define SM750HW_H__
#include 	"usbfb.h"
#include 	"ddk/ddk750_type.h"
#if 0
#define DEFAULT_CHIP_CLK	336
#define DEFAULT_MEMORY_CLK	DEFAULT_CHIP_CLK/1
#define	DEFAULT_MASTER_CLK	DEFAULT_CHIP_CLK/3
#endif

#ifdef VALIDATION_CHIP
	#define DEFAULT_CHIP_CLK                336
#else
	#define DEFAULT_CHIP_CLK                336
#endif

#ifdef VALIDATION_CHIP
    /* Memory Clock Default Value. */
    #define DEFAULT_MEMORY_CLK          165
    
    /* Master Clock Default Value. */
    #define DEFAULT_MASTER_CLK          112
#else
    /* Memory Clock Default Value. It can be calculated by dividing 
       the chip clock by certain valid values, which are: 1, 2, 3, and 4 */
    #define DEFAULT_MEMORY_CLK          (DEFAULT_CHIP_CLK/1)
    
    /* Master Clock Default Value. It can be calculated by dividing
       the chip clock by certain valid values, which are: 3, 4, 6, and 8 */
    #define DEFAULT_MASTER_CLK          (DEFAULT_CHIP_CLK/3)
#endif

#define DUMPREG_HEAD			0x1<<0
#define DUMPREG_PANEL			0x1<<1
#define DUMPREG_CRT				0x1<<2
#define DUMPREG_PANEL_CURSOR	0x1<<3
#define DUMPREG_CRT_CURSOR		0x1<<4
#define DUMPREG_ZVPORT			0x1<<5

#define DUMPREG_ALL	( DUMPREG_HEAD |\
			DUMPREG_PANEL |\
			DUMPREG_CRT |\
			DUMPREG_PANEL_CURSOR|\
			DUMPREG_CRT_CURSOR|\
			DUMPREG_ZVPORT|\
			0)
			
#define MAX_CURS_W	64
#define MAX_CURS_H 	64
#define MAX_CURS_SIZE	(MAX_CURS_W * MAX_CURS_H * 2 / 8)
#define HW_ROP2_COPY	0xC
#define HW_ROP2_XOR	0x6


#define sm750fb_hz_to_ps(x) sm750fb_ps_to_hz(x)
//#include 	"ddk/ddk750_type.h"
uint32_t peek32(struct ufb_data * ufb,uint32_t offset);
void poke32(struct ufb_data * ufb,uint32_t offset,uint32_t value);
unsigned char peek8(struct ufb_data * ufb,uint32_t offset);
void poke8(struct ufb_data * ufb,uint32_t offset, unsigned char value);

//void hw_set_ufb(struct ufb_data * ufb);

/* release resource get by kzalloc or kalloc */
void hw_release(void);
/* this routine did not need @par because par is not prepared when it be called */
int hw_get_memsize(struct ufb_data *);
int hw_init_chip(struct ufb_data *,struct sm750fb_state*);
int hw_set_mode(const struct sm750fb_par * par,const struct fb_var_screeninfo *var,const struct fb_fix_screeninfo * fix);
int hw_dump_reg(const struct sm750fb_par *,ushort);
int hw_set_colreg(const struct sm750fb_par *,int,ushort,ushort,ushort);
int hw_pan_display(const struct sm750fb_par *,const struct fb_var_screeninfo*);
int hw_2d_fillrect(const struct sm750fb_par * par,
			uint32_t base,uint32_t pitch,uint32_t bpp,
			uint32_t dx,uint32_t dy,uint32_t width,uint32_t height,
			uint32_t color,uint32_t rop2);
void hw_2d_copyarea(const struct sm750fb_par * par,
			uint32_t base,uint32_t pitch,uint32_t bpp,
			uint32_t dx,uint32_t dy,uint32_t sx,uint32_t sy,
			uint32_t width,uint32_t height);
int hw_2d_imageblit_mono(const struct sm750fb_par * par,
			uint32_t base,uint32_t pitch,uint32_t bpp,
			uint32_t dx,uint32_t dy,
			uint32_t width,uint32_t height,
			const unsigned char * data,
			uint32_t fgcol,uint32_t bgcol);
void hw_cursor_enable(const struct sm750fb_par * par,int on);
void hw_cursor_clear(const struct sm750fb_par * par);
void hw_cursor_setsize(const int w,const int h);
int hw_cursor_setpos(const struct sm750fb_par * par,uint32_t dx,uint32_t dy);
int hw_cursor_setcolor(const struct sm750fb_par * par,uint32_t bg,uint32_t fg);
int hw_cursor_setdata(const struct sm750fb_par * par,const struct fb_cursor *cursor,const u8 *pcol,const u8 *pmsk);
int hw_2d_wait_idle(const struct sm750fb_par * par);
int hw_set_blank(const struct sm750fb_par *par,int flag);
int hw_tst_dma(int size,int loop);
int hw_update_damage(struct fb_info * info,
		int dx,int dy,
		int width,int height);

int ufb_io_peek(struct ufb_data * ufb,void __user * value);
int ufb_vm_peek(struct ufb_data * ufb,void __user * value);
int ufb_io_poke(struct ufb_data * ufb,void __user * value);
//int ufb_io_setMode(struct ufb_data *,const void * );
//int hw_usb_update_rect(struct fb_info * info,int x,int y,int w,int h,int bpp,int pitch);
int ufb_io_update_async(struct fb_info * info,void * arg);
int ufb_io_update(struct fb_info *,void*);
int ufb_io_updatesub(struct fb_info * info,void * arg);
int ufb_io_transfer(struct fb_info * info,void * arg);
int ufb_io_video(struct fb_info * info,void * arg);
void hw_fill_2d_cache(struct ufb_data *);
int hw_output0_connected(struct ufb_data*);
int hw_output1_connected(struct ufb_data*);
void tst_hdmi(void * ufb,int d);
int hw_usb_update_rect_repack(struct fb_info * info,
		int x,int y,
		int w,int h,
		int bpp,int pitch);
int ufb_io_kernel_to_vm(struct fb_info * info,void * arg);
int hw_light_hdmi(struct ufb_data * ufb,mode_parameter_t * modeparm);
unsigned long sm750fb_ps_to_hz(unsigned long psvalue);
//void hw_wait_vsync(const struct sm750fb_par * par,int count);
#endif
