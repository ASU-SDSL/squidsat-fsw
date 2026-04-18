#ifndef I2C_UTIL_H
#define I2C_UTIL_H

#include <stdint.h>

#include "hardware/i2c.h"

/**
 * @brief Initialize i2c speed and pins
 */
void i2c_util_init();

/**
 * @brief Write [reg] and [buf] to i2c device, [reg] is just inserted before
 * [buf]
 *
 * @param i2c I2C instance to use
 * @param addr Device address
 * @param reg Register to write to
 * @param buf Data Buffer
 * @param nbytes Length of Data Buffer
 * @return int Status of operation (0 = good)
 */
int i2c_write_to_register(i2c_inst_t *i2c, const uint8_t addr,
                          const uint8_t reg, uint8_t *buf,
                          const uint8_t nbytes);

/**
 * @brief Write [reg] byte to i2c device and then read [nbytes] from it. Read
 * bytes are stored in [buf]
 *
 * @param i2c I2C instance to use
 * @param addr Device address
 * @param reg Register to read from
 * @param buf Data Buffer
 * @param nbytes Length of Data Buffer
 * @return int Status of operation (0 = good)
 */
int i2c_read_from_register(i2c_inst_t *i2c, const uint8_t addr,
                           const uint8_t reg, uint8_t *buf,
                           const uint8_t nbytes);

#endif  // I2C_UTIL_H