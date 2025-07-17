obj-m := chardev.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

insert:
	sudo insmod chardev.ko

remove:
	sudo rmmod chardev

clean-dmesg:
	sudo dmesg -c
status:
	sudo lsmod | head -n 2
dmesg:
	sudo dmesg -t | tail -n 10