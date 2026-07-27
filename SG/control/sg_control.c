/**
 * @file    sg_control.c
 * @brief   单相离网逆变控制环 — 电压 PR 外环 + 电流 PR 内环
 * @note    原 task_control_1p.c
 */
#include "sg_control.h"
#include "sg_global.h"
#include "sg_pi_pr.h"
#include "sg_hwcfg.h"
#include "sg_pwm.h"
#include "sg_protect.h"
#include "arm_math.h"

void sg_control_1p(void)
{
    static uint8_t  prev_active = 0;
    static PI_TypeDef  volt_pi;    /* 电压外环 PI (替代 PR 简化实现) */
    static PR_TypeDef  curr_pr;    /* 电流内环 PR */
    static float       sin_wt = 0.0f;
    static float       theta  = 0.0f;

    /* --- 停止状态 --- */
    if (!g_run_flag) {
        HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_RESET);
        if (prev_active) {
            sg_pwm_stop();
        }
        prev_active = 0;
        return;
    }

    /* --- 启动条件: 母线电压足够 --- */
    uint8_t active = (U_line[2] > OFFGRID_UDC_MIN);  /* U_line[2] = Vdc */
    uint8_t rising = (active && !prev_active);
    if (!active) {
        if (prev_active) {
            sg_pwm_stop();
        }
        prev_active = 0;
        return;
    }
    prev_active = active;

    if (rising) {
        HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_SET);
        f32_PI_Init(&volt_pi, 1.0f / CTRL_SAMPLE_RATE,
                    0.5f, 10.0f, OFFGRID_IREF_MAX, -OFFGRID_IREF_MAX);
        f32_PR_Init(&curr_pr, 1.0f / CTRL_SAMPLE_RATE,
                    5.0f, 200.0f, 2.0f * (float)PI * 50.0f, 2.0f * (float)PI * 5.0f);
        theta = 0.0f;
        g_protect_mask = PROT_IOUT_OC | PROT_UDC_OV | PROT_UDC_UV;
        sg_pwm_set_duty(0.5f);
        HAL_HRTIM_WaveformOutputStart(&hhrtim1,
            HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 |
            HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);
    }

    /* --- 1) 生成正弦参考 --- */
    theta += 2.0f * (float)PI * 50.0f / CTRL_SAMPLE_RATE;
    if (theta > 2.0f * (float)PI) theta -= 2.0f * (float)PI;
    sin_wt = arm_sin_f32(theta);

    /* --- 2) 电压外环: Vref → Iref --- */
    float v_out = U_line[1];                          /* Vout 瞬时值 */
    float v_ref = g_vref_rms * 1.414f * sin_wt;       /* 正弦参考 [V pk] */
    float i_ref = f32_PI_Calculate(&volt_pi, v_ref, v_out);

    if (i_ref >  OFFGRID_IREF_MAX) i_ref =  OFFGRID_IREF_MAX;
    if (i_ref < -OFFGRID_IREF_MAX) i_ref = -OFFGRID_IREF_MAX;

    /* --- 3) 电流内环: Iref → Duty --- */
    float i_out = I_line[0];                          /* Iout 瞬时值 */
    float err_i = i_ref - i_out;
    float duty  = f32_PR_Calculate(&curr_pr, err_i);

    /* 偏置到 0.5 (双极性) */
    duty = duty * 0.1f + 0.5f;
    if (duty > 0.95f) duty = 0.95f;
    if (duty < 0.05f) duty = 0.05f;

    /* --- 4) 输出 PWM --- */
    sg_pwm_set_duty(duty);

    g_dbg_err   = v_ref - v_out;
    g_dbg_vctrl = duty;
}
