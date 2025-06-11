#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define DEVICE_NAME "lab51"
#define CLASS_NAME "lab51_class"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("vancongng");
MODULE_DESCRIPTION("Character driver for number base conversion");
MODULE_VERSION("1.0");

// Cấu trúc dữ liệu để lưu trữ số ở các hệ cơ số khác nhau
struct number_data {
    int decimal;
    char binary[33];    // Tối đa 32 bit + null terminator
    char octal[12];     // Tối đa 11 chữ số + null terminator
    char hex[9];        // Tối đa 8 chữ số + null terminator
};

// Biến toàn cục
static dev_t dev_num;           // Device number
static struct cdev *mcdev;      // Character device structure
static struct class *dev_class; // Device class
static struct device *device;  // Device
static struct number_data *data; // Dữ liệu số

// Hàm chuyển đổi decimal sang binary
static void decimal_to_binary(int decimal, char *binary) {
    int i = 0;
    int temp = decimal;
    
    if (decimal == 0) {
        strcpy(binary, "0");
        return;
    }
    
    // Xử lý số âm
    if (decimal < 0) {
        temp = -decimal;
    }
    
    // Chuyển đổi
    char temp_binary[33];
    while (temp > 0) {
        temp_binary[i++] = (temp % 2) + '0';
        temp /= 2;
    }
    
    // Đảo ngược chuỗi
    int j;
    for (j = 0; j < i; j++) {
        binary[j] = temp_binary[i - 1 - j];
    }
    
    // Thêm dấu âm nếu cần
    if (decimal < 0) {
        memmove(binary + 1, binary, j + 1);
        binary[0] = '-';
        j++;
    }
    
    binary[j] = '\0';
}

static void decimal_to_octal(int decimal, char *octal) {
    int i = 0;
    int temp = decimal;

    if (decimal == 0) {
        strcpy(octal, "0");
        return;
    }

    if (decimal < 0) {
        temp = -decimal;
    }

    char temp_octal[32];
    while (temp > 0) {
        temp_octal[i++] = (temp % 8) + '0';
        temp /= 8;
    }

    // Đảo ngược chuỗi
    int j;
    for (j = 0; j < i; j++) {
        octal[j] = temp_octal[i - 1 - j];
    }

    // Thêm dấu âm nếu cần
    if (decimal < 0) {
        memmove(octal + 1, octal, j + 1);
        octal[0] = '-';
        j++;
    }

    octal[j] = '\0';
}

static void decimal_to_hex(int decimal, char *hex) {
    int i = 0;
    int temp = decimal;

    if (decimal == 0) {
        strcpy(hex, "0");
        return;
    }

    if (decimal < 0) {
        temp = -decimal;
    }

    char temp_hex[32];
    char digits[] = "0123456789ABCDEF";

    while (temp > 0) {
        temp_hex[i++] = digits[temp % 16];
        temp /= 16;
    }

    // Đảo ngược chuỗi
    int j;
    for (j = 0; j < i; j++) {
        hex[j] = temp_hex[i - 1 - j];
    }

    // Thêm dấu âm nếu cần
    if (decimal < 0) {
        memmove(hex + 1, hex, j + 1);
        hex[0] = '-';
        j++;
    }

    hex[j] = '\0';
}


// Hàm open device
static int device_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "lab5_1: Device opened\n");
    return 0;
}

// Hàm close device
static int device_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "lab5_1: Device closed\n");
    return 0;
}

// Hàm write - nhận số decimal từ user space
static ssize_t device_write(struct file *file, const char __user *buffer, 
                           size_t length, loff_t *offset) {
    char input[32];
    int decimal_num;
    
    if (length >= sizeof(input)) {
        return -EINVAL;
    }
    
    if (copy_from_user(input, buffer, length)) {
        return -EFAULT;
    }
    
    input[length] = '\0';
    
    // Chuyển chuỗi thành số decimal
    if (kstrtoint(input, 10, &decimal_num) != 0) {
        return -EINVAL;
    }
    
    // Lưu số decimal và chuyển đổi sang các hệ cơ số khác
    data->decimal = decimal_num;
    decimal_to_binary(decimal_num, data->binary);
    decimal_to_octal(decimal_num, data->octal);
    decimal_to_hex(decimal_num, data->hex);
    
    printk(KERN_INFO "lab5_1: Received decimal: %d\n", decimal_num);
    printk(KERN_INFO "lab5_1: Binary: %s, Octal: %s, Hex: %s\n", 
           data->binary, data->octal, data->hex);
    
    return length;
}

// Hàm read - đọc dữ liệu theo format
static ssize_t device_read(struct file *file, char __user *buffer, 
                          size_t length, loff_t *offset) {
    char output[256];
    int output_len;
    
    // Format output: decimal binary octal hex (mỗi loại trên một dòng)
    output_len = snprintf(output, sizeof(output), 
                         "%d\n%s\n%s\n%s\n", 
                         data->decimal, data->binary, data->octal, data->hex);
    
    // Nếu đã đọc hết thì return 0 để báo EOF
    if (*offset >= output_len) {
        return 0;
    }
    
    // Tính toán số bytes cần đọc
    int bytes_to_read = output_len - *offset;
    if (length < bytes_to_read) {
        bytes_to_read = length;
    }
    
    // Copy dữ liệu to user space
    if (copy_to_user(buffer, output + *offset, bytes_to_read)) {
        return -EFAULT;
    }
    
    *offset += bytes_to_read;
    return bytes_to_read;
}

// Hàm seek - cho phép reset offset
static loff_t device_llseek(struct file *file, loff_t offset, int whence) {
    loff_t new_pos = 0;
    
    switch (whence) {
        case SEEK_SET:
            new_pos = offset;
            break;
        case SEEK_CUR:
            new_pos = file->f_pos + offset;
            break;
        case SEEK_END:
            new_pos = 0; // Đặt về đầu file
            break;
        default:
            return -EINVAL;
    }
    
    if (new_pos < 0) {
        return -EINVAL;
    }
    
    file->f_pos = new_pos;
    return new_pos;
}

// File operations structure
static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .write = device_write,
    .read = device_read,
    .llseek = device_llseek,
};

// Hàm khởi tạo module
static int __init lab51_init(void) {
    // Cấp phát device number động
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0) {
        printk(KERN_ERR "lab5_1: Failed to allocate device number\n");
        return -1;
    }
    
    printk(KERN_INFO "lab5_1: Major number: %d, Minor number: %d\n", 
           MAJOR(dev_num), MINOR(dev_num));
    
    // Tạo cdev structure
    mcdev = cdev_alloc();
    if (!mcdev) {
        printk(KERN_ERR "lab5_1: Failed to allocate cdev\n");
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }
    
    cdev_init(mcdev, &fops);
    mcdev->owner = THIS_MODULE;
    
    // Thêm character device vào kernel
    if (cdev_add(mcdev, dev_num, 1) < 0) {
        printk(KERN_ERR "lab5_1: Failed to add cdev\n");
        cdev_del(mcdev);
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }
    
    // Tạo device class
    dev_class = class_create(CLASS_NAME);
    if (IS_ERR(dev_class)) {
        printk(KERN_ERR "lab5_1: Failed to create device class\n");
        cdev_del(mcdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(dev_class);
    }
    
    // Tạo device file
    device = device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(device)) {
        printk(KERN_ERR "lab5_1: Failed to create device\n");
        class_destroy(dev_class);
        cdev_del(mcdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(device);
    }
    
    // Cấp phát bộ nhớ cho dữ liệu
    data = kmalloc(sizeof(struct number_data), GFP_KERNEL);
    if (!data) {
        printk(KERN_ERR "lab5_1: Failed to allocate memory\n");
        device_destroy(dev_class, dev_num);
        class_destroy(dev_class);
        cdev_del(mcdev);
        unregister_chrdev_region(dev_num, 1);
        return -ENOMEM;
    }
    
    // Khởi tạo dữ liệu mặc định
    data->decimal = 0;
    strcpy(data->binary, "0");
    strcpy(data->octal, "0");
    strcpy(data->hex, "0");
    
    printk(KERN_INFO "lab5_1: Driver loaded successfully\n");
    return 0;
}

// Hàm dọn dẹp module
static void __exit lab51_exit(void) {
    kfree(data);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(mcdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "lab5_1: Driver unloaded\n");
}

module_init(lab51_init);
module_exit(lab51_exit);