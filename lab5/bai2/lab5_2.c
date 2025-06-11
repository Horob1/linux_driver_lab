#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/jiffies.h>

#define DEVICE_NAME "lab52"
#define CLASS_NAME "lab52_class"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("vancongng");
MODULE_DESCRIPTION("Character driver for high precision time measurement");
MODULE_VERSION("1.0");

#define TIME_TYPE_MICRO   1
#define TIME_TYPE_NANO    2
#define TIME_TYPE_RELATIVE 3

static dev_t dev_num;         
static struct cdev *mcdev;      
static struct class *dev_class; 
static struct device *device;  
static ktime_t start_time;     

// Hàm open device
static int device_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "lab5_2: Device opened\n");
    return 0;
}

// Hàm close device
static int device_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "lab5_2: Device closed\n");
    return 0;
}

// Hàm write - nhận loại thời gian cần lấy
static ssize_t device_write(struct file *file, const char __user *buffer, 
                           size_t length, loff_t *offset) {
    char input[16];
    int time_type;
    
    if (length >= sizeof(input)) {
        return -EINVAL;
    }
    
    if (copy_from_user(input, buffer, length)) {
        return -EFAULT;
    }
    
    input[length] = '\0';
    
    // Chuyển chuỗi thành số
    if (kstrtoint(input, 10, &time_type) != 0) {
        return -EINVAL;
    }
    
    // Lưu loại thời gian vào private_data để sử dụng trong read
    file->private_data = (void *)(long)time_type;
    
    printk(KERN_INFO "lab5_2: Time type set to: %d\n", time_type);
    
    return length;
}

// Hàm read - trả về thời gian theo loại đã chọn
static ssize_t device_read(struct file *file, char __user *buffer, 
                          size_t length, loff_t *offset) {
    char output[256];
    int output_len = 0;
    int time_type = (int)(long)file->private_data;
    
    struct timespec64 ts;
    ktime_t current_time;
    u64 microseconds, nanoseconds;
    unsigned long relative_jiffies;
    
    // Nếu đã đọc rồi thì return 0 (EOF)
    if (*offset > 0) {
        return 0;
    }
    
    switch (time_type) {
        case TIME_TYPE_MICRO:
            ktime_get_real_ts64(&ts);
            microseconds = (u64)ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000;
            
            output_len = snprintf(output, sizeof(output), 
                                "Absolute time (microseconds): %llu\n"
                                "Formatted: %lld.%06lld seconds\n"
                                "Date/Time: %ptT\n",
                                microseconds,
                                (long long)ts.tv_sec, 
                                (long long)(ts.tv_nsec / 1000),
                                &ts);
            break;
            
        case TIME_TYPE_NANO:
            ktime_get_real_ts64(&ts);
            nanoseconds = (u64)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
            
            output_len = snprintf(output, sizeof(output), 
                                "Absolute time (nanoseconds): %llu\n"
                                "Formatted: %lld.%09lld seconds\n"
                                "Date/Time: %ptT\n",
                                nanoseconds,
                                (long long)ts.tv_sec, 
                                (long long)ts.tv_nsec,
                                &ts);
            break;
            
        case TIME_TYPE_RELATIVE:
            current_time = ktime_get();
            relative_jiffies = jiffies; //số tick
            
            u64 relative_ns = ktime_to_ns(ktime_sub(current_time, start_time));
            u64 relative_us = relative_ns / 1000;
            u64 relative_ms = relative_us / 1000;
            u64 relative_sec = relative_ms / 1000;
            
            output_len = snprintf(output, sizeof(output), 
                                "Relative time since driver load:\n"
                                "Nanoseconds: %llu ns\n"
                                "Microseconds: %llu us\n"
                                "Milliseconds: %llu ms\n"
                                "Seconds: %llu.%03llu s\n"
                                "Jiffies: %lu (HZ=%d)\n",
                                relative_ns,
                                relative_us,
                                relative_ms,
                                relative_sec, relative_ms % 1000,
                                relative_jiffies,
                                HZ);
            break;
            
        default:
            output_len = snprintf(output, sizeof(output), 
                                "Error: Invalid time type. Use 1, 2, or 3.\n");
            break;
    }
    
    // Kiểm tra buffer size
    if (length < output_len) {
        return -EINVAL;
    }
    
    // Copy dữ liệu to user space
    if (copy_to_user(buffer, output, output_len)) {
        return -EFAULT;
    }
    
    *offset += output_len;
    return output_len;
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
            new_pos = 0;
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
static int __init lab52_init(void) {
    // Lưu thời gian khởi tạo cho relative time
    start_time = ktime_get();
    
    // Cấp phát device number động
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0) {
        printk(KERN_ERR "lab5_2: Failed to allocate device number\n");
        return -1;
    }
    
    printk(KERN_INFO "lab5_2: Major number: %d, Minor number: %d\n", 
           MAJOR(dev_num), MINOR(dev_num));
    
    // Tạo cdev structure
    mcdev = cdev_alloc();
    if (!mcdev) {
        printk(KERN_ERR "lab5_2: Failed to allocate cdev\n");
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }
    
    cdev_init(mcdev, &fops);
    mcdev->owner = THIS_MODULE;
    
    // Thêm character device vào kernel
    if (cdev_add(mcdev, dev_num, 1) < 0) {
        printk(KERN_ERR "lab5_2: Failed to add cdev\n");
        cdev_del(mcdev);
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }
    
    // Tạo device class
    dev_class = class_create(CLASS_NAME);
    if (IS_ERR(dev_class)) {
        printk(KERN_ERR "lab5_2: Failed to create device class\n");
        cdev_del(mcdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(dev_class);
    }
    
    // Tạo device file
    device = device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(device)) {
        printk(KERN_ERR "lab5_2: Failed to create device\n");
        class_destroy(dev_class);
        cdev_del(mcdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(device);
    }
    
    printk(KERN_INFO "lab5_2: Time driver loaded successfully\n");
    printk(KERN_INFO "lab5_2: Start time recorded: %lld ns\n", 
           ktime_to_ns(start_time));
    
    return 0;
}

// Hàm dọn dẹp module
static void __exit lab52_exit(void) {
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(mcdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "lab5_2: Time driver unloaded\n");
}

module_init(lab52_init);
module_exit(lab52_exit);