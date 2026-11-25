#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>

#include "pico/util/datetime.h"

#define ms_to_ticks(ms) (pdMS_TO_TICKS((ms)))
#define seconds_to_ticks(seconds) (pdMS_TO_TICKS((seconds) * (1000)))
#define minutes_to_ticks(minutes) (pdMS_TO_TICKS((minutes) * (60000)))
#define hours_to_ticks(hours) (pdMS_TO_TICKS((hours) * (3600000)))
#define days_to_ticks(days) (pdMS_TO_TICKS((days) * (86400000)))
#define MS_IN_SEC 1000
#define SECS_IN_MIN 60

uint8_t timing_init();
uint8_t timing_sync();

time_t timing_now_epoch();
struct tm timing_now_tm();

void timing_test();

#endif