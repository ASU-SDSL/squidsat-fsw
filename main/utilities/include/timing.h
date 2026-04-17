#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>

#include "pico/util/datetime.h"

/**
 * @brief Initializes the AON Timer from the RTC, intended to be called on boot
 * before schedular is started
 *
 * @return uint8_t
 */
uint8_t timing_init();

/**
 * @brief Syncs the AON Timer with the RTC, intended as the only reading
 * interaction with the RTC after this all timing should come from AON or uptime
 * - call periodically
 *
 * @return uint8_t
 */
uint8_t timing_sync();

/**
 * @brief Returns current epoch time timestamp from AON Timer
 *
 * @return time_t Epoch time in seconds
 */
time_t timing_now_epoch();

/**
 * @brief Returns current tm timestamp from AON Timer
 *
 * @return struct tm Timestamp
 */
struct tm timing_now_tm();

/**
 * @brief Testing timing functionality, needs to be called after timing_init()
 *
 */
void timing_test();

#endif