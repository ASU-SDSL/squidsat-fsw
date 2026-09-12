#ifndef I2C_UTIL_H
#define I2C_UTIL_H

#include <stdint.h>

#include "hardware/i2c.h"


void i2c_util_init();

int i2c_write_to_register(i2c_inst_t *i2c, const uint8_t addr,
                          const uint8_t reg, uint8_t *buf,
                          const uint8_t nbytes);


int i2c_read_from_register(i2c_inst_t *i2c, const uint8_t addr,
                           const uint8_t reg, uint8_t *buf,
                           const uint8_t nbytes);

#endif  // I2C_UTIL_H