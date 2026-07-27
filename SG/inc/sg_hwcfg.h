/**
 * @file    sg_hwcfg.h
 * @brief   SG 硬件配置 — 单相逆变（离网/并网）
 * @note    合并原 hardware_def.h + hardware_def_1p.h
 *
 * ADC1 注入: PA2(输入电流 Iin), PA3(输入电压 Vin)
 * ADC2 注入: PA0(输出电流 Iout), PA1(输出电压 Vout)
 * HRTIM1:   PA8-PA9(TA1-TA2), PA10-PA11(TB1-TB2) — H桥双极性SPWM
 * DAC1:     PA4 调试输出
 * I2C4:     PC6=SCL, PC7=SDA (OLED SSD1306)
 */
#ifndef SG_HWCFG_H
#define SG_HWCFG_H

#include "main.h"

/* ======================== 控制器输出钳位 ======================== */
#define PR_CTRL_CLAMP           1.0f
#define PI_OUTPUT_CLAMP         12.0f

/* ======================== ADC 标定系数 ========================
 * 硬件链路:
 *   Vin(PA3): 1:34 电阻分压 → ADC 输入 → Vin = ADC×3.3/4096×30.4 (实测校准)
 *   Vout(PA1): 1k÷68k 分压(≈1:69) → 偏置1.65V → ADC≈2048
 *   Iin(PA2), Iout(PA0): R010=10mΩ + 运放G≈11 + 偏置1.65V
 */
#define ADC_V_PER_COUNT         0.00080586f         /* 3.3V / 4096 */
#define VIN_DIV_RATIO           30.4f               /* 实测校准 */
#define CAL_VIN_V_PER_COUNT     (ADC_V_PER_COUNT * VIN_DIV_RATIO)  /* ≈0.0245 */
#define VOUT_DIV_RATIO          69.0f               /* Vout: (1k+68k)/1k */
#define VOUT_CAL_V_PER_COUNT    (ADC_V_PER_COUNT * VOUT_DIV_RATIO) /* ≈0.0556 */
#define CURRENT_SHUNT_OHM       0.01f               /* R010 = 10mΩ */
#define CURRENT_GAIN            11.0f               /* 运放 G = 1+100k/10k = 11 */
#define CURRENT_CAL_A_PER_COUNT (ADC_V_PER_COUNT / (CURRENT_GAIN * CURRENT_SHUNT_OHM)) /* ≈0.00733 */

/* 全局变量初始化用 */
#define VOLTAGE_CONST           CAL_VIN_V_PER_COUNT
#define CURRENT_CONST           CURRENT_CAL_A_PER_COUNT

/* ======================== 离网参数 ======================== */
#define OFFGRID_UREF            32.0f   /* 输出电压参考 [V RMS] */
#define OFFGRID_IREF_MAX         5.0f   /* 最大输出电流 [A RMS] */
#define OFFGRID_UDC_MIN         50.0f   /* 最小启动母线电压 [V] */
#define I_MAG_DEFAULT            1.0f   /* 初始电流幅值 [A] */
#define I_MAG_MAX               10.0f   /* PI 输出上限 [A] */
#define CTRL_SAMPLE_RATE     50000.0f   /* 控制环 50kHz */

/* ======================== 保护位掩码 ======================== */
#define PROT_NONE               0x0000u
#define PROT_IOUT_OC            0x0001u  /* 输出过流 */
#define PROT_IL1_OC             0x0002u  /* 输入过流 */
#define PROT_VOUT_OV            0x0004u  /* 输出过压 */
#define PROT_UDC_OV             0x0008u  /* 母线过压 */
#define PROT_UDC_UV             0x0010u  /* 母线欠压 */
#define PROT_OT                 0x0020u  /* 过温 */
#define PROT_HW_FAULT           0x8000u  /* 硬件故障 */

/* ======================== 用户 GPIO 引脚映射 ========================
 *   PB0  - Green  (运行心跳LED)
 *   PB11 - Red    (故障/继电器指示LED)
 *   PC12 - key_relay (主回路继电器)
 *   按键 (CubeMX: S0-S3, 不按=高电平, 按下=低电平):
 *     S0 (PC11) - user   启停
 *     S1 (PC10) - KEY1   减小
 *     S2 (PC9)  - KEY2   增大
 *     S3 (PC8)  - KEY3   故障复位
 */
#define Green_GPIO_Port         GPIOB
#define Green_Pin               GPIO_PIN_0
#define Red_GPIO_Port           GPIOB
#define Red_Pin                 GPIO_PIN_11
#define user_GPIO_Port          GPIOC
#define user_Pin                GPIO_PIN_11    /* S0: 启停 */
#define KEY1_GPIO_Port          GPIOC
#define KEY1_Pin                GPIO_PIN_10    /* S1: 减小 */
#define KEY2_GPIO_Port          GPIOC
#define KEY2_Pin                GPIO_PIN_9     /* S2: 增大 */
#define KEY3_GPIO_Port          GPIOC
#define KEY3_Pin                GPIO_PIN_8     /* S3: 故障复位 */
#define KEY_RUN_GPIO_Port       GPIOC
#define KEY_RUN_Pin             GPIO_PIN_5     /* PC5: 运行控制按键 */
#define key_relay_GPIO_Port     GPIOC
#define key_relay_Pin           GPIO_PIN_12

/* ======================== OLED I2C 端口 ======================== */
#define SSD1306_I2C_PORT        hi2c4
#define SSD1306_I2C_ADDR        (0x3C << 1)   /* 0x78 */

#endif /* SG_HWCFG_H */