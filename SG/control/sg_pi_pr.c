/**
 * @file    sg_pi_pr.c
 * @brief   PI/PR 控制器 + Clarke/Park 变换实现
 * @note    原 pi_pr_ctrl.c
 */
#include "sg_pi_pr.h"
#include "arm_math.h"

/* ======================== PI 控制器 ======================== */
void f32_PI_Init(PI_TypeDef *p, float Ts, float Kp, float Ki, float TH, float TL)
{
    p->Ts = Ts;
    p->Kp = Kp;
    p->Ki = Ki;
    p->TH = TH;
    p->TL = TL;
    p->integral = 0.0f;
    p->output   = 0.0f;
    p->err_prev = 0.0f;
}

float f32_PI_Calculate(PI_TypeDef *p, float ref, float fdb)
{
    float err = ref - fdb;
    p->integral += p->Ki * p->Ts * err;

    /* 积分抗饱和 */
    float tmp = p->Kp * err + p->integral;
    if (tmp > p->TH) {
        tmp = p->TH;
        p->integral -= p->Ki * p->Ts * err;
    } else if (tmp < p->TL) {
        tmp = p->TL;
        p->integral -= p->Ki * p->Ts * err;
    }
    p->output = tmp;
    p->err_prev = err;
    return p->output;
}

/* ======================== PR 控制器 ======================== */
void f32_PR_Init(PR_TypeDef *p, float Ts, float Kp, float Kr, float w0, float wc)
{
    p->Ts = Ts;
    p->Kp = Kp;
    p->Kr = Kr;
    p->w0 = w0;
    p->wc = wc;
    p->x1 = p->x2 = 0.0f;
    p->y1 = p->y2 = 0.0f;
    p->output = 0.0f;
}

float f32_PR_Calculate(PR_TypeDef *p, float err)
{
    /* 准 PR 控制器: G(s) = Kp + Kr·2·wc·s / (s² + 2·wc·s + w0²)
     * 双线性变换法 (Tustin) 离散化
     */
    float Ts = p->Ts;
    float wc = p->wc;
    float w0 = p->w0;
    float Kr = p->Kr;

    float a0 = 4.0f + 4.0f * wc * Ts + w0 * w0 * Ts * Ts;
    float a1 = 2.0f * w0 * w0 * Ts * Ts - 8.0f;
    float a2 = 4.0f - 4.0f * wc * Ts + w0 * w0 * Ts * Ts;

    float b0 = (4.0f * Kr * wc * Ts) / a0;
    float b1 = 0.0f;
    float b2 = (-4.0f * Kr * wc * Ts) / a0;

    float y = b0 * err + b1 * p->x1 + b2 * p->x2
              - a1 / a0 * p->y1 - a2 / a0 * p->y2;

    p->x2 = p->x1;
    p->x1 = err;
    p->y2 = p->y1;
    p->y1 = y;

    p->output = p->Kp * err + y;
    return p->output;
}

/* ======================== 积分器 ======================== */
void f32_Integral_Init(Integral_TypeDef *p, float Ts, float K)
{
    p->Ts = Ts;
    p->K  = K;
    p->x1 = 0.0f;
    p->y1 = 0.0f;
}

float f32_Integral_Calculate(Integral_TypeDef *p, float x1)
{
    p->y1 += p->K * p->Ts * x1;
    return p->y1;
}

/* ======================== Clarke 变换 (ABC → αβ) ========================
 * 等幅值变换:
 *   α = 2/3 * (ia - 0.5*ib - 0.5*ic)
 *   β = 2/3 * (√3/2 * ib - √3/2 * ic)
 */
void Clarke_Transform(float ia, float ib, float ic, float *alpha, float *beta)
{
    *alpha = 0.6666667f * (ia - 0.5f * ib - 0.5f * ic);
    *beta  = 0.6666667f * (0.8660254f * ib - 0.8660254f * ic);
}

/* ======================== Park 变换 (αβ → DQ) ======================== */
void Park_Transform(float alpha, float beta, float sin_th, float cos_th,
                    float *d, float *q)
{
    *d =  alpha * cos_th + beta * sin_th;
    *q = -alpha * sin_th + beta * cos_th;
}

/* ======================== 逆 Park 变换 (DQ → αβ) ======================== */
void Inv_Park_Transform(float d, float q, float sin_th, float cos_th,
                        float *alpha, float *beta)
{
    *alpha = d * cos_th - q * sin_th;
    *beta  = d * sin_th + q * cos_th;
}

/* ======================== ABC → DQ 一步到位 ======================== */
void ABC_to_DQ(float *dq, float *abc, float wt)
{
    float alpha, beta;
    float sin_th = arm_sin_f32(2.0f * (float)PI * wt);
    float cos_th = arm_cos_f32(2.0f * (float)PI * wt);
    Clarke_Transform(abc[0], abc[1], abc[2], &alpha, &beta);
    Park_Transform(alpha, beta, sin_th, cos_th, &dq[0], &dq[1]);
}

/* ======================== DQ → ABC 一步到位 ======================== */
void DQ_to_ABC(float *dq, float *abc, float wt)
{
    float alpha, beta;
    float sin_th = arm_sin_f32(2.0f * (float)PI * wt);
    float cos_th = arm_cos_f32(2.0f * (float)PI * wt);
    Inv_Park_Transform(dq[0], dq[1], sin_th, cos_th, &alpha, &beta);
    /* 逆 Clarke: abc = α (a=α, b=-0.5*α+√3/2*β, c=-0.5*α-√3/2*β) */
    abc[0] = alpha;
    abc[1] = -0.5f * alpha + 0.8660254f * beta;
    abc[2] = -0.5f * alpha - 0.8660254f * beta;
}