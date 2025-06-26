#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define DEVICE "/dev/cryptdev"
#define IOCTL_SET_ALGO     _IOW('k', 1, int)
#define IOCTL_SET_KEY      _IOW('k', 2, struct key_data)
#define IOCTL_ENCRYPT      _IOR('k', 3, int)
#define IOCTL_DECRYPT      _IOR('k', 4, int)

enum {
    ALGO_DES = 0,
    ALGO_AES
};

struct key_data {
    char key[32];
    int keylen;
};

char buffer[1024];
int fd;

void flush_stdin() {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

void print_hex(const char *data, int len) {
    for (int i = 0; i < len; i++)
        printf("%02x ", (unsigned char)data[i]);
    printf("\n");
}

struct key_data get_key(int keylen) {
    struct key_data kd;
    kd.keylen = keylen;
    memset(kd.key, 0, sizeof(kd.key));
    printf("Nhập khóa (%d bytes): ", keylen);
    fgets(kd.key, sizeof(kd.key), stdin);
    size_t len = strlen(kd.key);
    if (len > 0 && kd.key[len - 1] == '\n')
        kd.key[len - 1] = '\0';
    else
        flush_stdin();
    return kd;
}

void menu() {
    puts("\n=== MENU ===");
    puts("1. Nhập vào 1 xâu");
    puts("2. Mã hoá dùng DES");
    puts("3. Mã hoá dùng AES");
    puts("4. Giải mã dùng DES");
    puts("5. Giải mã dùng AES");
    puts("6. Kết thúc");
    printf("Lựa chọn: ");
}

int main() {
    int choice, keylen;
    struct key_data kd;
    ssize_t len;

    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Không thể mở thiết bị");
        return 1;
    }

    while (1) {
        menu();
        if (scanf("%d", &choice) != 1) {
            flush_stdin();
            printf("Lỗi nhập.\n");
            continue;
        }
        flush_stdin();

        switch (choice) {
        case 1:
            printf("Nhập xâu: ");
            fgets(buffer, sizeof(buffer), stdin);
            size_t slen = strlen(buffer);
            if (buffer[slen - 1] == '\n') buffer[slen - 1] = '\0';
            write(fd, buffer, strlen(buffer));
            printf("Đã ghi xâu: \"%s\"\n", buffer);
            break;

        case 2: // Encrypt DES
            ioctl(fd, IOCTL_SET_ALGO, &((int){ALGO_DES}));
            kd = get_key(8);
            ioctl(fd, IOCTL_SET_KEY, &kd);
            ioctl(fd, IOCTL_ENCRYPT, 0);
            // lseek(fd, 0, SEEK_SET);
            // len = read(fd, buffer, sizeof(buffer));
            // printf("Xâu mã hoá (DES - hex): ");
            // print_hex(buffer, len);
            break;

        case 3: // Encrypt AES
            ioctl(fd, IOCTL_SET_ALGO, &((int){ALGO_AES}));
            printf("Chọn độ dài khóa AES (16, 24, 32): ");
            if (scanf("%d", &keylen) != 1) {
                flush_stdin();
                printf("Lỗi nhập độ dài khóa.\n");
                break;
            }
            flush_stdin();
            if (keylen != 16 && keylen != 24 && keylen != 32) {
                printf("Độ dài không hợp lệ.\n");
                break;
            }
            kd = get_key(keylen);
            ioctl(fd, IOCTL_SET_KEY, &kd);
            ioctl(fd, IOCTL_ENCRYPT, 0);
            // lseek(fd, 0, SEEK_SET);
            // len = read(fd, buffer, sizeof(buffer));
            // printf("Xâu mã hoá (AES - hex): ");
            // print_hex(buffer, len);
            break;

        case 4: // Decrypt DES
            ioctl(fd, IOCTL_SET_ALGO, &((int){ALGO_DES}));
            kd = get_key(8);
            ioctl(fd, IOCTL_SET_KEY, &kd);
            if (ioctl(fd, IOCTL_DECRYPT, 0) != 0) {
                printf("Giải mã thất bại (DES).\n");
                break;
            }
            lseek(fd, 0, SEEK_SET);
            len = read(fd, buffer, sizeof(buffer) - 1);
            if (len > 0) {
                buffer[len] = '\0';
                printf("Xâu giải mã (DES): %s\n", buffer);
            }
            break;

        case 5: // Decrypt AES
            ioctl(fd, IOCTL_SET_ALGO, &((int){ALGO_AES}));
            printf("Chọn độ dài khóa AES (16, 24, 32): ");
            if (scanf("%d", &keylen) != 1) {
                flush_stdin();
                printf("Lỗi nhập.\n");
                break;
            }
            flush_stdin();
            if (keylen != 16 && keylen != 24 && keylen != 32) {
                printf("Độ dài không hợp lệ.\n");
                break;
            }
            kd = get_key(keylen);
            ioctl(fd, IOCTL_SET_KEY, &kd);
            if (ioctl(fd, IOCTL_DECRYPT, 0) != 0) {
                printf("Giải mã thất bại (AES).\n");
                break;
            }
            lseek(fd, 0, SEEK_SET);
            len = read(fd, buffer, sizeof(buffer) - 1);
            if (len > 0) {
                buffer[len] = '\0';
                printf("Xâu giải mã (AES): %s\n", buffer);
            }
            break;

        case 6:
            close(fd);
            return 0;

        default:
            printf("Lựa chọn không hợp lệ.\n");
        }
    }
}
