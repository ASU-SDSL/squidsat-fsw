#include <FreeRTOS.h>

#include "gse.h"
#include "log.h"

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
SemaphoreHandle_t debug_mode_mutex;

/*
    How we parse commands through USB. This is what you type into serial monitor.
    CMD_UNKNOWN is a base case, should never get there if you typed everything in correctly.
*/

Command parse_command(const char* str) {
    if (strcmp(str, "debug") == 0)   return CMD_DEBUG;
    if (strcmp(str, "no_debug") == 0) return CMD_NODEBUG;
    if (strcmp(str, "pull_log") == 0) return CMD_PULLLOG;
    return CMD_UNKNOWN;
}

void gse_init(void){
    tud_task();
    stdio_init_all();
}

void debug_mode_init(){
    if(debug_mode_mutex == NULL) debug_mode_mutex = xSemaphoreCreateMutex();
}

void debug_mode_set(bool value){
    debug_mode_init();
    xSemaphoreTake(debug_mode_mutex, portMAX_DELAY);
    debug_mode = value;
    xSemaphoreGive(debug_mode_mutex);
}

bool get_debug_mode(){
    debug_mode_init();
    bool value;
    xSemaphoreTake(debug_mode_mutex, portMAX_DELAY);
    value = debug_mode;
    xSemaphoreGive(debug_mode_mutex);
    log_info("Debug mode (0 = OFF, 1 = ON) %d", value);
    return value;
}


void vDebugTask(void* pm){
    debug_mode_init();
    char buffer[GSE_BUFFER_SIZE];
    int buffer_index = 0;

    for(;;){
        int c = getchar_timeout_us(0); 

        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '\n' || c == '\r') {
                buffer[buffer_index] = '\0'; 

                switch(parse_command(buffer)) {
                    case CMD_DEBUG:
                        log_debug("Debug Mode ON");
                        debug_mode_set(true);
                        get_debug_mode();
                        break;
                    case CMD_NODEBUG:
                        log_debug("Debug MODE OFF");
                        debug_mode_set(false);
                        get_debug_mode();
                        break;
                    case CMD_PULLLOG:
                        print_log();
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