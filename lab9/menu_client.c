// menu_client.c - Client kết nối server trung tâm để chat với mã hóa AES và chia sẻ khóa
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

#define MAX_BUF 1024

char chat_username[128];
int chat_driver_fd;
int logged_in = 0;
int hash_mode = MODE_MD5;
int cipher_mode = MODE_AES;
char shared_key[17] = "";

void *receive_thread_func(void *arg) {
    int sockfd = *((int *)arg);
    char buffer[MAX_BUF], decrypted[MAX_BUF];

    while (1) {
        int len = read(sockfd, buffer, sizeof(buffer));
        if (len <= 0) break;
        buffer[len] = '\0';
        crypto_process(chat_driver_fd, buffer, decrypted);
        decrypted[len] = '\0';
        printf("\n📩 Tin nhắn đến: %s\n", decrypted);
        fflush(stdout);
    }
    return NULL;
}

void run_chat_client(const char *username, const char *server_ip, int server_port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUF], encrypted[MAX_BUF];
    pthread_t recv_thread;

    chat_driver_fd = open(DEVICE_PATH, O_RDWR);
    if (chat_driver_fd < 0) {
        perror("Không mở được thiết bị mã hóa");
        return;
    }

    crypto_set_mode(chat_driver_fd, cipher_mode);
    crypto_set_key(chat_driver_fd, shared_key);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Không kết nối được tới server");
        close(sockfd);
        return;
    }

    write(sockfd, username, strlen(username));
    pthread_create(&recv_thread, NULL, receive_thread_func, &sockfd);

    while (1) {
        printf("🧑‍💻 %s: ", username);
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        crypto_process(chat_driver_fd, buffer, encrypted);
        write(sockfd, encrypted, strlen(buffer));
    }

    close(sockfd);
    close(chat_driver_fd);
}

int main() {
    char buffer[MAX_BUF];
    char server_ip[64];
    int server_port;

    while (1) {
        printf("\n=== CLIENT MENU ===\n");
        printf("1. Cấu hình\n");
        printf("2. Tạo người dùng\n");
        printf("3. Đăng nhập\n");
        printf("4. Chat với server\n");
        printf("5. Thoát\n> ");

        int choice;
        scanf("%d", &choice); getchar();

        switch (choice) {
            case 1:
                printf("Thuật toán băm (0:MD5, 1:SHA1, 2:SHA256): ");
                scanf("%d", &hash_mode); getchar();
                printf("Thuật toán mã hóa (0:AES, 1:DES): ");
                scanf("%d", &cipher_mode); getchar();
                printf("Server IP: ");
                fgets(server_ip, sizeof(server_ip), stdin);
                server_ip[strcspn(server_ip, "\n")] = 0;
                printf("Server Port: ");
                scanf("%d", &server_port); getchar();
                printf("Nhập khóa AES (16 ký tự): ");
                fgets(shared_key, sizeof(shared_key), stdin);
                shared_key[strcspn(shared_key, "\n")] = 0;
                if (strlen(shared_key) != 16) {
                    printf("⚠️  Khóa phải đúng 16 ký tự!");
                    memset(shared_key, 0, sizeof(shared_key));
                } else {
                    printf("✅ Đã cấu hình.");
                }
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
                if (!logged_in || strlen(shared_key) != 16) {
                    printf("⚠️  Bạn cần đăng nhập và nhập khóa AES hợp lệ trước!\n");
                } else {
                    run_chat_client(chat_username, server_ip, server_port);
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
}