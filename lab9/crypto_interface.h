// crypto_interface.h - Giao tiếp với driver mã hóa từ user space
#ifndef CRYPTO_INTERFACE_H
#define CRYPTO_INTERFACE_H

#include "crypto_ioctl.h"

#define DEVICE_PATH "/dev/crypto_driver"

int crypto_set_mode(int fd, int mode);
int crypto_process(int fd, const char *input, char *output);
int crypto_set_key(int fd, const char *key16);

#endif
