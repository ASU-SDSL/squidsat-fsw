#include "flash.h"

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

#include "HardwareConfig.h"


#define SFE_FLASH_COMMAND_WRITE_STATUS_REG 0x01  // WRSR
#define SFE_FLASH_COMMAND_PAGE_PROGRAM 0x02
#define SFE_FLASH_COMMAND_READ_DATA 0x03
#define SFE_FLASH_COMMAND_WRITE_DISABLE 0x04            // WRDI
#define SFE_FLASH_COMMAND_READ_STATUS_25XX 0x05         // RDSR
#define SFE_FLASH_COMMAND_WRITE_ENABLE 0x06             // WREN
#define SFE_FLASH_COMMAND_ENABLE_WRITE_STATUS_REG 0x50  // EWSR
#define SFE_FLASH_COMMAND_ENABLE_SO_DURING_AAI \
  0x70  // EBSY: Enable SO to Output RY/BY# Status during AAI Programming
#define SFE_FLASH_COMMAND_DISABLE_SO_DURING_AAI \
  0x80  // DBSY: Disable SO to Output RY/BY# Status during AAI Programming
#define SFE_FLASH_COMMAND_READ_JEDEC_ID 0x9F
#define SFE_FLASH_COMMAND_AAI_WORD_PROGRAM \
  0xAD  // Auto Address Increment Programming
#define SFE_FLASH_COMMAND_CHIP_ERASE 0xC7
#define SFE_FLASH_COMMAND_READ_STATUS_45XX 0xD7
#define SFE_FLASH_COMMAND_ERASE_SECTOR 0x20

static uint8_t _cs;
static spi_inst_t *_bus;

// reference:
// https://github.com/tylermnielsen/SparkFun_SPI_SerialFlash_Arduino_Library/tree/main
// critical sections are necessary
// later can be improved with spi dma - but that needs better synchronization if done 

uint8_t flash_read_status1() {
  taskENTER_CRITICAL();
  gpio_put(_cs, 0);
  uint8_t buf = SFE_FLASH_COMMAND_READ_STATUS_25XX;
  spi_write_blocking(_bus, &buf, 1);

  spi_read_blocking(_bus, 0xFF, &buf, 1);
  gpio_put(_cs, 1);
  taskEXIT_CRITICAL();

  return buf;
}

static bool is_busy() {
  uint8_t buf = flash_read_status1();

  return (buf & (1 << 0));
}

static void blocking_busy_wait() {
  while (is_busy()) {  // can be made more continuous
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

int flash_init(spi_inst_t *spi_bus, uint8_t cs, uint32_t spi_baud) {
  _cs = cs;
  _bus = spi_bus;

  spi_init(spi_bus, spi_baud);

  spi_set_format(spi_bus, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

  // spi pins
  gpio_set_function(SPI0_MISO_PIN, GPIO_FUNC_SPI);
  gpio_set_function(SPI0_SCLK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(SPI0_MOSI_PIN, GPIO_FUNC_SPI);

  gpio_init(cs);
  gpio_set_dir(cs, GPIO_OUT);
  gpio_put(cs, 1);

  return 0;
}

int flash_read_id() {
  blocking_busy_wait();

  taskENTER_CRITICAL();
  gpio_put(_cs, 0);

  int id = 0;
  uint8_t cmd = SFE_FLASH_COMMAND_READ_JEDEC_ID;
  int res = spi_write_blocking(_bus, &cmd, 1);

  uint8_t buf[4];
  spi_read_blocking(_bus, 0xFF, buf, 3);

  gpio_put(_cs, 1);
  taskEXIT_CRITICAL();

  id = (buf[0] << 16) | (buf[1] << 8) | (buf[2] << 0);

  return id;
}

static void send_simple_command(uint8_t cmd) {
  taskENTER_CRITICAL();
  gpio_put(_cs, 0);
  spi_write_blocking(_bus, &cmd, 1);
  gpio_put(_cs, 1);
  taskEXIT_CRITICAL();
}

int erase_sector(DWORD address) {
  blocking_busy_wait();

  // write enable
  send_simple_command(SFE_FLASH_COMMAND_WRITE_ENABLE);

  // erase sector
  taskENTER_CRITICAL();
  gpio_put(_cs, 0);
  uint8_t buf[4] = {SFE_FLASH_COMMAND_ERASE_SECTOR, address >> 16, address >> 8,
                    address & 0xFF};

  int res = spi_write_blocking(_bus, buf, sizeof(buf));

  gpio_put(_cs, 1);
  taskEXIT_CRITICAL();

  return res;
}

int read_page(DWORD address, BYTE *buff) {
  blocking_busy_wait();

  // read page
  taskENTER_CRITICAL();
  gpio_put(_cs, 0);

  uint8_t buf[4] = {SFE_FLASH_COMMAND_READ_DATA, address >> 16, address >> 8,
                    address & 0xFF};
  spi_write_blocking(_bus, buf, sizeof(buf));

  int res = spi_read_blocking(_bus, 0xFF, buff, PAGE_SIZE);

  gpio_put(_cs, 1);
  taskEXIT_CRITICAL();

  return res;
}

int read_sector(DWORD address, BYTE *buff) {
  // read sector by page
  int res = 0;

  for (int i = 0; i < SECTOR_SIZE; i += PAGE_SIZE) {
    res += read_page(address + i, buff + i);
  }

  return res;
}

static int write_page(DWORD address, const BYTE *buff) {
  blocking_busy_wait();

  // write enable
  send_simple_command(SFE_FLASH_COMMAND_WRITE_ENABLE);

  // write page
  taskENTER_CRITICAL();
  gpio_put(_cs, 0);
  uint8_t write_cmd[4] = {SFE_FLASH_COMMAND_PAGE_PROGRAM, address >> 16,
                          address >> 8, address & 0xFF};

  spi_write_blocking(_bus, write_cmd, sizeof(write_cmd));

  int res = spi_write_blocking(_bus, buff, PAGE_SIZE);

  gpio_put(_cs, 1);
  taskEXIT_CRITICAL();

  return res;
}

int write_sector(DWORD address, const BYTE *buff) {
  // erase sector first
  erase_sector(address);  // has blocking_busy_wait in it

  // wait to finish
  blocking_busy_wait();

  // write sector by page
  int res = 0;

  for (int i = 0; i < SECTOR_SIZE; i += PAGE_SIZE) {
    res += write_page(address + i, buff + i);
  }

  return res;
}
