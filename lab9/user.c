#include "crypto_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define DB_FILE "users.db"

void hash_password(const char *password, char *hashed, int mode) {
    int fd = open(DEVICE_PATH, O_RDWR);
    crypto_set_mode(fd, mode);
    crypto_process(fd, password, hashed);
    close(fd);
}

void create_user(const char *username, const char *password, int hash_mode) {
    FILE *f = fopen(DB_FILE, "a");
    char hashed[1024] = {};
    hash_password(password, hashed, hash_mode);
    fprintf(f, "%s:%s\n", username, hashed);
    fclose(f);
    printf("✅ User created!\n");
}

int login_user(const char *username, const char *password, int hash_mode) {
    FILE *f = fopen(DB_FILE, "r");
    char hashed_input[1024] = {};
    char line[1024], file_user[128], file_hash[1024];

    hash_password(password, hashed_input, hash_mode);

    while (fgets(line, sizeof(line), f)) {
        sscanf(line, "%127[^:]:%1023s", file_user, file_hash);
        if (strcmp(username, file_user) == 0 &&
            strncmp(hashed_input, file_hash, strlen(file_hash)) == 0) {
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}
