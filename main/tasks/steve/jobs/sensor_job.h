#pragma once

#include "../steve.h"

void led_blinking_job(void *args);
void sensor_setup(void);
void heart_beat_job(void *args);


extern jobs_t led_blinking;
extern jobs_t heart_beat;