obj-m := lab5_2.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	sudo insmod lab5_2.ko

remove:
	sudo rmmod lab5_2

user:
	gcc -o user_app user_app.c

.PHONY: all clean install remove user