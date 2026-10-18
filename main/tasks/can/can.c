#include "can.h"
#include "usb_serial.h"

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
volatile int can_task_state = 0;


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

int check_can_buff(can_message_t *msg){
    return (xQueueReceive(can_rx_queue, msg, 0) == pdTRUE) ? 1 : 0;
}

void can_task(void *pvParameters){
    can_task_state = 1;
    while(!safe_tud_cdc_connected()){
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    can_task_state = 2;
    while(mcp2515_init() != 1){
        can_task_state = 3;
        log_error("CAN: MCP2515 init failed, retrying in 1 s");
        vTaskDelay(pdMS_TO_TICKS(1000));
        can_task_state = 2;
    }
    log_info("CAN: MCP2515 ready");
    can_task_state = 4;
    can_message_t msg;
    bool tx_pending = false;
    TickType_t tx_start = 0, last_report = 0;

    for(;;){
        while(mcp2515_receive(&msg) == 1){
            log_infof("CAN RX id=0x%08lX dlc=%u d0=0x%02X d1=0x%02X",
                      (unsigned long)msg.id, msg.dlc, msg.data[0], msg.data[1]);
            if(xQueueSend(can_rx_queue, &msg, 0) != pdTRUE){
                log_warning("CAN: RX queue full, frame dropped");
            }
        }

        TickType_t now = xTaskGetTickCount();

        if(tx_pending){
            if(!mcp2515_tx_busy()){
                tx_pending = false;
                log_info("CAN TX complete: frame was ACKed");
            } else if(now - tx_start > pdMS_TO_TICKS(500) && now - last_report > pdMS_TO_TICKS(1000)){
                last_report = now;
                mcp2515_log_tx_status();       // stuck: nobody is ACKing
            }
        } else if(xQueuePeek(can_tx_queue, &msg, 0) == pdTRUE){
            if(mcp2515_send(&msg) == 0){
                xQueueReceive(can_tx_queue, &msg, 0);
                tx_pending = true;
                tx_start = now;
                log_infof("CAN TX loaded id=0x%08lX", (unsigned long)msg.id);
            }
        }

        vTaskDelay(1);
    }
}