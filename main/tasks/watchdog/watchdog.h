#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t xWatchdogTaskHandler;

void watchdog_init(); 

void watchdog_freeze();

void watchdog_task(void *pvParameters);

bool watchdog_check_browned_out(); 

#endif // WATCHDOG_H