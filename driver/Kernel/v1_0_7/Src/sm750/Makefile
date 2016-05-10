#
# Makefile for sm750 frame buffer 
# 					-by Monk.liu

#knv :=2.6.21
ifeq ($(KERNELRELEASE),)

knv :=$(shell uname -r)
KERNELDIR :=/lib/modules/$(knv)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) CONFIG_FB_SM750=m modules 
	$(MAKE) -C $(KERNELDIR) M=$(PWD) CONFIG_FB_SM750=m modules_install
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
else

obj-$(CONFIG_FB_SM750) += sm750fb.o

sm750fb-y := sm750drv.o sm750hw.o
sm750fb-y += ddk/ddk_chip.o ddk/ddk_power.o ddk/ddk_mode.o ddk/ddk_display.o ddk/ddk_2d.o 

ifneq ($(nodvi),1)
sm750fb-y += ddk/ddk_sii164.o
sm750fb-y += ddk/ddk_dvi.o
EXTRA_CFLAGS += -DUSE_DVICHIP
else
endif

ifneq ($(swi2c),1)
sm750fb-y += ddk/ddk_hwi2c.o
EXTRA_CFLAGS += -DUSE_HW_I2C
else
sm750fb-y += ddk/ddk_swi2c.o
endif

sm750fb-objs := $(sm750fb-y)

ifeq ($(debug),1)
CONFIG_FB_SM750_DEBUG=y
endif

ifdef CONFIG_FB_SM750_DEBUG
EXTRA_CFLAGS += -DDEBUG
endif

endif






