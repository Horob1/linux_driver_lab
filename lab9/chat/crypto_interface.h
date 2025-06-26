#ifndef CRYPTO_INTERFACE_H
#define CRYPTO_INTERFACE_H

#define DEVICE_PATH "/dev/crypto_driver"
#define IOCTL_SET_MODE _IOW('c', 1, int)

enum crypto_mode {
    MODE_AES,
    MODE_DES,
    MODE_MD5,
    MODE_SHA1,
    MODE_SHA256
};

// Đặt chế độ mã hóa hoặc băm
int crypto_set_mode(int fd, int mode);

// Gửi dữ liệu cần xử lý qua driver và đọc kết quả về
int crypto_process(int fd, const char *input, char *output);

#endif
