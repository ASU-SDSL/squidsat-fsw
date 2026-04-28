#include "log.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>


static uint8_t log_head = 0;
static uint8_t log_count = 0;

static QueueHandle_t log_buffer;
static SemaphoreHandle_t log_mutex;

const char* level_str[] = {"INFO", "ERROR", "WARNING", "MISSION CRITICAL"};


void log_init(void){
    if(log_buffer == NULL) 
        log_buffer = xQueueCreate(LOGGING_QUEUE_LENGTH, sizeof(LogEntry));
    if(log_mutex == NULL)
        log_mutex = xSemaphoreCreateMutex();
}


// Use heap 4 to allocate pointers to this
void log_to_queue(LogLevel priority, const char* fmt, ...){
    log_init();
    if(log_buffer == NULL || log_mutex == NULL) return;

    LogEntry entry;
    entry.timestamp = xTaskGetTickCount();
    entry.level = priority;

    va_list args;
    va_start(args, fmt);
    vsnprintf(entry.data, MAX_PACKET_SIZE, fmt, args);
    va_end(args);

    xSemaphoreTake(log_mutex, portMAX_DELAY);
    if(xQueueSendToBack(log_buffer, &entry, 0) != pdTRUE){
        log_warning("log queue full, dropping entry\n");
    }
    xSemaphoreGive(log_mutex);
}

void print_log(void){
    if(log_buffer == NULL || log_mutex == NULL){
        log_warning("log not initialized\n");
        return;
    }

    LogEntry entry;

    xSemaphoreTake(log_mutex, portMAX_DELAY);

    printf("=== LOG DUMP (%lu entries) ===\n", (unsigned long)uxQueueMessagesWaiting(log_buffer));

    while(xQueueReceive(log_buffer, &entry, 0) == pdTRUE){
        printf("[%6lu] [%s] %s\n", 
               (unsigned long)entry.timestamp,
               level_str[entry.level],
               entry.data);
    }

    printf("=== END ===\n");

    xSemaphoreGive(log_mutex);
}