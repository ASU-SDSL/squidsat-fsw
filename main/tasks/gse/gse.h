#ifndef GSE_TASK_H
#define GSE_TASK_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "atomic.h"

#include "FreeRTOS.h"
#include "task.h"
#include "tusb.h"

// ---- Logging ----
#define log_info(fmt, ...) \
        printf("[LOG] " fmt "\n", ##__VA_ARGS__);
        

#define log_error(fmt, ...) \
        printf("[ERROR] " fmt "\n", ##__VA_ARGS__);

// ---- Debug Mode Flag ----
extern volatile bool debug_mode;

// ---- GSE Task ----
#define GSE_TASK_STACK_SIZE 1024
#define GSE_TASK_PRIORITY   1
#define GSE_BUFFER_SIZE     256
#define GSE_TASK_DELAY_MS   10

void gse_init();
void vDebugTask(void *pvParameters);

#endif // GSE_TASK_H