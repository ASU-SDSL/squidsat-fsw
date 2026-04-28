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
void log_data(LogLevel priority, const char* fmt, ...);
void print_log(void);

// ---- Logging Macros ----

// TASK - need to change the Log_info and log_warning to actually just print when debug mode is on, not that hard
// TASK - need to add mission critical that will add to log error will also add to log 
#define log_info(fmt, ...) printf("[INFO] " fmt, ##__VA_ARGS__)

#define log_error(fmt, ...)\
        printf("[ERROR] " fmt, ##__VA_ARGS__)\
        log_data(LOG_ERROR,   fmt, ##__VA_ARGS__)

#define log_warning(fmt, ...)\
        printf("[WARNING] " fmt, ##__VA_ARGS__)\
        log_data(LOG_WARNING, fmt, ##__VA_ARGS__)
#define log_mission_crit(fmt, ...)\
        printf("[MISSION CRITICAL] " fmt, ##__VA_ARGS__)\
        log_data(LOG_MISSION_CRIT, fmt, ##__VA_ARGS__)

#define log_debug(fmt, ...)  printf("[GSE DEBUG LOG] " fmt, ##__VA_ARGS__)

#endif // LOG_H