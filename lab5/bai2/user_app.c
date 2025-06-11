#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#define DEVICE_PATH "/dev/lab52"
#define BUFFER_SIZE 1024

// Định nghĩa các loại thời gian (phải khớp với driver)
#define TIME_TYPE_MICRO   1
#define TIME_TYPE_NANO    2
#define TIME_TYPE_RELATIVE 3

// Hàm hiển thị menu
void display_menu() {
    printf("\n=== MENU CHỨC NĂNG THỜI GIAN ===\n");
    printf("1. Lấy thời gian tuyệt đối, chính xác đến micro giây\n");
    printf("2. Lấy thời gian tuyệt đối, chính xác đến nano giây\n");
    printf("3. Lấy thời gian tương đối\n");
    printf("4. Kết thúc\n");
    printf("Chọn: ");
}

// Hàm lấy thời gian từ driver
int get_time_from_driver(int time_type) {
    int fd;
    char time_type_str[16];
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    // Mở device
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Lỗi khi mở thiết bị");
        printf("Hãy chắc chắn rằng driver đã được load và device file tồn tại.\n");
        printf("Chạy lệnh: sudo insmod lab52_nguyen_van_a.ko\n");
        return -1;
    }
    
    // Gửi loại thời gian đến driver
    sprintf(time_type_str, "%d", time_type);
    if (write(fd, time_type_str, strlen(time_type_str)) < 0) {
        perror("Lỗi khi ghi vào thiết bị");
        close(fd);
        return -1;
    }
    
    // Reset offset để đọc từ đầu
    lseek(fd, 0, SEEK_SET);
    
    // Đọc kết quả từ driver
    bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("\n=== KẾT QUẢ TỪ KERNEL ===\n");
        printf("%s", buffer);
    } else {
        printf("Không thể đọc dữ liệu từ thiết bị\n");
    }
    
    close(fd);
    return 0;
}

// Hàm hiển thị thời gian user space để so sánh
void show_userspace_time() {
    struct timeval tv;
    struct timespec ts;
    time_t rawtime;
    struct tm *timeinfo;
    
    printf("\n=== SO SÁNH VỚI USER SPACE ===\n");
    
    // gettimeofday (microsecond precision)
    gettimeofday(&tv, NULL);
    printf("User space gettimeofday(): %ld.%06ld seconds\n", 
           tv.tv_sec, tv.tv_usec);
    
    // clock_gettime (nanosecond precision)
    clock_gettime(CLOCK_REALTIME, &ts);
    printf("User space clock_gettime(): %ld.%09ld seconds\n", 
           ts.tv_sec, ts.tv_nsec);
    
    // Formatted time
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    printf("Formatted time: %s", asctime(timeinfo));
}

int main() {
    int choice;
    
    printf("Chương trình điều khiển thiết bị thời gian lab5.2_nguyen_van_a\n");
    printf("Driver cung cấp thời gian chính xác từ kernel space\n");
    
    while (1) {
        display_menu();
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: // Thời gian tuyệt đối - microsecond
                printf("\n--- THỜI GIAN TUYỆT ĐỐI (MICROSECOND) ---\n");
                if (get_time_from_driver(TIME_TYPE_MICRO) == 0) {
                    show_userspace_time();
                }
                break;
                
            case 2: // Thời gian tuyệt đối - nanosecond  
                printf("\n--- THỜI GIAN TUYỆT ĐỐI (NANOSECOND) ---\n");
                if (get_time_from_driver(TIME_TYPE_NANO) == 0) {
                    show_userspace_time();
                }
                break;
                
            case 3: // Thời gian tương đối
                printf("\n--- THỜI GIAN TƯƠNG ĐỐI ---\n");
                get_time_from_driver(TIME_TYPE_RELATIVE);
                break;
                
            case 4: // Kết thúc
                printf("Kết thúc chương trình. Tạm biệt!\n");
                exit(0);
                break;
                
            default:
                printf("Lựa chọn không hợp lệ! Vui lòng chọn từ 1-4.\n");
                break;
        }
        
        printf("\nNhấn Enter để tiếp tục...");
        getchar(); // Đọc newline còn lại
        getchar(); // Chờ user nhấn Enter
    }
    
    return 0;
}