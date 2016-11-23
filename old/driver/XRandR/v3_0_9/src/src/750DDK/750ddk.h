#ifndef _750_DDK_H_
#define _750_DDK_H_


#include "defs.h"
#include "regdc.h"
#include "regde.h"
#include "regdma.h"
#include "reggpio.h"
#include "regi2c.h"
#include "regpwm.h"
#include "regsc.h"
#include "regssp.h"
#include "regzv.h"

#include "2d.h"
#include  "alpha.h"
#include "at25f512.h"
#include "bitmap.h"
#include "burst.h"
#include "capture.h"
#include "chip.h"
#include "clock.h"
#include "csc.h"
#include "cursor.h"
#include "ddkdebug.h"
#include "deint.h"
#include "edid.h"
#include "dispint.h"
#include "display.h"
#include "dma.h"
#include "font.h"
#include "hardware.h"
//#include "hwi2c.h"
#include "mode.h"
#include "os.h"
#include "panning.h"
#include "power.h"
#include "saa7118.h"
#include "ssp.h"
#include "sw2d.h"
#include "swi2c.h"
#include "tstpic.h"
#include "valpha.h"
//#include "version.h"
#include "video.h"

unsigned int xorg_setMode(logicalMode_t * pLogicalMode,DisplayModePtr adjusted_mode);
void xorg_I2CUDelay(int);




#endif


