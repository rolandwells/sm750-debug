
#ifndef DDK_TYPE_H__
#define DDK_TYPE_H__

#define MB(x)	(x << 20)	/* dont use this marco if x is fraction*/
#define REG_SIZE	MB(2)
//#define PADDING(align,data)	((data+align-1)&(~(align-1)))

#define DEFAULT_INPUT_CLOCK 14318181 /* Default reference clock */
#define MHz(x) (x*1000000) /* Don't use this macro if x is fraction number */

/* Blt Direction definitions */
#define TOP_TO_BOTTOM 0
#define LEFT_TO_RIGHT 0
#define BOTTOM_TO_TOP 1
#define RIGHT_TO_LEFT 1

/* n / d + 1 / 2 = (2n + d) / 2d */
#define roundedDiv(num,denom)	((2 * num + denom) / (2 * denom))


#define SWAP_DISPLAY_CTRL_FLAG	1

typedef int (*pf_output)(char * buf,const char *s, ...);

/* input struct to initChipParam() function */
typedef struct _initchip_param_t
{
    unsigned short powerMode;    /* Use power mode 0 or 1 */
    unsigned short chipClock;    /* Speed of main chip clock in MHz unit
                                    0 = keep the current clock setting
                                    Others = the new main chip clock
                                  */
    unsigned short memClock;     /* Speed of memory clock in MHz unit
                                    0 = keep the current clock setting
                                    Others = the new memory clock
                                  */
    unsigned short masterClock;  /* Speed of master clock in MHz unit 
                                    0 = keep the current clock setting
                                    Others = the new master clock
                                  */
    unsigned short setAllEngOff; /* 0 = leave all engine state untouched.
                                    1 = make sure they are off: 2D, Overlay,
                                    video alpha, alpha, hardware cursors
                                 */
    unsigned char resetMemory;   /* 0 = Do not reset the memory controller
                                    1 = Reset the memory controller
                                  */

    /* More initialization parameter can be added if needed */
}
initchip_param_t;

typedef enum _clock_type_t
{
    MXCLK_PLL,    /* Programmable Master clock */
    PANEL_PLL,    /* Programmable panel pixel clock */
    CRT_PLL,      /* Programmable CRT pixel clock */
    VGA0_PLL,
    VGA1_PLL,
}
clock_type_t;


typedef struct pll_value_t
{
    clock_type_t clockType;
    unsigned long inputFreq; /* Input clock frequency to the PLL */

    /* Use this when clockType = PANEL_PLL */    
    unsigned long M;
    unsigned long N;
    unsigned long OD;
    unsigned long POD;
}
pll_value_t;

typedef enum _spolarity_t
{
    POS, /* positive */
    NEG, /* negative */
}
spolarity_t;

typedef struct _mode_parameter_t
{
    /* Horizontal timing. */
    unsigned long horizontal_total;
    unsigned long horizontal_display_end;
    unsigned long horizontal_sync_start;
    unsigned long horizontal_sync_width;
    spolarity_t horizontal_sync_polarity;

    /* Vertical timing. */
    unsigned long vertical_total;
    unsigned long vertical_display_end;
    unsigned long vertical_sync_start;
    unsigned long vertical_sync_height;
    spolarity_t vertical_sync_polarity;

    /* Refresh timing. */
    unsigned long pixel_clock;
    unsigned long horizontal_frequency;
    unsigned long vertical_frequency;
    
    /* Clock Phase. This clock phase only applies to Panel. */
    spolarity_t clock_phase_polarity;
}
mode_parameter_t;


typedef enum _disp_control_t
{
    PANEL_CTRL = 0,
    CRT_CTRL   = 1,
    VGA_CTRL   = 2
    /* Think of defining other display control here: Video, alpha, etc. */
}
disp_control_t;

typedef struct _logicalMode_t
{
    unsigned long x;            /* X resolution */
    unsigned long y;            /* Y resolution */
    unsigned long bpp;          /* Bits per pixel */
    unsigned long hz;           /* Refresh rate */

    unsigned long baseAddress;  /* Offset from beginning of frame buffer.
                                   It is used to control the starting location of a mode.
                                   Calling function must initialize this field.
                                 */

    unsigned long pitch;        /* Mode pitch in byte.
                                   If initialized to 0, setMode function will set
                                   up this field.
                                   If not zero, setMode function will use this value.
                                 */

    disp_control_t dispCtrl;    /* CRT or PANEL display control channel */
								   
    unsigned long xLCD;         /* Panel width */
    unsigned long yLCD;         /* Panel height */    
    void *userData;             /* Not used now, set it to 0 (for future used only) */
}
logicalMode_t;



typedef enum _DPMS_t
{
    DPMS_ON,
    DPMS_STANDBY,
    DPMS_SUSPEND,
    DPMS_OFF
}
DPMS_t;


typedef enum _disp_path_t
{
    PANEL_PATH = 0,
    CRT_PATH   = 1,
}
disp_path_t;

typedef enum _disp_state_t
{
    DISP_OFF = 0,
    DISP_ON  = 1,
}
disp_state_t;

typedef enum _disp_output_t
{
    NO_DISPLAY,             /* All display off. */
    PANEL1_ONLY,    
    PANEL2_ONLY,
    CRT1_CRT2_SIMUL,
    PANEL_CRT_SIMUL,        /* Both Panel(s) and CRT(s) displaying the same content. */
    PANEL_CRT_SIMUL_SEC,	/*	like above but use secondary controller data	*/
    PANEL1_PANEL2_SIMUL,     /* Both Panel 1 and Panel 2 displaying the primary ctrl content. */
    PANEL1_PANEL2_SIMUL_SEC,		/*like above but use secondary ctrl content	*/
    CRT_PANEL2_SIMUL,       /* Both Panel 2 and CRT displaying the same content. */
    PANEL1_PANEL2_CRT_SIMUL, /* All displays displaying the same content. */
    PANEL_CRT_DUAL,         /* Panel and CRT displaying different contents. */
    PANEL_PANEL2_DUAL,      /* Panel and Panel 2 displaying different contents. */
    PANEL_PANEL2_CRT_DUAL,   /* Panel2 and CRT displaying the same content while
                               the Panel displaying different content. */
	PANEL_2_PRIMARY,
	CRT_2_SECONDARY
}
disp_output_t;

typedef enum _panel_type_t
{
    TFT_18BIT = 0,
    TFT_24BIT,
    TFT_36BIT
}
panel_type_t;

/* dvi chip stuffs structros */

typedef long (*PFN_DVICTRL_INIT)(
		void *,
    unsigned char edgeSelect,
    unsigned char busSelect,
    unsigned char dualEdgeClkSelect,
    unsigned char hsyncEnable,
    unsigned char vsyncEnable,
    unsigned char deskewEnable,
    unsigned char deskewSetting,
    unsigned char continuousSyncEnable,
    unsigned char pllFilterEnable,
    unsigned char pllFilterValue);
typedef void (*PFN_DVICTRL_RESETCHIP)(void*);
typedef char* (*PFN_DVICTRL_GETCHIPSTRING)(void);
typedef unsigned short (*PFN_DVICTRL_GETVENDORID)(void*);
typedef unsigned short (*PFN_DVICTRL_GETDEVICEID)(void*);
typedef void (*PFN_DVICTRL_SETPOWER)(void *,unsigned char powerUp);
typedef void (*PFN_DVICTRL_HOTPLUGDETECTION)(void *,unsigned char enableHotPlug);
typedef unsigned char (*PFN_DVICTRL_ISCONNECTED)(void*);
typedef unsigned char (*PFN_DVICTRL_CHECKINTERRUPT)(void*);
typedef void (*PFN_DVICTRL_CLEARINTERRUPT)(void*);


/* Structure to hold all the function pointer to the DVI Controller. */
typedef struct _dvi_ctrl_device_t
{
    PFN_DVICTRL_INIT                pfnInit;
    PFN_DVICTRL_RESETCHIP           pfnResetChip;
    PFN_DVICTRL_GETCHIPSTRING       pfnGetChipString;
    PFN_DVICTRL_GETVENDORID         pfnGetVendorId;
    PFN_DVICTRL_GETDEVICEID         pfnGetDeviceId;
    PFN_DVICTRL_SETPOWER            pfnSetPower;
    PFN_DVICTRL_HOTPLUGDETECTION    pfnEnableHotPlugDetection;
    PFN_DVICTRL_ISCONNECTED         pfnIsConnected;
    PFN_DVICTRL_CHECKINTERRUPT      pfnCheckInterrupt;
    PFN_DVICTRL_CLEARINTERRUPT      pfnClearInterrupt;
} dvi_ctrl_device_t;

#define DVI_CTRL_SII164
#define SII164_FULL_FUNCTIONS


/* Hot Plug detection mode structure */
typedef enum _sii164_hot_plug_mode_t
{
    SII164_HOTPLUG_DISABLE = 0,         /* Disable Hot Plug output bit (always high). */
    SII164_HOTPLUG_USE_MDI,             /* Use Monitor Detect Interrupt bit. */
    SII164_HOTPLUG_USE_RSEN,            /* Use Receiver Sense detect bit. */
    SII164_HOTPLUG_USE_HTPLG            /* Use Hot Plug detect bit. */
} sii164_hot_plug_mode_t;

#endif
