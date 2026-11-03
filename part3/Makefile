obj-m += elevator.o
elevator-objs := src/module.o src/pets.o src/thread.o src/proc.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(CURDIR)

all: elevator.ko

elevator.ko:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

.PHONY: all clean

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
