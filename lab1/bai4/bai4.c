#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/types.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Horob1");
MODULE_DESCRIPTION("Lab1.4 GCD of array");
MODULE_VERSION("1.0");

#define MAX_LEN 100
static int numbers[MAX_LEN];
static int count;
module_param_array(numbers, int, &count, 0);
MODULE_PARM_DESC(numbers, "Array of numbers to calculate GCD");

int gcd(int a, int b)
{
    if (a == 0 || b == 0)
        return a + b;
    while (a != b)
    {
        if (a > b)
            a -= b;
        else
            b -= a;
    }
    return a;
}

static int __init bai4_init(void)
{
    int i, result;
    printk(KERN_INFO "\n[lab1:part4] GCD of array\n");
    if (count < 2)
    {
        printk(KERN_ALERT "Need at least 2 numbers to calculate GCD\n");
        return -EINVAL;
    }
    result = numbers[0];
    for (i = 1; i < count; i++)
    {
        result = gcd(result, numbers[i]);
    }
    printk(KERN_INFO "GCD of array is: %d\n", result);
    return 0;
}

static void __exit bai4_exit(void)
{
    printk(KERN_INFO "[lab1:part4] Remove module\n");
}

module_init(bai4_init);
module_exit(bai4_exit);
