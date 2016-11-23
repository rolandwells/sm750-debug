#
# Makefile for wc patch kernel module
# 					-by Monk.liu
#
ifeq ($(KERNELRELEASE),)
knv :=$(shell uname -r)
KERNELDIR :=/lib/modules/$(knv)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules 
install:default
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
else

obj-m += ufb.o
ufb-y := usbfb.o usbacc.o ufb_hw750.o 
ufb-y += ddk/ddk_chip.o ddk/ddk_power.o ddk/ddk_mode.o ddk/ddk_display.o ddk/ddk_2d.o
ufb-y += ddk/ddk_hwi2c.o ddk/ddk_swi2c.o ddk/edid.o



EXTRA_CFLAGS += -DRIGOROUS=0
EXTRA_CFLAGS += -DVMALLOCFB=1
EXTRA_CFLAGS += -DBURST_CONSOLE

EXTRA_CFLAGS += -DUSE_HDMICHIP
ufb-y += ddk/ddk_dvi.o
ufb-y += ddk/ddk_sii9022.o
ufb-y += ddk/siHdmiTx_902x_TPI.o
EXTRA_CFLAGS += -DUSE_DVICHIP
ufb-y += ddk/ddk_sii164.o

ufb-objs := $(ufb-y)

#EXTRA_CFLAGS += -DUSE_HW_I2C

ifeq ($(cscpatch),1)
EXTRA_CFLAGS += -DCSC_PATCH=1
else
EXTRA_CFLAGS += -DCSC_PATCH=0
endif

ifeq ($(debug),1)
EXTRA_CFLAGS += -DDEBUG=1
endif


ifeq ($(debug),2)
EXTRA_CFLAGS += -DDEBUG=2
endif

endif
