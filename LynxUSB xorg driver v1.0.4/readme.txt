how to install:
1.enter the driver's folder(for example:cd /home/qa/Desktop/ubuntu1404).
2.use command "sh ./install.dual" to install (or sh ./install.clone).
(sh ./install.dual for xrandr , sh ./install.clone for norandr.)
after installation,the .conf file will be keep in /etc/X11 automatically.
3.when the message shows installation completed,use command "reboot" to restart OS,then the screen will be lighted.
how to uninstall:
1.cd /usr/lib/xorg/modules/drivers ,then delete smi_drv.so file.
2.switch to console,use command "service lightdm stop"to disable X,
  use command "lsmod | grep ufb"to check wether there is ufb information,
  if there is a ufb information,use command "rmmod ufb" ,
  use command "service lightdm start" to enter X, and it is uninstalled successfully.