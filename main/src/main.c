#include "FreeRTOS.h"
#include "task.h"
#include "gse.h"
#include <stdio.h>
#include "pico/stdlib.h"


void led_task(void *pvParameters)
{   
    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true) {
        // Talk to Tyler N abou why this is bricking the FreeRTOS config
        gpio_put(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        if(debug_mode == true){
            log_info("The LED is %d", gpio_get(LED_PIN));
        }
        gpio_put(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
        if(debug_mode == true){
            log_info("The LED is %d", gpio_get(LED_PIN));
        }        
    }
}


int main()
{
    tud_task();
    stdio_init_all();

    // Create the blink task and verify creation succeeded.
    xTaskCreate(led_task, "LED", 1024, NULL, tskIDLE_PRIORITY + 1UL, NULL);
    xTaskCreate(vDebugTask, "DEBUG", 1024, NULL, tskIDLE_PRIORITY + 2UL, NULL);
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
