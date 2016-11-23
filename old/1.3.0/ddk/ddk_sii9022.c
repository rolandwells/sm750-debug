#ifdef USE_HDMICHIP
#include "ddk750_common.h"
#include "siHdmiTx_902x_TPI.h"
#define SII9022_DEVICE_ID					0xb0

unsigned char sii9022xIsConnected(void * ufb)	
{
	uint8_t ret;
	ret = i2cReadReg(ufb,SII9022A_I2C_ADDRESS,0x3d);
	//printk("sii9022x reg3d is %x\n",ret);
	if ((ret&0xc) == 0xc){
		return 1;
	}
	return 0;

}


#if 0
static void dump9022a(void * ufb,uint8_t reg){
	printk("%x=%x\n",reg,i2cReadReg(ufb,SII9022A_I2C_ADDRESS,reg));
}
#endif

#define MATCH(H,V) \
	(modeparm->horizontal_display_end == (H) && modeparm->vertical_display_end == (V)) 

int sii9022xSetMode(void * ufb,mode_parameter_t *modeparm)
{
	int ret;
	enum avidmode num;
	if(MATCH(1920,1080))
		num = HDMI_1080P60;
	else if(MATCH(1280,720))
		num = HDMI_720P60;
	else if(MATCH(800,600))
		num = HDMI_800_600_60;
	else if(MATCH(1024,768))
		num = HDMI_1024_768_60;
	else if(MATCH(1280,960))
		num = HDMI_1280_960_60;
	else if(MATCH(1280,1024))
		num = HDMI_1280_1024_60;
	else if(MATCH(1360,768))
		num = HDMI_1360_768_60;
#if 0
	else if(MATCH(1366,768))
		num = HDMI_1366_768_60;
	else if(MATCH(1368,768))
		num = HDMI_1368_768_60;
	else if(MATCH(1440,900))
		num = HDMI_1440_900_60;
#endif
	else if(MATCH(1600,1200))
		num = HDMI_1600_1200_60;
	else if(MATCH(1400,1050))
		num = HDMI_1400_1050_60;
	else
		num = HDMI_1080P60;

	siHdmiTx_g_ufb(ufb);
#ifdef USE_HW_I2C
    /* Use fast mode. */
    hwI2CInit(ufb,1);
#else        
    swI2CInit(ufb,DEFAULT_I2C_SCL, DEFAULT_I2C_SDA);
#endif

	siHdmiTx_VideoSel(num);
	ret = siHdmiTx_VideoSet();

	i2cWriteReg(ufb,SII9022A_I2C_ADDRESS,0x63,0);
	return ret;
}

int sii9022xInitChip(void * ufb)
{
	uint8_t rcc,retries = 10;
#ifdef USE_HW_I2C
    /* Use fast mode. */
    hwI2CInit(ufb,1);
#else        
    swI2CInit(ufb,DEFAULT_I2C_SCL, DEFAULT_I2C_SDA);
#endif

	
	/* enter TPI mode */
    i2cWriteReg(ufb,SII9022A_I2C_ADDRESS,0xc7,0);
	do{
		msleep(1);
		rcc = i2cReadReg(ufb,SII9022A_I2C_ADDRESS,0x1B);
	}while((rcc != SII9022_DEVICE_ID) && retries--);

	if(rcc != SII9022_DEVICE_ID){
		return -ENODEV;
	}
	return 0;
}


#endif
