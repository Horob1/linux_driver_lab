#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <crypto/hash.h>

#define DEVICE_NAME "hashdev"
#define BUF_SIZE 1024

static int major;
static struct class *hash_class;
static struct cdev hash_cdev;

static char input_buf[BUF_SIZE];
static char output_buf[64]; // đủ lớn cho sha256
static int algo_type = 0; // 0: md5, 1: sha1, 2: sha256

static const char *hash_algo_names[] = {"md5", "sha1", "sha256"};

ssize_t hashdev_write(struct file *file, const char __user *buf, size_t len, loff_t *off) {
    struct crypto_shash *tfm;
    struct shash_desc *desc;
    int digest_len;
    char algo[16];
    int ret;

    if (len >= BUF_SIZE) return -EINVAL;
    if (copy_from_user(input_buf, buf, len)) return -EFAULT;
    input_buf[len] = '\0';

    snprintf(algo, sizeof(algo), "%s", hash_algo_names[algo_type]);

    tfm = crypto_alloc_shash(algo, 0, 0);
    if (IS_ERR(tfm)) return PTR_ERR(tfm);

    digest_len = crypto_shash_digestsize(tfm);
    desc = kmalloc(sizeof(*desc) + crypto_shash_descsize(tfm), GFP_KERNEL);
    if (!desc) {
        crypto_free_shash(tfm);
        return -ENOMEM;
    }

    desc->tfm = tfm;

    ret = crypto_shash_digest(desc, input_buf, strlen(input_buf), output_buf);

    crypto_free_shash(tfm);
    kfree(desc);

    if (ret < 0) return ret;

    return len;
}

ssize_t hashdev_read(struct file *file, char __user *buf, size_t len, loff_t *off) {
    char hex_output[128];
    int i, digest_len;
    int ret;

    if (*off > 0) return 0; // không cho đọc lại

    digest_len = algo_type == 0 ? 16 : (algo_type == 1 ? 20 : 32);

    for (i = 0; i < digest_len; i++) {
        sprintf(hex_output + i * 2, "%02x", (unsigned char)output_buf[i]);
    }
    hex_output[digest_len * 2] = '\0';

    if (copy_to_user(buf, hex_output, strlen(hex_output) + 1)) return -EFAULT;

    *off = strlen(hex_output); // đánh dấu đã đọc
    return strlen(hex_output);
}

long hashdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    if (arg > 2) return -EINVAL;
    algo_type = arg;
    return 0;
}

struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = hashdev_write,
    .read = hashdev_read,
    .unlocked_ioctl = hashdev_ioctl,
};

static int __init hashdev_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) return major;

    hash_class = class_create("hash_class");
    if (IS_ERR(hash_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        return PTR_ERR(hash_class);
    }

    if (IS_ERR(device_create(hash_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME))) {
        class_destroy(hash_class);
        unregister_chrdev(major, DEVICE_NAME);
        return -1;
    }

    pr_info("✅ Hash Device loaded, major: %d\n", major);
    return 0;
}

static void __exit hashdev_exit(void) {
    device_destroy(hash_class, MKDEV(major, 0));
    class_destroy(hash_class);
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("❌ Hash Device unloaded\n");
}

module_init(hashdev_init);
module_exit(hashdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("vancongng");
MODULE_DESCRIPTION("Character driver for hashing with MD5, SHA1, SHA256");
