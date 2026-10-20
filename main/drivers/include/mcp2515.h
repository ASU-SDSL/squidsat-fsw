#ifndef MCP2515
#define MCP2515

#include <stdio.h>

#include "FreeRTOS.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "semphr.h"


#include <stdio.h>

#include "FreeRTOS.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "semphr.h"

#define SPI_PORT    spi1
#define SPI_CS      13 // For the OBC PCB - 25
#define SPI_MOSI    11 // For the OBC PCB - 27
#define SPI_MISO    12 // For the OBC PCB - 28
#define SPI_SCK     10 // For the OBC PCB - 26
#define CLK_SPEED   100000

typedef struct{
    uint32_t    id;
    uint8_t     extended;
    uint8_t     dlc;
    uint8_t     data[8];
} can_message_t;

void mcp2515_spi_init(void);
int mcp2515_init(void);


uint8_t mcp_read_reg(uint8_t addr);
void mcp_write_reg(uint8_t addr, uint8_t data);
bool mcp2515_tx_busy(void);
void mcp2515_log_tx_status(void);

int mcp2515_send(const can_message_t *m);
int mcp2515_receive(can_message_t *m);

#endif
