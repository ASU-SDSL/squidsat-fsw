#ifndef GSE_TASK_H
#define GSE_TASK_H
#include <FreeRTOS.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <FreeRTOS.h>

#include "semphr.h"
#include "task.h"
#include "tusb.h"
#include "pico/error.h"



// ---- GSE Task ----
#define GSE_TASK_STACK_SIZE 1024
#define GSE_TASK_PRIORITY   1
#define GSE_BUFFER_SIZE     256
#define GSE_TASK_DELAY_MS   10

void gse_init(); // init stdio & tud_task (USB in tinyUSB)
void vDebugTask(void *pvParameters);
void debug_mode_init(void);
bool get_debug_mode(void);

#endif // GSE_TASK_H