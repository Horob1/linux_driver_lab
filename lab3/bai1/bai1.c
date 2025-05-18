#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Horob1");
MODULE_DESCRIPTION("Lab3.1 Kernel module with hardcoded vector, matrix, and permutation operations");

static int vec1[] = {1, 2, 3};
static int vec2[] = {4, 5, 6};
static int vec_size = sizeof(vec1) / sizeof(vec1[0]);

static int mat1[] = {
    1, 2,
    3, 4
};
static int mat2[] = {
    5, 6,
    7, 8
};
static int mat_rows = 2;
static int mat_cols = 2;

static int perm_n = 5;
static int perm_k = 3;

static int dot_product(const int *a, const int *b, int size) {
    int i, sum = 0;
    for (i = 0; i < size; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

static void matrix_add(const int *a, const int *b, int *res, int rows, int cols) {
    int i;
    for (i = 0; i < rows * cols; i++) {
        res[i] = a[i] + b[i];
    }
}

static void matrix_sub(const int *a, const int *b, int *res, int rows, int cols) {
    int i;
    for (i = 0; i < rows * cols; i++) {
        res[i] = a[i] - b[i];
    }
}

static void matrix_mul(const int *a, const int *b, int *res, int rows, int cols) {
    int i, j, k;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            int sum = 0;
            for (k = 0; k < cols; k++) {
                sum += a[i * cols + k] * b[k * cols + j];
            }
            res[i * cols + j] = sum;
        }
    }
}

// Calculate permutation A(N, k) = N! / (N-k)!
static unsigned long long permutation(int N, int k) {
    unsigned long long result = 1;
    int i;
    if (k > N)
        return 0;
    for (i = N; i > N - k; i--) {
        result *= i;
    }
    return result;
}

static int __init math_module_init(void) {
    int dot, matrix_result[100], i, j;

    printk(KERN_INFO "[lab3:part1] Math module loaded\n");

    dot = dot_product(vec1, vec2, vec_size);
    printk(KERN_INFO "Dot product of vectors: %d\n", dot);

    matrix_add(mat1, mat2, matrix_result, mat_rows, mat_cols);
    printk(KERN_INFO "Matrix addition result:\n");
    for (i = 0; i < mat_rows; i++) {
        printk(KERN_CONT "| ");
        for (j = 0; j < mat_cols; j++) {
            printk(KERN_CONT "%d ", matrix_result[i * mat_cols + j]);
        }
        printk(KERN_CONT "|\n");
    }

    matrix_sub(mat1, mat2, matrix_result, mat_rows, mat_cols);
    printk(KERN_INFO "Matrix subtraction result:\n");
    for (i = 0; i < mat_rows; i++) {
        printk(KERN_CONT "| ");
        for (j = 0; j < mat_cols; j++) {
            printk(KERN_CONT "%d ", matrix_result[i * mat_cols + j]);
        }
        printk(KERN_CONT "|\n");
    }

    matrix_mul(mat1, mat2, matrix_result, mat_rows, mat_cols);
    printk(KERN_INFO "Matrix multiplication result:\n");
    for (i = 0; i < mat_rows; i++) {
        printk(KERN_CONT "| ");
        for (j = 0; j < mat_cols; j++) {
            printk(KERN_CONT "%d ", matrix_result[i * mat_cols + j]);
        }
        printk(KERN_CONT "|\n");
    }

    printk(KERN_INFO "Permutation A(%d, %d) = %llu\n", perm_n, perm_k, permutation(perm_n, perm_k));

    return 0;
}

static void __exit math_module_exit(void) {
    printk(KERN_INFO "[lab3:part1] Math module unloaded\n");
}

module_init(math_module_init);
module_exit(math_module_exit);
