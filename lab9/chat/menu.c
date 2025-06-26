#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "crypto_interface.h"

extern void create_user(const char *, const char *, int);
extern int login_user(const char *, const char *, int);

#define PORT 8080

char chat_username[128];
int chat_driver_fd;
int chat_sockfd;

void *listen_thread_func(void *arg) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[1024], decrypted[1024];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) continue;

        int len = read(client_fd, buffer, sizeof(buffer));
        if (len > 0) {
            crypto_process(chat_driver_fd, buffer, decrypted);
            for (int i = 0; i < len; i++) decrypted[i] ^= 0xAA;
            decrypted[len] = '\0';
            printf("\n📩 Tin nhắn đến: %s\n🧑‍💻 Bạn: ", decrypted);
            fflush(stdout);
        }
        close(client_fd);
    }
    return NULL;
}

void run_chat_session(const char *username) {
    struct sockaddr_in addr;
    char buffer[1024], encrypted[1024], ip[100];
    pthread_t listen_thread;

    chat_driver_fd = open(DEVICE_PATH, O_RDWR);
    if (chat_driver_fd < 0) {
        perror("Không mở được thiết bị mã hóa");
        return;
    }
    crypto_set_mode(chat_driver_fd, MODE_AES);

    pthread_create(&listen_thread, NULL, listen_thread_func, NULL);

    printf("Nhập IP của người nhận (ví dụ: 127.0.0.1): ");
    fgets(ip, sizeof(ip), stdin);
    ip[strcspn(ip, "\n")] = 0;

    while (1) {
        chat_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);
        inet_pton(AF_INET, ip, &addr.sin_addr);

        if (connect(chat_sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("Không kết nối được tới người nhận");
            sleep(1);
            continue;
        }

        printf("🧑‍💻 %s: ", username);
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        for (int i = 0; i < strlen(buffer); i++)
            encrypted[i] = buffer[i] ^ 0xAA;

        write(chat_sockfd, encrypted, strlen(buffer));
        close(chat_sockfd);
    }

    close(chat_driver_fd);
}

int main() {
    int hash_mode = MODE_MD5;
    int cipher_mode = MODE_AES;
    int logged_in = 0;
    char buffer[1024];

    while (1) {
        printf("\n=== MENU ===\n");
        printf("1. Cấu hình thuật toán\n");
        printf("2. Tạo người dùng\n");
        printf("3. Đăng nhập\n");
        printf("4. Chat\n");
        printf("5. Thoát\n> ");

        int choice;
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                printf("Thuật toán băm (0:MD5, 1:SHA1, 2:SHA256): ");
                scanf("%d", &hash_mode); getchar();
                printf("Thuật toán mã hóa (0:AES, 1:DES): ");
                scanf("%d", &cipher_mode); getchar();
                printf("✅ Đã cấu hình.\n");
                break;

            case 2:
                printf("Tên đăng nhập: ");
                fgets(chat_username, sizeof(chat_username), stdin);
                printf("Mật khẩu: ");
                fgets(buffer, sizeof(buffer), stdin);

                chat_username[strcspn(chat_username, "\n")] = 0;
                buffer[strcspn(buffer, "\n")] = 0;

                create_user(chat_username, buffer, hash_mode);
                break;

            case 3:
                printf("Tên đăng nhập: ");
                fgets(chat_username, sizeof(chat_username), stdin);
                printf("Mật khẩu: ");
                fgets(buffer, sizeof(buffer), stdin);

                chat_username[strcspn(chat_username, "\n")] = 0;
                buffer[strcspn(buffer, "\n")] = 0;

                logged_in = login_user(chat_username, buffer, hash_mode);
                if (logged_in)
                    printf("✅ Đăng nhập thành công!\n");
                else
                    printf("❌ Sai tài khoản hoặc mật khẩu!\n");
                break;

            case 4:
                if (!logged_in) {
                    printf("⚠️  Bạn cần đăng nhập trước!\n");
                } else {
                    run_chat_session(chat_username);
                }
                break;

            case 5:
                printf("👋 Tạm biệt!\n");
                return 0;

            default:
                printf("⚠️  Lựa chọn không hợp lệ.\n");
                break;
        }
    }

    return 0;
}
