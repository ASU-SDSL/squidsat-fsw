
// Get temperature from onboard temp sensor every 1s. Die is on adc input sensor
// 4 for RP2350A. 

#include "pico/stdlib.h"
#include "temp_job.h"
#include "FreeRTOS.h"
#include "log.h"
#include "task.h"
#include "hardware/adc.h"

/**
 * @authors: Brett Schwartz
 * @breif: Onboard Temperature Job for Steve Scheduler
 * @version: 1.0
 * @date: 2026-10-13
 */

#define TEMP_SENSOR_ADC_INPUT 4
#define INTERVAL_MS 1000

static bool temp_sensor_state = false;
static uint32_t run_count = 0;
uint16_t raw_value = 0;
float volts = 0;
float temp = 0;

/**
 * @breif: Function to initilize ADC temperature sensor
 * 
 * This function initializes the ADC, sets the temperature 
 * sensor on ADC input 4 to true, and selects the ADC input
 * as the temp sensor (input 4).
 */
void adc_setup(void){
    temp_sensor_state = true;
    adc_init();
    adc_set_temp_sensor_enabled(temp_sensor_state);
    adc_select_input(TEMP_SENSOR_ADC_INPUT);
};

/**
 * @breif: Function to recieve and calculate onboard temp.
 * 
 * This function increments its run count as it is recurring,
 * reselects the temp adc input incase another job changed it,
 * reads the raw input from adc, converts to volts and then to
 * celsius, and logs the output.
 * 
 * @param args Any extra arguments passed through and unused.
 */
void read_temp_job_recurring(void *args){
    (void)args;
    run_count++;
    adc_select_input(TEMP_SENSOR_ADC_INPUT);
    raw_value = adc_read();
    volts = raw_value * (3.3f / (1 << 12));
    temp = 27.0 - (volts - 0.706) / 0.001721;
    log_infof("Onboard temp: %f", temp);
};

/**
 * @breif: STEVE Scheduler Implementation.
 * 
 * This is the job decleration that STEVE will use,
 * it sets the function as a pointer to the above
 * 'read_temp_job_recurring' function. Sets the
 * recurr time of 1000ms (1s) to ticks so freeRTOS
 * can use it. Sets execute time to 0 so it starts
 * ASAP, names it and ignores any inputted args.
 * 
 */
jobs_t read_temp_recurring = {
    .func = read_temp_job_recurring,
    .recurr_time = pdMS_TO_TICKS(INTERVAL_MS),
    .execute_time = 0,
    .name = "Temp per sec",
    .args = NULL,
};
