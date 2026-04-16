#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "semaphore.h"
#include "gse.h"
#include "task.h"
#include "pico/error.h"
#include "pico/stdlib.h"

/*
This function allows for basic debug handling. Will need to call a subfunction that actually runs the debug mode
Ideas:
    1. Talk to Tyler N on how to make debug hooks that we can call once debug mode
       is enabled. 
    2. Want to maybe make a log command to see all the errors (might be part of debug mode tho) 
*/

/*
    List of known commands
    Make sure that if you make a new command you want to use over usb, that is is added here 
*/
typedef enum {
    CMD_DEBUG,
    CMD_NODEBUG,
    CMD_PULLLOG,
    CMD_UNKNOWN
} Command;


SemaphoreHandle_t debug_handeling;
volatile bool debug_mode = false;

/*
    How we parse commands through USB. This is what you type into serial monitor.
    CMD_UNKNOWN is a base case, should never get there if you typed everything in right.
*/

Command parse_command(const char* str) {
    if (strcmp(str, "debug") == 0)   return CMD_DEBUG;
    if (strcmp(str, "no_debug") == 0) return CMD_NODEBUG;
    if (strcmp(str, "pull_log") == 0) return CMD_PULLLOG;
    return CMD_UNKNOWN;
}


void vDebugTask(void* pm){
    debug_handeling = xSemaphoreCreateMutex();
    char buffer[256];
    int buffer_index = 0;

    for(;;){
        int c = getchar_timeout_us(0); 

        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '\n' || c == '\r') {
                buffer[buffer_index] = '\0'; 

                switch(parse_command(buffer)) {
                    case CMD_DEBUG:
                        debug_mode = true;
                        log_info("Debug mode ON");
                        break;
                    case CMD_NODEBUG:
                        debug_mode = false;
                        log_info("Debug mode OFF");
                        break;
                    case CMD_PULLLOG:
                        log_info("The log are:");
                        break;
                    default:
                        log_error("Unknown command: %s", buffer);
                        break;
                }
                
                buffer_index = 0;
            } else {
                if(buffer_index < 255) {buffer[buffer_index++] = c;}
            }
        }
        vTaskDelay(pdMS_TO_TICKS(GSE_TASK_DELAY_MS));
    }
}