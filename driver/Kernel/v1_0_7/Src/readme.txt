#
# useful imformation about sm750 kernel frame buffer driver
# please add your signature if you modified this readme file
# like format: modified at 20xx-x-x by xx
# Good luck 2 U
#
--mod at 2010-03-18 bt monk.liu
--mod at 2009-11-03 by monk
--mod at 2009-08-21 by monk 
--mod at 2009-08-25 by monk
--mod at 2009-08-21 by monk
--first wrote at 2009-05-27 by monk.liu

1,How to install driver with kernel source tree ?
	A: Overwrite the "Kconfig" & "Makefile" under "linux-2.6.28/drivers/video" with ours
	B: Copy subdir "sm750" into "linux-2.6.28/drivers/video/"
	(for other kernel version ,you need to modify their Kconfig and Makefile files)

2.1,How to build the kernel with sm750 framebuffer support?
	A: get in linux-2.6.28 source directory
	B: type "make menuconfig"
	C: choose "Device Driver" ==> "framebuffer support"
	D: find and select "sm750xxxx" (either * or M is okay)
	E: Save && exit ,then type "make all"
	options:
	1, You can select "support debug message for sm750 framebuffer driver" if you 
	   need debug message.

2.2,how to build driver into kernel module?
	Go to "sm750" folder and type "make". 
	then you will get sm750fb.ko under both "sm750" folder and /lib/modules/x.x.x/extra/ folder
	Note:you should set boot parameter with
	"video=sm750fb:800x600-8@60" strings if you want kernel boot with sm750 driver.

	* Hardware I2C will be used by default.
	* If you want to use software iic, please type "make swi2c=1".
	Note:this I2C is an internal stuff to program sii164 chip.kernel won't be aware of it.
	* If your platform does not attach sii164,you need not want to build the driver with
	dvi chip source file ,so just type:	"make nodvi=1"
	Note:ddk_dvi.c and ddk_sii164.c will be built into module by default,
	"nodvi=1" will exclude both ddk_dvi.c and ddk_sii164.c

3,How to run linux-2.6.28 with sm750 framebuffer driver?
	Mostly in /etc/grub.conf,please append below parameters
	"video=sm750fb:WxH-bpp@Hz "	after "kernel /vmlinuz-2.6.28 ro root=xxx ..." 
	e.g. "video=sm750fb:800x600-16@60" 
	( for module, come into "sm750" dir ,after typing "insmod ./sm751fb.ko" , you will immediatly use 
	  sm750 kernel in default mode.
	  by adding "mode_option=WxH-bpp@hz" to specify a mode.
	  e.g. "insmod ./sm750fb.ko mode_option=800x600-16@60" )
	
4,How to run kernel without 2d acceleration?
	change *config string* like below:
	"video=sm750fb:noaccel,800x600-16@60"
	(for module,add "noaccel=1")

5,how to show dump register
	add string "dumpreg" into boot-up parameter. eg.video=sm750fb:800x600-16@60,dumpreg
	(for module,add "dumpreg=1")

6,how to turn off mtrr
	add string "nomtrr" into boot-up parameter
	(for module,add "nomtrr=1")

7,select output combination option
	A,simul TFT(expansion) + CRT
	  add string "tft+crt" into boot up parameter
	  add expansion string "exp:WxH" into boot up parameter eg. "exp:1280x1024"
	  add TFT type string into boot up paramter like below:
	  "18bit" or "24bit" or "36bit" 	
	
	B,simul TFT + CRT
	  add string "tft+crt" into boot up parameter
	  add TFT type string like above mentioned

	C,simul TFT + TFT	(dual 18 bit panel )
	  add string "tft1+tft2" into boot up parameter

	D,simul CRT + CRT	(it's a default option)
	  add string "crt1+crt2" into boot up paramter
	
	e.g. for option A:
	"video=sm750fb:800x600-16@60,exp:1280x1024,tft+crt,24bit"
		
	(for module,parameters description is below:
	output=[0|1|2] 
		0:"tft+crt"
		1:"tft1+tft2"
		2:"crt1+crt2"

	exp_res=WxH

	pnltype=[0|1|2]
		0:24bit 
		1:18bit
		2:36bit
	e.g.
	"insmod ./sm750fb.ko mode_option=800x600-16@60 exp_res=1280x1024 output=0 pnltype=0" )

w
		 
note:	1,If none of above option is typed,option D will be used as default
	2,"pnltype=2" and "36bit"  means double pixel panel
		

	


	

	
	

	
