#include <linux/module.h>
#include <linux/init.h> 
#include <linux/fs.h>
#include <linux/device.h> 
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>

static int param_major = 300;
module_param(param_major, int, 0444);  // int, permission đọc (r--r--r--)
MODULE_PARM_DESC(param_major, "An integer parameter for major number");

static struct cdev *cdev_lab4_2;

struct vchrdev {
    dev_t dev_num;
    struct class *dev_class;
    struct device *dev;
} vchrdev;

static int lab4_2_open(struct inode *inode, struct file *filp);
static int lab4_2_release(struct inode *inode, struct file *filp);
static ssize_t lab4_2_read(struct file *filp, char __user *user_buf, size_t len, loff_t *off);
static ssize_t lab4_2_write(struct file *filp, const char __user *user_buf, size_t len, loff_t *off);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = lab4_2_open,
    .release = lab4_2_release,
    .read = lab4_2_read,
    .write = lab4_2_write,
};

static int lab4_2_open(struct inode *inode, struct file *filp)
{
    printk("[Lab4_2] Driver: open()\n");
    return 0;
}

static int lab4_2_release(struct inode *inode, struct file *filp)
{
    printk("[Lab4_2] Driver: close()\n");
    return 0;
}

static ssize_t lab4_2_read(struct file *filp, char __user *user_buf, size_t len, loff_t *off)
{
    printk("[Lab4_2] Driver: read()\n");
    return 0;  // EOF
}

static ssize_t lab4_2_write(struct file *filp, const char __user *user_buf, size_t len, loff_t *off)
{
    printk("[Lab4_2] Driver: write()\n");
    return len;
}

static int __init init_chrdev(void)
{
    int ret;

    vchrdev.dev_num = MKDEV(param_major, 0);

    ret = register_chrdev_region(vchrdev.dev_num, 1, "lab4_2");
    if (ret < 0) {
        printk("[Lab4_2] Failed to register device number (%d, %d)\n", param_major, 0);
        return ret;
    }
    printk("[Lab4_2] Registered device number: major=%d minor=%d\n",
           MAJOR(vchrdev.dev_num), MINOR(vchrdev.dev_num));

    vchrdev.dev_class = class_create(THIS_MODULE, "class_dev_lab4_2");
    if (IS_ERR(vchrdev.dev_class)) {
        printk("[Lab4_2] Failed to create class\n");
        ret = PTR_ERR(vchrdev.dev_class);
        goto unregister_devnum;
    }

    vchrdev.dev = device_create(vchrdev.dev_class, NULL, vchrdev.dev_num, NULL, "device_lab4_2");
    if (IS_ERR(vchrdev.dev)) {
        printk("[Lab4_2] Failed to create device\n");
        ret = PTR_ERR(vchrdev.dev);
        goto destroy_class;
    }


    cdev_lab4_2 = cdev_alloc();
    if (!cdev_lab4_2) {
        printk("[Lab4_2] Failed to allocate cdev\n");
        ret = -ENOMEM;
        goto destroy_device;
    }

    cdev_init(cdev_lab4_2, &fops);

    ret = cdev_add(cdev_lab4_2, vchrdev.dev_num, 1);
    if (ret < 0) {
        printk("[Lab4_2] Failed to add cdev\n");
        goto free_cdev;
    }

    printk("[Lab4_2] Device initialized successfully\n");
    return 0;

free_cdev:
    kobject_put(&cdev_lab4_2->kobj);
destroy_device:
    device_destroy(vchrdev.dev_class, vchrdev.dev_num);
destroy_class:
    class_destroy(vchrdev.dev_class);
unregister_devnum:
    unregister_chrdev_region(vchrdev.dev_num, 1);
    return ret;
}

static void __exit exit_chrdev(void)
{
    cdev_del(cdev_lab4_2);
    device_destroy(vchrdev.dev_class, vchrdev.dev_num);
    class_destroy(vchrdev.dev_class);
    unregister_chrdev_region(vchrdev.dev_num, 1);
    printk("[Lab4_2] Device destroyed successfully\n");
}

module_init(init_chrdev);
module_exit(exit_chrdev);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Horob1");
MODULE_DESCRIPTION("Lab 4 - 2: Simple character driver interacting with user space!");
MODULE_VERSION("1.0");
