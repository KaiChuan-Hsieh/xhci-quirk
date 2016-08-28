This kernel module is for ASUS X550LB that usb enumerate bluetooth
device failed.

The port for bluetooth device tries to enable xhci after usb bus resume.
It sometimes cannot enumerate usb device, then make bluetooth function
broken.

Please use the following steps to complete installation.

make
make install
reboot
