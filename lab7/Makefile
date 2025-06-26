# Tên file nguồn
KERNEL_MODULE := hash_driver
USER_APP := main

obj-m += $(KERNEL_MODULE).o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Biên dịch module và ứng dụng người dùng
all: $(USER_APP) modules

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

$(USER_APP): main.c
	gcc -o $(USER_APP) main.c

# Làm sạch
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *.o *.mod.* *.order *.symvers $(USER_APP)

# Nạp module + tạo device node
load: all
	sudo insmod $(KERNEL_MODULE).ko || echo "⚠️ Module có thể đã được nạp rồi."
	-sudo mknod -m 666 /dev/hashdev c $$(grep hashdev /proc/devices | awk '{print $$1}') 0 || echo "⚠️ Device node đã tồn tại."

# Gỡ module và xóa device node
unload:
	sudo rmmod $(KERNEL_MODULE)
	sudo rm -f /dev/hashdev
