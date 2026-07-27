/**
 * @file    sg_sched.h
 * @brief   简易时间片调度器
 * @note    原 timebase_scheduler.h
 */
#ifndef SG_SCHED_H
#define SG_SCHED_H

#include "main.h"

/* 最大注册任务数 */
#define SCHED_MAX_TASKS  8

void sg_sched_init(void);
void sg_sched_register(uint16_t period_ms, void (*func)(void));
void sg_sched_run(void);

#endif /* SG_SCHED_H */