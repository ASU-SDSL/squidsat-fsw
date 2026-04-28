#ifndef GSE_TASK_H
#define GSE_TASK_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <FreeRTOS.h>

#include "semphr.h"
#include "task.h"
#include "tusb.h"
#include "pico/error.h"

// ---- Logging (Strictly for Debug Mode) ----

// ---- Debug Mode Flag ----
extern SemaphoreHandle_t debug_mode_mutex;
extern volatile bool debug_mode;

// ---- GSE Task ----
#define GSE_TASK_STACK_SIZE 1024
#define GSE_TASK_PRIORITY   1
#define GSE_BUFFER_SIZE     256
#define GSE_TASK_DELAY_MS   10

void gse_init(); // init stdio & tud_task (USB in tinyUSB)
void vDebugTask(void *pvParameters);

// ---- Internal GSE Task ----

void debug_mode_init(void); // Init the Mutex
void debug_mode_set(bool value); // Sets True or False
bool get_debug_mode(); // Returns value Debug_Mode currently is

#endif // GSE_TASK_H