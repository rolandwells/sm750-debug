#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86Module.h"
#if XORG_VERSION_CURRENT < 10706000
#include "xf86Resources.h"
#endif
#include "version.h"

static MODULESETUPPROTO(smi750ddk_Setup);

static XF86ModuleVersionInfo smi750ddkVersRec = 
  {
    "smi750ddk",
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XORG_VERSION_CURRENT,
    MAJOR_VERSION(LIBRARY_VERSION), 
    MINOR_VERSION(LIBRARY_VERSION),
    0,
    ABI_CLASS_VIDEODRV,
    ABI_VIDEODRV_VERSION,
    MOD_CLASS_NONE,
    { 0,0,0,0 }
  };

_X_EXPORT XF86ModuleData smi750ddkModuleData = {
  &smi750ddkVersRec,
  smi750ddk_Setup,
  NULL
};

static pointer
smi750ddk_Setup(pointer module, pointer opts, int *errmaj, int *errmin) {
  return (pointer)1;
}

