#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"

#include "rtc.h"

void led_task(void *pvParameters)
{   
    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true) {
        gpio_put(LED_PIN, 1);
        vTaskDelay(100);
        gpio_put(LED_PIN, 0);
        vTaskDelay(100);
    }
}

int main()
{
    const uint LED_PIN = 25;

    stdio_init_all();

    sleep_ms(1000);
    
    while(1){
        rtc_test(); 
    }

    // Create the blink task and verify creation succeeded.
    xTaskCreate(led_task, "LED", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    vTaskStartScheduler();

    while (1) {}
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void) xTask;
    (void) pcTaskName;

    // Trap here for debugging — replace with your own error handling
    configASSERT(0);
}

// Called if pvPortMalloc() fails to allocate memory.
void vApplicationMallocFailedHook(void)
{
    // Trap here for debugging — replace with your own error handling
    configASSERT(0);
}
