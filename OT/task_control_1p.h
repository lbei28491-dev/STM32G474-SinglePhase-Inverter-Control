#ifndef __TASK_CONTROL_1P_H
#define __TASK_CONTROL_1P_H

#include "main.h"

/* 离网单向逆变控制环
 * 在 ADC1 注入完成 ISR 中由 g_control_isr 调用 (50kHz)
 *
 * 拓扑: DC(Vin) -> H桥 -> AC(Vout)
 * 控制: 电压PR外环 -> 电流PR内环 -> 双极性SPWM
 * 目标: 输出 32V RMS / 50Hz 正弦波, 限流 5A RMS
 */
void Task_Control_1P(void);

#endif /* __TASK_CONTROL_1P_H */
