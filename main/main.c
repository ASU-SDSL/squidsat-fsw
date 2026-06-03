#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "gse.h"
#include "log.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "projdefs.h"

#include "hardware/gpio.h"
#include "hardware/irq.h"

#include "gse.h"
#include "usb_serial.h"
#include "i2c_util.h"
#include "log.h"

#include "tusb_config.h"
#include "tusb.h"

#include "timing.h"

void led_task(void *pvParameters)
{   
    int LED_PIN = PICO_DEFAULT_LED_PIN; // this is for testing OBC hardware
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    int it = 0; 
    while (true) {
        // log_info("Hello data"); 
        gpio_put(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));

        // printf("Hello data %d\n", it++);

        int c = getchar_timeout_us(0);
        while(c != PICO_ERROR_TIMEOUT){
            printf("Received input: %c\n", c);
            c = getchar_timeout_us(0);
        }

        gpio_put(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        // log_info("We are working"); // this is an example of how to use the logging metric, log_info can be replace with any of
        //                             other values depending on the severity.
    }
}

int main()
{
    __asm volatile ("nop"); // for debugger if desired - not used by default

    usb_serial_init(); 
    gse_init();
    i2c_util_init(); 

    // uint8_t ts_res = timing_init(); 
    
    // while(ts_res){ // retry bc this is critical - get a better solution to failure later 
    //     log_error("CRITICAL - Timing setup fail (%d)", ts_res); 
    //     sleep_ms(1000); 
    //     ts_res = timing_init();
    // }

    // Create the blink task and verify creation succeeded.
    BaseType_t ok;

    ok = xTaskCreate(usb_serial_task, "USB", 256, 0, configMAX_PRIORITIES - 2, &usbTaskHandle);
    vTaskCoreAffinitySet(usbTaskHandle, 1 << 0);
    configASSERT(ok == pdPASS);

    ok = xTaskCreate(led_task, "LED", 2048, NULL, tskIDLE_PRIORITY, NULL);
    configASSERT(ok == pdPASS);

    ok = xTaskCreate(vDebugTask, "DEBUG", 2048, NULL, tskIDLE_PRIORITY, NULL);
    configASSERT(ok == pdPASS);    

    ok = xTaskCreateAffinitySet(log_task, "LOGGING", 2048, NULL, tskIDLE_PRIORITY, (1 << 1), NULL);
    configASSERT(ok == pdPASS);
    
    vTaskStartScheduler();

    while (1) {}
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    printf("STACK OVERFLOW in task: %s\n", pcTaskName ? pcTaskName : "unknown");
    stdio_flush();
    taskDISABLE_INTERRUPTS();
    for (;;) {}
}

// Called if pvPortMalloc() fails to allocate memory.
void vApplicationMallocFailedHook(void)
{
    printf("MALLOC FAILED\n");
    stdio_flush();
    taskDISABLE_INTERRUPTS();
    for (;;) {}
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
static StaticTask_t xPassiveIdleTaskTCBs[configNUMBER_OF_CORES - 1];
static StackType_t  uxPassiveIdleTaskStacks[configNUMBER_OF_CORES - 1][configMINIMAL_STACK_SIZE];

void vApplicationGetPassiveIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                           StackType_t **ppxIdleTaskStackBuffer,
                                           configSTACK_DEPTH_TYPE *puxIdleTaskStackSize,
                                           BaseType_t xPassiveIdleTaskIndex) {
    *ppxIdleTaskTCBBuffer   = &xPassiveIdleTaskTCBs[xPassiveIdleTaskIndex];
    *ppxIdleTaskStackBuffer = uxPassiveIdleTaskStacks[xPassiveIdleTaskIndex];
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