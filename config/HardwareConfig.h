#pragma once 

#include "hardware/i2c.h"

/// SPI 0 Pins 
#define SPI0_MISO_PIN 4
#define SPI0_SCLK_PIN 6
#define SPI0_MOSI_PIN 7

#define I2C0_SDA_PIN 8
#define I2C0_SCL_PIN 9

/// Bus definitions 
#define RTC_I2C_BUS i2c0

#define FS_SPI_BUS spi0
#define FS_CS_PIN 5
#define FS_SPI_BAUDRATE 8000000