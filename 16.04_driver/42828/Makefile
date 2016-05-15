#
# Makefile for lynx frame buffer
# 					-by Monk.liu
#
ifeq ($(KERNELRELEASE),)

ifeq ($(kernel),)
# build the driver with kernel version currerntly using
knv :=$(shell uname -r)
else
# user want to build the driver based on specific kernel version
knv :=$(kernel)
endif

KERNELDIR :=/lib/modules/$(knv)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) CONFIG_FB_LYNXFB=m modules
install:default
	$(MAKE) -C $(KERNELDIR) M=$(PWD) CONFIG_FB_LYNXFB=m modules_install
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
else

obj-$(CONFIG_FB_LYNXFB) += lynxfb.o

lynxfb-y := lynx_drv.o lynx_hw750.o lynx_hw712.o lynx_hw502.o lynx_accel.o lynx_cursor.o
lynxfb-y += ddk750/ddk750_chip.o ddk750/ddk750_power.o ddk750/ddk750_mode.o ddk750/ddk750_display.o ddk750/ddk750_help.o ddk750/ddk750_edid.o
lynxfb-y += ddk712/ddk712_chip.o ddk712/ddk712_mode.o ddk712/ddk712_help.o ddk750/ddk750_swi2c.o

ifeq ($(hdmi),1)
EXTRA_CFLAGS += -DUSE_HDMICHIP
lynxfb-y += ddk750/ddk750_sii9022.o
lynxfb-y += ddk750/siHdmiTx_902x_TPI.o
endif


ifneq ($(nodvi),1)
lynxfb-y += ddk750/ddk750_sii164.o
lynxfb-y += ddk750/ddk750_dvi.o
EXTRA_CFLAGS += -DUSE_DVICHIP
else
endif

# if user define swi2c=1,then for sm750/sm718,its dvi chip (sii164) will be initilized by swi2c
# but for 750le, always swi2c used to setup its 7301 dvi chip
ifneq ($(swi2c),1)
lynxfb-y += ddk750/ddk750_hwi2c.o
EXTRA_CFLAGS += -DUSE_HW_I2C
endif

lynxfb-objs := $(lynxfb-y)

ifeq ($(debug),1)
#CONFIG_FB_LYNXFB_DEBUG=y
EXTRA_CFLAGS += -DDEBUG=1
endif

ifeq ($(debug),2)
EXTRA_CFLAGS += -DDEBUG=2
endif

#ifdef CONFIG_FB_LYNXFB_DEBUG
#endif

endif
