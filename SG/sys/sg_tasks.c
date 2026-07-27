/**
 * @file    sg_tasks.c
 * @brief   周期任务注册
 * @note    原 user_tasks.c
 */
#include "sg_tasks.h"
#include "sg_global.h"
#include "sg_sched.h"
#include "sg_protect.h"
#include "sg_control.h"
#include "sg_preproc.h"
#include "sg_display.h"
#include "sg_user.h"

/* 保护检查包装 */
static void _protect_wrap(void)
{
    float u_ch1 = (float)g_adc2_r1 * (3.3f / 4096.0f);
    float u_ch2 = (float)g_adc2_r2 * (3.3f / 4096.0f);
    sg_protect_check(U_line[2], I_line[0], u_ch1, u_ch2);
}

void sg_tasks_register(void)
{
    /* ADC ISR 中执行顺序: 预处理 → 控制 */
    g_adc_preproc = sg_preproc;
    g_control_isr = sg_control_1p;

    /* 周期任务: 20ms 按键 / 100ms 显示 / 1ms 保护 */
    sg_sched_register(20,   sg_button_scan);
    if (g_oled_ok)
        sg_sched_register(100, sg_display_1p);
    sg_sched_register(1, _protect_wrap);
}