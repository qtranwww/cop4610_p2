obj-m += elevator.o
elevator-objs := src/module.o src/pets.o src/thread.o src/proc.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	make -C tests/elevator-test
	cd tests/system-calls-test && make syscheck.ko
	gcc tests/system-calls-test/test-syscalls.c -o tests/system-calls-test/test-syscalls

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	make -C tests/elevator-test clean
	make -C tests/system-calls-test clean

.PHONY: all clean
