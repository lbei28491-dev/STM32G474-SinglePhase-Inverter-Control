/**
 * @file    sg_pwm.c
 * @brief   单相双极性 SPWM 驱动
 * @note    原 task_pwm_1p.c
 *
 * HRTIM1 配置:
 *   TimerA: TA1(PA8)=上管, TA2(PA9)=下管 (互补, 死区已由 HAL 配置)
 *   TimerB: TB1(PA10)=上管, TB2(PA11)=下管
 *   中心对齐模式, 周期=51200 → 50kHz
 *   半桥中点电压: Vout = (duty) * Vdc (duty∈[0,1])
 *   双极性调制: 同一桥臂上下管互补, 两臂同步
 */
#include "sg_pwm.h"
#include "sg_global.h"
#include "hrtim.h"
#include "stm32g4xx_ll_hrtim.h"

#define PWM_PERIOD  51200U
#define PWM_NEUTRAL (PWM_PERIOD / 2U)

void sg_pwm_set_duty(float duty)
{
    if (duty < 0.0f) duty = 0.0f;
    if (duty > 1.0f) duty = 1.0f;

    /*
     * 全桥双极性调制:
     * Output1 在 CMP1→PER 期间为高，因此 Timer A 使用 (1-duty)，
     * Timer B 使用 duty，使两个桥臂互为反相。
     */
    uint32_t cmp_a = (uint32_t)((1.0f - duty) * (float)PWM_PERIOD);
    uint32_t cmp_b = (uint32_t)(duty * (float)PWM_PERIOD);
    if (cmp_a > PWM_PERIOD) cmp_a = PWM_PERIOD;
    if (cmp_b > PWM_PERIOD) cmp_b = PWM_PERIOD;

    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = cmp_a;
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = cmp_b;

    LL_HRTIM_ForceUpdate(HRTIM1,
        LL_HRTIM_TIMER_A | LL_HRTIM_TIMER_B);
    g_duty = duty;
}

void sg_pwm_stop(void)
{
    HAL_HRTIM_WaveformOutputStop(&hhrtim1,
        HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 |
        HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);

    /* 保持两个计数器同步运行，比较值回到零差模的 50% 安全点。 */
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = PWM_NEUTRAL;
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = PWM_NEUTRAL;
    LL_HRTIM_ForceUpdate(HRTIM1,
        LL_HRTIM_TIMER_A | LL_HRTIM_TIMER_B);
    g_duty = 0.5f;
}
