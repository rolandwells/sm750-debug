/* Header:   //Mercury/Projects/archives/XFree86/4.0/smi_driver.c-arc   1.42   03 Jan 2001 13:52:16   Frido  $ */

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
#if XORG_VERSION_CURRENT < 10706000
#include "xf86Resources.h"
#include "xf86RAC.h"
#endif
#include "xf86DDC.h"
#include "xf86int10.h"
#include "vbe.h"


#include "smi.h"
#include "smi_501.h"
#include "smilynx.h"
#include "smi_crtc.h"
#include "smi750.h"


#include "globals.h"
#define DPMS_SERVER
#if XORG_VERSION_CURRENT < 10706000
#include <X11/extensions/dpms.h>
#endif
/* start [record in file]*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
/* end */


//#include "750ddk.h"

/* For MTRR */
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <asm/mtrr.h>
#include <fcntl.h>

#include <pciaccess.h>

/*
 * Internals
 */
static Bool SMI_MapMmio(ScrnInfoPtr pScrn);
static Bool SMI_DetectMem(ScrnInfoPtr pScrn);
static void SMI_EnableMmio(ScrnInfoPtr pScrn);
static void SMI_DisableMmio(ScrnInfoPtr pScrn);
static Bool SMI_HWInit(ScrnInfoPtr pScrn);

/*
 * Forward definitions for the functions that make up the driver.
 */

static const OptionInfoRec * SMI_AvailableOptions(int chipid, int busid);
static void SMI_Identify(int flags);
static Bool SMI_Probe(DriverPtr drv, int flags);
//mill add
static Bool SMI_PciProbe(DriverPtr drv, int entity, struct pci_device *dev, intptr_t data);
static Bool SMI_PreInit(ScrnInfoPtr pScrn, int flags);
static Bool SMI_EnterVT(int scrnIndex, int flags);
static void SMI_LeaveVT(int scrnIndex, int flags);
static Bool SMI_ScreenInit(int scrnIndex, ScreenPtr pScreen, int argc,
        char **argv);
static void SMI_DisableVideo(ScrnInfoPtr pScrn);
static void SMI_EnableVideo(ScrnInfoPtr pScrn);
static Bool SMI_CloseScreen(int scrnIndex, ScreenPtr pScreen);
static Bool SMI_SaveScreen(ScreenPtr pScreen, int mode);
static void SMI_FreeScreen(int ScrnIndex, int flags);
static void SMI_ProbeDDC(ScrnInfoPtr pScrn, int index);
static void SMI_DetectPanelSize(ScrnInfoPtr pScrn);
static void SMI_DetectMCLK(ScrnInfoPtr pScrn);

/*
 * xf86VDrvMsgVerb prints up to 14 characters prefix, where prefix has the
 * format "%s(%d): " so, use name "SMI" instead of "Silicon Motion"
 */
#define SILICONMOTION_NAME          "SMI"
#define SILICONMOTION_DRIVER_NAME   "siliconmotion"
#if 0
#define SILICONMOTION_VERSION_NAME  PACKAGE_VERSION
#define SILICONMOTION_VERSION_MAJOR PACKAGE_VERSION_MAJOR
#define SILICONMOTION_VERSION_MINOR PACKAGE_VERSION_MINOR
#define SILICONMOTION_PATCHLEVEL    PACKAGE_VERSION_PATCHLEVEL

#define SILICONMOTION_DRIVER_VERSION ((SILICONMOTION_VERSION_MAJOR << 24) | \
        (SILICONMOTION_VERSION_MINOR << 16) | \
        (SILICONMOTION_PATCHLEVEL))
#else
#define SILICONMOTION_VERSION_MAJOR 3
#define SILICONMOTION_VERSION_MINOR 0
#define SILICONMOTION_PATCHLEVEL    9

#define SILICONMOTION_DRIVER_VERSION ((SILICONMOTION_VERSION_MAJOR << 24) | \
        (SILICONMOTION_VERSION_MINOR << 16) | \
        (SILICONMOTION_PATCHLEVEL))

#define SILICONMOTION_VERSION_NAME   "3.0.9"
#endif

#if SMI_DEBUG
int smi_indent = 1;
#endif

/* for dualhead */
int gSMIEntityIndex = -1;
int entity_private_index = -1;
extern char *outputName[];

static int g_recordFD = 0;

/*
 * This contains the functions needed by the server after loading the
 * driver module.  It must be supplied, and gets added the driver list by
 * the Module Setup funtion in the dynamic case.  In the static case a
 * reference to this is compiled in, and this requires that the name of
 * this DriverRec be an upper-case version of the driver name.
 */

//mill
static const struct pci_id_match SMIPciIdMatchList[] = {
    { PCI_VENDOR_SMI, PCI_MATCH_ANY, PCI_MATCH_ANY, PCI_MATCH_ANY,
        PCI_CLASS_DISPLAY<<(PCI_CLASS_SHIFT - 8) , PCI_CLASS_MASK>>8, 0 },

    { 0, 0, 0 }
};

static const char *ddcSymbols[] =
{
    "xf86PrintEDID",
    "xf86DoEDID_DDC1",
    "xf86DoEDID_DDC2",
    "xf86SetDDCproperties",
    NULL
};

static const char *i2cSymbols[] =
{
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


/* Supported chipsets */
static SymTabRec SMIChipsets[] =
{
    /*
       { PCI_CHIP_SMI910, "Lynx"    },
       { PCI_CHIP_SMI810, "LynxE"   },
       { PCI_CHIP_SMI820, "Lynx3D"  },
       { PCI_CHIP_SMI710, "LynxEM"  },
       { PCI_CHIP_SMI712, "LynxEM+" },
       { PCI_CHIP_SMI720, "Lynx3DM" },
       { PCI_CHIP_SMI731, "Cougar3DR" },
       */
    { PCI_CHIP_SMI501, "MSOC sm502"	 },
    { PCI_CHIP_SMI750, "Lynx EXP sm750" },
    { PCI_CHIP_SMI718, "Lynx SE+ sm718" },
    { PCI_CHIP_SMI712, "Lynx EM+ sm712"},
    { -1,             NULL      }
};

static PciChipsets SMIPciChipsets[] =
{
    /* numChipset,	PciID,			Resource */
    /*    
          { PCI_CHIP_SMI910,	PCI_CHIP_SMI910,	RES_SHARED_VGA },
          { PCI_CHIP_SMI810,	PCI_CHIP_SMI810,	RES_SHARED_VGA },
          { PCI_CHIP_SMI820,	PCI_CHIP_SMI820,	RES_SHARED_VGA },
          { PCI_CHIP_SMI710,	PCI_CHIP_SMI710,	RES_SHARED_VGA },
          { PCI_CHIP_SMI712,	PCI_CHIP_SMI712,	RES_SHARED_VGA },
          { PCI_CHIP_SMI720,	PCI_CHIP_SMI720,	RES_SHARED_VGA },
          { PCI_CHIP_SMI731,	PCI_CHIP_SMI731,	RES_SHARED_VGA },
          */    
    { PCI_CHIP_SMI501,	PCI_CHIP_SMI501,	RES_UNDEFINED  },
    { PCI_CHIP_SMI750,	PCI_CHIP_SMI750,	RES_SHARED_VGA },
    { PCI_CHIP_SMI718,	PCI_CHIP_SMI718,	RES_SHARED_VGA },
    { PCI_CHIP_SM712,   PCI_CHIP_SM712,     RES_SHARED_VGA },
    { -1,		-1,			RES_UNDEFINED  }
};


_X_EXPORT DriverRec SILICONMOTION =
{
    SILICONMOTION_DRIVER_VERSION,
    SILICONMOTION_DRIVER_NAME,
    SMI_Identify,
    SMI_Probe,
    SMI_AvailableOptions,
    NULL,
    0, 
    NULL,
    SMIPciIdMatchList,
    SMI_PciProbe,
};

typedef enum
{
    OPTION_PCI_BURST,
    OPTION_PCI_RETRY,
    OPTION_NOACCEL,
    OPTION_MCLK,
    OPTION_MXCLK,
    OPTION_SWCURSOR,
    OPTION_HWCURSOR,
    OPTION_ARGBCURSOR,
    OPTION_VIDEOKEY,
    OPTION_BYTESWAP,
    /* CZ 26.10.2001: interlaced video */
    OPTION_INTERLACED,
    /* end CZ */
    OPTION_USEBIOS,
    OPTION_DUALVIEW,
    OPTION_ACCELMETHOD,
    OPTION_PANEL_SIZE,
    OPTION_USE_FBDEV,
    OPTION_CSCVIDEO,
    NUMBER_OF_OPTIONS,
    OPTION_XLCD,
    OPTION_YLCD,
    OPTION_TFT_TYPE,
} SMIOpts;

static const OptionInfoRec SMIOptions[] =
{
    { OPTION_PCI_BURST,	     "pci_burst",	  OPTV_BOOLEAN, {0}, TRUE },
    { OPTION_PCI_RETRY,	     "pci_retry",	  OPTV_BOOLEAN, {0}, TRUE },
    { OPTION_NOACCEL,	     "NoAccel",		  OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_MCLK,	     "MCLK",		  OPTV_FREQ,	{0}, FALSE },
    { OPTION_MXCLK,	     "MXCLK",		  OPTV_FREQ,	{0}, FALSE },
    { OPTION_HWCURSOR,	     "HWCursor",	  OPTV_BOOLEAN, {0}, TRUE },
    { OPTION_SWCURSOR,	     "SWCursor",	  OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_ARGBCURSOR,	"ARGBCursor",	OPTV_BOOLEAN,	{0}, TRUE	},
    { OPTION_VIDEOKEY,	     "VideoKey",	  OPTV_INTEGER, {0}, FALSE },
    { OPTION_BYTESWAP,	     "ByteSwap",	  OPTV_BOOLEAN, {0}, FALSE },
    /* CZ 26.10.2001: interlaced video */
    { OPTION_INTERLACED,     "Interlaced",        OPTV_BOOLEAN, {0}, FALSE },
    /* end CZ */
    { OPTION_USEBIOS,	     "UseBIOS",		  OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_DUALVIEW,	     "DualView",	  OPTV_BOOLEAN,	{0}, TRUE },
    { OPTION_ACCELMETHOD,    "AccelMethod",       OPTV_STRING,  {0}, FALSE },
    { OPTION_PANEL_SIZE,     "PanelSize",	  OPTV_ANYSTR,	{0}, FALSE },
    { OPTION_USE_FBDEV,	     "UseFBDev",	  OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_CSCVIDEO,	     "CSCVideo",	  OPTV_BOOLEAN, {0}, TRUE },
    { OPTION_XLCD,	"XLCD",	OPTV_INTEGER, {0},	FALSE	},
    { OPTION_YLCD,	"YLCD", OPTV_INTEGER, {0},	FALSE	},
    { OPTION_TFT_TYPE,	"TFT_TYPE", OPTV_INTEGER, {0},	FALSE	},    
    { -1,		     NULL,		  OPTV_NONE,	{0}, FALSE }
};

#ifdef XFree86LOADER

static MODULESETUPPROTO(siliconmotionSetup);

static XF86ModuleVersionInfo SMIVersRec =
{
    "siliconmotion",
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XORG_VERSION_CURRENT,
    SILICONMOTION_VERSION_MAJOR,
    SILICONMOTION_VERSION_MINOR,
    SILICONMOTION_PATCHLEVEL,
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
_X_EXPORT XF86ModuleData siliconmotionModuleData =
{
    &SMIVersRec,
    siliconmotionSetup,
    NULL
};

    static pointer
siliconmotionSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
    static Bool setupDone = FALSE;

    if (!setupDone) {
        setupDone = TRUE;
        xf86AddDriver(&SILICONMOTION, module, HaveDriverFuncs);

        /*
         * The return value must be non-NULL on success even though there
         * is no TearDownProc.
         */
        return (pointer) 1;

    } else {
        if (errmaj) {
            *errmaj = LDR_ONCEONLY;
        }
        return NULL;
    }
}

#endif /* XFree86LOADER */

    static Bool
SMI_GetRec(ScrnInfoPtr pScrn)
{
    ENTER();

    /*
     * Allocate an 'Chip'Rec, and hook it into pScrn->driverPrivate.
     * pScrn->driverPrivate is initialised to NULL, so we can check if
     * the allocation has already been done.
     */
    if (pScrn->driverPrivate == NULL) {
        pScrn->driverPrivate = xnfcalloc(sizeof(SMIRec), 1);
    }

    LEAVE(TRUE);
}

    static void
SMI_FreeRec(ScrnInfoPtr pScrn)
{
    SMIPtr	pSmi = SMIPTR(pScrn);

    ENTER();

    if (pSmi) {
		xfree(pSmi->fonts);
        xfree(pSmi->save);
        xfree(pSmi->mode);
        xfree(pScrn->driverPrivate);
        pScrn->driverPrivate = NULL;        
    }

    LEAVE();
}

    static const OptionInfoRec *
SMI_AvailableOptions(int chipid, int busid)
{
    ENTER();

    LEAVE(SMIOptions);
}

    static void
SMI_Identify(int flags)
{
    ENTER();

    xf86PrintChipsets(SILICONMOTION_NAME, "driver (version "
            SILICONMOTION_VERSION_NAME ") for Silicon Motion Lynx chipsets",
            SMIChipsets); 

    LEAVE();
}

//mill add
#define PCI_CMD_STAT_REG 0x04
static void SMI_pciMemAccessEnable(int entityIndex,pointer private)
{
    uint32_t save;
    struct pci_device *dev;

    ENTER();

    dev = xf86GetPciInfoForEntity(entityIndex);

    pci_device_cfg_read_u32(dev, &save, PCI_CMD_STAT_REG);

    pci_device_cfg_write_u32(dev, save|0x02, PCI_CMD_STAT_REG);

    LEAVE();
}

static Bool entity_init(ScrnInfoPtr pScrn,int entity,EntityInfoPtr pEntInfo)
{	
    DevUnion * pDevUnion;
    SMIEntPtr pEntPriv;
    ENTER();

    xf86SetEntitySharable(entity);
    if(entity_private_index == -1)
        entity_private_index = xf86AllocateEntityPrivateIndex();

    pDevUnion = xf86GetEntityPrivate(pScrn->entityList[0],entity_private_index);

    if(!pDevUnion->ptr)
    {
        /* this first time entity init called */
#if 0        
        switch (pEntInfo->chipset)
        {
            case PCI_CHIP_SMI750:
                pEntPriv = pDevUnion->ptr = xnfcalloc(sizeof(SMI750EntRec),1);
//                         pEntPriv = pDevUnion->ptr;
                lastInst = &(((SMI750EntPtr)pEntPriv)->lastInstance);
                break;
            case PCI_CHIP_SM712:
                pEntPriv = pDevUnion->ptr = xnfcalloc(sizeof(SMI712EntRec),1);                
                lastInst = &(((SMI712EntPtr)pEntPriv)->lastInstance);
                break;                 
        }        
#endif
         pEntPriv = pDevUnion->ptr = xnfcalloc(sizeof(SMIEntRec),1);     
         pEntPriv->lastInstance = -1;
    }
    else
    {
        pEntPriv = pDevUnion->ptr;        
        XMSG("Multi-card requirement ?? not supported by Xrandr 1.3- arch\n");
        LEAVE(FALSE);
    }
    
    pEntPriv->lastInstance++;    
    xf86SetEntityInstanceForScreen(pScrn,pScrn->entityList[0],pEntPriv->lastInstance);    
    LEAVE(TRUE);
}


    static Bool
SMI_PciProbe(DriverPtr drv, int entity, struct pci_device *dev, intptr_t data)
{
    int i;
    Bool foundScreen = FALSE;
    ScrnInfoPtr	pScrn;
    EntityInfoPtr	pEnt;

    ENTER();

    if(dev->vendor_id == PCI_VENDOR_SMI)
        xf86DrvMsg(0, X_WARNING,"smi driver: supported device 0x%x at %2.2x@%2.2x:%2.2x:%1.1x\n",
                dev->device_id, dev->bus, dev->domain, dev->dev, dev->func);

    if ((pScrn = xf86ConfigPciEntity(NULL, 0, entity,
                    SMIPciChipsets, NULL,
                    SMI_pciMemAccessEnable, NULL, NULL, NULL)))
    {
        pScrn->driverVersion = SILICONMOTION_DRIVER_VERSION;
        pScrn->driverName    = SILICONMOTION_DRIVER_NAME;
        pScrn->name	     	 = SILICONMOTION_NAME;
        pScrn->Probe	     = SMI_Probe;
        pScrn->PreInit	     = SMI_PreInit;
        pScrn->ScreenInit    = SMI_ScreenInit;
        pScrn->SwitchMode    = SMI_SwitchMode;
        pScrn->AdjustFrame   = SMI_AdjustFrame;
        pScrn->EnterVT   	 = SMI_EnterVT;
        pScrn->LeaveVT   	 = SMI_LeaveVT;
        pScrn->FreeScreen    = SMI_FreeScreen;
        foundScreen	    	 = TRUE;				
#if 1		
        pEnt = xf86GetEntityInfo(entity);
        //if(pEnt->chipset == PCI_CHIP_SMI750)
        {
            foundScreen = entity_init(pScrn,entity,pEnt);
        }
        xfree(pEnt);
#endif		
    }


    LEAVE(foundScreen);
}



static Bool
SMI_Probe(DriverPtr drv, int flags)
{
    int i;
    GDevPtr *devSections;
    int *usedChips;
    int numDevSections;
    int numUsed;
    Bool foundScreen = FALSE;

    ENTER();

    numDevSections = xf86MatchDevice(SILICONMOTION_DRIVER_NAME, &devSections);

    if (numDevSections <= 0)
        /* There's no matching device section in the config file, so quit now. */
        LEAVE(FALSE);

#ifndef XSERVER_LIBPCIACCESS
    if (xf86GetPciVideoInfo() == NULL)
        LEAVE(FALSE);
#endif

    numUsed = xf86MatchPciInstances(SILICONMOTION_NAME, PCI_SMI_VENDOR_ID,
            SMIChipsets, SMIPciChipsets, devSections,
            numDevSections, drv, &usedChips);

    /* Free it since we don't need that list after this */
    xfree(devSections);
    if (numUsed <= 0)
        LEAVE(FALSE);

    if (flags & PROBE_DETECT)
        foundScreen = TRUE;
    else {
        ScrnInfoPtr	pScrn;
        EntityInfoPtr	pEnt;

        for (i = 0; i < numUsed; i++) {
            if ((pScrn = xf86ConfigPciEntity(NULL, 0, usedChips[i],
                            SMIPciChipsets, NULL,
                            NULL, NULL, NULL, NULL))) {
                pScrn->driverVersion = SILICONMOTION_DRIVER_VERSION;
                pScrn->driverName    = SILICONMOTION_DRIVER_NAME;
                pScrn->name	     = SILICONMOTION_NAME;
                pScrn->Probe	     = SMI_Probe;
                pScrn->PreInit	     = SMI_PreInit;
                pScrn->ScreenInit    = SMI_ScreenInit;
                pScrn->SwitchMode    = SMI_SwitchMode;
                pScrn->AdjustFrame   = SMI_AdjustFrame;

                if ((pEnt = xf86GetEntityInfo(usedChips[i]))) {
                    pScrn->EnterVT   = SMI_EnterVT;
                    pScrn->LeaveVT   = SMI_LeaveVT;
                    xfree(pEnt);
                }
                pScrn->FreeScreen    = SMI_FreeScreen;
                foundScreen	     = TRUE;
            }
        }
    }
    xfree(usedChips);

    LEAVE(foundScreen);
}

static Bool SMI_PreInit(ScrnInfoPtr pScrn, int flags)
{
    EntityInfoPtr pEnt;
    SMIPtr pSmi;
    SMIEntPtr pEntPriv;
    MessageType from;
    vgaHWPtr hwp;

    ENTER();


    /* create or open file for temporary message recorde */    
    
#ifndef XSERVER_LIBPCIACCESS
    XMSG("XSERVER LIB PCI ACCESS NOT DEFINED \n");
#endif

    /* Ignoring the Type list for now.  It might be needed when multiple cards are supported.   	*/

    if (pScrn->numEntities > 1)
        LEAVE(FALSE);

    /* Allocate the SMIRec driverPrivate */
    if (!SMI_GetRec(pScrn))
        LEAVE(FALSE);
    pSmi = SMIPTR(pScrn);

    /* Find the PCI slot for this screen */
    pEnt = xf86GetEntityInfo(pScrn->entityList[0]);	    
    pSmi->PciInfo = xf86GetPciInfoForEntity(pEnt->index);
    pSmi->Primary = xf86IsPrimaryPci(pSmi->PciInfo);
    pSmi->Chipset = PCI_DEV_DEVICE_ID(pSmi->PciInfo);

    XMSG("pSmi->Chipset = %08x\n",pSmi->Chipset);

    if (IS_MSOC(pSmi)){
        pSmi->Save = SMI501_Save;
        pSmi->save = xnfcalloc(sizeof(MSOCRegRec), 1);
        pSmi->mode = xnfcalloc(sizeof(MSOCRegRec), 1);
    }
    else if(IS_SM750(pSmi)|| IS_SM718(pSmi))
    {		
        pSmi->Save = SMI750_Save;
        pSmi->save = xnfcalloc(sizeof(SMI750RegRec),1);
        pSmi->mode = NULL;			
        pEntPriv = xf86GetEntityPrivate(pScrn->entityList[0],entity_private_index)->ptr;			
        pSmi->entityPrivate = pEntPriv;		

        xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                "SMI driver for sm750 support 2 output,%s and %s\nPlease make sure your "
                "xorg.conf file specify the correct output name\n",outputName[0],outputName[1]);


#if 0		
        if(pSmi->DualView)
        {			
            /*
               in case of DualHead, we need to determine if we are the 'primary' head 
               or the 'secondary' head. In order to do that, at the end of first 
               initialisation, PrimInit is set as DONE to the shared entity,So that
               the second initialisation knows that something has been done before it
               This always assume that the first device initialised is the primary head
               and the second the secondary head.
               */
            if(!xf86IsPrimInitDone(pScrn->entityList[0]))
            {
                /* 	yes , it's the first initialisation, and this is primary head		*/
                pSmi->IsSecCrtc = FALSE;
                pEntPriv->pri_pScrn = pScrn;
            }
            else
            {
                /*	this is secondary head	*/				
                pSmi->IsSecCrtc = TRUE;
                pEntPriv->sec_pScrn = pScrn;				
            }
        }		
#endif		
    }
    else if(IS_SM712(pSmi)) //monk modified ++ IS_SM712(pSmi)
    {
       	pSmi->Save = SMILynx_Save;
        pSmi->save = xnfcalloc(sizeof(SMILynxRegRec), 1);
        pSmi->mode = xnfcalloc(sizeof(SMILynxRegRec), 1);
        pEntPriv = xf86GetEntityPrivate(pScrn->entityList[0],entity_private_index)->ptr;	
        pSmi->entityPrivate = pEntPriv;
    }

    if (flags & PROBE_DETECT) {
        if (!IS_MSOC(pSmi))
        {
            SMI_ProbeDDC(pScrn, xf86GetEntityInfo(pScrn->entityList[0])->index);
        }
        LEAVE(TRUE);
    }

    if (pEnt->location.type != BUS_PCI 
#if XORG_VERSION_CURRENT < 10706000		
	|| pEnt->resources
#endif	
	) 
    {
        xfree(pEnt);
        SMI_FreeRec(pScrn);
        LEAVE(FALSE);
    }
    pSmi->PciInfo = xf86GetPciInfoForEntity(pEnt->index);

    /* Set pScrn->monitor */
    pScrn->monitor = pScrn->confScreen->monitor;

    if(IS_SM712(pSmi) || IS_SM750(pSmi) || IS_SM718(pSmi))
    {
        /* The vgahw module should be loaded here when needed */
        if (!xf86LoadSubModule(pScrn, "vgahw"))
            LEAVE(FALSE);

        /* Allocate a vgaHWRec */
        if (!vgaHWGetHWRec(pScrn))
            LEAVE(FALSE);

        hwp = VGAHWPTR(pScrn);
        pSmi->PIOBase = hwp->PIOOffset;

		XMSG("%s:vgaCRTCIndex=%x,vgaIOBase=%x,MMIOBase=%p",__func__,
				hwp->IOBase + VGA_CRTC_INDEX_OFFSET,
				hwp->IOBase,
				hwp->MMIOBase);
    }
	
    if(IS_SM750(pSmi)|| IS_SM718(pSmi))
    {    
        if (!xf86LoadSubModule(pScrn, "smi750ddk"))
            LEAVE(FALSE);
    }

#ifdef SMI_RECORD    
    record_start("//crt_sda.txt");
#endif

    /* The first thing we should figure out is the depth, bpp, etc.   */
    if (!xf86SetDepthBpp(pScrn, 0, 0, 0, Support32bppFb))
        LEAVE(FALSE);

    /* Check that the returned depth is one we support */
    if(pScrn->depth != 16 && pScrn->depth != 24)
    {
    	XERR("Given depth (%d) is not supported by this driver\n",pScrn->depth);
        LEAVE(FALSE);
    }

    if(pScrn->bitsPerPixel != 16 && pScrn->bitsPerPixel != 32)
    {
        XERR("Given bpp (%d) is not supported by this driver\n", pScrn->bitsPerPixel);
        LEAVE(FALSE);
    }

    xf86PrintDepthBpp(pScrn);

    pSmi->Bpp = pScrn->bitsPerPixel >> 3;

    /*
     * This must happen after pScrn->display has been set because
     * xf86SetWeight references it.
     */
    if (pScrn->depth > 8) {
        /* The defaults are OK for us */
        rgb zeros = {0, 0, 0};
#if __BYTE_ORDER == __BIG_ENDIAN
        rgb masks = {0xff00,0xff0000,0xff000000};
#else
        rgb masks = {0, 0, 0};
#endif

        if (!xf86SetWeight(pScrn, zeros, masks))
            LEAVE(FALSE);
    }

    if (!xf86SetDefaultVisual(pScrn, -1))
        LEAVE(FALSE);

    /* We don't currently support DirectColor at > 8bpp */
    if (pScrn->depth > 8 && pScrn->defaultVisual != TrueColor)
	{
		XERR("Given default visual (%s) is not supported at depth %d\n",
                xf86GetVisualName(pScrn->defaultVisual), pScrn->depth);
        LEAVE(FALSE);
    }

    /* We use a programmable clock */
    pScrn->progClock = TRUE;

    /* Collect all of the relevant option flags (fill in pScrn->options) */
    xf86CollectOptions(pScrn, NULL);


	/* not support 8 bit depth */
#if 0
    /* Set the bits per RGB for 8bpp mode */
    if (pScrn->depth == 8){
        pScrn->rgbBits = (IS_MSOC(pSmi) || IS_SM750(pSmi)|| IS_SM718(pSmi)) ? 8 : 6;
    }else if(pScrn->depth == 16){
        /* Use 8 bit LUT for gamma correction*/
        pScrn->rgbBits = 8;
    }
#endif

    /* Process the options */
    if (!(pSmi->Options = xalloc(sizeof(SMIOptions))))
        LEAVE(FALSE);

    memcpy(pSmi->Options, SMIOptions, sizeof(SMIOptions));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, pSmi->Options);


    /* Enable pci burst by default */

    pSmi->PCIBurst = TRUE;
    pSmi->DualView = TRUE;


    from = X_DEFAULT;
    if(xf86GetOptValBool(pSmi->Options,OPTION_PCI_BURST,&pSmi->PCIBurst))
        from = X_CONFIG;
    xf86DrvMsg(pScrn->scrnIndex, from, "??? PCI Burst %s abled\n",pSmi->PCIBurst ? "en" : "dis");


    /* Pci retry enabled by default if pci burst also enabled */
    from = X_DEFAULT;
    pSmi->PCIRetry = pSmi->PCIBurst ? TRUE : FALSE;
    if (xf86GetOptValBool(pSmi->Options, OPTION_PCI_RETRY, &pSmi->PCIRetry)) 
    {
        from = X_CONFIG;
        if (pSmi->PCIRetry && !pSmi->PCIBurst)
        {
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                    "\"pci_retry\" option requires \"pci_burst\".\n");
            pSmi->PCIRetry = FALSE;
        }
    }
    xf86DrvMsg(pScrn->scrnIndex, from, "PCI Retry %sabled\n",pSmi->PCIRetry ? "en" : "dis");


    from = X_DEFAULT;
    if(xf86GetOptValBool(pSmi->Options,OPTION_DUALVIEW,&pSmi->DualView))
        from = X_CONFIG;
    xf86DrvMsg(pScrn->scrnIndex, from, "Dual View %sabled\n",pSmi->DualView ? "en" : "dis");

    if(IS_SM712(pSmi) || IS_SM750(pSmi)|| IS_SM718(pSmi))
    {        
        pEntPriv->xlcd = pEntPriv->ylcd = 0;
        from = X_DEFAULT;
        if( xf86GetOptValInteger(pSmi->Options,OPTION_XLCD,&pEntPriv->xlcd)
            && xf86GetOptValInteger(pSmi->Options,OPTION_YLCD,&pEntPriv->ylcd))
        {
            from = X_CONFIG;
        }
        xf86DrvMsg(pScrn->scrnIndex, from, "Set panel size to %dx%d. 0x0 means no limitation\n",
                                            pEntPriv->xlcd,pEntPriv->ylcd);
        
        int tft = -1;
        pEntPriv->TFT = tft;
        from = X_DEFAULT;
        /* if option TFT_TYPE not defined in xorg.conf, below xf86Getxxxx will return 0 */
        if(xf86GetOptValInteger(pSmi->Options,OPTION_TFT_TYPE,&tft)){
            from = X_CONFIG;
        }        
        
        switch (tft)
        {
            case 18:
            pEntPriv->TFT = TFT_18BIT;
            break;                                
            case 36:
            pEntPriv->TFT = TFT_36BIT;
            break;
            case 12:
            pEntPriv->TFT = TFT_12BIT;
            break;
            case 9:
            pEntPriv->TFT = TFT_9BIT;
            break;                  
            case 24:
            pEntPriv->TFT = TFT_24BIT;                
            break;
            default:
            pEntPriv->TFT = TFT_24BIT;
        }            
        
        xf86DrvMsg(pScrn->scrnIndex, from, "Set TFT to %d bit.default value[24bit]\n",pEntPriv->TFT);        
    }
#if 0    
    else if(IS_SM750(pSmi))
    {
        from = X_DEFAULT;
        if( xf86GetOptValInteger(pSmi->Options,OPTION_XLCD,&SM750ENT(pSmi)->xlcd)
            && xf86GetOptValInteger(pSmi->Options,OPTION_YLCD,&SM750ENT(pSmi)->ylcd))
        {	
            from = X_CONFIG;		
        }	
        xf86DrvMsg(pScrn->scrnIndex, from, "Set panel size to %dx%d.0x0 means no limitation\n",
                                            SM750ENT(pSmi)->xlcd,SM750ENT(pSmi)->ylcd);

        from = X_DEFAULT;
        if(SM750ENT(pSmi)->xlcd * SM750ENT(pSmi)->ylcd)
        {
            XMSG("Set Lcd Expansion to %dx%d\n",SM750ENT(pSmi)->xlcd,SM750ENT(pSmi)->ylcd);
        }

        from = X_DEFAULT;	
        int tft = -1;        
        SM750ENT(pSmi)->TFT = -1;
        if(xf86GetOptValInteger(pSmi->Options,OPTION_TFT_TYPE,&tft))
        {
            from = X_CONFIG;
            switch(tft)
            {
                case 18:
                    SM750ENT(pSmi)->TFT = TFT_18BIT;
                    break;
                case 24:
                    SM750ENT(pSmi)->TFT = TFT_24BIT;
                    break;
                case 36:
                    SM750ENT(pSmi)->TFT = TFT_36BIT;
                    break;			
            }		
        }	
        xf86DrvMsg(pScrn->scrnIndex, from, "Set TFT to %d bit \n",tft);
    }
#endif

    if (xf86ReturnOptValBool(pSmi->Options, OPTION_NOACCEL, FALSE))
    {
        pSmi->NoAccel = TRUE;
        xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: NoAccel - Acceleration "
                "disabled\n");
    } else {
        pSmi->NoAccel = FALSE;
    }

    if (IS_MSOC(pSmi))
    {
        from = X_DEFAULT;
        if (xf86GetOptValBool(pSmi->Options, OPTION_USE_FBDEV, &pSmi->UseFBDev))
            from = X_CONFIG;
        xf86DrvMsg(pScrn->scrnIndex, from, "UseFBDev %s.\n",
                pSmi->UseFBDev ? "enabled" : "disabled");
    }

    from = X_CONFIG;
	pSmi->HwCursor = TRUE;//default setting
	if(IS_SM712(pSmi))
		pSmi->HwCursor = FALSE;
	
    /* SWCursor overrides HWCusor if both specified */
    if (xf86ReturnOptValBool(pSmi->Options, OPTION_SWCURSOR, FALSE))
    {
        pSmi->HwCursor = FALSE;
        from = X_CONFIG;
    }
    else if (xf86GetOptValBool(pSmi->Options, OPTION_HWCURSOR, &pSmi->HwCursor))
    {		
        from = X_DEFAULT;   			
    }
    else
    {        
		//pSmi->HwCursor = TRUE;
    }

    xf86DrvMsg(pScrn->scrnIndex, from, "Using %sware Cursor\n",pSmi->HwCursor ? "Hard" : "Soft");

    if(IS_SM750(pSmi)|| IS_SM718(pSmi))
    {
        pSmi->ARGBCursor = FALSE;
        from = X_DEFAULT;
        if(pSmi->HwCursor)
        {			
            pSmi->ARGBCursor = TRUE;
            if(xf86GetOptValBool(pSmi->Options,OPTION_ARGBCURSOR,&pSmi->ARGBCursor))
            {
                from = X_CONFIG;
            }
          
//            if(pSmi->DualView || (SM750ENT(pSmi)->xlcd > 0 && SM750ENT(pSmi)->ylcd > 0))
            if(pSmi->DualView || pSmi->entityPrivate->xlcd > 0 && pSmi->entityPrivate->ylcd > 0)
            {
                from = X_DEFAULT;
                pSmi->ARGBCursor = FALSE;
            }
            xf86DrvMsg(pScrn->scrnIndex, from, "%s ARGBCursor\n",pSmi->ARGBCursor?"Use":"Not use");
        }
    }

    if (xf86GetOptValInteger(pSmi->Options, OPTION_VIDEOKEY, &pSmi->videoKey)) {
        xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: Video key set to "
                "0x%08X\n", pSmi->videoKey);
    } else {
        pSmi->videoKey = (1 << pScrn->offset.red) |
            (1 << pScrn->offset.green) |
            (((pScrn->mask.blue >> pScrn->offset.blue) - 1)
             << pScrn->offset.blue);
    }

    if (xf86ReturnOptValBool(pSmi->Options, OPTION_BYTESWAP, FALSE)) {
        pSmi->ByteSwap = TRUE;
        xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: ByteSwap enabled.\n");
    } else {
        pSmi->ByteSwap = FALSE;
    }

    /* CZ 26.10.2001: interlaced video */
    if (xf86ReturnOptValBool(pSmi->Options, OPTION_INTERLACED, FALSE)) {
        pSmi->interlaced = TRUE;
        xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: Interlaced enabled.\n");
    } else {
        pSmi->interlaced = FALSE;
    }
    /* end CZ */

    if (IS_MSOC(pSmi))
    {
        pSmi->useBIOS = FALSE;
    }
    else if(IS_SM750(pSmi)|| IS_SM718(pSmi))
    {
        pSmi->useBIOS = FALSE; //Now using BIOS on sm750 there are some issues	
    }
    else if (xf86GetOptValBool(pSmi->Options, OPTION_USEBIOS, &pSmi->useBIOS))
    {
        xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: UseBIOS %s.\n",pSmi->useBIOS ? "enabled" : "disabled");        
    }
    else if(pSmi->Chipset == SMI_LYNX3DM)
    {
        /* Default to UseBIOS disabled. */
        pSmi->useBIOS = FALSE;
    }
    else
    {
        /* Default to UseBIOS disabled. */
        pSmi->useBIOS = FALSE;
    }	

    XMSG("Use Bios %sabled\n",pSmi->useBIOS?"en":"dis");

	
    if (pSmi->useBIOS)
    {
        
        if (xf86LoadSubModule(pScrn,"int10")) {
            pSmi->pInt10 = xf86InitInt10(pEnt->index);
        }

        if (pSmi->pInt10 && xf86LoadSubModule(pScrn, "vbe"))
		{
        //    pSmi->pVbe = VBEInit(pSmi->pInt10, pEnt->index);
			pSmi->pVbe = VBEExtendedInit(pSmi->pInt10,pEnt->index,
										SET_BIOS_SCRATCH| RESTORE_BIOS_SCRATCH);
			
        }

        if(!pSmi->pVbe){
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "VBE initialization failed: falling back to UseBIOS disabled.\n");
            pSmi->useBIOS = FALSE;
        }
#if 0
		if(!xf86LoadSubModule(pScrn,"shadow")){
			vbeFree(pSmi->pVbe);			
		}

		if(xf86LoadSubModule(pScrn,"fb")==NULL){
			vbeFree(pSmi->pVbe);
		}
#endif		
    }
#if XORG_VERSION_CURRENT < 10706000
    xf86RegisterResources(pEnt->index, NULL, ResExclusive);
#endif
#if 0    
    if(!IS_SM750(pSmi))
    {
        xf86SetOperatingState(resVgaIo, pEnt->index, ResUnusedOpr); 
        xf86SetOperatingState(resVgaMem, pEnt->index, ResDisableOpr);
    }
#endif

    /*  Set the Chipset and ChipRev, allowing config file entries to override.  */
   
    if (pEnt->device->chipset && *pEnt->device->chipset) {
        pScrn->chipset = pEnt->device->chipset;
        XMSG("pEnt->device->chipset = %s\n",*pEnt->device->chipset);
        pSmi->Chipset = xf86StringToToken(SMIChipsets, pScrn->chipset);
        from = X_CONFIG;
    }
    else if (pEnt->device->chipID >= 0) {
        pSmi->Chipset = pEnt->device->chipID;
        pScrn->chipset = (char *) xf86TokenToString(SMIChipsets, pSmi->Chipset);
        from = X_CONFIG;
        XCONF("ChipID override: 0x%04X\n",pSmi->Chipset);
    }
    else {
        from = X_PROBED;
        pSmi->Chipset = PCI_DEV_DEVICE_ID(pSmi->PciInfo);
        pScrn->chipset = (char *) xf86TokenToString(SMIChipsets, pSmi->Chipset);
    }

    if (pEnt->device->chipRev >= 0) {
        pSmi->ChipRev = pEnt->device->chipRev;
        XCONF("ChipRev override: %d\n",pSmi->ChipRev);
    }
    else
        pSmi->ChipRev = PCI_DEV_REVISION(pSmi->PciInfo);
	
    xfree(pEnt);

    /*
     * This shouldn't happen because such problems should be caught in
     * SMI_Probe(), but check it just in case.
     */
    if (pScrn->chipset == NULL) {
		XERR("ChipID 0x%04X is not recognised\n", pSmi->Chipset);
        LEAVE(FALSE);
    }

    if (pSmi->Chipset < 0) {
        XERR("Chipset \"%s\" is not recognised\n", pScrn->chipset);
        LEAVE(FALSE);
    }

    xf86DrvMsg(pScrn->scrnIndex, from, "Chipset: \"%s\"\n", pScrn->chipset);
	XOUT(from,"Chipset: \"%s\"\n", pScrn->chipset);

#ifndef XSERVER_LIBPCIACCESS
    pSmi->PciTag = pciTag(pSmi->PciInfo->bus, pSmi->PciInfo->device,pSmi->PciInfo->func);
#endif

    if (IS_MSOC(pSmi))
    {
        pSmi->lcd = TRUE;
        if (pSmi->DualView && pSmi->UseFBDev)
        {
            XINF("Dual head disabled in fbdev mode\n");
            pSmi->DualView = FALSE;
        }
        /*  FIXME Randr cursor code only works properly when argb cursors
                 * are also supported.
                 * FIXME This probably is a randr cursor bug, and since access to
                 * hw/xfree86/ramdac/xf86CursorPriv.h:xf86CursorScreenRec.SWCursor
                 * field is not available, one cannot easily workaround the problem,
                 * so, just disable it...
                 * TODO Check with a X Server newer then 1.4.0.90 (that is being
                 * used in the 502 OEM image).
                 *
                 */
        if (pSmi->DualView && pSmi->HwCursor) 
        {
            XINF("HW Cursor disabled in dual head mode\n");
            pSmi->HwCursor = FALSE;
        }
    }
    else if (IS_OLDLYNX(pSmi))
    {
        /* tweak options for dualhead */
        if (pSmi->DualView)
        {
            pSmi->useBIOS = FALSE;
            XINF("UseBIOS disabled in DualView mode\n");
            pSmi->HwCursor = FALSE;
            XINF("No hardware cursor in DualView mode\n");
            if (pScrn->bitsPerPixel != 16)
            {
                XERR("DualView only supported at depth 16\n");
                LEAVE(FALSE);
            }
        }
    }

    xf86DrvMsg(pScrn->scrnIndex, from, "Dual head %sabled\n",pSmi->DualView ? "en" : "dis");

    if (!pSmi->NoAccel)
    {
        char *strptr;
        from = X_DEFAULT;
        if ((strptr = (char *)xf86GetOptValString(pSmi->Options,OPTION_ACCELMETHOD)))
        {
            if (!xf86NameCmp(strptr,"XAA")) 
            {
                from = X_CONFIG;
                pSmi->useEXA = FALSE;
            }
            else if(!xf86NameCmp(strptr,"EXA")) 
            {
                from = X_CONFIG;
                pSmi->useEXA = TRUE;
            }
        }
        xf86DrvMsg(pScrn->scrnIndex, from, "Using [ %s ] acceleration architecture\n",
                pSmi->useEXA ? "EXA" : "XAA");
    }

    if (IS_MSOC(pSmi) || IS_SM750(pSmi)|| IS_SM718(pSmi))
    {
        pSmi->CSCVideo = !pSmi->useEXA || !pSmi->DualView;
        from = X_DEFAULT;
        if (xf86GetOptValBool(pSmi->Options, OPTION_CSCVIDEO, &pSmi->CSCVideo))
        {
            from = X_CONFIG;

            /* FIXME */
            if (pSmi->CSCVideo && pSmi->useEXA && pSmi->DualView) 
            {
                XMSG("CSCVideo requires XAA or EXA in single head mode.\n");
                pSmi->CSCVideo = FALSE;
            }
        }
        xf86DrvMsg(pScrn->scrnIndex, from, "CSC Video %sabled\n",pSmi->CSCVideo ? "en" : "dis");
    }

    SMI_MapMmio(pScrn);
    SMI_DetectMem(pScrn);
    SMI_MapMem(pScrn);
    SMI_DisableVideo(pScrn);

    /* detect the panel size */        
    SMI_DetectPanelSize(pScrn);

    if(IS_SM712(pSmi) || IS_SM750(pSmi))
    {
        if (xf86LoadSubModule(pScrn, "i2c")){
            //xf86LoaderReqSymLists(i2cSymbols, NULL);
            SMI_I2CInit(pScrn);
        }
        if(xf86LoadSubModule(pScrn, "ddc")){
            //xf86LoaderReqSymLists(ddcSymbols, NULL);            
        }
    }

    /*  If the driver can do gamma correction, it should call xf86SetGamma()   */
    {
        Gamma zeros = { 0.0, 0.0, 0.0 };

        if (!xf86SetGamma(pScrn, zeros)) {
            SMI_EnableVideo(pScrn);
            SMI_UnmapMem(pScrn);
            LEAVE(FALSE);
        }
    }

    SMI_DetectMCLK(pScrn);

    /*
     * Setup the ClockRanges, which describe what clock ranges are available,
     * and what sort of modes they can be used for.
     */
    pSmi->clockRange.next = NULL;
    pSmi->clockRange.minClock = 20000;

    if (IS_OLDLYNX(pSmi) ||	IS_MSOC(pSmi))
        pSmi->clockRange.maxClock = 200000;
    else if(IS_SM750(pSmi)|| IS_SM718(pSmi))
        pSmi->clockRange.maxClock = 193160;
    else 
        pSmi->clockRange.maxClock = 135000;

    pSmi->clockRange.clockIndex = -1;
    pSmi->clockRange.interlaceAllowed = FALSE;
    pSmi->clockRange.doubleScanAllowed = FALSE;

    if(!SMI_CrtcPreInit(pScrn))
        LEAVE(FALSE);

    if(!SMI_OutputPreInit(pScrn))
        LEAVE(FALSE);

    /* Only allow growing the screen dimensions if EXA is being used */
    if (!xf86InitialConfiguration (pScrn, !pSmi->NoAccel && pSmi->useEXA))
    {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No valid modes found\n");
        SMI_EnableVideo(pScrn);
        SMI_UnmapMem(pScrn);
        LEAVE(FALSE);
    }

    /* Ensure that the framebuffer size just set fits in video memory. */
    {
        int aligned_pitch = (pScrn->virtualX * pSmi->Bpp + 15) & ~15;
		XINF("aligned pitch = %d,pScrn->virtualX = %d,pSmi->Bpp = %d\n",aligned_pitch,
			pScrn->virtualX,pSmi->Bpp);
		
        XINF("pScrn->virtualX = %d \n",pScrn->virtualX);
        XINF("pSmi->Bpp = %d \n",pSmi->Bpp);		
        XINF("Aligned_pitch=%d pScrn->virtualY=%d and pSmi->FBReserved = %08x \n",
                aligned_pitch,pScrn->virtualY,pSmi->FBReserved);

        if(aligned_pitch * pScrn->virtualY > pSmi->FBReserved)
        {
            XERR("Not enough video memory for the configured screen size (%dx%dx%dBpp) and color depth.\n",
                    pScrn->virtualX, pScrn->virtualY,pSmi->Bpp);

            SMI_EnableVideo(pScrn);
            SMI_UnmapMem(pScrn);
            LEAVE(FALSE);
        }
    }


    SMI_EnableVideo(pScrn);

#if 1	
    SMI_UnmapMem(pScrn);

    if(pSmi->pVbe){
        vbeFree(pSmi->pVbe);
        pSmi->pVbe = NULL;
    }
    if(pSmi->pInt10){
        xf86FreeInt10(pSmi->pInt10);
        pSmi->pInt10 = NULL;
    }
#endif

    /* Set display resolution */
    xf86SetDpi(pScrn, 0, 0);

    if (xf86LoadSubModule(pScrn, "fb") == NULL) {
        SMI_FreeRec(pScrn);
        LEAVE(FALSE);
    }

    /* Load XAA or EXA if needed */
    if (!pSmi->NoAccel) {
        if (!pSmi->useEXA) {
            if (!xf86LoadSubModule(pScrn, "xaa")) {
                SMI_FreeRec(pScrn);
                LEAVE(FALSE);
            }
        } else {
            XF86ModReqInfo req;
            int errmaj, errmin;

            memset(&req, 0, sizeof(XF86ModReqInfo));
            req.majorversion = 2;
            req.minorversion = 1;

            if (!LoadSubModule(pScrn->module, "exa", NULL, NULL, NULL,
                        &req, &errmaj, &errmin)) {
                LoaderErrorMsg(NULL, "exa", errmaj, errmin);
                SMI_FreeRec(pScrn);
                LEAVE(FALSE);
            }
        }
    }

    /* Load ramdac if needed */
    if (pSmi->HwCursor) {
        if (!xf86LoadSubModule(pScrn, "ramdac")) {
            SMI_FreeRec(pScrn);
            LEAVE(FALSE);
        }
    }

    xf86SetPrimInitDone(pScrn->entityList[0]);
    LEAVE(TRUE);
}

/*
	return 0 means current register status is a mess
*/
static int mess_reg_status(ScrnInfoPtr pScrn)
{	
	/* only used by 712 now*/
	SMIPtr pSmi = SMIPTR(pScrn);
#if 0
	// shit, below code not workable, try another solution
	int ret = -1;
	switch (pSmi->Chipset){
	case 0x712:
		ret = sm712_mess_reg_status(pScrn);
		break;	
	}
	return ret;
#else
	CARD8 data = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x70);
	XERR("3c4:70 == %02x\n",data);
	return ((data & 0xf0) != 0x50);
#endif
}


/*
 * This is called when VT switching back to the X server.  Its job is to
 * reinitialise the video mode. We may wish to unmap video/MMIO memory too.
 */

static Bool SMI_EnterVT(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    SMIPtr pSmi = SMIPTR(pScrn);
    ENTER();  

	/* Enable MMIO and map memory */
    SMI_MapMem(pScrn);

    if(IS_SM750(pSmi)|| IS_SM718(pSmi))
	{    
		uint tmp;
        if((PEEK32(0x68)&0xff000000) == 0x55000000)
		{
            pSmi->Save(pScrn);
        }
		else
        {
        	/* maybe wake up from suspend , do not save registers */
            XMSG("MONK : Seems 750 register cleared by others \n");
			tmp = PEEK32(0x68) & 0x00ffffff;
			tmp |= 0x55000000;
            POKE32(0x68,tmp);
        }
    }
#if   0
	else if (IS_SM712(pSmi))
    {
		//bios used gpr70
    	CARD8 data;
		//if enter vt from suspend
    	if(!mess_reg_status(pScrn))
		{
			pSmi->Save(pScrn);
			XERR("MONK:SAVE REGISTERS\n");
		}
		else
		{	
			XERR("MONK:do not SAVE REGISTERS\n");
			data = VGAIN8_INDEX(pSmi,VGA_SEQ_INDEX,VGA_SEQ_DATA,0x70);
			data &= 0x0f;
			data |= 0x50;
			VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x70, data);		
		}		
    }
#endif
	else
	{    
        XERR("un-supported chip:%04x\n",pSmi->Chipset);
        LEAVE(FALSE);
    }

    /* FBBase may have changed after remapping the memory */
#if 1	
    pScrn->pScreen->ModifyPixmapHeader(pScrn->pScreen->GetScreenPixmap(pScrn->pScreen),
            -1,-1,-1,-1,-1, pSmi->FBBase + pSmi->FBOffset);
#else
	pScrn->pixmapPrivate.ptr = pSmi->FBBase + pSmi->FBOffset;
#endif
    pScrn->pixmapPrivate.ptr = pSmi->FBBase;

    if(pSmi->useEXA)
        pSmi->EXADriverPtr->memoryBase=pSmi->FBBase;

    /* Do the CRTC independent initialization */
    if(!SMI_HWInit(pScrn))
        LEAVE(FALSE);

    /* Initialize the chosen modes */
    if (!xf86SetDesiredModes(pScrn))
        LEAVE(FALSE);
        
    /* 2d engine re-reset */
    if(!pSmi->NoAccel)
    {
        if(IS_SM750(pSmi)|| IS_SM718(pSmi))
            deInit();
        SMI_EngineReset(pScrn);
    }
	
	//SMI_PrintRegs(pScrn);	

#if 0
    /* Reset the grapics engine */
    if (!pSmi->NoAccel) {
        deInit();
        SMI_EngineReset(pScrn);
    }
    //if (pSmi->NoAccel && pSmi->CSCVideo)
    {
        deInit();
    }
#endif    
    
#if 0
    /*
            FixMe if you wish:
            It's weired : below code makes X unpreticable when switch back to X. 
            some  times black screen (timing ok) and some times just show console 
            data(but not right & ugly).
            the most worst situation even causes network failed ...
            Seems 750 alpha layer registers can't be touched by free , it's very limited 
            So I use a member of pSmi->entityPrivate to tell ARGB cursor to re-init 
            the alpha layer when in needed
            Also weird that the init code in ARGB routine doesn't duplicate above bugs
    */

    /* reset the alpha layer if needed */
    if(pSmi->ARGBCursor)
    {	    

        init_argb_cursor(pSmi);    	
    }
#endif   
    LEAVE(TRUE);
}

/*
 * This is called when VT switching away from the X server.  Its job is to
 * restore the previous (text) mode. We may wish to remap video/MMIO memory
 * too.
 */

static void SMI_LeaveVT(int scrnIndex, int flags)
{
    ScrnInfoPtr	pScrn = xf86Screens[scrnIndex];
    SMIPtr	pSmi = SMIPTR(pScrn);

    ENTER();    
    SMI_AccelSync(pScrn);

    /* Ensure that the rotation BlockHandler is unwrapped, and the shadow
       pixmaps are deallocated, as the video memory is going to be
       unmapped.  */
    xf86RotateCloseScreen(pScrn->pScreen);

    /* Pixmaps that by chance get allocated near the former aperture
       address shouldn't be considered offscreen. */
    if(pSmi->useEXA)
        pSmi->EXADriverPtr->memoryBase=NULL;

    /* Clear frame buffer */
    memset(pSmi->FBBase, 0, pSmi->videoRAMBytes);

    if(IS_MSOC(pSmi)){
        SMI501_Restore(pScrn, pSmi->save);
    }
    else if(IS_SM750(pSmi)|| IS_SM718(pSmi)){
        SMI750_Restore(pScrn, pSmi->save);				
    }
    else if(IS_SM712(pSmi)){      
        SMILynx_Restore(pScrn, pSmi->save);
    }

    SMI_UnmapMem(pScrn);

    LEAVE();

}

    static void
SMI_DetectPanelSize(ScrnInfoPtr pScrn)
{
    char	*s;
    int		 width, height;
    SMIPtr	 pSmi = SMIPTR(pScrn);

    pSmi->lcdWidth  = 0;
    pSmi->lcdHeight = 0;
    if ((s = xf86GetOptValString(pSmi->Options, OPTION_PANEL_SIZE)) != NULL) {
        if (sscanf(s, "%dx%d", &width, &height) != 2)
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                    "Invalid PanelSize option: %s\n", s);
        else {
            pSmi->lcdWidth  = width;
            pSmi->lcdHeight = height;
        }
    }

    if (pSmi->lcdWidth == 0 || pSmi->lcdHeight == 0) {
        /* panel size detection ... requires BIOS call on 730 hardware */
        if (pSmi->Chipset == SMI_COUGAR3DR) {
            if (pSmi->pInt10 != NULL) {
                pSmi->pInt10->num = 0x10;
                pSmi->pInt10->ax  = 0x5F00;
                pSmi->pInt10->bx  = 0;
                pSmi->pInt10->cx  = 0;
                pSmi->pInt10->dx  = 0;
                xf86ExecX86int10(pSmi->pInt10);
                if (pSmi->pInt10->ax == 0x005F) {
                    switch (pSmi->pInt10->cx & 0x0F) {
                        case PANEL_640x480:
                            pSmi->lcdWidth  = 640;
                            pSmi->lcdHeight = 480;
                            break;
                        case PANEL_800x600:
                            pSmi->lcdWidth  = 800;
                            pSmi->lcdHeight = 600;
                            break;
                        case PANEL_1024x768:
                            pSmi->lcdWidth  = 1024;
                            pSmi->lcdHeight = 768;
                            break;
                        case PANEL_1280x1024:
                            pSmi->lcdWidth  = 1280;
                            pSmi->lcdHeight = 1024;
                            break;
                        case PANEL_1600x1200:
                            pSmi->lcdWidth  = 1600;
                            pSmi->lcdHeight = 1200;
                            break;
                        case PANEL_1400x1050:
                            pSmi->lcdWidth  = 1400;
                            pSmi->lcdHeight = 1050;
                            break;
                    }

                    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Detected panel size via BIOS: %d x %d\n",
                            pSmi->lcdWidth, pSmi->lcdHeight);
                }
                else
                    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "BIOS error during 730 panel detection!\n");
            }
            else  {
                /* int10 support isn't setup on the second call to this function,
                   o if this is the second call, don't do detection again */
                if (pSmi->lcd == 0)
                    /* If we get here, int10 support is not loaded or not working */ 
                    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "No BIOS support for 730 panel detection!\n");
            }

            /* Set this to indicate that we've done the detection */
            pSmi->lcd = 1;
        }
        else if (IS_MSOC(pSmi)) {
            pSmi->lcdWidth  = 0;//(READ_SCR(pSmi, PANEL_WWIDTH)  >> 16) & 2047;
            pSmi->lcdHeight = 0;//(READ_SCR(pSmi, PANEL_WHEIGHT) >> 16) & 2047;
        }
        else {
            /* panel size detection for hardware other than 730 */
            pSmi->lcd = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA,
                    0x31) & 0x01;

            if (VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA,
                        0x30) & 0x01) {
                pSmi->lcd <<= 1;
            }
            switch (VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA,
                        0x30) & 0x0C) {
                case 0x00:
                    pSmi->lcdWidth  = 640;
                    pSmi->lcdHeight = 480;
                    break;
                case 0x04:
                    pSmi->lcdWidth  = 800;
                    pSmi->lcdHeight = 600;
                    break;
                case 0x08:
                    if (VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA,
                                0x74) & 0x02) {
                        pSmi->lcdWidth  = 1024;
                        pSmi->lcdHeight = 600;
                    }
                    else {
                        pSmi->lcdWidth  = 1024;
                        pSmi->lcdHeight = 768;
                    }
                    break;
                case 0x0C:
                    pSmi->lcdWidth  = 1280;
                    pSmi->lcdHeight = 1024;
                    break;
            }
        }
    }

    if (!pSmi->lcdWidth && (pSmi->lcdWidth = pScrn->virtualX) == 0)
        pSmi->lcdWidth = 1024;
    if (!pSmi->lcdHeight && (pSmi->lcdHeight = pScrn->virtualY) == 0)
        pSmi->lcdHeight = 768;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "%s Panel Size = %dx%d\n",
            (pSmi->lcd == 0) ? "OFF" : (pSmi->lcd == 1) ? "TFT" : "DSTN",
            pSmi->lcdWidth, pSmi->lcdHeight);

}

static void SMI_DetectMCLK(ScrnInfoPtr pScrn)
{
    double		real;
    MSOCClockRec	clock;
    int			mclk, mxclk;
    SMIPtr		pSmi = SMIPTR(pScrn);

    /* MCLK defaults */
    if (pSmi->Chipset == SMI_LYNXEMplus)
    {
        /* The SM712 can be safely clocked up to 157MHz, according to Silicon Motion engineers. */
        pSmi->MCLK = 157000;
    }
    else if (IS_MSOC(pSmi))
    {
        /*  Set some sane defaults for the clock settings if we are on a
                    SM502 and it's likely to be uninitialized. 
                */

        if (!xf86IsPrimaryPci(pSmi->PciInfo) &&
                (READ_SCR(pSmi, DEVICE_ID) & 0xFF) >= 0xC0) {
            pSmi->MCLK = 112000;
            pSmi->MXCLK = 144000;
        }
    }
    else if(IS_SM750(pSmi)|| IS_SM718(pSmi))
    {
        pSmi->MCLK = 165000;
    }
    else
    {
        pSmi->MCLK = 0;
        pSmi->MXCLK = 0;
    }

    /* MCLK from user settings */
    if (xf86GetOptValFreq(pSmi->Options, OPTION_MCLK, OPTUNITS_MHZ, &real)) 
    {
        if (IS_MSOC(pSmi) || (int)real <= 120) {
            pSmi->MCLK = (int)(real * 1000.0);
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
            "Memory Clock %1.3f MHz larger than limit of 120 MHz\n",
            real);
        }
    }
    
    mclk = pSmi->MCLK;

    if (IS_MSOC(pSmi)) {
        clock.value = READ_SCR(pSmi, CURRENT_CLOCK);
        if (xf86GetOptValFreq(pSmi->Options, OPTION_MXCLK,
                    OPTUNITS_MHZ, &real))
            pSmi->MXCLK = (int)(real * 1000.0);
    }

    /* Already programmed MCLK */
    if (pSmi->MCLK == 0)
    {
        if (IS_MSOC(pSmi)){
            mclk = ((clock.f.m_select ? 336 : 288) /
                    ((clock.f.m_divider ? 3 : 1) <<
                     (unsigned)clock.f.m_shift)) * 1000;
        }else if(IS_SM712(pSmi)){
            unsigned char	shift, m, n;

            m = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6A);
            n = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6B);
            switch (n >> 6)
            {
                case 1:
                    shift = 4;
                    break;
                case 2:
                    shift = 2;
                    break;
                default:
                    shift = 1;
                    break;
            }
            n &= 0x3F;
            mclk = ((1431818 * m) / n / shift + 50) / 100;
        }
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "MCLK = %1.3f\n", mclk / 1000.0);
    if (IS_MSOC(pSmi)) 
    {
        if (pSmi->MXCLK == 0)
        {
            mxclk = ((clock.f.m1_select ? 336 : 288) /
                    ((clock.f.m1_divider ? 3 : 1) <<
                     (unsigned)clock.f.m1_shift)) * 1000;
        }
        else if(IS_SM712(pSmi))
        {
            mxclk = pSmi->MXCLK;
        }
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "MXCLK = %1.3f\n", mxclk / 1000.0);
    }
}



#if 1
static Bool
SMI_MapMmio(ScrnInfoPtr pScrn)
{	
    SMIPtr	pSmi = SMIPTR(pScrn);
    SMIEntPtr  pEntPriv = pSmi->entityPrivate;
    ulong memBase;
    //unsigned char ** ppEntMMIO = NULL;

    ENTER();
    XMSG("MONK TEST : pScrn->chipID = %04x ; pScrn->ChipRev = %04x \n",pScrn->chipID,pScrn->chipRev);

    SMI_EnableMmio(pScrn);

    switch (pSmi->Chipset) {
        case SMI_COUGAR3DR:
            memBase = PCI_REGION_BASE(pSmi->PciInfo, 1, REGION_MEM);
            pSmi->MapSize = 0x200000;
            break;
        case SMI_LYNX3D:
            memBase = PCI_REGION_BASE(pSmi->PciInfo, 0, REGION_MEM) + 0x680000;
            pSmi->MapSize = 0x180000;
            break;
        case SMI_LYNXEM:
        case SMI_LYNXEMplus:
            memBase = PCI_REGION_BASE(pSmi->PciInfo, 0, REGION_MEM) + 0x400000;
            pSmi->MapSize = 0x400000;
           
            break;
        case SMI_LYNX3DM:
            memBase = PCI_REGION_BASE(pSmi->PciInfo, 0, REGION_MEM);
            pSmi->MapSize = 0x200000;
            break;
        case SMI_MSOC:
        case SMI_750:
		case SMI_718:
            memBase = PCI_REGION_BASE(pSmi->PciInfo, 1, REGION_MEM);
            pSmi->MapSize = 0x200000;
           
            break;
        default:
            memBase = PCI_REGION_BASE(pSmi->PciInfo, 0, REGION_MEM) + 0x400000;
            pSmi->MapSize = 0x10000;
            break;
    }

#ifndef XSERVER_LIBPCIACCESS
    pSmi->MapBase = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_MMIO, pSmi->PciTag,memBase, pSmi->MapSize);
#else   
    if(!pEntPriv->mmio)
    {
        XMSG("%s,It's time to map mmio\n",pScrn->chipset);
        void **result = (void**)&pSmi->MapBase;
        int err = pci_device_map_range(pSmi->PciInfo,
                memBase,
                pSmi->MapSize,
                PCI_DEV_MAP_FLAG_WRITABLE,
                result);

        if(err)
            return (FALSE);		
        pEntPriv->mmio = *result;
    }
    else
    {
        XMSG("%s,mmio already mapped \n",pScrn->chipset);       
        pSmi->MapBase = pEntPriv->mmio;
    }	
#endif

    if (pSmi->MapBase == NULL) 
    {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Internal error: could not map "
        "MMIO registers.\n");
        return (FALSE);
    }
    
    if(IS_SM750(pSmi)|| IS_SM718(pSmi)) 
        setMmioBase((unsigned long)pSmi->MapBase);//For SM750 only

    switch (pSmi->Chipset) 
    {
        case SMI_COUGAR3DR:
            pSmi->DPRBase = pSmi->MapBase + 0x000000;
            pSmi->VPRBase = pSmi->MapBase + 0x000800;
            pSmi->CPRBase = pSmi->MapBase + 0x001000;
            pSmi->FPRBase = pSmi->MapBase + 0x005800;
            pSmi->IOBase  = pSmi->MapBase + 0x0C0000;
            pSmi->DataPortBase = pSmi->MapBase + 0x100000;
            pSmi->DataPortSize = 0x100000;
            break;
        case SMI_LYNX3D:
            pSmi->DPRBase = pSmi->MapBase + 0x000000;
            pSmi->VPRBase = pSmi->MapBase + 0x000800;
            pSmi->CPRBase = pSmi->MapBase + 0x001000;
            pSmi->IOBase  = pSmi->MapBase + 0x040000;
            pSmi->DataPortBase = pSmi->MapBase + 0x080000;
            pSmi->DataPortSize = 0x100000;
            break;
        case SMI_LYNXEM:
        case SMI_LYNXEMplus:
            pSmi->DPRBase = pSmi->MapBase + 0x008000;
            pSmi->VPRBase = pSmi->MapBase + 0x00C000;
            pSmi->CPRBase = pSmi->MapBase + 0x00E000;
            pSmi->IOBase  = pSmi->MapBase + 0x300000;
            pSmi->DataPortBase = pSmi->MapBase + 0x100000;
            pSmi->DataPortSize = 0x200000;
            break;
        case SMI_LYNX3DM:
            pSmi->DPRBase = pSmi->MapBase + 0x000000;
            pSmi->VPRBase = pSmi->MapBase + 0x000800;
            pSmi->CPRBase = pSmi->MapBase + 0x001000;
            pSmi->IOBase  = pSmi->MapBase + 0x0C0000;
            pSmi->DataPortBase = pSmi->MapBase + 0x100000;
            pSmi->DataPortSize = 0x100000;
            break;
        case SMI_MSOC:
        case SMI_750:
		case SMI_718:
            pSmi->DPRBase = pSmi->MapBase + 0x100000;
            pSmi->VPRBase = pSmi->MapBase + 0x000000;
            pSmi->CPRBase = pSmi->MapBase + 0x090000;
            pSmi->DCRBase = pSmi->MapBase + 0x080000;
            pSmi->SCRBase = pSmi->MapBase + 0x000000;
            pSmi->IOBase = 0;
            pSmi->DataPortBase = pSmi->MapBase + 0x110000;
            pSmi->DataPortSize = 0x10000;
            break;
        default:
            pSmi->DPRBase = pSmi->MapBase + 0x8000;
            pSmi->VPRBase = pSmi->MapBase + 0xC000;
            pSmi->CPRBase = pSmi->MapBase + 0xE000;
            pSmi->IOBase  = NULL;
            pSmi->DataPortBase = pSmi->MapBase;
            pSmi->DataPortSize = 0x8000;
            break;
    }

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, VERBLEV,
            "Physical MMIO at 0x%p \n", (ulong)memBase);
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, VERBLEV,
            "Logical MMIO at %p - %p\n", pSmi->MapBase,
            pSmi->MapBase + pSmi->MapSize - 1);
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, VERBLEV,
            "DPR=%p, VPR=%p, IOBase=%p\n",
            pSmi->DPRBase, pSmi->VPRBase, pSmi->IOBase);
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, VERBLEV,
            "DataPort=%p - %p\n", pSmi->DataPortBase,
            pSmi->DataPortBase + pSmi->DataPortSize - 1);

    LEAVE(TRUE);
}
#else
static Bool
SMI_MapMmio(ScrnInfoPtr pScrn)
{	
    SMIPtr	pSmi = SMIPTR(pScrn);
//    SMI750EntPtr p750Ent = pSmi->entityPrivate;
    void * pEntPriv = pSmi->entityPrivate;
//    CARD64	memBase;
    ulong memBase;
    unsigned char ** ppEntMMIO = NULL;

    ENTER();

    SMI_EnableMmio(pScrn);

    switch (pSmi->Chipset) {
        case SMI_COUGAR3DR:
            memBase = PCI_REGION_BASE(pSmi->PciInfo, 1, REGION_MEM);
            pSmi->MapSize = 0x200000;
            break;
        case SMI_LYNX3D:
            memBase = PCI_REGION_BASE(pSmi->PciInfo, 0, REGION_MEM) + 0x680000;
            pSmi->MapSize = 0x180000;
            break;
        case SMI_LYNXEM:
        case SMI_LYNXEMplus:
            memBase = PCI_REGION_BASE(pSmi->PciInfo, 0, REGION_MEM) + 0x400000;
            pSmi->MapSize = 0x400000;
            ppEntMMIO = &CAST_2_712ENT(pEntPriv)->mmio;
            break;
        case SMI_LYNX3DM:
            memBase = PCI_REGION_BASE(pSmi->PciInfo, 0, REGION_MEM);
            pSmi->MapSize = 0x200000;
            break;
        case SMI_MSOC:
        case SMI_750:
            memBase = PCI_REGION_BASE(pSmi->PciInfo, 1, REGION_MEM);
            pSmi->MapSize = 0x200000;
            ppEntMMIO = &CAST_2_750ENT(pEntPriv)->mmio;
            break;
        default:
            memBase = PCI_REGION_BASE(pSmi->PciInfo, 0, REGION_MEM) + 0x400000;
            pSmi->MapSize = 0x10000;
            break;
    }

#ifndef XSERVER_LIBPCIACCESS
    pSmi->MapBase = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_MMIO, pSmi->PciTag,memBase, pSmi->MapSize);
#else
    if(IS_SM750(pSmi) && !p750Ent->mmio)
    {
        XMSG("Monk : %s,First time map mmio\n",pScrn->chipset);
        void **result = (void**)&pSmi->MapBase;
        int err = pci_device_map_range(pSmi->PciInfo,
                memBase,
                pSmi->MapSize,
                PCI_DEV_MAP_FLAG_WRITABLE,
                result);

        if(err)
            return (FALSE);		
        p750Ent->mmio = pSmi->MapBase;       
    }
    else
    {	
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "MONK:NOT First time map mmio !! \n");
        pSmi->MapBase = p750Ent->mmio;        
    }	
#endif

    if (pSmi->MapBase == NULL) 
    {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Internal error: could not map "
                "MMIO registers.\n");
        return (FALSE);
    }
    //For SM750
    if(IS_SM750(pSmi)) 
        setMmioBase((unsigned long)pSmi->MapBase);

    switch (pSmi->Chipset) {
        case SMI_COUGAR3DR:
            pSmi->DPRBase = pSmi->MapBase + 0x000000;
            pSmi->VPRBase = pSmi->MapBase + 0x000800;
            pSmi->CPRBase = pSmi->MapBase + 0x001000;
            pSmi->FPRBase = pSmi->MapBase + 0x005800;
            pSmi->IOBase  = pSmi->MapBase + 0x0C0000;
            pSmi->DataPortBase = pSmi->MapBase + 0x100000;
            pSmi->DataPortSize = 0x100000;
            break;
        case SMI_LYNX3D:
            pSmi->DPRBase = pSmi->MapBase + 0x000000;
            pSmi->VPRBase = pSmi->MapBase + 0x000800;
            pSmi->CPRBase = pSmi->MapBase + 0x001000;
            pSmi->IOBase  = pSmi->MapBase + 0x040000;
            pSmi->DataPortBase = pSmi->MapBase + 0x080000;
            pSmi->DataPortSize = 0x100000;
            break;
        case SMI_LYNXEM:
        case SMI_LYNXEMplus:
            pSmi->DPRBase = pSmi->MapBase + 0x008000;
            pSmi->VPRBase = pSmi->MapBase + 0x00C000;
            pSmi->CPRBase = pSmi->MapBase + 0x00E000;
            pSmi->IOBase  = pSmi->MapBase + 0x300000;
            pSmi->DataPortBase = pSmi->MapBase + 0x100000;
            pSmi->DataPortSize = 0x200000;
            break;
        case SMI_LYNX3DM:
            pSmi->DPRBase = pSmi->MapBase + 0x000000;
            pSmi->VPRBase = pSmi->MapBase + 0x000800;
            pSmi->CPRBase = pSmi->MapBase + 0x001000;
            pSmi->IOBase  = pSmi->MapBase + 0x0C0000;
            pSmi->DataPortBase = pSmi->MapBase + 0x100000;
            pSmi->DataPortSize = 0x100000;
            break;
        case SMI_MSOC:
        case SMI_750:
            pSmi->DPRBase = pSmi->MapBase + 0x100000;
            pSmi->VPRBase = pSmi->MapBase + 0x000000;
            pSmi->CPRBase = pSmi->MapBase + 0x090000;
            pSmi->DCRBase = pSmi->MapBase + 0x080000;
            pSmi->SCRBase = pSmi->MapBase + 0x000000;
            pSmi->IOBase = 0;
            pSmi->DataPortBase = pSmi->MapBase + 0x110000;
            pSmi->DataPortSize = 0x10000;
            break;
        default:
            pSmi->DPRBase = pSmi->MapBase + 0x8000;
            pSmi->VPRBase = pSmi->MapBase + 0xC000;
            pSmi->CPRBase = pSmi->MapBase + 0xE000;
            pSmi->IOBase  = NULL;
            pSmi->DataPortBase = pSmi->MapBase;
            pSmi->DataPortSize = 0x8000;
            break;
    }

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, VERBLEV,
            "Physical MMIO at 0x%08lX\n", (uint32_t)memBase);
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, VERBLEV,
            "Logical MMIO at %p - %p\n", pSmi->MapBase,
            pSmi->MapBase + pSmi->MapSize - 1);
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, VERBLEV,
            "DPR=%p, VPR=%p, IOBase=%p\n",
            pSmi->DPRBase, pSmi->VPRBase, pSmi->IOBase);
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, VERBLEV,
            "DataPort=%p - %p\n", pSmi->DataPortBase,
            pSmi->DataPortBase + pSmi->DataPortSize - 1);

    LEAVE(TRUE);
}
#endif
/* HACK - In some cases the BIOS hasn't filled in the "scratchpad
   registers" (SR71) with the right amount of memory installed (e.g. MIPS
   platform). Probe it manually. */
    static uint32_t
SMI_ProbeMem(ScrnInfoPtr pScrn, uint32_t mem_skip, uint32_t mem_max)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    uint32_t mem_probe = 1024*1024;
    uint32_t aperture_base;
    void* mem;

    ENTER();

    aperture_base = PCI_REGION_BASE(pSmi->PciInfo, 0, REGION_MEM) + mem_skip;
    mem_max = min(mem_max , PCI_REGION_SIZE(pSmi->PciInfo, 0) - mem_skip);

#ifndef XSERVER_LIBPCIACCESS
    mem = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_MMIO, pSmi->PciTag,
            aperture_base, mem_max);

    if(!mem)
        LEAVE(0);
#else
    if(pci_device_map_range(pSmi->PciInfo, aperture_base, mem_max,
                PCI_DEV_MAP_FLAG_WRITABLE, &mem))
        LEAVE(0);
#endif

    while(mem_probe <= mem_max){
        MMIO_OUT32(mem, mem_probe-4, 0x55555555);
        if(MMIO_IN32(mem, mem_probe-4) != 0x55555555)
            break;

        MMIO_OUT32(mem, mem_probe-4, 0xAAAAAAAA);
        if(MMIO_IN32(mem, mem_probe-4) != 0xAAAAAAAA)
            break;

        mem_probe <<= 1;
    }

#ifndef XSERVER_LIBPCIACCESS
    xf86UnMapVidMem(pScrn->scrnIndex, mem, mem_max);
#else
    pci_device_unmap_range(pSmi->PciInfo, mem, mem_max);
#endif

    LEAVE(mem_probe >> 1);
}


#if 1
static Bool
SMI_DetectMem(ScrnInfoPtr pScrn)
{
    SMIPtr	pSmi = SMIPTR(pScrn);
    SMIEntPtr pEntPriv = pSmi->entityPrivate;
    MessageType from;
    
    ENTER();

    switch (pSmi->Chipset)
    {
        case SMI_LYNXEMplus:
            //config = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x71);           
            pSmi->videoRAMBytes = pEntPriv->totalVideoRam = (SMI_ProbeMem(pScrn, 0, 0x400000) / 1024)*1024;
            //pSmi->videoRAMBytes = CAST_2_712ENT(pEntPriv)->totalVideoRam = 4*1024*1024;
            break;
        case SMI_750:
		case SMI_718:
            //pSmi->videoRAMBytes = CAST_2_750ENT(pEntPriv)->totalVideoRam = getFrameBufSize();//CALL DDK             
            pSmi->videoRAMBytes = pEntPriv->totalVideoRam = getFrameBufSize();
            break;
    }

    pSmi->videoRAMKBytes = pSmi->videoRAMBytes >> 10;

    pScrn->videoRam = pSmi->videoRAMKBytes;

    pScrn->confScreen->device->videoRam = pScrn->videoRam;

    XMSG("pSmi->videoRAMBytes = %08x \n",pSmi->videoRAMBytes);

    LEAVE(pSmi->videoRAMBytes != 0);  
}
#else
static Bool
SMI_DetectMem(ScrnInfoPtr pScrn)
{
    SMIPtr	pSmi = SMIPTR(pScrn);
    SMI750EntPtr p750Ent = pSmi->entityPrivate;

    MessageType from;
    ENTER();
    //if ((pScrn->videoRam = pScrn->confScreen->device->videoRam))
    if(IS_SM750(pSmi) && p750Ent->totoalVideoRam != 0)
    {

    }
    else 
    {
        /*	first time detect memory*/
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,"MONK: first time %s \n",__func__);
        unsigned char	 config;
        static int	 lynx3d_table[4]  = {  0, 2, 4, 6 };
        static int	 lynx3dm_table[4] = { 16, 2, 4, 8 };
        static int	 msoc_table[8]    = {  4, 8, 16, 32, 64, 2, 0, 0 };
        static int	 default_table[4] = {  1, 2, 4, 0 };

        if (IS_MSOC(pSmi)) 
        {
            config = (READ_SCR(pSmi, DRAM_CTL) >> 13) & 7;
            pSmi->videoRAMBytes = (msoc_table[config] * 1024 - SHARED_USB_DMA_BUFFER_SIZE) * 1024;
        }		
        else if(IS_SM750(pSmi))
        {	
            pSmi->videoRAMBytes = p750Ent->totoalVideoRam =  getFrameBufSize();
        }
        else
        {
            config = VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x71);
            switch (pSmi->Chipset) 
            {
                case SMI_LYNX3D:
                    pSmi->videoRAMBytes = (lynx3d_table[config >> 6] * 1024 + 512)*1024;
                    break;
                case SMI_LYNXEMplus:
                    pSmi->videoRAMBytes = (SMI_ProbeMem(pScrn, 0, 0x400000) / 1024)*1024;
                    break;
                case SMI_LYNX3DM:
                    pSmi->videoRAMBytes = (lynx3dm_table[config >> 6] * 1024)*1024;
                    break;
                case SMI_COUGAR3DR:
                    /*
                       DANGER - Cougar3DR BIOS is broken - hardcode video ram
                       size per instructions from Silicon Motion engineers 
                       */
                    pSmi->videoRAMBytes = (16 * 1024)*1024;
                    break;
                default:
                    pSmi->videoRAMBytes = (default_table[config >> 6] * 1024)*1024;
                    break;
            }
        }
        from = X_PROBED;
    }


    pScrn->videoRam = pSmi->videoRAMKBytes = pSmi->videoRAMBytes >> 10;
    pScrn->confScreen->device->videoRam = pScrn->videoRam;
    xf86DrvMsg(pScrn->scrnIndex, from,"pSmi->videoRAMKBytes=%d kB and pScrn->videoRam = %d KB\n",
            pSmi->videoRAMKBytes,pScrn->videoRam);

    LEAVE(TRUE);
}
#endif

Bool
SMI_MapMem(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    vgaHWPtr hwp;

    ENTER();

    if (pSmi->MapBase == NULL && SMI_MapMmio(pScrn) == FALSE)
        LEAVE(FALSE);

    pScrn->memPhysBase = PCI_REGION_BASE(pSmi->PciInfo, 0, REGION_MEM);
    pSmi->fbMapOffset = 0;
    switch (pSmi->Chipset)
    {
#if 0   /* not support 72x and 710 by now */
        case SMI_LYNX3DM:
            pSmi->fbMapOffset = MB(2);
            break;
        case SMI_LYNXEM:
#endif
        case SMI_LYNXEMplus:
        case SMI_MSOC:/* 501 is not supported yet,but remains here so far */
        case SMI_750:
	    case SMI_718:
        default:
            pSmi->fbMapOffset = 0;
            break;
    }

#ifndef XSERVER_LIBPCIACCESS
    pSmi->FBBase = xf86MapPciMem(pScrn->scrnIndex,VIDMEM_FRAMEBUFFER,
            pSmi->PciTag,
            pScrn->memPhysBase + pSmi->fbMapOffset,
            pSmi->videoRAMBytes);
#else
    {
        void	**result = (void**)&pSmi->FBBase;
        int	  err = pci_device_map_range(pSmi->PciInfo,
                pScrn->memPhysBase + pSmi->fbMapOffset,
                pSmi->videoRAMBytes,
                PCI_DEV_MAP_FLAG_WRITABLE | PCI_DEV_MAP_FLAG_WRITE_COMBINE,
                result);

        if (err)
            LEAVE(FALSE);
    }
#endif

    /* Patch for MTRR issues with vesafb */
    SMI_FixMTRR(pScrn);

    if (pSmi->FBBase == NULL) 
    {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,"Internal error: could not map framebuffer.\n");
        LEAVE(FALSE);
    }

    if(IS_SM750(pSmi)|| IS_SM718(pSmi))
        setMemoryBase((uint32_t)pSmi->FBBase);

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, VERBLEV,
            "Physical frame buffer at 0x%08lX offset: 0x%08lX\n",
            pScrn->memPhysBase, pSmi->fbMapOffset);
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, VERBLEV,
            "Logical frame buffer at %p - %p\n", pSmi->FBBase,
            pSmi->FBBase + pSmi->videoRAMBytes - 1);

    if (IS_MSOC(pSmi) || IS_SM750(pSmi)|| IS_SM718(pSmi))
    {
        /* Reserve space for panel cursr, and crt if in DUAL VIEW mode */
        pSmi->FBReserved = pSmi->videoRAMBytes - 
        SMI750_ARGB_CURSOR_SIZE * 2 - SMI501_CURSOR_SIZE * 2;
        
        pSmi->fb_argbCursorOffset = pSmi->FBReserved + SMI750_ARGB_CURSOR_SIZE;
        pSmi->fb_priCursorOffset = pSmi->fb_argbCursorOffset + SMI750_ARGB_CURSOR_SIZE;
        pSmi->fb_secCursorOffset = pSmi->fb_priCursorOffset + SMI501_CURSOR_SIZE;

        DEBUG("pSmi->FBReserved [alpha cursor backup] = %08x \n",pSmi->FBReserved);
        DEBUG("pSmi->fb_argbCursorOffset = %08x \n",pSmi->fb_argbCursorOffset);
        DEBUG("pSmi->fb_priCursorOffset = %08x \n",pSmi->fb_priCursorOffset);
        DEBUG("pSmi->fb_secCursorOffset = %08x \n",pSmi->fb_secCursorOffset);

#if 1	
		if(IS_SM750(pSmi) || IS_SM718(pSmi))
		{
			/* Assign hwp->MemBase & IOBase here */
	        hwp = VGAHWPTR(pScrn);
	        if (pSmi->IOBase != NULL)
	            vgaHWSetMmioFuncs(hwp, pSmi->MapBase, pSmi->IOBase - pSmi->MapBase);
	            
	        vgaHWGetIOBase(hwp);
				
	        /* Map the VGA memory when the primary video */
	        if (xf86IsPrimaryPci(pSmi->PciInfo))
	        {
	            hwp->MapSize = 0x10000;
	            if (!vgaHWMapMem(pScrn))
	                LEAVE(FALSE);
	            pSmi->PrimaryVidMapped = TRUE;
	        }

			XMSG("hwp->base = %p\n",hwp->Base);
		}
#endif		
    }
    else if(IS_SM712(pSmi))
    { 
        /* set up the fifo reserved space */       
        if (VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x30) & 0x01)
        {    /*  if DSTN panel selected */
            /* #1074 */ 
            CARD32 fifoOffset = 0;
            fifoOffset |= VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA,
                    0x46) << 3;
            fifoOffset |= VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA,
                    0x47) << 11;
            fifoOffset |= (VGAIN8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA,
                        0x49) & 0x1C) << 17;
            pSmi->FBReserved = fifoOffset;	/* PDR#1074 */
        }
        else    /*  if TFT panel selected */
        {
            /*    sm712 only handle 4 mega video memory which means 
                            hardware cursor is not suitable for this chip of new arch x server driver  */       
#if 0                            
            if(!pSmi->DualView){
                pSmi->FBReserved = pSmi->videoRAMBytes - 2048;
                pSmi->FBCursorOffset = pSmi->FBReserved;//invalid value for cursor offset
            }else{
             /*     HWC only enabled in  SINGLE VIEW because 712 only equipped hwc layer on vga channel     */
                pSmi->FBReserved = pSmi->videoRAMBytes;
                pSmi->FBCursorOffset = 0;//invalid value for cursor offset
            }
#else
            pSmi->FBReserved = pSmi->videoRAMBytes;
#endif
            
        }

        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Cursor Offset: %08lX. 0 means not use hw cursor\n",
                (uint32_t)pSmi->FBCursorOffset);

        /* Assign hwp->MemBase & IOBase here */
        hwp = VGAHWPTR(pScrn);
        if (pSmi->IOBase != NULL)
            vgaHWSetMmioFuncs(hwp, pSmi->MapBase, pSmi->IOBase - pSmi->MapBase);
            
        vgaHWGetIOBase(hwp);

        /* Map the VGA memory when the primary video */
        if (xf86IsPrimaryPci(pSmi->PciInfo))
        {
            hwp->MapSize = 0x10000;
            if (!vgaHWMapMem(pScrn))
                LEAVE(FALSE);
            pSmi->PrimaryVidMapped = TRUE;
        }

        XMSG("pSmi->FBReserved = %x\n",pSmi->FBReserved);
    }
    else
    {
        XMSG("not supported chip\n");
        LEAVE(FALSE);    
    }
  
    LEAVE(TRUE);
}

#if 1

void
SMI_UnmapMem(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);    
    ENTER();

    /* Unmap VGA mem if mapped. */
    if(IS_SM712(pSmi) || IS_SM750(pSmi)|| IS_SM718(pSmi))
    {
        if(pSmi->Primary && pSmi->PrimaryVidMapped)
        {
            vgaHWUnmapMem(pScrn);
            pSmi->PrimaryVidMapped = FALSE;
            SMI_DisableMmio(pScrn);
        }
    }

    /*  unMap memory mapped register  */
    if(pSmi->MapBase)
    {
#ifndef XSERVER_LIBPCIACCESS
        xf86UnMapVidMem(pScrn->scrnIndex, (pointer)pSmi->MapBase,pSmi->MapSize);
#else		   
        pci_device_unmap_range(pSmi->PciInfo,(pointer)pSmi->MapBase,pSmi->MapSize);
#endif

        pSmi->MapBase = NULL;
        pSmi->MapSize = 0;        
        /* clear related register pointer */
        pSmi->DPRBase = NULL;
        pSmi->VPRBase = NULL;
        pSmi->CPRBase = NULL;
        pSmi->IOBase  = NULL;
        pSmi->DataPortBase = NULL;
        pSmi->DataPortSize = 0;
        pSmi->DCRBase = NULL;
        pSmi->SCRBase = NULL;       
    }

    /*	un-map frame buffer 	*/
    if (pSmi->FBBase)
    {
#ifndef XSERVER_LIBPCIACCESS
        xf86UnMapVidMem(pScrn->scrnIndex, (pointer)pSmi->FBBase,pSmi->videoRAMBytes);
#else
        pci_device_unmap_range(pSmi->PciInfo, (pointer)pSmi->FBBase,pSmi->videoRAMBytes);
#endif
        pSmi->FBBase = NULL;           
    }

    /* take care of private sector  */        
    pSmi->entityPrivate->mmio = NULL;
    switch (pSmi->Chipset)
    {
        case SMI_LYNXEMplus:            
            break;
        case SMI_750:           
		case SMI_718:
            setMemoryBase(0);
            break;        
    }

    LEAVE();
}


#else

/* UnMapMem - contains half of pre-4.0 EnterLeave function.  The EnterLeave
 * function which en/disable access to IO ports and ext. regs
 */

void
SMI_UnmapMem(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    SMI750EntPtr p750Ent = pSmi->entityPrivate;

    ENTER();

    /* Unmap VGA mem if mapped. */
    if (!IS_SM750(pSmi) && !IS_MSOC(pSmi)&& pSmi->PrimaryVidMapped)
    {
        vgaHWUnmapMem(pScrn);
        pSmi->PrimaryVidMapped = FALSE;
        SMI_DisableMmio(pScrn);	//move here for all other chips except 501 and 750
    }

    if(IS_SM750(pSmi))
    {   
        if(p750Ent->mmio)
        {
#ifndef XSERVER_LIBPCIACCESS
            xf86UnMapVidMem(pScrn->scrnIndex, (pointer)p750Ent->mmio,pSmi->MapSize);
#else		   
            pci_device_unmap_range(pSmi->PciInfo, (pointer)p750Ent->mmio,pSmi->MapSize);
#endif
            p750Ent->mmio = NULL;
            setMmioBase(0);
        }
        pSmi->MapBase = NULL;
    }
    else if (pSmi->MapBase) 
    {
        /*	if is not 750 and not un-mapped	*/
#ifndef XSERVER_LIBPCIACCESS
        xf86UnMapVidMem(pScrn->scrnIndex, (pointer)pSmi->MapBase,
                pSmi->MapSize);
#else
        pci_device_unmap_range(pSmi->PciInfo, (pointer)pSmi->MapBase,pSmi->MapSize);
        //pci_device_unmap_range(pSmi->PciInfo, (pointer)p750Ent->mmio,pSmi->MapSize);
#endif
        pSmi->MapBase = NULL;
    }
    /*	un-map frame buffer 	*/
    if (pSmi->FBBase)
    {
#ifndef XSERVER_LIBPCIACCESS
        xf86UnMapVidMem(pScrn->scrnIndex, (pointer) pSmi->FBBase,
                pSmi->videoRAMBytes);
#else
        pci_device_unmap_range(pSmi->PciInfo, (pointer)pSmi->FBBase,
                pSmi->videoRAMBytes);
#endif
        pSmi->FBBase = NULL;		
        if(IS_SM750(pSmi))
            setMemoryBase(0);
    }
    LEAVE();
}
#endif
/* This gets called at the start of each server generation. */

static Bool
SMI_ScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
    ScrnInfoPtr		pScrn = xf86Screens[pScreen->myNum];
    SMIPtr		pSmi = SMIPTR(pScrn);
    EntityInfoPtr	pEnt;
    int		numLines;
    BoxRec	AvailFBArea;
    RegionRec Screen, Full;

    ENTER();
	
    /* Map MMIO regs and framebuffer */
    if (!SMI_MapMem(pScrn))
        LEAVE(FALSE);

    pEnt = xf86GetEntityInfo(pScrn->entityList[0]);

#if 1
    if (!pSmi->pInt10 && pSmi->useBIOS) {
        pSmi->pInt10 = xf86InitInt10(pEnt->index);
    }
    if (!pSmi->pVbe && pSmi->pInt10 && xf86LoaderCheckSymbol("VBEInit")) {
        pSmi->pVbe = VBEInit(pSmi->pInt10, pEnt->index);
    }
#else
	if (!pSmi->pInt10 ) {
		pSmi->pInt10 = xf86InitInt10(pEnt->index);
	}
	if (!pSmi->pVbe && pSmi->pInt10 && xf86LoaderCheckSymbol("VBEInit")) {
		pSmi->pVbe = VBEInit(pSmi->pInt10, pEnt->index);
	}
#endif
    
    /* set bits of mmio+0x6b of sm750 */
    if(IS_SM750(pSmi)|| IS_SM718(pSmi))
	{
		uint tmp;
		tmp = PEEK32(0x68) & 0x00ffffff;
        POKE32(0x68,0x55000000|tmp);
	}
#if 0	
	//bios used all general purpose registers
	//so disable the function below
	else if(IS_SM712(pSmi))
	{
	
		CARD8 data;
		data = VGAIN8_INDEX(pSmi,VGA_SEQ_INDEX,VGA_SEQ_DATA,0x70);
		XERR("MONK,GET 3C4:70 == %02x\n",data);
		data &= 0x0f;
		data |= 0x50;		
		VGAOUT8_INDEX(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x70, data);
		XERR("MONK,after set ,GET 3C4:70 == %02x\n",VGAIN8_INDEX(pSmi,VGA_SEQ_INDEX,VGA_SEQ_DATA,0x70));
	}
#endif	
	/* save vga fonts for vga */
	if(IS_SM750(pSmi)||IS_SM718(pSmi))
		SMI750_SaveFonts(pScrn);

	/* Save the chip/graphics state */
    pSmi->Save(pScrn);

    /* Fill in some needed pScrn fields */
    pScrn->vtSema = TRUE;
    pScrn->pScreen = pScreen;

	XINF("MONK : pScrn->[virtualX,virtualY] = [%d,%d]\n",pScrn->virtualX,pScrn->virtualY);
	
    //pScrn->displayWidth = ((pScrn->virtualX * pSmi->Bpp + 15) & ~15) / pSmi->Bpp;
	pScrn->displayWidth = pScrn->virtualX;

    pSmi->fbArea = NULL;
    pSmi->FBOffset = 0;
    pScrn->fbOffset = pSmi->FBOffset + pSmi->fbMapOffset;

    /* Clear frame buffer */
    memset(pSmi->FBBase, 0, pSmi->videoRAMBytes);

	/*
	* The next step is to setup the screen's visuals, and initialise the
	* framebuffer code.  In cases where the framebuffer's default choises for
	* things like visual layouts and bits per RGB are OK, this may be as simple
	* as calling the framebuffer's ScreenInit() function.  If not, the visuals
	* will need to be setup before calling a fb ScreenInit() function and fixed
	* up after.
	*/

	/*
	* Reset the visual list.
	*/
    miClearVisualTypes();

    /* Setup the visuals we support. */

    if (!miSetVisualTypes(pScrn->depth, miGetDefaultVisualMask(pScrn->depth),
                pScrn->rgbBits, pScrn->defaultVisual))
        LEAVE(FALSE);

    if (!miSetPixmapDepths ())
        LEAVE(FALSE);

	/*
	* Call the framebuffer layer's ScreenInit function
	*/

    DEBUG("\tInitializing FB @ 0x%08X for %dx%d (%d)\n",
            pSmi->FBBase, pScrn->virtualX, pScrn->virtualY, pScrn->displayWidth);
    if(!fbScreenInit(pScreen, pSmi->FBBase, pScrn->virtualX, pScrn->virtualY, pScrn->xDpi,
                pScrn->yDpi, pScrn->displayWidth, pScrn->bitsPerPixel))
        LEAVE(FALSE);

    xf86SetBlackWhitePixels(pScreen);

    if (pScrn->bitsPerPixel > 8)
    {
        VisualPtr visual;
        /* Fixup RGB ordering */
        visual = pScreen->visuals + pScreen->numVisuals;
        while (--visual >= pScreen->visuals)
        {
            if ((visual->class | DynamicClass) == DirectColor) {
                visual->offsetRed   = pScrn->offset.red;
                visual->offsetGreen = pScrn->offset.green;
                visual->offsetBlue  = pScrn->offset.blue;
                visual->redMask     = pScrn->mask.red;
                visual->greenMask   = pScrn->mask.green;
                visual->blueMask    = pScrn->mask.blue;
            }
        }
    }

    /* must be after RGB ordering fixed */
    fbPictureInit(pScreen, 0, 0);

    /* Do the CRTC independent initialization */
    if(!SMI_HWInit(pScrn))
        LEAVE(FALSE);

    /* Unless using EXA, regardless using XAA or not, needs offscreen
     *  management at least for video. 
     */
    if ((pSmi->NoAccel) || (!pSmi->useEXA))
    {
        numLines = pSmi->FBReserved / (pScrn->displayWidth * pSmi->Bpp);
        AvailFBArea.x1 = 0;
        AvailFBArea.y1 = 0;
        AvailFBArea.x2 = pScrn->virtualX;
        AvailFBArea.y2 = numLines;

        xf86DrvMsg(pScrn->scrnIndex, X_INFO,"FrameBuffer Box: %d,%d - %d,%d\n",
                AvailFBArea.x1, AvailFBArea.y1, AvailFBArea.x2,AvailFBArea.y2);

        xf86InitFBManager(pScreen, &AvailFBArea);
    }

    /* Initialize acceleration layer */
    if (!pSmi->NoAccel)
    {
        if (pSmi->useEXA && !SMI_EXAInit(pScreen))
            LEAVE(FALSE);
        else if (!pSmi->useEXA && !SMI_XAAInit(pScreen))
            LEAVE(FALSE);
    }

    /* Initialize the chosen modes */
    if (!xf86SetDesiredModes(pScrn))
        LEAVE(FALSE);

    miInitializeBackingStore(pScreen);

#ifdef HAVE_XMODES
    xf86DiDGAInit(pScreen, (uint32_t)(pSmi->FBBase + pScrn->fbOffset));
#endif

    /* Initialise cursor functions */
    miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

    /* Initialize HW cursor layer.  Must follow software cursor
     * initialization.
     */
    if (pSmi->HwCursor)
    {
        int	size, flags;

        if (IS_MSOC(pSmi)) 
        {
            size = SMI501_MAX_CURSOR;
            flags = (HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_1 |
                    HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK);
#if SMI_CURSOR_ALPHA_PLANE
            if (!pSmi->DualView)
                flags |= HARDWARE_CURSOR_ARGB;
#endif
        }
        else if(IS_OLDLYNX(pSmi))
        {
            size = SMILYNX_MAX_CURSOR;
            flags = (HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_8 |
                    HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK |
                    HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
                    HARDWARE_CURSOR_BIT_ORDER_MSBFIRST |
                    HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
                    HARDWARE_CURSOR_INVERT_MASK);
        }
        else
        {
            /* sm750 or sm718 */
            if(pSmi->ARGBCursor)
            {
                /*
	                        1,HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_1 must be set to the flag
                               or you can find cusor shap error when X draw hands cursor (ubuntu9.10)
                               
                               2,HARDWARE_CURSOR_AND_SOURCE_WITH_MASK is a freak flag, sometimes I set it and see 
                               the black hand bug while can't see the bug without set it. but some times the situation is just reversed
                               ......I'm damn crazy about it.
                               Black hand bug description:
                               You can see  black hand shap cursur when first time X want to draw a 
                               white hand shape cursor ... (ubuntu 9.10 and fedora 10/11)

                               But weired thing is that ATI X driver seems not gonna show the above *black hand* phenomenon 
                               whether the flag *HARDWARE_CURSOR_AND_SOURCE_WITH_MASK* set or not .
                               While the bahavors of flag *HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_1* is just the same 
                               as ours , tested on Radeon HD4350 with xserver-xorg-video-ati-6.12.99 
                               		03/17/2010   Monk.Liu                    
                               */
                flags = HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_1|    					
                        //HARDWARE_CURSOR_AND_SOURCE_WITH_MASK|    
                        HARDWARE_CURSOR_ARGB|
                        0;
                size = SMI750_MAX_CURSOR;
            }
            else
            {
                flags = HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_1|
                        //HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK|
                        HARDWARE_CURSOR_AND_SOURCE_WITH_MASK|
                        0;
                size = SMI501_MAX_CURSOR;
            }            
        }
        if (!xf86_cursors_init(pScreen, size, size, flags))
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,"Hardware cursor initialization failed\n");
    	//xf86ForceHWCursor(pScreen,TRUE);
    }

    /* Initialise default colormap */
    if (!miCreateDefColormap(pScreen))
        LEAVE(FALSE);

	/* Initialize colormap layer.  Must follow initialization of the default
	* colormap.  And SetGamma call, else it will load palette with solid white.
	*/
    if (!xf86HandleColormaps(pScreen, 256, pScrn->rgbBits,SMI_LoadPalette, NULL,
                CMAP_RELOAD_ON_MODE_SWITCH | CMAP_PALETTED_TRUECOLOR))
        LEAVE(FALSE);

    pScreen->SaveScreen = SMI_SaveScreen;
    pSmi->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = SMI_CloseScreen;

    if ((IS_MSOC(pSmi) && !xf86DPMSInit(pScreen, SMI501_DisplayPowerManagementSet, 0)) ||
            (IS_SM750(pSmi) &&	!xf86DPMSInit(pScreen, SMI750_DisplayPowerManagementSet, 0)) ||
            (IS_SM718(pSmi) && !xf86DPMSInit(pScreen,SMI750_DisplayPowerManagementSet,0))||
            (IS_OLDLYNX(pSmi) && !xf86DPMSInit(pScreen, SMILynx_DisplayPowerManagementSet, 0)))
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "DPMS initialization failed!\n");

    if(IS_SM750(pSmi)|| IS_SM718(pSmi))
        SMI_InitVideo(pScreen);

    if(!xf86CrtcScreenInit(pScreen)) 
        LEAVE(FALSE);

    /* Report any unused options (only for the first generation) */
    if (serverGeneration == 1) {
        xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);
    }

    LEAVE(TRUE);
}

/*
 * This is called at the end of each server generation.  It restores the
 * original (text) mode.  It should also unmap the video memory, and free any
 * per-generation data allocated by the driver.  It should finish by unwrapping
 * and calling the saved CloseScreen function.
 */

static Bool
SMI_CloseScreen(int scrnIndex, ScreenPtr pScreen)
{
    ScrnInfoPtr	pScrn = xf86Screens[scrnIndex];
    SMIPtr	pSmi = SMIPTR(pScrn);
    Bool	ret;

    ENTER();

    if (pSmi->HwCursor)
        xf86_cursors_fini(pScreen);

    if (pScrn->vtSema)
        /* Restore console mode and unmap framebuffer */
        SMI_LeaveVT(scrnIndex, 0);

    if (pSmi->XAAInfoRec != NULL) {
        XAADestroyInfoRec(pSmi->XAAInfoRec);
    }
    if (pSmi->EXADriverPtr) {
        exaDriverFini(pScreen);
        pSmi->EXADriverPtr = NULL;
    }
    if (pSmi->pVbe != NULL) {
        vbeFree(pSmi->pVbe);
        pSmi->pVbe = NULL;
    }
    if (pSmi->pInt10 != NULL) {
        xf86FreeInt10(pSmi->pInt10);
        pSmi->pInt10 = NULL;
    }
    if (pSmi->ptrAdaptor != NULL) {
        xfree(pSmi->ptrAdaptor);
    }
    if (pSmi->BlockHandler != NULL) {
        pScreen->BlockHandler = pSmi->BlockHandler;
    }

    pScrn->vtSema = FALSE;
    pScreen->CloseScreen = pSmi->CloseScreen;
    ret = (*pScreen->CloseScreen)(scrnIndex, pScreen);

#ifdef RECORD
    /* don't forget to close the record file description */
    record_close();
#endif    
    LEAVE(ret);
}

static void
SMI_FreeScreen(int scrnIndex, int flags)
{
    SMI_FreeRec(xf86Screens[scrnIndex]);
}

static Bool
SMI_SaveScreen(ScreenPtr pScreen, int mode)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];

    ENTER();

    if(xf86IsUnblank(mode)){
        pScrn->DPMSSet(pScrn, DPMSModeOn, 0);
    }else{
        pScrn->DPMSSet(pScrn, DPMSModeOff, 0);
    }

    LEAVE(TRUE);
}

void
SMI_AdjustFrame(int scrnIndex, int x, int y, int flags)
{
    xf86CrtcConfigPtr crtcConf = XF86_CRTC_CONFIG_PTR(xf86Screens[scrnIndex]);
    xf86CrtcPtr compat_crtc = crtcConf->output[crtcConf->compat_output]->crtc;

    ENTER();

    SMICRTC(compat_crtc)->adjust_frame(compat_crtc,x,y);

    LEAVE();
}

Bool
SMI_SwitchMode(int scrnIndex, DisplayModePtr mode, int flags)
{
    Bool ret;
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();

    ret = xf86SetSingleMode(pScrn, mode, RR_Rotate_0);

    if (!pSmi->NoAccel)
        SMI_EngineReset(pScrn);

    LEAVE(ret);
}

void
SMI_LoadPalette(ScrnInfoPtr pScrn, int numColors, int *indicies,
        LOCO *colors, VisualPtr pVisual)
{
    xf86CrtcConfigPtr crtcConf = XF86_CRTC_CONFIG_PTR(pScrn);
    int crtc_idx,i,j;

    ENTER();

    if(pScrn->bitsPerPixel == 16){
        /* Expand the RGB 565 palette into the 256-elements LUT */

        for(crtc_idx=0; crtc_idx<crtcConf->num_crtc; crtc_idx++){
            SMICrtcPrivatePtr crtcPriv = SMICRTC(crtcConf->crtc[crtc_idx]);

            for(i=0; i<numColors; i++){
                int idx = indicies[i];

                if(idx<32){
                    for(j=0; j<8; j++){
                        crtcPriv->lut_r[idx*8 + j] = colors[idx].red << 8;
                        crtcPriv->lut_b[idx*8 + j] = colors[idx].blue << 8;
                    }
                }

                for(j=0; j<4; j++)
                    crtcPriv->lut_g[idx*4 + j] = colors[idx].green << 8;
            }

            crtcPriv->load_lut(crtcConf->crtc[crtc_idx]);
        }
    }else{
        for(crtc_idx=0; crtc_idx<crtcConf->num_crtc; crtc_idx++){
            SMICrtcPrivatePtr crtcPriv = SMICRTC(crtcConf->crtc[crtc_idx]);

            for(i = 0; i < numColors; i++) {
                int idx = indicies[i];

                crtcPriv->lut_r[idx] = colors[idx].red << 8;
                crtcPriv->lut_g[idx] = colors[idx].green << 8;
                crtcPriv->lut_b[idx] = colors[idx].blue << 8;
            }

            crtcPriv->load_lut(crtcConf->crtc[crtc_idx]);
        }
    }

    LEAVE();
}

static void
SMI_DisableVideo(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    CARD8 tmp;

//    if(!IS_MSOC(pSmi)) 
    if(pSmi->Primary)   //if this card get a bois
    {
        if (!(tmp = VGAIN8(pSmi, VGA_DAC_MASK)))
            return;
        pSmi->DACmask = tmp;
        VGAOUT8(pSmi, VGA_DAC_MASK, 0);
    }
}

static void
SMI_EnableVideo(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);

//    if (!IS_MSOC(pSmi) && !IS_SM750(pSmi) && !IS_SM718(pSmi)) 
	if( IS_OLDLYNX(pSmi))
    {
        VGAOUT8(pSmi, VGA_DAC_MASK, pSmi->DACmask);
    }
}


void
SMI_EnableMmio(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();

    if (!IS_MSOC(pSmi) && !IS_SM750(pSmi) && !IS_SM718(pSmi)) {
        vgaHWPtr hwp = VGAHWPTR(pScrn);
        CARD8 tmp;

        /*
         * Enable chipset (seen on uninitialized secondary cards) might not be
         * needed once we use the VGA softbooter
         */
        vgaHWSetStdFuncs(hwp);

        /* Enable linear mode */
        outb(pSmi->PIOBase + VGA_SEQ_INDEX, 0x18);
        tmp = inb(pSmi->PIOBase + VGA_SEQ_DATA);
        pSmi->SR18Value = tmp;					/* PDR#521 */
        outb(pSmi->PIOBase + VGA_SEQ_DATA, tmp | 0x11);

        /* Enable 2D/3D Engine and Video Processor */
        outb(pSmi->PIOBase + VGA_SEQ_INDEX, 0x21);
        tmp = inb(pSmi->PIOBase + VGA_SEQ_DATA);
        pSmi->SR21Value = tmp;					/* PDR#521 */
        outb(pSmi->PIOBase + VGA_SEQ_DATA, tmp & ~0x03);
    }

    LEAVE();
}

void
SMI_DisableMmio(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();

//    if (!IS_MSOC(pSmi))
    {
        vgaHWPtr hwp = VGAHWPTR(pScrn);

        vgaHWSetStdFuncs(hwp);

        /* Disable 2D/3D Engine and Video Processor */
        outb(pSmi->PIOBase + VGA_SEQ_INDEX, 0x21);
        outb(pSmi->PIOBase + VGA_SEQ_DATA, pSmi->SR21Value);	/* PDR#521 */

        /* Disable linear mode */
        outb(pSmi->PIOBase + VGA_SEQ_INDEX, 0x18);
        outb(pSmi->PIOBase + VGA_SEQ_DATA, pSmi->SR18Value);	/* PDR#521 */
    }

    LEAVE();
}

static void
SMI_ProbeDDC(ScrnInfoPtr pScrn, int index)
{
    vbeInfoPtr pVbe;
    if (xf86LoadSubModule(pScrn, "vbe")) {
        pVbe = VBEInit(NULL, index);
        ConfiguredMonitor = vbeDoEDID(pVbe, NULL);
        vbeFree(pVbe);
    }
}

static Bool
SMI_HWInit(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();

    if(IS_MSOC(pSmi))
        LEAVE(SMI501_HWInit(pScrn));
    else if(IS_SM750(pSmi)|| IS_SM718(pSmi))
        LEAVE(SMI750_HWInit(pScrn));
    else if(IS_SM712(pSmi))
        LEAVE(SMILynx_HWInit(pScrn));        
    LEAVE(FALSE);
}

void
SMI_PrintRegs(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    int i;

    ENTER();

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, VERBLEV,
    "START register dump ------------------\n");

    if(IS_MSOC(pSmi)){
        SMI501_PrintRegs(pScrn);
    }else if(IS_SM750(pSmi)|| IS_SM718(pSmi)){
        SMI750_PrintRegs(pScrn);

        xf86ErrorFVerb(VERBLEV, "\n\n");
        xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, VERBLEV,
        "END register dump --------------------\n");

        LEAVE();
    }else if (IS_SM712(pSmi)){
        SMILynx_PrintRegs(pScrn);
    }

#if 0
    xf86ErrorFVerb(VERBLEV, "\n\nDPR    x0       x4       x8       xC");
    for (i = 0x00; i <= 0x44; i += 4) {
        if ((i & 0xF) == 0x0) xf86ErrorFVerb(VERBLEV, "\n%02X|", i);
        xf86ErrorFVerb(VERBLEV, " %08lX", (uint32_t)READ_DPR(pSmi, i));
    }

    xf86ErrorFVerb(VERBLEV, "\n\nVPR    x0       x4       x8       xC");
    for (i = 0x00; i <= 0x60; i += 4) {
        if ((i & 0xF) == 0x0) xf86ErrorFVerb(VERBLEV, "\n%02X|", i);
        xf86ErrorFVerb(VERBLEV, " %08lX", (uint32_t)READ_VPR(pSmi, i));
    }

    xf86ErrorFVerb(VERBLEV, "\n\nCPR    x0       x4       x8       xC");
    for (i = 0x00; i <= 0x18; i += 4) {
        if ((i & 0xF) == 0x0) xf86ErrorFVerb(VERBLEV, "\n%02X|", i);
        xf86ErrorFVerb(VERBLEV, " %08lX", (uint32_t)READ_CPR(pSmi, i));
    }

    xf86ErrorFVerb(VERBLEV, "\n\n");
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, VERBLEV,
            "END register dump --------------------\n");
#endif
    LEAVE();
}

#ifdef SMI_RECORD

/* below functions are used for records of information and messages */
int record_start(const char * path)
{
#define FILE_MODE S_IRWXU   
       g_recordFD = open(path,O_RDWR|O_APPEND|O_CREAT|O_TRUNC,FILE_MODE);
       return g_recordFD;       
}

void record_close()
{
    if(g_recordFD >0)
        close(g_recordFD);
}

size_t record_append(const char * message,int length)
{
    size_t written = 0;
    if(g_recordFD > 0)
    {
        written = write(g_recordFD,message,length);
    }
    return written;
    
}
#endif


void
SMI_FixMTRR(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    int fd;
    struct mtrr_sentry sentry;
    ENTER();
    fd = open("/sys/module/vesafb/initstate", O_RDONLY, 0);
    if (fd == -1) {
        DEBUG("Not using vesafb.  No need for MTRR fix.\n");
        LEAVE();
    }
    else {
        DEBUG("vesafb detected.  Fixing mtrr.\n");
    }
    if ((fd = open("/proc/mtrr", O_WRONLY, 0)) != -1 ) {
        sentry.base = pScrn->memPhysBase;
        sentry.size = pSmi->videoRAMBytes;
        sentry.type = MTRR_TYPE_WRCOMB;
        if (ioctl(fd, MTRRIOC_ADD_ENTRY, &sentry) == -1) {
            DEBUG("Faild to add MTRR range \n");
        }
    } else {
        DEBUG("Error opening /proc/mtrr");
    }
    LEAVE();
}


