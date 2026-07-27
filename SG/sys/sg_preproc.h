/**
 * @file    sg_preproc.h
 * @brief   ADC 预处理 — 原始码值 → 物理量
 * @note    原 task_preproc.h
 */
#ifndef SG_PREPROC_H
#define SG_PREPROC_H

#include "main.h"

/* ADC 注入完成 ISR 中调用 (g_adc_preproc 钩子)
 * ADC1 注入: PA2(Iin) → I_line[1], PA3(Vin) → U_line[2]
 * ADC2 注入: PA0(Iout) → I_line[0], PA1(Vout) → U_line[1]
 * 计算 RMS (滑窗 1000点 = 20ms @ 50kHz)
 */
void sg_preproc(void);

#endif /* SG_PREPROC_H */