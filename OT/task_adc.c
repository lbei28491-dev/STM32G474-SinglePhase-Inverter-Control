#include "task_adc.h"
#include "user.h"
#include "pi_pr_ctrl.h"
#include "hardware_def.h"
#include "hardware_def_1p.h"
#include "task_protect.h"
#include "hrtim.h"
#include "dac.h"

/* --- ADC 硬件初始化 (在 user_Init 中调用一次) --- */
/* ADC1 + ADC2 均为注入组, 由 HRTIM_TRG2 触发, 注入完成中断驱动控制环 */
void Task_ADC_Init(void)
{
    /* ADC1, ADC2 均为单端模式, 校准用 SINGLE_ENDED */
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);

    /* HRTIM ADC 触发后缩放: 50kHz / 1 = 50kHz (每个PWM周期触发一次) */
    HAL_HRTIM_ADCPostScalerConfig(&hhrtim1, HRTIM_ADCTRIGGER_2, 0);

    /* 启动 ADC1 + ADC2 注入转换中断 */
    HAL_ADCEx_InjectedStart_IT(&hadc1);
    HAL_ADCEx_InjectedStart_IT(&hadc2);

    /* 启动 DAC1 通道1 (PA4 调试输出) */
    HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);

    /* 正弦相位积分器: 输入=频率Hz, 输出=归一化相位[0,1)
     * Ki = Ts/2 = 1/(2*CTRL_SAMPLE_RATE), 每拍步进 = 50*2*Ki */
    f32_Integral_Init(&Sine_Phase_Integrator, 1.0f / CTRL_SAMPLE_RATE, 1.0f);
    Sine_Phase_Integrator.x1 = 50.0f;  /* 预充, 避免首拍半步 */
}

/* --- 控制钩子: 由 user_Init 设置为当前激活的控制函数 --- */
void (*g_control_isr)(void) = NULL;
