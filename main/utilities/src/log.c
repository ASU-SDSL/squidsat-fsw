#include "log.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static QueueHandle_t log_buffer;
SemaphoreHandle_t _log_mutex;

static const char* level_str[] = {"INFO", "ERROR", "WARNING", "MISSION CRITICAL"};


void log_init(void){
    if(log_buffer == NULL) 
        log_buffer = xQueueCreate(LOGGING_QUEUE_LENGTH, sizeof(LogEntry));
    if(_log_mutex == NULL)
        _log_mutex = xSemaphoreCreateMutex();
}


// Use heap 4 to allocate pointers to this
void log_to_queue(LogLevel priority, const char* fmt, ...){
    log_init();
    if(log_buffer == NULL || _log_mutex == NULL) return;

    LogEntry entry;
    entry.timestamp = xTaskGetTickCount();
    entry.level = priority;

    va_list args;
    va_start(args, fmt);
    vsnprintf(entry.data, MAX_PACKET_SIZE, fmt, args);
    va_end(args);

    if(xQueueSendToBack(log_buffer, &entry, 0) == pdFALSE){
        log_data("log queue full, dropping entry\n");
    }
}

void print_log(void){
    if(log_buffer == NULL || _log_mutex == NULL){
        log_warning("og not initialized\n");
        return;
    }

    if(xSemaphoreTake(_log_mutex, portMAX_DELAY)){
        LogEntry entry;

        printf("=== LOG DUMP (%lu entries) ===\n", (unsigned long)uxQueueMessagesWaiting(log_buffer));

        while(xQueueReceive(log_buffer, &entry, 0) == pdTRUE){
            printf("[%6lu] [%s] %s\n", 
                (unsigned long)entry.timestamp,
                level_str[entry.level],
                entry.data);
        }

        printf("=== END ===\n");

        xSemaphoreGive(_log_mutex);
    }
}