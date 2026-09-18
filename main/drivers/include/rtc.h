#ifndef RTC_H
#define RTC_H

#include <stdint.h>

#include "hardware/i2c.h"
#include "pico/util/datetime.h"

time_t rtc_tm_to_epoch(struct tm* timestamp);

uint8_t rtc_get_tm(struct tm* now);

uint8_t rtc_get_tm_base(i2c_inst_t* i2c, struct tm* now);

void rtc_test();

uint8_t rtc_set_time(i2c_inst_t* i2c, uint8_t year, uint8_t month, uint8_t day,
                     uint8_t hour, uint8_t minute, uint8_t second);


// -------------- depreciated functions -----------------------------------

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

uint8_t rtc_update_temp(i2c_inst_t* i2c);
uint8_t rtc_get_temp(i2c_inst_t* i2c, float* output);



#endif // RTC_H