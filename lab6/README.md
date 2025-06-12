#!/bin/bash

echo "=== BIÊN DỊCH VÀ CÀI ĐẶT CRYPTO DRIVER ==="

# Biên dịch driver
make

# Cài đặt driver
sudo insmod lab6.ko

# Xem log:
sudo dmesg | tail -n 10

# Thiết lập quyền truy cập
sudo chmod 666 /dev/lab6

# Biên dịch chương trình người dùng
gcc -o user_app user_app.c

# Chạy chương trình:
./user_app
# Gỡ bỏ driver:
sudo rmmod lab6
sudo rm /dev/lab6

