#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "crypto_interface.h"

#define DB_FILE "users.db"

// Băm mật khẩu bằng driver với thuật toán do người dùng chọn
void hash_password(const char *password, char *hashed, int mode) {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Không mở được driver");
        strcpy(hashed, "");
        return;
    }

    crypto_set_mode(fd, mode);
    memset(hashed, 0, 1024);
    crypto_process(fd, password, hashed);
    close(fd);
}

// Tạo người dùng mới
void create_user(const char *username, const char *password, int hash_mode) {
    FILE *f = fopen(DB_FILE, "a");
    if (!f) {
        perror("Không mở được file user");
        return;
    }

    char hashed[1024] = {0};
    hash_password(password, hashed, hash_mode);
    fprintf(f, "%s:%s\n", username, hashed);
    fclose(f);
    printf("✅ Tạo tài khoản thành công!\n");
}

// Kiểm tra đăng nhập
int login_user(const char *username, const char *password, int hash_mode) {
    FILE *f = fopen(DB_FILE, "r");
    if (!f) {
        perror("Không mở được file user");
        return 0;
    }

    char hashed_input[1024] = {0};
    hash_password(password, hashed_input, hash_mode);

    char line[1024], file_user[128], file_hash[1024];
    while (fgets(line, sizeof(line), f)) {
        sscanf(line, "%127[^:]:%1023s", file_user, file_hash);
        if (strcmp(username, file_user) == 0 &&
            strncmp(hashed_input, file_hash, strlen(file_hash)) == 0) {
            fclose(f);
            return 1; // Đúng tài khoản và mật khẩu
        }
    }

    fclose(f);
    return 0; // Sai
}
