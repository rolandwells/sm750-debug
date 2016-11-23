#ifndef DDK_COMMON_H__
#define DDK_COMMON_H__
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/delay.h>	
//#include <math.h>


#include "ddk750_regs.h"
#include "ddk750_type.h"
#include "ddk750_intern.h"
#include "ddk750_func.h"
#include "edid.h"

/* below two func is in sm750hw.c	*/
extern unsigned int peek32_2d(void *,unsigned long);
extern void poke32_2d(void *,unsigned long,unsigned int);
extern unsigned int peek32(void *,unsigned long);
extern void poke32(void *,unsigned long,unsigned int);
extern unsigned char peek8(void *,unsigned long);
extern void poke8(void *,unsigned long,unsigned char);

extern void poke32_dataport(void *,uint32_t);
extern void fillDataPort(void *,int);

#define DDKDEBUGPRINT 
#define DISPLAY_LEVEL 0

#endif

