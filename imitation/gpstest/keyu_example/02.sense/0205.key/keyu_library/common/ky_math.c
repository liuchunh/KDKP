/*********************************************************************************
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 版权所有：[2025] [北京科宇通博科技有限公司]
 *
 * 许可协议：采用 GNU GPL v3.0 开源许可
 * 您可依据协议进行二次开发、传播，但须保留原始版权信息
 * 协议详情参见：https://www.gnu.org/licenses/gpl-3.0.html
 *
 * 免责声明：本组件库仅提供技术参考，使用方需自行验证适用性
 *
 * 协议文件：GPL v3.0 完整文本位于根目录下
 *
 * === 文件信息 ===
 * 文件名：[ky_math.c]
 * 开发单位：北京科宇通博科技有限公司
 * 适用环境：[AURIX TC264DA]
 * 官方渠道：
 *   - 代码仓库：[https://gitee.com/beijing-keyu---jiangxi/Keyu_TC264DA_Open_Source_Library]
 *   - 淘宝店铺：https://kyznc.taobao.com/
 *   - 技术支持：QQ群 974530818
 *
 * === 修订记录 ===
 * 日期       |  开发者  | 变更说明
 * -----------|----------|----------------------
 * 2026.01.07 |   毛毛   | V3.0
 *********************************************************************************/
#include "ky_math.h"

/***************************************************************
  *  @brief     使用 atan 函数模拟实现的 atan2 函数
  *  @param     y       y坐标值
  *  @param     x       x坐标值
  *  @return    float   返回计算出的弧度值, 范围为 (-PI, PI]
  *  @Sample usage:     float angle = Fatan2(1.0f, -1.0f);
 **************************************************************/
float Fatan2(float y, float x)
{
    float angle;

    if (x > 0.0f) {
        angle = atan(y / x);
    }
    else if (x < 0.0f) {
        if (y >= 0.0f) { 
            angle = atan(y / x) + KEYU_MATH_PI;
        } else { 
            angle = atan(y / x) - KEYU_MATH_PI;
        }
    }
    else {
        if (y > 0.0f) {      
            angle = KEYU_MATH_PI / 2.0f;
        } else if (y < 0.0f) { 
            angle = -KEYU_MATH_PI / 2.0f;
        } else {
            angle = 0.0f;
        }
    }
    return angle;
}

/***************************************************************
  *  @brief     计算浮点数的绝对值
  *  @param     x       输入的浮点数值
  *  @return    float   返回x的绝对值
  *  @Sample usage:     float val = Ffabs(-5.5f);
 **************************************************************/
float Ffabs(float x)
{
    return (x < 0.0f) ? -x : x;
}

/***************************************************************
  *  @brief     计算自然常数e的x次幂
  *  @param     x       指数值
  *  @return    float   返回e^x的结果
  *  @Sample usage:     float res = Fexp(2.0f);
 **************************************************************/
float Fexp(float x)
{
    return (float)exp((double)x);
}

/***************************************************************
  *  @brief     快速计算平方根的倒数 (1 / sqrt(x))
  *  @param     x       输入的正浮点数
  *  @return    float   返回 1 / sqrt(x) 的近似值
  *  @Sample usage:     float val = finv_sqrt_fast(4.0f);
 **************************************************************/
float finv_sqrt_fast(float x)
{
    float halfx = 0.5f * x;
    float y = x;
    int32_t i = *(int32_t*)&y;   
    i = 0x5f3759df - (i >> 1);   
    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y)); 
    return y;
}

/***************************************************************
  *  @brief     将角度转换为弧度
  *  @param     deg     角度值
  *  @return    float   对应的弧度值
  *  @Sample usage:     float rad = deg_to_rad(180.0f);
 **************************************************************/
float deg_to_rad(float deg)
{
    return deg * (KEYU_MATH_PI / 180.0f);
}

/***************************************************************
  *  @brief     将弧度转换为角度
  *  @param     rad     弧度值
  *  @return    float   对应的角度值
  *  @Sample usage:     float deg = rad_to_deg(3.14f);
 **************************************************************/
float rad_to_deg(float rad)
{
    return rad * (180.0f / KEYU_MATH_PI);
}

/***************************************************************
  *  @brief     查找字符串子串 (替代标准库 strstr)
  *  @param     s1      被查找的源字符串
  *  @param     s2      要查找的目标子串
  *  @return    char*   若找到，返回第一次出现的起始位置指针；否则返回 NULL
  *  @Sample usage:     char *ptr = keyu_strstr("Hello World", "World");
 **************************************************************/
char *keyu_strstr(const char *s1, const char *s2)
{
    const char *p1, *p2;

    if (!*s2)
    {
        return (char *)s1;
    }

    while (*s1)
    {
        p1 = s1;
        p2 = s2;

        while (*p1 && *p2 && (*p1 == *p2))
        {
            p1++;
            p2++;
        }

        if (!*p2)
        {
            return (char *)s1;
        }

        s1++;
    }

    return (char *)0; 
}

/***************************************************************
  *  @brief     将字符串转换为整数
  *  @param     str     要转换的字符串
  *  @return    int     转换后的整数值
 **************************************************************/
int keyu_atoi(const char* str)
{
    int res = 0;
    int sign = 1;
    
    // 跳过空格
    while (*str == ' ') str++;
    
    // 符号
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // 数字转换
    while (*str >= '0' && *str <= '9') {
        res = res * 10 + (*str - '0');
        str++;
    }
    
    return sign * res;
}

/***************************************************************
  *  @brief     将字符串转换为浮点数
  *  @param     str     要转换的字符串
  *  @return    float   转换后的浮点数值
 **************************************************************/
float keyu_atof(const char* str)
{
    float res = 0.0f;
    float frac = 1.0f;
    int sign = 1;
    int decimal = 0;
    
    // 跳过空格
    while (*str == ' ') str++;
    
    // 符号
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // 数字转换
    while ((*str >= '0' && *str <= '9') || *str == '.') {
        if (*str == '.') {
            decimal = 1;
            str++;
            continue;
        }
        
        if (!decimal) {
            res = res * 10.0f + (*str - '0');
        } else {
            frac *= 10.0f;
            res = res * 10.0f + (*str - '0');
        }
        str++;
    }
    
    if (decimal) {
        res /= frac;
    }
    
    return sign * res;
}

/***************************************************************
  *  @brief     运行FPU和数学库的健壮性检查 (空实现，预留)
  *  @param     None
 **************************************************************/
void run_fpu_and_math_check(void)
{
    // 预留用于测试数学函数和FPU的接口
}
