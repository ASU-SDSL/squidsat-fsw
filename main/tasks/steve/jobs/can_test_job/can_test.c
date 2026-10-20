/**
 * @file can_test.c
 * @author Koowum Joshi
 * @brief CAN job to put on Steve Scheduler through the GSE task
 * 
 * @version 1.0
 * @date 10/19/2026
 */

#include "can_test.h"

void can_test_frame(void *args){
    (void) args;
    can_message_t m = {
        .id = 0x18CE0102,   // DATA traffic-class base 0x18CE0000 | target 0x01 (STM32) | source 0x02 (OBC)        
        .extended = 1,
        .dlc      = 8,
        .data     = {0x01,        // ISO-TP header: single frame, 1 payload byte
                    0x01,        // payload: led_test's blink command
                    0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC},  // padding
    };
    add_can_transmission(&m);
}

jobs_t can_job = {
    .args = NULL,
    .func = can_test_frame,
    .name = "CAN_Task_Test",
    .recurr_time = 0, // reoccur never
    .execute_time = 0, // execute immediately
};