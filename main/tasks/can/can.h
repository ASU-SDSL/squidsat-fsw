#ifndef CAN_TASK_H
#define CAN_TASK_H


#include <stdio.h>
 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "pico/stdlib.h"
 
#include "log.h"
#include "mcp2515.h"
#include "isotp.h"
#include "link.pb.h"


#define CAN_QUEUE_LEN           10
#define CAN_ISOTP_MAX_LEN       256
#define CAN_ISOTP_QUEUE_LEN     4       

#define OBC_ADDR                0x02
#define EPS_ADDR                0x01


void can_init(void);
int add_can_transmission();
int check_can_buff();

int add_can_transmission(const can_message_t *msg);
int check_can_buff(can_message_t *msg);

int send_link_message(const LinkMessage *msg);
int check_link_message(LinkMessage *msg);


void can_task(void *pvParameters);

extern volatile int can_task_state;

#endif