#ifndef CAN_TASK_H
#define CAN_TASK_H


#include <stdio.h>
 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "pico/stdlib.h"
 
#include "log.h"
#include "mcp2515.h"


#define CAN_QUEUE_LEN   10

int add_can_transmission();
int check_can_buff();


void can_task(void *pvParameters);

#endif