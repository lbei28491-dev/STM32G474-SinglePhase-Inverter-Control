#include "task_protect.h"
#include "user.h"
#include "hrtim.h"

volatile uint16_t g_fault_code  = 0;
volatile uint16_t g_protect_mask = PROT_NONE;

/* 保护阈值 */
#ifndef PROT_OC_THRESHOLD
#define PROT_OC_THRESHOLD       8.0f       /* A */
#endif
#ifndef PROT_UDC_OV_THRESHOLD
#define PROT_UDC_OV_THRESHOLD  60.0f      /* V */
#endif
#ifndef PROT_UDC_UV_THRESHOLD
#define PROT_UDC_UV_THRESHOLD  10.0f      /* V */
#endif
#ifndef PROT_CH1_OV_THRESHOLD
#define PROT_CH1_OV_THRESHOLD  3.3f       /* V */
#endif
#ifndef PROT_CH2_OV_THRESHOLD
#define PROT_CH2_OV_THRESHOLD  3.3f       /* V */
#endif
#ifndef PROT_CH2_UV_THRESHOLD
#define PROT_CH2_UV_THRESHOLD  0.2f       /* V */
#endif

void Task_Protect_Init(void)
{
    g_fault_code  = 0;
    g_protect_mask = PROT_NONE;
}

void Task_Protect_Trip(uint16_t fault_mask)
{
    __disable_irq();
    g_fault_code  |= fault_mask & g_protect_mask;
    Run_Flag       = 0;

    /* 关闭 PWM 输出 */
    HAL_HRTIM_WaveformOutputStop(&hhrtim1,
        HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 |
        HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);
    HAL_HRTIM_WaveformCounterStop(&hhrtim1,
        HRTIM_TIMERID_TIMER_B);

    HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_SET);
    __enable_irq();
}

void Task_Protect_Clear(void)
{
    g_fault_code = 0;
    HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_RESET);
}

void Task_Protect_Check(float U_dc, float I_l1, float U_ch1, float U_ch2)
{
    uint16_t m = g_protect_mask;
    uint16_t hit = 0;

    if ((m & PROT_IL1_OC)     && (I_l1  >  PROT_OC_THRESHOLD))      hit |= PROT_IL1_OC;
    if ((m & PROT_UDC_OV)     && (U_dc  >  PROT_UDC_OV_THRESHOLD))   hit |= PROT_UDC_OV;
    if ((m & PROT_UDC_UV)     && (U_dc  <  PROT_UDC_UV_THRESHOLD))   hit |= PROT_UDC_UV;
    if ((m & PROT_ADC2_R1_OV) && (U_ch1 >  PROT_CH1_OV_THRESHOLD))   hit |= PROT_ADC2_R1_OV;
    if ((m & PROT_ADC2_R2_OV) && (U_ch2 >  PROT_CH2_OV_THRESHOLD))   hit |= PROT_ADC2_R2_OV;
    if ((m & PROT_ADC2_R2_UV) && (U_ch2 <  PROT_CH2_UV_THRESHOLD))  hit |= PROT_ADC2_R2_UV;

    if (hit) Task_Protect_Trip(hit);
}
