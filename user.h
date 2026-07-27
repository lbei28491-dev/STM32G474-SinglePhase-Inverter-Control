#ifndef __USER_H
#define __USER_H

#include "main.h"
#include "pi_pr_ctrl.h"
#include "hardware_def.h"
#include "adc.h"     /* hadc1, hadc2 */
#include "dac.h"     /* hdac1 */
#include "hrtim.h"   /* hhrtim1 */
#include "i2c.h"     /* hi2c4 (OLED) */

/* ============= user.c 中定义的全局变量 ============= */
extern uint8_t  Run_Flag;                                  /* 1=运行, 0=停止 */
extern PI_TypeDef        Voltage_PI_Loop;
extern PR_TypeDef        Current_PR_Loop_alpha;
extern Integral_TypeDef Sine_Phase_Integrator;

extern float U_line[4];    /* [0]=输出瞬时电压, [1]=输入母线电压, [2..3]=备用 */
extern float I_line[3];    /* [0]=输出瞬时电流, [1]=输入瞬时电流, [2]=备用 */

extern float g_duty_a, g_duty_b, g_duty_c;
extern float g_uab_rms, g_ubc_rms, I_mag, g_irms;
extern float U_coefficient, Vout_coefficient, current_const;
extern float Iref_alpha, Iref_beta;

extern volatile int32_t g_il1, g_il2, g_il3;    /* ADC 原始码值 */
extern volatile uint8_t g_adc_data_ready;
extern float g_dbg_err, g_dbg_vctrl, g_isr_khz;
extern float g_vref_rms;             /* 输出电压参考 [V RMS], 由按键调节 */

extern float   g_pfc_phase_deg;       /* 期望相位角 [deg] */
extern uint8_t g_oled_ok;             /* OLED 是否初始化成功 */
extern float   g_grid_phi_deg;        /* 锁相角 [deg] */
extern float   g_wt;                  /* 归一化电角度 [0~1] */
extern uint8_t g_pll_locked;
extern float   g_freq_est;            /* 估计频率 [Hz] */
extern volatile float g_sin_wt;

/* ADC1 + ADC2 注入原始码值（由 main.c 回调填充） */
extern volatile uint32_t g_adc1_r1;   /* ADC1 Rank1 = PA2 输入电流 */
extern volatile uint32_t g_adc1_r2;   /* ADC1 Rank2 = PA3 输入电压 */
extern volatile uint32_t g_adc2_r1;   /* ADC2 Rank1 = PA0 输出电流 */
extern volatile uint32_t g_adc2_r2;   /* ADC2 Rank2 = PA1 输出电压 */
extern volatile uint8_t  g_adc1_done;
extern volatile uint8_t  g_adc2_done;

/* ============= 函数指针钩子（由 task 模块挂接） ============= */
extern void (*g_control_isr)(void);   /* 控制环 ISR */
extern void (*g_adc_preproc)(void);   /* ADC 预处理 */
extern void (*g_display_fn)(void);    /* 显示更新 (可选, NULL=不使用) */
extern void (*g_vofa_fn)(void);       /* VOFA 数据上传 (可选, NULL=不使用) */

/* ============= 全局函数 ============= */
void user_Init(void);
void Task_Button_Scan(void);

#endif /* __USER_H */
