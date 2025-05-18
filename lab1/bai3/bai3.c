#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Horob1");
MODULE_DESCRIPTION("Lab1.3 Matrix");
MODULE_VERSION("1.0");

int Sum(int arr[10][10], int n, int m);
int CountNegative(int arr[10][10], int n, int m);
int CountPositive(int arr[10][10], int n, int m);
int MaxOfMainDiagonal(int arr[10][10], int n, int m);
int MinOfMainDiagonal(int arr[10][10], int n, int m);
int MaxOfSecondaryDiagonal(int arr[10][10], int n, int m);
int MinOfSecondaryDiagonal(int arr[10][10], int n, int m);
int SumOfOdd(int arr[10][10], int n, int m);
int SumOfEven(int arr[10][10], int n, int m);

static int __init lab1_3_init(void)
{
    int n = 3, m = 3;
    int arr[10][10] = {
        {1, 2, 3},
        {5, 6, 7},
        {12, 13, 14}
    };

    printk(KERN_ALERT "\n[lab1:part3] Matrix\n");
    printk(KERN_ALERT "Sum = %d\n", Sum(arr, n, m));
    printk(KERN_ALERT "Count of negative = %d\n", CountNegative(arr, n, m));
    printk(KERN_ALERT "Count of positive = %d\n", CountPositive(arr, n, m));
    printk(KERN_ALERT "Max of main diagonal = %d\n", MaxOfMainDiagonal(arr, n, m));
    printk(KERN_ALERT "Min of main diagonal = %d\n", MinOfMainDiagonal(arr, n, m));
    printk(KERN_ALERT "Max of secondary diagonal = %d\n", MaxOfSecondaryDiagonal(arr, n, m));
    printk(KERN_ALERT "Min of secondary diagonal = %d\n", MinOfSecondaryDiagonal(arr, n, m));
    printk(KERN_ALERT "Sum of even = %d\n", SumOfEven(arr, n, m));
    printk(KERN_ALERT "Sum of odd = %d\n", SumOfOdd(arr, n, m));

    return 0;
}

int Sum(int arr[10][10], int n, int m)
{
    int i, j, sum = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    return sum;
}

int CountNegative(int arr[10][10], int n, int m)
{
    int i, j, count = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            if (arr[i][j] < 0)
                count++;
    return count;
}

int CountPositive(int arr[10][10], int n, int m)
{
    int i, j, count = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            if (arr[i][j] > 0)
                count++;
    return count;
}

int MaxOfMainDiagonal(int arr[10][10], int n, int m)
{
    int max = arr[0][0];
    int i;
    for (i = 1; i < n; i++)
        if (arr[i][i] > max)
            max = arr[i][i];
    return max;
}

int MinOfMainDiagonal(int arr[10][10], int n, int m)
{
    int min = arr[0][0];
    int i;
    for (i = 1; i < n; i++)
        if (arr[i][i] < min)
            min = arr[i][i];
    return min;
}

int MaxOfSecondaryDiagonal(int arr[10][10], int n, int m)
{
    int i, max = arr[0][n - 1];
    for (i = 0; i < n; i++)
        if (arr[i][n - 1 - i] > max)
            max = arr[i][n - 1 - i];
    return max;
}

int MinOfSecondaryDiagonal(int arr[10][10], int n, int m)
{
    int i, min = arr[0][n - 1];
    for (i = 0; i < n; i++)
        if (arr[i][n - 1 - i] < min)
            min = arr[i][n - 1 - i];
    return min;
}

int SumOfEven(int arr[10][10], int n, int m)
{
    int i, j, sum = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            if (arr[i][j] % 2 == 0)
                sum += arr[i][j];
    return sum;
}

int SumOfOdd(int arr[10][10], int n, int m)
{
    int i, j, sum = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            if (arr[i][j] % 2 != 0)
                sum += arr[i][j];
    return sum;
}

static void __exit lab1_3_exit(void)
{
    printk(KERN_ALERT "[lab1:part3] Remove module\n");
}

module_init(lab1_3_init);
module_exit(lab1_3_exit);
