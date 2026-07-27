/**
 * @file    sg_preproc.c
 * @brief   ADC 预处理 — 从原始码值计算物理量 + RMS 滑窗
 * @note    原 task_preproc.c
 */
#include "sg_preproc.h"
#include "sg_global.h"
#include "sg_hwcfg.h"
#include "sg_pi_pr.h"
#include "arm_math.h"

/* RMS 滑窗: 50kHz × 20ms = 1000 点 */
#define RMS_WIN_LEN  1000

static float    v_buf[RMS_WIN_LEN], i_buf[RMS_WIN_LEN];
static uint16_t rms_idx = 0, rms_fill = 0;
static float    v_sq_sum = 0.0f, i_sq_sum = 0.0f;

void sg_preproc(void)
{
    /* --- 1) 输出电流 (ADC2 Rank1, PA0, 交流偏置1.65V) --- */
    I_line[0] = ((float)g_adc2_r1 - 2048.0f) * current_const;

    /* --- 2) 输出电压 (ADC2 Rank2, PA1, 交流偏置1.65V) --- */
    U_line[1] = ((float)g_adc2_r2 - 2048.0f) * Vout_coefficient;

    /* --- 3) 输入电流 (ADC1 Rank1, PA2, 交流偏置1.65V) --- */
    I_line[1] = ((float)g_adc1_r1 - 2048.0f) * current_const;

    /* --- 4) 输入母线电压 (ADC1 Rank2, PA3, DC直通, 实测校准) --- */
    U_line[2] = (float)g_adc1_r2 * U_coefficient;

    /* --- 5) RMS 滑窗 --- */
    float v_sq_new = U_line[1] * U_line[1];
    float i_sq_new = I_line[0] * I_line[0];

    if (rms_fill < RMS_WIN_LEN) {
        v_sq_sum += v_sq_new;
        i_sq_sum += i_sq_new;
        rms_fill++;
    } else {
        v_sq_sum += v_sq_new - v_buf[rms_idx] * v_buf[rms_idx];
        i_sq_sum += i_sq_new - i_buf[rms_idx] * i_buf[rms_idx];
    }
    v_buf[rms_idx] = U_line[1];
    i_buf[rms_idx] = I_line[0];
    rms_idx = (rms_idx + 1) % RMS_WIN_LEN;

    float n = (float)rms_fill;
    if (n < 1.0f) n = 1.0f;
    g_vout_rms = sqrtf(v_sq_sum / n);
    g_irms     = sqrtf(i_sq_sum / n);
    g_iin      = I_line[1];
}