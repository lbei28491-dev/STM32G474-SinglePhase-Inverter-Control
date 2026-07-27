/**
 * @file    sg_user.h
 * @brief   用户初始化 + 按键扫描
 * @note    合并原 user.c + user.h 中的函数
 */
#ifndef SG_USER_H
#define SG_USER_H

#include "main.h"

void sg_user_init(void);
void sg_button_scan(void);

#endif /* SG_USER_H */