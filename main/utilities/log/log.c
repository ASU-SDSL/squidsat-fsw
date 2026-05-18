#include "log.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>


static const char* level_str[] = {"INFO", "ERROR", "WARNING", "MISSION CRITICAL"};


static SemaphoreHandle_t printf_mutex = NULL;

static QueueHandle_t log_queue;

void log_init(void){
    if (printf_mutex == NULL) {
        printf_mutex = xSemaphoreCreateMutex();
        configASSERT(printf_mutex != NULL);
    }

    if (log_queue == NULL) {
        log_queue = xQueueCreate(LOGGING_QUEUE_LENGTH, sizeof(LogEntry*));
        configASSERT(log_queue != NULL);
    }
}

void log_info(const char* msg){
    log_to_queue(LOG_INFO, msg);
};

void log_to_queue(LogLevel lvl, const char* msg){
    if(log_queue != NULL){
        LogEntry* entry = pvPortMalloc(sizeof(LogEntry));
        entry->level = lvl;
        entry->timestamp = xTaskGetTickCount();
        strncpy(entry->data, msg, sizeof(entry->data) - 1);
        entry->data[sizeof(entry->data) - 1] = '\0';


        if (xQueueSendToBack(log_queue, &entry, 0) != pdTRUE) {
            vPortFree(entry);
        }
    }
};

void print_log(void){
    if(log_queue != NULL && printf_mutex != NULL){

        LogEntry* entry = NULL;
        while(uxQueueMessagesWaiting(log_queue) != 0){
            if(xQueueReceive(log_queue, &entry, 0) != pdTRUE) break;

            if(xSemaphoreTake(printf_mutex, portMAX_DELAY) == pdTRUE){
                switch(entry->level){
                    case LOG_INFO:
                        printf("[%10lu] INFO         | %s\n", entry->timestamp, entry->data);
                        break;
                    default:
                        break;
                }
                xSemaphoreGive(printf_mutex);
            }else{
                vPortFree(entry);
                break;
            }

            vPortFree(entry);
        }
    }
}