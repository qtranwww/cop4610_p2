obj-m += elevator.o
elevator-objs := src/module.o src/pets.o src/thread.o src/proc.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	make -C tests/elevator-test
	make -C tests/system-calls-test

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	make -C tests/elevator-test clean
	make -C tests/system-calls-test clean

.PHONY: all clean
