#pragma once

#include "../steve.h"

void led_blinking_job(void *args);
void sensor_setup(void);
void heart_beat_job(void *args);


extern scheduler_t led_blinking;
extern scheduler_t heart_beat;