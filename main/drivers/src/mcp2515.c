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

void mcp_write_reg(uint8_t addr, uint8_t data){
    uint8_t tx_buf_packet[3] = {MCP2515_WRITE, addr, data};
    if(xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY)){
        cs_pull_down();
        spi_write_blocking(SPI_PORT, tx_buf_packet, 3);
        cs_pull_up();

        xSemaphoreGive(spi_transmit_semphr);
    }

    mcp_read_reg(addr); // confirmation that it has been sent properly
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

    mcp_reset(); // this puts the controller into configuration mode

    return 1;
}