#include "timing.h"
#include "FreeRTOS.h"
#include "semphr.h"

#include "gse.h"
#include "log.h"
#include "hardware/i2c.h"
#include "pico/aon_timer.h"
#include "rtc.h"

static SemaphoreHandle_t timing_mutex;

// only used before schedular starts 
uint8_t timing_init() {
  if (timing_mutex == NULL){
    timing_mutex = xSemaphoreCreateMutex(); 
  }

  // timing_sync WITHOUT mutex take to be use only before schedular starts 
  struct tm now;

  uint8_t res = rtc_get_tm(&now);
  if (res != 0) return res;

  if (aon_timer_set_time_calendar(&now) == false) {
    return 1;
  }

  //log_info("AON Timing initialized as %d/%d/%d %d:%d:%d", now.tm_year,
  //         now.tm_mon, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);

  return 0;
}

uint8_t timing_sync() {
  struct tm now;

  uint8_t res = rtc_get_tm(&now);
  if (res != 0) return res;

  if (xSemaphoreTake(timing_mutex, portMAX_DELAY) == pdTRUE) {
    if (aon_timer_set_time_calendar(&now) == false) {
      return 1;
    }
    xSemaphoreGive(timing_mutex);
  }

  return 0;
}

time_t timing_now_epoch() {
  struct timespec ts;

  if (xSemaphoreTake(timing_mutex, portMAX_DELAY) == pdTRUE) {
    if (aon_timer_get_time(&ts) == false) {
      //log_error("CRITICAL - AON Timer Failed (timing_now_epoch)");
    }
    xSemaphoreGive(timing_mutex);
  }

  return ts.tv_sec;
}

struct tm timing_now_tm() {
  struct tm now;
  
  if (xSemaphoreTake(timing_mutex, portMAX_DELAY) == pdTRUE) {
    if (aon_timer_get_time_calendar(&now) == false) {
      //log_error("CRITICAL - AON Timer Failed (timing_now_tm)")
    }
    xSemaphoreGive(timing_mutex);
  }

  return now;
}

void timing_test() {
  
  log_infof("Time: %lld", timing_now_epoch());

}
