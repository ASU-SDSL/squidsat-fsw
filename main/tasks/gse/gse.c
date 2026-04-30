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
    CMD_REALTIMELOG,
    CMD_UNKNOWN
} Command;


static volatile bool debug_mode = false;
static SemaphoreHandle_t debug_mode_mutex;

/*
    How we parse commands through USB. This is what you type into serial monitor.
    CMD_UNKNOWN is a base case, should never get there if you typed everything in correctly.
*/

static Command parse_command(const char* str) {
    if (strcmp(str, "debug") == 0)   return CMD_DEBUG;
    if (strcmp(str, "no_debug") == 0) return CMD_NODEBUG;
    if (strcmp(str, "pull_log") == 0) return CMD_PULLLOG;
    if (strcmp(str, "rt_log") == 0) return CMD_REALTIMELOG;
    return CMD_UNKNOWN;
}

void gse_init(void){
    stdio_init_all();
    debug_mode_init();
    log_init();
}

void debug_mode_init(void){
    if(debug_mode_mutex == NULL) debug_mode_mutex = xSemaphoreCreateMutex();
}

static void debug_mode_set(bool value){
    debug_mode_init();
    if(xSemaphoreTake(debug_mode_mutex, portMAX_DELAY)){
        debug_mode = value;
        xSemaphoreGive(debug_mode_mutex);
    }
}

static bool get_debug_mode(){
    debug_mode_init();
    bool value;
    if(xSemaphoreTake(debug_mode_mutex, portMAX_DELAY)){
        value = debug_mode;
        xSemaphoreGive(debug_mode_mutex);
    }
    log_info("Debug mode (0 = OFF, 1 = ON) %d", value);
    return value;
}

void usb_task(void * param){
    while(1) {
        tud_task(); 
        vTaskDelay(1); 
    }
}

void debug_task(void* param){
    debug_mode_init();
    log_init();
    char buffer[GSE_BUFFER_SIZE];
    int buffer_index = 0;

    for(;;){
        if(stdio_usb_connected() == false){
            vTaskDelay(pdMS_TO_TICKS(GSE_TASK_DELAY_MS)); 
            continue;
        }

        int c = getchar_timeout_us(0);

        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '\n' || c == '\r') {
                buffer[buffer_index] = '\0'; 

                switch(parse_command(buffer)) {
                    case CMD_DEBUG:
                        log_data("Debug mode ON");
                        debug_mode_set(true);
                        get_debug_mode();
                        break;
                    case CMD_NODEBUG:
                        log_data("Debug mode OFF");
                        debug_mode_set(false);
                        get_debug_mode();
                        break;
                    case CMD_PULLLOG:
                        print_log();
                        break;
                    case CMD_REALTIMELOG:
                        break;
                    default:
                        log_warning("Unknown command: %s", buffer);
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