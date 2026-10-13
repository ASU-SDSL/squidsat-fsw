
// Get temperature from onboard temp sensor every 1s. Die is on adc input sensor
// 4 for RP2350A. 

#include "pico/stdlib.h"
#include "temp_job.h"
#include "FreeRTOS.h"
#include "log.h"
#include "task.h"
#include "hardware/adc.h"

#define TEMP_SENSOR_ADC_INPUT 4
#define INTERVAL_MS 1000

static bool temp_sensor_state = false;
static uint32_t run_count = 0;
uint16_t raw_value = 0;
float volts = 0;
float temp = 0;

void adc_setup(void){
    temp_sensor_state = true;
    adc_init();
    adc_set_temp_sensor_enabled(temp_sensor_state);
    adc_select_input(TEMP_SENSOR_ADC_INPUT);
};

void read_temp_job_recurring(void *args){
    (void)args;
    run_count++;
    adc_select_input(TEMP_SENSOR_ADC_INPUT);
    raw_value = adc_read();
    volts = raw_value * (3.3f / (1 << 12));
    temp = 27.0 - (volts - 0.706) / 0.001721;
    log_infof("Onboard temp: %f", temp);
};

jobs_t read_temp_recurring = {
    .func = read_temp_job_recurring,
    .recurr_time = pdMS_TO_TICKS(INTERVAL_MS),
    .execute_time = 0,
    .name = "Temp per sec",
    .args = NULL,
};
