# Tên module
obj-m := crypt_driver.o

# Tên file thực thi user-space
USER_APP := main

# Đường dẫn kernel headers
KDIR := /lib/modules/$(shell uname -r)/build

# Thư mục hiện tại
PWD := $(shell pwd)

all: $(USER_APP) kernel

# Biên dịch chương trình người dùng
$(USER_APP): main.c
	$(CC) -o $@ $<

# Biên dịch kernel module
kernel:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(RM) -f $(USER_APP)

run: all
	sudo insmod crypt_driver.ko || true
	sudo mknod -m 666 /dev/cryptdev c $(shell grep cryptdev /proc/devices | awk '{print $$1}') 0 || true
	sudo ./main
