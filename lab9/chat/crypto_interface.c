#include "crypto_interface.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

int crypto_set_mode(int fd, int mode) {
    return ioctl(fd, IOCTL_SET_MODE, mode);
}

int crypto_process(int fd, const char *input, char *output) {
    write(fd, input, strlen(input));
    return read(fd, output, 1024);
}
