#ifndef BUZZERTASK
#define BUZZERTASK


#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "pico/stdlib.h"


#define BUZZER_PIN 26 // One of the unused GPIO pins, subject to change
#define BUZZER_FREQUENCY 5000 // MHz
#define BUZZER_DELAY 500 // ms
#define BUZZ_AMOUNT 3 // Subject to change

void buzzer_init(void);
void buzzer_on(void);
void buzzer_off(void);

void vBuzzerTask(void* pvParameters);

#endif