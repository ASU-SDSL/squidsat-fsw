#include "pico/stdlib.h"
#include <sensor_job.h>

//hardcode for testing


const uint LED_PIN = PICO_DEFAULT_LED_PIN; 

void setup(void *args){
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}

void led_blinking_job(void *args){
    static bool led_state = false; //track the state of the LED (true for on, false for off), set default to off
    led_state = !led_state; 
    gpio_put(LED_PIN, led_state); 
    printf("LED state: %s\n", led_state ? "ON" : "OFF"); 
}


static scheduler_t led_blinking = {
    .func = led_blinking_job,
    .recurr_time = pdMS_TO_TICKS(500),
    .execute_time = 0, 
    .name = "LED Blinking Job",
    .args = NULL

}