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
 * 文件名：[ky_math.h]
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
#ifndef __KY_MATH_H__
#define __KY_MATH_H__

#include <math.h>
#include "ky_typedef.h" 
#include <string.h>
#include <stdio.h>

#define KEYU_MATH_PI 3.1415926535f 

float keyu_atof(const char* str);              //字符串转浮点数
int keyu_atoi(const char* str);                //字符串转整数
float Fatan2(float y, float x);                //反正切函数
float finv_sqrt_fast(float x);                 //快速平方根倒数
float deg_to_rad(float deg);                   //角度转弧度
float rad_to_deg(float rad);                   //弧度转角度
float Ffabs(float x);                          //浮点数绝对值
float Fexp(float x);                           //指数函数
char *keyu_strstr(const char *s1, const char *s2);  //字符串查找
void run_fpu_and_math_check(void);             //FPU和数学库检查
#endif 