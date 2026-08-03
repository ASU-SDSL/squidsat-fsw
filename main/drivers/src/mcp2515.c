#include <stdio.h>
#include <string.h>

#include "mcp2515.h"
#include "log.h"

#include "hardware/spi.h"
#include "hardware/gpio.h"

#include "FreeRTOS.h"
#include "semphr.h"


static SemaphoreHandle_t spi_transmit_semphr;


/*
 * MCP2515 SPI Commands
 */
#define MCP2515_RESET        0xC0
#define MCP2515_READ         0x03
#define MCP2515_WRITE        0x02
#define MCP2515_READ_STATUS  0xA0
#define MCP2515_RX_STATUS    0xB0
#define MCP2515_READ_RXB0    0x90


/*
 * Initialize SPI
 */
void start_spi()
{
    spi_init(SPI_PORT, CLK_SPEED);

    // SPI pins
    gpio_set_function(MISO_PIN, GPIO_FUNC_SPI);   // MISO
    gpio_set_function(CS_PIN, GPIO_FUNC_SPI);   // CS
    gpio_set_function(SCK_PIN, GPIO_FUNC_SPI);  // SCK
    gpio_set_function(MOSI_PIN, GPIO_FUNC_SPI);  // MOSI


    // CS is manually controlled
    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_put(CS_PIN, 1);


    if(spi_transmit_semphr == NULL)
    {
        spi_transmit_semphr = xSemaphoreCreateMutex();
    }
}


/*
 * Chip select control
 */
static void cs_low(){
    gpio_put(CS_PIN, 0);
}

static void cs_high(){
    gpio_put(CS_PIN, 1);
}

/*
 * Reset MCP2515
 */
void mcp2515_reset(){
    uint8_t cmd = MCP2515_RESET;
    if (xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY) == pdTRUE){
        cs_low();

        spi_write_blocking(
            SPI_PORT,
            &cmd,
            1
        );

        cs_high();
        xSemaphoreGive(spi_transmit_semphr);
    }
    sleep_ms(10);

    log_info("MCP2515 reset complete");
}


/*
 * Read MCP2515 register
 */
uint8_t mcp2515_read_register(uint8_t address){
    uint8_t tx[3];

    uint8_t rx[3];


    tx[0] = MCP2515_READ;
    tx[1] = address;
    tx[2] = 0x00;


    if (xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY) == pdTRUE){
        cs_low();

        spi_write_read_blocking(
            SPI_PORT,
            tx,
            rx,
            3
        );

        cs_high();
        xSemaphoreGive(spi_transmit_semphr);
    }
    return rx[2];
}


/*
 * Write MCP2515 register
 */
void mcp2515_write_register(uint8_t address, uint8_t value){
    uint8_t tx[3];

    tx[0] = MCP2515_WRITE;
    tx[1] = address;
    tx[2] = value;

    if (xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY) == pdTRUE){
        cs_low();

        spi_write_blocking(
            SPI_PORT,
            tx,
            3
        );
        cs_high();
        xSemaphoreGive(spi_transmit_semphr);
    }
}


/*
 * Check if CAN message exists
 */
bool mcp2515_message_available() {
    uint8_t tx = MCP2515_RX_STATUS;
    uint8_t rx;

    if(xSemaphoreTake(spi_transmit_semphr, portMAX_DELAY) == pdTRUE){
        cs_low();

        spi_write_read_blocking(
            SPI_PORT,
            &tx,
            &rx,
            1
        );
        cs_high();

        xSemaphoreGive(spi_transmit_semphr);
    }

    // RX0IF or RX1IF bits
    if(rx & 0x40 || rx & 0x80)
    {
        return true;
    }

    return false;
}


/*
 * Read CAN message from RX buffer
 */
void mcp2515_receive() {
    uint8_t tx[14];
    uint8_t rx[14];

    memset(tx, 0x00, sizeof(tx));
    tx[0] = MCP2515_READ_RXB0;

    if (xSemaphoreTake(spi_transmit_semphr, 0xffffff) == pdTRUE){
        cs_low();
        spi_write_read_blocking(
            SPI_PORT,
            tx,
            rx,
            sizeof(tx)
        );

        cs_high();
        xSemaphoreGive(spi_transmit_semphr);

        char message[128];
        snprintf(
            message,
            sizeof(message),
            "CAN DATA BYTE 0: 0x%02X",
            rx[6]
        );
        log_info(message);
    }
}