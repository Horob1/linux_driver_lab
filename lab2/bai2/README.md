# Lab 2: part 2 String Operations Kernel Module

## To run code
```bash
make
sudo insmod bai2.ko input="___hello___kernel___world__"
sudo rmmod bai2
```
## Output
```bash
[lab2:part2] String module loaded
[lab2:part2] Original string: "___hello___kernel___world__" # becouse insmod can't pass space in parameter so we use _ instead of space
#...
[lab2:part2] String module unloaded.
```