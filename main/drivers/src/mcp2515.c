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


void mcp_init(){

}

/*
 * Initialize SPI
 */

static void cs_init(){
    gpio_init(SPI_CS);
    gpio_set_dir(SPI_CS, GPIO_OUT);
    gpio_pull_up(SPI_CS);
}

void start_spi()
{
    spi_init(SPI_PORT, CLK_SPEED);

    // SPI pins
    gpio_set_function(SPI_MISO, GPIO_FUNC_SPI);   // MISO
    gpio_set_function(SPI_CS, GPIO_FUNC_SPI);   // CS
    gpio_set_function(SPI_SCK, GPIO_FUNC_SPI);  // SCK
    gpio_set_function(SPI_MOSI, GPIO_FUNC_SPI);  // MOSI


    // CS is manually controlled

    if(spi_transmit_semphr == NULL)
    {
        spi_transmit_semphr = xSemaphoreCreateMutex();
    }
}



/*
 * Chip select control
 */
static void cs_low(){
    gpio_put(SPI_CS, 0);
}

static void cs_high(){
    gpio_put(SPI_CS, 1);
}