#ifndef __USER_TASKS_H
#define __USER_TASKS_H

#include "main.h"

/* 注册周期任务到调度器；在 user_Init() 末尾被调用 */
void UserTasks_Init(void);

#endif /* __USER_TASKS_H */
