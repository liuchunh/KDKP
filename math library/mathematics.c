#include "mathematics.h"
#include <math.h>

const double eps = 1e-10;
const long double PI = 3.14159265358979323846;
const long double E = 2.71828182845904523536;
const double my_nan = NAN;

static const double LN2 = 0.69314718055994530942;

LL my_abs(LL x){
    return x < 0 ? -x : x;
}

double my_fabs(double x){
    return x < 0 ? -x : x;
}

double fmax(double a, double b){
    return a > b ? a : b;
}

double fmin(double a, double b){
    return a < b ? a : b;
}

/**
 * @brief 通过快速幂算法计算 a 的 b 次幂
 * 
 * @param a 底数
 * @param b 指数
 * @return long long
 * 
 * @note i for integer
 */
LL ipow(LL a, LL b){
    if (b == 0){
        if (a == 0)
            return my_nan;
        return 1;
    }

    LL res = 1;
    while (b > 0){
        if (b & 1){
            res *= a;
        }
        a *= a;
        b >>= 1;
    }
    return res;
}

/**
 * @brief 通过快速幂算法计算 a 的 b 次幂
 * 
 * @param a 底数 `double` 类型
 * @param b 指数 `long long` 类型
 * @return double 
 */
double fpow(double a, LL b){
    if (b == 0){
        if (a == 0)
            return my_nan;
        return 1;
    }

    double res = 1.0;
    while (b > 0){
        if (b & 1){
            res *= a;
        }
        a *= a;
        b >>= 1;
    }
    return res;
}

/**
 * @brief 通过二分法求 a 的平方根
 * 
 * @param a 
 * @return double 
 * 
 * @note 与牛顿迭代法比较 二分法通常更慢 但求值更稳定
 */
double sqrt_bs(double a){
    if (a < 0){
        return my_nan; // sqrt(a) 在 a < 0 时无定义
    }
    
    if (a == 0)
        return 0.0;

    double left = fmin(0, a);
    double right = fmax(0, a);

    while (right - left > eps){
        double mid = (left + right) / 2;
        if (mid * mid < a){
            left = mid;
        }
        else {
            right = mid;
        }
    }

    return (left + right) / 2;
}

/**
 * @brief 通过牛顿迭代法求 a 的平方根
 * 
 * @param a 
 * @return double 
 * 
 * @note 与二分法比较 牛顿迭代法通常更快 但求值不稳定 可能会出现震荡或者发散的情况
 */
double sqrt_nt(double a){
    if (a < 0){
        return my_nan;
    }
    
    if (a == 0)
        return 0.0;

    double x = a;
    while (my_fabs(x * x - a) > eps){
        x = (x + a / x) / 2;
    }

    return x;
}

/**
 * @brief 将角度转换为弧度
 * 
 * @param deg 
 * @return double 
 */
double Deg2Rad(double deg){
    return deg * PI / 180.0;
}

/**
 * @brief 将弧度转换为角度
 * 
 * @param rad 
 * @return double 
 */
double Rad2Deg(double rad){
    return rad * 180.0 / PI;
}

/**
 * @brief 通过泰勒级数展开计算 sin(x) 的近似值
 * 
 * @param x 
 * @return double 
 */
double taylor_sin(double x){
    while (x >= 2 * PI)
        x -= 2 * PI;
    while (x < 0)
        x += 2 * PI;

    double term = x; // 第一项
    double sum = term; // 累加和
    int n = 1; // 当前项的阶数

    while (my_fabs(term) > eps){
        term *= -x * x / ((n + 1) * (n + 2)); // 计算下一项
        sum += term; // 累加
        n += 2; // 阶数增加2
    }

    return sum;
}

/**
 * @brief 通过泰勒级数展开以及诱导公式计算 cos(x) 的近似值
 * 
 * @param x 
 * @return double 
 */
double taylor_cos(double x){
    return taylor_sin(x + PI / 2);
}

/**
 * @brief 通过泰勒级数展开计算 ln(x) 的近似值
 * 
 * @param x 
 * @return double 
 */
double ln(double x){
    if (isnan(x))
        return my_nan;

    if (x <= 0){
        return my_nan; // ln(x) 在 x <= 0 时无定义
    }

    double term = (x - 1) / (x + 1); // 第一项
    double sum = term; // 累加和
    int n = 1; // 当前项的阶数

    while (my_fabs(term) > eps){
        term *= ((x - 1) * (x - 1)) / ((x + 1) * (x + 1)) * (2 * n - 1) / (2 * n + 1); // 计算下一项
        sum += term; // 累加
        n ++; // 阶数增加1
    }

    return 2 * sum;
}

/**
 * @brief 通过泰勒级数展开计算 log(a, b) 的近似值
 * 
 * @param a 底数
 * @param b 真数
 * @return double 
 * 
 * @note log(a, b) = ln(b) / ln(a) 其中 ln(x) 是自然对数函数
 */
double my_log(double a, double b){
    if (a <= 0 || b <= 0 || a == 1){
        return my_nan; // log(a, b) 在 a <= 0 或 b <= 0 或 a == 1 时无定义
    }

    return ln(b) / ln(a);
}

/**
 * @brief 通过泰勒级数展开计算 exp(x) 的近似值
 * 
 * @param x 
 * @return double 
 * 
 * @note 就是 e^x
 */
double taylor_exp(double x){
    double term = 1.0; // 第一项
    double sum = term; // 累加和
    int n = 1; // 当前项的阶数

    while (my_fabs(term) > eps){
        term *= x / n; // 计算下一项
        sum += term; // 累加
        n++; // 阶数增加1
    }

    return sum;
}

/**
 * @brief 求 n 的阶乘
 * 
 * @param n 
 * @return LL 
 */
LL factorial(LL n){
    if (n < 0)
        return my_nan; // 阶乘在 n < 0 时无定义
    if (n == 0 || n == 1)
        return 1;

    LL res = 1;
    for (LL i = 2; i <= n; i ++){
        res *= i;
    }
    return res;
}

/**
 * @brief 检查一个数是否为 NaN
 * 
 * @param x 
 * @return double 
 * 
 * @note 详细的可以去看 C 数学库的 isnan 函数
 * @note STATIC FUNCTION
 */
static double isNAN(double x){
    return x != x; // NaN 是唯一一个不等于自己的数
}

/**
 * @brief 确定一个数是否为正无穷或负无穷
 * 
 * @param x 
 * @return int 
 * 
 * @note STATIC FUNCTION
 */
static int isINF(double x){
    return !isNAN(x) && isNAN(x - x);
}

/**
 * @brief 确定一个数的符号位
 * 
 * @param x 
 * @return int 
 * 
 * @note STATIC FUNCTION
 */
static int my_signbit(double x){
    if (x == 0.0){
        return (1.0 / x) < 0.0; // 区分 +0 和 -0
    }
    return x < 0.0;
}

/**
 * @brief 求 arctan(x) 的近似值
 * 
 * @param x 
 * @return double 
 */
double arctan(double x)
{
    const double PI_HALF = 1.57079632679489661923;
    const double PI_QUARTER = 0.78539816339744830962;

    if (isNAN(x)) {
        return x;
    }

    double ax = my_fabs(x);
    int sign = (x < 0.0);
    int region = 0;

    /* 区间缩减，尽量把自变量压到小区间提高逼近精度 */
    if (ax > 2.4142135623730950488) {
        ax = 1.0 / ax;
        region = 2;
    }
    else if (ax > 0.4142135623730950488) {
        ax = (ax - 1.0) / (ax + 1.0);
        region = 1;
    }

    /* [5/5] Padé 逼近：atan(z) ~= z * (105 + 10z^2 + z^4) / (105 + 45z^2 + 6z^4) */
    double z2 = ax * ax;
    double num = ax * (105.0 + z2 * (10.0 + z2));
    double den = 105.0 + z2 * (45.0 + 6.0 * z2);
    double y = num / den;

    if (region == 1) {
        y += PI_QUARTER;
    }
    else if (region == 2) {
        y = PI_HALF - y;
    }

    return sign ? -y : y;
}

/**
 * @brief 求 arctan(y/x) 的近似值
 * 
 * @param y 
 * @param x 
 * @return double 
 * 
 * @note 这个相较 arctan(y/x) 的好处在于它能正确处理 x 和 y 的符号，从而返回正确的象限的角度值，而不仅仅是 arctan(y/x) 的值 
 */
double my_atan2(double y, double x){
    if (isNAN(x) || isNAN(y)){
        return my_nan;
    }

    int sx = my_signbit(x);
    int sy = my_signbit(y);
    int x_isINF = isINF(x);
    int y_isINF = isINF(y);

    if (x_isINF && y_isINF){
        if (!sx && !sy) return PI / 4.0;
        if (sx && !sy) return 3.0 * PI / 4.0;
        if (!sx && sy) return -PI / 4.0;
        return -3.0 * PI / 4.0;
    }

    if (x_isINF){
        if (!sx) return sy ? -0.0 : 0.0;
        return sy ? -PI : PI;
    }

    if (y_isINF){
        return sy ? -PI / 2.0 : PI / 2.0;
    }

    if (x == 0.0){
        if (y == 0.0){
            if (!sx) return sy ? -0.0 : 0.0;
            return sy ? -PI : PI;
        }
        return sy ? -PI / 2.0 : PI / 2.0;
    }

    if (y == 0.0){
        if (x > 0.0) return sy ? -0.0 : 0.0;
        return sy ? -PI : PI;
    }

    double a = arctan(y / x);
    if (x > 0.0){
        return a;
    }
    return (y > 0.0) ? (a + PI) : (a - PI);
}