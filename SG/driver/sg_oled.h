/**
 * @file    sg_oled.h
 * @brief   SSD1306 OLED 驱动封装
 * @note    原 ssd1306.h, 添加 sg_oled 前缀免冲突
 */
#ifndef SG_OLED_H
#define SG_OLED_H

#include "main.h"
#include "sg_hwcfg.h"

/* 透传 ssd1306 库的接口 */
void ssd1306_Init(void);
void ssd1306_Fill(uint8_t color);
void ssd1306_SetCursor(uint8_t x, uint8_t y);
void ssd1306_WriteString(const char *str, uint8_t color);
void ssd1306_UpdateScreen(void);

/* 颜色定义 — 与 ssd1306.h 的 enum 值保持一致 */
#ifndef __SSD1306_H
#define Black   0x00
#define White   0xFF
#endif

#endif /* SG_OLED_H */