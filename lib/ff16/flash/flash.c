#include "flash.h"
#include "HardwareConfig.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"


uint8_t flash_init(spi_inst_t* spi_bus, uint8_t cs, uint32_t spi_baud){

    gpio_init(cs); 
    gpio_set_dir(cs, GPIO_OUT); 
    gpio_put(cs, 1);

    spi_init(spi_bus, spi_baud);    

    return 0; 
}

uint8_t read_sector(DWORD address, const BYTE* buff){

}

uint8_t write_sector(DWORD address, const BYTE* buff){

}
