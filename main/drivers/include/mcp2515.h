#ifndef MCP2515
#define MCP2515

#include <stdio.h>

#include "FreeRTOS.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "semphr.h"



#define SPI_PORT    spi1
#define SPI_CS      25
#define SPI_MOSI    27
#define SPI_MISO    28
#define SPI_SCK     26
#define CLK_SPEED   100000

typedef struct{
    /* data */
    uint32_t    id;
    uint8_t     extended;
    uint8_t     dlc;
    uint8_t     data[8]
} can_message; 



void start_spi();

int mcp2515_send(uint32_t data);
can_message mcp2515_recieve();



#endif
