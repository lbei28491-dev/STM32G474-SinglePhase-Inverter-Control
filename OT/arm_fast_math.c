/* CMSIS-DSP FastMath 替代实现 — 使用标准 C 数学库
 * 当 DSP 库未正确链接时提供 arm_sin_f32 / arm_cos_f32 / arm_sin_q15 / arm_cos_q15
 * Cortex-M4F 硬件浮点下 sinf/cosf 性能足够
 */
#include "arm_math.h"
#include <math.h>

float32_t arm_sin_f32(float32_t x)
{
    return sinf(x);
}

float32_t arm_cos_f32(float32_t x)
{
    return cosf(x);
}

q15_t arm_sin_q15(q15_t x)
{
    /* 输入 Q15 [0, 0x7FFF] 映射到 [0, 2*pi) */
    float angle = (float)(uint16_t)x * (2.0f * PI / 65536.0f);
    return (q15_t)(sinf(angle) * 32768.0f);
}

q15_t arm_cos_q15(q15_t x)
{
    float angle = (float)(uint16_t)x * (2.0f * PI / 65536.0f);
    return (q15_t)(cosf(angle) * 32768.0f);
}
