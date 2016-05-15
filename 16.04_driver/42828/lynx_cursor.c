#include<linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
#include<linux/config.h>
#endif
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/errno.h>
#include<linux/string.h>
#include<linux/mm.h>
#include<linux/slab.h>
#include<linux/delay.h>
#include<linux/fb.h>
#include<linux/ioport.h>
#include<linux/init.h>
#include<linux/pci.h>
#include<linux/vmalloc.h>
#include<linux/pagemap.h>
#include <linux/console.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
/* no below two header files in 2.6.9 */
#include<linux/platform_device.h>
#include<linux/screen_info.h>
#else
/* nothing by far */
#endif

#include "lynx_drv.h"
#include "lynx_help.h"
#include "lynx_cursor.h"
#include "ddk712/ddk712_help.h"

#define PEEK32(addr) \
readl(cursor->mmio + (addr))

#define POKE32(addr,data) \
writel((data),cursor->mmio + (addr))

/* cursor control for voyager and 718/750*/
#define HWC_ADDRESS                         0x0
#define HWC_ADDRESS_ENABLE                  31:31
#define HWC_ADDRESS_ENABLE_DISABLE          0
#define HWC_ADDRESS_ENABLE_ENABLE           1
#define HWC_ADDRESS_EXT                     27:27
#define HWC_ADDRESS_EXT_LOCAL               0
#define HWC_ADDRESS_EXT_EXTERNAL            1
#define HWC_ADDRESS_CS                      26:26
#define HWC_ADDRESS_CS_0                    0
#define HWC_ADDRESS_CS_1                    1
#define HWC_ADDRESS_ADDRESS                 25:0

#define HWC_LOCATION                        0x4
#define HWC_LOCATION_TOP                    27:27
#define HWC_LOCATION_TOP_INSIDE             0
#define HWC_LOCATION_TOP_OUTSIDE            1
#define HWC_LOCATION_Y                      26:16
#define HWC_LOCATION_LEFT                   11:11
#define HWC_LOCATION_LEFT_INSIDE            0
#define HWC_LOCATION_LEFT_OUTSIDE           1
#define HWC_LOCATION_X                      10:0

#define HWC_COLOR_12                        0x8
#define HWC_COLOR_12_2_RGB565               31:16
#define HWC_COLOR_12_1_RGB565               15:0

#define HWC_COLOR_3                         0xC
#define HWC_COLOR_3_RGB565                  15:0

void hw712_cursor_enable(struct lynx_cursor * cursor)
{
	/* 712/722 offset is not in unit of byte,but in unit of 2kByte */
	poke_phr(0x80,cursor->offset & 0xff);
	poke_phr(0x81,~0x80 & (cursor->offset >> 8));
	poke_pop(0x82,0x80);
}

void hw712_cursor_disable(struct lynx_cursor * cursor)
{
	poke_hcr(0x82, 0);
}

void hw712_cursor_setSize(struct lynx_cursor * cursor,
					int w,int h)
{
	ENTER();
	cursor->w = w;
	cursor->h = h;
}

void hw712_cursor_setPos(struct lynx_cursor * cursor,
						int x,int y)
{
	poke_pop(0x90,(x & 0xff));
	poke_pop(0x91,(x >> 8)&0x7);
	poke_hcr(0x92, (y & 0xff));
	poke_hcr(0x93, (y >> 8)&0x7);
}

void hw712_cursor_setColor(struct lynx_cursor * cursor,
						u32 fg,u32 bg)
{
	char fg8 = 0;
	char bg8 = 0;
	ENTER();
#if 0
	fg8 = ((fg&0xc000)>>8)|((fg & 0x600)>>6) |((fg & 0x18)>>3);
	bg8 = ((bg&0xc000)>>8)|((bg & 0x600)>>6) |((bg & 0x18)>>3);
#else
	int r,g,b;
	r = (fg >> 11)&0x1f;
	g = (fg >> 5)&0x3f;
	b = fg & 0x1f;
	r = (r-2)*8/32;
	g = (g-4)*8/64;
	b = (b)*4/32;
	fg8 = (r<<5)|(g<<2)|b;

	r = (bg >> 11)&0x1f;
	g = (bg >> 5)&0x3f;
	b = bg & 0x1f;
	r = (r-2)*8/32;
	g = (g-4)*8/64;
	b = (b)*4/32;
	bg8 = (r<<5)|(g<<2)|b;
#endif

	poke_pop(0x84,fg8);
	poke_pop(0x86,bg8);
	LEAVE();
}


void hw712_cursor_setData(struct lynx_cursor * cursor,
			u16 rop,const u8* pcol,const u8* pmsk)
{
	int i,j,count,pitch,offset;
	u8 color,mask,opr;
	u16 data;
	u16 * pbuffer,*pstart;

	/*  in byte*/
	pitch = cursor->w >> 3;

	/* in byte	*/
	count = pitch * cursor->h;

	/* in ushort */
	offset = cursor->maxW * 2 / 8 / 2;
	data = 0;
	pstart = (u16 *)cursor->vstart;
	pbuffer = pstart;

	for(i=0;i<count;i++)
	{
		color = *pcol++;
		mask = *pmsk++;
		data = 0;

		/* either method below works well,
		 * but method 2 shows no lag
		 * and method 1 seems a bit wrong*/
		for(j=0;j<8;j++)
		{
			if(rop == ROP_XOR)
				opr = mask ^ color;
			else
				opr = mask & color;

			/* 2 stands for forecolor and 1 for backcolor */
			data |= ((opr & (0x80>>j))?0x4000:0xc000)>>(j*2);
		}

		data = ((data&0x00ff)<<8)|((data&0xff00)>>8);
		*pbuffer = data;

		/* assume pitch is 1,2,4,8,...*/
		if((i+1) % pitch == 0)/* below line equal to is line */
		{
			/* need a return */
			pstart += offset;
			pbuffer = pstart;
		}else{
			pbuffer++;
		}
	}
	return 0;
}

/* hw_cursor_xxx works for voyager,718 and 750 */
void hw_cursor_enable(struct lynx_cursor * cursor)
{
	u32 reg;
	reg = FIELD_VALUE(0,HWC_ADDRESS,ADDRESS,cursor->offset)|
			FIELD_SET(0,HWC_ADDRESS,EXT,LOCAL)|
			FIELD_SET(0,HWC_ADDRESS,ENABLE,ENABLE);
	POKE32(HWC_ADDRESS,reg);
}
void hw_cursor_disable(struct lynx_cursor * cursor)
{
	POKE32(HWC_ADDRESS,0);
}

void hw_cursor_setSize(struct lynx_cursor * cursor,
						int w,int h)
{
	cursor->w = w;
	cursor->h = h;
}
void hw_cursor_setPos(struct lynx_cursor * cursor,
						int x,int y)
{
	u32 reg;
	reg = FIELD_VALUE(0,HWC_LOCATION,Y,y)|
			FIELD_VALUE(0,HWC_LOCATION,X,x);
	POKE32(HWC_LOCATION,reg);
}
void hw_cursor_setColor(struct lynx_cursor * cursor,
						u32 fg,u32 bg)
{
	POKE32(HWC_COLOR_12,(fg<<16)|(bg&0xffff));
	POKE32(HWC_COLOR_3,0xffe0);
}

void hw_cursor_setData(struct lynx_cursor * cursor,
			u16 rop,const u8* pcol,const u8* pmsk)
{
	int i,j,count,pitch,offset;
	u8 color,mask,opr;
	u16 data;
	u16 * pbuffer,*pstart;
	static ulong odd = 0;

	/*  in byte*/
	pitch = cursor->w >> 3;

	/* in byte	*/
	count = pitch * cursor->h;

	/* in ushort */
	offset = cursor->maxW * 2 / 8 / 2;

	data = 0;
	pstart = (u16 *)cursor->vstart;
	pbuffer = pstart;

/*
	if(odd &1){
		hw_cursor_setData2(cursor,rop,pcol,pmsk);
	}
	odd++;
	if(odd > 0xfffffff0)
		odd=0;
*/

	for(i=0;i<count;i++)
	{
		color = *pcol++;
		mask = *pmsk++;
		data = 0;

		/* either method below works well,
		 * but method 2 shows no lag
		 * and method 1 seems a bit wrong*/
#if 0
		if(rop == ROP_XOR)
			opr = mask ^ color;
		else
			opr = mask & color;

		for(j=0;j<8;j++)
		{

			if(opr & (0x80 >> j))
			{	//use fg color,id = 2
				data |= 2 << (j*2);
			}else{
				//use bg color,id = 1
				data |= 1 << (j*2);
			}
		}
#else
		for(j=0;j<8;j++){
			if(mask & (0x80>>j)){
				if(rop == ROP_XOR)
					opr = mask ^ color;
				else
					opr = mask & color;

				/* 2 stands for forecolor and 1 for backcolor */
				data |= ((opr & (0x80>>j))?2:1)<<(j*2);
			}
		}
#endif
		*pbuffer = data;

		/* assume pitch is 1,2,4,8,...*/
#if 0
		if(!((i+1)&(pitch-1)))   /* below line equal to is line */
#else
		if((i+1) % pitch == 0)
#endif
		{
			/* need a return */
			pstart += offset;
			pbuffer = pstart;
		}else{
			pbuffer++;
		}

	}


}


void hw_cursor_setData2(struct lynx_cursor * cursor,
			u16 rop,const u8* pcol,const u8* pmsk)
{
	int i,j,count,pitch,offset;
	u8 color,mask,opr;
	u16 data;
	u16 * pbuffer,*pstart;

	/*  in byte*/
	pitch = cursor->w >> 3;

	/* in byte	*/
	count = pitch * cursor->h;

	/* in ushort */
	offset = cursor->maxW * 2 / 8 / 2;

	data = 0;
	pstart = (u16 *)cursor->vstart;
	pbuffer = pstart;

	for(i=0;i<count;i++)
	{
		color = *pcol++;
		mask = *pmsk++;
		data = 0;

		/* either method below works well, but method 2 shows no lag */
#if 0
		if(rop == ROP_XOR)
			opr = mask ^ color;
		else
			opr = mask & color;

		for(j=0;j<8;j++)
		{

			if(opr & (0x80 >> j))
			{	//use fg color,id = 2
				data |= 2 << (j*2);
			}else{
				//use bg color,id = 1
				data |= 1 << (j*2);
			}
		}
#else
		for(j=0;j<8;j++){
			if(mask & (1<<j))
				data |= ((color & (1<<j))?1:2)<<(j*2);
		}
#endif
		*pbuffer = data;

		/* assume pitch is 1,2,4,8,...*/
		if(!(i&(pitch-1)))
		//if((i+1) % pitch == 0)
		{
			/* need a return */
			pstart += offset;
			pbuffer = pstart;
		}else{
			pbuffer++;
		}

	}
	return 0;

}
