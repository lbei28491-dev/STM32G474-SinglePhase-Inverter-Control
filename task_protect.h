#ifndef __TASK_PROTECT_H
#define __TASK_PROTECT_H

#include "main.h"
#include "hardware_def.h"

/* 当前故障码（按 PROT_xxx 位组合；0 表示无故障） */
extern volatile uint16_t g_fault_code;
/* 当前故障屏蔽字（哪些位参与检测） */
extern volatile uint16_t g_protect_mask;

/* 初始化 */
void Task_Protect_Init(void);

/* 在主调度器内被周期调用：根据采样值检测故障，
 * 命中即置位 g_fault_code 并强制 Run_Flag=0
 * 参数：U_dc 母线电压, I_l1 输出电流, U_ch1 ADC2 通道1, U_ch2 ADC2 通道2
 */
void Task_Protect_Check(float U_dc, float I_l1, float U_ch1, float U_ch2);

/* 跳闸：立即关 PWM、断继电器、置 Run_Flag=0 */
void Task_Protect_Trip(uint16_t fault_mask);

/* 故障清零（在 Run_Flag 由 0->1 时调用） */
void Task_Protect_Clear(void);

#endif /* __TASK_PROTECT_H */
