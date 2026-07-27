/**
 * @file    sg_protect.h
 * @brief   保护模块 — 过流/过压/欠压检测
 * @note    原 task_protect.h
 */
#ifndef SG_PROTECT_H
#define SG_PROTECT_H

#include "main.h"
#include "sg_hwcfg.h"

extern volatile uint16_t g_fault_code;
extern volatile uint16_t g_protect_mask;

void sg_protect_init(void);
void sg_protect_check(float U_dc, float I_out, float U_ch1, float U_ch2);
void sg_protect_fast_check(float I_out);
void sg_protect_trip(uint16_t fault_mask);
void sg_protect_clear(void);

#endif /* SG_PROTECT_H */
