#include "pico/stdlib.h"
#include <sensor_job.h>
#include <log.h>

#define HOLD_MS 1000
#define HOLD_MS_onoff 7000
#define FAST_BLINK_MS  70

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

static bool led_state = false;
static uint32_t run_count = 0;

void sensor_setup(void){
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);
    led_state = true;
}

//test 1: LED flash one time
void led_blinking_job_once(void *args){
    (void)args;
    gpio_put(LED_PIN, 0);
    led_state = false;
    log_error("once: LED off");
}  
// test 2: turn the LED on, hold a few seconds, then turn it off
void led_blinking_job_on_off(void *args){
    (void)args;

    gpio_put(LED_PIN, 1);
    led_state = true;
    log_error("on_off: LED on");

    sleep_ms(HOLD_MS_onoff);

    gpio_put(LED_PIN, 0);
    led_state = false;
    log_error("on_off: LED off");
}

void led_blinking_job_fast_blink(void *args){
    (void)args;

    // fast blink two times
    for(int i = 0; i < 2; i++){
        gpio_put(LED_PIN, 1);
        sleep_ms(FAST_BLINK_MS);
        gpio_put(LED_PIN, 0);
        sleep_ms(FAST_BLINK_MS);
    }

    // hold on for a few seconds
    gpio_put(LED_PIN, 1);
    sleep_ms(HOLD_MS);
    gpio_put(LED_PIN, 0);

    for(int i = 0; i < 2; i++){
        gpio_put(LED_PIN, 1);
        sleep_ms(FAST_BLINK_MS);
        gpio_put(LED_PIN, 0);
        sleep_ms(FAST_BLINK_MS);
    }

    led_state = false;
    log_error("solid: done");
}

//test 4: LED running multiple times
void led_blinking_job_recurring(void *args){
    (void)args;
    run_count++;
    led_state = !led_state;
    gpio_put(LED_PIN, led_state);
    
}

jobs_t led_blinking_once = {
    .func = led_blinking_job_once,
    .recurr_time = 0,
    .execute_time = 1000, //lower number higher prio
    .name = "LED once",
    .args = NULL,
};

jobs_t led_blinking_onoff = {
    .func = led_blinking_job_on_off,
    .recurr_time = 0,
    .execute_time = 2000, 
    .name = "LED on/off",
    .args = NULL,
};

jobs_t led_blinking_fast_blink = {
    .func = led_blinking_job_fast_blink,
    .recurr_time = 0,
    .execute_time = 3000,
    .name = "LED fast blink",
    .args = NULL,
};

jobs_t led_blinking_recurr = {
    .func = led_blinking_job_recurring,
    .recurr_time = 500,     
    .execute_time = 4000,
    .name = "LED recurring",
    .args = NULL,
};