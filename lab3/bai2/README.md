# Lab 3: part 2 Cipher Operations Kernel Module

## To run code
```bash
make
sudo insmod bai2.ko XauRo="HelloWorld" k=3
sudo rmmod bai2
```
## Output
```bash
Cipher module loaded
Plain text: HelloWorld
Key for Caesar cipher: 3
Caesar Cipher: KhoorZruog
Substitution Cipher: QnuuxFyqnum
Global Permutation Cipher: loWorlHelld
Cipher module unloaded
```