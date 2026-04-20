#include "gse.h"

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

void gse_init(){
    tud_task();
    stdio_init_all();
}

void debug_mode_init(){
    debug_mode_mutex = xSemaphoreCreateMutex();
}

void debug_mode_set(bool value){
    xSemaphoreTake(debug_mode_mutex, portMAX_DELAY);
    debug_mode = value;
    xSemaphoreGive(debug_mode_mutex);
}

void get_debug_mode(){
    bool value;
    xSemaphoreTake(debug_mode_mutex, portMAX_DELAY);
    value = debug_mode;
    xSemaphoreGive(debug_mode_mutex);
    log_info("Debug mode (0 = false, 1 = true) %d", value);
}

void close_debug(SemaphoreHandle_t debug_mode_mutex){
    vSemaphoreDelete(debug_mode_mutex);
}

void vDebugTask(void* pm){
    char buffer[GSE_BUFFER_SIZE];
    int buffer_index = 0;

    for(;;){
        int c = getchar_timeout_us(0); 

        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '\n' || c == '\r') {
                buffer[buffer_index] = '\0'; 

                switch(parse_command(buffer)) {
                    case CMD_DEBUG:
                        debug_mode_init();
                        debug_mode_set(true);
                        get_debug_mode();
                        break;
                    case CMD_NODEBUG:
                        debug_mode_set(false);
                        get_debug_mode();
                        close_debug(debug_mode_mutex);
                        break;
                    case CMD_PULLLOG:
                        log_info("The logs are: ");
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

void logging_in_debug(volatile bool debug_mode){
    if(debug_mode){

    }else{
        return;
    }
}