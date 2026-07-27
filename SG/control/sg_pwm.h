/**
 * @file    sg_pwm.h
 * @brief   单相 PWM 调制 — 双极性 SPWM
 * @note    原 task_pwm_1p.h
 */
#ifndef SG_PWM_H
#define SG_PWM_H

#include "main.h"

/* 单相双极性 SPWM:
 *   duty ∈ [0, 1], 周期 50kHz (HRTIM Period=51200)
 *   TA1: CMP1 置位, PER 复位 (上管)
 *   TA2: PER 置位, CMP1 复位 (下管, 互补)
 *   TB1/TB2 与 TA1/TA2 同步 (H桥另一臂)
 */
void sg_pwm_set_duty(float duty);
void sg_pwm_stop(void);

#endif /* SG_PWM_H */