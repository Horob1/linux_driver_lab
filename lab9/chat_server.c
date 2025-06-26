// chat_server.c - Server trung tâm để định tuyến tin nhắn giữa các client
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CLIENTS 10
#define MAX_BUF 1024
#define SERVER_PORT 9999

struct client_info {
    int sockfd;
    struct sockaddr_in addr;
    char username[64];
};

struct client_info clients[MAX_CLIENTS];
int client_count = 0;

pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(const char *msg, const char *sender) {
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].username, sender) != 0) {
            write(clients[i].sockfd, msg, strlen(msg));
        }
    }
    pthread_mutex_unlock(&client_mutex);
}

void *handle_client(void *arg) {
    int idx = *((int *)arg);
    char buffer[MAX_BUF];
    int len;

    // Nhận tên người dùng đầu tiên
    len = read(clients[idx].sockfd, clients[idx].username, sizeof(clients[idx].username) - 1);
    clients[idx].username[len] = '\0';
    printf("✅ Kết nối từ %s\n", clients[idx].username);

    while ((len = read(clients[idx].sockfd, buffer, sizeof(buffer))) > 0) {
        buffer[len] = '\0';
        printf("📨 %s: %s\n", clients[idx].username, buffer);

        char wrapped[MAX_BUF + 64];
        snprintf(wrapped, sizeof(wrapped), "%s: %s", clients[idx].username, buffer);
        broadcast_message(wrapped, clients[idx].username);
    }

    close(clients[idx].sockfd);
    pthread_mutex_lock(&client_mutex);
    clients[idx] = clients[client_count - 1];
    client_count--;
    pthread_mutex_unlock(&client_mutex);
    printf("❌ %s đã ngắt kết nối\n", clients[idx].username);
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, MAX_CLIENTS);

    printf("🚀 Chat server đang chạy tại cổng %d...\n", SERVER_PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        pthread_mutex_lock(&client_mutex);
        if (client_count < MAX_CLIENTS) {
            clients[client_count].sockfd = client_fd;
            clients[client_count].addr = client_addr;
            int *arg = malloc(sizeof(int));
            *arg = client_count;
            pthread_create(&(pthread_t){0}, NULL, handle_client, arg);
            client_count++;
        } else {
            printf("⚠️  Server đầy, từ chối kết nối.\n");
            close(client_fd);
        }
        pthread_mutex_unlock(&client_mutex);
    }
    return 0;
}