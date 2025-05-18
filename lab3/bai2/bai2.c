#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/ctype.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Horob1");
MODULE_DESCRIPTION("Lab3.2 Kernel module with cipher operations");

// Dữ liệu rõ
static char XauRo[] = "HELLOKERNEL";
module_param_string(XauRo, XauRo, sizeof(XauRo), 0);
MODULE_PARM_DESC(XauRo, "Plain text");

// Khóa cho mã hóa chuyển vị
static int k = 3;
module_param(k, int, 0);
MODULE_PARM_DESC(k, "Key for transposition cipher");

// Khóa cho mã hóa thay thế (substitution)
static char key_sub[26] = {
    'Q','W','E','R','T','Y','U','I','O','P',
    'A','S','D','F','G','H','J','K','L','Z',
    'X','C','V','B','N','M'
};

static char XauMa[64];

static void ma_hoa_caesar(void)
{
    int len = strlen(XauRo);
    int i;

    memset(XauMa, 0, sizeof(XauMa));
    for (i = 0; i < len; i++) {
        char c = XauRo[i];
        if (c >= 'A' && c <= 'Z') {
            XauMa[i] = ((c - 'A' + k) % 26) + 'A'; // k là khóa dịch chuyển
        } else if (c >= 'a' && c <= 'z') {
            XauMa[i] = ((c - 'a' + k) % 26) + 'a';
        } else {
            XauMa[i] = c; // ký tự khác giữ nguyên
        }
    }
    XauMa[len] = '\0';
    printk(KERN_INFO "Caesar Cipher: %s\n", XauMa);
}

// Hàm mã hóa thay thế (substitution cipher)
static void ma_hoa_thay_the(void)
{
    int len = strlen(XauRo);
    int i;
    memset(XauMa, 0, sizeof(XauMa));

    for (i = 0; i < len; i++) {
        char ch = XauRo[i];
        if (ch >= 'A' && ch <= 'Z') {
            XauMa[i] = key_sub[ch - 'A'];
        } else if (ch >= 'a' && ch <= 'z') {
            XauMa[i] = tolower(key_sub[ch - 'a']);
        } else {
            XauMa[i] = ch;
        }
    }
    XauMa[len] = '\0';
    printk(KERN_INFO "Substitution Cipher: %s\n", XauMa);
}

// Hàm mã hóa hoán vị toàn cục (global permutation)
static void ma_hoa_hoan_vi_toan_cuc(void)
{
    int len = strlen(XauRo);
    int i;
    int perm[100]; // tạo mảng perm động

    // Sinh perm đơn giản
    for (i = 0; i < len; i++)
        perm[i] = (i + k) % len;

    memset(XauMa, 0, sizeof(XauMa));

    for (i = 0; i < len; i++) {
        XauMa[i] = XauRo[perm[i]];
    }
    XauMa[len] = '\0';

    printk(KERN_INFO "Global Permutation Cipher (shift k): %s\n", XauMa);
}

static int __init cipher_init(void)
{
    int len = strlen(XauRo);
    if (k > len) {
        printk(KERN_ERR "Key k for transposition cipher is too large\n");
        return -EINVAL;
    }
    if (len > 63) {
        printk(KERN_ERR "Plain text is too long\n");
        return -EINVAL;
    }
    if (len == 0) {
        printk(KERN_ERR "Plain text is empty\n");
        return -EINVAL;
    }
    printk(KERN_INFO "Cipher module loaded\n");
    printk(KERN_INFO "Plain text: %s\n", XauRo);
    printk(KERN_INFO "Key for Caesar cipher: %d\n", k);

    ma_hoa_caesar();
    ma_hoa_thay_the();
    ma_hoa_hoan_vi_toan_cuc();

    return 0;
}

static void __exit cipher_exit(void)
{
    printk(KERN_INFO "Cipher module unloaded\n");
}

module_init(cipher_init);
module_exit(cipher_exit);
