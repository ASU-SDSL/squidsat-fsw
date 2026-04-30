#ifndef LOG_H
#define LOG_H

#include "pico/stdlib.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include "gse.h"

// ---- Global Variables ----
#define LOGGING_QUEUE_LENGTH 16
#define MAX_PACKET_SIZE 256 // bits
#define MAX_LOG_LENGTH 256 // entries in the queue


// ---- Packet Definitions ----
typedef enum {
    LOG_INFO,
    LOG_ERROR,
    LOG_WARNING,
    LOG_MISSION_CRIT
} LogLevel;

typedef struct {
    uint32_t timestamp;
    LogLevel level;
    char data[MAX_PACKET_SIZE];
} LogEntry;

// ---- Logging Functions ----
void log_init(void);
void log_to_queue(LogLevel priority, const char* fmt, ...);
void print_log(void);

// ---- Logging Macros ----
extern SemaphoreHandle_t _log_mutex;

// protects printf for multicore calls 
#define _log_safe(fmt, ...) \
        if(xSemaphoreTake(_log_mutex, portMAX_DELAY)){ \
                printf(fmt, ##__VA_ARGS__); \
                xSemaphoreGive(_log_mutex); \
        }

#define log_data(fmt, ...) _log_safe("[DATA] " fmt "\n", ##__VA_ARGS__);

#define log_info(fmt, ...)\
        _log_safe("[INFO]" fmt "\n", ##__VA_ARGS__);\
        log_to_queue(LOG_INFO, fmt, ##__VA_ARGS__);

#define log_error(fmt, ...)\
        _log_safe("[ERROR] " fmt "\n", ##__VA_ARGS__);\
        log_to_queue(LOG_ERROR, fmt, ##__VA_ARGS__);

#define log_warning(fmt, ...)\
        _log_safe("[WARNING] " fmt "\n", ##__VA_ARGS__);\
        log_to_queue(LOG_WARNING, fmt, ##__VA_ARGS__);

#define log_mission_crit(fmt, ...)\
        _log_safe("[MISSION CRITICAL] " fmt "\n", ##__VA_ARGS__);\
        log_to_queue(LOG_MISSION_CRIT, fmt, ##__VA_ARGS__);

#endif // LOG_H