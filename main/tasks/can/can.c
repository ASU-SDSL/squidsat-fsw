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
#include <pb_encode.h>
#include <pb_decode.h>


static QueueHandle_t isotp_tx_queue;
static QueueHandle_t isotp_rx_queue;
static QueueHandle_t can_tx_queue; // frames waiting to be sent
static QueueHandle_t can_rx_queue; // frames received off the bus
volatile int can_task_state = 0; // Helper state to make sure startup completes properly

static IsoTpLink tx_link, rx_link;
static uint8_t tx_link_buf[CAN_ISOTP_MAX_LEN], tx_link_unused[8];
static uint8_t rx_link_buf[CAN_ISOTP_MAX_LEN], rx_link_unused[8];

#define ID_DATA(dst, src) (0x18CE0000u | ((uint32_t)(dst) << 8) | (uint32_t)(src))   // data frames
#define ID_FC(dst, src)   (0x18CF0000u | ((uint32_t)(dst) << 8) | (uint32_t)(src))   // flow control

typedef struct {
    uint16_t len;
    uint8_t  data[CAN_ISOTP_MAX_LEN];
} can_isotp_msg_t;


/* ======================= isotp-c platform hooks ======================= */

/* Only ever called from inside can_task (isotp_poll / isotp_on_can_message) */

/**
 * @brief Hand one ISO-TP-built CAN frame to the TX queue.
 * @return ISOTP_RET_OK if queued, ISOTP_RET_ERROR if the queue is full
 */
int isotp_user_send_can(const uint32_t arbitration_id, const uint8_t *data, const uint8_t size){
    can_message_t f = { .id = arbitration_id, .extended = 1, .dlc = size };
    memcpy(f.data, data, size);
    return (xQueueSend(can_tx_queue, &f, 0) == pdTRUE) ? ISOTP_RET_OK : ISOTP_RET_ERROR;
}

/**
 * @brief Millisecond clock for isotp-c timeouts and STmin.
 */
uint32_t isotp_user_get_ms(void){
    return to_ms_since_boot(get_absolute_time());
}

/**
 * @brief Route isotp-c debug messages into the logger.
 */
void isotp_user_debug(const char *message, ...){
    char buf[128];
    va_list args;
    va_start(args, message);
    vsnprintf(buf, sizeof buf, message, args);
    va_end(args);
    log_info(buf);
}

/* ======================= can helper functions ======================= */

/**
 * @brief Initializes the tx/rx buffer and confirms that the queues are operating properly
 */

void can_init(void){
    isotp_tx_queue = xQueueCreate(CAN_ISOTP_QUEUE_LEN, sizeof(can_isotp_msg_t));
    isotp_rx_queue = xQueueCreate(CAN_ISOTP_QUEUE_LEN, sizeof(can_isotp_msg_t));

    can_tx_queue = xQueueCreate(CAN_QUEUE_LEN, sizeof(can_message_t));
    can_rx_queue = xQueueCreate(CAN_QUEUE_LEN, sizeof(can_message_t));

    isotp_init_link(&tx_link, ID_DATA(EPS_ADDR, OBC_ADDR),
                    tx_link_buf, sizeof tx_link_buf, tx_link_unused, sizeof tx_link_unused);
    isotp_init_link(&rx_link, ID_FC(EPS_ADDR, OBC_ADDR),
                    rx_link_unused, sizeof rx_link_unused, rx_link_buf, sizeof rx_link_buf);

    configASSERT(can_tx_queue && can_rx_queue && isotp_tx_queue && isotp_rx_queue);
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

int send_link_message(const LinkMessage *msg){
    can_isotp_msg_t out;
    pb_ostream_t stream = pb_ostream_from_buffer(out.data, sizeof out.data);
    if(!pb_encode(&stream, LinkMessage_fields, msg)){
        log_infof("LinkMessage encode failed: %s", PB_GET_ERROR(&stream));
        return -1;
    }
    out.len = (uint16_t)stream.bytes_written;
    if(xQueueSend(isotp_tx_queue, &out, 0) != pdTRUE){
        log_warning("ISO-TP TX queue full, message not queued");
        return -1;
    }
    return 0;
}

int check_link_message(LinkMessage *msg){
    can_isotp_msg_t in;
    if(xQueueReceive(isotp_rx_queue, &in, 0) != pdTRUE) return 0;
    *msg = (LinkMessage)LinkMessage_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(in.data, in.len);
    if(!pb_decode(&stream, LinkMessage_fields, msg)){
        log_infof("LinkMessage decode failed: %s", PB_GET_ERROR(&stream));
        return -1;
    }
    return 1;
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
    (void)pvParameters;

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

    can_message_t frame;
    bool frame_pending = false;                 // a frame sits in the MCP2515 waiting for an ACK
    TickType_t frame_start = 0, last_report = 0;
    static can_isotp_msg_t msg_out, msg_in;     // static: 258 B each, keep them off the task stack

    for(;;){
        // ---- Receive: route each frame to the right ISO-TP link ----
        while(mcp2515_receive(&frame) == 1){
            if(frame.extended && frame.id == ID_FC(OBC_ADDR, EPS_ADDR)){
                isotp_on_can_message(&tx_link, frame.data, frame.dlc);   // STM32's FC for our send
            } else if(frame.extended && frame.id == ID_DATA(OBC_ADDR, EPS_ADDR)){
                isotp_on_can_message(&rx_link, frame.data, frame.dlc);   // STM32 sending to us
            } else {
                log_infof("CAN: ignoring frame id=0x%08lX dlc=%u", (unsigned long)frame.id, frame.dlc);
            }
        }

        TickType_t now = xTaskGetTickCount();

        // ---- Frame transmit: one frame in the MCP2515 at a time, wait for its ACK ----
        if(frame_pending){
            if(!mcp2515_tx_busy()){
                frame_pending = false;
            } else if(now - frame_start > pdMS_TO_TICKS(500) && now - last_report > pdMS_TO_TICKS(1000)){
                last_report = now;
                mcp2515_log_tx_status();            // TEC stuck at 128 = nobody ACKing
            }
        } else if(xQueuePeek(can_tx_queue, &frame, 0) == pdTRUE){
            // Only remove the frame once the chip accepted it
            if(mcp2515_send(&frame) == 0){
                xQueueReceive(can_tx_queue, &frame, 0);
                frame_pending = true;
                frame_start = now;
            }
        }

        // ---- ISO-TP transmit ----
        if(tx_link.send_status == ISOTP_SEND_STATUS_ERROR){
            log_infof("ISO-TP send failed, result=%d", tx_link.send_protocol_result);
            tx_link.send_status = ISOTP_SEND_STATUS_IDLE;
            xQueueReset(can_tx_queue);
        }
        if(tx_link.send_status != ISOTP_SEND_STATUS_INPROGRESS &&
           xQueueReceive(isotp_tx_queue, &msg_out, 0) == pdTRUE){
            // < 8 bytes -> single frame; otherwise first frame, then wait for the STM32's FC
            int r = isotp_send(&tx_link, msg_out.data, msg_out.len);
            if(r != ISOTP_RET_OK) log_infof("isotp_send returned %d", r);
        }

        // Next consecutive frame only if the frame queue has room (a failed send aborts the transfer)
        if(uxQueueSpacesAvailable(can_tx_queue) > 0) isotp_poll(&tx_link);
        isotp_poll(&rx_link);

        // ---- ISO-TP receive: a complete message arrived ----
        if(isotp_receive(&rx_link, msg_in.data, sizeof msg_in.data, &msg_in.len) == ISOTP_RET_OK){
            LinkMessage dbg = LinkMessage_init_zero;
            pb_istream_t s = pb_istream_from_buffer(msg_in.data, msg_in.len);
            if(pb_decode(&s, LinkMessage_fields, &dbg)){
                log_infof("CAN RX %u B: node %lu -> %lu seq=%lu type=%d prio=%lu uptime=%lu",
                          msg_in.len,
                          (unsigned long)dbg.node_id, (unsigned long)dbg.target_node,
                          (unsigned long)dbg.seq, (int)dbg.type,
                          (unsigned long)dbg.priority, (unsigned long)dbg.uptime_ms);
            } else {
                log_infof("CAN RX %u B: protobuf decode failed: %s", msg_in.len, PB_GET_ERROR(&s));
            }

            if(xQueueSend(isotp_rx_queue, &msg_in, 0) != pdTRUE){
                log_warning("CAN: message RX queue full, LinkMessage dropped");
            }
        }

        vTaskDelay(1);
    }
}