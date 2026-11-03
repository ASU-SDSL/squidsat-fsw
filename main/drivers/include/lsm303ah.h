/**
 * @file lsm303ah.h
 * @author Aidan Doyle (Doyle-Squared)
 * @brief 
 * @version 0.1
 * @date 2026-05-12
 * 
 */

#ifndef LSM303AH_H
#define LSM303AH_H

#include "i2c_util.h"

#include "log.h"
#include <stdint.h>

/**
 * @brief Configures the LSM3 to read the Magnetometer
 * 
 * @param i2c I2C Instance
 * @return int Status (0 = Success) (1 = Fail)
 */
uint8_t lsm3_config(i2c_inst_t *i2c);

/**
 * @brief Gets the Magnetometer data.
 * 
 * @param i2c I2C Instance
 * @param x_axis Data buffer for the X-Axis. 
 * @param y_axis Data buffer for the Y-Axis. 
 * @param z_axis Data buffer for the Z-Axis. 
 * @return uint8_t Status (0 = Success)
 */
uint8_t lsm3_get_mag(i2c_inst_t *i2c, float* x_axis, float* y_axis, float* z_axis);

#endif