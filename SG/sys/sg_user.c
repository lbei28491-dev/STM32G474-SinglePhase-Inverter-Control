/**
 * @file    sg_user.c
 * @brief   用户初始化 + 按键扫描 + 全局变量定义
 * @note    合并原 user.c + user.h, 原 OT/user.c
 */
#include "sg_user.h"
#include "sg_global.h"
#include "sg_tasks.h"
#include "sg_adc.h"
#include "sg_pwm.h"
#include "sg_protect.h"
#include "sg_sched.h"
#include "sg_hwcfg.h"
#include "sg_oled.h"

/* ======================== 全局变量定义 ======================== */
volatile uint8_t g_run_flag = 0;
float    U_line[3]  = {0};
float    I_line[2]  = {0};
float    g_duty     = 0.5f;
float    g_vout_rms = 0.0f, g_iin = 0.0f, g_irms = 0.0f;
float    U_coefficient   = CAL_VIN_V_PER_COUNT;
float    Vout_coefficient = VOUT_CAL_V_PER_COUNT;
float    current_const   = CURRENT_CAL_A_PER_COUNT;
float    g_dbg_err, g_dbg_vctrl, g_isr_khz;
float    g_vref_rms = OFFGRID_UREF;
uint8_t  g_oled_ok = 0;

volatile uint32_t g_adc1_r1, g_adc1_r2;
volatile uint32_t g_adc2_r1, g_adc2_r2;
volatile uint8_t  g_adc1_done, g_adc2_done;

void (*g_control_isr)(void)  = NULL;
void (*g_adc_preproc)(void)  = NULL;
void (*g_display_fn)(void)   = NULL;
void (*g_vofa_fn)(void)      = NULL;

volatile uint8_t g_adc_data_ready = 0;

/* ======================== 用户初始化 ======================== */
void sg_user_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    /* HRTIM DLL 校准 */
    HAL_HRTIM_DLLCalibrationStart(&hhrtim1, HRTIM_CALIBRATIONRATE_3);
    if (HAL_HRTIM_PollForDLLCalibration(&hhrtim1, 500) != HAL_OK) {
        while (1) {
            HAL_GPIO_TogglePin(Green_GPIO_Port, Green_Pin);
            for (volatile uint32_t i = 0; i < 200000; i++);
        }
    }

    sg_adc_init();   /* ADC 校准 + 启动 */
    sg_sched_init(); /* 调度器初始化 */

    /* OLED init (I2C4, 非关键) */
    if (HAL_I2C_IsDeviceReady(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 2, 10) == HAL_OK) {
        ssd1306_Init();
        ssd1306_Fill(Black);
        ssd1306_UpdateScreen();
        g_oled_ok = 1;
    }

    HAL_GPIO_WritePin(Green_GPIO_Port, Green_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Red_GPIO_Port,   Red_Pin,   GPIO_PIN_RESET);

    /*
     * 同一次写操作启动 Timer A/B，使两个桥臂保持相位同步。
     * 此处只启动计数器，不使能功率输出；Timer A 同时提供 ADC 触发。
     */
    if (HAL_HRTIM_WaveformCounterStart(&hhrtim1,
            HRTIM_TIMERID_TIMER_A | HRTIM_TIMERID_TIMER_B) != HAL_OK) {
        Error_Handler();
    }

    sg_tasks_register();
}

/* ======================== 按键扫描 ======================== */
void sg_button_scan(void)
{
    /* 心跳 */
    HAL_GPIO_TogglePin(Green_GPIO_Port, Green_Pin);

    /* S0 (PC11): 启停 */
    static uint8_t last_user = 1;
    uint8_t user = HAL_GPIO_ReadPin(user_GPIO_Port, user_Pin);
    if (last_user == GPIO_PIN_SET && user == GPIO_PIN_RESET) {
        if (!g_run_flag) {
            g_fault_code = 0;
            g_run_flag = 1;
        } else {
            g_run_flag = 0;
        }
    }
    last_user = user;

    if (!g_run_flag) {
        HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_RESET);
    }

    /* S1 (PC10) KEY1: Vref - / S2 (PC9) KEY2: Vref + */
    static uint16_t k1_hold = 0, k2_hold = 0;
    static uint8_t  k1_last = 1, k2_last = 1;
    static float    v_step = 0.5f;
    uint8_t k1 = HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin);
    uint8_t k2 = HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin);

    if (k1_last == 1 && k1 == 0) k1_hold = 0;
    else if (k1 == 0) k1_hold++;
    else if (k1_last == 0 && k1 == 1) {
        if (k1_hold < 50) { g_vref_rms -= v_step; if (g_vref_rms < 5.0f) g_vref_rms = 5.0f; }
        else v_step = (v_step > 0.2f) ? 0.1f : 0.5f;
    }
    k1_last = k1;

    if (k2_last == 1 && k2 == 0) k2_hold = 0;
    else if (k2 == 0) k2_hold++;
    else if (k2_last == 0 && k2 == 1) {
        if (k2_hold < 50) { g_vref_rms += v_step; if (g_vref_rms > 40.0f) g_vref_rms = 40.0f; }
        else v_step = (v_step > 0.2f) ? 0.1f : 0.5f;
    }
    k2_last = k2;

    /* PC5 KEY_RUN: 运行控制 (独立于 S0，功能相同) */
    static uint8_t krun_last = 1;
    uint8_t krun = HAL_GPIO_ReadPin(KEY_RUN_GPIO_Port, KEY_RUN_Pin);
    if (krun_last == GPIO_PIN_SET && krun == GPIO_PIN_RESET) {
        if (!g_run_flag) {
            g_fault_code = 0;
            g_run_flag = 1;
        } else {
            g_run_flag = 0;
        }
    }
    krun_last = krun;

    /* S3 (PC8) KEY3: 故障复位 */
    static uint8_t k3_last = 1;
    uint8_t k3 = HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin);
    if (k3_last == GPIO_PIN_SET && k3 == GPIO_PIN_RESET) {
        if (!g_run_flag && g_fault_code) sg_protect_clear();
    }
    k3_last = k3;
}
