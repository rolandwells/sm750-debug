#ifndef DDK_INTERN_H__
#define DDK_INTERN_H__
//inline unsigned long roundedDiv(ulong,ulong);
#if 0
unsigned long peekRegisterDWord(unsigned long offset);
void pokeRegisterDWord(unsigned long offset,data);
#endif

void setDualPanel(unsigned long enable);
long setPanelType(panel_type_t panelType);
void displaySetInterpolation(unsigned long enableHorzInterpolation,unsigned long enableVertInterpolation);
long scaleDisplay(logicalMode_t *pLogicalMode);
long setCustomModeEx(logicalMode_t *pLogicalMode, mode_parameter_t *pUserModeParam);
unsigned long isDisplayControlSwapped(void);
unsigned long getScratchData(unsigned long dataFlag);
void setMasterClock(unsigned long frequency);
void setPowerMode(unsigned long powerMode);
void setMemoryClock(unsigned long frequency);

void setChipClock(unsigned long frequency);
unsigned long getChipClock(void);
unsigned long getPllValue(clock_type_t clockType, pll_value_t *pPLL);
void setCurrentGate(unsigned long gate);
inline unsigned long getPowerMode(void);

unsigned long calcPllValue(unsigned long ulRequestClk,pll_value_t *pPLL);

inline unsigned long formatPllReg(pll_value_t *pPLL);
inline unsigned long twoToPowerOfx(unsigned long x);
inline unsigned long absDiff(unsigned long a, unsigned long b);
inline unsigned long calcPLL(pll_value_t *pPLL);
clock_type_t getClockType(disp_control_t dispCtrl);

void programModeRegisters(
mode_parameter_t *pModeParam,   /* mode information about pixel clock, horizontal total, etc. */
unsigned long ulBpp,            /* Color depth for this mode */
unsigned long ulBaseAddress,    /* Offset in frame buffer */
unsigned long ulPitch,          /* Mode pitch value in byte: no of bytes between two lines. */
pll_value_t *pPLL               /* Pre-calculated values for the PLL */
);

mode_parameter_t *findVesaModeParam(
unsigned long width, 
unsigned long height, 
unsigned long refresh_rate);


void deReset(void);
unsigned long deGetTransparency(void);
long deSetClipping(
unsigned long enable, /* 0 = disable clipping, 1 = enable clipping */
unsigned long x1,     /* x1, y1 is the upper left corner of the clipping area */
unsigned long y1,     /* Note that the region includes x1 and y1 */
unsigned long x2,     /* x2, y2 is the lower right corner of the clippiing area */
unsigned long y2);     /* Note that the region will not include x2 and y2 */

long deSetTransparency(
unsigned long enable,     /* 0 = disable, 1 = enable transparency feature */
unsigned long tSelect,    /* 0 = compare source, 1 = compare destination */
unsigned long tMatch,     /* 0 = Opaque mode, 1 = transparent mode */
unsigned long ulColor);    /* Color to compare. */


void enable2DEngine(unsigned long enable);
void enableZVPort(unsigned long enable);

void panelWaitVerticalSync(unsigned long vsync_count);
void setDAC(disp_state_t state);
void setDisplayControl(disp_control_t dispControl, disp_state_t dispState);
void setPath(disp_path_t dispPath, disp_control_t dispControl, disp_state_t dispState);
void enableI2C(unsigned long);
void enableGPIO(unsigned long);


/* swi2c functions */

long swI2CInit(
    unsigned char i2cClkGPIO, 
    unsigned char i2cDataGPIO
);

unsigned char swI2CReadReg(
    unsigned char deviceAddress, 
    unsigned char registerIndex
);

long swI2CWriteReg(
    unsigned char deviceAddress, 
    unsigned char registerIndex, 
    unsigned char data
);

void swI2CSCL(unsigned char value);

void swI2CSDA(unsigned char value);


/* hwi2c functions */
long hwI2CInit(unsigned char busSpeedMode);
void hwI2CClose(void);

unsigned char hwI2CReadReg(unsigned char deviceAddress,unsigned char registerIndex);
long hwI2CWriteReg(unsigned char deviceAddress,unsigned char registerIndex,unsigned char data);

/* Silicon Image SiI164 chip prototype */
long sii164InitChip(
    unsigned char edgeSelect,
    unsigned char busSelect,
    unsigned char dualEdgeClkSelect,
    unsigned char hsyncEnable,
    unsigned char vsyncEnable,
    unsigned char deskewEnable,
    unsigned char deskewSetting,
    unsigned char continuousSyncEnable,
    unsigned char pllFilterEnable,
    unsigned char pllFilterValue
);

void sii164ResetChip(void);
char *sii164GetChipString(void);
unsigned short sii164GetVendorID(void);
unsigned short sii164GetDeviceID(void);
void sii164SetPower(unsigned char powerUp);
void sii164EnableHotPlugDetection(unsigned char enableHotPlug);
unsigned char sii164IsConnected(void);
unsigned char sii164CheckInterrupt(void);
void sii164ClearInterrupt(void);

/* dvi functions prototype */
long dviInit(
    unsigned char edgeSelect,
    unsigned char busSelect,
    unsigned char dualEdgeClkSelect,
    unsigned char hsyncEnable,
    unsigned char vsyncEnable,
    unsigned char deskewEnable,
    unsigned char deskewSetting,
    unsigned char continuousSyncEnable,
    unsigned char pllFilterEnable,
    unsigned char pllFilterValue
);

unsigned short dviGetVendorID(void);
unsigned short dviGetDeviceID(void);


#endif
