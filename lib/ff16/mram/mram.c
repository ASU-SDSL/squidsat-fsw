#include "mram.h"

// reference: https://netsol.co.kr/wp-content/uploads/2023/10/S3A3204x0M_rev1.2.pdf
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

#include "HardwareConfig.h"

/// Command codes
/// Control
#define WREN 0x06
#define WRDI 0x04
#define DPDE 0xB9
#define DPDX 0xAB

/// Register Read / Write 
#define RDSR 0x05
#define RDC1 0x35
#define RDC2 0x3F
#define RDC3 0x44
#define RDC4 0x45
#define RDCX 0x46
#define RDID 0x9F 
#define RUID 0x4C

#define WRSR 0x01
#define WRCX 0x87

/// Memory Array Read / Write  
#define READ 0x03

#define WRTE 0x02


static spi_inst_t* _bus; 
static uint _cs; 

static void send_simple_command(uint8_t cmd) {
  taskENTER_CRITICAL(); 
  gpio_put(_cs, 0);

  spi_write_blocking(_bus, &cmd, 1);

  gpio_put(_cs, 1);
  taskEXIT_CRITICAL(); 
}

static void blocking_busy_wait() {
  // TODO: traffic control 
}

int mram_init(spi_inst_t *spi_bus, uint cs, uint32_t spi_baud){
  _bus = spi_bus; 
  _cs = cs; 

  uint real_baud = spi_init(spi_bus, spi_baud);
  // printf("real baud: %u\n", real_baud); 

  // spi pins
  gpio_set_function(SPI0_MISO_PIN, GPIO_FUNC_SPI);
  gpio_set_function(SPI0_SCLK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(SPI0_MOSI_PIN, GPIO_FUNC_SPI);

  gpio_init(_cs);
  gpio_set_dir(cs, GPIO_OUT);
  gpio_put(_cs, 1);

  send_simple_command(DPDX); // wake

  return 0; 
}

// expected id is 0xd9
int mram_read_id(){
  blocking_busy_wait(); 

  taskENTER_CRITICAL(); 
  gpio_put(_cs, 0); 

  uint8_t cmd = RDID; 
  spi_write_blocking(_bus, &cmd, 1);

  uint8_t buf[4];
  spi_read_blocking(_bus, 0xFF, buf, 4);

  gpio_put(_cs, 1); 
  taskEXIT_CRITICAL(); 

  return buf[0]; // just the manufacturer id 
}

int read_sector(DWORD address, BYTE *buff){
  blocking_busy_wait(); 

  uint8_t buf[4] = { READ, address >> 16, address >> 8, address & 0xFF };

  taskENTER_CRITICAL(); 
  gpio_put(_cs, 0); 

  spi_write_blocking(_bus, buf, sizeof(buf)); 
  int res = spi_read_blocking(_bus, 0xFF, buff, SECTOR_SIZE); 

  gpio_put(_cs, 1); 
  taskEXIT_CRITICAL(); 

  return res; 
}

int write_sector(DWORD address, const BYTE *buff){
  blocking_busy_wait(); 

  uint8_t buf[4] = { WRTE, address >> 16, address >> 8, address & 0xFF };

  send_simple_command(WREN); 

  taskENTER_CRITICAL(); 
  gpio_put(_cs, 0); 

  spi_write_blocking(_bus, buf, sizeof(buf)); 
  int res = spi_write_blocking(_bus, buff, SECTOR_SIZE); 

  gpio_put(_cs, 1); 
  taskEXIT_CRITICAL(); 

  // send_simple_command(WRDI); // I don't think I need this 

  return res;
}


void mram_test() {
  printf("MRAM Test\n");
  
  mram_init(FS_SPI_BUS, FS_CS_PIN, FS_SPI_BAUDRATE);

  int id = mram_read_id(); 
  printf("mram id: 0x%x\n", id); 

  printf("End MRAM Test\n"); 
}