#ifndef __SSD1306_H
#define __SSD1306_H

#include "main.h"
#include "hardware_def.h"

/* OLED 颜色（位图表示） */
typedef enum {
    Black  = 0x00,
    White  = 0xFF
} SSD1306_Color;

/* 屏幕分辨率 */
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64
#define SSD1306_BUF_SIZE        (SSD1306_WIDTH * SSD1306_HEIGHT / 8)

/* 字体尺寸（5x7） */
#define SSD1306_FONT_WIDTH      5
#define SSD1306_FONT_HEIGHT     7

void ssd1306_Init(void);
void ssd1306_Fill(SSD1306_Color color);
void ssd1306_UpdateScreen(void);

void ssd1306_SetCursor(uint8_t x, uint8_t y);
char ssd1306_WriteChar(char ch, SSD1306_Color color);
void ssd1306_WriteString(const char *str, SSD1306_Color color);

#endif /* __SSD1306_H */
