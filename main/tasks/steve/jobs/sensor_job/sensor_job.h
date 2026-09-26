#pragma once

#include "steve.h"

void led_blinking_job_once(void *args);
void led_blinking_job_on_off(void *args);
void led_blinking_job_fast_blink(void *args);
void led_blinking_job_recurring(void *args);
void sensor_setup(void);

extern jobs_t led_blinking_once;
extern jobs_t led_blinking_onoff;
extern jobs_t led_blinking_fast_blink;
extern jobs_t led_blinking_recurr;

