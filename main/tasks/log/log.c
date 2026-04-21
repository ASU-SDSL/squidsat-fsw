#include "log.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>


static uint8_t  log_head = 0;
static uint8_t  log_count = 0;

static QueueHandle_t log_buffer;


void log_init(void){
    if(log_buffer == NULL) log_buffer = xQueueCreate(MAX_LOG_LENGTH, sizeof(LogEntry));
}

void log_data(LogLevel priority, const char* fmt, ...){
    log_init();

    LogEntry entry;
    entry.timestamp = xTaskGetTickCount();
    entry.level = priority;

    va_list args;
    va_start(args, fmt);
    vsnprintf(entry.data, MAX_PACKET_SIZE, fmt, args);
    va_end(args);

    xQueueSend(log_buffer, &entry, 0);
}

void print_log(void){
    LogEntry entry;
    
    printf("=== LOG DUMP ===\n");
    
    const char* level_str[] = { "INFO", "ERROR", "WARN"};
    
    if (log_buffer == NULL) {
        log_error("Buffer is empty!");
        return;
    }

    // keep pulling entries until queue is empty
    while (xQueueReceive(log_buffer, &entry, 0) == pdTRUE) {
        printf("[%6lu] [%s] %s\n", 
               (unsigned long) entry.timestamp,
               level_str[entry.level],
               entry.data);
    }
    
    printf("=== END ===\n");
}