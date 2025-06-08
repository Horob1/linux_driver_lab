#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE "/dev/lab6"

// Khớp với driver
#define CRYPTO_SHIFT_ENCRYPT          _IOW('c', 1, int)
#define CRYPTO_SHIFT_DECRYPT          _IOW('c', 2, int)
#define CRYPTO_SUBSTITUTION_ENCRYPT  _IO('c', 3)
#define CRYPTO_SUBSTITUTION_DECRYPT  _IO('c', 4)
#define CRYPTO_TRANSPOSITION_ENCRYPT _IOW('c', 5, int)
#define CRYPTO_TRANSPOSITION_DECRYPT _IOW('c', 6, int)

void menu() {
    printf("\n--- MENU ---\n");
    printf("1. Nhập xâu\n");
    printf("2. Mã hóa bằng dịch chuyển\n");
    printf("3. Mã hóa bằng thay thế\n");
    printf("4. Mã hóa bằng hoán vị\n");
    printf("5. Giải mã bằng dịch chuyển\n");
    printf("6. Giải mã bằng hoán vị\n");
    printf("7. Giải mã bằng thay thế\n");
    printf("0. Thoát\n");
}

int main() {
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Không mở được thiết bị");
        return 1;
    }

    int choice;
    char input[1024];
    int shift_key;

    while (1) {
        menu();
        printf("Lựa chọn: ");
        if (scanf("%d", &choice) != 1) {
            printf("Vui lòng nhập số hợp lệ!\n");
            while(getchar() != '\n'); // Xóa buffer nhập sai
            continue;
        }
        getchar(); // bỏ \n sau scanf

        switch (choice) {
            case 1:
                printf("Nhập chuỗi: ");
                if (!fgets(input, sizeof(input), stdin)) {
                    printf("Lỗi khi nhập chuỗi\n");
                    break;
                }
                input[strcspn(input, "\n")] = '\0';
                if (write(fd, input, strlen(input)) < 0) {
                    perror("Lỗi ghi vào thiết bị");
                    break;
                }
                lseek(fd, 0, SEEK_SET);  // Reset con trỏ đọc
                break;

            case 2:
                printf("Nhập khóa dịch chuyển: ");
                if (scanf("%d", &shift_key) != 1) {
                    printf("Khóa không hợp lệ\n");
                    while(getchar() != '\n');
                    break;
                }
                getchar();
                if (ioctl(fd, CRYPTO_SHIFT_ENCRYPT, &shift_key) < 0) {
                    perror("Lỗi ioctl mã hóa dịch chuyển");
                    break;
                }
                lseek(fd, 0, SEEK_SET);
                break;

            case 3:
                if (ioctl(fd, CRYPTO_SUBSTITUTION_ENCRYPT) < 0) {
                    perror("Lỗi ioctl mã hóa thay thế");
                    break;
                }
                lseek(fd, 0, SEEK_SET);
                break;

            case 4:
                printf("Nhập khóa hoán vị: ");
                if (scanf("%d", &shift_key) != 1) {
                    printf("Khóa không hợp lệ\n");
                    while(getchar() != '\n');
                    break;
                }
                getchar();
                if (ioctl(fd, CRYPTO_TRANSPOSITION_ENCRYPT, &shift_key) < 0) {
                    perror("Lỗi ioctl mã hóa hoán vị");
                    break;
                }
                lseek(fd, 0, SEEK_SET);
                break;

            case 5:
                printf("Nhập khóa dịch chuyển để giải mã: ");
                if (scanf("%d", &shift_key) != 1) {
                    printf("Khóa không hợp lệ\n");
                    while(getchar() != '\n');
                    break;
                }
                getchar();
                if (ioctl(fd, CRYPTO_SHIFT_DECRYPT, &shift_key) < 0) {
                    perror("Lỗi ioctl giải mã dịch chuyển");
                    break;
                }
                lseek(fd, 0, SEEK_SET);
                break;

            case 6:
                printf("Nhập khóa hoán vị để giải mã: ");
                if (scanf("%d", &shift_key) != 1) {
                    printf("Khóa không hợp lệ\n");
                    while(getchar() != '\n');
                    break;
                }
                getchar();
                if (ioctl(fd, CRYPTO_TRANSPOSITION_DECRYPT, &shift_key) < 0) {
                    perror("Lỗi ioctl giải mã hoán vị");
                    break;
                }
                lseek(fd, 0, SEEK_SET);
                break;

            case 7:
                if (ioctl(fd, CRYPTO_SUBSTITUTION_DECRYPT) < 0) {
                    perror("Lỗi ioctl giải mã thay thế");
                    break;
                }
                lseek(fd, 0, SEEK_SET);
                break;

            case 0:
                close(fd);
                return 0;

            default:
                printf("Lựa chọn không hợp lệ!\n");
                continue;
        }

        char buffer[1024] = {0};
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read < 0) {
            perror("Lỗi đọc dữ liệu");
        } else {
            buffer[bytes_read] = '\0';
            printf("Kết quả: %s\n", buffer);
        }
    }

    close(fd);
    return 0;
}