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
void log_to_queue(LogLevel lvl, const char* msg);
void print_log(void);

// ---- Logging Functions ----

void log_info(const char* msg);
//void log_error(const char* msg);


#endif // LOG_H