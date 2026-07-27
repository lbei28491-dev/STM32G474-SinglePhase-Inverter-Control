#include "user.h"
#include "user_tasks.h"
#include "task_adc.h"
#include "task_pwm_1p.h"
#include "task_protect.h"
#include "timebase_scheduler.h"
#include "hardware_def_1p.h"
#include "ssd1306.h"

uint8_t Run_Flag = 0;
PI_TypeDef Voltage_PI_Loop;
PR_TypeDef Current_PR_Loop_alpha;
Integral_TypeDef Sine_Phase_Integrator;
float U_line[4] = {0}, I_line[3] = {0};
float g_duty_a = 0.5f, g_duty_b = 0.5f, g_duty_c = 0.5f;
float g_uab_rms = 0.0f, g_ubc_rms = 0.0f, I_mag = 1.0f, g_irms = 0.0f;
float U_coefficient, Vout_coefficient, current_const;
float Iref_alpha = 0.0f, Iref_beta = 0.0f;
volatile int32_t g_il1, g_il2, g_il3;
volatile uint8_t g_adc_data_ready;
float g_dbg_err, g_dbg_vctrl, g_isr_khz;
float g_vref_rms = OFFGRID_UREF;   /* 输出电压参考, 按键可调 */
float g_pfc_phase_deg = 0.0f;
uint8_t g_oled_ok = 0;
float g_grid_phi_deg  = 0.0f;
float   g_wt         = 0.0f;
uint8_t g_pll_locked = 0;
float   g_freq_est   = 50.0f;
volatile float g_sin_wt;

/* ADC1 + ADC2 注入原始码值 (由 main.c 回调填充) */
volatile uint32_t g_adc1_r1;   /* ADC1 Rank1 = PA2 输入电流 */
volatile uint32_t g_adc1_r2;   /* ADC1 Rank2 = PA3 输入电压 */
volatile uint32_t g_adc2_r1;   /* ADC2 Rank1 = PA0 输出电流 */
volatile uint32_t g_adc2_r2;   /* ADC2 Rank2 = PA1 输出电压 */
volatile uint8_t  g_adc1_done;
volatile uint8_t  g_adc2_done;

void (*g_adc_preproc)(void) = NULL;
void (*g_display_fn)(void)  = NULL;
void (*g_vofa_fn)(void)     = NULL;

void user_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    U_coefficient   = CAL_VIN_V_PER_COUNT;     /* Vin: 实测校准 30.4倍 */
    Vout_coefficient = VOUT_CAL_V_PER_COUNT;   /* Vout: 0.0556 V/count (1:69分压) */
    current_const    = CURRENT_CAL_A_PER_COUNT; /* 电流: 0.00733 A/count */

    /* Re-do DLL calibration with longer timeout (bare board may be slow) */
    HAL_HRTIM_DLLCalibrationStart(&hhrtim1, HRTIM_CALIBRATIONRATE_3);
    if (HAL_HRTIM_PollForDLLCalibration(&hhrtim1, 500) != HAL_OK) {
        /* DLL failed — blink Green fast */
        while (1) {
            HAL_GPIO_TogglePin(Green_GPIO_Port, Green_Pin);
            for (volatile uint32_t i = 0; i < 200000; i++);
        }
    }

    Task_ADC_Init();  /* ADC 校准, 注入启动, DAC, 积分器 */

    f32_PI_Init(&Voltage_PI_Loop, 0.02f, 0.25f, 12.0f, 10, 0);

    /* --- OLED init (I2C4, 非关键, 未连接则跳过) --- */
    if (HAL_I2C_IsDeviceReady(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 2, 10) == HAL_OK) {
        ssd1306_Init();
        ssd1306_Fill(Black);
        ssd1306_UpdateScreen();
        g_oled_ok = 1;
    }

    HAL_GPIO_WritePin(Green_GPIO_Port, Green_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Red_GPIO_Port,   Red_Pin,   GPIO_PIN_RESET);

    HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_MASTER);
    HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_TIMER_A);  /* ADC trigger source */
    HRTIM1->sMasterRegs.MDIER |= HRTIM_MDIER_MCMP1IE;  /* re-apply after CubeMX clobber */

    UserTasks_Init();
}

void Task_Button_Scan(void)
{
    /* --- 心跳 --- */
    HAL_GPIO_TogglePin(Green_GPIO_Port, Green_Pin);  /* 50Hz */

    /* --- S0 (PC11): 启停 --- */
    static uint8_t last_Run_Flag = 0xFF, last_user = 1;
    if (last_Run_Flag == 0xFF) last_user = HAL_GPIO_ReadPin(user_GPIO_Port, user_Pin);
    uint8_t user = HAL_GPIO_ReadPin(user_GPIO_Port, user_Pin);
    if (last_user == GPIO_PIN_SET && user == GPIO_PIN_RESET) {
        if (!Run_Flag) {
            g_fault_code = 0;   /* 清除故障 */
            Run_Flag = 1;       /* 启动 */
        } else {
            Run_Flag = 0;       /* 停止 */
        }
    }
    last_user = user;

    if (Run_Flag != last_Run_Flag) {
        last_Run_Flag = Run_Flag;
        if (!Run_Flag) {
            HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_RESET);
        }
    }

    /* --- S1 (PC10) KEY1: 电压参考 -
     *    S2 (PC9)  KEY2: 电压参考 +
     *    长按切换步进 0.5V / 0.1V --- */
    static uint16_t k1_hold = 0, k2_hold = 0;
    static uint8_t  k1_last = 1, k2_last = 1;
    static float    v_step = 0.5f;
    uint8_t k1 = HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin);
    uint8_t k2 = HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin);

    /* KEY1: 减小电压参考 */
    if (k1_last == 1 && k1 == 0) k1_hold = 0;
    else if (k1 == 0) k1_hold++;
    else if (k1_last == 0 && k1 == 1) {
        if (k1_hold < 50) {
            g_vref_rms -= v_step;
            if (g_vref_rms < 5.0f) g_vref_rms = 5.0f;
        } else v_step = (v_step > 0.2f) ? 0.1f : 0.5f;
    }
    k1_last = k1;

    /* KEY2: 增大电压参考 */
    if (k2_last == 1 && k2 == 0) k2_hold = 0;
    else if (k2 == 0) k2_hold++;
    else if (k2_last == 0 && k2 == 1) {
        if (k2_hold < 50) {
            g_vref_rms += v_step;
            if (g_vref_rms > 40.0f) g_vref_rms = 40.0f;
        } else v_step = (v_step > 0.2f) ? 0.1f : 0.5f;
    }
    k2_last = k2;

    /* --- S3 (PC8) KEY3: 故障复位 (仅在停止状态有效) --- */
    static uint8_t k3_last = 1;
    uint8_t k3 = HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin);
    if (k3_last == GPIO_PIN_SET && k3 == GPIO_PIN_RESET) {
        if (!Run_Flag && g_fault_code) {
            Task_Protect_Clear();
        }
    }
    k3_last = k3;
}
