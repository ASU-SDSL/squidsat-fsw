#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "gse.h"
#include "task.h"
#include "pico/error.h"
#include "pico/stdlib.h"

/*
This function allows for basic debug handling. Will need to call a subfunction that actually runs the debug mode
Ideas:
    1. Step through code
    2. Log error messages into a log file
    3. Allow for unique hooks (Look into this)
    4. (Talk to Tyler N)
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

volatile bool debug_mode = false;

// function to parse the input and return if it is a known or unknown command
Command parse_command(const char* str) {
    if (strcmp(str, "debug") == 0)   return CMD_DEBUG;
    if (strcmp(str, "no_debug") == 0) return CMD_NODEBUG;
    if (strcmp(str, "pull_log") == 0) return CMD_PULLLOG;
    return CMD_UNKNOWN;
}


void vDebugTask(void* pm){
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
                        log_info("Unknown command: %s", buffer);
                        break;
                }
                
                buffer_index = 0;
            } else {
                if(buffer[buffer_index] < 255) buffer[buffer_index++] = c;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(GSE_TASK_DELAY_MS));
    }
}