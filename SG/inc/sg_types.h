/**
 * @file    sg_types.h
 * @brief   SG 通用类型定义 — PI, PR, 积分器
 * @note    从原 pi_pr_ctrl.h 分离出类型定义, 供所有模块引用
 */
#ifndef SG_TYPES_H
#define SG_TYPES_H

#include "arm_math.h"

/* ======================== PI 控制器结构体 ======================== */
typedef struct {
    float Ts;           /* 采样周期 [s] */
    float Kp;           /* 比例系数 */
    float Ki;           /* 积分系数 */
    float TH;           /* 输出上限 */
    float TL;           /* 输出下限 */
    float integral;     /* 积分累积值 */
    float output;       /* 当前输出 */
    float err_prev;     /* 前一次误差 (用于部分算法) */
} PI_TypeDef;

/* ======================== PR 控制器结构体 ======================== */
typedef struct {
    float Ts;           /* 采样周期 [s] */
    float Kp;           /* 比例系数 */
    float Kr;           /* 谐振增益 */
    float w0;           /* 谐振频率 [rad/s] */
    float wc;           /* 截止频率 [rad/s] */
    float x1, x2;       /* 状态变量 */
    float y1, y2;       /* 输出延迟 */
    float output;       /* 当前输出 */
} PR_TypeDef;

/* ======================== 积分器结构体 ======================== */
typedef struct {
    float Ts;           /* 采样周期 [s] */
    float K;            /* 增益 */
    float x1;           /* 输入 (当前) */
    float y1;           /* 输出 (当前) */
} Integral_TypeDef;

/* ======================== 函数声明 ======================== */
void f32_PI_Init(PI_TypeDef *p, float Ts, float Kp, float Ki, float TH, float TL);
float f32_PI_Calculate(PI_TypeDef *p, float ref, float fdb);
void f32_PR_Init(PR_TypeDef *p, float Ts, float Kp, float Kr, float w0, float wc);
float f32_PR_Calculate(PR_TypeDef *p, float err);
void f32_Integral_Init(Integral_TypeDef *p, float Ts, float K);
float f32_Integral_Calculate(Integral_TypeDef *p, float x1);

/* ======================== Clarke / Park 变换 ======================== */
void Clarke_Transform(float ia, float ib, float ic, float *alpha, float *beta);
void Park_Transform(float alpha, float beta, float sin_th, float cos_th,
                    float *d, float *q);
void Inv_Park_Transform(float d, float q, float sin_th, float cos_th,
                        float *alpha, float *beta);
void ABC_to_DQ(float *dq, float *abc, float wt);
void DQ_to_ABC(float *dq, float *abc, float wt);

#endif /* SG_TYPES_H */