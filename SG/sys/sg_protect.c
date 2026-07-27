/**
 * @file    sg_protect.c
 * @brief   保护模块实现
 * @note    原 task_protect.c
 */
#include "sg_protect.h"
#include "sg_global.h"
#include "hrtim.h"
#include <math.h>

volatile uint16_t g_fault_code  = 0;
volatile uint16_t g_protect_mask = PROT_NONE;

#define PROT_OC_THRESHOLD   8.0f    /* 过流 [A] */
#define PROT_UDC_OV_THR     60.0f   /* 母线过压 [V] */
#define PROT_UDC_UV_THR     10.0f   /* 母线欠压 [V] */
#define PROT_CH1_OV_THR     3.3f    /* ADC 通道1 过压 [V] */
#define PROT_CH2_OV_THR     3.3f    /* ADC 通道2 过压 [V] */

void sg_protect_init(void)
{
    g_fault_code   = 0;
    g_protect_mask = PROT_NONE;
}

void sg_protect_trip(uint16_t fault_mask)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    g_fault_code |= fault_mask & g_protect_mask;
    g_run_flag    = 0;

    HAL_HRTIM_WaveformOutputStop(&hhrtim1,
        HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 |
        HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);

    HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_SET);
    if (primask == 0U) {
        __enable_irq();
    }
}

void sg_protect_clear(void)
{
    g_fault_code = 0;
    HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_RESET);
}

void sg_protect_check(float U_dc, float I_out, float U_ch1, float U_ch2)
{
    uint16_t m = g_protect_mask;
    uint16_t hit = 0;

    if ((m & PROT_IOUT_OC) && (fabsf(I_out) > PROT_OC_THRESHOLD)) hit |= PROT_IOUT_OC;
    if ((m & PROT_UDC_OV)  && (U_dc  > PROT_UDC_OV_THR))     hit |= PROT_UDC_OV;
    if ((m & PROT_UDC_UV)  && (U_dc  < PROT_UDC_UV_THR))     hit |= PROT_UDC_UV;
    if ((m & PROT_VOUT_OV) && (U_ch1 > PROT_CH1_OV_THR))     hit |= PROT_VOUT_OV;
    if ((m & PROT_VOUT_OV) && (U_ch2 > PROT_CH2_OV_THR))     hit |= PROT_VOUT_OV;

    if (hit) sg_protect_trip(hit);
}

void sg_protect_fast_check(float I_out)
{
    if ((g_protect_mask & PROT_IOUT_OC) &&
        (fabsf(I_out) > PROT_OC_THRESHOLD)) {
        sg_protect_trip(PROT_IOUT_OC);
    }
}
