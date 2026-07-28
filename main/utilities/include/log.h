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



/** ---- Global Variables ---- */
#define LOGGING_QUEUE_LENGTH    16
#define MAX_PACKET_SIZE         256 // bytes

/** ---- Packet Definitions ---- */ 
typedef enum {
    LOG_INFO,
    LOG_ERROR,
    LOG_WARNING,
    LOG_MISSION_CRIT
} LogLevel;


/** ---- Queue Entries ----
    This is the way all the Log Entries are defined:
    1. timestamp  - The time at which the log entry was created, recorded in
                    FreeRTOS ticks via xTaskGetTickCount().

    2. level      - The severity level of the log entry, defined by LogLevel.
                    See LogLevel enum for all possible values.

    3. data       - The log message itself, stored as a null-terminated string.
                    Truncated to MAX_PACKET_SIZE - 1 characters if longer.
 */
typedef struct {
    uint32_t timestamp;
    LogLevel level;
    char data[MAX_PACKET_SIZE];
} LogEntry;


/** ---- Logging Functions ---- */


/** ---- Initialization ----
    This is called at the beginning of main.c. It is used to init all the mutex(s)
    and queue(s) that are necessary for the logging functionality to work.
*/
void log_init                   (void);

/** ---- Adding to Queue ----
    This is a sub-function that cannot be called outside of this c file. It sole purpose is to
    create an entry for the packet (Called LogEntry) and then (if the queue has space) add it to the end of the
    queue
*/
static void log_to_queue        (LogLevel lvl, const char* msg);

/** ---- Log Task ----
    This is the actual task that does all the logging, logging is only avialable if debug_mode is on. There is a switch statement
    that defined what all the log entries are supposed to look like and which ones are available for use.
    This function NEEDS to be tied to a singular core, as the C standard printf is not core safe. If both cores attempt to
    access a printf statement at the same time, your freeRTOS project will crash.
*/
void log_task                   (void *pvParameters);

/** ---- Log Levels ----
    This is where all the log levels will be defined:
    
    1. log_info           - Baseline logging level for general status updates and non-critical
                            information. Use this for routine messages that are useful during
                            debugging but not indicative of any problem.

    2. log_error          - Use when something has gone wrong but the system can still continue
                            operating. Indicates a recoverable failure that should be investigated,
                            e.g. a failed sensor read, a dropped packet, or an unexpected return value.

    3. log_warning        - Use when something unexpected happened but the system can recover on
                            its own. Indicates a condition that may become an error if ignored,
                            e.g. a value approaching a limit, a retry attempt, or a degraded 
                            operating mode.

    4. log_mission_critical - Highest severity level. Use when a failure has occurred that endangers
                              the mission or requires immediate operator intervention, e.g. a safety-critical system 
                              going offline.
                              These messages should always be investigated.

    All levels are no-ops if debug mode is disabled or the queue is full.
    All messages are truncated to MAX_PACKET_SIZE - 1 characters if longer.
*/
void log_info                   (const char* msg);
void log_error                  (const char* msg);
void log_warning                (const char* msg);
void log_mission_critical       (const char* msg);

// temp
#define log_infof(fmt, ...) { char buf[256]; snprintf(buf, sizeof(buf), fmt, __VA_ARGS__); log_info(buf); }


#endif // LOG_H