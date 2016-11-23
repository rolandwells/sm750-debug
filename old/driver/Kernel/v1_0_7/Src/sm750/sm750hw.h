#ifndef SM750HW_H__
#define SM750HW_H__


#if 0
#define DEFAULT_CHIP_CLK	336
#define DEFAULT_MEMORY_CLK	DEFAULT_CHIP_CLK/1
#define	DEFAULT_MASTER_CLK	DEFAULT_CHIP_CLK/3
#endif

#ifdef VALIDATION_CHIP
	#define DEFAULT_CHIP_CLK                336
#else
	#define DEFAULT_CHIP_CLK                290
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

unsigned int smread32(unsigned long offset);
unsigned char smread8(unsigned long offset);
void smwrite32(unsigned long offset,unsigned int value);
void smwrite8(unsigned long offset,unsigned char value);

#define sm750fb_hz_to_ps(x) sm750fb_ps_to_hz(x)
//static unsigned long sm750fb_ps_to_hz(unsigned long psvalue);

/* release resource get by kzalloc or kalloc */
void hw_release(void);
/* this routine did not need @par because par is not prepared when it be called */
unsigned long hw_get_memsize(void);
int hw_init_chip(struct sm750fb_state*);
int hw_alloc_mem( struct sm750fb_par *,int ,enum sm750_allocmem_reason);
int hw_set_mode(const struct sm750fb_par * par,const struct fb_var_screeninfo *var,const struct fb_fix_screeninfo * fix);
int hw_dump_reg(const struct sm750fb_par *,ushort);
int hw_set_colreg(const struct sm750fb_par *,int,ushort,ushort,ushort);
int hw_pan_display(const struct sm750fb_par *,const struct fb_var_screeninfo*);
void hw_2d_fillrect(const struct sm750fb_par * par,
			ulong base,ulong pitch,ulong bpp,
			ulong dx,ulong dy,ulong width,ulong height,
			ulong color,ulong rop2);
void hw_2d_copyarea(const struct sm750fb_par * par,
			ulong base,ulong pitch,ulong bpp,
			ulong dx,ulong dy,ulong sx,ulong sy,
			ulong width,ulong height);
void hw_2d_imageblit_mono(const struct sm750fb_par * par,
			ulong base,ulong pitch,ulong bpp,
			ulong dx,ulong dy,
			ulong width,ulong height,
			const unsigned char * data,
			ulong fgcol,ulong bgcol);
void hw_cursor_enable(const struct sm750fb_par * par,int on);
void hw_cursor_clear(const struct sm750fb_par * par);
void hw_cursor_setsize(const int w,const int h);
int hw_cursor_setpos(const struct sm750fb_par * par,ulong dx,ulong dy);
int hw_cursor_setcolor(const struct sm750fb_par * par,u32 bg,u32 fg);
int hw_cursor_setdata(const struct sm750fb_par * par,const struct fb_cursor *cursor,const u8 *pcol,const u8 *pmsk);
int hw_2d_wait_idle(const struct sm750fb_par * par);
int hw_set_blank(const struct sm750fb_par *par,int flag);
//void hw_wait_vsync(const struct sm750fb_par * par,int count);
#ifdef CONFIG_PM
void hw_suspend(const struct sm750fb_par * par);
#endif
#endif
