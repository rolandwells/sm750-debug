#ifndef DDK712_MODE_H__
#define DDK712_MODE_H__

typedef struct{
	unsigned short h_res;
	unsigned short v_res;
	char vsync;
	char svr[14];/* shadow vga register :svr 40 => svr 4d*/
	char ccr[2];/* pixel pll:ccr6c,6d */
}SM712CrtTiming;

typedef struct{
	unsigned short h_res;
	unsigned short v_res;
	char vsync;
	char fpr[14];/* fpr50 ==> fpr57,fpr5a*/
	char ccr[2];/* ccr6e.ccr6f*/
}SM712PnlTiming;


void ddk712_setModeTiming(int channel,int x,int y,int hz);
#endif
