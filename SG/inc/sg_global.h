/**
 * @file    sg_global.h
 * @brief   全局变量声明 + 函数指针钩子
 * @note    合并原 user.h 中的全局变量和 extern 声明
 */
#ifndef SG_GLOBAL_H
#define SG_GLOBAL_H

#include "main.h"
#include "sg_types.h"
#include "sg_hwcfg.h"
#include "adc.h"       /* hadc1, hadc2 */
#include "dac.h"       /* hdac1 */
#include "hrtim.h"     /* hhrtim1 */
#include "i2c.h"       /* hi2c4 (OLED) */

/* ======================== 运行状态 ======================== */
extern volatile uint8_t g_run_flag;             /* 1=运行, 0=停止 */
extern float    U_line[3];                      /* [0]=保留, [1]=Vout, [2]=Vdc */
extern float    I_line[2];                      /* [0]=Iout, [1]=Iin */
extern float    g_duty;                         /* PWM 占空比 [0~1] */
extern float    g_vout_rms, g_iin, g_irms;      /* 统计量 */
extern float    U_coefficient, Vout_coefficient, current_const;
extern float    g_dbg_err, g_dbg_vctrl, g_isr_khz; /* 调试 */
extern float    g_vref_rms;                     /* 输出电压参考 [V RMS], 按键可调 */
extern uint8_t  g_oled_ok;                      /* OLED 是否初始化成功 */

/* ADC 注入原始码值 (由 main.c 回调填充) */
extern volatile uint32_t g_adc1_r1, g_adc1_r2;  /* ADC1: PA2(Iin), PA3(Vin) */
extern volatile uint32_t g_adc2_r1, g_adc2_r2;  /* ADC2: PA0(Iout), PA1(Vout) */
extern volatile uint8_t  g_adc1_done, g_adc2_done;

/* ======================== 函数指针钩子 ======================== */
extern void (*g_control_isr)(void);   /* 控制环 ISR (50kHz) */
extern void (*g_adc_preproc)(void);   /* ADC 预处理 */
extern void (*g_display_fn)(void);    /* 显示更新 (可选) */
extern void (*g_vofa_fn)(void);       /* VOFA 数据上传 (可选) */

/* ADC 数据就绪标志 (由 ISR 置位, 主循环使用) */
extern volatile uint8_t  g_adc_data_ready;

#endif /* SG_GLOBAL_H */
