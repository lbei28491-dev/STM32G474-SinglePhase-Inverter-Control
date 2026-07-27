/**
 * @file    sg_control.h
 * @brief   单相离网逆变控制环
 * @note    原 task_control_1p.h
 *
 * 在 ADC 注入完成 ISR 中由 g_control_isr 调用 (50kHz)
 * 拓扑: H桥 → LC滤波 → 单相AC输出
 * 控制: 电压 PR 外环 → 电流 PR 内环 → 双极性 SPWM
 */
#ifndef SG_CONTROL_H
#define SG_CONTROL_H

#include "main.h"

void sg_control_1p(void);

#endif /* SG_CONTROL_H */