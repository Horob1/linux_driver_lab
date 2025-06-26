#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/crypto.h>
#include <crypto/skcipher.h>
#include <linux/scatterlist.h>

#define DEVICE_NAME "cryptdev"
#define BUF_SIZE 1024
#define BLOCK_SIZE 16

#define IOCTL_SET_ALGO     _IOW('k', 1, int)
#define IOCTL_SET_KEY      _IOW('k', 2, struct key_data)
#define IOCTL_ENCRYPT      _IOR('k', 3, int)
#define IOCTL_DECRYPT      _IOR('k', 4, int)

enum {
    ALGO_DES = 0,
    ALGO_AES
};

struct key_data {
    char key[32];
    int keylen;
};

static struct {
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct crypto_skcipher *tfm;
    struct skcipher_request *req;
    char key[32];
    char data[BUF_SIZE + BLOCK_SIZE];
    int datalen;
    int algo;
} crypt;

static int dev_open(struct inode *inode, struct file *file) {
    return 0;
}

static int dev_release(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *off) {
    if (len > BUF_SIZE)
        return -EINVAL;
    memset(crypt.data, 0, sizeof(crypt.data));
    if (copy_from_user(crypt.data, buf, len))
        return -EFAULT;
    crypt.datalen = len;
    return len;
}

static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *off) {
    if (*off >= crypt.datalen)
        return 0;
    if (len > crypt.datalen - *off)
        len = crypt.datalen - *off;

    if (crypt.datalen < BUF_SIZE + BLOCK_SIZE)
        crypt.data[crypt.datalen] = '\0';

    if (copy_to_user(buf, crypt.data + *off, len))
        return -EFAULT;
    *off += len;
    return len;
}

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int ret = 0;

    switch (cmd) {
    case IOCTL_SET_ALGO:
        if (copy_from_user(&crypt.algo, (int *)arg, sizeof(int)))
            return -EFAULT;

        if (crypt.req) {
            skcipher_request_free(crypt.req);
            crypt.req = NULL;
        }

        if (crypt.tfm) {
            crypto_free_skcipher(crypt.tfm);
            crypt.tfm = NULL;
        }

        crypt.tfm = crypto_alloc_skcipher(
            (crypt.algo == ALGO_AES) ? "cbc(aes)" : "cbc(des)", 0, 0);
        if (IS_ERR(crypt.tfm))
            return PTR_ERR(crypt.tfm);

        crypt.req = skcipher_request_alloc(crypt.tfm, GFP_KERNEL);
        if (!crypt.req) {
            crypto_free_skcipher(crypt.tfm);
            crypt.tfm = NULL;
            return -ENOMEM;
        }
        break;

    case IOCTL_SET_KEY: {
        struct key_data kdata;
        if (copy_from_user(&kdata, (void *)arg, sizeof(kdata)))
            return -EFAULT;
        memcpy(crypt.key, kdata.key, kdata.keylen);
        ret = crypto_skcipher_setkey(crypt.tfm, crypt.key, kdata.keylen);
        break;
    }

    case IOCTL_ENCRYPT: {
        int pad = BLOCK_SIZE - (crypt.datalen % BLOCK_SIZE);
        if (pad == 0) pad = BLOCK_SIZE;
        if (crypt.datalen + pad > BUF_SIZE + BLOCK_SIZE)
            return -ENOMEM;

        memset(crypt.data + crypt.datalen, pad, pad);
        crypt.datalen += pad;

        struct scatterlist sg_in, sg_out;
        char iv[BLOCK_SIZE] = {0};

        sg_init_one(&sg_in, crypt.data, crypt.datalen);
        sg_init_one(&sg_out, crypt.data, crypt.datalen);

        skcipher_request_set_crypt(crypt.req, &sg_in, &sg_out, crypt.datalen, iv);
        ret = crypto_skcipher_encrypt(crypt.req);
        break;
    }

    case IOCTL_DECRYPT: {
        struct scatterlist sg_in, sg_out;
        char iv[BLOCK_SIZE] = {0};

        sg_init_one(&sg_in, crypt.data, crypt.datalen);
        sg_init_one(&sg_out, crypt.data, crypt.datalen);

        skcipher_request_set_crypt(crypt.req, &sg_in, &sg_out, crypt.datalen, iv);
        ret = crypto_skcipher_decrypt(crypt.req);
        if (ret == 0) {
            int pad = crypt.data[crypt.datalen - 1];
            if (pad <= 0 || pad > BLOCK_SIZE)
                return -EINVAL;

            for (int i = 1; i <= pad; i++) {
                if (crypt.data[crypt.datalen - i] != pad)
                    return -EINVAL;
            }

            crypt.datalen -= pad;
        }
        break;
    }
    }

    return ret;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,
};

static int major;
static struct class *crypt_class;

static int __init crypt_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) return major;

    crypt_class = class_create( "crypt_class");
    if (IS_ERR(crypt_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        return PTR_ERR(crypt_class);
    }

    device_create(crypt_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    return 0;
}

static void __exit crypt_exit(void) {
    device_destroy(crypt_class, MKDEV(major, 0));
    class_destroy(crypt_class);
    unregister_chrdev(major, DEVICE_NAME);
    if (crypt.req) skcipher_request_free(crypt.req);
    if (crypt.tfm) crypto_free_skcipher(crypt.tfm);
}

module_init(crypt_init);
module_exit(crypt_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("OpenAI - GPT-4");
MODULE_DESCRIPTION("Kernel module for AES/DES encryption using skcipher");
