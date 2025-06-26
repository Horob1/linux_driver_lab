// crypto_driver.c - Driver mã hóa hỗ trợ AES thực bằng skcipher và padding
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <crypto/skcipher.h>
#include <crypto/hash.h>
#include <linux/mutex.h>
#include <linux/scatterlist.h>
#include "crypto_ioctl.h"

#define DEVICE_NAME "crypto_driver"
#define CLASS_NAME "crypto"
#define MAX_BUF_LEN 1024

static dev_t dev_number;
static struct cdev crypto_cdev;
static struct class *crypto_class;
static DEFINE_MUTEX(crypto_mutex);

static int current_mode = MODE_AES;
static char input_buffer[MAX_BUF_LEN];
static char shared_key[16];

static long crypto_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case IOCTL_SET_MODE:
            if (copy_from_user(&current_mode, (int __user *)arg, sizeof(int)))
                return -EFAULT;
            break;
        case IOCTL_SET_KEY:
            if (copy_from_user(shared_key, (char __user *)arg, 16))
                return -EFAULT;
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static ssize_t crypto_write(struct file *file, const char __user *buf, size_t len, loff_t *off) {
    if (len > MAX_BUF_LEN) return -EINVAL;
    memset(input_buffer, 0, MAX_BUF_LEN);
    if (copy_from_user(input_buffer, buf, len)) return -EFAULT;
    return len;
}

static int do_hash(int mode, const char *input, char *output) {
    struct crypto_shash *tfm;
    struct shash_desc *desc;
    int digest_len;
    const char *algo;

    switch (mode) {
        case MODE_MD5: algo = "md5"; break;
        case MODE_SHA1: algo = "sha1"; break;
        case MODE_SHA256: algo = "sha256"; break;
        default: return -EINVAL;
    }

    tfm = crypto_alloc_shash(algo, 0, 0);
    if (IS_ERR(tfm)) return PTR_ERR(tfm);
    digest_len = crypto_shash_digestsize(tfm);
    desc = kmalloc(sizeof(*desc) + crypto_shash_descsize(tfm), GFP_KERNEL);
    if (!desc) {
        crypto_free_shash(tfm);
        return -ENOMEM;
    }

    desc->tfm = tfm;
    crypto_shash_digest(desc, input, strlen(input), output);
    crypto_free_shash(tfm);
    kfree(desc);
    return digest_len;
}

static int do_cipher(int mode, const char *input, char *output) {
    struct crypto_skcipher *skcipher;
    struct skcipher_request *req;
    struct scatterlist sg_in, sg_out;
    char *ivdata = NULL;
    int ret, len = strlen(input);
    int padded_len = (len + 15) & ~15; // làm tròn lên bội số 16

    skcipher = crypto_alloc_skcipher("cbc(aes)", 0, 0);
    if (IS_ERR(skcipher)) return PTR_ERR(skcipher);

    req = skcipher_request_alloc(skcipher, GFP_KERNEL);
    if (!req) {
        crypto_free_skcipher(skcipher);
        return -ENOMEM;
    }

    ivdata = kzalloc(16, GFP_KERNEL);
    sg_init_one(&sg_in, input_buffer, padded_len);
    sg_init_one(&sg_out, output, padded_len);

    crypto_skcipher_setkey(skcipher, shared_key, 16);
    skcipher_request_set_crypt(req, &sg_in, &sg_out, padded_len, ivdata);

    ret = crypto_skcipher_encrypt(req);

    kfree(ivdata);
    skcipher_request_free(req);
    crypto_free_skcipher(skcipher);

    return (ret == 0) ? padded_len : ret;
}

static ssize_t crypto_read(struct file *file, char __user *buf, size_t len, loff_t *off) {
    char outbuf[MAX_BUF_LEN] = {};
    int ret = 0;
    mutex_lock(&crypto_mutex);

    if (current_mode <= MODE_SHA256)
        ret = (current_mode <= MODE_DES) ? do_cipher(current_mode, input_buffer, outbuf)
                                         : do_hash(current_mode, input_buffer, outbuf);
    mutex_unlock(&crypto_mutex);

    if (ret < 0) return ret;
    if (copy_to_user(buf, outbuf, ret)) return -EFAULT;
    return ret;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = crypto_ioctl,
    .write = crypto_write,
    .read = crypto_read,
};

static int __init crypto_init(void) {
    alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    cdev_init(&crypto_cdev, &fops);
    cdev_add(&crypto_cdev, dev_number, 1);
    crypto_class = class_create(THIS_MODULE, CLASS_NAME);
    device_create(crypto_class, NULL, dev_number, NULL, DEVICE_NAME);
    mutex_init(&crypto_mutex);
    memset(shared_key, 0, 16);
    printk(KERN_INFO "[crypto_driver] Loaded with AES + padding support\n");
    return 0;
}

static void __exit crypto_exit(void) {
    mutex_destroy(&crypto_mutex);
    device_destroy(crypto_class, dev_number);
    class_destroy(crypto_class);
    cdev_del(&crypto_cdev);
    unregister_chrdev_region(dev_number, 1);
    printk(KERN_INFO "[crypto_driver] Unloaded\n");
}

module_init(crypto_init);
module_exit(crypto_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("OpenAI & VanCong");
MODULE_DESCRIPTION("Crypto driver with AES + shared-key + padding support");
