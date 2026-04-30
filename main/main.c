#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "gse.h"
#include "log.h"
#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/gpio.h"
#include "hardware/irq.h"

#include "gse.h"
#include "i2c_util.h"

#include "timing.h"

void led_task(void *pvParameters)
{   
    int LED_PIN = PICO_DEFAULT_LED_PIN; // this is for testing OBC hardware
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true) {
        log_data("Hello data"); 
        gpio_put(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_put(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


int main()
{
    gse_init();
    // i2c_util_init(); 

    // uint8_t ts_res = timing_init(); 
    
    // while(ts_res){ // retry bc this is critical - get a better solution to failure later 
    //     log_error("CRITICAL - Timing setup fail (%d)", ts_res); 
    //     sleep_ms(1000); 
    //     ts_res = timing_init();
    // }

    // Create the blink task and verify creation succeeded.
    xTaskCreate(usb_task, "USB", 1024, NULL, tskIDLE_PRIORITY + 2UL, NULL);
    xTaskCreate(led_task, "LED", 1024, NULL, tskIDLE_PRIORITY + 1UL, NULL);
    xTaskCreate(debug_task, "DEBUG", 1024, NULL, tskIDLE_PRIORITY + 2UL, NULL);
    vTaskStartScheduler();

    while (1) {}
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    log_error("STACK OVERFLOW in task: %s\n", pcTaskName);
    stdio_flush();    
    configASSERT(0);
}

// Called if pvPortMalloc() fails to allocate memory.
void vApplicationMallocFailedHook(void)
{
    // Trap here for debugging — replace with your own error handling
    configASSERT(0);
}


/* Idle task memory */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];


void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    configSTACK_DEPTH_TYPE *puxIdleTaskStackSize) {
    *ppxIdleTaskTCBBuffer   = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *puxIdleTaskStackSize   = configMINIMAL_STACK_SIZE;
}

/* RP2350 passive idle task memory (second core idle) */
static StaticTask_t xPassiveIdleTaskTCB;
static StackType_t uxPassiveIdleTaskStack[configMINIMAL_STACK_SIZE];


void vApplicationGetPassiveIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                           StackType_t **ppxIdleTaskStackBuffer,
                                           configSTACK_DEPTH_TYPE *puxIdleTaskStackSize,
                                           BaseType_t xPassiveIdleTaskIndex) {
    *ppxIdleTaskTCBBuffer   = &xPassiveIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxPassiveIdleTaskStack;
    *puxIdleTaskStackSize   = configMINIMAL_STACK_SIZE;
}

/* Timer task memory */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];


void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     configSTACK_DEPTH_TYPE *puxTimerTaskStackSize) {
    *ppxTimerTaskTCBBuffer   = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *puxTimerTaskStackSize   = configTIMER_TASK_STACK_DEPTH;
}