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

   Except as contained in this notice, the names of The XFree86 Project and
   Silicon Motion shall not be used in advertising or otherwise to promote the
   sale, use or other dealings in this Software without prior written
   authorization from The XFree86 Project or Silicon Motion.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "compiler.h"
#include "smi.h"
#include "smi_750.h"

#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
#include "xf86Resources.h"
#include "xf86RAC.h"
#endif

#include "xf86DDC.h"
#include "xf86int10.h"
#include "vbe.h"
#include "shadowfb.h"
#include "smi.h"

#include "globals.h"
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,7,0,0,0)
#include <X11/extensions/dpmsconst.h>
#else
#define DPMS_SERVER
#include  <X11/extensions/dpms.h>
#endif


#if SMI_DEBUG
int smi_indent = 1;
#endif

#define KB(X)	(X*1024)
#define MB(X)	(KB(X)*1024)

#ifdef RANDR
static Bool    SMI_DriverFunc(ScrnInfoPtr pScrnInfo, xorgDriverFuncOp op,
		pointer data);
#endif

/*#include "smi_501.h"*/
extern void setPower_501 (SMIPtr pSmi, unsigned int nGates, unsigned int nClock,
		int control_value);
extern void setDPMS_501 (SMIPtr pSmi, int state);
extern void crtSetMode_501 (SMIPtr pSmi, int nWidth, int nHeight,
		unsigned int fMode, int nHertz, int fbPitch, int bpp);
extern void panelSetMode_501 (SMIPtr pSmi, int nWidth, int nHeight,
		unsigned int fMode, int nHertz, int fbPitch,
		int bpp);
extern void panelUseCRT (SMIPtr pSmi, BOOL bEnable);
//extern Bool entity_init_501(ScrnInfoPtr pScrn, int entity);


void SMI_PrintMode(DisplayModePtr mode);

/*
 * Internals
 */
void SMI_EnableMmio (ScrnInfoPtr pScrn);
static void SMI_DisableMmio (ScrnInfoPtr pScrn);
static void SMI_RestoreReg_Con(ScrnInfoPtr pScrn);
static void SMI_SaveReg_Con(ScrnInfoPtr pScrn);

/*
 * Forward definitions for the functions that make up the driver.
 */

static const OptionInfoRec *SMI_AvailableOptions (int chipid, int busid);
static void SMI_Identify (int flags);
static Bool SMI_Probe (DriverPtr drv, int flags);
static Bool SMI_PreInit (ScrnInfoPtr pScrn, int flags);
static Bool SMI_EnterVT (int scrnIndex, int flags);
static void SMI_LeaveVT (int scrnIndex, int flags);
static void SMI_Save (ScrnInfoPtr pScrn);
static void SMI_BiosWriteMode (ScrnInfoPtr pScrn, DisplayModePtr mode, SMIRegPtr restore);
static void SMI_WriteMode (ScrnInfoPtr pScrn, vgaRegPtr, SMIRegPtr);
static Bool SMI_ScreenInit (int scrnIndex, ScreenPtr pScreen, int argc,
		char **argv);
static int SMI_InternalScreenInit (int scrnIndex, ScreenPtr pScreen);
static void SMI_PrintRegs (ScrnInfoPtr);
static ModeStatus SMI_ValidMode (int scrnIndex, DisplayModePtr mode,
		Bool verbose, int flags);
static void SMI_DisableVideo (ScrnInfoPtr pScrn);
static void SMI_EnableVideo (ScrnInfoPtr pScrn);
static Bool SMI_MapMem (ScrnInfoPtr pScrn);
static void SMI_UnmapMem (ScrnInfoPtr pScrn);
static Bool SMI_ModeInit (ScrnInfoPtr pScrn, DisplayModePtr mode);
static Bool SMI_CloseScreen (int scrnIndex, ScreenPtr pScreen);
static Bool SMI_SaveScreen (ScreenPtr pScreen, int mode);
static void SMI_LoadPalette (ScrnInfoPtr pScrn, int numColors, int *indicies,
		LOCO * colors, VisualPtr pVisual);
static void SMI_DisplayPowerManagementSet (ScrnInfoPtr pScrn,
		int PowerManagementMode,
		int flags);
static Bool SMI_ddc1 (int scrnIndex);
static unsigned int SMI_ddc1Read (ScrnInfoPtr pScrn);
static void SMI_FreeScreen (int ScrnIndex, int flags);
static void SMI_ProbeDDC (ScrnInfoPtr pScrn, int index);

static Bool SMI_MSOCSetMode_501 (ScrnInfoPtr pScrn, DisplayModePtr mode);

static Bool SMI_MSOCSetMode_750 (ScrnInfoPtr pScrn, DisplayModePtr mode);
static void SMI_SaveReg_750(ScrnInfoPtr pScrn );
static void SMI_RegInit(ScrnInfoPtr pScrn);
static void SMI_RestoreReg_750(ScrnInfoPtr pScrn );
static void SMI_SaveReg_502(ScrnInfoPtr pScrn );
static void SMI_RestoreReg_502(ScrnInfoPtr pScrn );
static void SMI750_Restore(ScrnInfoPtr pScrn);
static void SMI750_Save(ScrnInfoPtr pScrn);
//extern Bool entity_init_750(int entity,pointer private);




//void EnableOverlay(SMIPtr pSmi);
#define SILICONMOTION_NAME          "Silicon Motion"
#define SILICONMOTION_DRIVER_NAME   "siliconmotion"
#if 0
#define SILICONMOTION_VERSION_NAME  "2.4.0"
#define SMI_VER_MAJOR 2
#define SMI_VER_MINOR 4
#define SMI_VER_PATCH    0
#else
#define SMI_VER_MAJOR	2
#define SMI_VER_MINOR	4
#define SMI_VER_PATCH	3
#define VERSION_ATTACH_TMP(a,b,c)	#a"."#b"."#c
#define VERSION_ATTACH_TMP2(a,b,c)	VERSION_ATTACH_TMP(a,b,c)
#define SILICONMOTION_VERSION_NAME	VERSION_ATTACH_TMP2(SMI_VER_MAJOR,SMI_VER_MINOR,SMI_VER_PATCH)
#endif


#define SILICONMOTION_DRIVER_VERSION ((SMI_VER_MAJOR << 24) | \
		(SMI_VER_MINOR << 16) | \
		(SMI_VER_PATCH))



extern int	free_video_memory;
extern mode_table_t mode_table_edid;

/* I think entity_priv_index should better be renamed to entity_privIndex */
int	entity_priv_index[MAX_ENTITIES] = {-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1};

int	pci_tag = 0x0;
static int	saved_console_reg = -1;
static int  saved_video_mode_count =0;
char videoInterpolation = 0xff;	


/*
 * This contains the functions needed by the server after loading the
 * driver module.  It must be supplied, and gets added the driver list by
 * the Module Setup funtion in the dynamic case.  In the static case a
 * reference to this is compiled in, and this requires that the name of
 * this DriverRec be an upper-case version of the driver name.
 */

/* #if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0) */
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0)
	_X_EXPORT DriverRec SILICONMOTION =
#elif XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
	_X_EXPORT DriverRec SILICONMOTION =
#else
	DriverRec SILICONMOTION =
#endif
{
	SILICONMOTION_DRIVER_VERSION,
	SILICONMOTION_DRIVER_NAME,
	SMI_Identify,
	SMI_Probe,
	SMI_AvailableOptions,
	NULL,
	0
};

/* Supported chipsets */
static SymTabRec SMIChipsets[] = {
	{PCI_CHIP_SMI910, "Lynx"},
	{PCI_CHIP_SMI810, "LynxE"},
	{PCI_CHIP_SMI820, "Lynx3D"},
	{PCI_CHIP_SMI710, "LynxEM"},
	{PCI_CHIP_SMI712, "LynxEM+"},
	{PCI_CHIP_SMI720, "Lynx3DM"},
	{PCI_CHIP_SMI731, "Cougar3DR"},
	{PCI_CHIP_SMI501, "MSOC"},
	{PCI_CHIP_SMI750, "MSOCE"},
	{-1, NULL}
};

static PciChipsets SMIPciChipsets[] = {
	/* numChipset,          PciID,                          Resource */
	{PCI_CHIP_SMI910, PCI_CHIP_SMI910, RES_SHARED_VGA},
	{PCI_CHIP_SMI810, PCI_CHIP_SMI810, RES_SHARED_VGA},
	{PCI_CHIP_SMI820, PCI_CHIP_SMI820, RES_SHARED_VGA},
	{PCI_CHIP_SMI710, PCI_CHIP_SMI710, RES_SHARED_VGA},
	//{PCI_CHIP_SMI712, PCI_CHIP_SMI712, RES_SHARED_VGA},
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
	{PCI_CHIP_SMI712, PCI_CHIP_SMI712, resVgaIo},
#else
	{PCI_CHIP_SMI712, PCI_CHIP_SMI712, RES_SHARED_VGA},
#endif
	{PCI_CHIP_SMI720, PCI_CHIP_SMI720, RES_SHARED_VGA},
	{PCI_CHIP_SMI731, PCI_CHIP_SMI731, RES_SHARED_VGA},
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
	{PCI_CHIP_SMI501, PCI_CHIP_SMI501, resVgaIo},
#else
	{PCI_CHIP_SMI501, PCI_CHIP_SMI501, RES_SHARED_VGA},
#endif
	//added by t-bag
	//  {PCI_CHIP_SMI750, PCI_CHIP_SMI750, RES_SHARED_VGA},
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
	{PCI_CHIP_SMI750, PCI_CHIP_SMI750, resVgaIo},	
#else
	{PCI_CHIP_SMI750, PCI_CHIP_SMI750, RES_SHARED_VGA},	
#endif
	{-1, -1, RES_UNDEFINED}
};

typedef enum
{
	OPTION_PCI_BURST,
	OPTION_FIFO_CONSERV,
	OPTION_FIFO_MODERATE,
	OPTION_FIFO_AGGRESSIVE,
	OPTION_PCI_RETRY,
	OPTION_NOACCEL,
	OPTION_MCLK,
	OPTION_SHOWCACHE,
	OPTION_SWCURSOR,  
	OPTION_CLONE_MODE,
	OPTION_HWCURSOR,
	OPTION_SHADOW_FB,
	OPTION_ROTATE,
	OPTION_VIDEOKEY,
	OPTION_BYTESWAP,
	/* CZ 26.10.2001: interlaced video */
	OPTION_INTERLACED,
	/* end CZ */
	OPTION_USEBIOS,
	OPTION_REMOVEBIOS,
	OPTION_ZOOMONLCD,
	OPTION_PANELWIDTH,
	OPTION_PANELHEIGHT,
	NUMBER_OF_OPTIONS,
	/*Add for CSC Video*/
	OPTION_CSCVIDEO,

	OPTION_OUTPUT,
	OPTION_PNLTYPE,
	OPTION_XLCD,
	OPTION_YLCD,

	OPTION_EDID,
} SMIOpts;

static const OptionInfoRec SMIOptions[] = {
	{OPTION_PCI_BURST, "pci_burst", OPTV_BOOLEAN, {0}, TRUE},
	{OPTION_FIFO_CONSERV, "fifo_conservative", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_FIFO_MODERATE, "fifo_moderate", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_FIFO_AGGRESSIVE, "fifo_aggressive", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_PCI_RETRY, "pci_retry", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_NOACCEL, "NoAccel", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_MCLK, "set_mclk", OPTV_FREQ, {0}, FALSE},
	{OPTION_SHOWCACHE, "show_cache", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_HWCURSOR, "HWCursor", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_SWCURSOR, "SWCursor", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_SHADOW_FB, "ShadowFB", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_CLONE_MODE, "CloneMode", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_ROTATE, "Rotate", OPTV_ANYSTR, {0}, FALSE},
	{OPTION_VIDEOKEY, "VideoKey", OPTV_INTEGER, {0}, FALSE},
	{OPTION_BYTESWAP, "ByteSwap", OPTV_BOOLEAN, {0}, FALSE},
	/* CZ 26.10.2001: interlaced video */
	{OPTION_INTERLACED, "Interlaced", OPTV_BOOLEAN, {0}, FALSE},
	/* end CZ */
	{OPTION_USEBIOS, "UseBIOS", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_REMOVEBIOS, "RemoveBIOS", OPTV_BOOLEAN ,{0}, FALSE},
	{OPTION_ZOOMONLCD, "ZoomOnLCD", OPTV_BOOLEAN, {0}, FALSE},
	/* Added by Belcon */
	{OPTION_PANELHEIGHT, "LCDHeight", OPTV_INTEGER, {0}, FALSE},
	{OPTION_PANELWIDTH, "LCDWidth", OPTV_INTEGER, {0}, FALSE},
	/*Add for CSC Video*/
	{OPTION_CSCVIDEO, "CSCVideo",OPTV_BOOLEAN, {0}, FALSE},

	{OPTION_OUTPUT,"output",OPTV_ANYSTR,{0},FALSE},
	{OPTION_PNLTYPE,"pnltype",OPTV_ANYSTR,{0},FALSE},
	{OPTION_XLCD,"xlcd",OPTV_INTEGER,{0},FALSE},
	{OPTION_YLCD,"ylcd",OPTV_INTEGER,{0},FALSE},
	{OPTION_EDID,"EDID_EN",OPTV_BOOLEAN,{0},FALSE},

	{-1, NULL, OPTV_NONE, {0}, FALSE}
};

/*
 * Lists of symbols that may/may not be required by this driver.
 * This allows the loader to know which ones to issue warnings for.
 *
 * Note that vgahwSymbols and xaaSymbols are referenced outside the
 * XFree86LOADER define in later code, so are defined outside of that
 * define here also.
 */

static const char *vgahwSymbols[] = {
	"vgaHWCopyReg",
	"vgaHWGetHWRec",
	"vgaHWGetIOBase",
	"vgaHWGetIndex",
	"vgaHWInit",
	"vgaHWLock",
	"vgaHWUnLock",
	"vgaHWMapMem",
	"vgaHWProtect",
	"vgaHWRestore",
	"vgaHWSave",
	"vgaHWSaveScreen",
	"vgaHWSetMmioFuncs",
	"vgaHWSetStdFuncs",
	"vgaHWUnmapMem",
	/* #if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0) */
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0)
	"vgaHWddc1SetSpeedWeak",
#elif XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
	"vgaHWddc1SetSpeedWeak", 
#endif
	NULL
};

static const char *xaaSymbols[] = {
	// #if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0)
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0)
	"XAAGetCopyROP",
#elif XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
	"XAAGetCopyROP",
#else
	"XAACopyROP",
#endif
	"XAACreateInfoRec",
	"XAADestroyInfoRec",
	// #if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0)
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0)
	"XAAGetFallbackOps",
#elif XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
	"XAAGetFallbackOps",
#else
	"XAAFallbackOps",
	"XAAScreenIndex",
#endif
	"XAAInit",
	// #if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0)
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0)
	"XAAGetPatternROP",
#elif XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
	"XAAGetPatternROP",
#else
	"XAAPatternROP",
#endif
	NULL
};

static const char *ramdacSymbols[] = {
	"xf86CreateCursorInfoRec",
	"xf86DestroyCursorInfoRec",
	"xf86InitCursor",
	NULL
};

static const char *ddcSymbols[] = {
	"xf86PrintEDID",
	"xf86DoEDID_DDC1",
	"xf86InterpretEDID",
	"xf86DoEDID_DDC2",
	"xf86SetDDCproperties",
	NULL
};

static const char *i2cSymbols[] = {
	"xf86CreateI2CBusRec",
	"xf86CreateI2CDevRec",
	"xf86DestroyI2CBusRec",
	"xf86DestroyI2CDevRec",
	"xf86I2CBusInit",
	"xf86I2CDevInit",
	"xf86I2CReadBytes",
	"xf86I2CWriteByte",
	NULL
};

static const char *shadowSymbols[] = {
	"ShadowFBInit",
	NULL
};

static const char *int10Symbols[] = {
	"xf86ExecX86int10",
	"xf86FreeInt10",
	"xf86InitInt10",
	NULL
};

static const char *vbeSymbols[] = {
	"VBEInit",
	"vbeDoEDID",
	"vbeFree",
	NULL
};

static const char *fbSymbols[] = {

	"fbPictureInit",
	"fbScreenInit",
	NULL
};


static unsigned char SeqTable[144];
static unsigned char CrtTable[168];
/*static unsigned char DprIndexTable[27]={
  0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 
  0x10, 0x12, 0x14, 0x18, 0x1c, 0x1e, 0x20, 0x24,
  0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x38,
  0x3c, 0x40, 0x44,
  };*/
static unsigned char DprIndexTable[21]={
	0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 
	0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
	0x40, 0x44, 0x48, 0x4c, 0x50
};
static unsigned int DprTable[21];
static unsigned char VprIndexTable_501[23]={
	0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 
	0x20, 0x24, 0x2c, 0x30, 0x34, 0x38, 0x3c, 0x40, 
	0x54, 0x58, 0x5c, 0x60, 0x64, 0x68, 0x6c
};
static unsigned int VprTable_501[23];
static unsigned char VprIndexTable_750[35]={
	0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c,
	0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
	0x40, 0x44, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c, 
	0x60, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c,
	0x80, 0x84, 0x8c
};
static unsigned int VprTable_750[35];
/*static unsigned int DcrIndexTable[33] = {
  0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e,
  0x10, 0x12, 0x14, 0x18, 0x1c, 0x1e, 0x20, 0x24,
  0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x200,
  0x204, 0x208, 0x20c, 0x210, 0x214, 0x218, 0x21c,
  0x220, 0x224 
  };*/

static unsigned int DcrIndexTable[26] = {
	0x00, 0x04, 0x08, 0x0c,0x10, 0x14, 0x18, 0x1c, 
	0x20, 0x24, 0x28, 0x2c,0x30, 0x34, 0x200,0x204,
	0x208, 0x20c, 0x210, 0x214, 0x218, 0x21c,0x220,
	0x224 
};
static unsigned int DcrTable[26];



/*just only for console timing registers restoration*/
static unsigned char SeqTable_Con[144];
static unsigned char CrtTable_Con[168];
/*static unsigned char DprIndexTable_Con[27]={
  0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e,
  0x10, 0x12, 0x14, 0x18, 0x1c, 0x1e, 0x20, 0x24,
  0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x38,
  0x3c, 0x40, 0x44  
  };
  static unsigned int DprTable_Con[27];*/
static unsigned char DprIndexTable_Con[21]={
	0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 
	0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
	0x40, 0x44, 0x48, 0x4c, 0x50
};
static unsigned int DprTable_Con[21];
static unsigned char VprIndexTable_Con[35]={
	0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c,
	0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
	0x40, 0x44, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c, 
	0x60, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c,
	0x80, 0x84, 0x8c
};
static unsigned int VprTable_Con[35];
/*static unsigned int DcrIndexTable_Con[33] = {
  0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e,
  0x10, 0x12, 0x14, 0x18, 0x1c, 0x1e, 0x20, 0x24,
  0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x200,
  0x204, 0x208, 0x20c, 0x210, 0x214, 0x218, 0x21c,
  0x220, 0x224
  };
  static unsigned int DcrTable_Con[33];*/
static unsigned int DcrIndexTable_Con[26] = {
	0x00, 0x04, 0x08, 0x0c,0x10, 0x14, 0x18, 0x1c, 
	0x20, 0x24, 0x28, 0x2c,0x30, 0x34, 0x200,0x204,
	0x208, 0x20c, 0x210, 0x214, 0x218, 0x21c,0x220,
	0x224 
};

unsigned char BIOS_SequencerReg720[] =
{
	0x03, 0x00, 0x03, 0x00, 0x02, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32,
	0x03, 0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x2F, 0x0F, 0x00,
	0x84, 0x31, 0x02, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x32, 0x03, 0x20, 0x09, 0xC0, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x00, 0x00, 0x03, 0xFF, 
	0x00, 0xFC, 0x00, 0x00, 0x20, 0xF0, 0x00, 0xFC, 0x20, 0x3C, 0x44, 0x20, 0x00, 0x00, 0x00, 0x32, 
	0x04, 0x24, 0x63, 0x4F, 0x52, 0x0B, 0xDF, 0xE9, 0x07, 0x03, 0x19, 0x32, 0x32, 0x00, 0x00, 0x32, 
	0x00, 0x00, 0xFF, 0x1A, 0x1A, 0x00, 0x03, 0x00, 0x54, 0x04, 0x0E, 0x02, 0x07, 0x82, 0x07, 0x82,
	0x00, 0xCD, 0x0C, 0x0C, 0x04, 0x00, 0x3F, 0x32, 0x32, 0x32, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x3D, 0x7A, 0x0B, 0x32, 0x00, 0x00, 0x00, 0x00, 0x55, 0x61, 0x32, 0x32,
	0x00, 0x00, 0x00, 0x00, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32,
	0x00, 0x08, 0x00, 0x00, 0x00, 0xED, 0xED, 0xED, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};


unsigned char BIOS_Graphic_ctl[]=
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00, 0xFF
};

unsigned char BIOS_AT_CTL[]=
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07, 0x38, 0x09, 0x3A, 0x0B, 0x3C, 0x0D, 0x3E, 0x0F,
	0x0C, 0x11, 0x0F, 0x13, 0x00 
};

static unsigned int DcrTable_Con[26];
#ifdef XFree86LOADER

static MODULESETUPPROTO (siliconmotionSetup);

static XF86ModuleVersionInfo SMIVersRec = {
	"siliconmotion",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	// #if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0)
#ifdef XORG_VERSION_CURRENT
	XORG_VERSION_CURRENT,
#else
	XF86_VERSION_CURRENT,
#endif
	SMI_VER_MAJOR,
	SMI_VER_MINOR,
	SMI_VER_PATCH,
	ABI_CLASS_VIDEODRV,
	ABI_VIDEODRV_VERSION,
	MOD_CLASS_VIDEODRV,
	{0, 0, 0, 0}
};

/*
 * This is the module init data for XFree86 modules.
 *
 * Its name has to be the driver name followed by ModuleData.
 */

// #if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0)
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0)
	_X_EXPORT XF86ModuleData siliconmotionModuleData =
#elif XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
	_X_EXPORT XF86ModuleData siliconmotionModuleData =
#else
	XF86ModuleData siliconmotionModuleData =
#endif
{
	&SMIVersRec,
	siliconmotionSetup,
	NULL
};

	static pointer
siliconmotionSetup (pointer module, pointer opts, int *errmaj, int *errmin)
{
	static Bool setupDone = FALSE;

	if (!setupDone)
	{
		setupDone = TRUE;
		xf86AddDriver (&SILICONMOTION, module, 0);

		/*
		 * Modules that this driver always requires can be loaded here
		 * by calling LoadSubModule().
		 */

		/*
		 * Tell the loader about symbols from other modules that this module
		 * might refer to.
		 */
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
		LoaderRefSymLists (vgahwSymbols, fbSymbols, xaaSymbols, ramdacSymbols,
				ddcSymbols, i2cSymbols, int10Symbols, vbeSymbols,
				shadowSymbols, NULL);
#endif
		/*
		 * The return value must be non-NULL on success even though there
		 * is no TearDownProc.
		 */
		return (pointer) 1;
	}
	else
	{
		if (errmaj)
		{
			*errmaj = LDR_ONCEONLY;
		}
		return (NULL);
	}
}

#endif /* XFree86LOADER */



	SMIRegPtr
SMIEntPriv (ScrnInfoPtr pScrn)
{
	DevUnion *pPriv;

	//  pPriv = xf86GetEntityPrivate (pScrn->entityList[0], gSMIEntityIndex);
	pPriv = xf86GetEntityPrivate (pScrn->entityList[0], entity_priv_index[pScrn->entityList[0]]);

	return pPriv->ptr;
}



	static Bool
SMI_GetRec (ScrnInfoPtr pScrn)
{

	/*
	 * Allocate an 'Chip'Rec, and hook it into pScrn->driverPrivate.
	 * pScrn->driverPrivate is initialised to NULL, so we can check if
	 * the allocation has already been done.
	 */
	if (pScrn->driverPrivate == NULL)
	{
		pScrn->driverPrivate = xnfcalloc (sizeof (SMIRec), 1);
#if SMI_DEBUG
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "For screen %d, allocate an 'chip' rec (0x%x)\n", pScrn->scrnIndex, pScrn->driverPrivate);
#endif
	}

	return (pScrn->driverPrivate ? TRUE : FALSE);
}

	static void
SMI_FreeRec (ScrnInfoPtr pScrn)
{

	if (pScrn->driverPrivate != NULL)
	{
		xfree (pScrn->driverPrivate);
		pScrn->driverPrivate = NULL;
	}

}

	static const OptionInfoRec *
SMI_AvailableOptions (int chipid, int busid)
{
	return (SMIOptions);
}

	static void
SMI_Identify (int flags)
{

	xf86PrintChipsets (SILICONMOTION_NAME, "driver (version "
			SILICONMOTION_VERSION_NAME
			") for Silicon Motion Lynx chipsets", SMIChipsets);

}

/* this routine supposed to be called only one time for each entity */
int Smi_EntityInit_Common(int entityIndex,pointer private)
{
	DevUnion *pPriv;
	ENTER();	

	/*entityIndex should better not lager than 16*/
	if(entityIndex > MAX_ENT_IDX){
		xf86ErrorF("entityIndex:%d is too large\n",entityIndex);
		LEAVE(1);
	}

	if(entity_priv_index[entityIndex] == -1)
	{
		entity_priv_index[entityIndex] = xf86AllocateEntityPrivateIndex();
		xf86Msg(X_INFO,"entity_prv_index[%d] = %d\n",entityIndex,entity_priv_index[entityIndex]);	
	}

	pPriv =  xf86GetEntityPrivate(entityIndex,entity_priv_index[entityIndex]);
#if SMI_DEBUG == 1	
	if(!pPriv->ptr){
#endif			
		pPriv->ptr = private;
#if SMI_DEBUG == 1			
	}else{
		xf86ErrorF("NANI? this should never happen!\n");
		LEAVE(2);
	}
#endif	

	LEAVE (0);
}

Bool entity_init_vga(int entityIndex,pointer private)
{
	SMIRegPtr pRePtr = (SMIRegPtr)private;
	struct pci_device * pd;
	ENTER();

	pd = xf86GetPciInfoForEntity(entityIndex);	
	if(pd && !xf86IsEntityPrimary(entityIndex)){
#ifdef 	XSERVER_LIBPCIACCESS	
		/* give up vga legency resource decodes for none-primary vga cards */
		pci_device_vgaarb_set_target(pd);
		pci_device_vgaarb_decodes(0);
		/*	monk:
			Weird, with only above line, it not work.
			it works well together with below line.
			root cause:

			Libpciaccess will not use new decode flag passing to its immediate kernel sys call 
			but use original decode flag, and save new decode flag to its struct member after kernel sys call.
			That's make kernel not sync with the latest request .Maybe it's a bug of libpciaccess
			So I used a tricky method to fool around the libpciacess in order to make kernel vgaarb 
			consider this card already disclaimed for its vga legency resource decodings:

			Call again "pci_device_vgaarb_decodes" with meaningless  decode flag but not the same as last one
			it will make libpciaccess using last saved decode flag (it is 0) as the parameter for kernel  sys call
		 */
		pci_device_vgaarb_decodes(0x10);		
#endif	

	}else{
		xf86ErrorF("NANI!? this should never happen!\n");
		LEAVE(FALSE);
	}
	LEAVE(TRUE);
}


/* each entity get one chance to execute SMI_Entityinit */
void SMI_EntityInit(int entityIndex,pointer private)
{		
	struct pci_device * pd;
	EntityInfoPtr pEnt;	
	ENTER();

	if(Smi_EntityInit_Common(entityIndex,private)){
		xf86ErrorF("error happend in entity init common\n");
		LEAVE();
	}

	pd = xf86GetPciInfoForEntity(entityIndex);
	pEnt = xf86GetEntityInfo (entityIndex);	

	switch(pEnt->chipset)
	{		
		case PCI_CHIP_SMI750:			
			entity_init_vga(entityIndex,private);
#ifdef XSERVER_LIBPCIACCESS			
			/*	enable pci device will open pci config space io/mem bits in theory ( according to kernel source code: pci-sysfs.c )
				Butter... seems 718 and 750 only be enabled for mem bits
			 */
			pci_device_enable(pd);	
#else
			/* TODO: how to enable pci device with out libpciaccess ? */
#endif			
			break;
		case PCI_CHIP_SMI501:				
#ifdef XSERVER_LIBPCIACCESS			
			/*	sm501 need do pci device probe manually because libpciaccess not did,
				due to the non-vga class 501 claims 
			 */			
			pci_device_probe(pd);
#endif
			break;
		default:
			break;
	}
	/* don't forget to free it */
	xfree(pEnt);
	LEAVE();
}

	static Bool
SMI_Probe (DriverPtr drv, int flags)
{
	int i;
	GDevPtr *devSections;
	int *usedChips;
	int numDevSections;
	int numUsed;
	Bool foundScreen = FALSE;

	static void * entity_private[MAX_ENTITIES] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
		NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

#ifdef XSERVER_LIBPCIACCESS
	ScrnInfoPtr pScrn;
	struct pci_device * pd;
#endif

	numDevSections = xf86MatchDevice (SILICONMOTION_DRIVER_NAME, &devSections);

	if (numDevSections <= 0)
		return (FALSE);

#ifndef XSERVER_LIBPCIACCESS
	/* who can tell me what the hell below line mean?? */
	if (xf86GetPciVideoInfo () == NULL)
	{
		return (FALSE);
	}
#endif

	numUsed = xf86MatchPciInstances (SILICONMOTION_NAME, PCI_SMI_VENDOR_ID,
			SMIChipsets, SMIPciChipsets, devSections,
			numDevSections, drv, &usedChips);
	DEBUG("numUsed = %d\n",numUsed);

	/* Free it since we don't need that list after this */
	xfree (devSections);

	if (numUsed <= 0)
	{
		return (FALSE);
	}

	if (flags & PROBE_DETECT)
	{
		foundScreen = TRUE;
	}
	else
	{
		for (i = 0; i < numUsed; i++)
		{
			SMIRegPtr pReg;
			EntityInfoPtr pEnt;			
			ScrnInfoPtr pScrn;
			pScrn = NULL;

			if ((pScrn = xf86ConfigPciEntity (pScrn, 0, usedChips[i],
							SMIPciChipsets, 0, 0, 0, 0,0)))
			{

				/* 	monk : we only support two head per card now,  
					I think below code is well enough to handle dualview stuffs 
				 */	
				if(!entity_private[usedChips[i]]){
					pReg = entity_private[usedChips[i]] = xnfcalloc(sizeof(SMIRegRec),1);
					pReg->pPrimaryScrn = pScrn; 	
					pReg->DualHead = 0;
					pReg->lastInstance = 0; 				
				}else{
					pReg = entity_private[usedChips[i]];
					pReg->pSecondaryScrn = pScrn;
					pReg->DualHead = 1;
					pReg->lastInstance ++;
				}			

				/* SetEntitySharable must be called between two xf86ConfigPciEntity's calling for dual view situation */
				xf86SetEntitySharable(usedChips[i]);
				/* pass private and entityInit  to the entity */
				xf86SetEntityFuncs(usedChips[i],SMI_EntityInit,NULL,NULL,pReg);
				/* set screen instance id for pScrn */
				xf86SetEntityInstanceForScreen(pScrn, usedChips[i],pReg ->lastInstance);
#if 0
				if (pScrn->display != NULL) {
					xf86DrvMsg("", X_INFO, "pScrn->display->modes:\n"
							"FrameX0: %d, FrameY0: %d\n VirtualX: %d,\n"
							"virtualY: %d\nDepth: %d, fbbpp %d\n \n"
							"defaultVisual: %d\n",
							pScrn->display->frameX0,
							pScrn->display->frameY0, 
							pScrn->display->virtualX, 
							pScrn->display->virtualY, 
							pScrn->display->depth, pScrn->display->fbbpp, 
							pScrn->display->defaultVisual);
				} else {
					xf86DrvMsg("", X_INFO, "pScrn->display is null pointer\n");
				}
#endif				
				/* Fill in what we can of the ScrnInfoRec */
				pScrn->driverVersion = SILICONMOTION_DRIVER_VERSION;
				pScrn->driverName = SILICONMOTION_DRIVER_NAME;
				pScrn->name = SILICONMOTION_NAME;
				pScrn->Probe = SMI_Probe;
				pScrn->PreInit = SMI_PreInit;
				pScrn->ScreenInit = SMI_ScreenInit;
				pScrn->SwitchMode = SMI_SwitchMode;
				pScrn->AdjustFrame = SMI_AdjustFrame;
				pScrn->EnterVT = SMI_EnterVT;
				pScrn->LeaveVT = SMI_LeaveVT;
				pScrn->FreeScreen = SMI_FreeScreen;
				pScrn->ValidMode = SMI_ValidMode;
				foundScreen = TRUE;

			}
#if 0
			pEnt = xf86GetEntityInfo (usedChips[i]);			
			DEBUG("pEnt->index = %x,chipset = %x,active is %x\n",
					pEnt->index,pEnt->chipset,pEnt->active);

			switch(pEnt->chipset) {
				case PCI_CHIP_SMI501:
					xf86DrvMsg("", X_INFO, "501 found\n");
					entity_init_501(pScrn, usedChips[i]);
#ifdef XSERVER_LIBPCIACCESS
					pd = xf86GetPciInfoForEntity(pEnt->index);
					pci_device_probe(pd);
#endif
					break;
				case PCI_CHIP_SMI750:
					xf86DrvMsg("", X_INFO, "750 found\n");
					entity_init_750(pScrn, usedChips[i]);
#ifdef XSERVER_LIBPCIACCESS
					pd = xf86GetPciInfoForEntity(pEnt->index);
					//pci_device_probe(pd);
					pci_device_enable(pd);
#endif
					break;

				default:
					break;
			}
			xfree (pEnt);
#endif			
		}
	}

	xfree (usedChips);
	return (foundScreen);
}

static Bool
SMI_PreInit (ScrnInfoPtr pScrn, int flags)
{
	EntityInfoPtr pEntInfo;
	DevUnion * pEntPriv;
	SMIRegPtr pRegPtr;

	SMIPtr pSmi;
	MessageType from;
	int i, tmpvalue;
	double real;
	ClockRangePtr clockRanges;
	char *s;
	unsigned char config, m, n, shift;
	int mclk;
	vgaHWPtr hwp;
	int vgaCRIndex, vgaIOBase;
	DisplayModePtr p;
	vbeInfoPtr pVbe = NULL;
	ENTER();

	unsigned char Generic_EDID[] =
	{ 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x22, 0x64, 0x91, 0x89,
		0xE4, 0x12, 0x00, 0x00,
		0x16, 0x10, 0x01, 0x03, 0x80, 0x29, 0x1A, 0x78, 0x2A, 0x9B, 0xB6, 0xA4,
		0x53, 0x4B, 0x9D, 0x24,
		0x14, 0x4F, 0x54, 0xBF, 0xEF, 0x00, 0x31, 0x46, 0x61, 0x46, 0x71, 0x4F,
		0x81, 0x40, 0x81, 0x80,
		0x95, 0x00, 0x95, 0x0F, 0x01, 0x01, 0x9A, 0x29, 0xA0, 0xD0, 0x51, 0x84,
		0x22, 0x30, 0x50, 0x98,
		0x36, 0x00, 0x98, 0xFF, 0x10, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0xFD,
		0x00, 0x31, 0x4B, 0x1E,
		0x50, 0x0E, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00,
		0x00, 0xFC, 0x00, 0x48,
		0x57, 0x31, 0x39, 0x31, 0x44, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x36, 0x32, 0x32, 0x47, 0x48, 0x33, 0x32, 0x43, 0x41, 0x34, 0x38,
		0x33, 0x36, 0x00, 0x29
	};

	int chipType;

	if (pScrn->numEntities > 1)
	{
		return (FALSE);
	}

	/* Allocate the SMIRec driverPrivate */
	if (!SMI_GetRec (pScrn))
	{
		return (FALSE);
	}

	pSmi = SMIPTR (pScrn);

	int entityIndex = pScrn->entityList[0];

	pEntInfo = xf86GetEntityInfo(pScrn->entityList[0]);
	pEntPriv = xf86GetEntityPrivate(entityIndex,entity_priv_index[entityIndex]);
	pRegPtr = (SMIRegPtr)pEntPriv->ptr;
	//pSmi->pEnt_private = pRegPtr;

	/* Get the entity, and make sure it is PCI. */
	if (pEntInfo->location.type != BUS_PCI) {
		return (FALSE);
	}

	pSmi->pEnt_info = pEntInfo;
	pSmi->pEnt_private = pEntPriv;

	pSmi->PciInfo = xf86GetPciInfoForEntity (pEntInfo->index);



#ifndef XSERVER_LIBPCIACCESS
	chipType = pSmi->PciInfo->chipType;
#else
	chipType = PCI_DEV_DEVICE_ID(pSmi->PciInfo);	//caesar modified
#endif

	if (flags & PROBE_DETECT)
	{
		if ((SMI_MSOC != chipType) && (SMI_MSOCE != chipType)&&(pSmi->RemoveBIOS == FALSE))
		{
			xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Not sm502, chiptype is 0x%x\n", chipType);
			SMI_ProbeDDC (pScrn, xf86GetEntityInfo (pScrn->entityList[0])->index);
		}
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "SM502\n");
		return (TRUE);
	}

	//if(chipType == SMI_MSOCE && pSmi->IsPrimary)
	if(chipType == SMI_MSOCE && !pRegPtr->save750)
	{
		//pSmi->save750 = xnfcalloc(sizeof(SMI750RegRec),1);
		//if(!pSmi->save750 )
		//	return FALSE;
		pRegPtr->save750 = xnfcalloc(sizeof(SMI750RegRec),1);
		if(!pRegPtr->save750){
			LEAVE(FALSE);
		}
	}


	/* The vgahw module should be loaded here when needed */
	if (!xf86LoadSubModule (pScrn, "vgahw"))
	{
		return (FALSE);
	}

#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
	xf86LoaderReqSymLists (vgahwSymbols, NULL);
#endif
	/*
	 * Allocate a vgaHWRec
	 */
	if (!vgaHWGetHWRec (pScrn))
	{
		return (FALSE);
	}


	/* Set pScrn->monitor */
	pScrn->monitor = pScrn->confScreen->monitor;

	/*
	 * The first thing we should figure out is the depth, bpp, etc.  Our
	 * default depth is 8, so pass it to the helper function.  We support
	 * only 24bpp layouts, so indicate that.
	 */
	switch (chipType) {
		case SMI_MSOC:
			if (smi_setdepbpp_501(pScrn) == FALSE) {
				return FALSE;
			}
			break;
		case SMI_MSOCE:
			if (smi_setdepbpp_750(pScrn) == FALSE) {
				return FALSE;
			}
			break;
		default:
			if (smi_setdepbpp(pScrn) == FALSE) {
				return FALSE;
			}
			break;
	}

	xf86PrintDepthBpp (pScrn);

	/*
	 * This must happen after pScrn->display has been set because
	 * xf86SetWeight references it.
	 */
	if ((pScrn->depth > 8))	/*&&(pScrn->depth <= 24)) */
	{
		/* The defaults are OK for us */
		rgb BitsPerComponent = { 0, 0, 0 };
		rgb BitMask = { 0, 0, 0 };

		if (!xf86SetWeight (pScrn, BitsPerComponent, BitMask))
		{
			return (FALSE);
		}
	}
	if (!xf86SetDefaultVisual (pScrn, -1))
	{
		return (FALSE);
	}

	/* We don't currently support DirectColor at > 8bpp */
	if ((pScrn->depth > 8) && (pScrn->defaultVisual != TrueColor))
	{
		xf86DrvMsg (pScrn->scrnIndex, X_ERROR, "Given default visual (%s) "
				"is not supported at depth %d\n",
				xf86GetVisualName (pScrn->defaultVisual), pScrn->depth);
		return (FALSE);
	}

	/* We use a programmable clock */
	pScrn->progClock = TRUE;

	/* Collect all of the relevant option flags (fill in pScrn->options) */
	xf86CollectOptions (pScrn, NULL);


	/* Set the bits per RGB for 8bpp mode */
	if (pScrn->depth == 8)
	{
		if ((SMI_MSOC == chipType) || (SMI_MSOCE == chipType))
		{
			pScrn->rgbBits = 8;
		}
		else
		{
			pScrn->rgbBits = 6;
		}
	}

	/* Process the options */
	if (!(pSmi->Options = xalloc (sizeof (SMIOptions))))
		return FALSE;

	memcpy (pSmi->Options, SMIOptions, sizeof (SMIOptions));
	xf86ProcessOptions (pScrn->scrnIndex, pScrn->options, pSmi->Options);

	if (xf86ReturnOptValBool (pSmi->Options, OPTION_PCI_BURST, FALSE))
	{
		pSmi->pci_burst = TRUE;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: pci_burst - PCI burst "
				"read enabled\n");
	}
	else
	{
		pSmi->pci_burst = FALSE;
	}


	/*For CSC video*/
	if (xf86ReturnOptValBool (pSmi->Options, OPTION_CSCVIDEO, FALSE))
	{
		pSmi->IsCSCVideo = TRUE;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option:  csc video "
				"read enabled\n");
	}
	else
	{
		pSmi->IsCSCVideo = FALSE;
	}

	pSmi->NoPCIRetry = TRUE;
	if (xf86ReturnOptValBool (pSmi->Options, OPTION_PCI_RETRY, FALSE))
	{
		if (xf86ReturnOptValBool (pSmi->Options, OPTION_PCI_BURST, FALSE))
		{
			pSmi->NoPCIRetry = FALSE;
			xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: pci_retry\n");
		}
		else
		{
			xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "\"pci_retry\" option "
					"requires \"pci_burst\".\n");
		}
	}

	if (xf86IsOptionSet (pSmi->Options, OPTION_FIFO_CONSERV))
	{
		pSmi->fifo_conservative = TRUE;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: fifo_conservative "
				"set\n");
	}
	else
	{
		pSmi->fifo_conservative = FALSE;
	}

	if (xf86IsOptionSet (pSmi->Options, OPTION_FIFO_MODERATE))
	{
		pSmi->fifo_moderate = TRUE;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: fifo_moderate set\n");
	}
	else
	{
		pSmi->fifo_moderate = FALSE;
	}

	if (xf86IsOptionSet (pSmi->Options, OPTION_FIFO_AGGRESSIVE))
	{
		pSmi->fifo_aggressive = TRUE;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG,
				"Option: fifo_aggressive set\n");
	}
	else
	{
		pSmi->fifo_aggressive = FALSE;
	}

	if (xf86ReturnOptValBool (pSmi->Options, OPTION_NOACCEL, FALSE))
	{
		pSmi->NoAccel = TRUE;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG,
				"Option: NoAccel - Acceleration " "disabled\n");
	}
	else
	{
		pSmi->NoAccel = FALSE;
	}

	if (xf86ReturnOptValBool (pSmi->Options, OPTION_SHOWCACHE, FALSE))
	{
		pSmi->ShowCache = TRUE;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: show_cache set\n");
	}
	else
	{
		pSmi->ShowCache = FALSE;
	}

	if (xf86GetOptValFreq (pSmi->Options, OPTION_MCLK, OPTUNITS_MHZ, &real))
	{
		pSmi->MCLK = (int) (real * 1000.0);
		if((chipType != SMI_MSOC) && (chipType != SMI_MSOCE))
		{
			if (pSmi->MCLK <= 120000)
			{
				xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: set_mclk set to "
						"%1.3f MHz\n", pSmi->MCLK / 1000.0);
			}
			else
			{
				xf86DrvMsg (pScrn->scrnIndex, X_WARNING, "Memory Clock value of "
						"%1.3f MHz is larger than limit of 120 MHz\n",
						pSmi->MCLK / 1000.0);
				pSmi->MCLK = 0;
			}
		}
	}
	else
	{
		pSmi->MCLK = 0;
	}

	from = X_DEFAULT;	
	/* 
	   if no HWCursor option defined in xorg.conf ,then software cursor will be used 
	   by default
	 */
	pSmi->hwcursor = FALSE;
	if (xf86GetOptValBool (pSmi->Options, OPTION_HWCURSOR, &pSmi->hwcursor)){
		from = X_CONFIG;
	}
#if 0	
	else if(xf86ReturnOptValBool (pSmi->Options, OPTION_SWCURSOR, FALSE)){
		pSmi->hwcursor = FALSE;
		from = X_CONFIG;
	}
#endif	

#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,5,1,0,0)

	if(pScrn->scrnIndex != 0)
	{
		if(pScrn->depth != xf86Screens[0]->depth){
			/*	for X server 1.51+ , use ugly hardware cursor for none-primary screen or system will hang 
				kind of X server bug		*/
			pSmi->hwcursor = TRUE;
		}				
	}


#endif	

	if (xf86GetOptValBool (pSmi->Options, OPTION_CLONE_MODE, &pSmi->clone_mode))
	{
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Clone mode %s.\n",
				pSmi->clone_mode ? "enabled" : "disabled");
		if (pSmi->clone_mode){
			pSmi->hwcursor = FALSE; 
			xf86DrvMsg(pScrn->scrnIndex, X_INFO, "No hardware cursor in clone mode\n");  
			if (pScrn->bitsPerPixel != 16) {
				xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "CloneMode only supported at "
						"depth 16\n");
				return (FALSE);
			}
		}
	}

	xf86DrvMsg (pScrn->scrnIndex, from, "Using %s Cursor\n",
			pSmi->hwcursor ? "Hardware" : "Software");

	if (xf86GetOptValBool (pSmi->Options, OPTION_SHADOW_FB, &pSmi->shadowFB))
	{
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "ShadowFB %s.\n",
				pSmi->shadowFB ? "enabled" : "disabled");
	}

#if 0				/* PDR#932 */
	if ((pScrn->depth == 8) || (pScrn->depth == 16))
#endif /* PDR#932 */
		if ((s = xf86GetOptValString (pSmi->Options, OPTION_ROTATE)))
		{
			if (!xf86NameCmp (s, "CCW"))
			{
				pSmi->shadowFB = TRUE;
				pSmi->rotate = SMI_ROTATE_CCW;
				xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Rotating screen "
						"clockwise\n");
			}
			else if (!xf86NameCmp (s, "CW"))
			{
				pSmi->shadowFB = TRUE;
				pSmi->rotate = SMI_ROTATE_CW;
				xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Rotating screen counter "
						"clockwise\n");
			}
			else if (!xf86NameCmp (s, "UD"))
			{
				pSmi->shadowFB = TRUE;
				pSmi->rotate = SMI_ROTATE_UD;
				xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Rotating screen half "
						"clockwise\n");
			}	
			else if(!xf86NameCmp(s, "RandR")) 
			{
#ifdef RANDR
				pSmi->shadowFB = TRUE;
				pSmi->RandRRotation = TRUE;
				pSmi->rotate = SMI_ROTATE_ZERO;	
				//		pSmi->rotate = SMI_ROTATE_CW;
				xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
						"Using RandR rotation - acceleration disabled\n");
#else
				xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
						"This driver was not compiled with support for the Resize and "
						"Rotate extension.  Cannot honor 'Option \"Rotate\" "
						"\"RandR\"'.\n");
#endif
			}
			else
			{
				xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "\"%s\" is not a valid "
						"value for Option \"Rotate\"\n", s);
				xf86DrvMsg (pScrn->scrnIndex, X_INFO,
						"Valid options are \"CW\" or " "\"CCW\" or " "\"UD\" or " "\"RandR\"\n");
			}
		}

	if (pSmi->rotate)
	{
		/* Disable XF86 rotation, it hoses up SMI rotation */
		xf86DisableRandR ();
	}

	if (xf86GetOptValInteger (pSmi->Options, OPTION_VIDEOKEY, &pSmi->videoKey))
	{
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: Video key set to "
				"0x%08X\n", pSmi->videoKey);
	}
	else
	{
		pSmi->videoKey = (1 << pScrn->offset.red) | (1 << pScrn->offset.green)
			| (((pScrn->mask.blue >> pScrn->offset.blue) - 1)
					<< pScrn->offset.blue);
	}

	if (xf86ReturnOptValBool (pSmi->Options, OPTION_BYTESWAP, FALSE))
	{
		pSmi->ByteSwap = TRUE;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: ByteSwap enabled.\n");
	}
	else
	{
		pSmi->ByteSwap = FALSE;
	}

	/* CZ 26.10.2001: interlaced video */
	if (xf86ReturnOptValBool (pSmi->Options, OPTION_INTERLACED, FALSE))
	{
		pSmi->interlaced = TRUE;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG,
				"Option: Interlaced enabled.\n");
	}
	else
	{
		pSmi->interlaced = FALSE;
	}
	/* end CZ */

	if (xf86GetOptValBool (pSmi->Options, OPTION_USEBIOS, &pSmi->useBIOS))
	{
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: UseBIOS %s.\n",
				pSmi->useBIOS ? "enabled" : "disabled");
	}
	else
	{
		/* Default to UseBIOS enabled. */
		pSmi->useBIOS = FALSE;
	}
	if(xf86GetOptValBool (pSmi->Options, OPTION_REMOVEBIOS, &pSmi->RemoveBIOS))
	{
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: RemoveBIOS %s.\n",
				pSmi->RemoveBIOS? "enabled" : "disabled");
	}
	else
	{
		pSmi->RemoveBIOS = FALSE;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: RemoveBIOS %s.\n",
				pSmi->RemoveBIOS? "enabled" : "disabled");
	}

	if (xf86GetOptValBool (pSmi->Options, OPTION_ZOOMONLCD, &pSmi->zoomOnLCD))
	{
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: ZoomOnLCD %s.\n",
				pSmi->zoomOnLCD ? "enabled" : "disabled");
	}
	else
	{
		/* Default to ZoomOnLCD enabled. */
		pSmi->zoomOnLCD = TRUE;
	}
	if (xf86GetOptValBool (pSmi->Options, OPTION_PANELHEIGHT, &pSmi->lcdHeight))
	{
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: LCDHeight %d.\n",
				pSmi->lcdHeight);
	}
	else
	{
		/* Default to ZoomOnLCD enabled. */
		pSmi->lcdHeight = 0;
	}
	if (xf86GetOptValBool (pSmi->Options, OPTION_PANELWIDTH, &pSmi->lcdWidth))
	{
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: LCDWidth %d.\n",
				pSmi->lcdWidth);
	}
	else
	{
		/* Default to ZoomOnLCD enabled. */
		pSmi->lcdWidth = 0;
	}
	if(xf86GetOptValBool (pSmi->Options, OPTION_EDID, &pSmi->edid_enable))
	{
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: EDID_EN %s.\n",
				pSmi->edid_enable? "enabled" : "disabled");
	}
	else
	{
		pSmi->edid_enable = 0;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: EDID_EN %s.\n",
				pSmi->edid_enable? "enabled" : "disabled");
	}

	/* Find the PCI slot for this screen */
	pEntInfo = xf86GetEntityInfo (pScrn->entityList[0]);
	if ((pEntInfo->location.type != BUS_PCI) 
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
			|| (pEntInfo->resources)
#endif
	   )
	{
		xfree (pEntInfo);
		SMI_FreeRec (pScrn);
		return (FALSE);
	}

	if (chipType != SMI_MSOCE && chipType != SMI_MSOC && pSmi->RemoveBIOS == FALSE )		
	{

		if (xf86LoadSubModule (pScrn, "int10"))
		{
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
			xf86LoaderReqSymLists (int10Symbols, NULL);
#endif
			pSmi->pInt10 = xf86InitInt10 (pEntInfo->index);
		}

		if (pSmi->pInt10 && xf86LoadSubModule (pScrn, "vbe"))
		{
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
			xf86LoaderReqSymLists (vbeSymbols, NULL);
#endif
			pVbe = VBEInit (pSmi->pInt10, pEntInfo->index);
		}
	}
	else if(pSmi->RemoveBIOS == TRUE) 
	{
		SMI_RegInit(pScrn); //this function will init reg when bios remove by david
	}
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
	xf86RegisterResources (pEntInfo->index, NULL, ResExclusive);
#endif
	/*	xf86SetOperatingState(resVgaIo, pEnt->index, ResUnusedOpr); */
	/*	xf86SetOperatingState(resVgaMem, pEnt->index, ResDisableOpr); */

	/*
	 * Set the Chipset and ChipRev, allowing config file entries to
	 * override.
	 */
	if (pEntInfo->device->chipset && *pEntInfo->device->chipset)
	{
		pScrn->chipset = pEntInfo->device->chipset;
		pSmi->Chipset = xf86StringToToken (SMIChipsets, pScrn->chipset);
		from = X_CONFIG;
	}
	else if (pEntInfo->device->chipID >= 0)
	{
#ifndef XSERVER_LIBPCIACCESS    
		pSmi->Chipset = pEntInfo->device->chipID;
#else
		pSmi->Chipset = PCI_DEV_DEVICE_ID(pSmi->PciInfo);	//caesar modified
#endif
		pScrn->chipset =
			(char *) xf86TokenToString (SMIChipsets, pSmi->Chipset);
		from = X_CONFIG;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "ChipID override: 0x%04X\n",
				pSmi->Chipset);
	}
	else
	{
		from = X_PROBED;
#ifndef XSERVER_LIBPCIACCESS  	  
		pSmi->Chipset = pSmi->PciInfo->chipType;
#else
		pSmi->Chipset =   PCI_DEV_DEVICE_ID(pSmi->PciInfo); //caesar modified
#endif
		pScrn->chipset =
			(char *) xf86TokenToString (SMIChipsets, pSmi->Chipset);
	}

	if (pEntInfo->device->chipRev >= 0)
	{
		pSmi->ChipRev = pEntInfo->device->chipRev;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "ChipRev override: %d\n",
				pSmi->ChipRev);
	}
	else
	{
#ifndef XSERVER_LIBPCIACCESS    
		pSmi->ChipRev = pSmi->PciInfo->chipRev;
#else
		pSmi->ChipRev = PCI_DEV_REVISION(pSmi->PciInfo);	//caesar modified
#endif
	}
	xfree (pEntInfo);

	/*
	 * This shouldn't happen because such problems should be caught in
	 * SMI_Probe(), but check it just in case.
	 */
	if (pScrn->chipset == NULL)
	{
		xf86DrvMsg (pScrn->scrnIndex, X_ERROR, "ChipID 0x%04X is not "
				"recognised\n", pSmi->Chipset);
		return (FALSE);
	}
	if (pSmi->Chipset < 0)
	{
		xf86DrvMsg (pScrn->scrnIndex, X_ERROR, "Chipset \"%s\" is not "
				"recognised\n", pScrn->chipset);
		return (FALSE);
	}

	xf86DrvMsg (pScrn->scrnIndex, from, "Chipset: \"%s\"\n", pScrn->chipset);

#ifndef XSERVER_LIBPCIACCESS
	pSmi->PciTag = pciTag (pSmi->PciInfo->bus, pSmi->PciInfo->device,pSmi->PciInfo->func);
	pci_tag = pSmi->PciTag;
	config = pciReadByte(pSmi->PciTag, PCI_CMD_STAT_REG);
	pciWriteByte(pSmi->PciTag, PCI_CMD_STAT_REG, config | PCI_CMD_MEM_ENABLE);
#endif

	/*boyod */

	pSmi->IsSecondary = FALSE;
	pSmi->IsPrimary = TRUE;
	pSmi->IsLCD = TRUE;
	pSmi->IsCRT = FALSE;

	switch (chipType) {
		case SMI_MSOC:
			smi_set_dualhead_501(pScrn, pSmi);
			break;
		case SMI_MSOCE:
			smi_set_dualhead_750(pScrn, pSmi);
			break;
		default:
			break;
	}

	/* And compute the amount of video memory and offscreen memory */
	if(chipType != SMI_MSOCE){
		pSmi->videoRAMKBytes = 0;

		config = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x71);

		if (!pScrn->videoRam) {
			switch(pSmi->Chipset) {		
				case SMI_MSOC:
					smi_setvideomem_501(config, pScrn, pSmi);
					break;
#ifndef XSERVER_LIBPCIACCESS			
				case SMI_LYNX3D:
					smi_setvideomem_820(config, pScrn, pSmi);
					break;

				case SMI_LYNX3DM:
					smi_setvideomem_720(config, pScrn, pSmi);
					break;

				case SMI_COUGAR3DR:
					smi_setvideomem_730(config, pScrn, pSmi);
					break;
				case SMI_LYNXEM:
				case SMI_LYNXEMplus:
					smi_setvideomem_712(config, pScrn, pSmi);
					break;
#endif
				case SMI_MSOCE:
					//smi_setvideomem_750(config, pScrn, pSmi);
					break;
				default:
					smi_setvideomem(config, pScrn, pSmi);
					break;
			}
		} else {
			pSmi->videoRAMKBytes = pScrn->videoRam;
			pSmi->videoRAMBytes = pScrn->videoRam * 1024;
		}
	}

	SMI_MapMem (pScrn);

	/*
	 * 750 multi card will halt if not add:chipType != SMI_MSOCE 
	 * actually, the second card will halt
	 * I suspect the reason is that second card should not touch IO
	 * So SMI_DisableVideo should not be called by second card
	 */
	if (chipType != SMI_MSOC && chipType != SMI_MSOCE)
	{
		SMI_DisableVideo (pScrn);
		hwp = VGAHWPTR (pScrn);
		vgaIOBase = hwp->IOBase;
		vgaCRIndex = vgaIOBase + VGA_CRTC_INDEX_OFFSET;

		/*	    vgaCRReg   = vgaIOBase + VGA_CRTC_DATA_OFFSET;  */
		pSmi->PIOBase = hwp->PIOOffset;

		XINF("%s:vgaCRIndex=%x, vgaIOBase=%x,MMIOBase=%x PIOBase=%x\n",
				__func__,vgaCRIndex, vgaIOBase, hwp->MMIOBase,hwp->PIOOffset);

#ifdef _XSERVER64

		/*  Fake the reading of a valid EDID block to allow the population of the
			default configuration file upon startup. */

		if (xf86LoadSubModule (pScrn, "i2c"))
		{
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
			xf86LoaderReqSymLists (i2cSymbols, NULL);
#endif
			SMI_I2CInit (pScrn);	/* IGXMAL DEBUG */
			xf86ErrorFVerb (VERBLEV, "\tSMI_I2CInit ");
		}


		if (xf86LoadSubModule (pScrn, "ddc"))
		{
			xf86MonPtr pMon = NULL;

#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
			xf86LoaderReqSymLists (ddcSymbols, NULL);
#endif
			{

				unsigned char *EDID_Block = Generic_EDID;
				pMon = xf86InterpretEDID (pScrn, EDID_Block);

				if (pMon != NULL)
				{
					pMon = xf86PrintEDID (pMon);
					if (pMon != NULL)
					{
						xf86SetDDCproperties (pScrn, pMon);
					}
				}
			}
		}

#else


		if (xf86LoadSubModule (pScrn, "i2c"))
		{
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
			xf86LoaderReqSymLists (i2cSymbols, NULL);
#endif
			SMI_I2CInit (pScrn);
		}

		if (xf86LoadSubModule (pScrn, "ddc"))
		{
			xf86MonPtr pMon = NULL;

#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
			xf86LoaderReqSymLists (ddcSymbols, NULL);
#endif
#if 1				/* PDR#579 */
			if (pVbe)
			{
				pMon = vbeDoEDID (pVbe, NULL);
				if (pMon != NULL)
				{
					if ((pMon->rawData[0] == 0x00)
							&& (pMon->rawData[1] == 0xFF)
							&& (pMon->rawData[2] == 0xFF)
							&& (pMon->rawData[3] == 0xFF)
							&& (pMon->rawData[4] == 0xFF)
							&& (pMon->rawData[5] == 0xFF)
							&& (pMon->rawData[6] == 0xFF)
							&& (pMon->rawData[7] == 0x00))
					{
						pMon = xf86PrintEDID (pMon);
						if (pMon != NULL)
						{
							xf86SetDDCproperties (pScrn, pMon);
						}
					}
				}
			}
#else
			if ((pVbe)
					&& ((pMon = xf86PrintEDID (vbeDoEDID (pVbe, NULL))) != NULL))
			{
				xf86SetDDCproperties (pScrn, pMon);
			}
#endif
			else if (!SMI_ddc1 (pScrn->scrnIndex))
			{
				if (pSmi->I2C)
				{
					xf86SetDDCproperties (pScrn,
							xf86PrintEDID (xf86DoEDID_DDC2
								(pScrn->scrnIndex,
								 pSmi->I2C)));
				}
			}
		}
		if(pVbe)
			vbeFree (pVbe);
		if(pSmi->pInt10)
			xf86FreeInt10 (pSmi->pInt10);
		pSmi->pInt10 = NULL;

#endif
	}				/* End !SMI_MSOC */
	else if ( chipType == SMI_MSOCE)
	{
		if (xf86LoadSubModule (pScrn, "i2c"))
		{
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
			xf86LoaderReqSymLists (i2cSymbols, NULL);
#endif
			SMI750_I2cInit (pScrn);
		}

		if (xf86LoadSubModule (pScrn, "ddc"))
		{
			xf86MonPtr pMon = NULL;

#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
			xf86LoaderReqSymLists (ddcSymbols, NULL);
#endif
#if 1				/* PDR#579 */
			if (pVbe)
			{
				pMon = vbeDoEDID (pVbe, NULL);
				if (pMon != NULL)
				{
					if ((pMon->rawData[0] == 0x00)
							&& (pMon->rawData[1] == 0xFF)
							&& (pMon->rawData[2] == 0xFF)
							&& (pMon->rawData[3] == 0xFF)
							&& (pMon->rawData[4] == 0xFF)
							&& (pMon->rawData[5] == 0xFF)
							&& (pMon->rawData[6] == 0xFF)
							&& (pMon->rawData[7] == 0x00))
					{
						pMon = xf86PrintEDID (pMon);
						if (pMon != NULL)
						{
							xf86SetDDCproperties (pScrn, pMon);
						}
					}
				}
			}
			else if(pSmi->I2C)  
			{
				I2CBusPtr bus;		
				int is_crt = 0;
				is_crt = pSmi->IsCRT;
				xf86DrvMsg(pScrn->scrnIndex, X_INFO,
						"david : Isprime:%d\n IsSecond:%d\n IsCRT:%d\n IsLCD:%d\n", pSmi->IsPrimary, pSmi->IsSecondary, pSmi->IsCRT, pSmi->IsLCD);
				for(is_crt; is_crt<2; is_crt++)
				{
					SMI_750_swI2CInit(pScrn, is_crt); //xf86 use sw i2c
					bus = (pSmi->IsPrimary&is_crt)? pSmi->I2C_secondary : pSmi->I2C_primary;
					if(bus)
					{
						pMon = xf86DoEDID_DDC2(pScrn->scrnIndex, bus);
						if (pMon)
						{
							pMon = xf86PrintEDID (pMon);
	                    	 			xf86SetDDCproperties (pScrn,pMon);
							break;
						}
						else
						{
							xf86DrvMsg(pScrn->scrnIndex, X_INFO,
							"detect edid by ddk\n");
				                	unsigned char retry, edidisok;
							int offset;
							unsigned char* edidBuffer;
							edidBuffer = xcalloc(1, 128);
							pSmi->ediddata = edidBuffer;
							edidisok = 0;
							for (retry = 0; retry < 8; retry++)
							{
							    if(retry < 4)//try sw frist
							    {
								/* Read the EDID from the monitor. */
								for (offset = 0; offset < 128; offset++)
								edidBuffer[offset] = swI2CReadReg(0xa0, (unsigned char)offset);
							    }
								else if(!is_crt)//hw i2c only support dvi
								{

										if (retry == 4)// try hard ware if sw i2c failed
										{
											xf86DrvMsg(pScrn->scrnIndex, X_INFO,
											"hardware detect edid by ddk\n");
											hwI2CInit(1);
										}
										for (offset = 0; offset < 128; offset++)
									            edidBuffer[offset] = hwI2CReadReg(0xa0, (unsigned char)offset);
								}
								if ((edidBuffer[0] == 0x00)
									&& (edidBuffer[1] == 0xFF)
									&& (edidBuffer[2] == 0xFF)
									&& (edidBuffer[3] == 0xFF)
									&& (edidBuffer[4] == 0xFF)
									&& (edidBuffer[5] == 0xFF)
									&& (edidBuffer[6] == 0xFF)
									&& (edidBuffer[7] == 0x00))
									{
										edidisok = 1;
										break;
									}

								}
								if (edidisok)
								{
									pMon=xf86InterpretEDID(pScrn->scrnIndex, edidBuffer);
									if (pMon != NULL)
									{
										pMon = xf86PrintEDID (pMon);
										//pScrn->monitor->nVrefresh = 60;

										xf86SetDDCproperties (pScrn,pMon);
										
										pScrn->monitor->Modes = xf86DDCGetModes(pScrn->scrnIndex , pMon);
										DisplayModePtr      Mode = pScrn->monitor->Modes;
										xf86DrvMsg(pScrn->scrnIndex, X_INFO,
												"second modeline star\n");
										while (Mode) {

											xf86PrintModeline(pScrn->scrnIndex, Mode);
											Mode = Mode->next;
										}
										xf86DrvMsg(pScrn->scrnIndex, X_INFO,
												"second modeline end\n");
										

										//pScrn->monitor->DDC = pMon;
										// DisplayModePtr pMode = xf86DDCGetModes(pScrn->scrnIndex , pMon);
										//SMI_DDCGetModes(pMode, &mode_table_edid);
									}
									break;
								}
								else
								{
									xf86DrvMsg(pScrn->scrnIndex, X_INFO,
											"this driver cannot detect EDID Version\n");
								}
							}

					}
				}

			}
#else
			if ((pVbe)
					&& ((pMon = xf86PrintEDID (vbeDoEDID (pVbe, NULL))) != NULL))
			{
				xf86SetDDCproperties (pScrn, pMon);
			}

#endif
			//else if(pSmi->I2C_primary)
			//{
			//	pMon=xf86OutputGetEDID(output,pSmi->I2C_primary);
			//	if(pMon)
			//	{
			//		xf86OutputSetEDID(output,pMon);
			//	}
			//}
		}
	}

	/*
	 * If the driver can do gamma correction, it should call xf86SetGamma()
	 * here. (from MGA, no ViRGE gamma support yet, but needed for
	 * xf86HandleColormaps support.)
	 */
	{
		Gamma zeros = { 0.0, 0.0, 0.0 };

		if (!xf86SetGamma (pScrn, zeros))
		{
			SMI_EnableVideo (pScrn);
			SMI_UnmapMem (pScrn);
			return (FALSE);
		}
	}


	/* Lynx built-in ramdac speeds */
	pScrn->numClocks = 4;

	if ((pScrn->clock[3] <= 0) && (pScrn->clock[2] > 0))
	{
		pScrn->clock[3] = pScrn->clock[2];
	}

	switch (pSmi->Chipset) {
		case SMI_LYNX3DM:
		case SMI_COUGAR3DR:
		case SMI_MSOC:
		case SMI_MSOCE:		
			smi_setclk(pScrn, 200000, 200000, 200000, 200000);
			break;

		default:
			smi_setclk(pScrn, 135000, 135000, 135000, 135000);
			break;
	}
	/*
	   if ((pSmi->Chipset == SMI_LYNX3DM) ||
	   (pSmi->Chipset == SMI_COUGAR3DR) || (pSmi->Chipset == SMI_MSOC))
	   {
	   if (pScrn->clock[0] <= 0)
	   pScrn->clock[0] = 200000;
	   if (pScrn->clock[1] <= 0)
	   pScrn->clock[1] = 200000;
	   if (pScrn->clock[2] <= 0)
	   pScrn->clock[2] = 200000;
	   if (pScrn->clock[3] <= 0)
	   pScrn->clock[3] = 200000;
	   }
	   else
	   {
	   if (pScrn->clock[0] <= 0)
	   pScrn->clock[0] = 135000;
	   if (pScrn->clock[1] <= 0)
	   pScrn->clock[1] = 135000;
	   if (pScrn->clock[2] <= 0)
	   pScrn->clock[2] = 135000;
	   if (pScrn->clock[3] <= 0)
	   pScrn->clock[3] = 135000;
	   }
	 */

	/* Now set RAMDAC limits */
	switch (pSmi->Chipset)
	{
		default:
			pSmi->minClock = 12000;
			/*			pSmi->maxClock = 270000;*/
			pSmi->maxClock = 270000;

			break;
	}
	xf86ErrorFVerb (VERBLEV, "\tSMI_PreInit minClock=%d, maxClock=%d\n",
			pSmi->minClock, pSmi->maxClock);

	if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
	{

		/* Detect current MCLK and print it for user */
		m = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6A);
		n = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6B);
		switch (n >> 6)
		{
			default:
				shift = 1;
				break;

			case 1:
				shift = 4;
				break;

			case 2:
				shift = 2;
				break;
		}
		n &= 0x3F;
		mclk = ((1431818 * m) / n / shift + 50) / 100;
		xf86DrvMsg (pScrn->scrnIndex, X_PROBED,
				"Detected current MCLK value of " "%1.3f MHz\n",
				mclk / 1000.0);
	}

	SMI_EnableVideo (pScrn);
	SMI_UnmapMem (pScrn);
	xf86DrvMsg("", X_INFO, "pScrn->virtualX is %d, pScrn->display->virtualX is %d\n", pScrn->virtualX, pScrn->display->virtualX);
	pScrn->virtualX = pScrn->display->virtualX;

	/*
	 * Setup the ClockRanges, which describe what clock ranges are available,
	 * and what sort of modes they can be used for.
	 */
	clockRanges = xnfcalloc (sizeof (ClockRange), 1);
	clockRanges->next = NULL;

	clockRanges->minClock = pSmi->minClock;
	clockRanges->maxClock = pSmi->maxClock;
	clockRanges->clockIndex = -1;
	clockRanges->interlaceAllowed = FALSE;
	clockRanges->doubleScanAllowed = FALSE;


	/*boyod */
	/*             tmpvalue = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31) & ~0xCD;
				   VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31, tmpvalue );
	 */
#if 0
	xf86DrvMsg ("", X_INFO,
			"Belcon: before xf86ValidateModes, Monitor->Modes->HDisplay is %d\n",
			pScrn->monitor->Modes->HDisplay);
	xf86DrvMsg ("", X_INFO,
			"Belcon: before xf86ValidateModes,minClock is %d(kHz), maxClock is %d(kHz)\n",
			clockRanges->minClock, clockRanges->maxClock);
	// Belcon Debug
	for (p = pScrn->monitor->Modes; p != NULL; p = p->next) {
		xf86DrvMsg("", X_INFO, "Belcon Debug: Monitor mode name is %s\n", p->name);
	}
	for (p = pScrn->display->modes; p != NULL; p = p->next) {
		xf86DrvMsg("", X_INFO, "Belcon Debug: Request mode name is %s\n", p->name);
	}
#endif

	xf86DrvMsg (pScrn->scrnIndex, X_INFO, "BDEBUG pScrn->display->virtualX is \
			%d, pScrn->virtualX is %d\n", pScrn->display->virtualX, 
			pScrn->virtualX);
	i = xf86ValidateModes (pScrn,	/* Screen pointer */
			//  i = smi_xf86ValidateModes (pScrn,	/* Screen pointer  */
	  pScrn->monitor->Modes,	/* Available monitor modes  */
	  pScrn->display->modes,	/* req mode names for screen */
	  clockRanges,	/* list of clock ranges allowed */
	  NULL,	/* use min/max below */
	  128,	/* min line pitch (width) */
	  4096,	/* maximum line pitch (width) */
	  128,	/* bits of granularity for line pitch */
	  /* (width) above */
	  128,	/* min virtual height */
	  4096,	/* max virtual height */
	  pScrn->display->virtualX,	/* force virtual x */
	  pScrn->display->virtualY,	/* force virtual Y */
	  pSmi->videoRAMBytes,	/* size of aperture used to access */
	  /* video memory */
	  LOOKUP_BEST_REFRESH);	/* how to pick modes */


			xf86DrvMsg("", X_INFO, "displayWidth is %d\n", pScrn->displayWidth);
			pScrn->displayWidth = (pScrn->virtualX + 15) & ~15;
			xf86DrvMsg("", X_INFO, "displayWidth is %d\n", pScrn->displayWidth);
		//	pScrn->virtualX = pScrn->modes ->HDisplay;
		//	pScrn->virtualY = pScrn->modes->VDisplay;
		//	pScrn->display->virtualX = pScrn->modes ->HDisplay;
		//	pScrn->display->virtualY = pScrn->modes ->VDisplay;

#if 0
			if (pSmi->Chipset == SMI_MSOC)
			{
				SMIRegPtr	pSMIEnt = SMIEntPriv (pScrn);
				if (pSMIEnt->DualHead == TRUE) {
					pScrn->videoRam = (pScrn->displayWidth * pScrn->virtualY * pScrn->bitsPerPixel) / (1024 * 8);
				} else {
					pScrn->videoRam = free_video_memory;
				}
				/*
				   } else {
				   pScrn->videoRam = free_video_memory;
				   }
				 */
				//	pScrn->videoRam = (pScrn->virtualX * pScrn->virtualY * pScrn->bitsPerPixel) / (1024 * 8) + ( pSmi->IsPrimary ? 170 : 0);
				xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: PreInit, free_video_memory is %d\n", free_video_memory);
				free_video_memory -= (pScrn->videoRam + 4);
				xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: PreInit, free_video_memory is %d\n", free_video_memory);
				pSmi->videoRAMKBytes = (pScrn->videoRam + 4);
				pSmi->videoRAMBytes = pSmi->videoRAMKBytes * 1024;
				pSmi->fbMapOffset = (pScrn->videoRam + 4) * 1024;
				pSmi->FBReserved = pSmi->videoRAMBytes - 4096;
				pSmi->FBCursorOffset = pSmi->videoRAMBytes - 2048;
				if (!pSmi->IsSecondary) {
					pSmi->FBOffset = 0;
					//	pScrn->fbOffset = pSmi->FBOffset + pSmi->fbMapOffset;
					pScrn->fbOffset = pSmi->FBOffset;
					pSmi->fbMapOffset = 0;
					xf86DrvMsg(pScrn->scrnIndex, X_INFO, "primary pSmi is %p, pScrn is %p\n", pSmi, pScrn);
				} else {
					ScrnInfoPtr	primaryScrn = pSMIEnt->pPrimaryScrn;
					SMIRegPtr	primarySMIEnt = SMIEntPriv(primaryScrn);
					SMIPtr		primarySmi = SMIPTR(primaryScrn);

					pSmi->FBOffset += free_video_memory * 1024;
					pScrn->fbOffset = pSmi->FBOffset;
					pSmi->fbMapOffset = pSmi->FBOffset;


					xf86DrvMsg(pScrn->scrnIndex, X_INFO, "  primary pSmi is %p, pScrn is %p\n", primarySmi, primaryScrn);
					xf86DrvMsg(pScrn->scrnIndex, X_INFO, "  primary pScrn->fbOffset is 0x%x, secondary fbOffset is 0x%x\n", primaryScrn->fbOffset, pScrn->fbOffset);
					primaryScrn->videoRam = (pScrn->fbOffset - primaryScrn->fbOffset) / 1024 - 4;
					xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Re-count video memroy size for primary head. %dK\n", primaryScrn->videoRam);
					primarySmi->videoRAMKBytes = primaryScrn->videoRam + 4;
					primarySmi->videoRAMBytes = primaryScrn->videoRam * 1024;
					primarySmi->FBReserved = primarySmi->videoRAMBytes - 4096;
					primarySmi->FBCursorOffset = pSmi->videoRAMBytes - 2048;

				}
				xf86DrvMsg(pScrn->scrnIndex, X_INFO, "LINE: %d, pScrn->fbOffset is 0x%x\n", __LINE__, pScrn->fbOffset);
}
#endif
xf86DrvMsg (pScrn->scrnIndex, X_INFO, "BDEBUG pScrn->display->virtualX is %d, pScrn->virtualX is %d, pScrn->virtualY is %d, pScrn->bitsPerPixel is %d, pScrn->depth is %d\n", pScrn->display->virtualX, pScrn->virtualX, pScrn->virtualY, pScrn->bitsPerPixel, pScrn->depth);
xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG pScrn->videoRam is %d\n", pScrn->videoRam);
#if 0
// Belcon Debug
for (p = pScrn->modes; p != NULL; p = p->next) {
	xf86DrvMsg("", X_INFO, "Belcon Debug: mode name is %s\n", p->name);
	xf86DrvMsg("", X_INFO, "Belcon Debug: status is %s\n", xf86ModeStatusToString(p->status));
}
#endif

if (i == -1)
{
	SMI_FreeRec (pScrn);
	return (FALSE);
}

/* Prune the modes marked as invalid */
xf86PruneDriverModes (pScrn);
/*
// Belcon Debug
for (p = pScrn->modes; p != NULL; p = p->next) {
xf86DrvMsg("", X_INFO, "Belcon Debug: after xf86PruneDriverModes\n");
xf86DrvMsg("", X_INFO, "Belcon Debug: mode name is %s\n", p->name);
xf86DrvMsg("", X_INFO, "Belcon Debug: status is %s\n", xf86ModeStatusToString(p->status));
}
 */

if ((i == 0) || (pScrn->modes == NULL))
{
	xf86DrvMsg (pScrn->scrnIndex, X_ERROR, "No valid modes found\n");
	SMI_FreeRec (pScrn);
	return (FALSE);
}

/* Don't need this for 501 */
if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
{
	xf86SetCrtcForModes (pScrn, 0);
}

/* Set the current mode to the first in the list */
pScrn->currentMode = pScrn->modes;
SMIRegPtr	pSMIEnt = SMIEntPriv (pScrn);
if( (pSMIEnt->DualHead)&&(pSmi->edid_enable))
	
{
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "david print virtualX=%d, virtualY = %d\n", pScrn->virtualX, pScrn->virtualY);
	pScrn->virtualX = pScrn->currentMode ->HDisplay;
	pScrn->virtualY = pScrn->currentMode->VDisplay;
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "david print virtualX=%d, virtualY = %d\n", pScrn->virtualX, pScrn->virtualY);
}

/* Print the list of modes being used */
xf86PrintModes (pScrn);

/* Set display resolution */
xf86SetDpi (pScrn, 0, 0);


if ((xf86LoadSubModule (pScrn, "fb") == NULL))
{
	SMI_FreeRec (pScrn);
	return (FALSE);
}

#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
xf86LoaderReqSymLists (fbSymbols, NULL);
#endif

/* Load XAA if needed */
if (!pSmi->NoAccel || pSmi->hwcursor)
{
	if (!xf86LoadSubModule (pScrn, "xaa"))
	{
		SMI_FreeRec (pScrn);
		return (FALSE);
	}
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
	xf86LoaderReqSymLists (xaaSymbols, NULL);
#endif
}

/* Load ramdac if needed */
if (pSmi->hwcursor)
{
	if (!xf86LoadSubModule (pScrn, "ramdac"))
	{
		SMI_FreeRec (pScrn);
		return (FALSE);
	}
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
	xf86LoaderReqSymLists (ramdacSymbols, NULL);
#endif
}

if (pSmi->shadowFB)
{
	if (!xf86LoadSubModule (pScrn, "shadowfb"))
	{
		SMI_FreeRec (pScrn);
		return (FALSE);
	}
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
	xf86LoaderReqSymLists (shadowSymbols, NULL);
#endif
}

#ifndef XSERVER_LIBPCIACCESS
//xf86DrvMsg("", X_INFO, "PCI: command is 0x%x, line %d\n", pciReadByte(pSmi->PciTag, PCI_CMD_STAT_REG), __LINE__);
//read_cmd_reg(4);
#endif


LEAVE (TRUE);
}

	static void
WriteAttr(SMIPtr pSmi, int index, int value)
{
	(void) inb(pSmi->PIOBase + VGA_IOBASE_COLOR + VGA_IN_STAT_1_OFFSET);

	index |= 0x20;
	outb(pSmi->PIOBase + VGA_ATTR_INDEX, index);
	outb(pSmi->PIOBase + VGA_ATTR_DATA_W, value);
}

	static int
ReadAttr(SMIPtr pSmi, int index)
{
	(void) inb(pSmi->PIOBase + VGA_IOBASE_COLOR + VGA_IN_STAT_1_OFFSET);

	index |= 0x20;
	outb(pSmi->PIOBase + VGA_ATTR_INDEX, index);
	return (inb(pSmi->PIOBase + VGA_ATTR_DATA_R));
}

#define WriteMiscOut(value)	outb(pSmi->PIOBase + VGA_MISC_OUT_W, value)
#define ReadMiscOut()		inb(pSmi->PIOBase + VGA_MISC_OUT_R)
#define WriteSeq(index, value)	outb(pSmi->PIOBase + VGA_SEQ_INDEX, index);\
	outb(pSmi->PIOBase + VGA_SEQ_DATA, value)

	static int
ReadSeq(SMIPtr pSmi, int index)
{
	outb(pSmi->PIOBase + VGA_SEQ_INDEX, index);
	return (inb(pSmi->PIOBase + VGA_SEQ_DATA));
}

#define WriteGr(index, value)				\
	outb(pSmi->PIOBase + VGA_GRAPH_INDEX, index);	\
outb(pSmi->PIOBase + VGA_GRAPH_DATA, value)

	static int
ReadGr(SMIPtr pSmi, int index)
{
	outb(pSmi->PIOBase + VGA_GRAPH_INDEX, index);

	return (inb(pSmi->PIOBase + VGA_GRAPH_DATA));
}

#define WriteCrtc(index, value)						     \
	outb(pSmi->PIOBase + (VGA_IOBASE_COLOR + VGA_CRTC_INDEX_OFFSET), index); \
outb(pSmi->PIOBase + (VGA_IOBASE_COLOR + VGA_CRTC_DATA_OFFSET), value)

	static void
SeqReset(SMIPtr pSmi, Bool start)
{
	if (start) {
		WriteSeq(0x00, 0x01);		/* Synchronous Reset */
	}
	else {
		WriteSeq(0x00, 0x03);		/* End Reset */
	}
}


static void SMI_RestoreFonts(ScrnInfoPtr pScrn)
{
	SMIPtr	pSmi = SMIPTR(pScrn);	
	vgaHWPtr hwp = VGAHWPTR(pScrn);	
	SMIRegPtr pRegPtr = (SMIRegPtr)pSmi->pEnt_private->ptr;
	unsigned char miscOut, attr10, gr1, gr3, gr4, gr5, gr6, gr8, seq2, seq4, scrn;

	ENTER();

	XINF("pSmi->Primary = %d,pRegPtr->fonts = %p\n",pSmi->IsPrimary,pRegPtr->fonts);

	/* only primary pci device and first pScrn  need do vga fonts restore routine */
	if(xf86IsPrimaryPci(pSmi->PciInfo) && pSmi->IsPrimary && pRegPtr->fonts){
#if 1
		memcpy(pSmi->FBBase + KB(0),pRegPtr->fonts,KB(64));
		/* not use legency method or multi-card cries */
#else

		/* save the registers that are needed here */
		miscOut = ReadMiscOut();
		attr10 = ReadAttr(pSmi, 0x10);
		gr1 = ReadGr(pSmi, 0x01);
		gr3 = ReadGr(pSmi, 0x03);
		gr4 = ReadGr(pSmi, 0x04);
		gr5 = ReadGr(pSmi, 0x05);
		gr6 = ReadGr(pSmi, 0x06);
		gr8 = ReadGr(pSmi, 0x08);
		seq2 = ReadSeq(pSmi, 0x02);
		seq4 = ReadSeq(pSmi, 0x04);

		/* Force into colour mode */
		WriteMiscOut(miscOut | 0x01);

		scrn = ReadSeq(pSmi, 0x01) | 0x20;
		SeqReset(pSmi, TRUE);
		WriteSeq(0x01, scrn);
		SeqReset(pSmi, FALSE);

		WriteAttr(pSmi, 0x10, 0x01);	/* graphics mode */
		if (pScrn->depth == 4) {
			/* GJA */
			WriteGr(0x03, 0x00);	/* don't rotate, write unmodified */
			WriteGr(0x08, 0xFF);	/* write all bits in a byte */
			WriteGr(0x01, 0x00);	/* all planes come from CPU */
		}

		WriteSeq(0x02, 0x04);   /* write to plane 2 */
		WriteSeq(0x04, 0x06);   /* enable plane graphics */
		WriteGr(0x04, 0x02);    /* read plane 2 */
		WriteGr(0x05, 0x00);    /* write mode 0, read mode 0 */
		WriteGr(0x06, 0x05);    /* set graphics */
		slowbcopy_tobus(pRegPtr->fonts, hwp->Base, 8192);

		WriteSeq(0x02, 0x08);   /* write to plane 3 */
		WriteSeq(0x04, 0x06);   /* enable plane graphics */
		WriteGr(0x04, 0x03);    /* read plane 3 */
		WriteGr(0x05, 0x00);    /* write mode 0, read mode 0 */
		WriteGr(0x06, 0x05);    /* set graphics */
		slowbcopy_tobus(pRegPtr->fonts + 8192, hwp->Base, 8192);

		scrn = ReadSeq(pSmi, 0x01) & ~0x20;
		SeqReset(pSmi, TRUE);
		WriteSeq(0x01, scrn);
		SeqReset(pSmi, FALSE);

		/* restore the registers that were changed */
		WriteMiscOut(miscOut);
		WriteAttr(pSmi, 0x10, attr10);
		WriteGr(0x01, gr1);
		WriteGr(0x03, gr3);
		WriteGr(0x04, gr4);
		WriteGr(0x05, gr5);
		WriteGr(0x06, gr6);
		WriteGr(0x08, gr8);
		WriteSeq(0x02, seq2);
		WriteSeq(0x04, seq4);
#endif		
	}

	LEAVE();
}



/* save vga fonts */
static void SMI_SaveFonts(ScrnInfoPtr pScrn)
{	
	SMIPtr	pSmi = SMIPTR(pScrn);
	SMIRegPtr pRegPtr = (SMIRegPtr)pSmi->pEnt_private->ptr;
	vgaHWPtr hwp = VGAHWPTR(pScrn);
	unsigned char miscOut, attr10, gr4, gr5, gr6, seq2, seq4, scrn;	

	ENTER();

	/* only primary  device/screen need save  vga fonts*/
	if(!xf86IsPrimaryPci(pSmi->PciInfo) || pSmi->IsSecondary){		
		LEAVE();
	}

	/* seems the fonts had been saved ,maybe saved by first pScrn */
	if(pRegPtr->fonts != NULL){
		LEAVE();
	}

#if 1
	/* leave if in graphic mode */
	if(PEEK32(0x88) & 0x2)
		LEAVE();

	pRegPtr->fonts = xalloc(KB(64));
	memcpy(pRegPtr->fonts,pSmi->FBBase + KB(0),KB(64));
	/* not use legency method to access vga fonts , or multi-card will cry */
#else


	/* if in graphics mode,don't save anythig */	
	attr10 = ReadAttr(pSmi,0x10);
	if(attr10 & 1){
		LEAVE();
	}

	pRegPtr->fonts = xalloc(8192*2);

	/* save the registers that needed here  */
	miscOut = ReadMiscOut();
	gr4 = ReadGr(pSmi,0x4);
	gr5 = ReadGr(pSmi,0x5);
	gr6 = ReadGr(pSmi, 0x06);
	seq2 = ReadSeq(pSmi, 0x02);
	seq4 = ReadSeq(pSmi, 0x04);

	/* force into colour mode  */	
	WriteMiscOut(miscOut | 0x01);
	scrn = ReadSeq(pSmi, 0x01) | 0x20;
	SeqReset(pSmi, TRUE);
	WriteSeq(0x01, scrn);
	SeqReset(pSmi, FALSE);
	WriteAttr(pSmi, 0x10, 0x01);	/* graphics mode */

	/* fonts 1 */
	WriteSeq(0x02, 0x04);	/* write to plane 2 */
	WriteSeq(0x04, 0x06);	/* enable plane graphics */
	WriteGr(0x04, 0x02);	/* read plane 2 */
	WriteGr(0x05, 0x00);	/* write mode 0, read mode 0 */
	WriteGr(0x06, 0x05);	/* set graphics */
	slowbcopy_frombus(hwp->Base, pRegPtr->fonts, 8192);

	/* fonts 2 */	
	WriteSeq(0x02, 0x08);	/* write to plane 3 */
	WriteSeq(0x04, 0x06);	/* enable plane graphics */
	WriteGr(0x04, 0x03);	/* read plane 3 */
	WriteGr(0x05, 0x00);	/* write mode 0, read mode 0 */
	WriteGr(0x06, 0x05);	/* set graphics */
	slowbcopy_frombus(hwp->Base, pRegPtr->fonts + 8192, 8192);

	scrn = ReadSeq(pSmi,0x1) & ~0x20;
	SeqReset(pSmi,TRUE);
	WriteSeq(0x01,scrn);
	SeqReset(pSmi,FALSE);

	/* Restore clobbered registers */
	WriteAttr(pSmi, 0x10, attr10);
	WriteSeq(0x02, seq2);
	WriteSeq(0x04, seq4);
	WriteGr(0x04, gr4);
	WriteGr(0x05, gr5);
	WriteGr(0x06, gr6);
	WriteMiscOut(miscOut);	

#endif
	LEAVE();
}


/*
 * This is called when VT switching back to the X server.  Its job is to
 * reinitialise the video mode. We may wish to unmap video/MMIO memory too.
 */

	static Bool
SMI_EnterVT (int scrnIndex, int flags)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	SMIPtr pSmi = SMIPTR (pScrn);
	Bool ret;


	/* Enable MMIO and map memory */
	/*
	   SMI_MapMem(pScrn);
	   SMI_Save(pScrn);
	 */

	if (!SMI_MapMem (pScrn))
	{
		return (FALSE);
	}

	pScrn->pScreen->ModifyPixmapHeader(pScrn->pScreen->GetScreenPixmap(pScrn->pScreen),-1,-1,-1,-1,-1, pSmi->FBBase + pSmi->FBOffset);
	pScrn->pixmapPrivate.ptr=pSmi->FBBase;
#if 0
	//  SMI_Save (pScrn);
	if (pSmi->Chipset == SMI_MSOCE)
	{
		// SMI_SaveReg_Con(pScrn);       
		SMI_RestoreReg_750(pScrn);
	}
	else
		SMI_RestoreReg_502(pScrn);
#endif	
	/* #670 */
	if (pSmi->shadowFB)
	{
		pSmi->FBOffset = pSmi->savedFBOffset;
		pSmi->FBReserved = pSmi->savedFBReserved;
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "line %d: pSmi->FBReserved is 0x%x\n", __LINE__, pSmi->FBReserved);
	}


	ret = SMI_ModeInit (pScrn, pScrn->currentMode);


	if(pSmi->Chipset == SMI_MSOC || pSmi->Chipset == SMI_MSOCE)
	{
		/* shutdown hardware console's cursor */
		int d = PEEK32(0x800f0);
		d = d & 0x7fffffff;
		POKE32(0x800f0,d);
	}


	/* #670 */
	if (ret && pSmi->shadowFB)
	{
		BoxRec box;

		/* #920 */
		if (pSmi->paletteBuffer)
		{
			int i;

			VGAOUT8 (pSmi, VGA_DAC_WRITE_ADDR, 0);
			for (i = 0; i < 256 * 3; i++)
			{
				VGAOUT8 (pSmi, VGA_DAC_DATA, pSmi->paletteBuffer[i]);
			}
			xfree (pSmi->paletteBuffer);
			pSmi->paletteBuffer = NULL;
		}

		if (pSmi->pSaveBuffer)
		{
			memcpy (pSmi->FBBase, pSmi->pSaveBuffer, pSmi->saveBufferSize);
			xfree (pSmi->pSaveBuffer);
			pSmi->pSaveBuffer = NULL;
		}

		box.x1 = 0;
		box.y1 = 0;
		box.x2 = pScrn->virtualY;
		box.y2 = pScrn->virtualX;

		if (pSmi->Chipset == SMI_COUGAR3DR)
		{
			SMI_RefreshArea730 (pScrn, 1, &box);
		}
		else
		{
			SMI_RefreshArea (pScrn, 1, &box);
		}

	}

	/* Reset the grapics engine */
	if (!pSmi->NoAccel) 
		SMI_EngineReset (pScrn);

	if(pSmi->Chipset == SMI_MSOC){
		SMI_EnableVideo(pScrn);
		/*
		   found below line sometimes make 750 exit errorly when switch back from console
		   but this line is needed by recover video registers, remind to fix later 
		 */
		SMI_ResetVideo(pScrn);
	}
	return (ret);
}

/*
 * This is called when VT switching away from the X server.  Its job is to
 * restore the previous (text) mode. We may wish to remap video/MMIO memory
 * too.
 */

	static void
SMI_LeaveVT (int scrnIndex, int flags)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	vgaHWPtr hwp = VGAHWPTR (pScrn);
	SMIPtr pSmi = SMIPTR (pScrn);
	vgaRegPtr vgaSavePtr = &hwp->SavedReg;
	SMIRegPtr SMISavePtr = &pSmi->SavedReg;

	ENTER();

	XERR("is secondary = %d,FBOFFSET = %d\n",pSmi->IsSecondary,pSmi->FBOffset);


	//SMI_AccelSync(pScrn);
#if 0	
	if(SMI_MSOCE == pSmi->Chipset)
	{
		if(saved_video_mode_count == 0)
		{
			SMI_SaveReg_750(pScrn );
			saved_video_mode_count++;
		}
		SMI_RestoreReg_Con(pScrn);
	} 
	else
	{
		SMI_SaveReg_502(pScrn );
	}
#endif	
	/* #670 */
#if 1
	if (pSmi->shadowFB)
	{
		pSmi->pSaveBuffer = xnfalloc (pSmi->saveBufferSize);
		//        if (pSmi->pSaveBuffer && !pSmi->IsSecondary)
		if(pSmi->pSaveBuffer)
		{
			memcpy (pSmi->pSaveBuffer, pSmi->FBBase, pSmi->saveBufferSize);
		}

		pSmi->savedFBOffset = pSmi->FBOffset;
		pSmi->savedFBReserved = pSmi->FBReserved;

		/* #920 */	

		if (pSmi->Bpp == 1)	
		{
			pSmi->paletteBuffer = xnfalloc (256 * 3);
			if (pSmi->paletteBuffer)
			{
				int i;

				VGAOUT8 (pSmi, VGA_DAC_READ_ADDR, 0);
				for (i = 0; i < 256 * 3; i++)
				{
					pSmi->paletteBuffer[i] = VGAIN8 (pSmi, VGA_DAC_DATA);
				}
			}
		}
	}
#endif
	//    memset (pSmi->FBBase, 0, 256 * 1024);	/* #689 */
#if  XORG_VERSION_CURRENT <=  XORG_VERSION_NUMERIC(7,1,1,0,0)
	//caesar removed for X server 1.5.3    	
	if(pSmi->Chipset != SMI_MSOC && pSmi->Chipset != SMI_MSOCE)	
		SMI_WriteMode (pScrn, vgaSavePtr, SMISavePtr);
#endif

#if 0
	/*
	 * Belcon: Fix #001
	 * 	Restore hardware cursor register
	 */
	if (pSmi->dcrF0)
		WRITE_DCR(pSmi, DCRF0, pSmi->dcrF0);
	if (pSmi->dcr230)
		WRITE_DCR(pSmi, DCR230, pSmi->dcr230);
	/* #001 ended */

	/* Teddy: close clone mode */
	if (pSmi->clone_mode) {
		CARD8 tmp;
		tmp = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31);
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31, tmp&0x0f);	
	}
#endif	

	if(pSmi->Chipset == SMI_MSOCE)
		SMI750_Restore(pScrn);

	SMI_UnmapMem (pScrn);
	LEAVE();
}

/*
 * This function performs the inverse of the restore function: It saves all the
 * standard and extended registers that we are going to modify to set up a video
 * mode.
 */

	static void
SMI_Save (ScrnInfoPtr pScrn)
{
	int i;
	CARD32 offset;

	vgaHWPtr hwp = VGAHWPTR (pScrn);
	vgaRegPtr vgaSavePtr = &hwp->SavedReg;
	SMIPtr pSmi = SMIPTR (pScrn);
	SMIRegPtr save = &pSmi->SavedReg;

	int vgaIOBase = hwp->IOBase;
	int vgaCRIndex = vgaIOBase + VGA_CRTC_INDEX_OFFSET;
	int vgaCRData = vgaIOBase + VGA_CRTC_DATA_OFFSET;


	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Belcon:DEBUG:EntityList %d\n", pScrn->entityList[0]);

	if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
	{

		/* Save the standard VGA registers */
		vgaHWSave (pScrn, vgaSavePtr, VGA_SR_ALL);
		save->smiDACMask = VGAIN8 (pSmi, VGA_DAC_MASK);
		VGAOUT8 (pSmi, VGA_DAC_READ_ADDR, 0);
		for (i = 0; i < 256; i++)
		{
			save->smiDacRegs[i][0] = VGAIN8 (pSmi, VGA_DAC_DATA);
			save->smiDacRegs[i][1] = VGAIN8 (pSmi, VGA_DAC_DATA);
			save->smiDacRegs[i][2] = VGAIN8 (pSmi, VGA_DAC_DATA);
		}
		for (i = 0, offset = 2; i < 8192; i++, offset += 8)
		{
			save->smiFont[i] = *(pSmi->FBBase + offset);
		}

		/* Now we save all the extended registers we need. */
		save->SR17 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x17);
		/* Debug */
		if (-1 == saved_console_reg) {
			save->SR18 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x18);
			// xf86DrvMsg(pScrn->scrnIndex, X_INFO, "SMI_Save(), Set save->SR18 value: 0x%x\n", save->SR18);
			saved_console_reg = 0x0;
		} else {
			// xf86DrvMsg(pScrn->scrnIndex, X_INFO, "SMI_Save(), assign (pSmi->SavedReg).SR18 to save->SR18 is 0x%x\n", (pSmi->SavedReg).SR18);
			save->SR18 = (pSmi->SavedReg).SR18;
		}
		//      save->SR18 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x18);
		// xf86DrvMsg(pScrn->scrnIndex, X_INFO, "SMI_Save(), save->SR18 is 0x%x\n", save->SR18);
		save->SR21 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21);
		save->SR31 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31);
		save->SR32 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x32);
		save->SR6A = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6A);
		save->SR6B = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6B);
		/*Add for 64bit */
		save->SR6C = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6C);
		save->SR6D = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6D);

		/* Belcon: clone mode */
		if (pSmi->clone_mode) {
			save->SR6E = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6E);
			save->SR6F = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6F);
		}

		save->SR81 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x81);
		save->SRA0 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0xA0);


		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "SEQ Save\n");
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "17.%X\n", save->SR17);
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "18.%X\n", save->SR18);
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "21.%X\n", save->SR21);
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "31.%X\n", save->SR31);
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "32.%X\n", save->SR32);
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "6A.%X\n", save->SR6A);
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "6B.%X\n", save->SR6B);
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "6C.%X\n", save->SR6C);
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "6D.%X\n", save->SR6D);

		/* Belcon: clone mode */
		if (pSmi->clone_mode) {
			xf86DrvMsg (pScrn->scrnIndex, X_INFO, "6E.%X\n", save->SR6E);
			xf86DrvMsg (pScrn->scrnIndex, X_INFO, "6F.%X\n", save->SR6F);
		}

		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "81.%X\n", save->SR81);
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "A0.%X\n", save->SRA0);

		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "59.%X\n",
				VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x59));


		if (SMI_LYNXM_SERIES (pSmi->Chipset))
		{
			/* Save primary registers */
			save->CR90[14] = VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x9E);
			VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x9E,
					save->CR90[14] & ~0x20);

			for (i = 0; i < 16; i++)
			{
				save->CR90[i] =
					VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x90 + i);
			}
			save->CR30 = VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x30);
			xf86DrvMsg("", X_INFO, "Belcon:test:line: %d cr30 is 0x%x\n", __LINE__, save->CR30);
			save->CR33 = VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x33);
			save->CR3A = VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x3A);

			/* Belcon : clone mode */
			if (pSmi->clone_mode) {
				for ( i = 0; i < 8; i++) {
					save->FPR50[i] =
						VGAIN8_INDEX(pSmi, vgaCRIndex, vgaCRData, 0x50 + i);
				}
				save->FPR50[0x0A] =
					VGAIN8_INDEX(pSmi, vgaCRIndex, vgaCRData, 0x5A);
			}

			for (i = 0; i < 14; i++)
			{
				// save->CR40_2[i] =
				save->CR40[i] =
					VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x40 + i);
				// xf86DrvMsg("", X_INFO, "Belcon:haha, saveing 1st CRT%d: 0x%x\n", 0x40 + i, save->CR40[i]);
			}

			/* Save secondary registers */
			VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x9E,
					save->CR90[14] | 0x20);
			save->CR33_2 = VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x33);
			for (i = 0; i < 14; i++)
			{
				save->CR40_2[i] = VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData,
						0x40 + i);
				xf86DrvMsg("", X_INFO, "Belcon:haha, saveing 2nd CRT%d: 0x%x\n", 0x40 + i, save->CR40_2[i]);
			}
			save->CR9F_2 = VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x9F);

			/* Save common registers */
			for (i = 0; i < 14; i++)
			{
				save->CRA0[i] =
					VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0xA0 + i);
			}

			/* PDR#1069 */
			VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x9E, save->CR90[14]);
		}
		else
		{
			save->CR30 = VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x30);
			xf86DrvMsg("", X_INFO, "Belcon:test:line: %d cr30 is 0x%x\n", __LINE__, save->CR30);
			save->CR33 = VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x33);
			save->CR3A = VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x3A);
			for (i = 0; i < 14; i++)
			{
				save->CR40[i] =
					VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x40 + i);
			}
		}

		/* CZ 2.11.2001: for gamma correction (TODO: other chipsets?) */
		if ((pSmi->Chipset == SMI_LYNX3DM) || (pSmi->Chipset == SMI_COUGAR3DR))
		{
			save->CCR66 =
				VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x66);
		}
		/* end CZ */
	}


	save->DPR10 = READ_DPR (pSmi, 0x10);
	save->DPR1C = READ_DPR (pSmi, 0x1C);
	save->DPR20 = READ_DPR (pSmi, 0x20);
	save->DPR24 = READ_DPR (pSmi, 0x24);
	save->DPR28 = READ_DPR (pSmi, 0x28);
	save->DPR2C = READ_DPR (pSmi, 0x2C);
	save->DPR30 = READ_DPR (pSmi, 0x30);
	save->DPR3C = READ_DPR (pSmi, 0x3C);
	save->DPR40 = READ_DPR (pSmi, 0x40);
	save->DPR44 = READ_DPR (pSmi, 0x44);

	save->VPR00 = READ_VPR (pSmi, 0x00);
	save->VPR0C = READ_VPR (pSmi, 0x0C);
	save->VPR10 = READ_VPR (pSmi, 0x10);

	if (pSmi->Chipset == SMI_COUGAR3DR)
	{
		save->FPR00_ = READ_FPR (pSmi, FPR00);
		save->FPR0C_ = READ_FPR (pSmi, FPR0C);
		save->FPR10_ = READ_FPR (pSmi, FPR10);
	}

	save->CPR00 = READ_CPR (pSmi, 0x00);

	if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
	{
		if (!pSmi->ModeStructInit)
		{
			/* XXX Should check the return value of vgaHWCopyReg() */
			vgaHWCopyReg (&hwp->ModeReg, vgaSavePtr);
			memcpy (&pSmi->ModeReg, save, sizeof (SMIRegRec));
			pSmi->ModeStructInit = TRUE;
		}
	}

	if (xf86GetVerbosity () > 1)
	{
		xf86DrvMsgVerb (pScrn->scrnIndex, X_INFO, VERBLEV,
				"Saved current video mode.  David Register dump Line:%d:\n", __LINE__);
		SMI_PrintRegs (pScrn);
	}

}

int GetFreshRate(DisplayModePtr mode)
{
	char *pString = mode->name;
	float Refresh;

	while(((*pString) != '\0'))
	{
		if(*pString == '@')
			break;

		pString++;
	}


	if((*pString) == '\0')
	{
		Refresh = mode->VRefresh;
		if(Refresh > 85 * (1.0 - SYNC_TOLERANCE)) return 85;
		if(Refresh > 75 * (1.0 - SYNC_TOLERANCE)) return 75;
		if(Refresh > 60 * (1.0 - SYNC_TOLERANCE)) return 60;

		return 60;

	}
	else
	{

		pString++;
		if((*pString == '6')&&(*(pString+1) == '0')) return 60;
		if((*pString == '7')&&(*(pString+1) == '5')) return 75;
		if((*pString == '8')&&(*(pString+1) == '5')) return 85;

		return 60;
	}

}




	static void
SMI_BiosWriteMode (ScrnInfoPtr pScrn, DisplayModePtr mode, SMIRegPtr restore)
{
	int i;
	CARD8 tmp;
	EntityInfoPtr pEnt;

	vgaHWPtr hwp  = VGAHWPTR (pScrn);
	SMIPtr pSmi     = SMIPTR (pScrn);
	int vgaIOBase  = hwp->IOBase;
	int vgaCRIndex = vgaIOBase + VGA_CRTC_INDEX_OFFSET;
	int vgaCRData   = vgaIOBase + VGA_CRTC_DATA_OFFSET;

	//if not use bios call, return
	if(!pSmi->useBIOS) return;	

	int modeindex = 0; 
	int VsyncBits = 0x0;        //00 -- 43Hz
	//01 -- 60Hz 
	//10 -- 75Hz
	//11 -- 85Hz
	/* Find the INT 10 mode number */
	static struct
	{
		int x, y, bpp, freshrate;
		CARD16 mode;
	} 
	modeTable[] =
	{
		{640, 480, 8, 60,0x50},
		{640, 480, 16, 60,0x52},
		{640, 480, 24, 60,0x53},
		{640, 480, 32, 60,0x54},
		{640, 480, 8, 75,0x50},
		{640, 480, 16, 75,0x52},
		{640, 480, 24, 75,0x53},
		{640, 480, 32, 75,0x54},
		{640, 480, 8, 85,0x50},
		{640, 480, 16, 85,0x52},
		{640, 480, 24, 85,0x53},
		{640, 480, 32, 85,0x54},

		{800, 480, 8, 60,0x4A},
		{800, 480, 16, 60,0x4C},
		{800, 480, 24, 60,0x4D},
		{800, 480, 8, 75,0x4A},
		{800, 480, 16, 75,0x4C},
		{800, 480, 24, 75,0x4D},
		{800, 480, 8, 85,0x4A},
		{800, 480, 16, 85,0x4C},
		{800, 480, 24, 85,0x4D},


		{800, 600, 8, 60,0x55},
		{800, 600, 16,60, 0x57},
		{800, 600, 24, 60,0x58},
		{800, 600, 32, 60,0x59},
		{800, 600, 8, 75,0x55},
		{800, 600, 16,75, 0x57},
		{800, 600, 24, 75,0x58},
		{800, 600, 32, 75,0x59},
		{800, 600, 8, 85,0x55},
		{800, 600, 16,85, 0x57},
		{800, 600, 24, 85,0x58},
		{800, 600, 32, 85,0x59},


		{1024, 768, 8, 60,0x60},
		{1024, 768, 16, 60,0x62},
		{1024, 768, 24, 60,0x63},
		{1024, 768, 32, 60,0x64},
		{1024, 768, 8, 75,0x60},
		{1024, 768, 16, 75,0x62},
		{1024, 768, 24, 75,0x63},
		{1024, 768, 32, 75,0x64},
		{1024, 768, 8, 85,0x60},
		{1024, 768, 16, 85,0x62},
		{1024, 768, 24, 85,0x63},
		{1024, 768, 32, 85,0x64},


		{1280, 1024, 8, 60,0x65},
		{1280, 1024, 16, 60,0x67},
		{1280, 1024, 24, 60,0x68},
		{1280, 1024, 32, 60,0x69},
		{1280, 1024, 8, 75,0x65},
		{1280, 1024, 16, 75,0x67},
		{1280, 1024, 24, 75,0x68},
		{1280, 1024, 32, 75,0x69},
	};

	//check mode width, height, freshrate
	for (i = 0; i < sizeof (modeTable) / sizeof (modeTable[0]); i++)
	{
		if ((modeTable[i].x == mode->HDisplay)
				&& (modeTable[i].y == mode->VDisplay)
				&& (modeTable[i].bpp == pScrn->bitsPerPixel)
				&&(modeTable[i].freshrate == GetFreshRate(mode)))
		{
			//new->mode = modeTable[i].mode;
			modeindex = i;
			//xf86DrvMsg("", X_INFO, "Mill:  H -- %d, V -- %d, BPP -- %d\n", mode->HDisplay, mode->VDisplay, pScrn->bitsPerPixel );
			break;
		}
	}

	xf86DrvMsg (pScrn->scrnIndex, X_INFO, " Mill: modeindex -- %d\n", modeindex);

	vgaHWProtect (pScrn, TRUE);

	/* Wait for engine to become idle */
	WaitIdle ();

	//xf86DrvMsg (pScrn->scrnIndex, X_INFO, " Mill: pSmi: 0x%x, useBIOS %d, pinit10 %d, mode : %d\n",
	//pSmi, pSmi->useBIOS,	pSmi->pInt10,  restore->mode);

	//if no use int10 function, load it    - = - mill.chen
		if (xf86LoadSubModule (pScrn, "int10"))
		{
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
			xf86LoaderReqSymLists (int10Symbols, NULL);
#endif
			pEnt = xf86GetEntityInfo (pScrn->entityList[0]);
			pSmi->pInt10 = xf86InitInt10 (pEnt->index);
		}
		/*
	if(pSmi->pInt10 == NULL)
	{
		pEnt = xf86GetEntityInfo (pScrn->entityList[0]);
		pSmi->pInt10 = xf86InitInt10 (pEnt->index);

		//xf86DrvMsg (pScrn->scrnIndex, X_INFO, " Mill: pSmi: 0x%x, useBIOS %d, pinit10 0x%x, mode : %d\n",
		//          pSmi, pSmi->useBIOS,	pSmi->pInt10,  restore->mode);
	}
	*/

	//begin to use bios call
	if (pSmi->pInt10 != NULL) 
	{
		pSmi->pInt10->num = 0x10;
		pSmi->pInt10->ax = modeTable[modeindex].mode | 0x80;
		///////////////////////////mill added
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "Mill: Setting mode 0x%02X\n",
				modeTable[modeindex].mode );

		switch(modeTable[modeindex].freshrate)
		{
			case 60: 	VsyncBits = 0x1; break;
			case 75:		VsyncBits = 0x2; break;
			case 85:		VsyncBits = 0x3; break;
			default:		VsyncBits = 0x0; 		
		}

		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "Mill: modeTable[i].freshrate 0x%02X\n",
				modeTable[i].freshrate );

		tmp = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x71);   // only use in our bios -- mill added
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x71, tmp & 0xFC | VsyncBits);

		SMI_DisableMmio (pScrn);
		xf86ExecX86int10 (pSmi->pInt10);

		/* Enable linear mode. */
		outb (pSmi->PIOBase + VGA_SEQ_INDEX, 0x18);
		tmp = inb (pSmi->PIOBase + VGA_SEQ_DATA);
		outb (pSmi->PIOBase + VGA_SEQ_DATA, tmp | 0x01);

		/* Enable DPR/VPR registers. */
		tmp = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21);
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21, tmp & ~0x03);


		//////////////////////////////////mill added
		xf86FreeInt10 (pSmi->pInt10);
		pSmi->pInt10 = NULL;
	}

	//old code
	/* CZ 2.11.2001: for gamma correction (TODO: other chipsets?) */
	if ((pSmi->Chipset == SMI_LYNX3DM) || (pSmi->Chipset == SMI_COUGAR3DR))
	{
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x66, restore->CCR66);
	}
	/* end CZ */

	VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6C, restore->SR6C);
	VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6D, restore->SR6D);

	/* Belcon: clone mode */
	if (pSmi->clone_mode) {
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6E, restore->SR6E);
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6F, restore->SR6F);
	}

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "6c %02X, 6d %02X, 6e %02X, 6f %02X\n", restore->SR6C, restore->SR6D, restore->SR6E, restore->SR6F);
	VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x81, 0x00);

	WRITE_DPR (pSmi, 0x10, restore->DPR10);
	WRITE_DPR (pSmi, 0x1C, restore->DPR1C);
	WRITE_DPR (pSmi, 0x20, restore->DPR20);
	WRITE_DPR (pSmi, 0x24, restore->DPR24);
	WRITE_DPR (pSmi, 0x28, restore->DPR28);
	WRITE_DPR (pSmi, 0x2C, restore->DPR2C);
	WRITE_DPR (pSmi, 0x30, restore->DPR30);
	WRITE_DPR (pSmi, 0x3C, restore->DPR3C);
	WRITE_DPR (pSmi, 0x40, restore->DPR40);
	WRITE_DPR (pSmi, 0x44, restore->DPR44);
	if (pSmi->Chipset == SMI_MSOCE)
	{
		for(i = 0; i < (sizeof(VprTable_750) / sizeof(unsigned int)); i++)
		{
			VprTable_750[i] = READ_VPR(pSmi, VprIndexTable_750[i]);
			WRITE_VPR (pSmi,  VprIndexTable_750[i], VprTable_750[i]);
		}
	}
	else
	{
		WRITE_VPR (pSmi, 0x00, restore->VPR00);
		WRITE_VPR (pSmi, 0x0C, restore->VPR0C);
		WRITE_VPR (pSmi, 0x10, restore->VPR10);
	}
	xf86DrvMsg (pScrn->scrnIndex, X_ERROR, " FPR0C = %X FPR10 = %X\n",
			restore->VPR0C, restore->VPR10);

	if (pSmi->Chipset == SMI_COUGAR3DR)
	{
		WRITE_FPR (pSmi, FPR00, restore->FPR00_);
		WRITE_FPR (pSmi, FPR0C, restore->FPR0C_);
		WRITE_FPR (pSmi, FPR10, restore->FPR10_);
	}

	WRITE_CPR (pSmi, 0x00, restore->CPR00);


	//old code 
	if (xf86GetVerbosity () > 1)
	{
		xf86DrvMsgVerb (pScrn->scrnIndex, X_INFO, VERBLEV,
				"Done restoring mode.  David Register dump Line:%d:\n", __LINE__);
		SMI_PrintRegs (pScrn);
	}

	vgaHWProtect (pScrn, FALSE);



}
/*
 * This function is used to restore a video mode. It writes out all of the
 * standard VGA and extended registers needed to setup a video mode.
 */

	static void
SMI_WriteMode (ScrnInfoPtr pScrn, vgaRegPtr vgaSavePtr, SMIRegPtr restore)
{
	int i;
	CARD8 tmp;
	CARD32 offset;

	vgaHWPtr hwp = VGAHWPTR (pScrn);
	SMIPtr pSmi = SMIPTR (pScrn);
	int vgaIOBase = hwp->IOBase;
	int vgaCRIndex = vgaIOBase + VGA_CRTC_INDEX_OFFSET;
	int vgaCRData = vgaIOBase + VGA_CRTC_DATA_OFFSET;


	vgaHWProtect (pScrn, TRUE);

	/* Wait for engine to become idle */
	WaitIdle ();

#if 1

	if (pSmi->useBIOS && (pSmi->pInt10 != NULL) && (restore->mode != 0))
	{
		pSmi->pInt10->num = 0x10;
		pSmi->pInt10->ax = restore->mode | 0x80;
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "Setting mode 0x%02X\n",
				restore->mode);

		SMI_DisableMmio (pScrn);
		xf86ExecX86int10 (pSmi->pInt10);

		/* Enable linear mode. */
		outb (pSmi->PIOBase + VGA_SEQ_INDEX, 0x18);
		tmp = inb (pSmi->PIOBase + VGA_SEQ_DATA);
		outb (pSmi->PIOBase + VGA_SEQ_DATA, tmp | 0x01);

		/* Enable DPR/VPR registers. */
		tmp = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21);
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21, tmp & ~0x03);
	}
	else
#endif
	{

#if 0
		pSmi->pInt10->num = 0x10;
		pSmi->pInt10->ax = 0x63;
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, " Setting mode 63\n");
		SMI_DisableMmio (pScrn);
		xf86ExecX86int10 (pSmi->pInt10);	/*Call Bios  boyod */
		SMI_EnableMmio (pScrn);
#else
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, " Restore mode 0x%02X\n",
				restore->mode);

		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x17, restore->SR17);

		tmp = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x18) & ~0x1F;
		// xf86DrvMsg(pScrn->scrnIndex, X_INFO, "SMI_WriteMode(), read SR18 is 0x%x\n", VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x18));
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x18, tmp | (restore->SR18 & 0x1F) | 0x11);
		// xf86DrvMsg(pScrn->scrnIndex, X_INFO, "SMI_WriteMode(), save->SR18 is 0x%x, writing 0x%x to SEQ18 now\n", restore->SR18, tmp | (restore->SR18 & 0x1F));

		tmp = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21);
		// VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21, tmp & ~0x03);
		/* changed by Belcon to enable CRT output */
		/* Belcon : clone mode */
		if (pSmi->clone_mode) {
			VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21, tmp & ~0x13 | 0x20);
		} else {      //if set PDR21[5] to 0,many garbage will be written into video memory
			VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21, tmp & ~0x03 | 0x20);
			//VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21, tmp & ~0x03 & ~0x20);     
		}
		/*
		   tmp = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x22);
		   VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21, 0x02);
		 */


		tmp = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31) & ~0xC0;

		/* Belcon: clone mode */
		if (pSmi->clone_mode) {
			VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31, tmp |
					(restore->SR31 | 0xC3));
		} else {
			VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31, tmp |
					(restore->SR31 & 0xC0));
		}
		// VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31, tmp | 0xC0);

		tmp = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x32) & ~0x07;
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x32, tmp |
				(restore->SR32 & 0x07));
		if (restore->SR6B != 0xFF)
		{
			VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6A,
					restore->SR6A);
			VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6B,
					restore->SR6B);
		}
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, " Mode Pclock 0x%02X 0x%02X\n",
				restore->SR6C, restore->SR6D);
		/*boyod add */
		if (restore->SR6D != 0x0)
		{
			VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6C,
					restore->SR6C);
			VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6D,
					restore->SR6D);
			// xf86DrvMsg(pScrn->scrnIndex, X_INFO, " Alert: SEQ6C is 0x%02X, SEQ6D is 0x%02X\n", VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6c), VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6d));
		}

		/* Belcon: clone mode */
		if ((pSmi->clone_mode) && (restore->SR6F != 0x0)) {
			VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6E,
					restore->SR6E);
			VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6F,
					restore->SR6F);
		}


		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x81, restore->SR81);
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0xA0, restore->SRA0);

		/* Restore the standard VGA registers */
		vgaHWRestore (pScrn, vgaSavePtr, VGA_SR_ALL);
		if (restore->smiDACMask)
		{
			VGAOUT8 (pSmi, VGA_DAC_MASK, restore->smiDACMask);
		}
		else
		{
			VGAOUT8 (pSmi, VGA_DAC_MASK, 0xFF);
		}
		VGAOUT8 (pSmi, VGA_DAC_WRITE_ADDR, 0);
		for (i = 0; i < 256; i++)
		{
			VGAOUT8 (pSmi, VGA_DAC_DATA, restore->smiDacRegs[i][0]);
			VGAOUT8 (pSmi, VGA_DAC_DATA, restore->smiDacRegs[i][1]);
			VGAOUT8 (pSmi, VGA_DAC_DATA, restore->smiDacRegs[i][2]);
		}
		for (i = 0, offset = 2; i < 8192; i++, offset += 8)
		{
			*(pSmi->FBBase + offset) = restore->smiFont[i];
		}

		if (SMI_LYNXM_SERIES (pSmi->Chipset))
		{
			/*
			 * FIXME:: hardcode here
			 */
			/*
			   if (pSmi->Chipset == SMI_LYNXEMplus) {
			   tmp = VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x9E);
			   VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x9E, 0x00);
			   }
			 */
			/* Restore secondary registers */
			VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x9E,
					restore->CR90[14] | 0x20);

			VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x30, restore->CR30);
			// xf86DrvMsg("", X_INFO, "Belcon:test:line: %d cr30 is 0x%x\n", __LINE__, restore->CR30);
			VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x33, restore->CR33_2);

			/*
			   tmp = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x33) | 0x20;
			   VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x33, tmp);
			 */

			/* Belcon : clone mode */
			if (pSmi->clone_mode) {
				for (i = 0; i < 8; i++) {
					VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x50 + i,
							restore->FPR50[i]);
				}
				VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x5A,
						restore->FPR50[0x0A]);

				/* LCD Frame Buffer starting address */
				VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x40, (CARD8)((restore->VPR0C) & 0xFF));
				VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x41, (CARD8)((restore->VPR0C >> 8) & 0xFF));
				VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x44, 0);
				/* why | 0x40 ? hardcode here. Belcon */
				VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x45, (CARD8)((restore->VPR0C >> 13) & 0x38) | 0x40);
			}

			for (i = 0; i < 14; i++)
			{
				VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x40 + i,
						restore->CR40_2[i]);
				// xf86DrvMsg("", X_INFO, "Belcon:haha restore secondary CR%d: 0x%x\n", 0x40+i, restore->CR40_2[i]);
			}
			VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x9F, restore->CR9F_2);

			/* Restore primary registers */
			VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x9E,
					restore->CR90[14] & ~0x20);

			VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x33, restore->CR33);
			VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x3A, restore->CR3A);
			for (i = 0; i < 14; i++)
			{
				VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x40 + i,
						restore->CR40[i]);
				xf86DrvMsg("", X_INFO, "Belcon:haha restore primary CR%d: 0x%x\n", 0x40+i, restore->CR40[i]);
			}

			for (i = 0; i < 16; i++)
			{
				if (i != 14)
				{
					VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x90 + i,
							restore->CR90[i]);
				}

			}
			VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x9E,
					restore->CR90[14]);

			/* Restore common registers */
			for (i = 0; i < 14; i++)
			{
				VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0xA0 + i,
						restore->CRA0[i]);
			}
		}

		/* Restore the standard VGA registers */
		if (xf86IsPrimaryPci (pSmi->PciInfo))
		{
			vgaHWRestore (pScrn, vgaSavePtr, VGA_SR_CMAP | VGA_SR_FONTS);
		}

		if (restore->modeInit)
			vgaHWRestore (pScrn, vgaSavePtr, VGA_SR_ALL);

		if (!SMI_LYNXM_SERIES (pSmi->Chipset))
		{
			VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x30, restore->CR30);
			VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x33, restore->CR33);
			VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x3A, restore->CR3A);
			for (i = 0; i < 14; i++)
			{
				VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x40 + i,
						restore->CR40[i]);
			}
		}

#endif
	}

	/* CZ 2.11.2001: for gamma correction (TODO: other chipsets?) */
	if ((pSmi->Chipset == SMI_LYNX3DM) || (pSmi->Chipset == SMI_COUGAR3DR))
	{
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x66, restore->CCR66);
	}
	/* end CZ */

	VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x81, 0x00);

	WRITE_DPR (pSmi, 0x10, restore->DPR10);
	WRITE_DPR (pSmi, 0x1C, restore->DPR1C);
	WRITE_DPR (pSmi, 0x20, restore->DPR20);
	WRITE_DPR (pSmi, 0x24, restore->DPR24);
	WRITE_DPR (pSmi, 0x28, restore->DPR28);
	WRITE_DPR (pSmi, 0x2C, restore->DPR2C);
	WRITE_DPR (pSmi, 0x30, restore->DPR30);
	WRITE_DPR (pSmi, 0x3C, restore->DPR3C);
	WRITE_DPR (pSmi, 0x40, restore->DPR40);
	WRITE_DPR (pSmi, 0x44, restore->DPR44);
	if (pSmi->Chipset == SMI_MSOCE)
	{
		WRITE_VPR(pSmi, VprIndexTable_750[0], VprTable_750[0]);
		WRITE_VPR(pSmi, VprIndexTable_750[1], VprTable_750[1]);
		WRITE_VPR(pSmi, VprIndexTable_750[2], VprTable_750[2]);
		WRITE_VPR(pSmi, VprIndexTable_750[4], VprTable_750[4]);
	}
	else
	{
		WRITE_VPR (pSmi, 0x00, restore->VPR00);
		WRITE_VPR (pSmi, 0x0C, restore->VPR0C);
		WRITE_VPR (pSmi, 0x10, restore->VPR10);
	}
	xf86DrvMsg (pScrn->scrnIndex, X_INFO, " FPR00 = %X FPR0C = %X FPR10 = %X\n",
			restore->VPR00, restore->VPR0C, restore->VPR10);

	if (pSmi->Chipset == SMI_COUGAR3DR)
	{
		WRITE_FPR (pSmi, FPR00, restore->FPR00_);
		WRITE_FPR (pSmi, FPR0C, restore->FPR0C_);
		WRITE_FPR (pSmi, FPR10, restore->FPR10_);
	}

	WRITE_CPR (pSmi, 0x00, restore->CPR00);




#ifdef  _EXPERIMENTAL_XSERVER64	/*  IGXMAL -- Manage Simultaneous (virtual refresh) mode */
	{

		/* Get Hres, Vres for this mode */
		int iHres;
		int iVres;

		if (restore->mode >= 0x65)
		{
			iHres = 1280;
			iVres = 1024;
		}
		else if (restore->mode >= 0x60)
		{
			iHres = 1024;
			iVres = 768;
		}
		else if ((restore->mode >= 0x50) && (restore->mode <= 0x54))
		{
			iHres = 640;
			iVres = 480;
		}
		else
		{
			iHres = 800;
			iVres = 600;
		}

		xf86DrvMsg(pScrn->scrnIndex, X_INFO, " XServer 64, iHres is %d, iVres is %d\n", iHres, iVres);
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, " XServer 64, lcdWidth %d, lcdWidth is %d\n", iHres, iVres);
		if (restore->mode > 0)
		{
			int iVertAdjust;
			int iHorzAdjust;

			/*boyod */
			/* Set VR bit for hi-res modes */
			tmp = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31) & ~0x07;
			VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31, tmp | 0x80);

			/* Handle centering here */
			/* If (res < panelsize) then set 3d4.9e and 3d4.a6 */
			if ((iHres < pSmi->lcdWidth) || (iVres < pSmi->lcdHeight))
			{
				tmp = VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x9E);
				VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0x9E, 0x00);

				iVertAdjust = min (63, (pSmi->lcdHeight - iVres) / 2 / 4);
				iHorzAdjust = min (127, (pSmi->lcdWidth - iHres) / 2 / 8);

				VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0xA7, iHorzAdjust);
				VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, 0xA6, iVertAdjust);
			}
		}
		else
		{
			/* Clear virtual refresh bit for text modes */
			tmp = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31) & ~0x80;
			VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31, tmp);


			/*   removed by Boyod */
			/* clear centering bits in 3d4.9e */
			/*
			   tmp = VGAIN8_INDEX(pSmi, vgaCRIndex, vgaCRData, 0x9E) & ~0x00;
			   VGAOUT8_INDEX(pSmi, vgaCRIndex, vgaCRData, 0x9E, tmp);
			 */
		}
	}
#endif


	if (xf86GetVerbosity () > 1)
	{
		xf86DrvMsgVerb (pScrn->scrnIndex, X_INFO, VERBLEV,
				"Done restoring mode.  David Register dump Line:%d:\n", __LINE__);
		SMI_PrintRegs (pScrn);
	}

	vgaHWProtect (pScrn, FALSE);

}

	static Bool
SMI_MapMem (ScrnInfoPtr pScrn)
{
	SMIPtr pSmi = SMIPTR (pScrn);
	SMIRegPtr pRegPtr = (SMIRegPtr)pSmi->pEnt_private->ptr;

	vgaHWPtr hwp;
	CARD32 memBase;
	int	ret;	

	/* map general resource */

	switch (pSmi->Chipset)
	{
		default:
			ret = smi_mapmemory(pScrn, pSmi);
			break;
#ifndef XSERVER_LIBPCIACCESS	
		case SMI_COUGAR3DR:
			ret = smi_mapmemory_730(pScrn, pSmi);
			break;

		case SMI_LYNX3D:
			ret = smi_mapmemory_820(pScrn, pSmi);
			break;

		case SMI_LYNXEM:
		case SMI_LYNXEMplus:		
			ret = smi_mapmemory_712(pScrn, pSmi);
			break;

		case SMI_LYNX3DM:
			ret = smi_mapmemory_720(pScrn, pSmi);
			break;
#endif
		case SMI_MSOC:
			ret = smi_mapmemory_501(pScrn, pSmi);
			break;
		case SMI_MSOCE:
			ret = smi_mapmemory_750(pScrn, pSmi);
			break;	  
	}
	if (ret == FALSE)
		return (ret);

	/* map legency resource for vga card */

	if(pSmi->Chipset != SMI_MSOC)
	{
		/* Assign hwp->MemBase & IOBase here */
		hwp = VGAHWPTR (pScrn);
		if (pSmi->IOBase != NULL)
		{
			vgaHWSetMmioFuncs (hwp, pSmi->MapBase,
					pSmi->IOBase - pSmi->MapBase);
		}
		vgaHWGetIOBase(hwp);

		/* Map the VGA memory when the primary video */
		if (xf86IsPrimaryPci (pSmi->PciInfo))
		{
			hwp->MapSize = 0x10000;
			if (!vgaHWMapMem (pScrn))
			{
				return (FALSE);
			}
			pSmi->PrimaryVidMapped = TRUE;
		}
	}

	return (TRUE);
}

/* UnMapMem - contains half of pre-4.0 EnterLeave function.  The EnterLeave
 * function which en/disable access to IO ports and ext. regs
 */

	static void
SMI750_UnmapMem (ScrnInfoPtr pScrn)
{
	SMIPtr pSmi = SMIPTR (pScrn);	
	SMIRegPtr pRegPtr = (SMIRegPtr)pSmi->pEnt_private->ptr;

	ENTER();

	/* Unmap VGA mem if mapped. */
	if (pSmi->PrimaryVidMapped)
	{
		vgaHWUnmapMem (pScrn);
		pSmi->PrimaryVidMapped = FALSE;
	}

	SMI_DisableMmio (pScrn);

	if(pRegPtr->mmio_require == 1)
	{
		pRegPtr->mmio_require--;	
		if(pRegPtr->MMIOBase)
		{
#ifndef XSERVER_LIBPCIACCESS
			xf86UnMapVidMem (pScrn->scrnIndex,
					(pointer) pRegPtr->MMIOBase,pRegPtr->MapSize);
#else
			pci_device_unmap_range(pSmi->PciInfo,
					(pointer)pRegPtr->MMIOBase,pRegPtr->MapSize);

#endif
			pRegPtr->MMIOBase = NULL;				
		}
	}

	if(pSmi->FBBase){
#ifndef XSERVER_LIBPCIACCESS
		xf86UnMapVidMem (pScrn->scrnIndex, 
				(pointer) pSmi->FBBase,pSmi->videoRAMBytes);
#else
		pci_device_unmap_range(pSmi->PciInfo,
				(pointer)pSmi->FBBase,pSmi->videoRAMBytes);
#endif
	}	

	pSmi->MapBase = NULL;//mmio base
	pSmi->FBBase = NULL;	
	pSmi->IOBase = NULL;

	LEAVE();
}

	static void
SMI_UnmapMem (ScrnInfoPtr pScrn)
{
	SMIPtr pSmi = SMIPTR (pScrn);
	ENTER();

	/* 750 unmap use another routine */
	if(pSmi->Chipset == SMI_MSOCE){
		SMI750_UnmapMem(pScrn);		
		LEAVE();
	}


	/* Unmap VGA mem if mapped. */
	if (pSmi->PrimaryVidMapped)
	{
		vgaHWUnmapMem (pScrn);
		pSmi->PrimaryVidMapped = FALSE;
	}

	SMI_DisableMmio (pScrn);

	if(pSmi->MapBase){
#ifndef XSERVER_LIBPCIACCESS
		xf86UnMapVidMem (pScrn->scrnIndex, (pointer) pSmi->MapBase, pSmi->MapSize);
#else
		if(pSmi->IsPrimary) {
			pci_device_unmap_range(pSmi->PciInfo,(pointer)pSmi->MapBase,pSmi->MapSize);
		}
#endif
	}

	if (pSmi->FBBase){
#ifndef XSERVER_LIBPCIACCESS
		xf86UnMapVidMem (pScrn->scrnIndex, (pointer) pSmi->FBBase,pSmi->videoRAMBytes);
#else
		pci_device_unmap_range(pSmi->PciInfo,(pointer)pSmi->FBBase,pSmi->videoRAMBytes);
#endif
	}
	pSmi->IOBase = NULL;	
	LEAVE();
}


/* This gets called at the start of each server generation. */

static void SMI_SetSeq_720(ScrnInfoPtr pScrn)
{
	SMIPtr pSmi = SMIPTR (pScrn);
	int i;
	VGAOUT8 (pSmi, 0x3c2, 0x67);
	for (i = 0x00; i <= 0xAF; i++)
	{
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA,
				i, BIOS_SequencerReg720[i]);
	}
	for (i = 0x00; i <= 0x08; i++)
	{
		VGAOUT8_INDEX (pSmi, VGA_GRAPH_INDEX, VGA_GRAPH_DATA,
				i, BIOS_Graphic_ctl[i]);
	}
	for (i = 0x00; i <= 0x14; i++)
	{
		VGAOUT8_INDEX (pSmi, VGA_ATTR_INDEX, VGA_ATTR_DATA_R,
				i, BIOS_AT_CTL[i]);
	}

}

	static void
SMI_RegInit(ScrnInfoPtr pScrn)
{
	SMIPtr pSmi = SMIPTR (pScrn);
	int chipType;
#ifndef XSERVER_LIBPCIACCESS
	chipType = pSmi->PciInfo->chipType;
#else
	chipType = PCI_DEV_DEVICE_ID(pSmi->PciInfo);	//caesar modified
#endif
	xf86DrvMsg (pScrn->scrnIndex, X_INFO, " David Chip is 0x%x\n", chipType);

	switch (chipType)
	{
		case SMI_COUGAR3DR:
		case SMI_LYNX3D:
		case SMI_LYNXEM:
		case SMI_LYNXEMplus:		
			break;
		case SMI_LYNX3DM:
			SMI_SetSeq_720(pScrn);
			break;
		case SMI_MSOC:
		case SMI_MSOCE:
			break;	  
		default:
			break;
	}
}


static void SMI750_Save(ScrnInfoPtr pScrn)
{
	int 	i, j;	
	SMIPtr	pSmi = SMIPTR(pScrn);
	SMIRegPtr pRegPtr = (SMIRegPtr)pSmi->pEnt_private->ptr;
	SMI750RegPtr save = (SMI750RegPtr)pRegPtr->save750;

	ENTER();

	/* only fist pScrn need save all registers */
	if(pSmi->IsPrimary && save){		
		/* save Fonts */
		SMI_SaveFonts(pScrn);
		/* save mmio */
		for (i = REG_SYS_HEAD, j = 0; i <= REG_SYS_TAIL; i += 4, j++){
			save->System[j] = PEEK32(i);
		}

		for (i = REG_PNL_HEAD, j = 0; i <= REG_PNL_TAIL; i += 4, j++){
			save->PanelControl[j] = PEEK32(i);
		}

		for (i = REG_CRT_HEAD, j = 0; i<=REG_CRT_TAIL; i+= 4, j++) {
			save->CRTControl[j] = PEEK32(i);
		}

		for(i=REG_PCUR_HEAD,j = 0;i<=REG_PCUR_TAIL;i+=4,j++){
			save->PriCursorControl[j] = PEEK32(i);
		}

		for(i=REG_SCUR_HEAD,j = 0;i<=REG_SCUR_TAIL;i+=4,j++){
			save->SecCursorControl[j] = PEEK32(i);
		}	
	}
	LEAVE();
}

static void SMI501_Save(ScrnInfoPtr pScrn)
{

}

static void SMI750_Restore(ScrnInfoPtr pScrn)
{
	int 	i, j;
	SMIPtr	pSmi = SMIPTR(pScrn);	
	SMIRegPtr pRegPtr = (SMIRegPtr)pSmi->pEnt_private->ptr;
	SMI750RegPtr save = (SMI750RegPtr)pRegPtr->save750;

	ENTER();

	XERR("pSmi->IsPrimary = %d,save = %p\n",pSmi->IsPrimary,save);

	/* only first pScrn need restore regsiter context */	
	if(pSmi->IsPrimary && save)
	{
		/* restore mmio */
		for(i = REG_SYS_HEAD,j = 0 ;i<= REG_SYS_TAIL ;i+=4,j++)
		{
			POKE32(i,save->System[j]);
		}

		for(i = REG_PNL_HEAD, j = 0 ;i<= REG_PNL_TAIL ;i+=4,j++)
		{
			POKE32(i,save->PanelControl[j]);
		}

		for(i = REG_CRT_HEAD,j = 0 ;i<= REG_CRT_TAIL ;i+=4,j++)
		{
			POKE32(i,save->CRTControl[j]);
		}

		for(i = REG_PCUR_HEAD,j = 0 ;i<= REG_PCUR_TAIL ;i+=4,j++)
		{		 
			POKE32(i,save->PriCursorControl[j]);
		}


		for(i = REG_SCUR_HEAD,j = 0 ;i<= REG_SCUR_TAIL ;i+=4,j++)
		{		
			POKE32(i,save->SecCursorControl[j]);
		}		

	}		

	/* restore fonts */ 	
	SMI_RestoreFonts(pScrn);	
	LEAVE();	
}

static Bool
SMI_ScreenInit (int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	SMIPtr pSmi = SMIPTR (pScrn);
	EntityInfoPtr pEnt;

	ENTER();

	pEnt = xf86GetEntityInfo (pScrn->entityList[0]);

#ifdef _XSERVER64
	if (!pSmi->pInt10 && SMI_MSOC != pSmi->Chipset && pSmi->RemoveBIOS == FALSE)
	{
		//david temp adde for 720 bois remove
		//pSmi->pInt10 = xf86InitInt10 (pEnt->index);
	}
#endif

	/* Map MMIO regs and framebuffer */
	if (!SMI_MapMem (pScrn))
	{
		LEAVE (FALSE);
	}

	/* Save the chip/graphics state */  
	if(pSmi->Chipset != SMI_MSOC){
		if(pSmi->Chipset != SMI_MSOCE)
			SMI_Save (pScrn);		//lynx 712 7122 save funtion
		else
			SMI750_Save(pScrn);		//sm750 save function
	}
	else{
		SMI501_Save(pScrn);
	}


	if (!SMI_ModeInit (pScrn, pScrn->currentMode))
	{
		LEAVE (FALSE);
	}

	if(pSmi->Chipset == SMI_MSOC || pSmi->Chipset == SMI_MSOCE)
	{
		/* shutdown hardware console's cursor */
		int d = PEEK32(0x800f0);
		d = d & 0x7fffffff;
		POKE32(0x800f0,d);
	}
	memset (pSmi->FBBase, 0, pSmi->videoRAMBytes);

	/*
	 * The next step is to setup the screen's visuals, and initialise the
	 * framebuffer code.  In cases where the framebuffer's default choises for
	 * things like visual layouts and bits per RGB are OK, this may be as simple
	 * as calling the framebuffer's ScreenInit() function.  If not, the visuals
	 * will need to be setup before calling a fb ScreenInit() function and fixed
	 * up after.
	 *
	 * For most PC hardware at depths >= 8, the defaults that cfb uses are not
	 * appropriate.  In this driver, we fixup the visuals after.
	 */

	/*
	 * Reset the visual list.
	 */
	miClearVisualTypes ();

	/* Setup the visuals we support. */

	/*
	 * For bpp > 8, the default visuals are not acceptable because we only
	 * support TrueColor and not DirectColor.  To deal with this, call
	 * miSetVisualTypes with the appropriate visual mask.
	 */

	if (!miSetVisualTypes (pScrn->depth, TrueColorMask, pScrn->rgbBits,
				pScrn->defaultVisual))
	{
		LEAVE (FALSE);
	}

	/*#ifdef USE_FB*/
	if (!miSetPixmapDepths ())
	{
		LEAVE(FALSE);
	}

	/* #endif */

	if (!SMI_InternalScreenInit (scrnIndex, pScreen))
	{
		LEAVE (FALSE);
	}



	xf86SetBlackWhitePixels (pScreen);

	if (pScrn->bitsPerPixel > 8)
	{
		VisualPtr visual;
		/* Fixup RGB ordering */
		visual = pScreen->visuals + pScreen->numVisuals;
		while (--visual >= pScreen->visuals)
		{
			if ((visual->class | DynamicClass) == DirectColor)
			{
				visual->offsetRed = pScrn->offset.red;
				visual->offsetGreen = pScrn->offset.green;
				visual->offsetBlue = pScrn->offset.blue;
				visual->redMask = pScrn->mask.red;
				visual->greenMask = pScrn->mask.green;
				visual->blueMask = pScrn->mask.blue;
			}
		}
	}

	/* #ifdef USE_FB*/
	/* must be after RGB ordering fixed */

	fbPictureInit (pScreen, 0, 0);

	/*#endif*/

	/* CZ 18.06.2001: moved here from smi_accel.c to have offscreen
	   framebuffer in NoAccel mode */
	{
		int numLines, maxLines;
		BoxRec AvailFBArea;

		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BDEBUG: pSmi->FBReserved is %d, pSmi->width is %d, pSmi->Bpp is %d\n", pSmi->FBReserved, pSmi->width, pSmi->Bpp);
		//    maxLines = pSmi->FBReserved / (pSmi->width * pSmi->Bpp);
		maxLines = pSmi->FBReserved / (pScrn->displayWidth * pSmi->Bpp);
		if (pSmi->rotate)
		{
			numLines = maxLines;
		}
		else
		{
			/* CZ 3.11.2001: What does the following code? see also smi_video.c aaa line 1226 */
			/*#if SMI_USE_VIDEO */
#if 0
			numLines = ((pSmi->FBReserved - pSmi->width * pSmi->Bpp
						* pSmi->height) * 25 / 100 + pSmi->width
					* pSmi->Bpp - 1) / (pSmi->width * pSmi->Bpp);
			numLines += pSmi->height;
#else
			numLines = maxLines;
#endif
		}

		AvailFBArea.x1 = 0;
		AvailFBArea.y1 = 0;
		AvailFBArea.x2 = (pSmi->width + 15) & ~15;;
		//    AvailFBArea.x2 = pScrn->displayWidth;
		AvailFBArea.y2 = numLines;
		xf86DrvMsg (pScrn->scrnIndex, X_INFO,
				"FrameBuffer Box: %d,%d - %d,%d\n",
				AvailFBArea.x1, AvailFBArea.y1, AvailFBArea.x2,
				AvailFBArea.y2);

		xf86InitFBManager (pScreen, &AvailFBArea);

	}
	/* end CZ */

#if 1
	/* Initialize acceleration layer */
	if (!pSmi->NoAccel)
	{
		if (!SMI_AccelInit (pScreen))
		{
			LEAVE(FALSE);
		}
	}
#endif
	miInitializeBackingStore (pScreen);

	/* hardware cursor needs to wrap this layer */
	SMI_DGAInit (pScreen);

	/* Initialise cursor functions */
	miDCInitialize (pScreen, xf86GetPointerScreenFuncs ());

	/* Initialize HW cursor layer.  Must follow software cursor
	 * initialization.
	 */

	/*
	   monk @ 10/14/2010:
	   for non-xrandr arch driver running on X server 1.6+ (maybe 1.51+ not sure)
	   different depth of two screen will make system hang after move cursor from primary screen to 
	   secondary screen because of bugs of X server
	   but if hardware cursor enabled, the bug will disappare 
	   Let's enable hardware cursor if screens with different color depth
	 */
#if 1 	//caesar removed 
	if (pSmi->hwcursor)
	{
		if (!SMI_HWCursorInit (pScreen))
		{
			xf86DrvMsg (pScrn->scrnIndex, X_ERROR, "Hardware cursor "
					"initialization failed\n");
		}
	}
#endif

	if (pSmi->shadowFB)
	{
		Bool bRetCode;

		RefreshAreaFuncPtr refreshArea;

		if (pSmi->Chipset == SMI_COUGAR3DR)
		{
			refreshArea = SMI_RefreshArea730;
		}
		else
		{
			refreshArea = SMI_RefreshArea;
		}


		if (pSmi->rotate || pSmi->RandRRotation) //caesar modify
		{
			if (pSmi->PointerMoved == NULL)
			{
				pSmi->PointerMoved = pScrn->PointerMoved;
				pScrn->PointerMoved = SMI_PointerMoved;
			}
		}

		bRetCode = ShadowFBInit (pScreen, refreshArea);
	}

	/* Initialise default colormap */
	if (!miCreateDefColormap (pScreen))
	{
		LEAVE (FALSE);
	}

	/* Initialize colormap layer.  Must follow initialization of the default
	 * colormap.  And SetGamma call, else it will load palette with solid white.
	 */
	/* CZ 2.11.2001: CMAP_PALETTED_TRUECOLOR for gamma correction */
	if (!xf86HandleColormaps
			(pScreen, 256, pScrn->rgbBits, SMI_LoadPalette, NULL,
			 CMAP_RELOAD_ON_MODE_SWITCH | CMAP_PALETTED_TRUECOLOR))
	{
		LEAVE (FALSE);
	}

	pScreen->SaveScreen = SMI_SaveScreen;
	pSmi->CloseScreen = pScreen->CloseScreen;
	pScreen->CloseScreen = SMI_CloseScreen;

	/* Added by Belcon to enable LCD Panel Control Select */
	if (pSmi->Chipset == SMI_LYNXEMplus)
	{
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x22, 2);
	}

	if (!xf86DPMSInit (pScreen, SMI_DisplayPowerManagementSet, 0))
	{
		xf86DrvMsg (pScrn->scrnIndex, X_ERROR, "DPMS initialization failed!\n");
	}

	SMI_InitVideo (pScreen);

#ifdef RANDR
	/* Install our DriverFunc.  We have to do it this way instead of using the
	 * HaveDriverFuncs argument to xf86AddDriver, because InitOutput clobbers
	 * pScrn->DriverFunc */
	pScrn->DriverFunc = SMI_DriverFunc;
#endif


	/* Report any unused options (only for the first generation) */
	if (serverGeneration == 1)
	{
		xf86ShowUnusedOptions (pScrn->scrnIndex, pScrn->options);
	}


#ifndef XSERVER_LIBPCIACCESS
	{
		/*don't do that, make system hang deadly */
#if 0	
		volatile CARD8	config;
		config = pciReadByte(pSmi->PciTag, PCI_CMD_STAT_REG);
		pciWriteByte(pSmi->PciTag, PCI_CMD_STAT_REG, config | PCI_CMD_MEM_ENABLE);
#endif		
	}
#endif
	if(pSmi->edid_enable)
	{
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "david print virtualX=%d, virtualY = %d\n", pScrn->virtualX, pScrn->virtualY);
		pScrn->virtualX = pScrn->currentMode ->HDisplay;
		pScrn->virtualY = pScrn->currentMode->VDisplay;
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "david print virtualX=%d, virtualY = %d\n", pScrn->virtualX, pScrn->virtualY);
	}
	LEAVE (TRUE);
}

/* Common init routines needed in EnterVT and ScreenInit */

	static int
SMI_InternalScreenInit (int scrnIndex, ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	SMIPtr pSmi = SMIPTR (pScrn);
	int width, height, displayWidth;
	int bytesPerPixel = pScrn->bitsPerPixel / 8;
	int xDpi, yDpi;
	int ret;

	ENTER();


	if (pSmi->rotate && pSmi->rotate != SMI_ROTATE_UD)
	{
		width = pScrn->virtualY;
		//      width = (pScrn->virtualY + 15) & ~15;
		height = pScrn->virtualX;
		//      height = (pScrn->virtualX + 15) & ~15;
		xDpi = pScrn->yDpi;
		yDpi = pScrn->xDpi;
		displayWidth = ((width * bytesPerPixel + 15) & ~15) / bytesPerPixel;
	}
	else
	{
		width = pScrn->virtualX;
		//	  width = (pScrn->virtualX + 15) & ~15;		
		height = pScrn->virtualY;
		//      height = (pScrn->virtualY + 15) & ~15;
		xDpi = pScrn->xDpi;
		yDpi = pScrn->yDpi;
		displayWidth = pScrn->displayWidth;
	}

	if (pSmi->shadowFB)
	{
		pSmi->ShadowWidth = width;
		pSmi->ShadowHeight = height;
		//      pSmi->ShadowWidthBytes = (width * bytesPerPixel + 15) & ~15;
		pSmi->ShadowWidthBytes = ((width + 15) & ~15) * bytesPerPixel;
		if (bytesPerPixel == 3)
		{
			if(pSmi->rotate == SMI_ROTATE_CW || pSmi->rotate == SMI_ROTATE_CCW)	
			{
				pSmi->ShadowPitch = ((height * 3) << 16) | pSmi->ShadowWidthBytes;
			}
			else
			{
				pSmi->ShadowPitch = ((width * 3) << 16) | pSmi->ShadowWidthBytes;
			}
		}
		else
		{

			if(pSmi->rotate == SMI_ROTATE_CW || pSmi->rotate == SMI_ROTATE_CCW)	
			{
				//	  		pSmi->ShadowPitch = (height << 16) |(pSmi->ShadowWidthBytes / bytesPerPixel);
				pSmi->ShadowPitch = (((height + 15) & ~15)<< 16) |(pSmi->ShadowWidthBytes / bytesPerPixel);				
			}
			else
			{
				//			pSmi->ShadowPitch = (width << 16) |(pSmi->ShadowWidthBytes / bytesPerPixel);	
				pSmi->ShadowPitch = (((width + 15) & ~15)<< 16) |(pSmi->ShadowWidthBytes / bytesPerPixel);	
			}

		}



		pSmi->saveBufferSize = pSmi->ShadowWidthBytes * pSmi->ShadowHeight;
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "line %d: pSmi->FBReserved is 0x%x\n", __LINE__, pSmi->FBReserved);
		pSmi->FBReserved -= pSmi->saveBufferSize;
		pSmi->FBReserved &= ~0x15;
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "line %d: pSmi->FBReserved is 0x%x\n", __LINE__, pSmi->FBReserved);
		WRITE_VPR (pSmi, 0x0C, (pSmi->FBOffset = pSmi->FBReserved) >> 3);

		if (pSmi->Chipset == SMI_COUGAR3DR)
		{
			WRITE_FPR (pSmi, FPR0C, pSmi->FBOffset >> 3);
		}

#if 1
		if ((SMI_MSOC == pSmi->Chipset) || (SMI_MSOCE == pSmi->Chipset))
		{
			xf86DrvMsg("", X_INFO, "pSmi->SCRBase is 0x%x, DCRBase is 0x%x, FBOffset is 0x%x\n", pSmi->SCRBase, pSmi->DCRBase, pSmi->FBOffset);
			if (!pSmi->IsSecondary) {
				WRITE_DCR (pSmi, DCR0C, pSmi->FBOffset);
			} else {
				WRITE_DCR (pSmi, DCR204, pSmi->FBOffset);
			}
		}
#endif

		pScrn->fbOffset = pSmi->FBOffset + pSmi->fbMapOffset;
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "Shadow: width=%d height=%d "
				"offset=0x%08X pitch=0x%08X\n", pSmi->ShadowWidth,
				pSmi->ShadowHeight, pSmi->FBOffset, pSmi->ShadowPitch);
		xf86DrvMsg("", X_INFO, "line %d: Internalxxx: fbOffset = 0x%x\n", __LINE__, pScrn->fbOffset);
	}
	else
	{
#if 0
		pSmi->FBOffset = 0;
		pScrn->fbOffset = pSmi->FBOffset + pSmi->fbMapOffset;
#endif
		xf86DrvMsg("", X_INFO, "line %d: Internalxxx: fbOffset = 0x%x, FBOffset is 0x%x\n", __LINE__, pScrn->fbOffset, pSmi->FBOffset);
	}

	/*
	 * Call the framebuffer layer's ScreenInit function, and fill in other
	 * pScreen fields.
	 */


	DEBUG ("\tInitializing FB @ 0x%08X for %dx%d (%d)\n",
			pSmi->FBBase, width, height, displayWidth);
	switch (pScrn->bitsPerPixel)
	{
		/* #ifdef USE_FB*/
		case 8:
		case 16:
		case 24:
		case 32:

#ifdef XORG_VERSION_CURRENT
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
			if ((SMI_MSOC != pSmi->Chipset) && (pScrn->bitsPerPixel == 32) && (SMI_MSOCE != pSmi->Chipset))
				ret = fbScreenInit (pScreen, pSmi->FBBase, width, height, xDpi,
						yDpi, displayWidth, pScrn->bitsPerPixel);
#endif
#endif
			if ((SMI_MSOC == pSmi->Chipset && pScrn->bitsPerPixel != 24) ||
					(SMI_MSOC != pSmi->Chipset && pScrn->bitsPerPixel != 32) || 
					(SMI_MSOCE == pSmi->Chipset && pScrn->bitsPerPixel != 24) || 
					(SMI_MSOCE != pSmi->Chipset && pScrn->bitsPerPixel != 32))

			{
				ret = fbScreenInit (pScreen, pSmi->FBBase, width, height, xDpi,
						yDpi, displayWidth, pScrn->bitsPerPixel);
			}
			break;

		default:
			xf86DrvMsg (scrnIndex, X_ERROR, "Internal error: invalid bpp (%d) "
					"in SMI_InternalScreenInit\n", pScrn->bitsPerPixel);
			LEAVE (FALSE);
	}

	if ((SMI_MSOC == pSmi->Chipset) && (pScrn->bitsPerPixel == 8) ||
			(SMI_MSOCE == pSmi->Chipset) && (pScrn->bitsPerPixel == 8))
	{
		/* Initialize Palette entries 0 and 1, they don't seem to get hit */
		if (!pSmi->IsSecondary)
		{
			WRITE_DCR (pSmi, DCR400 + 0, 0x00000000);	/* CRT Palette       */
			WRITE_DCR (pSmi, DCR400 + 4, 0x00FFFFFF);	/* CRT Palette       */
		}
		else
		{
			WRITE_DCR (pSmi, DCR800 + 0, 0x00000000);	/* Panel Palette */
			WRITE_DCR (pSmi, DCR800 + 4, 0x00FFFFFF);	/* Panel Palette */
		}

	}

	LEAVE (ret);
}

/* Checks if a mode is suitable for the selected configuration. */
	static ModeStatus
SMI_ValidMode (int scrnIndex, DisplayModePtr mode, Bool verbose, int flags)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	SMIPtr pSmi = SMIPTR (pScrn);
	float refresh;
	int tmpRfrsh;

	ENTER();

	// xf86DrvMsg("", X_INFO, "Belcon: %d x %d , %d-bpp\n", mode->HDisplay, mode->VDisplay, pScrn->bitsPerPixel);
	refresh = (mode->VRefresh > 0) ? mode->VRefresh
		: mode->Clock * 1000.0 / mode->VTotal / mode->HTotal;
	xf86DrvMsg (scrnIndex, X_INFO, " Mode: %dx%d %d-bpp, %fHz\n",
			mode->HDisplay, mode->VDisplay, pScrn->bitsPerPixel, refresh);

	tmpRfrsh = (int)(refresh + 0.5);

	if (pSmi->shadowFB)
	{
		int mem;

		/*		if (pScrn->bitsPerPixel > 16)
				{
				return(MODE_BAD);
				}
		 */
		mem = (pScrn->virtualX * pScrn->bitsPerPixel / 8 + 15) & ~15;
		mem *= pScrn->virtualY * 2;

		if (mem > pSmi->FBReserved)	/* PDR#1074 */
		{
			LEAVE (MODE_MEM);
		}
	}

	/*
	 * FIXME:
	 *	Why we need these code here?
	 * #if 1
	 */
#if 0
	if (!pSmi->useBIOS || pSmi->lcd)
	{

#if 1				/* PDR#983 */
		if (pSmi->zoomOnLCD)
		{
			xf86DrvMsg("", X_INFO, "HDisplay %d, lcdWidth %d\n", mode->HDisplay, pSmi->lcdWidth);
			if ((mode->HDisplay > pSmi->lcdWidth)
					|| (mode->VDisplay > pSmi->lcdHeight))
			{
				return (MODE_PANEL);
			}
		}
		else
#endif
		{
			if ((mode->HDisplay != pSmi->lcdWidth)
					|| (mode->VDisplay != pSmi->lcdHeight))
			{
				return (MODE_PANEL);
			}
		}

	}
#endif
	tmpRfrsh = (int)(refresh + 0.5);
	if((tmpRfrsh != 60) &&
			(tmpRfrsh !=75) &&
			(tmpRfrsh != 85)) {
		return MODE_NOMODE;
	}

	// Mode added by Belcon according new mode table
	if(SMI_MSOCE == pSmi->Chipset)
	{
		if(!pSmi->edid_enable)
		{
			if (!(((mode->HDisplay == 1280) && (mode->VDisplay == 1024)) ||
						((mode->HDisplay == 1024) && (mode->VDisplay == 768)) ||
						((mode->HDisplay == 800) && (mode->VDisplay == 600)) ||
						((mode->HDisplay == 640) && (mode->VDisplay == 480)) ||
						((mode->HDisplay == 320) && (mode->VDisplay == 240)) ||
						((mode->HDisplay == 400) && (mode->VDisplay == 300)) ||
						((mode->HDisplay == 1280) && (mode->VDisplay == 960)) ||
						((mode->HDisplay == 1280) && (mode->VDisplay == 768)) ||
						((mode->HDisplay == 1280) && (mode->VDisplay == 720)) ||
						((mode->HDisplay == 1366) && (mode->VDisplay == 768)) ||
						((mode->HDisplay == 1360) && (mode->VDisplay == 768)) ||
						((mode->HDisplay == 1024) && (mode->VDisplay == 600)) ||
						((mode->HDisplay == 800) && (mode->VDisplay == 480)) ||
						((mode->HDisplay == 720) && (mode->VDisplay == 540)) ||
						((mode->HDisplay == 1440) && (mode->VDisplay == 960)) ||
						((mode->HDisplay == 1600) && (mode->VDisplay == 1200)) ||
						((mode->HDisplay == 1920) && (mode->VDisplay == 1080)) ||
						((mode->HDisplay == 1920) && (mode->VDisplay == 1200)) ||
						((mode->HDisplay == 1920) && (mode->VDisplay == 1440)) ||
						((mode->HDisplay == 720) && (mode->VDisplay == 480))))
			{
				xf86DrvMsg(pScrn->scrnIndex, X_INFO, "HDisplay %d, VDisplay %d\n", mode->HDisplay, mode->VDisplay);
				return (MODE_BAD_WIDTH);
			}
		}
	}
	else
	{
		if (!(((mode->HDisplay == 1280) && (mode->VDisplay == 1024)) ||
					((mode->HDisplay == 1024) && (mode->VDisplay == 768)) ||
					((mode->HDisplay == 800) && (mode->VDisplay == 600)) ||
					((mode->HDisplay == 640) && (mode->VDisplay == 480)) ||
					((mode->HDisplay == 320) && (mode->VDisplay == 240)) ||
					((mode->HDisplay == 400) && (mode->VDisplay == 300)) ||
					((mode->HDisplay == 1280) && (mode->VDisplay == 960)) ||
					((mode->HDisplay == 1280) && (mode->VDisplay == 768)) ||
					((mode->HDisplay == 1280) && (mode->VDisplay == 720)) ||
					((mode->HDisplay == 1366) && (mode->VDisplay == 768)) ||
					((mode->HDisplay == 1360) && (mode->VDisplay == 768)) ||
					((mode->HDisplay == 1024) && (mode->VDisplay == 600)) ||
					((mode->HDisplay == 800) && (mode->VDisplay == 480)) ||
					((mode->HDisplay == 720) && (mode->VDisplay == 540)) ||
					((mode->HDisplay == 1440) && (mode->VDisplay == 960)) ||
					((mode->HDisplay == 720) && (mode->VDisplay == 480))))
		{
			xf86DrvMsg(pScrn->scrnIndex, X_INFO, "HDisplay %d, VDisplay %d\n", mode->HDisplay, mode->VDisplay);
			return (MODE_BAD_WIDTH);
		}
	}

#if 1				/* PDR#944 */
	if ((pSmi->rotate && pSmi->rotate != SMI_ROTATE_UD) && (SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
	{
		if ((mode->HDisplay != pSmi->lcdWidth)
				|| (mode->VDisplay != pSmi->lcdHeight))
		{
			LEAVE (MODE_PANEL);
		}
	}
#endif

	LEAVE (MODE_OK);
}




static void SMI_SaveReg_502(ScrnInfoPtr pScrn )
{
	int i;
	vgaHWPtr hwp = VGAHWPTR (pScrn);
	SMIPtr pSmi = SMIPTR (pScrn);
	int vgaIOBase = hwp->IOBase;
	int vgaCRIndex = vgaIOBase + VGA_CRTC_INDEX_OFFSET;
	int vgaCRData = vgaIOBase + VGA_CRTC_DATA_OFFSET;

	ENTER();

	/*save seq registers*/
	for(i = 0; i < (sizeof(SeqTable) / sizeof(unsigned char)); i++)
	{
		SeqTable[i] = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, i);	
	}

	/*save crt registers*/
	for(i = 0; i < (sizeof(CrtTable) / sizeof(unsigned char)); i++)
	{
		CrtTable[i] = VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, i);
	}

	/*save dpr registers*/
	for(i = 0; i < (sizeof(DprTable) / sizeof(unsigned int)); i++)
	{
		DprTable[i] = READ_DPR(pSmi, DprIndexTable[i]);
	}
	for(i = 0; i < (sizeof(VprTable_501) / sizeof(unsigned int)); i++)
	{
		VprTable_501[i] = READ_VPR(pSmi, VprIndexTable_501[i]);
	}

	LEAVE();
}
static void SMI_SaveReg_750(ScrnInfoPtr pScrn )
{
	int i;
	vgaHWPtr hwp = VGAHWPTR (pScrn);
	SMIPtr pSmi = SMIPTR (pScrn);
	int vgaIOBase = hwp->IOBase;
	int vgaCRIndex = vgaIOBase + VGA_CRTC_INDEX_OFFSET;
	int vgaCRData = vgaIOBase + VGA_CRTC_DATA_OFFSET;
#if  0	
	/*save seq registers*/
	for(i = 0; i < (sizeof(SeqTable) / sizeof(unsigned char)); i++)
	{
		SeqTable[i] = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, i);	
		/*
		   if ( 0x18 == i) {
		   SeqTable[i] |= 0x11;
		   }
		 */
	}
	//	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "SaveReg, SR18 is 0x%x\n", SeqTable[18]);

	/*save crt registers*/
	for(i = 0; i < (sizeof(CrtTable) / sizeof(unsigned char)); i++)
	{
		CrtTable[i] = VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRData, i);
	}
#endif	
	/*save dpr registers*/
	for(i = 0; i < (sizeof(DprTable) / sizeof(unsigned int)); i++)
	{
		DprTable[i] = READ_DPR(pSmi,DprIndexTable[i]);
	}
	for(i = 0; i < (sizeof(VprTable_750) / sizeof(unsigned int)); i++)
	{
		VprTable_750[i] = READ_VPR(pSmi, VprIndexTable_750[i]);
	}

	for(i = 0; i < (sizeof(DcrTable) / sizeof(unsigned int)); i++)
	{
		DcrTable[i] = READ_DCR(pSmi, DcrIndexTable[i]);
		xf86DrvMsg("", X_INFO, "T-Bag:Test, DcrTable[i] is %x  FUNC: %s\n", DcrTable[i],__FUNCTION__); 
	}

	LEAVE();
}
static void SMI_SaveReg_Con(ScrnInfoPtr pScrn )
{
	int i;
	vgaHWPtr hwp = VGAHWPTR (pScrn);
	SMIPtr pSmi = SMIPTR (pScrn);
	//      int vgaIOBase = hwp->IOBase;
	//     int vgaCRIndex = vgaIOBase + VGA_CRTC_INDEX_OFFSET;
	//    int vgaCRData = vgaIOBase + VGA_CRTC_DATA_OFFSET;

	for(i = 0; i < (sizeof(DprTable_Con) / sizeof(unsigned int)); i++)
	{
		DprTable_Con[i] = READ_DPR(pSmi,DprIndexTable_Con[i]);
	}
	for(i = 0; i < (sizeof(VprTable_Con) / sizeof(unsigned int)); i++)
	{
		VprTable_Con[i] = READ_VPR(pSmi, VprIndexTable_Con[i]);
	}

	for(i = 0; i < (sizeof(DcrTable_Con) / sizeof(unsigned int)); i++)
	{
		DcrTable_Con[i] = READ_DCR(pSmi, DcrIndexTable_Con[i]);
		xf86DrvMsg("", X_INFO, "T-Bag:Test, DcrTable_Con[i] is %x  FUNC: %s\n", 

				DcrTable_Con[i],__FUNCTION__);
	}

	return;
}

static void SMI_RestoreReg_502(ScrnInfoPtr pScrn )
{
	int i;
	vgaHWPtr hwp = VGAHWPTR (pScrn);
	SMIPtr pSmi = SMIPTR (pScrn);
	int vgaIOBase = hwp->IOBase;
	int vgaCRIndex = vgaIOBase + VGA_CRTC_INDEX_OFFSET;
	int vgaCRData = vgaIOBase + VGA_CRTC_DATA_OFFSET;

	/*restore seq registers*/
	for(i = 0; i < (sizeof(SeqTable) / sizeof(unsigned char)); i++)
	{
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, i, SeqTable[i]);	
	}

	//	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "RestoreReg, SR18 is 0x%x\n", SeqTable[18]);
	/*restore crt registers*/
	for(i = 0; i < (sizeof(CrtTable) / sizeof(unsigned char)); i++)
	{	
		VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, i, CrtTable[i]);	
	}

	/*restore dpr registers*/
	for(i = 0; i < (sizeof(DprTable) / sizeof(unsigned int)); i++)
	{
		WRITE_DPR(pSmi, DprIndexTable[i], DprTable[i]); 
	}
	for(i = 0; i < (sizeof(VprTable_501) / sizeof(unsigned int)); i++)
	{
		WRITE_VPR(pSmi, VprIndexTable_501[i], VprTable_501[i]); 
	}
#if 0
	if (SMI_MSOC == pSmi->Chipset) {
		/* Restore cursor color which maybe changed by kernel driver */
		WRITE_DCR(pSmi, DCRF8,  0xFFFFFFFF);
		WRITE_DCR(pSmi, DCRFC,  0x0);
		WRITE_DCR(pSmi, DCR238,  0xFFFFFFFF);
		WRITE_DCR(pSmi, DCR23C,  0x0);
	}
#endif

	return;
}
static void SMI_RestoreReg_750(ScrnInfoPtr pScrn )
{
	int i;
	vgaHWPtr hwp = VGAHWPTR (pScrn);
	SMIPtr pSmi = SMIPTR (pScrn);
	int vgaIOBase = hwp->IOBase;
	int vgaCRIndex = vgaIOBase + VGA_CRTC_INDEX_OFFSET;
	int vgaCRData = vgaIOBase + VGA_CRTC_DATA_OFFSET;
#if 0	
	/*restore seq registers*/
	for(i = 0; i < (sizeof(SeqTable) / sizeof(unsigned char)); i++)
	{
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, i, SeqTable[i]);	
	}

	//	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "RestoreReg, SR18 is 0x%x\n", SeqTable[18]);
	/*restore crt registers*/
	for(i = 0; i < (sizeof(CrtTable) / sizeof(unsigned char)); i++)
	{	
		VGAOUT8_INDEX (pSmi, vgaCRIndex, vgaCRData, i, CrtTable[i]);	
	}

#endif
	/*restore dpr registers*/
	for(i = 0; i < (sizeof(DprTable) / sizeof(unsigned int)); i++)
	{
		WRITE_DPR(pSmi, DprIndexTable[i], DprTable[i]); 
	}
	for(i = 0; i < (sizeof(VprTable_750) / sizeof(unsigned int)); i++)
	{
		WRITE_VPR(pSmi, VprIndexTable_750[i], VprTable_750[i]); 
	}
#if 1
	for(i = 0; i < (sizeof(DcrTable) / sizeof(unsigned int)); i++)
	{
		WRITE_DCR(pSmi, DcrIndexTable[i], DcrTable[i]);
	}
#endif
#if 0
	if (SMI_MSOCE == pSmi->Chipset) {
		/* Restore cursor color which maybe changed by kernel driver */
		WRITE_DCR(pSmi, DCRF8,  0xFFFFFFFF);
		WRITE_DCR(pSmi, DCRFC,  0x0);
		WRITE_DCR(pSmi, DCR238,  0xFFFFFFFF);
		WRITE_DCR(pSmi, DCR23C,  0x0);
	}
#endif

	return;
}
static void SMI_RestoreReg_Con(ScrnInfoPtr pScrn )
{

	int i;
	vgaHWPtr hwp = VGAHWPTR (pScrn);
	SMIPtr pSmi = SMIPTR (pScrn);
	//       int vgaIOBase = hwp->IOBase;
	//      int vgaCRIndex = vgaIOBase + VGA_CRTC_INDEX_OFFSET;
	//      int vgaCRData = vgaIOBase + VGA_CRTC_DATA_OFFSET;


	for(i = 0; i < (sizeof(DprTable_Con) / sizeof(unsigned int)); i++)
	{
		WRITE_DPR(pSmi, DprIndexTable_Con[i], DprTable_Con[i]);
	}
	for(i = 0; i < (sizeof(VprTable_Con) / sizeof(unsigned int)); i++)
	{
		WRITE_VPR(pSmi, VprIndexTable_Con[i], VprTable_Con[i]);
	}
#if 1
	for(i = 0; i < (sizeof(DcrTable_Con) / sizeof(unsigned int)); i++)
	{
		WRITE_DCR(pSmi, DcrIndexTable_Con[i], DcrTable_Con[i]);
	}
#endif

	return;

}


	static Bool
SMI_ModeInit (ScrnInfoPtr pScrn, DisplayModePtr mode)
{
	vgaHWPtr hwp = VGAHWPTR (pScrn);
	SMIPtr pSmi = SMIPTR (pScrn);
	unsigned char tmp;
	int panelIndex, modeIndex, i;
	int xyAddress[] = { 320, 400, 512, 640, 800, 1024, 1280, 1600, 2048 };
	CARD32 DEDataFormat = 0;

	ENTER();

	/* Store values to current mode register structs */
	SMIRegPtr new = &pSmi->ModeReg;
	vgaRegPtr vganew = &hwp->ModeReg;


	if ((SMI_MSOC == pSmi->Chipset) || (SMI_MSOCE == pSmi->Chipset))
	{
		pSmi->Bpp = pScrn->bitsPerPixel / 8;


		if (pSmi->rotate && pSmi->rotate != SMI_ROTATE_UD)
		{
			pSmi->width = pScrn->virtualY;
			pSmi->height = pScrn->virtualX;
			pSmi->Stride = (pSmi->height * pSmi->Bpp + 15) & ~15;
		}
		else
		{
			pSmi->width = pScrn->virtualX;
			pSmi->height = pScrn->virtualY;
			pSmi->Stride = (pSmi->width * pSmi->Bpp + 15) & ~15;
		}



		/* Set mode on Voyager */
		// xf86DrvMsg("", X_INFO, "Belcon: ModeInit(), nHertz is %f\n", mode->VRefresh);
		// xf86DrvMsg("", X_INFO, "Belcon: ModeInit(), current mode, nHertz is %f\n", pScrn->currentMode->VRefresh);
		if(pSmi -> Chipset == SMI_MSOCE)
		{
			SMI_MSOCSetMode_750 (pScrn, mode);
			xf86DrvMsg("", X_INFO, "Kevin:750_ModeInit()");
		}
		else
		{ 
			SMI_MSOCSetMode_501 (pScrn, mode);
			xf86DrvMsg("", X_INFO, "Kevin: 502_ModeInit(");
		}
		// xf86DrvMsg("", X_INFO, "Belcon: Before SMI_AdjustFrame(), frameX0 is %d, frameY0 is %d\n, frameX1 is %d, frameY1 is %d", pScrn->frameX0, pScrn->frameY0, pScrn->frameX1, pScrn->frameY1);
		/* Adjust the viewport */
		SMI_AdjustFrame (pScrn->scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);
		// xf86DrvMsg("", X_INFO, "Belcon: After SMI_AdjustFrame(), frameX0 is %d, frameY0 is %d\n, frameX1 is %d, frameY1 is %d", pScrn->frameX0, pScrn->frameY0, pScrn->frameX1, pScrn->frameY1);
#if 0        
		xf86DrvMsgVerb (pScrn->scrnIndex, X_INFO, VERBLEV,
				"Done restoring mode.  David Register dump Line:%d:\n", __LINE__);
		SMI_PrintRegs (pScrn);
#endif		

		LEAVE (TRUE);
	}

	if (!vgaHWInit (pScrn, mode))
	{
		LEAVE (FALSE);
	}  

	new->modeInit = TRUE;

	if (pSmi->rotate && pSmi->rotate != SMI_ROTATE_UD)
	{
		pSmi->width = pScrn->virtualY;
		pSmi->height = pScrn->virtualX;
	}
	else
	{
		pSmi->width = pScrn->virtualX;
		pSmi->height = pScrn->virtualY;
	}


	pSmi->Bpp = pScrn->bitsPerPixel / 8;

	outb (pSmi->PIOBase + VGA_SEQ_INDEX, 0x17);
	tmp = inb (pSmi->PIOBase + VGA_SEQ_DATA);

	if (pSmi->pci_burst)
	{
		new->SR17 = tmp | 0x20;
	}
	else
	{
		new->SR17 = tmp & ~0x20;
	}

	outb (pSmi->PIOBase + VGA_SEQ_INDEX, 0x18);
	new->SR18 = inb (pSmi->PIOBase + VGA_SEQ_DATA) | 0x11;
	// xf86DrvMsg(pScrn->scrnIndex, X_INFO, "SMI_ModeInit(), new->SR18 is 0x%x\n", new->SR18);

	outb (pSmi->PIOBase + VGA_SEQ_INDEX, 0x21);
	new->SR21 = inb (pSmi->PIOBase + VGA_SEQ_DATA) & ~0x03;

	if (pSmi->Chipset != SMI_COUGAR3DR)
	{
		outb (pSmi->PIOBase + VGA_SEQ_INDEX, 0x31);
		new->SR31 = inb (pSmi->PIOBase + VGA_SEQ_DATA) & ~0xC0;

		outb (pSmi->PIOBase + VGA_SEQ_INDEX, 0x32);
		new->SR32 = inb (pSmi->PIOBase + VGA_SEQ_DATA) & ~0x07;

		if (SMI_LYNXM_SERIES (pSmi->Chipset))
		{
			new->SR32 |= 0x04;
		}
	}

	new->SRA0 = new->CR33 = new->CR3A = 0x00;

	xf86DrvMsg (pScrn->scrnIndex, X_ERROR, "lcdWidth = %d HDisplay = %d\n",
			pSmi->lcdWidth, mode->HDisplay);

	/*boyod*/

	if (pSmi->lcdWidth == 640)
	{
		panelIndex = 0;
	}
	else if (pSmi->lcdWidth == 800)
	{
		panelIndex = 1;
	}
	else if (pSmi->lcdWidth == 1024)
	{
		panelIndex = 2;
	}
	else
	{
		panelIndex = 3;
	}



	if (mode->HDisplay == 640)
	{
		modeIndex = 0;
	}
	else if (mode->HDisplay == 800)
	{
		if (mode->VDisplay == 480)
			modeIndex = 1;
		else
			modeIndex = 2;
	}
	else if (mode->HDisplay == 1024)
	{
		if (mode->VDisplay == 600)
			modeIndex = 3;
		else
			modeIndex = 4;
	}
	else
	{
		modeIndex = 5;
	}

	xf86DrvMsg (pScrn->scrnIndex, X_INFO, "HDisplay %d, VDisplay %d\n",
			mode->HDisplay, mode->VDisplay);
	xf86DrvMsg (pScrn->scrnIndex, X_INFO, "panelIndex = %d modeIndex = %d\n",
			panelIndex, modeIndex);
	// #ifdef LCD_SIZE_DETECT
	if (SMI_LYNXM_SERIES (pSmi->Chipset))
#ifdef LCD_SIZE_DETECT
	{
		static unsigned char PanelTable[4][16] = {
			/*              640x480         */
			{0x5F, 0x4F, 0x00, 0x52, 0x1E, 0x0B, 0xDF, 0x00, 0xE9, 0x0B, 0x2E,
				0x00, 0x4F, 0xDF, 0x07, 0x82},
			/*              800x600                */
			{0x7F, 0x63, 0x00, 0x69, 0x19, 0x72, 0x57, 0x00, 0x58, 0x0C, 0xA2,
				0x20, 0x4F, 0xDF, 0x1C, 0x85},
			/*              1024x768                */
			{0xA3, 0x7F, 0x00, 0x83, 0x14, 0x24, 0xFF, 0x00, 0x02, 0x08, 0xA7,
				0xE0, 0x4F, 0xDF, 0x52, 0x89},
			/*              other           */
			{0xCE, 0x9F, 0x00, 0xA7, 0x15, 0x28, 0xFF, 0x28, 0x00, 0xA3, 0x5A,
				0x20, 0x9F, 0xFF, 0x53, 0x0B},
		};
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "LCD_SIZE_DETECT\n");

		for (i = 0; i < 14; i++)
		{
			new->CR40[i] = PanelTable[panelIndex][i];
		}
		new->SR6C = PanelTable[panelIndex][14];
		new->SR6D = PanelTable[panelIndex][15];


		if (panelIndex == 3)
		{
			new->CR90[14] = 0x18;
		}
		else
		{
			new->CR90[14] = 0x00;
		}

		if (mode->VDisplay < pSmi->lcdHeight)
		{
			new->CRA0[6] = (pSmi->lcdHeight - mode->VDisplay) / 8;
		}
		else
		{
			new->CRA0[6] = 0;
		}

		if (mode->HDisplay < pSmi->lcdWidth)
		{
			new->CRA0[7] = (pSmi->lcdWidth - mode->HDisplay) / 16;
		}
		else
		{
			new->CRA0[7] = 0;
		}
	}
	else
#else

	{
#if 0
		static unsigned char PanelTable[4][4][16] = {
			{				/* 640x480 panel */
				{0x5F, 0x4F, 0x00, 0x53, 0x00, 0x0B, 0xDF, 0x00, 0xEA, 0x0C,
					0x2E, 0x00, 0x4F, 0xDF, 0x07, 0x82},
				{0x5F, 0x4F, 0x00, 0x53, 0x00, 0x0B, 0xDF, 0x00, 0xEA, 0x0C,
					0x2E, 0x00, 0x4F, 0xDF, 0x07, 0x82},
				{0x5F, 0x4F, 0x00, 0x53, 0x00, 0x0B, 0xDF, 0x00, 0xEA, 0x0C,
					0x2E, 0x00, 0x4F, 0xDF, 0x07, 0x82},
				{0x5F, 0x4F, 0x00, 0x53, 0x00, 0x0B, 0xDF, 0x00, 0xEA, 0x0C,
					0x2E, 0x00, 0x4F, 0xDF, 0x07, 0x82},
			},
			{				/* 800x600 panel */
				{0x7F, 0x59, 0x19, 0x5E, 0x8E, 0x72, 0x1C, 0x37, 0x1D, 0x00,
					0xA2, 0x20, 0x4F, 0xDF, 0x1c, 0x85},
				{0x7F, 0x63, 0x00, 0x68, 0x18, 0x72, 0x58, 0x00, 0x59, 0x0C,
					0xE0, 0x20, 0x63, 0x57, 0x1c, 0x85},
				{0x7F, 0x63, 0x00, 0x68, 0x18, 0x72, 0x58, 0x00, 0x59, 0x0C,
					0xE0, 0x20, 0x63, 0x57, 0x1c, 0x85},
				{0x7F, 0x63, 0x00, 0x68, 0x18, 0x72, 0x58, 0x00, 0x59, 0x0C,
					0xE0, 0x20, 0x63, 0x57, 0x1c, 0x85},
			},
			{				/* 1024x768 panel */
				{0xA3, 0x67, 0x0F, 0x6D, 0x1D, 0x24, 0x70, 0x95, 0x72, 0x07,
					0xA3, 0x20, 0x4F, 0xDF, 0x52, 0x89},
				{0xA3, 0x71, 0x19, 0x77, 0x07, 0x24, 0xAC, 0xD1, 0xAE, 0x03,
					0xE1, 0x20, 0x63, 0x57, 0x52, 0x89},
				{0xA3, 0x7F, 0x00, 0x85, 0x15, 0x24, 0xFF, 0x00, 0x01, 0x07,
					0xE5, 0x20, 0x7F, 0xFF, 0x52, 0x89},
				{0xA3, 0x7F, 0x00, 0x85, 0x15, 0x24, 0xFF, 0x00, 0x01, 0x07,
					0xE5, 0x20, 0x7F, 0xFF, 0x52, 0x89},
			},
			{                        /* 1280x1024 panel */
				{0xCE, 0x9F, 0x00, 0xA7, 0x15, 0x28, 0xFF, 0x28, 0x00, 0x03,
					0x4A, 0x20, 0x9F, 0xFF, 0x1E, 0x82},
				{0xCE, 0x9F, 0x00, 0xA7, 0x15, 0x28, 0xFF, 0x28, 0x00, 0x03,
					0x4A, 0x20, 0x9F, 0xFF, 0x1E, 0x82},
				{0xCE, 0x9F, 0x00, 0xA7, 0x15, 0x28, 0xFF, 0x28, 0x00, 0x03,
					0x4A, 0x20, 0x9F, 0xFF, 0x1E, 0x82},
				{0xCE, 0x9F, 0x00, 0xA7, 0x15, 0x28, 0xFF, 0x28, 0x00, 0x03,
					0x4A, 0x20, 0x9F, 0xFF, 0x1E, 0x82},
			}

		};

		for (i = 0; i < 14; i++)
		{
			new->CR40[i] = PanelTable[panelIndex][modeIndex][i];
		}
		new->SR6C = PanelTable[panelIndex][modeIndex][14];
		new->SR6D = PanelTable[panelIndex][modeIndex][15];
		/*
		 * FIXME::
		 * Temparily hardcoded here
		 * Refer to WriteMode() function
		 */
		if (panelIndex == 3)
		{
			new->CR90[14] = 0x18;
		}
		else
		{
			new->CR90[14] = 0x00;
		}

		/* Added by Belcon */
		if (modeIndex < 3) {
			new->CR30 &= ~0x09;
		} else {
			new->CR30 |= 0x09;
		}

		if (mode->VDisplay < pSmi->lcdHeight)
		{
			new->CRA0[6] = (pSmi->lcdHeight - mode->VDisplay) / 8;
		}
		else
		{
			new->CRA0[6] = 0;
		}

		if (mode->HDisplay < pSmi->lcdWidth)
		{
			new->CRA0[7] = (pSmi->lcdWidth - mode->HDisplay) / 16;
		}
		else
		{
			new->CRA0[7] = 0;
		}
#endif
		/*
		 * Belcon Added
		 */
		/*
		   if (pSmi->lcdWidth == 0) {
		 */
		static unsigned char ModeTable[6][3][16] = {
			/* 640x480 */
			{
				/* 60 Hz */
				{
					0x5F, 0x4F, 0x00, 0x54, 0x00, 0x0B, 0xDF, 0x00,
					0xEA, 0x0C, 0x2E, 0x00, 0x4F, 0xDF, 0x07, 0x82,
				},
				/* 75 Hz */
				{
					0x64, 0x4F, 0x00, 0x52, 0x1A, 0xF2, 0xDF, 0x00,
					0xE0, 0x03, 0x0F, 0xC0, 0x4F, 0xDF, 0x16, 0x85,
				},
				/* 85 Hz */
				{
					0x63, 0x4F, 0x00, 0x57, 0x1E, 0xFB, 0xDF, 0x00,
					0xE0, 0x03, 0x0F, 0xC0, 0x4F, 0xDF, 0x88, 0x9B,
				},
			},

			/* 800x480 */
			{
				/* 60 Hz */
				{
					0x6B, 0x63, 0x00, 0x69, 0x1B, 0xF2, 0xDF, 0x00,
					0xE2, 0xE4, 0x1F, 0xC0, 0x63, 0xDF, 0x2C, 0x17,
				},
				/* 75 Hz */
				{
					0x6B, 0x63, 0x00, 0x69, 0x1B, 0xF2, 0xDF, 0x00,
					0xE2, 0xE4, 0x1F, 0xC0, 0x63, 0xDF, 0x2C, 0x17,
				},
				/* 85 Hz */
				{
					0x6B, 0x63, 0x00, 0x69, 0x1B, 0xF2, 0xDF, 0x00,
					0xE2, 0xE4, 0x1F, 0xC0, 0x63, 0xDF, 0x2C, 0x17,
				},
			},

			/* 800x600 */
			{
				/* 60 Hz */
				{
					0x7F, 0x63, 0x00, 0x69, 0x18, 0x72, 0x57, 0x00,
					0x58, 0x0C, 0xE0, 0x20, 0x63, 0x57, 0x1C, 0x85,
				},
				/* 75 Hz */
				{
					0x7F, 0x63, 0x00, 0x66, 0x10, 0x6F, 0x57, 0x00,
					0x58, 0x0B, 0xE0, 0x20, 0x63, 0x57, 0x4C, 0x8B,
				},
				/* 85 Hz */
				{
					0x7E, 0x63, 0x00, 0x68, 0x10, 0x75, 0x57, 0x00,
					0x58, 0x0B, 0xE0, 0x20, 0x63, 0x57, 0x37, 0x87,
				},
			},

			/* 1024x600 */
			{
				/* 60 Hz */
				{
					0xA3, 0x7F, 0x00, 0x82, 0x0B, 0x6F, 0x57, 0x00,
					0x5C, 0x0F, 0xE0, 0xE0, 0x7F, 0x57, 0x16, 0x07,
				},
				/* 60 Hz */
				{
					0xA3, 0x7F, 0x00, 0x82, 0x0B, 0x6F, 0x57, 0x00,
					0x5C, 0x0F, 0xE0, 0xE0, 0x7F, 0x57, 0x16, 0x07,
				},
				/* 60 Hz */
				{
					0xA3, 0x7F, 0x00, 0x82, 0x0B, 0x6F, 0x57, 0x00,
					0x5C, 0x0F, 0xE0, 0xE0, 0x7F, 0x57, 0x16, 0x07,
				},
			},

			/* 1024x768 */
			{
				/* 60 Hz */
				{
					0xA3, 0x7F, 0x00, 0x86, 0x15, 0x24, 0xFF, 0x00,
					0x01, 0x07, 0xE5, 0x20, 0x7F, 0xFF, 0x52, 0x89,
				},
				/* 75 Hz */
				{
					0x9F, 0x7F, 0x00, 0x82, 0x0E, 0x1E, 0xFF, 0x00,
					0x00, 0x03, 0xE5, 0x20, 0x7F, 0xFF, 0x0B, 0x02,
				},
				/* 85 Hz */
				{
					0xA7, 0x7F, 0x00, 0x86, 0x12, 0x26, 0xFF, 0x00,
					0x00, 0x03, 0xE5, 0x20, 0x7F, 0xFF, 0x70, 0x11,
				},
			},

			/* 1280x1024 */
			{
				/* 60 Hz */
				{
					0xCE, 0x9F, 0x00, 0xA7, 0x15, 0x28, 0xFF, 0x00,
					0x00, 0x03, 0x4A, 0x20, 0x9F, 0xFF, 0x53, 0x0B,
				},
				/* 75 Hz */
				{
					0xCE, 0x9F, 0x00, 0xA2, 0x14, 0x28, 0xFF, 0x00,
					//           0x00, 0x03, 0x4A, 0x20, 0x9F, 0xFF, 0x13, 0x02,
					0x00, 0x03, 0x4A, 0x20, 0x9F, 0xFF, 0x42, 0x07,           
				},
				/* 85 Hz */
				{
					0xD3, 0x9F, 0x00, 0xA8, 0x1C, 0x2E, 0xFF, 0x00,
					//           0x00, 0x03, 0x4A, 0x20, 0x9F, 0xFF, 0x16, 0x42,
					0x00, 0x03, 0x4A, 0x20, 0x9F, 0xFF, 0x0b, 0x01,           
				},
			},
		};
		int	refresh_rate_index = -1;
		/*
		 * SR50 ~ SR57
		 *
		 * SR50: LCD Overflow Register 1 for Virtual Refresh
		 * SR51: LCD Overflow Register 2 for Virtual Refresh
		 * SR52: LCD Horizontal Total for Virtual Refresh
		 * SR53: LCD Horizontal Display Enable for Virtual Refresh
		 * SR54: LCD Horizontal Sync Start for Virtual Refresh
		 * SR55: LCD Vertical Total for Virtual Refresh
		 * SR56: LCD Vertical Display Enable for Virtual Refresh
		 * SR57: LCD Vertical Sync Start for Virtual Refresh
		 * SR5A: SYNC Pulse-widths Adjustment
		 */
		static unsigned char ModeTable_2[6][3][16] = {
			/* 640x480 */
			{
				/* 60 Hz */
				{
					0x04, 0x24, 0x63, 0x4F, 0x52, 0x0C, 0xDF, 0xE9,
					0x00, 0x03, 0x59, 0x00, 0x4F, 0xDF, 0x03, 0x02,
				},
				/* 75 Hz */
				{
					0x04, 0x24, 0x63, 0x4F, 0x52, 0x0C, 0xDF, 0xE9,
					0x00, 0x03, 0x59, 0xC0, 0x4F, 0xDF, 0x16, 0x85,
				},
				/* 85 Hz */
				{
					0x04, 0x24, 0x63, 0x4F, 0x52, 0x0C, 0xDF, 0xE9,
					0x00, 0x03, 0x59, 0xC0, 0x4F, 0xDF, 0x88, 0x9B,
				},
			},

			/* 800x480 */
			{
				/* 60 Hz */
				{
					0x02, 0x24, 0x7B, 0x63, 0x67, 0xF3, 0xDF, 0xE2,
					0x00, 0x03, 0x41, 0xC0, 0x63, 0xDF, 0x2C, 0x17,
				},
				/* 75 Hz */
				{
					0x6B, 0x63, 0x00, 0x69, 0x1B, 0xF2, 0xDF, 0x00,
					0xE2, 0xE4, 0x1F, 0xC0, 0x63, 0xDF, 0x2C, 0x17,
				},
				/* 85 Hz */
				{
					0x6B, 0x63, 0x00, 0x69, 0x1B, 0xF2, 0xDF, 0x00,
					0xE2, 0xE4, 0x1F, 0xC0, 0x63, 0xDF, 0x2C, 0x17,
				},
			},

			/* 800x600 */
			{
				/* 60 Hz */
				{
					0x04, 0x48, 0x83, 0x63, 0x69, 0x73, 0x57, 0x58,
					0x00, 0x03, 0x7B, 0x20, 0x63, 0x57, 0x0E, 0x05,
				},
				/* 75 Hz */
				{
					0x7F, 0x63, 0x00, 0x66, 0x10, 0x6F, 0x57, 0x00,
					0x58, 0x0B, 0xE0, 0x20, 0x63, 0x57, 0x4C, 0x8B,
				},
				/* 85 Hz */
				{
					0x7E, 0x63, 0x00, 0x68, 0x10, 0x75, 0x57, 0x00,
					0x58, 0x0B, 0xE0, 0x20, 0x63, 0x57, 0x37, 0x87,
				},
			},

			/* 1024x600 */
			{
				/* 60 Hz */
				{
					0x04, 0x48, 0x95, 0x7F, 0x86, 0x70, 0x57, 0x5B,
					0x00, 0x60, 0x1c, 0x22, 0x7F, 0x57, 0x16, 0x07,
				},
				/* 60 Hz */
				{
					0x04, 0x48, 0x95, 0x7F, 0x86, 0x70, 0x57, 0x5B,
					0x00, 0x60, 0x1c, 0x22, 0x7F, 0x57, 0x16, 0x07,
				},
				/* 60 Hz */
				{
					0x04, 0x48, 0x95, 0x7F, 0x86, 0x70, 0x57, 0x5B,
					0x00, 0x60, 0x1c, 0x22, 0x7F, 0x57, 0x16, 0x07,
				},
			},

			/* 1024x768 */
			{
				/* 60 Hz */
				{
					0x06, 0x68, 0xA7, 0x7F, 0x83, 0x25, 0xFF, 0x02,
					0x00, 0x62, 0x85, 0x20, 0x7F, 0xFF, 0x29, 0x09,
				},
				/* 75 Hz */
				{
					0x9F, 0x7F, 0x00, 0x82, 0x0E, 0x1E, 0xFF, 0x00,
					0x00, 0x03, 0xE5, 0x20, 0x7F, 0xFF, 0x0B, 0x02,
				},
				/* 85 Hz */
				{
					0xA7, 0x7F, 0x00, 0x86, 0x12, 0x26, 0xFF, 0x00,
					0x00, 0x03, 0xE5, 0x20, 0x7F, 0xFF, 0x70, 0x11,
				},
			},

			/* 1280x1024 */
			{
				/* 60 Hz */
				{
					0x08, 0x8C, 0xD5, 0x9F, 0xAB, 0x26, 0xFF, 0x00,
					0x00, 0x03, 0x7E, 0x20, 0x9F, 0xFF, 0x53, 0x0B,
				},
				/* 75 Hz */
				{
					0xCE, 0x9F, 0x00, 0xA2, 0x14, 0x28, 0xFF, 0x00,
					0x00, 0x03, 0x4A, 0x20, 0x9F, 0xFF, 0x42, 0x07,           
				},
				/* 85 Hz */
				{
					0xD3, 0x9F, 0x00, 0xA8, 0x1C, 0x2E, 0xFF, 0x00,
					0x00, 0x03, 0x4A, 0x20, 0x9F, 0xFF, 0x0b, 0x01,           
				},
			},
		};

		if (abs(mode->VRefresh - 60.0) < 5) {
			refresh_rate_index = 0;
		} else if (abs(mode->VRefresh - 75.0) < 5 ){
			refresh_rate_index = 1;
		} else if (abs(mode->VRefresh - 85.0) < 5) {
			refresh_rate_index = 2;
		} else {
			xf86DrvMsg (pScrn->scrnIndex, X_ERROR, "Refresh Rate %fHz is not supported\n", mode->VRefresh);
			return (FALSE);
		}
		xf86DrvMsg (pScrn->scrnIndex, X_ERROR, "Refresh Rate %fHz , index is %d\n", mode->VRefresh, refresh_rate_index);
		for (i = 0; i < 14; i++) {
			/*
			   new->CR40[i] = ModeTable[refresh_rate_index][modeIndex][i];
			 */
			new->CR40[i] = ModeTable[modeIndex][refresh_rate_index][i];
			xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ModeTable[%d][%d][%d] is 0x%x\n", modeIndex, refresh_rate_index, i, new->CR40[i]);
		}

		/* Belcon : clone mode */
		if (pSmi->clone_mode) {
			for (i = 0; i < 14; i++) {
				new->FPR50[i] = ModeTable_2[modeIndex][refresh_rate_index][i];
				xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ModeTable_2[%d][%d][%d] is 0x%x\n", modeIndex, refresh_rate_index, i, new->FPR50[i]);
			}
			new->SR6E = ModeTable_2[modeIndex][refresh_rate_index][14];
			new->SR6F = ModeTable_2[modeIndex][refresh_rate_index][15];
		}

		new->SR6C = ModeTable[modeIndex][refresh_rate_index][14];
		new->SR6D = ModeTable[modeIndex][refresh_rate_index][15];
		xf86DrvMsg (pScrn->scrnIndex, X_ERROR, "SR6C is 0x%x, SR6D is 0x%x\n", new->SR6C, new->SR6D);
		if (panelIndex == 3) {
			new->CR90[14] = 0x18;
		} else {
			new->CR90[14] = 0x00;
		}
		if (modeIndex < 5) {
			new->CR30 &= ~0x09;
		} else {
			new->CR30 |= 0x09;
		}

		if (mode->VDisplay < pSmi->lcdHeight) {
			new->CRA0[6] = (pSmi->lcdHeight - mode->VDisplay) / 8;
		} else {
			new->CRA0[6] = 0;
		}
		if (mode->HDisplay < pSmi->lcdWidth) {
			new->CRA0[7] = (pSmi->lcdWidth - mode->HDisplay) / 16;
		} else {
			new->CRA0[7] = 0;
		}
		/*
		   }
		 */

	}
#endif

	/* CZ 2.11.2001: for gamma correction (TODO: other chipsets?) */
	new->CCR66 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x66);
	if ((pSmi->Chipset == SMI_LYNX3DM) || (pSmi->Chipset == SMI_COUGAR3DR))
	{
		switch (pScrn->bitsPerPixel)
		{
			case 8:
				new->CCR66 = (new->CCR66 & 0xF3) | 0x00;	/* 6 bits-RAM */
				break;
			case 16:
				new->CCR66 = (new->CCR66 & 0xF3) | 0x00;	/* 6 bits-RAM */
				/* no Gamma correction in 16 Bit mode (s. Release.txt 1.3.1) */
				break;
			case 24:
			case 32:
				new->CCR66 = (new->CCR66 & 0xF3) | 0x04;	/* Gamma correct ON */
				break;
			default:
				return (FALSE);
		}
	}

	if (pSmi->Chipset != SMI_COUGAR3DR)
	{
		outb (pSmi->PIOBase + VGA_SEQ_INDEX, 0x30);
		if (inb (pSmi->PIOBase + VGA_SEQ_DATA) & 0x01)
		{
			new->SR21 = 0x00;
		}
	}

	if (pSmi->MCLK > 0)
	{
		SMI_CommonCalcClock (pScrn->scrnIndex, pSmi->MCLK,
				1, 1, 31, 0, 2, pSmi->minClock,
				pSmi->maxClock, &new->SR6A, &new->SR6B);
	}
	else
	{
		new->SR6B = 0xFF;
	}

	if ((mode->HDisplay == 640) && SMI_LYNXM_SERIES (pSmi->Chipset))
	{
		vganew->MiscOutReg &= ~0x0C;
	}
	else
	{
		vganew->MiscOutReg |= 0x0C;
	}
	vganew->MiscOutReg |= 0xE0;
	if (mode->HDisplay == 800)
	{
		vganew->MiscOutReg &= ~0xC0;
	}

	if ((mode->HDisplay == 1024) && SMI_LYNXM_SERIES (pSmi->Chipset))
	{
		vganew->MiscOutReg &= ~0xC0;
	}

	/* Set DPR registers */
	pSmi->Stride = (pSmi->lcdWidth * pSmi->Bpp + 15) & ~15;
	switch (pScrn->bitsPerPixel)
	{
		case 8:
			DEDataFormat = 0x00000000;
			break;

		case 16:
			pSmi->Stride >>= 1;
			DEDataFormat = 0x00100000;
			break;

		case 24:
			DEDataFormat = 0x00300000;
			break;

		case 32:
			pSmi->Stride >>= 2;
			DEDataFormat = 0x00200000;
			break;
	}
	for (i = 0; i < sizeof (xyAddress) / sizeof (xyAddress[0]); i++)
	{
		if (pSmi->rotate && pSmi->rotate != SMI_ROTATE_UD)
		{
			if (xyAddress[i] == pSmi->lcdHeight)
			{
				DEDataFormat |= i << 16;
				break;
			}
		}
		else
		{
			if (xyAddress[i] == pSmi->lcdWidth)
			{
				DEDataFormat |= i << 16;
				break;
			}
		}
	}
	new->DPR10 = (pSmi->Stride << 16) | pSmi->Stride;
	new->DPR1C = DEDataFormat;
	new->DPR20 = 0;
	new->DPR24 = 0xFFFFFFFF;
	new->DPR28 = 0xFFFFFFFF;
	new->DPR2C = 0;
	new->DPR30 = 0;
	new->DPR3C = (pSmi->Stride << 16) | pSmi->Stride;
	new->DPR40 = 0;
	new->DPR44 = 0;

	/* Set VPR registers (and FPR registers for SM731) */
	switch (pScrn->bitsPerPixel)
	{
		case 8:
			new->VPR00 = 0x00000000;
			new->FPR00_ = 0x00080000;
			break;

		case 16:
			new->VPR00 = 0x00020000;
			new->FPR00_ = 0x000A0000;
			break;

		case 24:
			new->VPR00 = 0x00040000;
			new->FPR00_ = 0x000C0000;
			break;

		case 32:
			new->VPR00 = 0x00030000;
			new->FPR00_ = 0x000B0000;
			break;
	}
	new->VPR0C = pSmi->FBOffset >> 3;
	if (pSmi->rotate && pSmi->rotate != SMI_ROTATE_UD)
	{
		new->VPR10 = ((((pSmi->lcdHeight * pSmi->Bpp) >> 3)
					+ 2) << 16) | ((pSmi->lcdHeight * pSmi->Bpp) >> 3);
	}
	else
	{
		new->VPR10 = ((((pSmi->lcdWidth * pSmi->Bpp) >> 3)
					+ 2) << 16) | ((pSmi->lcdWidth * pSmi->Bpp) >> 3);
	}

	new->FPR0C_ = new->VPR0C;
	new->FPR10_ = new->VPR10;

	/* Set CPR registers */
	new->CPR00 = 0x00000000;

	pScrn->vtSema = TRUE;

	/* Find the INT 10 mode number */
	{
		static struct
		{
			int x, y, bpp;
			CARD16 mode;

		} modeTable[] =
		{
			{
				640, 480, 8, 0x50},
			{
				640, 480, 16, 0x52},
			{
				640, 480, 24, 0x53},
			{
				640, 480, 32, 0x54},
			{
				800, 480, 8, 0x4A},
			{
				800, 480, 16, 0x4C},
			{
				800, 480, 24, 0x4D},
			{
				800, 600, 8, 0x55},
			{
				800, 600, 16, 0x57},
			{
				800, 600, 24, 0x58},
			{
				800, 600, 32, 0x59},
			{
				1024, 768, 8, 0x60},
			{
				1024, 768, 16, 0x62},
			{
				1024, 768, 24, 0x63},
			{
				1024, 768, 32, 0x64},
			{
				1280, 1024, 8, 0x65},
			{
				1280, 1024, 16, 0x67},
			{
				1280, 1024, 24, 0x68},
			{
				1280, 1024, 32, 0x69},};

				new->mode = 0;
				// xf86DrvMsg("", X_INFO, "Belcon: test, %dx%dx%d\n", mode->HDisplay, mode->VDisplay, pScrn->bitsPerPixel);
				for (i = 0; i < sizeof (modeTable) / sizeof (modeTable[0]); i++)
				{
					if ((modeTable[i].x == mode->HDisplay)
							&& (modeTable[i].y == mode->VDisplay)
							&& (modeTable[i].bpp == pScrn->bitsPerPixel))
					{
						new->mode = modeTable[i].mode;
						break;
					}
				}
	}

	/* Zero the font memory */
	memset (new->smiFont, 0, sizeof (new->smiFont));

	/* Write the mode registers to hardware */
	if (pSmi->useBIOS)	// Use bios call -- by mill.chen
	{
		SMI_BiosWriteMode(pScrn, mode, new);
	}
	else
		SMI_WriteMode (pScrn, vganew, new);

	/* Adjust the viewport */
	SMI_AdjustFrame (pScrn->scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);

#if 1
	xf86DrvMsgVerb (pScrn->scrnIndex, X_INFO, VERBLEV,
			"Done restoring mode.  David Register dump Line:%d:\n", __LINE__);
	SMI_PrintRegs(pScrn);
#endif
	LEAVE (TRUE);
}

/*
 * This is called at the end of each server generation.  It restores the
 * original (text) mode.  It should also unmap the video memory, and free any
 * per-generation data allocated by the driver.  It should finish by unwrapping
 * and calling the saved CloseScreen function.
 */

	static Bool
SMI_CloseScreen (int scrnIndex, ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	vgaHWPtr hwp = VGAHWPTR (pScrn);
	SMIPtr pSmi = SMIPTR (pScrn);
	vgaRegPtr vgaSavePtr = &hwp->SavedReg;
	SMIRegPtr SMISavePtr = &pSmi->SavedReg;

	SMIRegPtr pRegPtr = (SMIRegPtr)pSmi->pEnt_private->ptr;
	Bool ret;

	ENTER();

#if 0
	/*
	 * Belcon: Fix #001
	 * 	Restore hardware cursor register
	 */
	if (pSmi->dcrF0)
		WRITE_DCR(pSmi, DCRF0, pSmi->dcrF0);
	if (pSmi->dcr230)
		WRITE_DCR(pSmi, DCR230, pSmi->dcr230);
	/* #001 ended */

#endif
	if (pScrn->vtSema)
	{
		xf86DrvMsg (pScrn->scrnIndex, X_INFO, "DEBUG:%s %d\n", __FUNCTION__, __LINE__);
		/*
		   SMI_WriteMode (pScrn, vgaSavePtr, SMISavePtr);
		   vgaHWLock (hwp);
		   SMI_UnmapMem (pScrn);
		 */
		SMI_LeaveVT(scrnIndex, 0);/* Teddy:Restore console mode and unmap framebuffer */
	}

    if (pSmi->AccelInfoRec != NULL)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        XAADestroyInfoRec (pSmi->AccelInfoRec);
    }
    if (pSmi->CursorInfoRec != NULL)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        xf86DestroyCursorInfoRec (pSmi->CursorInfoRec);
    }
    if (pSmi->DGAModes != NULL)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        xfree (pSmi->DGAModes);
    }
    if (pSmi->pInt10 != NULL)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        xf86FreeInt10 (pSmi->pInt10);
        pSmi->pInt10 = NULL;
    }
    if (pSmi->ptrAdaptor != NULL)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        xfree (pSmi->ptrAdaptor);
    }
    if (pSmi->BlockHandler != NULL)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        pScreen->BlockHandler = pSmi->BlockHandler;
    }
    if (pSmi->I2C != NULL)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        xf86DestroyI2CBusRec (pSmi->I2C, FALSE, TRUE);
        xfree (pSmi->I2C);
        pSmi->I2C = NULL;
    }
    if (pSmi->I2C_secondary != NULL)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        xf86DestroyI2CBusRec (pSmi->I2C_secondary, FALSE, TRUE);
        xfree (pSmi->I2C_secondary);
        pSmi->I2C_secondary = NULL;
    }
    if (pSmi->ediddata!= NULL)
    {
        xf86DrvMsg ("", X_INFO, "free edidbuffer line %d\n", __LINE__);
        //xf86DestroyI2CBusRec (pSmi->I2C_secondary, FALSE, TRUE);
        xfree (pSmi->ediddata);
        pSmi->ediddata = NULL;
    }	
    /* #670 */
    if (pSmi->pSaveBuffer)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        xfree (pSmi->pSaveBuffer);
    }
    /* #920 */
    if (pSmi->paletteBuffer)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        xfree (pSmi->paletteBuffer);
    }

	/*	remember that primary pScrn always the last one get closed !! 
		so don't release SHARED resource while not the PRIMARY pScrn (pSmi)
		by Monk 09/16/2010
	 */

	//	if(pSmi->save750)
	if(pSmi->IsPrimary && pRegPtr->save750)
	{
		//xfree(pSmi->save750);
		xfree(pRegPtr->save750);
		pRegPtr->save750 = NULL;
	}

	//	if(pSmi->fonts)
	if(pSmi->IsPrimary &&pRegPtr->fonts)
	{
		//xfree(pSmi->fonts);
		xfree(pRegPtr->fonts);
		pRegPtr->fonts = NULL;
	}

	pScrn->vtSema = FALSE;
	pScreen->CloseScreen = pSmi->CloseScreen;
	ret = (*pScreen->CloseScreen) (scrnIndex, pScreen);

	LEAVE(ret);
}

	static void
SMI_FreeScreen (int scrnIndex, int flags)
{
	SMI_FreeRec (xf86Screens[scrnIndex]);
}

	static Bool
SMI_SaveScreen (ScreenPtr pScreen, int mode)
{
	Bool ret;


	ret = vgaHWSaveScreen (pScreen, mode);
	{
		ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
		SMIPtr pSmi = SMIPTR (pScrn);
	}

	return (ret);
}

	void
SMI_AdjustFrame (int scrnIndex, int x, int y, int flags)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	SMIPtr pSmi = SMIPTR (pScrn);
	CARD32 Base;


	if (pSmi->ShowCache && y)
	{
		y += pScrn->virtualY - 1;
	}

	xf86DrvMsg("", X_INFO, "pSmi->FBOffset is 0x%x, x is %d, y is %d\n", pSmi->FBOffset, x, y);
	// Base = pSmi->FBOffset + (x + y * pScrn->virtualX) * pSmi->Bpp;
	Base = pSmi->FBOffset + (x + y * pScrn->displayWidth) * pSmi->Bpp;

	if (SMI_LYNX3D_SERIES (pSmi->Chipset) || SMI_COUGAR_SERIES (pSmi->Chipset))
	{
		Base = (Base + 15) & ~15;
#if 1				/* PDR#1058 */
		while ((Base % pSmi->Bpp) > 0)
		{
			Base -= 16;
		}
#endif
	}
	else if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
	{
		Base = (Base + 7) & ~7;
#if 1				/* PDR#1058 */
		while ((Base % pSmi->Bpp) > 0)
		{
			Base -= 8;
		}
#endif
	}

	if ((pSmi->Chipset != SMI_MSOC) && (SMI_MSOCE != pSmi->Chipset))
	{
		WRITE_VPR (pSmi, 0x0C, Base >> 3);
	}
	else
	{

		if (!pSmi->IsSecondary)
		{
			// WRITE_DCR (pSmi, DCR0C, 0);
			WRITE_DCR (pSmi, DCR0C, Base);
		}
		else
		{
			/* changed by Belcon */
			//	  Base = pSmi->videoRAMBytes / 16 << 4;
			//  Base = pSmi->FBOffset / 16 << 4;
			// Base = pScrn->fbOffset / 16 << 4;
			WRITE_DCR (pSmi, DCR204, Base);
		}
	}


	if (pSmi->Chipset == SMI_COUGAR3DR)
	{
		WRITE_FPR (pSmi, FPR0C, Base >> 3);
	}

}


	Bool
SMI_SwitchMode (int scrnIndex, DisplayModePtr mode, int flags)
{
	Bool ret;
	SMIPtr pSmi = SMIPTR (xf86Screens[scrnIndex]);


	ret = SMI_ModeInit (xf86Screens[scrnIndex], mode);
	if (!pSmi->NoAccel)
		SMI_EngineReset (xf86Screens[scrnIndex]);
	return (ret);
}

	void
SMI_LoadPalette (ScrnInfoPtr pScrn, int numColors, int *indicies,
		LOCO * colors, VisualPtr pVisual)
{
	SMIPtr pSmi = SMIPTR (pScrn);
	int i;
	int iRGB;




	/* Enable both the CRT and LCD DAC RAM paths, so both palettes are updated */
	if ((pSmi->Chipset == SMI_LYNX3DM) || (pSmi->Chipset == SMI_COUGAR3DR))
	{
		CARD8 ccr66;

		ccr66 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x66);
		ccr66 &= 0x0f;
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x66, ccr66);
	}

	if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
	{
		for (i = 0; i < numColors; i++)
		{
			DEBUG ("pal[%d] = %d %d %d\n", indicies[i],
					colors[indicies[i]].red, colors[indicies[i]].green,
					colors[indicies[i]].blue);
			VGAOUT8 (pSmi, VGA_DAC_WRITE_ADDR, indicies[i]);
			VGAOUT8 (pSmi, VGA_DAC_DATA, colors[indicies[i]].red);
			VGAOUT8 (pSmi, VGA_DAC_DATA, colors[indicies[i]].green);
			VGAOUT8 (pSmi, VGA_DAC_DATA, colors[indicies[i]].blue);
		}
	}
	else				/* For SMI 501 Palette control */
	{
		for (i = 0; i < numColors; i++)
		{
			DEBUG ("pal[%d] = %d %d %d\n", indicies[i],
					colors[indicies[i]].red, colors[indicies[i]].green,
					colors[indicies[i]].blue);

			iRGB = (colors[indicies[i]].red << 16) |
				(colors[indicies[i]].green << 8) | (colors[indicies[i]].blue);

			if (!pSmi->IsSecondary)
			{
				WRITE_DCR (pSmi, DCR400 + (4 * (indicies[i])), iRGB);	/* CRT Palette   */
			}
			else
				WRITE_DCR (pSmi, DCR800 + (4 * (indicies[i])), iRGB);	/* Panel Palette */
		}

	}


}

	static void
SMI_DisableVideo (ScrnInfoPtr pScrn)
{
	SMIPtr pSmi = SMIPTR (pScrn);
	CARD8 tmp;
	int rVal;

	if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
	{
		if (!(tmp = VGAIN8 (pSmi, VGA_DAC_MASK)))
			return;
		pSmi->DACmask = tmp;
		VGAOUT8 (pSmi, VGA_DAC_MASK, 0);
	}
	else
	{
		rVal = READ_DCR (pSmi, DCR200);
		rVal |= DCR200_CRT_BLANK;
		WRITE_DCR (pSmi, DCR200, rVal);
	}
}

	static void
SMI_EnableVideo (ScrnInfoPtr pScrn)
{
	SMIPtr pSmi = SMIPTR (pScrn);
	int rVal;

	if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
	{
		VGAOUT8 (pSmi, VGA_DAC_MASK, pSmi->DACmask);
	}
	else
	{
		rVal = READ_DCR (pSmi, DCR200);
		rVal &= ~DCR200_CRT_BLANK;
		WRITE_DCR (pSmi, DCR200, rVal);
	}
}


	void
SMI_EnableMmio (ScrnInfoPtr pScrn)
{
	vgaHWPtr hwp = VGAHWPTR (pScrn);
	SMIPtr pSmi = SMIPTR (pScrn);
	CARD8 tmp;

	/* below code make sense for sm71x and sm72x */
	if (SMI_MSOC != pSmi->Chipset && SMI_MSOCE != pSmi->Chipset)
	{
		/*
		 * Enable chipset (seen on uninitialized secondary cards) might not be
		 * needed once we use the VGA softbooter
		 */
		vgaHWSetStdFuncs (hwp);

		/* Enable linear mode */
		outb (pSmi->PIOBase + VGA_SEQ_INDEX, 0x18);
		tmp = inb (pSmi->PIOBase + VGA_SEQ_DATA);
		pSmi->SR18Value = tmp;	/* PDR#521 */
		/* Debug */
		if (-1 == saved_console_reg) {
			// xf86DrvMsg(pScrn->scrnIndex, X_INFO, "SMI_EnableMmio(), Set save->SR18 value: 0x%x\n", tmp);
			(pSmi->SavedReg).SR18 = tmp;
			saved_console_reg = 0;
		}
		/*
		   xf86DrvMsg (pScrn->scrnIndex, X_NOTICE,
		   " Enable pSmi->SR18Value:%02X, tmp is %02X\n", pSmi->SR18Value, tmp);
		 */
		outb (pSmi->PIOBase + VGA_SEQ_DATA, tmp | 0x11);
		/* Enable 2D/3D Engine and Video Processor */
		outb (pSmi->PIOBase + VGA_SEQ_INDEX, 0x21);
		tmp = inb (pSmi->PIOBase + VGA_SEQ_DATA);
		pSmi->SR21Value = tmp;	/* PDR#521 */
		xf86DrvMsg (pScrn->scrnIndex, X_NOTICE,
				" Enable pSmi->SR21Value:%02X\n", pSmi->SR21Value);
		outb (pSmi->PIOBase + VGA_SEQ_DATA, tmp & ~0x03);

	}

}

	void
SMI_DisableMmio (ScrnInfoPtr pScrn)
{
	vgaHWPtr hwp = VGAHWPTR (pScrn);
	SMIPtr pSmi = SMIPTR (pScrn);

	/* below code make sense for sm71x and sm72x*/
	if (SMI_MSOC != pSmi->Chipset && SMI_MSOCE != pSmi->Chipset)
	{
		vgaHWSetStdFuncs (hwp);

		/* Disable 2D/3D Engine and Video Processor */
		outb (pSmi->PIOBase + VGA_SEQ_INDEX, 0x21);
		outb (pSmi->PIOBase + VGA_SEQ_DATA, pSmi->SR21Value & (~0x80));	/* PDR#521 */

		/* Disable linear mode */
		outb (pSmi->PIOBase + VGA_SEQ_INDEX, 0x18);
		//      outb (pSmi->PIOBase + VGA_SEQ_DATA, pSmi->SR18Value);	/* PDR#521 */
		outb (pSmi->PIOBase + VGA_SEQ_DATA, (pSmi->SavedReg).SR18);	/* PDR#521 */
		/*
		   xf86DrvMsg (pScrn->scrnIndex, X_NOTICE,
		   " Disable pSmi->SR18Value:%02X %02X\n", pSmi->SR18Value, (pSmi->SavedReg).SR18);
		 */
	}

}

/* This function is used to debug, it prints out the contents of Lynx regs */
	static void
SMI_PrintRegs (ScrnInfoPtr pScrn)
{
	unsigned char i, tmp;
	unsigned int j;
	vgaHWPtr hwp = VGAHWPTR (pScrn);
	SMIPtr pSmi = SMIPTR (pScrn);
	int vgaCRIndex = hwp->IOBase + VGA_CRTC_INDEX_OFFSET;
	int vgaCRReg = hwp->IOBase + VGA_CRTC_DATA_OFFSET;
	int vgaStatus = hwp->IOBase + VGA_IN_STAT_1_OFFSET;

	xf86DrvMsgVerb (pScrn->scrnIndex, X_INFO, VERBLEV,
			"START register dump ------------------\nEntitlyList is %d",
			pScrn->entityList[0]);

	if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE != pSmi->Chipset))
	{
		xf86ErrorFVerb (VERBLEV, "MISCELLANEOUS OUTPUT\n    %02X\n",
				VGAIN8 (pSmi, VGA_MISC_OUT_R));

		xf86ErrorFVerb (VERBLEV, "\nSEQUENCER\n"
				"    x0 x1 x2 x3  x4 x5 x6 x7  x8 x9 xA xB  xC xD xE xF");
		for (i = 0x00; i <= 0xAF; i++)
		{
			if ((i & 0xF) == 0x0)
				xf86ErrorFVerb (VERBLEV, "\n%02X|", i);
			if ((i & 0x3) == 0x0)
				xf86ErrorFVerb (VERBLEV, " ");
			xf86ErrorFVerb (VERBLEV, "%02X ",
					VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA,
						i));
		}

		xf86ErrorFVerb (VERBLEV, "\n\nCRT CONTROLLER\n"
				"    x0 x1 x2 x3  x4 x5 x6 x7  x8 x9 xA xB  xC xD xE xF");
		for (i = 0x00; i <= 0xAD; i++)
		{
			if (i == 0x20)
				i = 0x30;
			if (i == 0x50)
				i = 0x90;
			if ((i & 0xF) == 0x0)
				xf86ErrorFVerb (VERBLEV, "\n%02X|", i);
			if ((i & 0x3) == 0x0)
				xf86ErrorFVerb (VERBLEV, " ");
			xf86ErrorFVerb (VERBLEV, "%02X ",
					VGAIN8_INDEX (pSmi, vgaCRIndex, vgaCRReg, i));
		}

		xf86ErrorFVerb (VERBLEV, "\n\nGRAPHICS CONTROLLER\n"
				"    x0 x1 x2 x3  x4 x5 x6 x7  x8 x9 xA xB  xC xD xE xF");
		for (i = 0x00; i <= 0x08; i++)
		{
			if ((i & 0xF) == 0x0)
				xf86ErrorFVerb (VERBLEV, "\n%02X|", i);
			if ((i & 0x3) == 0x0)
				xf86ErrorFVerb (VERBLEV, " ");
			xf86ErrorFVerb (VERBLEV, "%02X ",
					VGAIN8_INDEX (pSmi, VGA_GRAPH_INDEX, VGA_GRAPH_DATA,
						i));
		}

		xf86ErrorFVerb (VERBLEV, "\n\nATTRIBUTE 0CONTROLLER\n"
				"    x0 x1 x2 x3  x4 x5 x6 x7  x8 x9 xA xB  xC xD xE xF");
		for (i = 0x00; i <= 0x14; i++)
		{
			(void) VGAIN8 (pSmi, vgaStatus);
			if ((i & 0xF) == 0x0)
				xf86ErrorFVerb (VERBLEV, "\n%02X|", i);
			if ((i & 0x3) == 0x0)
				xf86ErrorFVerb (VERBLEV, " ");
			xf86ErrorFVerb (VERBLEV, "%02X ",
					VGAIN8_INDEX (pSmi, VGA_ATTR_INDEX, VGA_ATTR_DATA_R,
						i));
		}
		(void) VGAIN8 (pSmi, vgaStatus);
		VGAOUT8 (pSmi, VGA_ATTR_INDEX, 0x20);
	}

	xf86ErrorFVerb (VERBLEV, "\n\nDPR    x0       x4       x8       xC");
	for (i = 0x00; i <= 0x44; i += 4)
	{
		if ((i & 0xF) == 0x0)
			xf86ErrorFVerb (VERBLEV, "\n%02X|", i);
		xf86ErrorFVerb (VERBLEV, " %08lX", (unsigned long) READ_DPR (pSmi, i));
	}

	xf86ErrorFVerb (VERBLEV, "\n\nVPR    x0       x4       x8       xC");
	for (i = 0x00; i <= 0x60; i += 4)
	{
		if ((i & 0xF) == 0x0)
			xf86ErrorFVerb (VERBLEV, "\n%02X|", i);
		xf86ErrorFVerb (VERBLEV, " %08lX", (unsigned long) READ_VPR (pSmi, i));

	}

	xf86ErrorFVerb (VERBLEV, "\n\nCPR    x0       x4       x8       xC");
	for (i = 0x00; i <= 0x18; i += 4)
	{
		if ((i & 0xF) == 0x0)
			xf86ErrorFVerb (VERBLEV, "\n%02X|", i);
		xf86ErrorFVerb (VERBLEV, " %08lX", (unsigned long) READ_CPR (pSmi, i));
	}

	if (SMI_MSOC == pSmi->Chipset)
	{
		xf86ErrorFVerb (VERBLEV, "\n\nSCR    x0       x4       x8       xC");
		for (j = 0x00; j <= 0x6C; j += 4)
		{
			if ((j & 0xF) == 0x0)
				xf86ErrorFVerb (VERBLEV, "\n%02X|", j);
			xf86ErrorFVerb (VERBLEV, " %08X", READ_SCR (pSmi, j));
		}
		xf86ErrorFVerb (VERBLEV, "\n\nDCR    x0       x4       x8       xC");
		for (j = 0x200; j <= 0x23C; j += 4)
		{
			if ((j & 0xF) == 0x0)
				xf86ErrorFVerb (VERBLEV, "\n%02X|", j);
			xf86ErrorFVerb (VERBLEV, " %08X", READ_DCR (pSmi, j));
		}
	}

	xf86ErrorFVerb (VERBLEV, "\n\n");
	xf86DrvMsgVerb (pScrn->scrnIndex, X_INFO, VERBLEV,
			"END register dump --------------------\n");
}

/*
 * SMI_DisplayPowerManagementSet -- Sets VESA Display Power Management
 * Signaling (DPMS) Mode.
 */
	static void
SMI_DisplayPowerManagementSet (ScrnInfoPtr pScrn, int PowerManagementMode,
		int flags)
{
	vgaHWPtr hwp = VGAHWPTR (pScrn);
	SMIPtr pSmi = SMIPTR (pScrn);
	CARD8 SR01, SR20, SR21, SR22, SR23, SR24, SR31, SR34;


	xf86DrvMsg("", X_INFO, "Belcon::PowerManagementMode is %d\n", PowerManagementMode);
	/* If we already are in the requested DPMS mode, just return */
	if (pSmi->CurrentDPMS == PowerManagementMode)
	{
		return;
	}


	if ((SMI_MSOC != pSmi->Chipset) && (SMI_MSOCE!= pSmi->Chipset))
	{

#if 0				/* PDR#735 */
		if (pSmi->pInt10 != NULL)
		{
			pSmi->pInt10->ax = 0x4F10;
			switch (PowerManagementMode)
			{
				case DPMSModeOn:
					pSmi->pInt10->bx = 0x0001;
					break;

				case DPMSModeStandby:
					pSmi->pInt10->bx = 0x0101;
					break;

				case DPMSModeSuspend:
					pSmi->pInt10->bx = 0x0201;
					break;

				case DPMSModeOff:
					pSmi->pInt10->bx = 0x0401;
					break;
			}
			pSmi->pInt10->cx = 0x0000;
			pSmi->pInt10->num = 0x10;
			xf86DrvMsg (pScrn->scrnIndex, X_NOTICE,
					"PowerManagementMode:%d\n", PowerManagementMode);

			xf86ExecX86int10 (pSmi->pInt10);

			xf86DrvMsg (pScrn->scrnIndex, X_NOTICE,
					"pSmi->pInt10->ax:%d\n", pSmi->pInt10->ax);

			if (pSmi->pInt10->ax == 0x004F)
			{
				pSmi->CurrentDPMS = PowerManagementMode;
#if 1				/* PDR#835 */
				if (PowerManagementMode == DPMSModeOn)
				{
					SR01 =
						VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x01);
					VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x01,
							SR01 & ~0x20);
				}
#endif
				return;
			}
		}
#else

		/* Save the current SR registers */
		if (pSmi->CurrentDPMS == DPMSModeOn)
		{
			pSmi->DPMS_SR20 =
				VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x20);
			pSmi->DPMS_SR21 =
				VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21);
			pSmi->DPMS_SR31 =
				VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31);
			pSmi->DPMS_SR34 =
				VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x34);
		}

		/* Read the required SR registers for the DPMS handler */
		SR01 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x01);
		SR20 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x20);
		SR21 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21);
		SR22 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x22);
		SR23 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x23);
		SR24 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x24);
		SR31 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31);
		SR34 = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x34);

		switch (PowerManagementMode)
		{
			case DPMSModeOn:
				/* Screen On: HSync: On, VSync : On */
				SR01 &= ~0x20;
				SR20 = pSmi->DPMS_SR20;
				SR21 = pSmi->DPMS_SR21;
				SR22 &= ~0x30;
				SR23 &= ~0xC0;
				SR24 |= 0x01;
				SR31 = pSmi->DPMS_SR31;
				SR34 = pSmi->DPMS_SR34;
				break;

			case DPMSModeStandby:
				/* Screen: Off; HSync: Off, VSync: On */
				SR01 |= 0x20;
				SR20 = (SR20 & ~0xB0) | 0x10;
				SR21 |= 0x88;
				SR22 = (SR22 & ~0x30) | 0x10;
				SR23 = (SR23 & ~0x07) | 0xD8;
				SR24 &= ~0x01;
				SR31 = (SR31 & ~0x07) | 0x00;
				SR34 |= 0x80;
				break;

			case DPMSModeSuspend:
				/* Screen: Off; HSync: On, VSync: Off */
				SR01 |= 0x20;
				SR20 = (SR20 & ~0xB0) | 0x10;
				SR21 |= 0x88;
				SR22 = (SR22 & ~0x30) | 0x20;
				SR23 = (SR23 & ~0x07) | 0xD8;
				SR24 &= ~0x01;
				SR31 = (SR31 & ~0x07) | 0x00;
				SR34 |= 0x80;
				break;

			case DPMSModeOff:
				/* Screen: Off; HSync: Off, VSync: Off */
				SR01 |= 0x20;
				SR20 = (SR20 & ~0xB0) | 0x10;
				SR21 |= 0x88;
				SR22 = (SR22 & ~0x30) | 0x30;
				SR23 = (SR23 & ~0x07) | 0xD8;
				SR24 &= ~0x01;
				SR31 = (SR31 & ~0x07) | 0x00;
				SR34 |= 0x80;
				break;

			default:
				xf86ErrorFVerb (VERBLEV, "Invalid PowerManagementMode %d passed to "
						"SMI_DisplayPowerManagementSet\n",
						PowerManagementMode);
				return;
		}

		xf86DrvMsg (pScrn->scrnIndex, X_NOTICE,
				"PowerManagementMode:%d\n", PowerManagementMode);

		/* Wait for vertical retrace */
		while (hwp->readST01 (hwp) & 0x8);
		while (!(hwp->readST01 (hwp) & 0x8));

		/* Write the registers */
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x01, SR01);
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x34, SR34);
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x31, SR31);
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x20, SR20);
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x22, SR22);
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x23, SR23);
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x21, SR21);
		VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x24, SR24);
#endif

	}
	else if(SMI_MSOC == pSmi->Chipset)
	{

		setDPMS_501 (pSmi, PowerManagementMode);

		switch (PowerManagementMode)
		{
			case DPMSModeOn:
				setPower_501 (pSmi, 0, 0, 0);
				break;

			case DPMSModeStandby:
				xf86DrvMsg ("", X_NOTICE, "DPMSModeOn start\n ");
				setPower_501 (pSmi, 0, 0, 2);
				xf86DrvMsg ("", X_NOTICE, "DPMSModeOn stop\n ");
				break;

			case DPMSModeSuspend:
				setPower_501 (pSmi, 0, 0, 2);
				break;

			case DPMSModeOff:
				setPower_501 (pSmi, 0, 0, 2);
				break;
			default:
				break;

		}


	}
	else
	{
		setDPMS_750(pSmi, PowerManagementMode);

		switch (PowerManagementMode)
		{
			case DPMSModeOn:
				setPower_750 (pSmi, 0, 0, 0);
				break;

			case DPMSModeStandby:
				xf86DrvMsg ("", X_NOTICE, "DPMSModeOn start\n ");
				setPower_750 (pSmi, 0, 0, 2);
				xf86DrvMsg ("", X_NOTICE, "DPMSModeOn stop\n ");
				break;

			case DPMSModeSuspend:
				setPower_750 (pSmi, 0, 0, 2);
				break;

			case DPMSModeOff:
				setPower_750 (pSmi, 0, 0, 2);
				break;
			default:
				break;

		}

	}
	/* Save the current power state */
	pSmi->CurrentDPMS = PowerManagementMode;

}

	static void
SMI_ProbeDDC (ScrnInfoPtr pScrn, int index)
{
	vbeInfoPtr pVbe;
	if (xf86LoadSubModule (pScrn, "vbe"))
	{
		pVbe = VBEInit (NULL, index);
		ConfiguredMonitor = vbeDoEDID (pVbe, NULL);
		vbeFree (pVbe);
	}
}

	static unsigned int
SMI_ddc1Read (ScrnInfoPtr pScrn)
{
	register vgaHWPtr hwp = VGAHWPTR (pScrn);
	SMIPtr pSmi = SMIPTR (pScrn);
	unsigned int ret;

	if(pSmi->RemoveBIOS != TRUE)
	{
		while (hwp->readST01 (hwp) & 0x8);
		while (!(hwp->readST01 (hwp) & 0x8));
	}

	ret = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x72) & 0x08;

	return (ret);
}


/*
   this function is prepare for sm712
 */
	static Bool
SMI_ddc1 (int scrnIndex)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	SMIPtr pSmi = SMIPTR (pScrn);
	Bool success = FALSE;
	xf86MonPtr pMon;
	unsigned char tmp;


	tmp = VGAIN8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x72);
	VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x72, tmp | 0x20);

	pMon = xf86PrintEDID (xf86DoEDID_DDC1 (scrnIndex,
				// #if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0)
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0)
				vgaHWddc1SetSpeedWeak (),
#elif XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
				vgaHWddc1SetSpeedWeak (),
#else
				vgaHWddc1SetSpeed,
#endif
				SMI_ddc1Read));

	if (pMon != NULL)
	{
		success = TRUE;
	}
	xf86SetDDCproperties (pScrn, pMon);

	VGAOUT8_INDEX (pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x72, tmp);

	return (success);
}

	static Bool
SMI_MSOCSetMode_501(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
	SMIPtr pSmi = SMIPTR (pScrn);
	unsigned int tmpData, tmpReg;



	/* Remarked by Belcon */
	//      mode->VRefresh = 60;
	/*boyod */
	/*
	   xf86DrvMsg("", X_INFO, "Belcon: Hdisplay and Vdisplay is %d, %d\n", mode->HDisplay, mode->VDisplay);
	   xf86DrvMsg("", X_INFO, "Belcon: virtualX is %d, virtualY is %d\n", pScrn->virtualX, pScrn->virtualY);
	 */

	if (!pSmi->IsSecondary)
	{
		// xf86DrvMsg("", X_INFO, "Belcon: SMI_MSOCSetMode(), nHertz is %f\n", mode->VRefresh);
		panelSetMode_501 (pSmi, mode->HDisplay, mode->VDisplay, 0, 60, pSmi->Stride,
				pScrn->depth);
		panelUseLCD (pSmi, TRUE);
		tmpReg = DCR00;
	}
	else
	{
		//       xf86DrvMsg("", X_INFO, "Belcon: SMI_MSOCSetMode(), before crtSetMode(), %d x %d, nHertz is %f, pitch is %d, depth is %d\n", mode->HDisplay, mode->VDisplay, mode->VRefresh, pSmi->Stride, pScrn->depth);
		crtSetMode_501 (pSmi, mode->HDisplay, mode->VDisplay, 0,
				(int) (mode->VRefresh + 0.5), pSmi->Stride, pScrn->depth);
		panelUseCRT (pSmi, TRUE);
		tmpReg = DCR200;
	}
	/* Jason 2010/09/21
	   Set registers according to modeline sync polarity */
	if (mode->Flags & V_PHSYNC) {
		tmpData = READ_DCR(pSmi, tmpReg);
		WRITE_DCR(pSmi, tmpReg, (tmpData & 0xFFFFEFFF));
	} 
	if (mode->Flags & V_NHSYNC){
		tmpData = READ_DCR(pSmi, tmpReg);
		WRITE_DCR(pSmi, tmpReg, (tmpData | 0x00001000));
	} 
	if (mode->Flags & V_PVSYNC){
		tmpData = READ_DCR(pSmi, tmpReg);
		WRITE_DCR(pSmi, tmpReg, (tmpData & 0xFFFFDFFF));
	} 
	if (mode->Flags & V_NVSYNC) {
		tmpData = READ_DCR(pSmi, tmpReg);
		WRITE_DCR(pSmi, tmpReg, (tmpData | 0x00002000));
	} 


	/*boyod */


	return (TRUE);

}
#define PANEL_OUTPUT 1
#define CRT_OUTPUT 2
static void monk_set_output(SMIPtr pSmi,display_t channel,int terminal)
{
	unsigned long value;
	if(terminal == PANEL_OUTPUT)
	{
		value = regRead32(pSmi,PANEL_DISPLAY_CTRL);
		if(channel == PANEL)
			value = FIELD_SET(value,PANEL_DISPLAY_CTRL,SELECT,PANEL);
		else
			value = FIELD_SET(value,PANEL_DISPLAY_CTRL,SELECT,CRT);	
		regWrite32(pSmi,PANEL_DISPLAY_CTRL,value);
	}
	else
	{
		value = regRead32(pSmi,CRT_DISPLAY_CTRL);
		if(channel == PANEL)
			value = FIELD_SET(value,CRT_DISPLAY_CTRL,SELECT,PANEL);
		else
			value = FIELD_SET(value,CRT_DISPLAY_CTRL,SELECT,CRT);
		regWrite32(pSmi,CRT_DISPLAY_CTRL,value);		
	}	
}

	static Bool
SMI_MSOCSetMode_750(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
	SMIPtr pSmi = SMIPTR (pScrn);
	unsigned long ulvalue;
	if(pSmi->edid_enable)
	{
		SMI_DDCGetModes(mode, &mode_table_edid);
	}


	if (!pSmi->IsSecondary)
	{	
		SetMode_750 (pSmi, mode->HDisplay, mode->VDisplay, 0, (int)(mode->VRefresh + 0.5) ,
				PANEL,pSmi->Stride, pScrn->depth);
		open_primary_channel (pSmi, TRUE);
		int tmp = regRead32(pSmi,0x80200);
		tmp &= 0xfff3ffff;
		regWrite32(pSmi,0x80200,tmp);
	} 
	else
	{   
		SetMode_750 (pSmi, mode->HDisplay, mode->VDisplay, 0, (int)(mode->VRefresh + 0.5) ,
				CRT,pSmi->Stride, pScrn->depth);
		open_secondary_channel (pSmi, TRUE);
	}
#if 0
	/*	Below code make driver expansion feature enabled.
		Note that expansion only affect secondary channel  data
		which will show the second screen of Xorg
		en...the primary screen of Xorg will not do expansion
		We will nice the limit in phaseII cuz this version (phase I)
		is too mass to modify or enchant.
	 */
	if(pSmi->IsSecondary)
	{
		if(pSmi->output == OUTPUT_TFT_CRT && pSmi->xlcd!=0 && pSmi->ylcd !=0)
		{		
			/*	step 1: reset crt-channel timing  and disable crt-hardware cusor	*/

			setSecTiming(pSmi,CRT,pSmi->xlcd,pSmi->ylcd,60);

			ulvalue = regRead32(pSmi,CRT_HWC_ADDRESS);
			ulvalue = FIELD_SET(ulvalue,CRT_HWC_ADDRESS,ENABLE,DISABLE);
			regWrite32(pSmi,CRT_HWC_ADDRESS,ulvalue);

			/*	step 2: set crt-scaller register		*/
			setScalar(pSmi,mode->HDisplay,pSmi->xlcd,mode->VDisplay,pSmi->ylcd);

			/*
			   step 3: let CRT-OUTPUT show panel-channel	(xorg.screen 1)
			   let PANEL-OUTPUT show crt-channel	(xorg.screen 0)
			 */
			monk_set_output(pSmi,CRT,PANEL_OUTPUT);	
			monk_set_output(pSmi,PANEL,CRT_OUTPUT);
		}
		else
		{
			setNoScalar(pSmi);
			/* 	set tft type	*/
			ulvalue = regRead32(pSmi,PANEL_DISPLAY_CTRL);			
			if(pSmi->output == OUTPUT_TFT_CRT)
			{
				switch (pSmi->pnltype)
				{
					case PNLTYPE_18:
						ulvalue = FIELD_SET(ulvalue,PANEL_DISPLAY_CTRL,LCD,18bit);
						break;
					case PNLTYPE_24:
						ulvalue = FIELD_SET(ulvalue,PANEL_DISPLAY_CTRL,LCD,24bit);
						break;
					case PNLTYPE_36:
						ulvalue = FIELD_SET(ulvalue,PANEL_DISPLAY_CTRL,LCD,36bit);
						break;
					default:
						ulvalue = FIELD_SET(ulvalue,PANEL_DISPLAY_CTRL,LCD,24bit);
						break;
				}			
			}
			else if (pSmi->output == OUTPUT_TFT_TFT)
			{
				ulvalue = FIELD_SET(ulvalue,PANEL_DISPLAY_CTRL,LCD,18bit);
			}

			/*	set controller	*/			
			ulvalue = FIELD_SET(ulvalue,PANEL_DISPLAY_CTRL,SELECT,PANEL);
			regWrite32(pSmi,PANEL_DISPLAY_CTRL,ulvalue);

			ulvalue = FIELD_SET(regRead32(pSmi,CRT_DISPLAY_CTRL),CRT_DISPLAY_CTRL,SELECT,CRT);
			regWrite32(pSmi,CRT_DISPLAY_CTRL,ulvalue);

			/*	SET DAC	*/
			if(pSmi->output != OUTPUT_TFT_TFT)
			{
				ulvalue = FIELD_SET(regRead32(pSmi,MISC_CTRL),MISC_CTRL,DAC_POWER,ON);
				regWrite32(pSmi,MISC_CTRL,ulvalue);
				ulvalue = FIELD_SET(regRead32(pSmi,SYSTEM_CTRL),SYSTEM_CTRL,DPMS,VPHP);
				regWrite32(pSmi,SYSTEM_CTRL,ulvalue);
			}			
		}		
	}
#endif
	return (TRUE);
}
#ifdef RANDR
	static Bool
SMI_RandRGetInfo(ScrnInfoPtr pScrn, Rotation *rotations)
{
	SMIPtr pSmi = SMIPTR (pScrn); 

	if(pSmi->RandRRotation)
		*rotations = RR_Rotate_0 | RR_Rotate_90 | RR_Rotate_180 | RR_Rotate_270;
	else
		*rotations = RR_Rotate_0;
	return TRUE;
}

	static Bool
SMI_RandRSetConfig(ScrnInfoPtr pScrn, xorgRRConfig *config)
{
	SMIPtr pSmi = SMIPTR (pScrn);
	int bytesPerPixel = pScrn->bitsPerPixel / 8;

	switch(config->rotation) {
		case RR_Rotate_0:
			pSmi->rotate = SMI_ROTATE_ZERO;
			pScrn->PointerMoved = pSmi->PointerMoved;
			break;

		case RR_Rotate_90:
			pSmi->rotate = SMI_ROTATE_CW;
			pScrn->PointerMoved = SMI_PointerMoved;
			pSmi->height= pSmi->ShadowHeight = pScrn->virtualX;
			pSmi->width = pSmi->ShadowWidth= pScrn->virtualY;
			pSmi->ShadowWidthBytes  = (pSmi->ShadowWidth * bytesPerPixel + 15) & ~15;
			break;

		case RR_Rotate_180:
			pSmi->rotate = SMI_ROTATE_UD;
			pScrn->PointerMoved = SMI_PointerMoved;
			pSmi->height= pSmi->ShadowHeight = pScrn->virtualY;
			pSmi->width = pSmi->ShadowWidth= pScrn->virtualX;
			pSmi->ShadowWidthBytes  = (pSmi->ShadowWidth * bytesPerPixel + 15) & ~15;			
			break;						

		case RR_Rotate_270:
			pSmi->rotate = SMI_ROTATE_CCW;
			pScrn->PointerMoved = SMI_PointerMoved;
			pSmi->height= pSmi->ShadowHeight = pScrn->virtualX;
			pSmi->width = pSmi->ShadowWidth= pScrn->virtualY;
			pSmi->ShadowWidthBytes  = (pSmi->ShadowWidth * bytesPerPixel + 15) & ~15;
			break;

		default:
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
					"Unexpected rotation in SMI_RandRSetConfig!\n");
			pSmi->rotate = 0;
			pScrn->PointerMoved = pSmi->PointerMoved;
			return FALSE;
	}  	
	return TRUE;
}

	static Bool
SMI_DriverFunc(ScrnInfoPtr pScrn, xorgDriverFuncOp op, pointer data)
{
	switch(op) {
		case RR_GET_INFO:
			return SMI_RandRGetInfo(pScrn, (Rotation*)data);
		case RR_SET_CONFIG:
			return SMI_RandRSetConfig(pScrn, (xorgRRConfig*)data);
		default:
			return FALSE;
	}
	return FALSE;
}
#endif
