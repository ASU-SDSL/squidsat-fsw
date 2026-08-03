#ifndef MCP2515
#define MCP2515

#include <stdio.h>

#include "FreeRTOS.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "semphr.h"



#define SPI_PORT    spi1
#define CLK_SPEED   100000
#define CS_PIN      9
#define MISO_PIN    8
#define MOSI_PIN    11
#define SCK_PIN     10

typedef struct
{
    /* data */
    uint32_t    id;
    uint8_t     extended;
    uint8_t     dlc;
    uint8_t     data[8]
};



void start_spi();

int mcp2515_send(uint32_t data);
int mcp2515_recieve();



#endif