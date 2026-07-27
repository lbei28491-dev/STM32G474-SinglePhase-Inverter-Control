#ifndef __TIMEBASE_SCHEDULER_H
#define __TIMEBASE_SCHEDULER_H

#include "main.h"

/* 最大周期任务数 */
#define SCHED_MAX_TASKS   8

/* 任务函数原型 */
typedef void (*Sched_TaskFn)(void);

/* 注册一个周期任务
 * period_ms: 调用周期 [ms]
 * fn: 任务函数
 * 返回：0=成功, -1=满
 */
int  Sched_Register(uint32_t period_ms, Sched_TaskFn fn);

/* 在主循环中调用：扫描到期任务并执行 */
void Sched_Run(void);

/* 初始化 */
void Sched_Init(void);

#endif /* __TIMEBASE_SCHEDULER_H */
