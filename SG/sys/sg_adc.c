/**
 * @file    sg_adc.c
 * @brief   ADC 初始化 — 校准 + 启动注入转换
 * @note    原 task_adc.c
 */
#include "sg_adc.h"
#include "sg_global.h"
#include "sg_pi_pr.h"
#include "sg_hwcfg.h"
#include "sg_protect.h"
#include "hrtim.h"
#include "dac.h"

void sg_adc_init(void)
{
    /* ADC1, ADC2 校准 */
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);

    /* HRTIM ADC 触发后缩放: 50kHz / 1 */
    HAL_HRTIM_ADCPostScalerConfig(&hhrtim1, HRTIM_ADCTRIGGER_2, 0);

    /* 启动 ADC1 + ADC2 注入转换中断 */
    HAL_ADCEx_InjectedStart_IT(&hadc1);
    HAL_ADCEx_InjectedStart_IT(&hadc2);

    /* 启动 DAC1 通道1 (PA4 调试输出) */
    HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
}