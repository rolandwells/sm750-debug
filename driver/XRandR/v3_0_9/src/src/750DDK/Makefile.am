#  Copyright 2005 Adam Jackson.
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  on the rights to use, copy, modify, merge, publish, distribute, sub
#  license, and/or sell copies of the Software, and to permit persons to whom
#  the Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice (including the next
#  paragraph) shall be included in all copies or substantial portions of the
#  Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
#  ADAM JACKSON BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
#  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
#  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# this is obnoxious:
# -module lets us name the module exactly how we want
# -avoid-version prevents gratuitous .0.0.0 version numbers on the end
# _ladir passes a dummy rpath to libtool so the thing will actually link
# TODO: -nostdlib/-Bstatic/-lgcc platform magic, not installing the .a, etc.
####################################################
####################################################
AM_CFLAGS = @XORG_CFLAGS@
smi750ddk_drv_la_LTLIBRARIES = smi750ddk_drv.la
smi750ddk_drv_la_LDFLAGS = -module -avoid-version
smi750ddk_drv_ladir = @moduledir@/drivers
INCLUDES = -I./regdef/ -I./libsrc
#DEFS = -DLINUX

smi750ddk_drv_la_SOURCES = \
              ./xorg.c \
		./750ddk_module.c \
		./libsrc/2d.c \
		./libsrc/2d.h \
		./libsrc/alpha.c \
		./libsrc/alpha.h \
		./libsrc/at25f512.c \
		./libsrc/at25f512.h \
		./libsrc/bitmap.c \
		./libsrc/bitmap.h \
		./libsrc/burst.c \
		./libsrc/burst.h \
		./libsrc/capture.c \
		./libsrc/capture.h \
		./libsrc/chip.c \
		./libsrc/chip.h \
		./libsrc/clock.c \
		./libsrc/clock.h \
		./libsrc/csc.c \
		./libsrc/csc.h \
		./libsrc/cursor.c \
		./libsrc/cursor.h \
		./libsrc/ddkdebug.c \
		./libsrc/ddkdebug.h \
		./libsrc/deint.c \
		./libsrc/deint.h \
		./libsrc/dispint.c \
		./libsrc/dispint.h \
		./libsrc/display.c \
		./libsrc/display.h \
		./libsrc/dma.c \
		./libsrc/dma.h \
		./libsrc/font.c \
		./libsrc/font.h \
		./libsrc/hardware.c \
		./libsrc/hardware.h \
		./libsrc/mode.c \
		./libsrc/mode.h \
		./libsrc/os.h \
		./libsrc/panning.c \
		./libsrc/panning.h \
		./libsrc/power.c \
		./libsrc/power.h \
		./libsrc/ssp.c \
		./libsrc/ssp.h \
		./libsrc/sw2d.c \
		./libsrc/sw2d.h \
		./libsrc/swi2c.c \
		./libsrc/swi2c.h \
		./libsrc/hwi2c.c \
		./libsrc/hwi2c.h \
		./libsrc/valpha.c \
		./libsrc/valpha.h \
		./libsrc/version.h \
		./libsrc/video.c \
		./libsrc/video.h \
		./libsrc/sii164.c \
		./libsrc/sii164.h \
		./libsrc/dvi.c \
		./libsrc/dvi.h \
		./libsrc/dviint.c \
		./libsrc/dviint.h \
		./libsrc/gpioint.h \
		./libsrc/gpioint.c \
		./libsrc/intr.c \
		./libsrc/intr.h \
		./libsrc/edid.c \
		./libsrc/edid.h 

#		./libsrc/vcoder.c 
#		./libsrc/saa7118.c 
#		./libsrc/saa7118.h 
#            ./libsrc/os.c
# below file was added by ddk 1.2

