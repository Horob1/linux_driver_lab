#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ctype.h>

// Metadata of module
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Horob1");
MODULE_DESCRIPTION("Lab2.1 Number base conversions");
MODULE_VERSION("1.0");


// Hàm chuyển từ số nguyên sang chuỗi nhị phân (hệ 2)
void dec_to_bin(unsigned int num, char *bin_str, int size)
{
    int i;
    bin_str[size] = '\0';
    for (i = size - 1; i >= 0; i--) {
        bin_str[i] = (num & 1) ? '1' : '0';
        num >>= 1; // num = num >> 1
    }
}

// Hàm chuyển từ chuỗi nhị phân sang số nguyên
unsigned int bin_to_dec(const char *bin_str)
{
    unsigned int num = 0;
    while (*bin_str) {
        num <<= 1; // num = num << 1
        if (*bin_str == '1')
            num |= 1; // num = num | 1
        else if (*bin_str != '0')
            break; // ký tự không hợp lệ
        bin_str++;
    }
    return num;
}

// Hàm chuyển số nguyên sang chuỗi thập lục phân
void dec_to_hex(unsigned int num, char *hex_str, int size)
{
    static const char hex_digits[] = "0123456789ABCDEF";
    int i;
    hex_str[size] = '\0';
    for (i = size - 1; i >= 0; i--) {
        hex_str[i] = hex_digits[num & 0xF];
        num >>= 4; // num = num >> 4
    }
}

// Hàm chuyển chuỗi thập lục phân sang số nguyên
unsigned int hex_to_dec(const char *hex_str)
{
    unsigned int num = 0;
    char c;
    while ((c = *hex_str++)) {
        num <<= 4;
        if (c >= '0' && c <= '9')
            num += c - '0';
        else if (c >= 'a' && c <= 'f')
            num += c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
            num += c - 'A' + 10;
        else
            break; // ký tự không hợp lệ
    }
    return num;
}

// Hàm chuyển số nguyên sang chuỗi hệ 8
void dec_to_oct(unsigned int num, char *oct_str, int size)
{
    int i = size - 1;
    oct_str[size] = '\0';
    while (i >= 0) {
        oct_str[i--] = (num & 7) + '0';
        num >>= 3;
    }
}

// Hàm chuyển chuỗi hệ 8 sang số nguyên
unsigned int oct_to_dec(const char *oct_str)
{
    unsigned int num = 0;
    char c;
    while ((c = *oct_str++)) {
        num <<= 3;
        if (c >= '0' && c <= '7')
            num += c - '0';
        else
            break; // ký tự không hợp lệ
    }
    return num;
}

static int __init conv_init(void)
{
    char bin[33];
    char hex[9];
    char oct[12];

    unsigned int num_dec;
    unsigned int num_from_bin;
    unsigned int num_from_oct;
    unsigned int num_from_hex;

    const char *hex_input = "1A3F";
    const char *bin_input = "1101011";
    const char *oct_input = "752";

    printk(KERN_INFO "[lab2:part1] Module conversion base start\n");

    // 1. Hex -> Bin
    num_from_hex = hex_to_dec(hex_input);
    dec_to_bin(num_from_hex, bin, 32);
    printk(KERN_INFO "Hex %s -> Dec %u -> Bin %s\n", hex_input, num_from_hex, bin);

    // 2. Dec -> Bin
    num_dec = 439;
    dec_to_bin(num_dec, bin, 32);
    printk(KERN_INFO "Dec %u -> Bin %s\n", num_dec, bin);

    // 3. Dec -> Hex
    dec_to_hex(num_dec, hex, 8);
    printk(KERN_INFO "Dec %u -> Hex %s\n", num_dec, hex);

    // 4. Bin -> Oct
    num_from_bin = bin_to_dec(bin_input);
    dec_to_oct(num_from_bin, oct, 11);
    printk(KERN_INFO "Bin %s -> Dec %u -> Oct %s\n", bin_input, num_from_bin, oct);

    // 5. Oct -> Hex
    num_from_oct = oct_to_dec(oct_input);
    dec_to_hex(num_from_oct, hex, 8);
    printk(KERN_INFO "Oct %s -> Dec %u -> Hex %s\n", oct_input, num_from_oct, hex);

    // 6. Hex -> Oct
    num_from_hex = hex_to_dec(hex_input);
    dec_to_oct(num_from_hex, oct, 11);
    printk(KERN_INFO "Hex %s -> Dec %u -> Oct %s\n", hex_input, num_from_hex, oct);

    // 7. Bin -> Dec
    num_from_bin = bin_to_dec(bin_input);
    printk(KERN_INFO "Bin %s -> Dec %u\n", bin_input, num_from_bin);

    printk(KERN_INFO "[lab2:part1] Module conversion base loaded\n");
    return 0;
}

static void __exit conv_exit(void)
{
    printk(KERN_INFO "[lab2:part1] Module conversion base unloaded\n");
}

module_init(conv_init);
module_exit(conv_exit);
