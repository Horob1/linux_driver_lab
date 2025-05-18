#include <linux/init.h>
#include <linux/module.h>
//Metadata of module
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Horob1");
MODULE_DESCRIPTION("Lab1.1 Sum and GCD");
MODULE_VERSION("1.0");

int sum(int arr[100], int n);
int gcd(int a, int b);

static int __init lab1_1_init(void)
{
    int arr[5] = {1, 2, 3, 4, 5};
    int n = 5;
    int a = 4;
    int b = 18;

    printk(KERN_ALERT "\n[lab1:part1] Sum of array: sum = %d\n", sum(arr, n));
    printk(KERN_ALERT "[lab1:part1] GCD(%d, %d) = %d\n", a, b, gcd(a, b));

    return 0;
}

int sum(int arr[100], int n)
{
    int i, s = 0;
    for(i = 0; i < n; i++)
    {
        s += arr[i];
    }
    return s;
}

//Euclid algorithm
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

static void __exit lab1_1_exit(void)
{   
    // Kern alert is used to print to the kernel log (This flag is used to print to the kernel log with the highest priority)
    printk(KERN_ALERT "[lab1_1] Ket thuc module\n");
}

// Register init and exit functions
module_init(lab1_1_init);
module_exit(lab1_1_exit);
