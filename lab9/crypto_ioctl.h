// crypto_ioctl.h - IOCTL định nghĩa cho driver mã hóa
#ifndef CRYPTO_IOCTL_H
#define CRYPTO_IOCTL_H

#include <linux/ioctl.h>

#define IOCTL_SET_MODE _IOW('c', 1, int)
#define IOCTL_SET_KEY  _IOW('c', 2, char[16])

enum crypto_mode {
    MODE_AES = 0,
    MODE_DES = 1,
    MODE_MD5 = 2,
    MODE_SHA1 = 3,
    MODE_SHA256 = 4
};

#endif // CRYPTO_IOCTL_H