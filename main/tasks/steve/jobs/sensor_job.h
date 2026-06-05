#pragma once

#include "../steve.h"

void led_blinking_job(void *args);
void sensor_setup(void);


extern scheduler_t led_blinking;