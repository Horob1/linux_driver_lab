#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#define IOCTL_SET_ALGO     _IOW('k', 1, int)
#define IOCTL_SET_KEY      _IOW('k', 2, char*)
#define IOCTL_ENCRYPT      _IO('k', 3)
#define IOCTL_DECRYPT      _IO('k', 4)

#define BUF_SIZE 1024

// 👉 In dữ liệu hex cho dễ xem
void print_hex(const char* label, const char *data, int len) {
    printf("%s:", label);
    for (int i = 0; i < len; i++) {
        printf(" %02x", (unsigned char)data[i]);
    }
    printf("\n");
}

int main() {
    int fd;
    char input[BUF_SIZE], key[32], buffer[BUF_SIZE];
    int choice;
    int buf_len;

    fd = open("/dev/cryptdev", O_RDWR);
    if (fd < 0) {
        perror("Open device");
        return 1;
    }

    while (1) {
        printf("\n=== MENU ===\n");
        printf("1. Nhập vào 1 xâu\n");
        printf("2. Mã hoá và lưu xâu mã dùng mã DES (khóa 8 byte)\n");
        printf("3. Mã hoá và lưu xâu mã dùng mã AES (khóa 16, 24 hoặc 32 byte)\n");
        printf("4. Đọc và giải mã xâu mã dùng mã DES (khóa 8 byte)\n");
        printf("5. Đọc và giải mã xâu mã dùng mã AES (khóa 16, 24 hoặc 32 byte)\n");
        printf("6. Kết thúc\n");
        printf("Lựa chọn: ");
        scanf("%d", &choice);
        getchar();  // bỏ '\n'

        switch (choice) {
            case 1:
                printf("Nhập xâu: ");
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = 0;  // bỏ '\n'
                write(fd, input, strlen(input));
                break;

            case 2:
            case 3: {
                printf("Nhập khoá (%s): ", choice == 2 ? "8 byte" : "16, 24 hoặc 32 byte");
                fgets(key, sizeof(key), stdin);
                key[strcspn(key, "\n")] = 0;
                int key_len = strlen(key);
                if ((choice == 2 && key_len != 8) ||
                    (choice == 3 && (key_len != 16 && key_len != 24 && key_len != 32))) {
                    printf("❌ Độ dài khóa không hợp lệ!\n");
                    break;
                }

                ioctl(fd, IOCTL_SET_ALGO, choice == 2 ? 0 : 1);
                ioctl(fd, IOCTL_SET_KEY, key);
                ioctl(fd, IOCTL_ENCRYPT);

                // buf_len = read(fd, buffer, BUF_SIZE);
                // if (buf_len > 0) {
                //     print_hex("🔒 Chuỗi đã mã hoá (hex)", buffer, buf_len);
                // } else {
                //     printf("⚠️ Không đọc được dữ liệu mã hoá.\n");
                // }
                break;
            }

            case 4:
            case 5: {
                ioctl(fd, IOCTL_SET_ALGO, choice == 4 ? 0 : 1);
                printf("Nhập khoá (%s): ", choice == 4 ? "8 byte" : "16, 24 hoặc 32 byte");
                fgets(key, sizeof(key), stdin);
                key[strcspn(key, "\n")] = 0;

                int key_len = strlen(key);
                if ((choice == 4 && key_len != 8) ||
                    (choice == 5 && (key_len != 16 && key_len != 24 && key_len != 32))) {
                    printf("❌ Độ dài khóa không hợp lệ!\n");
                    break;
                }

                ioctl(fd, IOCTL_SET_KEY, key);
                if (ioctl(fd, IOCTL_DECRYPT) == 0) {
                    buf_len = read(fd, buffer, BUF_SIZE);
                    if (buf_len > 0) {
                        buffer[buf_len] = '\0';  // đảm bảo null-terminate
                        printf("✅ Chuỗi đã giải mã: %s\n", buffer);
                    } else {
                        printf("⚠️ Không đọc được dữ liệu giải mã.\n");
                    }
                } else {
                    printf("❌ Giải mã thất bại. Có thể sai khóa hoặc thuật toán.\n");
                }
                break;
            }

            case 6:
                close(fd);
                return 0;
        }
    }

    return 0;
}
