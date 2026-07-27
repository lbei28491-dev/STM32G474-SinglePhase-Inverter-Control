#include "task_preproc.h"
#include "user.h"
#include "hardware_def.h"
#include "hardware_def_1p.h"
#include "pi_pr_ctrl.h"
#include "arm_math.h"

/* ========================================================================
 * ADC 通道映射（adc.c, ADC1+ADC2 均为注入组, HRTIM_TRG2 触发）
 *   ADC1 注入 Rank1 (PA2)  -> 输入电流 Iin  -> I_line[1]
 *   ADC1 注入 Rank2 (PA3)  -> 输入电压 Vin  -> U_line[1] (母线DC)
 *   ADC2 注入 Rank1 (PA0)  -> 输出电流 Iout -> I_line[0]
 *   ADC2 注入 Rank2 (PA1)  -> 输出电压 Vout -> U_line[0] (AC)
 *
 * 原始码值由 main.c 的 HAL_ADCEx_InjectedConvCpltCallback 填入
 *   g_adc1_r1, g_adc1_r2, g_adc2_r1, g_adc2_r2 (uint32_t, 12bit单端无符号)
 * ======================================================================== */

/* RMS 滑窗长度：50kHz x 20ms = 1000 点 (1个50Hz周期) */
#define RMS_WIN_LEN   1000

static float    u_buf[RMS_WIN_LEN];
static float    i_buf[RMS_WIN_LEN];
static uint16_t rms_idx = 0;
static float    u_sq_sum = 0.0f;   /* 滑动累加器, O(1) 更新 */
static float    i_sq_sum = 0.0f;
static uint16_t rms_fill_cnt = 0;  /* 窗口填充计数 */

void Task_Preproc(void)
{
    /* --- 1) 输出电流 (ADC2 Rank1, PA0, 交流偏置1.65V→ADC≈2048) --- */
    I_line[0] = ((float)g_adc2_r1 - 2048.0f) * current_const;

    /* --- 2) 输出电压 (ADC2 Rank2, PA1, AC, 分压1k/68k, 偏置1.65V) --- */
    U_line[0] = ((float)g_adc2_r2 - 2048.0f) * Vout_coefficient;

    /* --- 3) 输入电流 (ADC1 Rank1, PA2, 交流偏置1.65V→ADC≈2048) --- */
    I_line[1] = ((float)g_adc1_r1 - 2048.0f) * current_const;

    /* --- 4) 输入母线电压 (ADC1 Rank2, PA3, DC直通, 实测校准) --- */
    U_line[1] = (float)g_adc1_r2 * U_coefficient;

    /* --- 5) RMS 滑动窗口 (O(1) 累加器, 避免每帧遍历1000点) --- */
    float u_sq_new = U_line[0] * U_line[0];
    float i_sq_new = I_line[0] * I_line[0];

    if (rms_fill_cnt < RMS_WIN_LEN) {
        /* 窗口未满, 直接累加 */
        u_sq_sum += u_sq_new;
        i_sq_sum += i_sq_new;
        rms_fill_cnt++;
    } else {
        /* 滑动窗口: 减旧值, 加新值 */
        u_sq_sum += u_sq_new - u_buf[rms_idx] * u_buf[rms_idx];
        i_sq_sum += i_sq_new - i_buf[rms_idx] * i_buf[rms_idx];
    }
    u_buf[rms_idx] = U_line[0];
    i_buf[rms_idx] = I_line[0];
    rms_idx = (rms_idx + 1) % RMS_WIN_LEN;

    float n = (float)rms_fill_cnt;
    if (n < 1.0f) n = 1.0f;
    g_uab_rms = sqrtf(u_sq_sum / n);
    g_irms    = sqrtf(i_sq_sum / n);
    g_ubc_rms = g_uab_rms;   /* 单相系统 */

    /* --- 6) 估计频率 (过零检测, 仅用于监控) --- */
    static float last_u = 0.0f;
    static uint32_t zc_last = 0;
    if (last_u < 0.0f && U_line[0] >= 0.0f) {
        uint32_t now = HAL_GetTick();
        if (zc_last != 0) {
            uint32_t dt = now - zc_last;   /* 半周期 ms */
            if (dt > 0) {
                float f = 1000.0f / (float)(dt * 2);
                g_freq_est = 0.9f * g_freq_est + 0.1f * f;
            }
        }
        zc_last = now;
    }
    last_u = U_line[0];

    if (g_uab_rms > 5.0f && g_freq_est > 45.0f && g_freq_est < 55.0f)
        g_pll_locked = 1;
    else
        g_pll_locked = 0;

    /* --- 7) 累积相位 wt (归一化 [0~1]) --- */
    g_wt += g_freq_est / CTRL_SAMPLE_RATE;
    if (g_wt >= 1.0f) g_wt -= 1.0f;
    g_sin_wt = arm_sin_f32(2.0f * (float)PI * g_wt);
}
