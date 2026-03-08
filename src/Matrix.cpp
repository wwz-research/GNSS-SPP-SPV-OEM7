#include "SPP.h"
//使用eign库，函数参数可直接使用Eigen中的矩阵或者向量，方便矩阵运算

/****************************************************************************
  CopyArray

  目的：将输入矩阵a的内容复制到输出矩阵b

****************************************************************************/
void CopyArray(int size, double* dest, const double* src) {
    for (int i = 0; i < size; i++) {
        dest[i] = src[i];
    }
}

/****************************************************************************
  MatrixInv

  目的：矩阵求逆,采用全选主元高斯-约当法

  参数:
  n      M1的行数和列数
  a      输入矩阵
  b      输出矩阵   b=inv(a)
  返回值：1=正常，0=无法求逆

****************************************************************************/
int MatrixInv(int n, double a[], double b[])
{
    int i, j, k, l, u, v, is[160] = { 0 }, js[160] = { 0 };   /* matrix dimension <= 10 */
    double d, p;

    /* 将输入矩阵赋值给输出矩阵b，下面对b矩阵求逆，a矩阵不变 */
    CopyArray(n * n, b, a);
    for (k = 0; k < n; k++)
    {
        d = 0.0;
        for (i = k; i < n; i++)   /* 查找右下角方阵中主元素的位置 */
        {
            for (j = k; j < n; j++)
            {
                l = n * i + j;
                p = fabs(b[l]);
                if (p > d)
                {
                    d = p;
                    is[k] = i;
                    js[k] = j;
                }
            }
        }

        if (d < 1.0E-15)
        {
            printf("Divided by 0 in MatrixInv!\n");
            return 0;
        }

        if (is[k] != k)  /* 对主元素所在的行与右下角方阵的首行进行调换 */
        {
            for (j = 0; j < n; j++)
            {
                u = k * n + j;
                v = is[k] * n + j;
                p = b[u];
                b[u] = b[v];
                b[v] = p;
            }
        }

        if (js[k] != k)  /* 对主元素所在的列与右下角方阵的首列进行调换 */
        {
            for (i = 0; i < n; i++)
            {
                u = i * n + k;
                v = i * n + js[k];
                p = b[u];
                b[u] = b[v];
                b[v] = p;
            }
        }

        l = k * n + k;
        b[l] = 1.0 / b[l];  /* 初等行变换 */
        for (j = 0; j < n; j++)
        {
            if (j != k)
            {
                u = k * n + j;
                b[u] = b[u] * b[l];
            }
        }
        for (i = 0; i < n; i++)
        {
            if (i != k)
            {
                for (j = 0; j < n; j++)
                {
                    if (j != k)
                    {
                        u = i * n + j;
                        b[u] = b[u] - b[i * n + k] * b[k * n + j];
                    }
                }
            }
        }
        for (i = 0; i < n; i++)
        {
            if (i != k)
            {
                u = i * n + k;
                b[u] = -b[u] * b[l];
            }
        }
    }

    for (k = n - 1; k >= 0; k--)  /* 将上面的行列调换重新恢复 */
    {
        if (js[k] != k)
        {
            for (j = 0; j < n; j++)
            {
                u = k * n + j;
                v = js[k] * n + j;
                p = b[u];
                b[u] = b[v];
                b[v] = p;
            }
        }
        if (is[k] != k)
        {
            for (i = 0; i < n; i++)
            {
                u = i * n + k;
                v = is[k] + i * n;
                p = b[u];
                b[u] = b[v];
                b[v] = p;
            }
        }
    }

    return (1);
}

/****************************************************************************
  MatrixInv_SRS

  目的：对称正定矩阵求逆

  参数:
  n      M1的行数和列数
  a      输入矩阵，输出为inv(a)
  返回值：1=正常，0=求逆失败

****************************************************************************/
int MatrixInv_SRS(const int n, double a[])
{
    int i, j, k, m;
    double w, g, b[160] = { 0 };   // 当前最大支持30维

    for (k = 0; k <= n - 1; k++)
    {
        w = a[0];

        if (fabs(w) < 1E-15) /* 主元素接近于0，矩阵不可逆 */
        {
            printf("Divided by 0 in MatrixInv_SRS!\n");
            return (0);
        }

        m = n - k - 1;
        for (i = 1; i <= n - 1; i++)
        {
            g = a[i * n];
            b[i] = g / w;
            if (i <= m) b[i] = -b[i];

            for (j = 1; j <= i; j++)
            {
                a[(i - 1) * n + j - 1] = a[i * n + j] + g * b[j];
            }
        }
        a[n * n - 1] = 1.0 / w;
        for (i = 1; i <= n - 1; i++)
        {
            a[(n - 1) * n + i - 1] = b[i];
        }
    }
    for (i = 0; i <= n - 2; i++)
    {
        for (j = i + 1; j <= n - 1; j++)
        {
            a[i * n + j] = a[j * n + i];
        }
    }

    return (1);
}


/****************************************************************************
  MatrixMultiply

  目的：矩阵相乘 M3 = M1*M2

  参数:
  m1      M1的行数
  n1      M1的列数
  m2      M2的行数
  n2      M2的列数
****************************************************************************/
void MatrixMultiply(int m1, int n1, int m2, int n2, const double M1[], const double M2[], double M3[])
{
    int i, j, k;
    double Sum;

    if ((n1 != m2) || (m1 <= 0) || (n1 <= 0) || (m2 <= 0) || (n2 <= 0))
    {
        printf("Error dimension in MatrixMultiply!\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < m1; i++)
    {
        for (j = 0; j < n2; j++)
        {
            Sum = 0.0;

            for (k = 0; k < n1; k++)
            {
                Sum = Sum + *(M1 + i * n1 + k) * *(M2 + k * n2 + j);
            }

            *(M3 + i * n2 + j) = Sum;
        }
    }
}