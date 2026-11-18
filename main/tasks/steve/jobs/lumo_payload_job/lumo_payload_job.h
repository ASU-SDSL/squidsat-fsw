#pragma once

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "can.h"

typedef struct{
    can_message_t send;
    can_message_t reply;

    uint32_t request_time; 
    uint32_t start_time;
    bool payload_active; //check if payload is activate
    bool reply_recived; //check if message is recived
} payload_context_t;


void start_payload();
void payload_interface();
void store_mram();
void payload_status();

//can message functions
void can_dant_to_obc();
void can_obc_to_payload();
void can_payload_to_obc();
void can_obc_to_dant();