#include "can.h"

// MCP2515 registers the task needs to finish setup
#define CANSTAT     0x0E
#define CANCTRL     0x0F
#define CNF3        0x28
#define CNF2        0x29
#define CNF1        0x2A
#define RXB0CTRL    0x60
#define RXB1CTRL    0x70

#define MODE_MASK       0xE0
#define MODE_LOOPBACK   0x40 // change to 0x00 for normal mode once a second node is on the bus

static QueueHandle_t can_tx_queue; // frames waiting to be sent
static QueueHandle_t can_rx_queue; // frames received off the bus


void can_init(void){
    can_tx_queue = xQueueCreate(CAN_QUEUE_LEN, sizeof(can_message_t));
    can_rx_queue = xQueueCreate(CAN_QUEUE_LEN, sizeof(can_message_t));
    configASSERT(can_tx_queue != NULL && can_rx_queue != NULL);
}

int add_can_transmission(const can_message_t *msg){
    if(xQueueSend(can_tx_queue, msg, pdMS_TO_TICKS(10)) != pdTRUE){
        log_warning("CAN TX queue full, frame not queued");
        return -1;
    }
    return 0;
}