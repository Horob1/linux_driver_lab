// crypto_interface.c - Giao tiếp với driver mã hóa
#include "crypto_interface.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

int crypto_set_mode(int fd, int mode) {
    return ioctl(fd, IOCTL_SET_MODE, &mode);
}

int crypto_set_key(int fd, const char *key16) {
    return ioctl(fd, IOCTL_SET_KEY, key16);
}

int crypto_process(int fd, const char *input, char *output) {
    write(fd, input, strlen(input));
    int len = read(fd, output, 1024);
    output[len] = '\0';
    return len;
}