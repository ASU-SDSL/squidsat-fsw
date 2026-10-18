#include <stdio.h>
#include <string.h>

#include "mcp2515.h"
#include "log.h"

#include "hardware/spi.h"
#include "hardware/gpio.h"

#include "FreeRTOS.h"
#include "semphr.h"

/**
 * @author Koowum Joshi
 * @brief Driver for MCP2515 CAN Controller Implementation
 * @version 0.1
 */

static SemaphoreHandle_t spi_transmit_semphr;

/**
 * @brief
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

#define CANSTAT     0x0E
#define CANCTRL     0x0F
#define CNF3        0x28
#define CNF2        0x29
#define CNF1        0x2A
#define RXB0CTRL    0x60
#define RXB1CTRL    0x70

#define MODE_MASK       0xE0
#define MODE_NORMAL     0x00
#define MODE_LOOPBACK   0x40

#define TXB0CTRL 0x30
#define TEC      0x1C
#define REC      0x1D
#define EFLG     0x2D


// Helper Functions

static void cs_pull_up(void){
    gpio_put(SPI_CS, 1);
}

static void cs_pull_down(void){
    gpio_put(SPI_CS, 0);
}

static void mcp_bit_modify(uint8_t addr, uint8_t mask, uint8_t data){
    uint8_t tx[4] = {MCP2515_BIT_MODIFY, addr, mask, data};
    if(xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY)){
        cs_pull_down();
        spi_write_blocking(SPI_PORT, tx, 4);
        cs_pull_up();
        xSemaphoreGive(spi_transmit_semphr);
    }
}

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

static void mcp_reset(void){
    uint8_t cmd = MCP2515_RESET;
    if(xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY)){
        cs_pull_down();
        spi_write_blocking(spi1, &cmd, 1);
        cs_pull_up();

        vTaskDelay(pdMS_TO_TICKS(10));
        xSemaphoreGive(spi_transmit_semphr);
    }
}

uint8_t mcp_read_reg(uint8_t addr){ 
    uint8_t rx_buf_packet[2] = {MCP2515_READ, addr};
    uint8_t val = 0;
    if(xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY)){
        cs_pull_down();
        spi_write_blocking(spi1, rx_buf_packet, 2);
        spi_read_blocking(spi1, 0x00, &val, 1);
        cs_pull_up();

        xSemaphoreGive(spi_transmit_semphr);
    }
    
    char msg[32];

    snprintf(msg, sizeof(msg), "Data: %d", val);
    log_info(msg);

    return val;
}

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

static void mcp_write_quiet(uint8_t addr, uint8_t data){
    uint8_t tx[3] = {MCP2515_WRITE, addr, data};
    if(xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY)){
        cs_pull_down();
        spi_write_blocking(SPI_PORT, tx, 3);
        cs_pull_up();
        xSemaphoreGive(spi_transmit_semphr);
    }
}

bool mcp2515_tx_busy(void){
    return (mcp_read_status() & STATUS_TX0REQ) != 0;
}

void mcp2515_log_tx_status(void){
    log_infof("CAN TX status: TXB0CTRL=0x%02X TEC=%u REC=%u EFLG=0x%02X",
              mcp_read_quiet(TXB0CTRL), mcp_read_quiet(TEC),
              mcp_read_quiet(REC), mcp_read_quiet(EFLG));
}


void mcp2515_spi_init(void){
    spi_init(spi1, CLK_SPEED); // MCP --> Pico communication speed over SPI, NOT CAN speed
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(SPI_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MOSI, GPIO_FUNC_SPI);


    gpio_init(SPI_CS); // Init Chip select
    gpio_set_dir(SPI_CS, GPIO_OUT);
    gpio_put(SPI_CS, 1); // MCP is active low
}


int mcp2515_init(void){
    mcp2515_spi_init();
    if (spi_transmit_semphr == NULL) spi_transmit_semphr = xSemaphoreCreateMutex();

    mcp_reset();

    // Reset defaults prove the chip is alive, independent of our writes
    uint8_t ctrl = mcp_read_quiet(CANCTRL), stat = mcp_read_quiet(CANSTAT);
    log_infof("MCP after reset: CANCTRL=0x%02X (exp 0x87) CANSTAT=0x%02X (exp 0x80)", ctrl, stat);
    if(ctrl != 0x87 || (stat & MODE_MASK) != 0x80){
        log_error("MCP2515 not answering on SPI: check wiring, RST tied high, 3V/GND");
        return 0;
    }

    mcp_write_quiet(CNF1, 0x09);
    mcp_write_quiet(CNF2, 0xB5);
    mcp_write_quiet(CNF3, 0x01);
    mcp_write_quiet(RXB0CTRL, 0x60);
    mcp_write_quiet(RXB1CTRL, 0x60);

    uint8_t c1 = mcp_read_quiet(CNF1), c2 = mcp_read_quiet(CNF2), c3 = mcp_read_quiet(CNF3);
    log_infof("MCP CNF1=0x%02X CNF2=0x%02X CNF3=0x%02X (exp 09 B5 01)", c1, c2, c3);
    if(c1 != 0x09 || c2 != 0xB5 || c3 != 0x01){
        log_error("MCP2515 CNF readback mismatch");
        return 0;
    }

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


int mcp2515_send(const can_message_t *m){
    if(m->dlc > 8) return -1;
    if(!m->extended && m->id > CAN_STD_ID_MAX) return -1;
    if( m->extended && m->id > CAN_EXT_ID_MAX) return -1;

    if(mcp_read_status() & STATUS_TX0REQ) return -1;

    uint8_t tx[14];
    tx[0] = MCP2515_LOAD_TXB0_ID;

    if(m->extended){
        tx[1] = (uint8_t)(m->id >> 21);
        tx[2] = (uint8_t)((((m->id >> 18) & 0x07) << 5) | SIDL_EXIDE | ((m->id >> 16) & 0x03));
        tx[3] = (uint8_t)(m->id >> 8);
        tx[4] = (uint8_t)(m->id);
    }else{
        tx[1] = (uint8_t)(m->id >> 3);
        tx[2] = (uint8_t)((m->id & 0x07) << 5);
        tx[3] = 0;
        tx[4] = 0;
    }

    tx[5] = m->dlc & DLC_MASK;
    memcpy(&tx[6], m->data, m->dlc);

    uint8_t rts = MCP2515_RTS_TXB0;

    if(xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY)){
        cs_pull_down();
        spi_write_blocking(SPI_PORT, tx, 6 + m->dlc);
        cs_pull_up();

        cs_pull_down();
        spi_write_blocking(SPI_PORT, &rts, 1);
        cs_pull_up();

        xSemaphoreGive(spi_transmit_semphr);
    }
    return 0;
}

int mcp2515_receive(can_message_t *m){
    
    uint8_t status = mcp_read_status();
    uint8_t cmd;

    if(status & STATUS_RX0IF)       cmd = MCP2515_READ_RXB0_ID;
    else if(status & STATUS_RX1IF)  cmd = MCP2515_READ_RXB1_ID;
    else return 0;

    uint8_t rx[13] = {0};
    if(xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY)){
        cs_pull_down();
        spi_write_blocking(SPI_PORT, &cmd, 1);
        spi_read_blocking(SPI_PORT, 0x00, rx, 13);
        cs_pull_up();

        xSemaphoreGive(spi_transmit_semphr);
    }

    
    m->extended = (rx[1] & SIDL_EXIDE) ? 1 : 0;
    if(m->extended){
        m->id = ((uint32_t)rx[0] << 21)
              | ((uint32_t)(rx[1] >> 5) << 18)
              | ((uint32_t)(rx[1] & 0x03) << 16)
              | ((uint32_t)rx[2] << 8)
              |  (uint32_t)rx[3];
    }else{
        m->id = ((uint32_t)rx[0] << 3) | (rx[1] >> 5);
    }


    m->dlc = rx[4] & DLC_MASK;
    if(m->dlc > 8) m->dlc = 8;
    memset(m->data, 0, sizeof(m->data));
    memcpy(m->data, &rx[5], m->dlc);

    return 1;
}