#include <linux/module.h>
#include <linux/init.h> 
#include <linux/fs.h>
#include <linux/device.h> 
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>

static struct cdev *cdev_lab4_1; 

static int lab4_1_open(struct inode *inode, struct file *filp);
static int lab4_1_release(struct inode *inode, struct file *filp);
static ssize_t lab4_1_read(struct file *filp, char __user *user_buf, size_t len, loff_t *off);
static ssize_t lab4_1_write(struct file *filp, const char *user_buf, size_t len, loff_t *off);


struct vchrdev {
	dev_t dev_num;
	struct class *dev_class;
	struct device *dev;
} vchrdev;

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = lab4_1_open,
	.release = lab4_1_release,
	.read = lab4_1_read,
	.write = lab4_1_write,
};

static int lab4_1_open(struct inode *inode, struct file *filp)
{
  	printk( "[Lab4_1] Driver: open()\n");
  	return 0;
}

static int lab4_1_release(struct inode *inode, struct file *filp)
{
  	printk( "[Lab4_1] Driver: close()\n");
  	return 0;
}

static ssize_t lab4_1_read(struct file *filp, char __user *user_buf, size_t len, loff_t *off)
{
	printk( "[Lab4_1] Driver: read()\n");
  	return len;
}

static ssize_t lab4_1_write(struct file *f, const char *user_buf, size_t len, loff_t *off)
{	
  	printk( "[Lab4_1] Driver: write()\n");
  	return len;
}

static int __init init_chrdev(void){
	int ret = 0;
	
	/* define device number */
	vchrdev.dev_num = 0;
	ret = alloc_chrdev_region(&vchrdev.dev_num, 0, 1, "lab4_1");
	if(ret < 0){
		printk("[Lab4_1] Can not allocate device number\n");
		goto failed_register_devnum;
	}
	printk("[Lab4_1] Allocate device number success (%d, %d)", MAJOR(vchrdev.dev_num), MINOR(vchrdev.dev_num));
	
	// Create class and save in /sys/class/class_dev_lab4_1
	vchrdev.dev_class = class_create(THIS_MODULE, "class_dev_lab4_1");
	if(IS_ERR(vchrdev.dev_class)){
		printk("[Lab4_1] Can not create class\n");
		goto failed_create_class;
	}
	
	// Create device and save in /dev/device_lab4_1
	vchrdev.dev = device_create(vchrdev.dev_class, NULL, vchrdev.dev_num, NULL, "device_lab4_1");
	if(IS_ERR(vchrdev.dev)){
		printk("[Lab4_1] Can not create device\n");
		goto failed_create_dev;
	}
	cdev_lab4_1 = cdev_alloc();
	cdev_init(cdev_lab4_1, &fops);
	cdev_add(cdev_lab4_1, vchrdev.dev_num, 1);

	// Show device number
	printk("[Lab4_1] Device number: (%d, %d)\n", MAJOR(vchrdev.dev_num), MINOR(vchrdev.dev_num));
	printk("[Lab4_1] Initialize device success\n");
	return 0;
	
failed_create_dev:
	class_destroy(vchrdev.dev_class);
failed_create_class:
	unregister_chrdev_region(vchrdev.dev_num, 1);
failed_register_devnum:
	return ret;
}


static void __exit exit_chrdev(void){
	cdev_del(cdev_lab4_1);
		
	device_destroy(vchrdev.dev_class, vchrdev.dev_num);
	class_destroy(vchrdev.dev_class);
	
	unregister_chrdev_region(vchrdev.dev_num, 1);
	
	printk("[Lab4_1] Destroy device success!!!");
}

module_init(init_chrdev);
module_exit(exit_chrdev);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Horob1"); 
MODULE_DESCRIPTION("Lab 4 - 1: Simple character driver interact with user space!");
MODULE_VERSION("1.0"); 