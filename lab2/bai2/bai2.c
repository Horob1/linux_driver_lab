#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/ctype.h>

#define MAX_LEN 256

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Horob1");
MODULE_DESCRIPTION("Lab2.2 String Operations Kernel Module");

static char input[MAX_LEN] = "   hello   kernel    world  ";
module_param_string(input, input, sizeof(input), 0);
MODULE_PARM_DESC(input, "Input string");

void replace_underscores_with_space(char *str) {
  while (*str) {
      if (*str == '_') {
          *str = ' ';
      } else {
          str++;
      }
  }
}

static int str_length(const char *str) {
    int count = 0;
    while (str[count] != '\0') count++;
    return count;
}

static void reverse_str(char *str) {
    int len = str_length(str), i;
    for (i = 0; i < len / 2; i++) {
        char tmp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = tmp;
    }
}

static void concat_str(char *dest, const char *src) {
    int i = 0, j = 0;
    while (dest[i] != '\0') i++;
    while (src[j] != '\0') dest[i++] = src[j++];
    dest[i] = '\0';
}

static void normalize_str(char *str) {
    int i = 0, j = 0, in_word = 0;
    char temp[MAX_LEN];

    // Bỏ khoảng trắng đầu
    while (isspace(str[i])) i++;

    while (str[i] != '\0') {
        if (isspace(str[i])) {
            if (in_word) {
                temp[j++] = ' ';
                in_word = 0;
            }
        } else {
            if (!in_word)
                temp[j++] = toupper(str[i]);
            else
                temp[j++] = tolower(str[i]);
            in_word = 1;
        }
        i++;
    }

    // Bỏ khoảng trắng cuối
    if (j > 0 && temp[j - 1] == ' ')
        j--;
    temp[j] = '\0';
    strcpy(str, temp);
}

static int __init string_module_init(void)
{
    char *data = kmalloc(MAX_LEN, GFP_KERNEL);
    // GFP_KERNEL: GFP stands for "get free page". It is a flag used in the Linux kernel to specify the type of memory allocation being requested.
    if (!data)
        return -ENOMEM;

    replace_underscores_with_space(input);
    strncpy(data, input, MAX_LEN);
    printk(KERN_INFO "[lab2:part2] String module loaded\n");
    printk(KERN_INFO "[lab2:part2] Original string: \"%s\"\n", data);
    printk(KERN_INFO "[lab2:part2] Length: %d\n", str_length(data));

    reverse_str(data);
    printk(KERN_INFO "[lab2:part2] Reversed: \"%s\"\n", data);

    reverse_str(data); // revert to original before normalize
    normalize_str(data);
    printk(KERN_INFO "[lab2:part2] Normalized: \"%s\"\n", data);

    concat_str(data, " [kernel]");
    printk(KERN_INFO "[lab2:part2] Concatenated: \"%s\"\n", data);

    kfree(data);
    return 0;
}

static void __exit string_module_exit(void)
{
    printk(KERN_INFO "[lab2:part2] String module unloaded.\n");
}

module_init(string_module_init);
module_exit(string_module_exit);
