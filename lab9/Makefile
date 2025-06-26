# === Thông tin ===
KERNEL_MODULE := crypto_driver
CLIENT := client_chat
SERVER := chat_server

# Source code
CLIENT_SRCS := menu_client.c user.c crypto_interface.c
SERVER_SRCS := chat_server.c

# Kernel build
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m += $(KERNEL_MODULE).o

# === Mặc định ===
all: $(KERNEL_MODULE).ko $(CLIENT) $(SERVER)

# === Build kernel module ===
$(KERNEL_MODULE).ko:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# === Build client ===
$(CLIENT): $(CLIENT_SRCS)
	gcc -o $(CLIENT) $(CLIENT_SRCS) -lpthread

# === Build server ===
$(SERVER): $(SERVER_SRCS)
	gcc -o $(SERVER) $(SERVER_SRCS) -lpthread

# === Clean ===
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *.o *.ko *.mod.* *.order *.symvers $(CLIENT) $(SERVER)
