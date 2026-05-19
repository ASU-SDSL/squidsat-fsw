#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "gse.h"
#include "log.h"
#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/adc.h"

#include "gse.h"
#include "i2c_util.h"

#include "timing.h"

void led_task(void *pvParameters)
{   
    int LED_PIN = PICO_DEFAULT_LED_PIN; // this is for testing OBC hardware
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true) {
        gpio_put(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_put(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void pico_temp_task(void *pvParameters)
{   
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);
    while (true) {
        uint16_t result = adc_read();
        
        // Convert to voltage (3.3V reference)
        const float conversion_factor = 3.3f / (1 << 12);
        float voltage = result * conversion_factor;
        
        // Convert voltage to temperature in Celsius
        // Formula: Temp = 27 - (Voltage - 0.706) / 0.001721
        float temp = 27.0f - (voltage - 0.706f) / 0.001721f;
        
        printf("Temperature: %.2f C\n", temp);
        
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
    xTaskCreate(led_task, "LED", 1024, NULL, tskIDLE_PRIORITY + 1UL, NULL);
    xTaskCreate(pico_temp_task, "PICO_TEMP", 1024, NULL, 2, NULL);
    xTaskCreate(vDebugTask, "DEBUG", 1024, NULL, tskIDLE_PRIORITY + 3UL, NULL);
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