# Tên module và chương trình
obj-m += crypt_driver.o
USER_APP = main
DEVICE = cryptdev

# Thư mục kernel
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# ⚙️ Mặc định: build cả driver và user app
all: modules $(USER_APP)

# Build chương trình người dùng
$(USER_APP): main.c
	gcc -o $(USER_APP) main.c

# Build kernel module
modules:
	make -C $(KDIR) M=$(PWD) modules

# Nạp module + tạo /dev + phân quyền
load: modules
	@echo "🚀 Loading kernel module..."
	-sudo insmod crypt_driver.ko || echo "⚠️  Có thể module đã được nạp"
	@if [ ! -e /dev/$(DEVICE) ]; then \
		sudo mknod /dev/$(DEVICE) c $$(grep $(DEVICE) /proc/devices | awk '{print $$1}') 0; \
		echo "📦 Đã tạo /dev/$(DEVICE)"; \
	fi
	sudo chmod 666 /dev/$(DEVICE)
	dmesg | tail -10

# Gỡ module + xóa /dev
unload:
	@echo "🧹 Removing kernel module..."
	-sudo rmmod crypt_driver
	-sudo rm -f /dev/$(DEVICE)

# Chạy chương trình người dùng với sudo
run: load
	sudo ./$(USER_APP)

# Dọn sạch tất cả file sinh ra
clean:
	@echo "🧼 Đang dọn sạch..."
	make -C $(KDIR) M=$(PWD) clean
	-sudo rm -f $(USER_APP)
	rm -f *.o *.ko *.mod.* *.symvers *.order Module.*
	-sudo rm -f /dev/$(DEVICE)
