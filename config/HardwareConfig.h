/**
 * @file HardwareConfig.h
 * @author Tyler Nielsen
 * @brief Macro definition for all hardware configuration (pin and bus assignment, etc)
 * @version 0.1
 * @date 2026-06-26
 * 
 * @copyright Copyright (c) 2026
 * 
 */
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
// mram stopped working died on the breadboard when tried at 20 MHz which is 
// less than the datasheet, but like that kind of speed isn't needed. 
// The trade off here is a higher speed is a less stable bus but reduces the 
// duration of transactions (critical sections) which should be better for the 
// scheduler 
#define FS_SPI_BAUDRATE 15000000 