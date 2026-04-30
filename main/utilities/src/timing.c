#include "timing.h"

#include "gse.h"
#include "log.h"
#include "hardware/i2c.h"
#include "pico/aon_timer.h"
#include "rtc.h"

uint8_t timing_init() {
  struct tm now;

  uint8_t res = rtc_get_tm(&now);
  if (res != 0) return res;

  if (aon_timer_start_calendar(&now) == false) {
    return 1;
  }

  log_info("AON Timing initialized as %d/%d/%d %d:%d:%d", now.tm_year,
           now.tm_mon, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);

  return 0;
}

uint8_t timing_sync() {
  struct tm now;

  uint8_t res = rtc_get_tm(&now);
  if (res != 0) return res;

  if (aon_timer_set_time_calendar(&now) == false) {
    return 1;
  }

  return 0;
}

time_t timing_now_epoch() {
  struct timespec ts;
  if (aon_timer_get_time(&ts) == false) {
    log_error("CRITICAL - AON Timer Failed (timing_now_epoch)");
  }

  return ts.tv_sec;
}

struct tm timing_now_tm() {
  struct tm now;

  if (aon_timer_get_time_calendar(&now) == false) {
    log_error("CRITICAL - AON Timer Failed (timing_now_tm)")
  }

  return now;
}

void timing_test() {
  while (1) {
    log_info("Time: %lld", timing_now_epoch());
    sleep_ms(1000);
  }
}
