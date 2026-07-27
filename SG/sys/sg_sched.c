/**
 * @file    sg_sched.c
 * @brief   简易时间片调度器
 * @note    原 timebase_scheduler.c
 */
#include "sg_sched.h"

typedef struct {
    uint16_t period_ms;
    uint16_t tick_cnt;
    void (*func)(void);
} SchedTask_t;

static SchedTask_t tasks[SCHED_MAX_TASKS];
static uint8_t     task_cnt = 0;
static uint32_t    last_tick = 0;

void sg_sched_init(void)
{
    task_cnt = 0;
    last_tick = HAL_GetTick();
}

void sg_sched_register(uint16_t period_ms, void (*func)(void))
{
    if (task_cnt >= SCHED_MAX_TASKS || func == NULL) return;
    tasks[task_cnt].period_ms = period_ms;
    tasks[task_cnt].tick_cnt  = 0;
    tasks[task_cnt].func      = func;
    task_cnt++;
}

void sg_sched_run(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t delta = now - last_tick;
    if (delta == 0) return;
    last_tick = now;

    for (uint8_t i = 0; i < task_cnt; i++) {
        tasks[i].tick_cnt += delta;
        if (tasks[i].tick_cnt >= tasks[i].period_ms) {
            tasks[i].tick_cnt -= tasks[i].period_ms;
            if (tasks[i].func) tasks[i].func();
        }
    }
}