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
#define MAX_LOG_LENGTH 256 // bits


// ---- Packet Definitions ----
typedef enum {
    LOG_INFO,
    LOG_ERROR,
    LOG_WARNING
} LogLevel;

typedef struct {
    uint32_t timestamp;
    LogLevel level;
    char data[MAX_LOG_LENGTH];
} LogEntry;

// ---- Logging Functions ----
void log_init(void);
void log_data(LogLevel priority, const char* fmt, ...);
void print_log(void);

// ---- Logging Macros ----
#define log_info(fmt, ...)    log_data(LOG_INFO,    fmt, ##__VA_ARGS__)
#define log_error(fmt, ...)   log_data(LOG_ERROR,   fmt, ##__VA_ARGS__)
#define log_warning(fmt, ...) log_data(LOG_WARNING, fmt, ##__VA_ARGS__)

#define log_debug(fmt, ...) printf("[DEBUG]" fmt, #__VA_ARGS__) // specifically used when testing GSE Task, do not use out of logging and GSE

#endif // LOG_H