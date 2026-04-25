#ifndef RTC_H
#define RTC_H

#include <stdint.h>

#include "hardware/i2c.h"
#include "pico/util/datetime.h"

// Module: DS3231 - datasheet:
// https://www.elecrow.com/download/DS3231.pdf?srsltid=AfmBOoqeXQdxPk0yC-QR_5l1YSoJ2DDCTgk_9zDA_0T4oqLn5cb0tx7o

/**
 * @brief Converts a tm timestamp to epoch time
 *
 * @param timestamp tm timestamp
 * @return time_t Epoch time (seconds)
 */
time_t rtc_tm_to_epoch(struct tm* timestamp);

/**
 * @brief Calls rtc_get_tm_base using i2c bus RTC_I2C_BUS from HardwareConfig.h
 *
 * @param now tm struct to write into
 * @return uint8_t 0 on success, 1 on failure (or passthrough i2c error code)
 */
uint8_t rtc_get_tm(struct tm* now);

/**
 * @brief Reads date and time from RTC and builds a tm struct
 *
 * @param i2c I2C bus to use
 * @param now tm struct to write into
 * @return uint8_t 0 on success, 1 on failure (or passthrough i2c error code)
 */
uint8_t rtc_get_tm_base(i2c_inst_t* i2c, struct tm* now);

// -------------- unpreferred functions -----------------------------------

/**
 * @brief Set the RTC's date and time, assumes 24h input.
 *
 * @param i2c I2C Instance
 * @param year Last 2 digits (20XX)
 * @param month
 * @param day
 * @param hour
 * @param minute
 * @param second
 * @return uint8_t Status (0 = success)
 */
uint8_t rtc_set_time(i2c_inst_t* i2c, uint8_t year, uint8_t month, uint8_t day,
                     uint8_t hour, uint8_t minute, uint8_t second);

/**
 * ALL GET FUNCTIONS
 * Reads value from RTC and writes it to [output]
 * Returns 0 on success.
 */
uint8_t rtc_get_second(i2c_inst_t* i2c, uint8_t* output);
uint8_t rtc_get_minute(i2c_inst_t* i2c, uint8_t* output);
uint8_t rtc_get_hour(i2c_inst_t* i2c, uint8_t* output);
uint8_t rtc_get_date(i2c_inst_t* i2c, uint8_t* output);
uint8_t rtc_get_month(i2c_inst_t* i2c, uint8_t* output);
uint8_t rtc_get_year(i2c_inst_t* i2c, uint8_t* output);

/**
 * @brief Reads temp (C) from RTC and writes it to [output]
 *
 * @param i2c I2C Instance
 * @param output Temperature in Celsius
 * @return uint8_t Status
 */
uint8_t rtc_get_temp(i2c_inst_t* i2c, float* output);

/**
 * @brief Manually updates value in temp register. RTC auto-updates the temp
 * every 64 seconds.
 *
 * @param i2c I2C Instance
 * @return uint8_t Status (0 = success)
 */
uint8_t rtc_update_temp(i2c_inst_t* i2c);

/**
 * @brief Test RTC functions
 */
void rtc_test();

#endif // RTC_H