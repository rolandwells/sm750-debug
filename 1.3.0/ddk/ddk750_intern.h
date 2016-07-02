#ifndef DDK_INTERN_H__
#define DDK_INTERN_H__

void setDualPanel(void *,unsigned long enable);
long setPanelType(void *,panel_type_t panelType);
void displaySetInterpolation(void *,unsigned long enableHorzInterpolation,unsigned long enableVertInterpolation);
long scaleDisplay(void *,logicalMode_t *pLogicalMode);
long setCustomModeEx(void *,logicalMode_t *pLogicalMode, mode_parameter_t *pUserModeParam);
unsigned long isDisplayControlSwapped(void);
unsigned long getScratchData(void *,unsigned long dataFlag);
void setMasterClock(void *,unsigned long frequency);
void setPowerMode(void *,unsigned long powerMode);
void setMemoryClock(void *,unsigned long frequency);

void setChipClock(void *,unsigned long frequency);
unsigned long getChipClock(void*);
unsigned long getPllValue(void *,clock_type_t clockType, pll_value_t *pPLL);
void setCurrentGate(void *,unsigned long gate);
inline unsigned long getPowerMode(void*);

unsigned long calcPllValue(unsigned long ulRequestClk,pll_value_t *pPLL);

inline unsigned long formatPllReg(pll_value_t *pPLL);
inline unsigned long twoToPowerOfx(unsigned long x);
inline unsigned long absDiff(unsigned long a, unsigned long b);
inline unsigned long calcPLL(pll_value_t *pPLL);
clock_type_t getClockType(disp_control_t dispCtrl);

void programModeRegisters(
		void *,
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


void deReset(void*);
//unsigned long deGetTransparency(void);
long deSetClipping(
		void *,
		unsigned long enable, /* 0 = disable clipping, 1 = enable clipping */
		unsigned long x1,     /* x1, y1 is the upper left corner of the clipping area */
		unsigned long y1,     /* Note that the region includes x1 and y1 */
		unsigned long x2,     /* x2, y2 is the lower right corner of the clippiing area */
		unsigned long y2);     /* Note that the region will not include x2 and y2 */

#if 0
long deSetTransparency(
		unsigned long enable,     /* 0 = disable, 1 = enable transparency feature */
		unsigned long tSelect,    /* 0 = compare source, 1 = compare destination */
		unsigned long tMatch,     /* 0 = Opaque mode, 1 = transparent mode */
		unsigned long ulColor);    /* Color to compare. */
#endif


void enable2DEngine(void *,unsigned long enable);
void enableZVPort(void *,unsigned long enable);

void panelWaitVerticalSync(void *,unsigned long vsync_count);
void setDAC(void *,disp_state_t state);
void setDisplayControl(void *,disp_control_t dispControl, disp_state_t dispState);
void setPath(void *,disp_path_t dispPath, disp_control_t dispControl, disp_state_t dispState);
void enableI2C(void *,unsigned long);
void enableGPIO(void *,unsigned long);



void swI2CSCL(void *,unsigned char value);
void swI2CSDA(void *,unsigned char value);
unsigned char swI2CReadSCL(void*);
unsigned char swI2CReadSDA(void*);


/* hwi2c functions */
long hwI2CInit(void *,unsigned char busSpeedMode);
void hwI2CClose(void*);

unsigned char hwI2CReadReg(void *,unsigned char deviceAddress,unsigned char registerIndex);
long hwI2CWriteReg(void *,unsigned char deviceAddress,unsigned char registerIndex,unsigned char data);

/* Silicon Image SiI164 chip prototype */
long sii164InitChip(
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
		unsigned char pllFilterValue
		);

void sii164ResetChip(void*);
char *sii164GetChipString(void);
unsigned short sii164GetVendorID(void*);
unsigned short sii164GetDeviceID(void*);
void sii164SetPower(void*,unsigned char powerUp);
void sii164EnableHotPlugDetection(void*,unsigned char enableHotPlug);
unsigned char sii164IsConnected(void*);
unsigned char sii164CheckInterrupt(void*);
void sii164ClearInterrupt(void*);

/* dvi functions prototype */
long dviInit(
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
		unsigned char pllFilterValue
		);

unsigned short dviGetVendorID(void*);
unsigned short dviGetDeviceID(void*);


#endif
