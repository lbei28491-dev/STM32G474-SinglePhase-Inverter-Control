#ifndef __TASK_PREPROC_H
#define __TASK_PREPROC_H

#include "main.h"

/* ADC 预处理任务：从原始 ADC 码值计算物理量
 * 在 ADC1 注入完成 ISR 中调用（由 g_adc_preproc 挂接）
 *
 * ADC 通道映射（adc.c）：
 *   ADC1 注入 Rank1 (PA2)  → 输入电流 Iin  → I_line[1]
 *   ADC1 注入 Rank2 (PA3)  → 输入电压 Vin  → U_line[1]
 *   ADC2 注入 Rank1 (PA0)  → 输出电流 Iout → I_line[0]
 *   ADC2 注入 Rank2 (PA1)  → 输出电压 Vout → U_line[0]
 *
 * 计算项：
 *   - I_line[0] = 输出瞬时电流 [A]
 *   - I_line[1] = 输入瞬时电流 [A]
 *   - U_line[0] = 输出瞬时电压 [V]
 *   - U_line[1] = 输入母线电压 [V]
 *   - g_uab_rms = 输出电压 RMS
 *   - g_irms    = 输出电流 RMS
 */
void Task_Preproc(void);

#endif /* __TASK_PREPROC_H */
