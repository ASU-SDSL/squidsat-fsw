/**
 * @file lsm6dsrx.h
 * @author Aidan Doyle (Doyle-Squared)
 * @brief Accelerometer, Gyroscope, and Tempurature Driver (LSM6DSRX)
 * @version 0.3
 * @date 2026-05-07
 * 
 */

#ifndef LSM6DSRX_H
#define LSM6DSRX_H

#include "i2c_util.h"

#include "log.h"
#include <stdint.h>


/**
 * @brief Configures the LSM6 to read the temperature only as of 5/7/26
 * 
 * @param i2c I2C Instance
 * @return int Status (0 = Success) (1 = Fail)
 */
uint8_t lsm6_config(i2c_inst_t *i2c);

/**
 * @brief Gets the temperature. Temperature data buffer is split into two integers to avoid costly floating
 * point operations
 * 
 * @param i2c I2C Instance
 * @param whole Data buffer for storing the whole number value of the temperature
 * @param fraction Data buffer for storing the point fractional(decimal) value of the temperature
 * @return uint8_t Status (0 = Success) 
 */
uint8_t lsm6_get_temp(i2c_inst_t *i2c, float* temp);

/**
 * @brief Gets the acceleration. Keeps the high and low together, to be worked on with fixed-point arithmetic 
 * avoid costly floating point ops. 
 * 
 * @param i2c I2C Instance
 * @param x_axis Data buffer for the X-Axis. Divide by 10^6 for the actaul value in (g)
 * @param y_axis Data buffer for the Y-Axis. Divide by 10^6 for the actaul value in (g)
 * @param z_axis Data buffer for the Z-Axis. Divide by 10^6 for the actaul value in (g)
 * @return uint8_t Status (0 = Success)
 */
uint8_t lsm6_get_accel(i2c_inst_t *i2c, float* x_axis, float* y_axis, float* z_axis);

/**
 * @brief Gets the gyroscope's data. Keeps the high and low together, to be worked on with fixed-point arithmetic 
 * avoid costly floating point ops. 
 * 
 * @param i2c I2C Instance
 * @param x_axis Data buffer for the X-Axis. Divide by 10^3 for the actaul value in degrees
 * @param y_axis Data buffer for the Y-Axis. Divide by 10^3 for the actaul value in degrees
 * @param z_axis Data buffer for the Z-Axis. Divide by 10^3 for the actaul value in degrees
 * @return uint8_t Status (0 = Success)
 */
uint8_t lsm6_get_gyro(i2c_inst_t *i2c, float* x_axis, float* y_axis, float* z_axis);

#endif