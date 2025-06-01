# To run code
```bash
make
sudo insmod lab4_2.ko param_major=300
dmesg # to get major number
gcc user_app.c -o user_app
sudo ./user_app
sudo rmmod lab4_2
```
# Optional to run with other device name
```bash
sudo mknod /dev/<device_name> c <major_number> 0
sudo chmod 666 /dev/<device_name>
# change device name in user_app.c
# recompile user_app.c
# run again
```


