#ifndef DDK_COMMON_H__
#define DDK_COMMON_H__
#include<linux/string.h>
#include "ddk750_regs.h"
#include "ddk750_type.h"
#include "ddk750_intern.h"
#include "ddk750_func.h"

/* below two func is in sm750hw.c	*/
extern unsigned int smread32(unsigned long);
extern void smwrite32(unsigned long,unsigned int);
extern unsigned char smread8(unsigned long);
extern void smwrite8(unsigned long,unsigned char);

#if 1
#define peekRegisterByte 	smread8
#define peekRegisterDWord	smread32
#define pokeRegisterDWord	smwrite32
#define pokeRegisterByte 	smwrite8
#define POKE_32	smwrite32
#define PEEK_32	smread32
#endif

#define DDKDEBUGPRINT 
#define DISPLAY_LEVEL 0


#endif

