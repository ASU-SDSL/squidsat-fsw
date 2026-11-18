#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static bool payload_active;
static uint32_t payload_started_ms;
static uint32_t payload_run_ms;

static void turn_on_payload(void)
{
    //start payload
}

static uint16_t read_payload_data(void)
{
    
}

static void payload_downlink(uint16_t sample)
{
    //downlink
}

static void store_payload_info(uint16_t sample)
{
   // save to mram
}

void payload_start(uint32_t now_ms, uint32_t run_ms)
{
    if (payload_active) {
        return;
    }

    turn_on_payload();
    payload_started_ms = now_ms;
    payload_run_ms = run_ms;
    payload_active = true;
}


void payload_interface(uint32_t now_ms)
{
    if (!payload_active || (uint32_t)(now_ms - payload_started_ms) < payload_run_ms) {
        return;
    }

    uint16_t sample = read_payload_data();

    payload_downlink(sample);
    store_payload_info(sample);
    payload_active = false;
}