#include "gse.h"
#include "log.h"

typedef enum {
    CMD_DEBUG,
    CMD_NODEBUG,
    CMD_PULLLOG,
    CMD_UNKNOWN
} Command;

// default to debug off for release builds, debug on for debug builds
#ifdef DEBUG_BUILD 
static volatile bool debug_mode = true;
#else 
static volatile bool debug_mode = true;
#endif

static SemaphoreHandle_t debug_mode_mutex;


static Command parse_command(const char* str) {
    if (strcmp(str, "debug") == 0)   return CMD_DEBUG;
    if (strcmp(str, "no_debug") == 0) return CMD_NODEBUG;
    if (strcmp(str, "pull_log") == 0) return CMD_PULLLOG;
    return CMD_UNKNOWN;
};

void gse_init(void){
    stdio_init_all();
    debug_mode_init();
    log_init();
};



void debug_mode_init(void){
    if(debug_mode_mutex == NULL) debug_mode_mutex = xSemaphoreCreateMutex();
};



static void debug_mode_set(bool value){
    debug_mode_init();
    if(xSemaphoreTake(debug_mode_mutex, portMAX_DELAY) == pdTRUE){
        debug_mode = value;
        xSemaphoreGive(debug_mode_mutex);
    }
};



bool get_debug_mode(void){
    debug_mode_init();
    if (debug_mode_mutex == NULL) {
        return false;
    }
    bool value = false;
    if (xSemaphoreTake(debug_mode_mutex, portMAX_DELAY) == pdTRUE) {
        value = debug_mode;
        xSemaphoreGive(debug_mode_mutex);
    }
    return value;
};

void usb_task(void * param){
    while(1) {
        tud_task(); 
        vTaskDelay(1); 
    }
}


void vDebugTask(void *pvParameters){
    debug_mode_init();
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
                        debug_mode_set(true);
                        get_debug_mode();
                        break;
                    case CMD_NODEBUG:
                        debug_mode_set(false);
                        get_debug_mode();
                        break;
                    default:
                        break;
                }
                buffer_index = 0;
            } else {
                if(buffer_index < 255) {buffer[buffer_index++] = c;}
            }
        }
        vTaskDelay(pdMS_TO_TICKS(GSE_TASK_DELAY_MS));
    }
};