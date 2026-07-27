#include "task_pwm_1p.h"
#include "hrtim.h"
#include "user.h"
#include "stm32g4xx_ll_hrtim.h"

#define PWM_PERIOD  51200U
#define PWM_M_MAX   0.90f    /* 含100ns死区余量, 50kHz: 20us周期 */
#define PWM_DZ      0.02f    // dead zone around m=0 (hysteresis)

extern float g_duty_a, g_duty_b;

// Edge-aligned bipolar SPWM: m -> [cmp_a, cmp_b]
// HRTIM: TA1/TB1 set at CMP1, reset at PER → duty = (PERIOD - CMP1) / PERIOD
// V_bridge = (leg_A - leg_B) * Vdc = (CMP_B - CMP_A) / PERIOD * Vdc
// m = v_ref/Vdc ∈ [-1, 1];  cmp_a = half - m*half;  cmp_b = half + m*half
// output: V_bridge = (cmp_b - cmp_a) / PERIOD * Vdc = m * Vdc
void _pwm_bipolar(float m)
{
    if (m >  PWM_M_MAX) m =  PWM_M_MAX;
    if (m < -PWM_M_MAX) m = -PWM_M_MAX;

    uint32_t half = PWM_PERIOD >> 1;
    uint32_t cmp_a = (uint32_t)(-m *(float)half + (float)half);  // half - m*half
    uint32_t cmp_b = (uint32_t)( m * (float)half + (float)half);  // half + m*half

    if (cmp_a > PWM_PERIOD) cmp_a = PWM_PERIOD;
    if (cmp_b > PWM_PERIOD) cmp_b = PWM_PERIOD;

    /* 直接写入 HRTIM 比较寄存器 (CMP1xR, 带预装载) */
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = cmp_a;
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = cmp_b;

    /* 强制软件更新: 将预装载值传输到活动寄存器 */
    LL_HRTIM_ForceUpdate(HRTIM1, LL_HRTIM_TIMER_A | LL_HRTIM_TIMER_B);
}

// unipolar frequency-doubling PWM: one leg active, the other held at 0
// m > 0 → A switches, B=0.  m < 0 → B switches, A=0.  ripple @ 2×f_sw
void _pwm_unipolar(float m)
{
    float abs_m = (m > 0.0f) ? m : -m;
    if (abs_m > 0.95f) abs_m = 0.95f;

    // ZC hysteresis: prevent noise toggling near m=0
    static int8_t leg = 0;
    if (leg == 0) {
        if      (m >  PWM_DZ) leg =  1;
        else if (m < -PWM_DZ) leg = -1;
    } else if (leg == 1 && m < PWM_DZ * 0.5f) {
        leg = 0;
    } else if (leg == -1 && m > -PWM_DZ * 0.5f) {
        leg = 0;
    }

    if (leg == 0) {
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, 0);
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, 0);
    } else if (leg == 1) {
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1,
            (uint32_t)(abs_m * (float)PWM_PERIOD));
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, 0);
    } else {
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, 0);
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1,
            (uint32_t)(abs_m * (float)PWM_PERIOD));
    }
    LL_HRTIM_ForceUpdate(HRTIM1, LL_HRTIM_TIMER_A | LL_HRTIM_TIMER_B);
}

// unipolar frequency-doubling PWM — for PFC (task_control_pfc.c)
// m ∈ [-1, 1] → leg A/B alternate, output ripple @ 2×f_sw
void Task_PWM_1P_UniUpdate(float m)
{
    if (!Run_Flag) return;

    float abs_m = (m > 0.0f) ? m : -m;
    if (abs_m > 0.95f) abs_m = 0.95f;

    // ZC hysteresis: prevent noise toggling near m=0
    static int8_t leg = 0;
    if (leg == 0) {
        if      (m >  PWM_DZ) leg =  1;
        else if (m < -PWM_DZ) leg = -1;
    } else if (leg == 1 && m < PWM_DZ * 0.5f) {
        leg = 0;
    } else if (leg == -1 && m > -PWM_DZ * 0.5f) {
        leg = 0;
    }

    uint32_t cmp_a, cmp_b;
    if (leg == 0) {
        cmp_a = cmp_b = 0;
    } else if (leg == 1) {
        cmp_a = (uint32_t)(abs_m * (float)PWM_PERIOD);
        cmp_b = 0;
    } else {
        cmp_a = 0;
        cmp_b = (uint32_t)(abs_m * (float)PWM_PERIOD);
    }

    if (cmp_a > PWM_PERIOD) cmp_a = PWM_PERIOD;
    if (cmp_b > PWM_PERIOD) cmp_b = PWM_PERIOD;

    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, cmp_a);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, cmp_b);

    /* Force immediate preload → active transfer for Timer A and Timer B.
     * With PRE_EN=1, CMP1xR writes go to the shadow (preload) register.
     * Setting TxSWU bits pushes them to the active register immediately. */
    LL_HRTIM_ForceUpdate(HRTIM1, LL_HRTIM_TIMER_A | LL_HRTIM_TIMER_B);
}
