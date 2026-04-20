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
 * 文件名：[ky_utils.c]
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
#include "ky_utils.h"

/***************************************************************
 *  @brief     快速计算 10 的 N 次方
 *  @param     n: 指数
 *  @return    uint32: 10^n 结果
 **************************************************************/
uint32 utils_pow10(uint8 n)
{
    uint32 r = 1;
    while (n--)
        r *= 10;
    return r;
}

/***************************************************************
 *  @brief     整数转字符串 (有符号)
 *  @param     num: 待转换整数
 *  @param     buf: 输出缓冲区 (建议至少 12 字节)
 *  @return    uint8: 转换后的字符串长度
 **************************************************************/
uint8 utils_int2_str(int32 num, char *buf)
{
    char temp[12];
    uint8 i = 0, j = 0;
    uint8 isNeg = 0;

    if (num == 0)
    {
        buf[0] = '0';
        buf[1] = '\0';
        return 1;
    }

    if (num < 0)
    {
        isNeg = 1;
        num = -num;
    }
    while (num > 0)
    {
        temp[i++] = (num % 10) + '0';
        num /= 10;
    }
    if (isNeg)
    {
        buf[j++] = '-';
    }
    while (i > 0)
    {
        buf[j++] = temp[--i];
    }
    buf[j] = '\0';

    return j;
}

/***************************************************************
 *  @brief     整数转字符串 (无符号)
 *  @param     num: 待转换整数
 *  @param     buf: 输出缓冲区
 *  @return    uint8: 转换后的字符串长度
 **************************************************************/
uint8 utils_u_int2_str(uint32 num, char *buf)
{
    char temp[12];
    uint8 i = 0, j = 0;

    if (num == 0)
    {
        buf[0] = '0';
        buf[1] = '\0';
        return 1;
    }

    while (num > 0)
    {
        temp[i++] = (num % 10) + '0';
        num /= 10;
    }

    while (i > 0)
    {
        buf[j++] = temp[--i];
    }
    buf[j] = '\0';

    return j;
}

/***************************************************************
 *  @brief     浮点数转字符串
 *  @param     num: 待转换浮点数
 *  @param     buf: 输出缓冲区 (建议至少 16 字节)
 *  @param     frac_len: 保留小数位数 (0~9)
 *  @return    uint8: 转换后的字符串长度
 **************************************************************/
uint8 utils_float2_str(float32 num, char *buf, uint8 frac_len)
{
    int32 intPart;
    uint32 fracPart;
    uint32 multiplier;
    uint8 i = 0, len = 0;

    if (num < 0)
    {
        buf[len++] = '-';
        num = -num;
    }

    intPart = (int32)num;
    multiplier = utils_pow10(frac_len);
    fracPart = (uint32)((num - intPart) * multiplier + 0.5f);
    if (fracPart >= multiplier)
    {
        intPart++;
        fracPart = 0;
    }
    if (intPart == 0)
    {
        buf[len++] = '0';
    }
    else
    {
        char temp[12];
        uint8 k = 0;
        while (intPart > 0)
        {
            temp[k++] = (intPart % 10) + '0';
            intPart /= 10;
        }
        while (k > 0)
        {
            buf[len++] = temp[--k];
        }
    }

    if (frac_len > 0)
    {
        buf[len++] = '.';
        {
            char temp[10];
            for (i = 0; i < frac_len; i++)
            {
                temp[i] = (fracPart % 10) + '0';
                fracPart /= 10;
            }
            for (i = frac_len; i > 0; i--)
            {
                buf[len++] = temp[i - 1];
            }
        }
    }

    buf[len] = '\0';
    return len;
}
