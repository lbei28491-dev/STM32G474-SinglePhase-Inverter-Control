/**
 * @file    sg_display.c
 * @brief   单相逆变 OLED 显示
 * @note    原 task_display_1p.c
 */
#include "sg_display.h"
#include "sg_global.h"
#include "sg_protect.h"
#include "sg_oled.h"
#include <stdio.h>

void sg_display_1p(void)
{
    if (!g_oled_ok) return;

    char line[22];
    ssd1306_Fill(Black);

    ssd1306_SetCursor(0, 0);
    snprintf(line, sizeof(line), "%s Vin=%4.1fV",
             g_run_flag ? "RUN " : "STOP", U_line[2]);
    ssd1306_WriteString(line, White);

    ssd1306_SetCursor(0, 12);
    snprintf(line, sizeof(line), "Vout=%4.1fV Iout=%4.2fA",
             g_vout_rms, g_irms);
    ssd1306_WriteString(line, White);

    ssd1306_SetCursor(0, 24);
    snprintf(line, sizeof(line), "Duty=%.3f ISR=%4.1fus",
             g_duty, g_isr_khz);
    ssd1306_WriteString(line, White);

    ssd1306_SetCursor(0, 36);
    if (g_fault_code) {
        snprintf(line, sizeof(line), "FAULT=0x%04X", g_fault_code);
        ssd1306_WriteString(line, White);
    } else {
        ssd1306_WriteString("No Fault", White);
    }

    ssd1306_SetCursor(0, 48);
    snprintf(line, sizeof(line), "Vref=%.1fV", g_vref_rms);
    ssd1306_WriteString(line, White);

    ssd1306_UpdateScreen();
}