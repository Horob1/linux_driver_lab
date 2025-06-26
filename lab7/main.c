#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

void hash_password(char *password, int algo, char *output) {
    int fd = open("/dev/hashdev", O_RDWR);
    if (fd < 0) {
        perror("Open device failed");
        exit(1);
    }

    ioctl(fd, 0, algo);
    write(fd, password, strlen(password));
    read(fd, output, 128);

    close(fd);
}

void menu() {
    int choice;
    char username[50], password[50], hashed[128];
    int algo;
    FILE *fp;

    do {
        printf("\n1. Nhập danh sách người dùng\n");
        printf("2. Kiểm tra đăng nhập\n");
        printf("3. Kết thúc\n");
        printf("Lựa chọn: ");
        scanf("%d", &choice);
        getchar(); // remove newline

        switch (choice) {
            case 1:
                fp = fopen("users.txt", "a");
                printf("Tên đăng nhập: "); fgets(username, sizeof(username), stdin);
                printf("Mật khẩu: "); fgets(password, sizeof(password), stdin);
                username[strcspn(username, "\n")] = 0;
                password[strcspn(password, "\n")] = 0;

                for (algo = 0; algo < 3; algo++) {
                    hash_password(password, algo, hashed);
                    fprintf(fp, "%s;%d;%s\n", username, algo, hashed);
                }

                fclose(fp);
                printf("Đã lưu người dùng.\n");
                break;

            case 2:
                printf("Thuật toán (0-MD5, 1-SHA1, 2-SHA256): ");
                scanf("%d", &algo); getchar();
                printf("Tên đăng nhập: "); fgets(username, sizeof(username), stdin);
                printf("Mật khẩu: "); fgets(password, sizeof(password), stdin);
                username[strcspn(username, "\n")] = 0;
                password[strcspn(password, "\n")] = 0;

                hash_password(password, algo, hashed);

                fp = fopen("users.txt", "r");
                char line[256], file_user[50], file_hash[128];
                int file_algo, found = 0;
                while (fgets(line, sizeof(line), fp)) {
                    sscanf(line, "%[^;];%d;%s", file_user, &file_algo, file_hash);
                    if (strcmp(username, file_user) == 0 && file_algo == algo && strcmp(file_hash, hashed) == 0) {
                        found = 1;
                        break;
                    }
                }
                fclose(fp);
                if (found)
                    printf("✅ Đăng nhập thành công!\n");
                else
                    printf("❌ Sai thông tin!\n");
                break;

            case 3:
                printf("Thoát...\n");
                break;
        }
    } while (choice != 3);
}

int main() {
    menu();
    return 0;
}
