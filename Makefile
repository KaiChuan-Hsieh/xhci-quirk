# Please add insmod disable_mmc.ko to rc.local or /etc/modules
obj-m = xhci-quirk.o

all:
	make -C /lib/modules/`uname -r`/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/`uname -r`/build M=$(shell pwd) clean

install:
	sudo mkdir -p /lib/modules/'uname -r'/update
	sudo cp -p xhci-quirk.ko /lib/modules/'uname -r'/update
	sudo cp rc.local /etc/rc.local
