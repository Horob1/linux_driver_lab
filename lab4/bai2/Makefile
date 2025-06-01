# Makefile cho kernel module và chương trình user-space
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build

# Tên module kernel
obj-m := lab4_2.o

# Target mặc định build kernel module
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
	gcc -o user_app user_app.c

# Clean file build
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -f user_app

.PHONY: all clean