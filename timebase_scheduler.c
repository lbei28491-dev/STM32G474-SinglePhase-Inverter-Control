#include "timebase_scheduler.h"

typedef struct {
    Sched_TaskFn fn;
    uint32_t     period_ms;
    uint32_t     last_ms;
} Sched_Task;

static Sched_Task sched_tasks[SCHED_MAX_TASKS];
static uint8_t     sched_count = 0;

void Sched_Init(void)
{
    sched_count = 0;
}

int Sched_Register(uint32_t period_ms, Sched_TaskFn fn)
{
    if (sched_count >= SCHED_MAX_TASKS) return -1;
    if (fn == NULL) return -1;
    if (period_ms == 0) return -1;

    sched_tasks[sched_count].fn        = fn;
    sched_tasks[sched_count].period_ms = period_ms;
    sched_tasks[sched_count].last_ms   = HAL_GetTick();
    sched_count++;
    return 0;
}

void Sched_Run(void)
{
    uint32_t now = HAL_GetTick();
    for (uint8_t i = 0; i < sched_count; i++) {
        uint32_t elapsed = now - sched_tasks[i].last_ms;
        if (elapsed >= sched_tasks[i].period_ms) {
            sched_tasks[i].last_ms = now;
            sched_tasks[i].fn();
        }
    }
}
