#include "task_display_1p.h"
#include "ssd1306.h"
#include "user.h"
#include "task_protect.h"
#include <stdio.h>

static char line[24];

void Task_Display_1P(void)
{
    if (!g_oled_ok) return;

    ssd1306_Fill(Black);

    /* 第 1 行：状态 + 母线电压 */
    ssd1306_SetCursor(0, 0);
    snprintf(line, sizeof(line), "%s Vin=%4.1fV",
             Run_Flag ? "RUN " : "STOP",
             U_line[1]);
    ssd1306_WriteString(line, White);

    /* 第 2 行：输出电压 RMS + 电流 RMS */
    ssd1306_SetCursor(0, 8);
    snprintf(line, sizeof(line), "Vo=%.1fV Io=%.2fA", g_uab_rms, g_irms);
    ssd1306_WriteString(line, White);

    /* 第 3 行：母线电压 vs 输出参考 (校准对比) */
    ssd1306_SetCursor(0, 16);
    snprintf(line, sizeof(line), "Vin=%.1fV Ref=%.1fV", U_line[1], g_vref_rms);
    ssd1306_WriteString(line, White);

    /* 第 4 行：频率与锁定 */
    ssd1306_SetCursor(0, 24);
    snprintf(line, sizeof(line), "f=%4.1fHz %s",
             g_freq_est, g_pll_locked ? "LCK" : "UNC");
    ssd1306_WriteString(line, White);

    /* 第 5 行：故障码 */
    ssd1306_SetCursor(0, 32);
    if (g_fault_code) {
        snprintf(line, sizeof(line), "FAULT=0x%04X", g_fault_code);
        ssd1306_WriteString(line, White);
    } else {
        ssd1306_WriteString("No Fault", White);
    }

    /* 第 6 行：调试量 + 保护屏蔽字 */
    ssd1306_SetCursor(0, 40);
    snprintf(line, sizeof(line), "E=%.2f Vc=%.1f m=%04X",
             g_dbg_err, g_dbg_vctrl, g_protect_mask);
    ssd1306_WriteString(line, White);

    /* 第 7 行：ISR 执行时间 [μs] */
    ssd1306_SetCursor(0, 48);
    snprintf(line, sizeof(line), "t_isr=%4.1fus", g_isr_khz);
    ssd1306_WriteString(line, White);

    /* 第 8 行：电压参考 */
    ssd1306_SetCursor(0, 56);
    snprintf(line, sizeof(line), "Vref=%.1fV", g_vref_rms);
    ssd1306_WriteString(line, White);

    ssd1306_UpdateScreen();
}
