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
    static uint32_t seq = 0;

    LinkMessage msg = LinkMessage_init_zero;
    msg.priority    = 6;                                   // low priority test traffic
    msg.node_id     = OBC_ADDR;                            // 0x01, us
    msg.target_node = EPS_ADDR;                            // 0x02, STM32
    msg.seq         = seq++;
    msg.uptime_ms   = to_ms_since_boot(get_absolute_time());
    msg.type        = LinkMessage_MsgType_HEARTBEAT;

    if(send_link_message(&msg) != 0){
        log_warning("can_test: LinkMessage not queued");
    }

    log_info("CAN_TEST SUCESSFULLY SENT");
}

jobs_t can_job = {
    .args = NULL,
    .func = can_test_frame,
    .name = "CAN_Task_Test",
    .recurr_time = 1000, // reoccur never
    .execute_time = 0, // execute immediately
};