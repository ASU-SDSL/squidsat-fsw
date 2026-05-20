#include "log.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

//** ---- Variable Declarations ---- */ 

static SemaphoreHandle_t printf_mutex = NULL;
static QueueHandle_t log_queue = NULL;

//** ---- Function Declarations and Definitions ---- */

void log_init(void){
    if (printf_mutex == NULL) {
        printf_mutex = xSemaphoreCreateMutex();
        configASSERT(printf_mutex != NULL);
    }

    if (log_queue == NULL) {
        log_queue = xQueueCreate(LOGGING_QUEUE_LENGTH, sizeof(LogEntry*));
        configASSERT(log_queue != NULL);
    }
};

static void log_to_queue(LogLevel lvl, const char* msg){
    if(log_queue == NULL || !get_debug_mode()) return;

    LogEntry* entry = pvPortMalloc(sizeof(LogEntry));
    if(entry == NULL) return;
    entry->level = lvl;
    entry->timestamp = xTaskGetTickCount();
    strncpy(entry->data, msg, MAX_PACKET_SIZE - 1);
    entry->data[MAX_PACKET_SIZE - 1] = '\0';

    if(xQueueSendToBack(log_queue, &entry, 0) != pdTRUE) vPortFree(entry);
};


void log_task(void *pvParameters){
    for(;;){
        if(!get_debug_mode() || log_queue == NULL || printf_mutex == NULL){
            LogEntry* entry;
            while(xQueueReceive(log_queue, &entry, 0) == pdTRUE){
                vPortFree(entry);
            }
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;  // keep looping, waiting for debug to be enabled
        }

        {
            LogEntry* entry;
            if(xQueueReceive(log_queue, &entry, portMAX_DELAY) == pdTRUE){
                if(xSemaphoreTake(printf_mutex, portMAX_DELAY) == pdTRUE){
                    switch(entry->level){
                        case LOG_INFO:
                            printf("[%10lu] INFO         | %s\n", entry->timestamp, entry->data);
                            break;
                        case LOG_ERROR:
                            printf("[%10lu] ERROR        | %s\n", entry->timestamp, entry->data);
                            break;
                        case LOG_WARNING:
                            printf("[%10lu] WARNING      | %s\n", entry->timestamp, entry->data);
                            break;
                        case LOG_MISSION_CRIT:
                            printf("[%10lu] MISSION CRIT | %s\n", entry->timestamp, entry->data);
                            break;
                        default:
                            printf("[%10lu] UNKNOWN      | %s\n", entry->timestamp, entry->data);
                            break;
                    }
                    xSemaphoreGive(printf_mutex);
                }
                vPortFree(entry);
            }

        }
    }
}

void log_info(const char* msg){ log_to_queue(LOG_INFO, msg); };
void log_error(const char* msg){ log_to_queue(LOG_ERROR, msg); };
void log_warning(const char* msg){ log_to_queue(LOG_WARNING, msg); };
void log_mission_critical(const char* msg){ log_to_queue(LOG_MISSION_CRIT, msg); };