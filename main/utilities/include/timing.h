#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>

#include "pico/util/datetime.h"

uint8_t timing_init();
uint8_t timing_sync();

time_t timing_now_epoch();
struct tm timing_now_tm();

void timing_test();

#endif