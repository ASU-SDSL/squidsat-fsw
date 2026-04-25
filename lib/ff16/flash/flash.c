#include "flash.h"
#include "HardwareConfig.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"


uint8_t flash_init(spi_inst_t* spi_bus, uint8_t cs){

    gpio_init(FS_CS_PIN); 
    gpio_set_dir(FS_CS_PIN, GPIO_OUT); 
    gpio_put(FS_CS_PIN, 1);

    spi_init(FS_SPI_BUS, FS_SPI_BAUDRATE);    

    return 0; 
}

uint8_t read_sector(DWORD address, BYTE* buff){

}

uint8_t write_sector(DWORD address, BYTE* buff){

}