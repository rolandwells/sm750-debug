/* Header:   //Mercury/Projects/archives/XFree86/4.0/smi.h-arc   1.51   29 Nov 2000 17:45:16   Frido  $ */

/*
Copyright (C) 1994-1999 The XFree86 Project, Inc.  All Rights Reserved.
Copyright (C) 2000 Silicon Motion, Inc.  All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the names of the XFree86 Project and
Silicon Motion shall not be used in advertising or otherwise to promote the
sale, use or other dealings in this Software without prior written
authorization from the XFree86 Project and Silicon Motion.
*/
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/siliconmotion/smi.h,v 1.13 2003/04/23 21:51:44 tsi Exp $ */

#ifndef _SMI_H
#define _SMI_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "smi_pcirename.h" //caesar added

/*
#define USE_FB
*/

// Added for compiling Xorg 6.9
#ifndef XORG_VERSION_NUMERIC
#define XORG_VERSION_NUMERIC(major,minor,patch,snap,dummy) \
	(((major) * 10000000) + ((minor) * 100000) + ((patch) * 1000) + snap)
#endif



#include "xf86.h"
#include "xf86_OSproc.h"
#ifndef XSERVER_LIBPCIACCESS
//#if  XORG_VERSION_CURRENT <=  XORG_VERSION_NUMERIC(7,1,1,0,0)
#include "xf86_ansic.h"
#endif
#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "xf86Cursor.h"
#include "vgaHW.h"

#include "compiler.h"

#include "mipointer.h"
#include "micmap.h"
/*
#ifdef USE_FB
*/
#include "fb.h"
/*
#else

#define PSZ 8
#include "cfb.h"
#undef PSZ
#include "cfb16.h"
#include "cfb24.h"
#endif
*/

#include "xaa.h"
#include "xf86cmap.h"
#include "xf86i2c.h"

#include "xf86int10.h"
#include "vbe.h"

#include "xf86xv.h"
#include <X11/extensions/Xv.h>

#ifdef RANDR
#include <X11/extensions/randr.h>
#endif
/*
 * If you compile the driver with Xorg 1.3.0, please comment the line
 * #ifdef XORG_VERSION_CURRENT and the correspond #endif line
 */
// #ifdef XORG_VERSION_CURRENT
#include "xorg-server.h"
#include "xorgVersion.h"
// #endif


/******************************************************************************/
/*			D E F I N I T I O N S				      */
/******************************************************************************/
#define OUTPUT_TFT_CRT	1
#define OUTPUT_TFT_TFT 	2
#define OUTPUT_CRT_CRT	0

#define PNLTYPE_18	1
#define PNLTYPE_24	0
#define PNLTYPE_36	2
#ifndef SMI_DEBUG
#define SMI_DEBUG	 1
#endif

/*If define this flag, the driver will check  the "Drawing Engine FIFO Empty Status" when drawing.
	Note:
	1. This status is removed from the newest SM712 chip.
	2. We shouldn't check this status when running on the server, it will hang the system.
*/
#define SMI_CHECK_2D_FIFO 1

#define MAX_ENTITIES		16
#define MAX_ENT_IDX 	16

#define SMI_USE_IMAGE_WRITES	0
#define SMI_USE_VIDEO		1
#define SMI_USE_CAPTURE		0
// #define LCD_SIZE_DETECT          1


extern char videoInterpolation; /*enable videlo layer H/V interpolation*/

/*
 * Define some global variable
 */
/*
int	entity_priv_index[MAX_ENTITIES] = {-1, -1, -1, -1, -1, -1, -1, -1,
					-1, -1, -1, -1, -1, -1, -1, -1};
int	pci_tag = 0x0;
*/

#if 0
#if SMI_DEBUG
extern int smi_indent;
# define VERBLEV	1
# define ENTER()	xf86ErrorFVerb(VERBLEV, "%*c %s\n",\
						   smi_indent++, '>', __FUNCTION__)
# define LEAVE(...)							\
		do {								\
		xf86ErrorFVerb(VERBLEV, "%*c %s\n", 			\
				   --smi_indent, '<', __FUNCTION__);		\
		return __VA_ARGS__; 					\
		} while (0)
//# define DEBUG(...)	xf86ErrorFVerb(VERBLEV, __VA_ARGS__)
#define DEBUG(...) xf86DrvMsg(0, X_NOTICE,__VA_ARGS__);
#else
# define VERBLEV	4
# define ENTER()	/**/
# define LEAVE(...)	return __VA_ARGS__
# define DEBUG(...)	/**/
#endif

#define XMSG(...) xf86DrvMsg(pScrn->scrnIndex, X_NOTICE,__VA_ARGS__);
#define XERR(...) xf86DrvMsg(pScrn->scrnIndex, X_ERROR,__VA_ARGS__);
#define XCONF(...) xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,__VA_ARGS__);
#define XINF(...) xf86DrvMsg(pScrn->scrnIndex, X_INFO,__VA_ARGS__);
#define XOUT(from,...) xf86DrvMsg(pScrn->scrnIndex, from,__VA_ARGS__);
#else
#define XMSG(...) xf86DrvMsg(pScrn->scrnIndex, X_NOTICE,": "__VA_ARGS__)
#define XERR(...) xf86DrvMsg(pScrn->scrnIndex, X_ERROR,": "__VA_ARGS__)
#define XCONF(...) xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,": "__VA_ARGS__)
#define XINF(...) xf86DrvMsg(pScrn->scrnIndex, X_INFO,": "__VA_ARGS__)
#define XOUT(from,...) xf86DrvMsg(pScrn->scrnIndex, from,": "__VA_ARGS__)
#if SMI_DEBUG
extern int smi_indent;
# define VERBLEV	1
# define ENTER()	xf86ErrorFVerb(VERBLEV, "%*c %s\n",\
				       smi_indent++, '>', __FUNCTION__)
# define LEAVE(...)							\
    do {								\
	xf86ErrorFVerb(VERBLEV, "%*c %s\n",				\
		       --smi_indent, '<', __FUNCTION__);		\
	return __VA_ARGS__;						\
    } while (0)
    
#define DEBUG(...)	xf86Msg(X_NOTICE,__VA_ARGS__)
#else
# define VERBLEV	4
# define ENTER()	/**/
# define LEAVE(...)	return __VA_ARGS__
# define DEBUG(...)	/**/
#endif

#endif





/******************************************************************************/
/*			S T R U C T U R E S				      */
/******************************************************************************/

/* Driver data structure; this should contain all needed info for a mode */
typedef struct
{    
	Bool    modeInit;
	CARD16	mode;
       CARD8   SR17, SR18, SR21, SR31, SR32, SR6A, SR6B,SR6C, SR6D, SR6E, SR6F, SR81, SRA0;
       CARD8   CR30, CR33, CR33_2, CR3A;
	CARD8	CR40[14], CR40_2[14];
	CARD8	FPR50[14], FPR44, FPR45;
	CARD8	CR90[16], CR9F_2;
	CARD8	CRA0[14];
	CARD8	smiDACMask, smiDacRegs[256][3];
    /* CZ 2.11.2001: for gamma correction */
    CARD8   CCR66;
    /* end CZ */
	CARD8	smiFont[8192];
	CARD32	DPR10, DPR1C, DPR20, DPR24, DPR28, DPR2C, DPR30, DPR3C,
		DPR40, DPR44;
	CARD32	VPR00, VPR0C, VPR10;
	CARD32	CPR00;
	CARD32	FPR00_, FPR0C_, FPR10_;

	/* entity information */
	Bool DualHead;
	ScrnInfoPtr pSecondaryScrn;
	ScrnInfoPtr pPrimaryScrn;
	int		lastInstance;
	
	/* shared resource */
	int mmio_require;
	volatile unsigned char * MMIOBase;		/* Base of MMIO */
	int MapSize;	/* how many mmio should map and unmap */
	IOADDRESS		PIOBase;	/* Base of IO ports */
	int total_videoRam;			/* memory count in bytes */
	char * save750;			/* register value console keep */
	/* vga stuffs */
	char * fonts;	
} SMIRegRec, *SMIRegPtr;

/* Global PDEV structure. */
typedef struct
{
	/* function pointer */
	char * fonts;	
	char * save750;
	/* accel additions */
	CARD32			AccelCmd;	/* Value for DPR0C */
	CARD32			Stride;		/* Stride of frame buffer */
	CARD32			ScissorsLeft;	/* Left/top of current
						   scissors */
	CARD32			ScissorsRight;	/* Right/bottom of current
						   scissors */
	CARD32			dcrF0, dcr230;	/* Belcon Fix #001: Preserve
						   				hardware cursor address
						   				register of console mode */
	Bool			ClipTurnedOn;	/* Clipping was turned on by
						   the previous command */
	CARD8			SR18Value;	/* PDR#521: original SR18
						   value */
	CARD8			SR21Value;	/* PDR#521: original SR21
						   value */
	SMIRegRec		SavedReg;	/* console saved mode
						   registers */
	SMIRegRec		ModeReg;	/* XServer video state mode
						   registers */
	xf86CursorInfoPtr	CursorInfoRec;	/* HW Cursor info */

	Bool			ModeStructInit;	/* Flag indicating ModeReg has
						   been duped from console
						   state */
	int			vgaCRIndex, vgaCRReg;
	int			width, height;	/* Width and height of the
						   screen */
	int			Bpp;		/* Bytes per pixel */

	/* XAA */
	int			videoRAMBytes;	/* In units as noted, set in
						   PreInit  */
	int			videoRAMKBytes;	/* In units as noted, set in
						   PreInit */
	int			MapSize;	/* Size of mapped memory */
	CARD8 *			DPRBase;	/* Base of DPR registers */
	CARD8 *			VPRBase;	/* Base of VPR registers */
	CARD8 *			CPRBase;	/* Base of CPR registers */
    CARD8 *			FPRBase;    /* Base of FPR registers - for 0730 chipset */
    CARD8 *			DCRBase;    /* Base of DCR registers - for 0501 chipset */
    CARD8 *			SCRBase;    /* Base of SCR registers - for 0501 chipset */
	CARD8 *			DataPortBase;	/* Base of data port */
	int			DataPortSize;	/* Size of data port */
	
	/* below resource should get directly from entity but no mmap */
	CARD8 *			IOBase;		/* Base of MMIO VGA ports */
	IOADDRESS		PIOBase;	/* Base of I/O ports */
	unsigned char *		FBBase;		/* Base of FB */	
	unsigned char *		MapBase;	/* Base of mapped io */
	/*be monk*/
	
	CARD32			FBOffset;	/* Current visual FB starting
						   location */
	CARD32			FBCursorOffset;	/* Cursor storage location */
	CARD32			FBReserved;	/* Reserved memory in frame
						   buffer */
	Bool			PrimaryVidMapped;	/* Flag indicating if
							   vgaHWMapMem was used
							   successfully for
							   this screen */
	int			dacSpeedBpp;	/* Clock value */
	int			minClock;	/* Mimimum clock */
	int			maxClock;	/* Maximum clock */
	int			MCLK;		/* Memory Clock  */
	int			GEResetCnt;	/* Limit the number of errors
						   printed using a counter */

	Bool			pci_burst;	/* Enable PCI burst mode for
						   reads? */
	Bool			NoPCIRetry;	/* Disable PCI retries */
	Bool			fifo_conservative;	/* Adjust fifo for
							   acceleration? */
	Bool			fifo_moderate;	/* Adjust fifo for
						   acceleration? */
	Bool			fifo_aggressive;	/* Adjust fifo for
							   acceleration? */
	Bool			NoAccel;	/* Disable Acceleration */
	Bool			hwcursor;	/* hardware cursor enabled */
	Bool                    cursorflag;
	Bool			ShowCache;	/* Debugging option */
	Bool			useBIOS;	/* Use BIOS for mode sets */
	Bool 			RemoveBIOS; 	/*Use to some card bios remove*/
	Bool			zoomOnLCD;	/* Zoom on LCD */
	Bool			edid_enable;
	unsigned char* 	ediddata;
	CloseScreenProcPtr	CloseScreen;	/* Pointer used to save wrapped
						   CloseScreen function */
	XAAInfoRecPtr		AccelInfoRec;	/* XAA info Rec */
	pciVideoPtr		PciInfo;	/* PCI info vars */
	PCITAG			PciTag;
	int			Chipset;	/* Chip info, set using PCI
						   above */
	int			ChipRev;

	/* DGA */
	DGAModePtr		DGAModes;	/* Pointer to DGA modes */
	int			numDGAModes;	/* Number of DGA modes */
	Bool			DGAactive;	/* Flag if DGA is active */
	int			DGAViewportStatus;

	/* DPMS */
	int			CurrentDPMS;	/* Current DPMS state */
	unsigned char		DPMS_SR20;	/* Saved DPMS SR20 register */
	unsigned char		DPMS_SR21;	/* Saved DPMS SR21 register */
	unsigned char		DPMS_SR31;	/* Saved DPMS SR31 register */
	unsigned char		DPMS_SR34;	/* Saved DPMS SR34 register */

	/* Panel information */
	Bool			lcd;		/* LCD active, 1=TFT, 2=DSTN */
	int			lcdWidth;	/* LCD width */
	int			lcdHeight;	/* LCD height */

	I2CBusPtr		I2C;		/* Pointer into I2C module */
	I2CBusPtr		I2C_primary;	
	I2CBusPtr		I2C_secondary;	
	xf86Int10InfoPtr	pInt10;		/* Pointer to INT10 module */

	/* Shadow frame buffer (rotation) */
	Bool			shadowFB;	/* Flag if shadow buffer is
						   used */
	Bool			clone_mode;	/* Flag if clone mode is
						   used */
	int			rotate;		/* Rotation flags */
	Bool		RandRRotation;
	int			ShadowPitch;	/* Pitch of shadow buffer */
	int			ShadowWidthBytes;	/* Width of shadow
							   buffer in bytes */
	int			ShadowWidth;	/* Width of shadow buffer in
						   pixels */
	int			ShadowHeight;	/* Height of shadow buffer in
						   pixels */
	CARD32			saveBufferSize;	/* #670 - FB save buffer size */
	void *			pSaveBuffer;	/* #670 - FB save buffer */
	CARD32          fbMapOffset;    /* offset for fb mapping */
	CARD32			savedFBOffset;	/* #670 - Saved FBOffset value */
	CARD32			savedFBReserved;	/* #670 - Saved
							   FBReserved value */
	CARD8 *			paletteBuffer;	/* #920 - Palette save buffer */

	/* Polylines - #671 */
	ValidateGCProcPtr	ValidatePolylines;	/* Org.
							   ValidatePolylines
							   function */
	Bool			polyLines;	/* Our polylines patch is
						   active */

	void (*PointerMoved)(int index, int x, int y);

	int			videoKey;	/* Video chroma key */
	Bool			ByteSwap;	/* Byte swap for ZV port */
	Bool			interlaced;	/* True: Interlaced Video */
	/* XvExtension */
	XF86VideoAdaptorPtr	ptrAdaptor;	/* Pointer to VideoAdapter
						   structure */
	void (*BlockHandler)(int i, pointer blockData, pointer pTimeout,
						 pointer pReadMask);
	GCPtr			videoGC;
	OptionInfoPtr		Options;
    CARD8 DACmask;
		
	 /* dualhead */
	int 	output;
	int	xlcd;
	int	ylcd;
	int	pnltype;
	
	 Bool					 IsSecondary;  /* second Screen */
	 Bool					 IsPrimary;  /* first Screen */
	 
	 EntityInfoPtr			 pEnt_info;
	 DevUnion * 			 pEnt_private;	 
	 
	 Bool					 IsLCD;
	 Bool					 IsCRT;
	 /*For CSC Video*/
	 Bool 					IsCSCVideo;
} SMIRec, *SMIPtr;

#define SMIPTR(p) ((SMIPtr)((p)->driverPrivate))


/******************************************************************************/
/*			M A C R O S					      */
/******************************************************************************/

#if 0
#if SMI_DEBUG
#	define VERBLEV 1
/*
#	define ENTER_PROC(PROCNAME)	xf86ErrorFVerb(VERBLEV, "ENTER\t" PROCNAME \
									"(%d)\n", __LINE__); xf86Break1()
#	define DEBUG_PROC(PROCNAME)	xf86ErrorFVerb(VERBLEV, "DEBUG\t" PROCNAME \
									"(%d)\n", __LINE__); xf86Break2()
#	define LEAVE_PROC(PROCNAME)	xf86ErrorFVerb(VERBLEV, "LEAVE\t" PROCNAME \
									"(%d)\n", __LINE__); xf86Break1()
*/
#	define ENTER_PROC(PROCNAME)	xf86ErrorFVerb(VERBLEV, "ENTER\t" PROCNAME \
										"(%d)\n", __LINE__); 
#	define DEBUG_PROC(PROCNAME)	xf86ErrorFVerb(VERBLEV, "DEBUG\t" PROCNAME \
										"(%d)\n", __LINE__); 
#	define LEAVE_PROC(PROCNAME)	xf86ErrorFVerb(VERBLEV, "LEAVE\t" PROCNAME \
										"(%d)\n", __LINE__); 
#	define DEBUG(...)				xf86ErrorFVerb(VERBLEV, __VA_ARGS__)
#else
#	define VERBLEV	1   /* was 4 */
#	define ENTER_PROC(PROCNAME)
#	define DEBUG_PROC(PROCNAME)
#	define LEAVE_PROC(PROCNAME)
#	define DEBUG(...)
#endif
#endif

/* Some Silicon Motion structs & registers */
#include "regsmi.h"

#if !defined (MetroLink) && !defined (VertDebug)
#define VerticalRetraceWait()						\
do									\
{									\
    if (VGAIN8_INDEX(pSmi, vgaCRIndex, vgaCRData, 0x17) & 0x80)		\
    {									\
	while ((VGAIN8(pSmi, vgaIOBase + 0x0A) & 0x08) == 0x00);	\
	while ((VGAIN8(pSmi, vgaIOBase + 0x0A) & 0x08) == 0x08);	\
	while ((VGAIN8(pSmi, vgaIOBase + 0x0A) & 0x08) == 0x00);	\
    }									\
} while (0)
#else
#define SPIN_LIMIT 1000000
#define VerticalRetraceWait()						\
do									\
{									\
    if (VGAIN8_INDEX(pSmi, vgaCRIndex, vgaCRData, 0x17) & 0x80)		\
    {									\
	volatile unsigned long _spin_me;				\
	for (_spin_me = SPIN_LIMIT;					\
	     ((VGAIN8(pSmi, vgaIOBase + 0x0A) & 0x08) == 0x00) && 	\
	     _spin_me;							\
	     _spin_me--);						\
	if (!_spin_me)							\
	    ErrorF("smi: warning: VerticalRetraceWait timed out.\n");	\
	for (_spin_me = SPIN_LIMIT;					\
	     ((VGAIN8(pSmi, vgaIOBase + 0x0A) & 0x08) == 0x08) && 	\
	     _spin_me;							\
	     _spin_me--);						\
	if (!_spin_me)							\
	    ErrorF("smi: warning: VerticalRetraceWait timed out.\n");	\
	for (_spin_me = SPIN_LIMIT;					\
	     ((VGAIN8(pSmi, vgaIOBase + 0x0A) & 0x08) == 0x00) && 	\
	     _spin_me;							\
	     _spin_me--);						\
	if (!_spin_me)							\
	    ErrorF("smi: warning: VerticalRetraceWait timed out.\n");	\
	}								\
} while (0)
#endif

/******************************************************************************/
/*			F U N C T I O N   P R O T O T Y P E S		      */
/******************************************************************************/

/* smi_dac.c */
void SMI_CommonCalcClock(int scrnIndex, long freq, int min_m, int min_n1, 
			 int max_n1, int min_n2, int max_n2, long freq_min, 
			 long freq_max, unsigned char * mdiv, 
			 unsigned char * ndiv);

/* smi_i2c */
Bool SMI_I2CInit(ScrnInfoPtr pScrn);

/* smi_accel.c */
Bool SMI_AccelInit(ScreenPtr pScrn);
void SMI_AccelSync(ScrnInfoPtr pScrn);
void SMI_GEReset(ScrnInfoPtr pScrn, int from_timeout, int line, char *file);
void SMI_EngineReset(ScrnInfoPtr);

/* smi_hwcurs.c */
Bool SMI_HWCursorInit(ScreenPtr pScrn);

/* smi_driver.c */
void SMI_AdjustFrame(int scrnIndex, int x, int y, int flags);
Bool SMI_SwitchMode(int scrnIndex, DisplayModePtr mode, int flags);

/* smi_dga.c */
Bool SMI_DGAInit(ScreenPtr pScrn);

/* smi_shadow.c */
void SMI_PointerMoved(int index, int x, int y);
void SMI_RefreshArea(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void SMI_RefreshArea730(ScrnInfoPtr pScrn, int num, BoxPtr pbox);

/* smi_video.c */
void SMI_InitVideo(ScreenPtr pScreen);

#define PEEK32(I)		(*(volatile unsigned int *)(pSmi->MapBase + I))
#define POKE32(I,D)		(*(volatile unsigned int *)(pSmi->MapBase + I)) = (D)
	


#endif  /*_SMI_H*/
