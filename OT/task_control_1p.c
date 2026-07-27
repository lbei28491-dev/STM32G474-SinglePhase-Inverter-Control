#include "task_control_1p.h"
#include "user.h"
#include "pi_pr_ctrl.h"
#include "hardware_def_1p.h"
#include "hardware_def.h"
#include "task_pwm_1p.h"
#include "task_protect.h"
#include "arm_math.h"

#define IREF_PK     (OFFGRID_IREF_MAX * 1.414f) /* 输出电流峰值 ~7.07A */

/* PR 控制器: 输出钳位, 大值表示不钳位 */
#define NO_CLAMP    32767

void Task_Control_1P(void)
{
    static uint8_t  prev_active = 0;
    static PR_TypeDef v_pr;      /* 电压外环 PR */
    static PR_TypeDef i_pr;      /* 电流内环 PR */

    /* --- 停止状态：逆变器未启用或被外部开关关断 --- */
    uint8_t inverter_enable = (HAL_GPIO_ReadPin(INV_EN_GPIO_Port, INV_EN_Pin) == GPIO_PIN_RESET);
    if (!Run_Flag || !inverter_enable) {
        HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_RESET);
        if (prev_active) {
            HAL_HRTIM_WaveformOutputStop(&hhrtim1,
                HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 |
                HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);
            HAL_HRTIM_WaveformCounterStop(&hhrtim1,
                HRTIM_TIMERID_TIMER_B);
        }
        prev_active = 0;
        return;
    }

    /* --- 启动条件: 母线电压足够 --- */
    uint8_t active = (U_line[1] > OFFGRID_UDC_MIN);
    uint8_t rising = (active && !prev_active);

    if (!active && prev_active) {
        HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_RESET);
        HAL_HRTIM_WaveformOutputStop(&hhrtim1,
            HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 |
            HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);
        HAL_HRTIM_WaveformCounterStop(&hhrtim1,
            HRTIM_TIMERID_TIMER_B);
    }
    prev_active = active;
    if (!active) return;

    /* --- 上升沿一次性初始化 --- */
    if (rising) {
        HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_SET);

        /* 电压外环 PR: Kp=0.5, Kr=30, f0=50Hz, BW=5Hz, Fs=50kHz
         * 输出 = 电流参考 [A], 钳位 ±IREF_PK */
        f32_PR_Init(&v_pr, 0.5f, 30.0f, 50.0f, 5.0f, CTRL_SAMPLE_RATE,
                     IREF_PK, -IREF_PK);

        /* 电流内环 PR: Kp=5, Kr=100, f0=50Hz, BW=10Hz, Fs=50kHz
         * 输出 = 电压指令 [V], 不钳位 (由调制比钳位) */
        f32_PR_Init(&i_pr, 5.0f, 100.0f, 50.0f, 10.0f, CTRL_SAMPLE_RATE,
                     NO_CLAMP, -NO_CLAMP);

        /* 重置相位积分器 */
        Sine_Phase_Integrator.x0 = Sine_Phase_Integrator.x1 = 50.0f;
        Sine_Phase_Integrator.y0 = Sine_Phase_Integrator.y1 = 0.0f;

        /* 启动 PWM 输出与 Timer B */
        HAL_HRTIM_WaveformOutputStart(&hhrtim1,
            HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 |
            HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);
        HAL_HRTIM_WaveformCounterStart(&hhrtim1,
            HRTIM_TIMERID_TIMER_B);

        /* 使能保护 */
        g_protect_mask = PROT_IL1_OC | PROT_UDC_OV | PROT_UDC_UV;
    }

    /* --- 1) 生成电压参考 (50Hz 正弦, 幅度由按键调节) --- */
    float wt = f32_Integral_Calculate(&Sine_Phase_Integrator, 50.0f);
    float v_ref = g_vref_rms * 1.414f * arm_sin_f32(2.0f * (float)PI * wt);

    /* --- 2) 电压外环 PR: v_err -> i_ref --- */
    float v_err = v_ref - U_line[0];
    float i_ref = f32_PR_Calculate(&v_pr, v_err);
    if (i_ref >  IREF_PK) i_ref =  IREF_PK;
    if (i_ref < -IREF_PK) i_ref = -IREF_PK;

    /* --- 3) 电流内环 PR: i_err -> v_ctrl --- */
    float i_err = i_ref - I_line[0];
    float v_ctrl = f32_PR_Calculate(&i_pr, i_err);

    /* --- 4) 调制比: m = v_ctrl / Vin --- */
    float udc = U_line[1];
    if (udc < 1.0f) udc = 1.0f;
    float m = v_ctrl / udc;

    /* --- 5) 双极性 SPWM 输出 --- */
    _pwm_bipolar(m);

    /* 调试变量 */
    g_dbg_err   = i_err;
    g_dbg_vctrl = v_ctrl;
}
