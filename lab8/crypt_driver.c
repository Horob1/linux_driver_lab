#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/crypto.h>
#include <crypto/skcipher.h>
#include <linux/scatterlist.h>

#define DEVICE_NAME "cryptdev"
#define CLASS_NAME  "crypt"
#define BUF_SIZE 1024

#define IOCTL_SET_ALGO     _IOW('k', 1, int)
#define IOCTL_SET_KEY      _IOW('k', 2, char*)
#define IOCTL_ENCRYPT      _IO('k', 3)
#define IOCTL_DECRYPT      _IO('k', 4)

static int major;
static struct class *crypt_class = NULL;
static struct cdev crypt_cdev;

static struct crypto_skcipher *skcipher = NULL;
static struct skcipher_request *req = NULL;
static struct scatterlist sg_in, sg_out;
static char key[32] = "defaultkey123456";
static char input_buf[BUF_SIZE], output_buf[BUF_SIZE];
static int buf_len = 0;
static int output_len = 0;
static char algo_name[20] = "cbc(des)";

static int device_open(struct inode *inode, struct file *file) { return 0; }
static int device_release(struct inode *inode, struct file *file) { return 0; }

static ssize_t device_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    if (copy_to_user(buf, output_buf, output_len)) return -EFAULT;
    return output_len;
}

static ssize_t device_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    if (len > BUF_SIZE) return -EINVAL;
    if (copy_from_user(input_buf, buf, len)) return -EFAULT;
    buf_len = len;
    output_len = 0;
    return len;
}

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int ret;
    size_t key_len;

    switch (cmd) {
        case IOCTL_SET_ALGO:
            if (arg == 0) strncpy(algo_name, "cbc(des)", sizeof(algo_name));
            else strncpy(algo_name, "cbc(aes)", sizeof(algo_name));
            printk(KERN_INFO "[cryptdev] 🔧 Thuật toán: %s\n", algo_name);
            break;

        case IOCTL_SET_KEY:
            memset(key, 0, sizeof(key));
            if (copy_from_user(key, (char __user *)arg, 32)) return -EFAULT;

            key_len = strnlen(key, 32);
            if ((strcmp(algo_name, "cbc(des)") == 0 && key_len != 8) ||
                (strcmp(algo_name, "cbc(aes)") == 0 && (key_len != 16 && key_len != 24 && key_len != 32))) {
                printk(KERN_WARNING "[cryptdev] ❌ Độ dài khoá không hợp lệ (%zu bytes)\n", key_len);
                return -EINVAL;
            }

            printk(KERN_INFO "[cryptdev] 🔑 Khóa hợp lệ (%zu bytes)\n", key_len);
            printk(KERN_INFO "[cryptdev] 🔑 Key (hex):");
            for (int i = 0; i < key_len; i++)
                printk(KERN_CONT " %02x", (unsigned char)key[i]);
            printk(KERN_CONT "\n");
            break;

        case IOCTL_ENCRYPT:
        case IOCTL_DECRYPT:
            key_len = strnlen(key, 32);

            skcipher = crypto_alloc_skcipher(algo_name, 0, 0);
            if (IS_ERR(skcipher)) {
                printk(KERN_ERR "[cryptdev] ❌ Không thể tạo skcipher\n");
                return PTR_ERR(skcipher);
            }

            req = skcipher_request_alloc(skcipher, GFP_KERNEL);
            if (!req) {
                crypto_free_skcipher(skcipher);
                return -ENOMEM;
            }

            if (crypto_skcipher_setkey(skcipher, key, key_len)) {
                printk(KERN_ERR "[cryptdev] ❌ Đặt khóa thất bại\n");
                skcipher_request_free(req);
                crypto_free_skcipher(skcipher);
                return -EKEYREJECTED;
            }

            memset(output_buf, 0, BUF_SIZE);
            sg_init_one(&sg_in, input_buf, buf_len);
            sg_init_one(&sg_out, output_buf, buf_len);
            skcipher_request_set_crypt(req, &sg_in, &sg_out, buf_len, NULL);

            ret = (cmd == IOCTL_ENCRYPT)
                ? crypto_skcipher_encrypt(req)
                : crypto_skcipher_decrypt(req);

            if (ret == 0) {
                output_len = buf_len;
                if (cmd == IOCTL_DECRYPT) output_buf[output_len] = '\0';

                printk(KERN_INFO "[cryptdev] ✅ %s thành công. Dữ liệu (hex):",
                       cmd == IOCTL_ENCRYPT ? "Mã hóa" : "Giải mã");
                for (int i = 0; i < output_len; i++) {
                    printk(KERN_CONT " %02x", (unsigned char)output_buf[i]);
                }
                printk(KERN_CONT "\n");
            } else {
                printk(KERN_ERR "[cryptdev] ❌ %s thất bại\n",
                       cmd == IOCTL_ENCRYPT ? "Mã hóa" : "Giải mã");
            }

            skcipher_request_free(req);
            crypto_free_skcipher(skcipher);
            return ret;
    }

    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl,
};

static int __init crypt_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    crypt_class = class_create(CLASS_NAME);
    device_create(crypt_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    printk(KERN_INFO "[cryptdev] 🔌 Module khởi động. Major = %d\n", major);
    return 0;
}

static void __exit crypt_exit(void) {
    device_destroy(crypt_class, MKDEV(major, 0));
    class_unregister(crypt_class);
    class_destroy(crypt_class);
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "[cryptdev] 🧹 Đã gỡ module\n");
}

module_init(crypt_init);
module_exit(crypt_exit);
MODULE_LICENSE("GPL");
