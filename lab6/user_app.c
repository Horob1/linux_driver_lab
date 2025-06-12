#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ctype.h>

#define DEVICE "/dev/lab6"

#define CRYPTO_SHIFT_ENCRYPT          _IOW('c', 1, int)
#define CRYPTO_SHIFT_DECRYPT          _IOW('c', 2, int)
#define CRYPTO_SUBSTITUTION_ENCRYPT  _IOW('c', 3, char[26])
#define CRYPTO_SUBSTITUTION_DECRYPT  _IOW('c', 4, char[26])
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

    char buffer[1024];
    int choice;
    int shift_key;
    int transposition_key;
    char subst_key[27];  // 26 ký tự + null terminator

    while (1) {
        menu();
        printf("Lựa chọn: ");
        scanf("%d", &choice);
        getchar(); // bỏ '\n' sau scanf

        switch (choice) {
            case 1:
                printf("Nhập xâu: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = '\0'; // xóa ký tự newline
                write(fd, buffer, strlen(buffer));
                break;
            case 2:
                printf("Nhập khóa dịch chuyển: ");
                scanf("%d", &shift_key);
                getchar(); // bỏ '\n' sau scanf
                ioctl(fd, CRYPTO_SHIFT_ENCRYPT, &shift_key);
                break;
            case 3:
                printf("Nhập khóa thay thế (vd: qwertyuiopasdfghjklzxcvbnm): ");
                fgets(subst_key, sizeof(subst_key), stdin);
                subst_key[strcspn(subst_key, "\n")] = '\0';
                while (strlen(subst_key) < 26) {
                    fgets(subst_key + strlen(subst_key), sizeof(subst_key) - strlen(subst_key), stdin);
                    subst_key[strcspn(subst_key, "\n")] = '\0';
                }

                ioctl(fd, CRYPTO_SUBSTITUTION_ENCRYPT, subst_key);
                break;
            case 4:
                printf("Nhập khóa hoán vị (số nguyên > 1): ");
                scanf("%d", &transposition_key);
                getchar();
                ioctl(fd, CRYPTO_TRANSPOSITION_ENCRYPT, &transposition_key);
                break;
            case 5:
                printf("Nhập khóa dịch chuyển: ");
                scanf("%d", &shift_key);
                getchar();
                ioctl(fd, CRYPTO_SHIFT_DECRYPT, &shift_key);
                break;
            case 6:
                printf("Nhập khóa hoán vị (số nguyên > 1): ");
                scanf("%d", &transposition_key);
                getchar();
                ioctl(fd, CRYPTO_TRANSPOSITION_DECRYPT, &transposition_key);
                break;
            case 7:
                printf("Nhập khóa thay thế (26 ký tự, có thể là hoa hoặc thường): ");
                fgets(subst_key, sizeof(subst_key), stdin);
                subst_key[strcspn(subst_key, "\n")] = '\0';

                while (strlen(subst_key) < 26) {
                    fgets(subst_key + strlen(subst_key), sizeof(subst_key) - strlen(subst_key), stdin);
                    subst_key[strcspn(subst_key, "\n")] = '\0';
                }

                ioctl(fd, CRYPTO_SUBSTITUTION_DECRYPT, subst_key);
                break;
            case 0:
                close(fd);
                return 0;
            default:
                printf("Lựa chọn không hợp lệ.\n");
        }

        // Đọc kết quả từ thiết bị
        lseek(fd, 0, SEEK_SET); // đặt lại offset
        int n = read(fd, buffer, sizeof(buffer) - 1);
        buffer[n] = '\0';
        printf("Kết quả: %s\n", buffer);
    }

    close(fd);
    return 0;
}
