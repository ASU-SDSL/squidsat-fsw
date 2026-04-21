#include "buzzer.h"

void buzzer_init(void) {
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
}

void buzzer_on(void)  { gpio_put(BUZZER_PIN, 1); }
void buzzer_off(void) { gpio_put(BUZZER_PIN, 0); }


void vBuzzerTask(void* pvParameters){
    buzzer_init();
    for (int i = 0; i <= BUZZ_AMOUNT; i++){
        buzzer_on();
        vTaskDelay(500);
        buzzer_off();
    }
}