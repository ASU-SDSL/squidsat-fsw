/**
 * @file can.c
 * @author Koowum Joshi
 * @brief Can Implementation through MCP2515 Can Controller and TJA1050 CAN Tranciever
 * @date 10/19/2026
 * 
 * @version 1.0
 */


#include "can.h"
#include "usb_serial.h"

static QueueHandle_t can_tx_queue; // frames waiting to be sent
static QueueHandle_t can_rx_queue; // frames received off the bus
volatile int can_task_state = 0; // Helper state to make sure startup completes properly

/**
 * @brief Initializes the tx/rx buffer and confirms that the queues are operating properly
 */

void can_init(void){
    can_tx_queue = xQueueCreate(CAN_QUEUE_LEN, sizeof(can_message_t));
    can_rx_queue = xQueueCreate(CAN_QUEUE_LEN, sizeof(can_message_t));
    configASSERT(can_tx_queue != NULL && can_rx_queue != NULL);
}

/**
 * @brief Queue a CAN frame for transmission. Safe to call from any task (e.g. STEVE jobs).
 *        The frame is copied into the TX queue; can_task sends it when the MCP2515 is free.
 *        Waits up to 10 ms for space if the queue is full.
 *
 * @param msg Frame to send (copied, so the caller's struct can be reused right away)
 * @return 0 if queued, -1 if the queue stayed full (frame dropped)
 */

int add_can_transmission(const can_message_t *msg){
    if(xQueueSend(can_tx_queue, msg, pdMS_TO_TICKS(10)) != pdTRUE){
        log_warning("CAN TX queue full, frame not queued");
        return -1;
    }
    return 0;
}

/**
 * @brief Take the oldest received CAN frame out of the RX queue, if there is one.
 *        Non-blocking: returns immediately when nothing has been received.
 *
 * @param msg Where to copy the received frame
 * @return 1 if a frame was copied into msg, 0 if the RX queue was empty
 */

int check_can_buff(can_message_t *msg){
    return (xQueueReceive(can_rx_queue, msg, 0) == pdTRUE) ? 1 : 0;
}

/**
 * @brief FreeRTOS task that owns the MCP2515.
 *        1. Waits for a serial monitor so init logs aren't lost
 *        2. Initializes the MCP2515, retrying every second until it succeeds
 *        3. Loops forever: moves received frames into can_rx_queue and sends
 *           queued frames from can_tx_queue, one at a time
 *
 *        can_task_state (readable with the GSE "can_state" command):
 *        1 = waiting for USB, 2 = initializing, 3 = init failed (retrying), 4 = running
 */

void can_task(void *pvParameters){
    // Hold off until a serial monitor is connected, otherwise init logs are dropped
    can_task_state = 1;
    while(!safe_tud_cdc_connected()){
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Bring up the MCP2515; keep retrying (e.g. board unplugged or still powering up)
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
    bool tx_pending = false;                 // true while a frame sits in the MCP2515 waiting for an ACK
    TickType_t tx_start = 0, last_report = 0; // when that frame was loaded / when we last logged its status

    for(;;){
        // ---- Receive: drain every frame the MCP2515 is holding into the RX queue ----
        while(mcp2515_receive(&msg) == 1){
            log_infof("CAN RX id=0x%08lX dlc=%u d0=0x%02X d1=0x%02X",
                      (unsigned long)msg.id, msg.dlc, msg.data[0], msg.data[1]);
            if(xQueueSend(can_rx_queue, &msg, 0) != pdTRUE){
                log_warning("CAN: RX queue full, frame dropped");   // nobody is reading check_can_buff()
            }
        }

        TickType_t now = xTaskGetTickCount();

        // ---- Transmit: only one frame in flight at a time ----
        if(tx_pending){
            // A frame was handed to the MCP2515 earlier; see if another node has ACKed it yet
            if(!mcp2515_tx_busy()){
                tx_pending = false;
                log_info("CAN TX complete: frame was ACKed");
            } else if(now - tx_start > pdMS_TO_TICKS(500) && now - last_report > pdMS_TO_TICKS(1000)){
                // Still not ACKed after 500 ms: nobody on the bus is answering.
                // Log the error registers once a second (TEC stuck at 128 = no ACK)
                last_report = now;
                mcp2515_log_tx_status();
            }
        } else if(xQueuePeek(can_tx_queue, &msg, 0) == pdTRUE){
            // Nothing in flight and a frame is waiting: try to load it into the MCP2515.
            // Peek first and only remove it from the queue once the chip accepted it,
            // so the frame isn't lost if TX buffer 0 happens to be busy
            if(mcp2515_send(&msg) == 0){
                xQueueReceive(can_tx_queue, &msg, 0);
                tx_pending = true;
                tx_start = now;
                log_infof("CAN TX loaded id=0x%08lX", (unsigned long)msg.id);
            }
        }

        // Yield for 1 tick (1 ms) so other tasks run; also sets the polling rate
        vTaskDelay(1);
    }
}