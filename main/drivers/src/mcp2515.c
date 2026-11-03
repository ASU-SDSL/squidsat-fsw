/**
 * @file mcp2515.c
 * @author Koowum Joshi
 * @brief Driver for MCP2515 CAN Controller Implementation
 * @version 1.0
 * 
 * @date 10/19/26
 */


#include <stdio.h>
#include <string.h>

#include "mcp2515.h"
#include "log.h"

#include "hardware/gpio.h"

#include "FreeRTOS.h"
#include "semphr.h"

static SemaphoreHandle_t spi_transmit_semphr;

/**
 * @brief High level macros and commands to send over SPI, everything has a brief explination
 */
#define MCP2515_RESET           0xC0 // used to reset internal registers and put into Config mode

#define MCP2515_READ            0x03 // Reads data from the register beginning at selected address

#define MCP2515_READ_RXB0_ID    0x90 // Read RX buffer 0 starting at RXB0SIDH (0x61): ID, DLC, then data
#define MCP2515_READ_RXB0_DATA  0x92 // Read RX buffer 0 starting at RXB0D0 (0x66): data bytes only
#define MCP2515_READ_RXB1_ID    0x94 // Read RX buffer 1 starting at RXB1SIDH (0x71): ID, DLC, then data
#define MCP2515_READ_RXB1_DATA  0x96 // Read RX buffer 1 starting at RXB1D0 (0x76): data bytes only

#define MCP2515_WRITE           0x02 // Writes data to the register beginning at the selected address


#define MCP2515_LOAD_TXB0_ID    0x40 // Load TX buffer 0 starting at TXB0SIDH (0x31): ID, DLC, then data
#define MCP2515_LOAD_TXB0_DATA  0x41 // Load TX buffer 0 starting at TXB0D0 (0x36): data bytes only
#define MCP2515_LOAD_TXB1_ID    0x42 // Load TX buffer 1 starting at TXB1SIDH (0x41): ID, DLC, then data
#define MCP2515_LOAD_TXB1_DATA  0x43 // Load TX buffer 1 starting at TXB1D0 (0x46): data bytes only
#define MCP2515_LOAD_TXB2_ID    0x44 // Load TX buffer 2 starting at TXB2SIDH (0x51): ID, DLC, then data
#define MCP2515_LOAD_TXB2_DATA  0x45 // Load TX buffer 2 starting at TXB2D0 (0x56): data bytes only


#define MCP2515_RTS_TXB0        0x81 // Request to send TX buffer 0
#define MCP2515_RTS_TXB1        0x82 // Request to send TX buffer 1
#define MCP2515_RTS_TXB2        0x84 // Request to send TX buffer 2


#define MCP2515_READ_STATUS     0xA0 // A quick poll to see status of transmission and recieve functions
#define MCP2515_RX_STATUS       0xB0 // A quick poll command to see the type of message being recieved

#define MCP2515_BIT_MODIFY      0x05 // Allows user to set/clear individual bits in a particular register

#define STATUS_RX0IF    0x01 // READ STATUS bit 0: frame waiting in RX buffer 0
#define STATUS_RX1IF    0x02 // READ STATUS bit 1: frame waiting in RX buffer 1
#define STATUS_TX0REQ   0x04 // READ STATUS bit 2: TX buffer 0 still waiting to send

#define SIDL_EXIDE      0x08 // bit 3 of TXBnSIDL / RXBnSIDL: 1 = extended ID
#define DLC_MASK        0x0F // data length lives in the low 4 bits of the DLC register

#define CAN_STD_ID_MAX  0x7FF // Max ID_NUM for normal CAN packets
#define CAN_EXT_ID_MAX  0x1FFFFFFF // Max ID_NUM for extended CAN packets

#define CANSTAT     0x0E // Operating-mode status (OPMOD bits 7:5) — poll to confirm a mode change
#define CANCTRL     0x0F // Mode request (REQOP bits 7:5), plus one-shot and CLKOUT control
#define CNF3        0x28 // Bit timing: phase segment 2 (PHSEG2)
#define CNF2        0x29 // Bit timing: propagation segment + phase segment 1, BTLMODE
#define CNF1        0x2A // Bit timing: baud-rate prescaler (BRP) + sync jump width (SJW)
#define RXB0CTRL    0x60 // RX buffer 0 control: receive mode / filter enable (RXM bits)
#define RXB1CTRL    0x70 // RX buffer 1 control: receive mode / filter enable (RXM bits)

#define MODE_MASK       0xE0 // Mask mode: frame accepted when (id & mask) == (filter & mask); mask bit 1 = must match, 0 = don't care
#define MODE_NORMAL     0x00 // Normal CAN operation
#define MODE_LOOPBACK   0x40 // Loopback mode allows you to witness your own packets that you send

#define TXB0CTRL 0x30 // TX buffer 0 control: TXREQ (pending), TXERR, MLOA (lost arbitration), ABTF (aborted)
#define TEC      0x1C // Transmit error counter: ≥96 warning, ≥128 error-passive, >255 bus-off
#define REC      0x1D // Receive error counter: ≥96 warning, ≥128 error-passive
#define EFLG     0x2D // Error flags: TXBO (bus-off), TXEP/RXEP (error-passive), TX/RXWAR, RX0/1OVR (overflow)

// Helper Functions

static void cs_pull_up(void){
    gpio_put(SPI_CS, 1);
}

static void cs_pull_down(void){
    gpio_put(SPI_CS, 0);
}

/**
 * @brief Atomically set/clear selected bits in an MCP2515 register (BIT MODIFY instruction).
 *        Only bits set in mask are changed
 *        Works only on bit-modifiable registers (e.g. CANCTRL, CNFx, CANINTF, EFLG, TXBnCTRL, RXBnCTRL).
 * @param addr Register address
 * @param mask Bits to change (1 = change, 0 = leave untouched)
 * @param data New values for the masked bits
 */

static void mcp_bit_modify(uint8_t addr, uint8_t mask, uint8_t data){
    uint8_t tx[4] = {MCP2515_BIT_MODIFY, addr, mask, data};
    if(xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY)){
        cs_pull_down();
        spi_write_blocking(SPI_PORT, tx, 4);
        cs_pull_up();
        xSemaphoreGive(spi_transmit_semphr);
    }
}

/**
 * @brief Quick poll of RX/TX buffer state via the READ STATUS instruction (one SPI transaction).
 *        bit0 RX0IF  - frame waiting in RX buffer 0
 *        bit1 RX1IF  - frame waiting in RX buffer 1
 *        bit2 TXB0 TXREQ - TX buffer 0 still pending
 *        bit3 TX0IF  - TX buffer 0 sent
 *        bit4 TXB1 TXREQ, bit5 TX1IF, bit6 TXB2 TXREQ, bit7 TX2IF
 * @return Status byte (0x00 if the SPI mutex could not be taken)
 */

static uint8_t mcp_read_status(void){
    uint8_t cmd = MCP2515_READ_STATUS;
    uint8_t val = 0;
    if(xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY)){
        cs_pull_down();
        spi_write_blocking(SPI_PORT, &cmd, 1);
        spi_read_blocking(SPI_PORT, 0x00, &val, 1);
        cs_pull_up();
        xSemaphoreGive(spi_transmit_semphr);
    }
    return val;
}

/**
 * @brief Software-reset the MCP2515 (RESET instruction): all registers return to defaults
 *        and the chip enters configuration mode, so CNFx/filters can be written.
 *        Waits 10 ms for the oscillator to restart before releasing the SPI bus.
 */

static void mcp_reset(void){
    uint8_t cmd = MCP2515_RESET;
    if(xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY)){
        cs_pull_down();
        spi_write_blocking(SPI_PORT, &cmd, 1);
        cs_pull_up();

        vTaskDelay(pdMS_TO_TICKS(10));
        xSemaphoreGive(spi_transmit_semphr);
    }
}

/**
 * @brief Read one MCP2515 register over SPI (READ instruction) and log the value (debug).
 * 
 * @param addr Register Address that we are attempting to read from
 * @return Register value (0x00 if the SPI mutex could not be taken)
 */

uint8_t mcp_read_reg(uint8_t addr){ 
    uint8_t rx_buf_packet[2] = {MCP2515_READ, addr};
    uint8_t val = 0;
    if(xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY)){
        cs_pull_down();
        spi_write_blocking(SPI_PORT, rx_buf_packet, 2);
        spi_read_blocking(SPI_PORT, 0x00, &val, 1);
        cs_pull_up();

        xSemaphoreGive(spi_transmit_semphr);
    }
    
    log_infof("Data: %d", val);

    return val;
}

/**
 * @brief Similar to the mcp_read_reg, but we do not print out the value of the register.
 *        Useful for final code.
 * 
 * @param addr Register Address that we are attempting to read from
 * @return Register value (0x00 if the SPI mutex could not be taken)
 */

static uint8_t mcp_read_quiet(uint8_t addr){
    uint8_t tx[2] = {MCP2515_READ, addr}, val = 0;
    if(xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY)){
        cs_pull_down();
        spi_write_blocking(SPI_PORT, tx, 2);
        spi_read_blocking(SPI_PORT, 0x00, &val, 1);
        cs_pull_up();
        xSemaphoreGive(spi_transmit_semphr);
    }
    return val;
}

/**
 * @brief Write one byte to an MCP2515 register over SPI (WRITE instruction), without logging or read-back.
 * 
 * @param addr Register Address that we are attempting to write to
 * @param data Data that we are attempting to write
 */

static void mcp_write_quiet(uint8_t addr, uint8_t data){
    uint8_t tx[3] = {MCP2515_WRITE, addr, data};
    if(xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY)){
        cs_pull_down();
        spi_write_blocking(SPI_PORT, tx, 3);
        cs_pull_up();
        xSemaphoreGive(spi_transmit_semphr);
    }
}

/**
 * @brief Check whether TX buffer 0 still has a frame waiting to be sent or acknowledged (TXREQ set).
 *
 * @return true if TXB0 is busy (send pending or retrying), false if it is free for a new frame
 */

bool mcp2515_tx_busy(void){
    return (mcp_read_status() & STATUS_TX0REQ) != 0;
}

/**
 * @brief Logging for the MCP2515
 */

void mcp2515_log_tx_status(void){
    log_infof("CAN TX status: TXB0CTRL=0x%02X TEC=%u REC=%u EFLG=0x%02X",
              mcp_read_quiet(TXB0CTRL), mcp_read_quiet(TEC),
              mcp_read_quiet(REC), mcp_read_quiet(EFLG));
}

/**
 * @brief Initialize the Pico SPI peripheral and pins for the MCP2515:
 *        CLK_SPEED baud, SPI mode 0 (CPOL=0, CPHA=0), 8-bit MSB-first,
 *        and chip select driven manually as a GPIO, idling high (deselected).
 */

void mcp2515_spi_init(void){
    spi_init(SPI_PORT, CLK_SPEED); // MCP --> Pico communication speed over SPI, NOT CAN speed
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(SPI_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MOSI, GPIO_FUNC_SPI);


    gpio_init(SPI_CS); // Init Chip select
    gpio_set_dir(SPI_CS, GPIO_OUT);
    gpio_put(SPI_CS, 1); // MCP is active low
}

/**
 * @brief Bring up the MCP2515: reset it, verify SPI comms, set 50 kbit/s bit timing,
 *        open both RX buffers to all frames, then switch to normal mode.
 *
 * @return 1 on success (chip in normal mode), 0 on any failure (caller should retry)
 */

int mcp2515_init(void){
    mcp2515_spi_init();
    if (spi_transmit_semphr == NULL) spi_transmit_semphr = xSemaphoreCreateMutex();

    mcp_reset();

    // Reset defaults prove the chip is alive, independent of our writes
    uint8_t ctrl = mcp_read_quiet(CANCTRL), stat = mcp_read_quiet(CANSTAT);
    log_infof("MCP after reset: CANCTRL=0x%02X (exp 0x87) CANSTAT=0x%02X (exp 0x80)", ctrl, stat);
    if(ctrl != 0x87 || (stat & MODE_MASK) != 0x80){
        // log_error("MCP2515 not answering on SPI: check wiring, RST tied high, 3V/GND");
        return 0;
    }

    // this is setting the bit rate to 50 kbps (matching what we have on Zephyr)
    mcp_write_quiet(CNF1, 0x09);
    mcp_write_quiet(CNF2, 0xB5);
    mcp_write_quiet(CNF3, 0x01);
    mcp_write_quiet(RXB0CTRL, 0x60);
    mcp_write_quiet(RXB1CTRL, 0x60);

    // Confirming that everything was initialized correctly
    uint8_t c1 = mcp_read_quiet(CNF1), c2 = mcp_read_quiet(CNF2), c3 = mcp_read_quiet(CNF3);
    log_infof("MCP CNF1=0x%02X CNF2=0x%02X CNF3=0x%02X (exp 09 B5 01)", c1, c2, c3);
    if(c1 != 0x09 || c2 != 0xB5 || c3 != 0x01){
        log_error("MCP2515 CNF readback mismatch");
        return 0;
    }

    // turn to normal operations
    mcp_write_quiet(CANCTRL, MODE_NORMAL);
    for(int i = 0; i < 10; i++){
        stat = mcp_read_quiet(CANSTAT);
        if((stat & MODE_MASK) == MODE_NORMAL){
            log_info("MCP2515 in normal mode, 50 kbit/s");
            return 1;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    log_infof("MCP2515 refused normal mode, CANSTAT=0x%02X", stat);
    return 0;
}

/**
 * @brief Load one CAN frame into MCP2515 TX buffer 0 and request transmission.
 *        Non-blocking: returns as soon as the frame is handed to the chip. A return of 0 means
 *        "queued on the MCP2515", not "delivered". Use mcp2515_tx_busy() to see when it's ACKed.
 *
 * @param m Frame to send (id, extended flag, dlc 0-8, data)
 * @return 0 if the frame was loaded and transmission requested,
 *         -1 if the frame is invalid or TX buffer 0 is still busy with a previous frame
 */

int mcp2515_send(const can_message_t *m){
    // Reject malformed frames: classic CAN carries at most 8 data bytes,
    // standard IDs are 11 bits (<= 0x7FF), extended IDs are 29 bits (<= 0x1FFFFFFF)
    if(m->dlc > 8) return -1;
    if(!m->extended && m->id > CAN_STD_ID_MAX) return -1;
    if( m->extended && m->id > CAN_EXT_ID_MAX) return -1;

    // Only TX buffer 0 is used; if its previous frame is still pending (not yet ACKed),
    // don't overwrite it. The caller keeps the frame queued and retries later
    if(mcp_read_status() & STATUS_TX0REQ) return -1;

    // SPI packet for LOAD TX BUFFER: [cmd][SIDH][SIDL][EID8][EID0][DLC][D0..D7] = up to 14 bytes
    // The LOAD instruction starts writing at TXB0SIDH and auto-increments through the buffer
    uint8_t tx[14];
    tx[0] = MCP2515_LOAD_TXB0_ID;

    if(m->extended){
        // The MCP2515 stores a 29-bit ID in four 8-bit registers, so we chop the ID into pieces:
        //   tx[1] SIDH: the top 8 bits of the ID
        //   tx[2] SIDL: the next 3 bits, plus a flag saying "this is an extended ID",
        //               plus 2 more ID bits (the chip's register layout puts them in odd spots)
        //   tx[3] EID8: the next 8 bits
        //   tx[4] EID0: the last (lowest) 8 bits
        // Example: 0x18CE0102 -> SIDH=0xC6, SIDL=0x0A, EID8=0x01, EID0=0x02        tx[1] = (uint8_t)(m->id >> 21);
        tx[2] = (uint8_t)((((m->id >> 18) & 0x07) << 5) | SIDL_EXIDE | ((m->id >> 16) & 0x03));
        tx[3] = (uint8_t)(m->id >> 8);
        tx[4] = (uint8_t)(m->id);
    }else{
        // 11-bit ID: SIDH = ID[10:3], SIDL = ID[2:0] in bits 7:5 (EXIDE = 0), extended bytes unused
        tx[1] = (uint8_t)(m->id >> 3);
        tx[2] = (uint8_t)((m->id & 0x07) << 5);
        tx[3] = 0;
        tx[4] = 0;
    }

    // DLC register: data length in bits 3:0 (RTR bit left 0 = data frame), then the payload bytes
    tx[5] = m->dlc & DLC_MASK;
    memcpy(&tx[6], m->data, m->dlc);

    // RTS (Request To Send) for TX buffer 0: tells the MCP2515 to start transmitting on the bus
    uint8_t rts = MCP2515_RTS_TXB0;

    // Hold the SPI mutex across both transactions so no other task touches the chip in between.
    // Each instruction needs its own CS low/high cycle: first load the buffer, then send RTS
    if(xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY)){
        cs_pull_down();
        spi_write_blocking(SPI_PORT, tx, 6 + m->dlc);   // header (6 bytes) + only the used data bytes
        cs_pull_up();

        cs_pull_down();
        spi_write_blocking(SPI_PORT, &rts, 1);
        cs_pull_up();

        xSemaphoreGive(spi_transmit_semphr);
    }

    // From here the MCP2515 handles arbitration, retransmission and ACK on its own;
    // TXREQ stays set until a node ACKs the frame (see mcp2515_tx_busy / mcp2515_log_tx_status)
    return 0;
}

/**
 * @brief Check the MCP2515 for a received frame and, if one is waiting, copy it into *m.
 *        Non-blocking: returns immediately if nothing has arrived.
 *
 * @param m Where to store the received frame (id, extended flag, dlc, data)
 * @return 1 if a frame was read into *m, 0 if no frame was waiting
 */

int mcp2515_receive(can_message_t *m){

    // Quick poll: are either of the two receive buffers holding a new frame?
    uint8_t status = mcp_read_status();
    uint8_t cmd;

    // Pick which buffer to read (buffer 0 first). Nothing waiting -> nothing to do
    if(status & STATUS_RX0IF)       cmd = MCP2515_READ_RXB0_ID;
    else if(status & STATUS_RX1IF)  cmd = MCP2515_READ_RXB1_ID;
    else return 0;

    // Read the whole frame in one go: 4 ID bytes + 1 length byte + 8 data bytes = 13 bytes.
    // Using the READ RX BUFFER instruction also tells the chip "this buffer is free again"
    // when CS goes high, so no separate flag-clear is needed
    uint8_t rx[13] = {0};
    if(xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY)){
        cs_pull_down();
        spi_write_blocking(SPI_PORT, &cmd, 1);
        spi_read_blocking(SPI_PORT, 0x00, rx, 13);
        cs_pull_up();

        xSemaphoreGive(spi_transmit_semphr);
    }

    // Glue the ID back together (the reverse of what mcp2515_send does).
    // The "extended ID" flag in rx[1] tells us whether it's a 29-bit or 11-bit ID
    m->extended = (rx[1] & SIDL_EXIDE) ? 1 : 0;
    if(m->extended){
        // 29-bit ID: rx[0] is the top 8 bits, rx[1] holds 3 + 2 bits in odd spots,
        // rx[2] the next 8 bits, rx[3] the lowest 8 bits
        m->id = ((uint32_t)rx[0] << 21)
              | ((uint32_t)(rx[1] >> 5) << 18)
              | ((uint32_t)(rx[1] & 0x03) << 16)
              | ((uint32_t)rx[2] << 8)
              |  (uint32_t)rx[3];
    }else{
        // 11-bit ID: rx[0] is the top 8 bits, the top 3 bits of rx[1] are the rest
        m->id = ((uint32_t)rx[0] << 3) | (rx[1] >> 5);
    }

    // Data length is the low 4 bits of rx[4]; cap at 8 in case the chip reports more
    m->dlc = rx[4] & DLC_MASK;
    if(m->dlc > 8) m->dlc = 8;

    // Clear old data first so unused bytes are 0, then copy only the bytes that arrived
    memset(m->data, 0, sizeof(m->data));
    memcpy(m->data, &rx[5], m->dlc);

    return 1;
}