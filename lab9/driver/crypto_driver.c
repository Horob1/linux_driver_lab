#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/init.h>
#include <crypto/skcipher.h>
#include <crypto/hash.h>

#define DEVICE_NAME "crypto_driver"
#define BUF_SIZE 1024
#define IOCTL_SET_MODE _IOW('c', 1, int)

enum crypto_mode {
    MODE_AES,
    MODE_DES,
    MODE_MD5,
    MODE_SHA1,
    MODE_SHA256
};

static int major;
static struct class *crypto_class;
static struct cdev crypto_cdev;
static int current_mode = MODE_AES;

static char input_buf[BUF_SIZE];
static char output_buf[BUF_SIZE];
static int data_len = 0;

static int dev_open(struct inode *inode, struct file *file) {
    return 0;
}

static int dev_release(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    if (len > BUF_SIZE) return -EINVAL;
    if (copy_from_user(input_buf, buf, len)) return -EFAULT;
    data_len = len;

    switch (current_mode) {
        case MODE_MD5:
        case MODE_SHA1:
        case MODE_SHA256: {
            struct crypto_shash *tfm;
            struct shash_desc *shash;
            char *algo;
            int digest_size;

            if (current_mode == MODE_MD5) algo = "md5";
            else if (current_mode == MODE_SHA1) algo = "sha1";
            else algo = "sha256";

            tfm = crypto_alloc_shash(algo, 0, 0);
            if (IS_ERR(tfm)) return -EINVAL;

            digest_size = crypto_shash_digestsize(tfm);
            shash = kmalloc(sizeof(*shash) + crypto_shash_descsize(tfm), GFP_KERNEL);
            shash->tfm = tfm;

            crypto_shash_digest(shash, input_buf, len, output_buf);
            data_len = digest_size;

            crypto_free_shash(tfm);
            kfree(shash);
            break;
        }
        default:
            for (int i = 0; i < len; i++)
                output_buf[i] = input_buf[i] ^ 0xAA;
            data_len = len;
            break;
    }
    return len;
}

static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    if (len > data_len) len = data_len;
    if (copy_to_user(buf, output_buf, len)) return -EFAULT;
    return len;
}

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    if (cmd == IOCTL_SET_MODE) {
        current_mode = arg;
        return 0;
    }
    return -EINVAL;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl
};

static int __init crypto_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) return major;
    crypto_class = class_create(DEVICE_NAME);
    device_create(crypto_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    printk(KERN_INFO "Crypto driver loaded.\n");
    return 0;
}

static void __exit crypto_exit(void) {
    device_destroy(crypto_class, MKDEV(major, 0));
    class_destroy(crypto_class);
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "Crypto driver unloaded.\n");
}

module_init(crypto_init);
module_exit(crypto_exit);
MODULE_LICENSE("GPL");
