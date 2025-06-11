#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE_PATH "/dev/lab51"
#define BUFFER_SIZE 256

// Hàm hiển thị menu
void display_menu() {
    printf("\n=== MENU CHỨC NĂNG ===\n");
    printf("1. Open thiết bị\n");
    printf("2. Nhập số hệ 10 và ghi ra thiết bị số hệ 10, 2, 8, 16\n");
    printf("3. Đọc số hệ 2\n");
    printf("4. Đọc số hệ 8\n");
    printf("5. Đọc số hệ 16\n");
    printf("6. Đóng thiết bị và kết thúc\n");
    printf("Chọn: ");
}

// Hàm đọc dữ liệu từ thiết bị và trả về theo dòng
void read_device_data(int fd, char lines[][64]) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    // Reset file offset
    lseek(fd, 0, SEEK_SET);
    
    bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        
        // Phân tách dữ liệu theo dòng
        char *token = strtok(buffer, "\n");
        int line_count = 0;
        
        while (token != NULL && line_count < 4) {
            strcpy(lines[line_count], token);
            token = strtok(NULL, "\n");
            line_count++;
        }
    }
}

int main() {
    int fd = -1;
    int choice;
    int decimal_num;
    char buffer[BUFFER_SIZE];
    char lines[4][64]; // Lưu 4 dòng: decimal, binary, octal, hex
    
    printf("Chương trình điều khiển thiết bị lab5.1_nguyen_van_a\n");
    
    while (1) {
        display_menu();
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: // Open thiết bị
                if (fd != -1) {
                    printf("Thiết bị đã được mở trước đó!\n");
                    break;
                }
                
                fd = open(DEVICE_PATH, O_RDWR);
                if (fd < 0) {
                    perror("Lỗi khi mở thiết bị");
                    printf("Hãy chắc chắn rằng driver đã được load và device file tồn tại.\n");
                    printf("Chạy lệnh: sudo insmod lab51_nguyen_van_a.ko\n");
                } else {
                    printf("Thiết bị đã được mở thành công! (File descriptor: %d)\n", fd);
                }
                break;
                
            case 2: // Nhập và ghi số
                if (fd == -1) {
                    printf("Vui lòng mở thiết bị trước (chọn option 1)!\n");
                    break;
                }
                
                printf("Nhập số hệ 10: ");
                scanf("%d", &decimal_num);
                
                // Chuyển số thành chuỗi để ghi vào thiết bị
                sprintf(buffer, "%d", decimal_num);
                
                if (write(fd, buffer, strlen(buffer)) < 0) {
                    perror("Lỗi khi ghi vào thiết bị");
                } else {
                    printf("Đã ghi số %d vào thiết bị thành công!\n", decimal_num);
                    
                    // Đọc và hiển thị kết quả chuyển đổi
                    read_device_data(fd, lines);
                    printf("Kết quả chuyển đổi:\n");
                    printf("  Hệ 10: %s\n", lines[0]);
                    printf("  Hệ 2:  %s\n", lines[1]);
                    printf("  Hệ 8:  %s\n", lines[2]);
                    printf("  Hệ 16: %s\n", lines[3]);
                }
                break;
                
            case 3: // Đọc số hệ 2
                if (fd == -1) {
                    printf("Vui lòng mở thiết bị trước (chọn option 1)!\n");
                    break;
                }
                
                read_device_data(fd, lines);
                printf("Số hệ 2: %s\n", lines[1]);
                break;
                
            case 4: // Đọc số hệ 8
                if (fd == -1) {
                    printf("Vui lòng mở thiết bị trước (chọn option 1)!\n");
                    break;
                }
                
                read_device_data(fd, lines);
                printf("Số hệ 8: %s\n", lines[2]);
                break;
                
            case 5: // Đọc số hệ 16
                if (fd == -1) {
                    printf("Vui lòng mở thiết bị trước (chọn option 1)!\n");
                    break;
                }
                
                read_device_data(fd, lines);
                printf("Số hệ 16: %s\n", lines[3]);
                break;
                
            case 6: // Đóng thiết bị và kết thúc
                if (fd != -1) {
                    close(fd);
                    printf("Thiết bị đã được đóng.\n");
                }
                printf("Kết thúc chương trình. Tạm biệt!\n");
                exit(0);
                break;
                
            default:
                printf("Lựa chọn không hợp lệ! Vui lòng chọn từ 1-6.\n");
                break;
        }
    }
    
    return 0;
}