#include "user_tasks.h"
#include "timebase_scheduler.h"
#include "user.h"
#include "task_protect.h"
#include "task_control_1p.h"
#include "task_preproc.h"
#include "task_display_1p.h"

/* 控制周期保护检查包装：尽量快地响应异常 */
static void Task_Protect_Wrap(void)
{
    /* ADC2 注入原始码值 -> 电压 (3.3V/4096) */
    float u_ch1 = (float)g_adc2_r1 * (3.3f / 4096.0f);
    float u_ch2 = (float)g_adc2_r2 * (3.3f / 4096.0f);
    Task_Protect_Check(U_line[1], I_line[0], u_ch1, u_ch2);
}

void UserTasks_Init(void)
{
    Sched_Init();

    /* ADC1 注入 ISR 中执行顺序: 预处理 -> 控制 */
    g_adc_preproc  = Task_Preproc;       /* RMS / 瞬时值填充 */
    g_control_isr  = Task_Control_1P;    /* 离网逆变控制环 */

    /* 周期任务: 50Hz 按键 / 10Hz OLED / 控制周期级保护检查 */
    Sched_Register(20,   Task_Button_Scan);
    if (g_oled_ok)
        Sched_Register(100, Task_Display_1P);
    Sched_Register(1, Task_Protect_Wrap);
}
