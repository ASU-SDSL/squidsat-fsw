#ifndef LSM6DSRX_H
#define LSM6DSRX_H

/**
 * @file lsm6dsrx.h
 * @author Aidan Doyle (Doyle-Squared)
 * @brief Accelerometer, Gyroscope, and Tempurature Driver (LSM6DSRX)
 * @version 1.1
 * @date 2026-05-07
 * Date is the date created, not
 */

#include "i2c_util.h"

#include "log.h"
#include <stdint.h>


/**
 * @brief Configures the LSM6. Can change range of measurements or features enabled
 * @param i2c I2C Instance
 * @return int Status (0 = Success) (1 = Fail)
 */
uint8_t lsm6_config(i2c_inst_t *i2c);

/**
 * @brief Gets the temperature.
 * 
 * @param i2c I2C Instance
 * @param temp Pointer to where the calculated temperature in Celcius, will be stored.
 * @return uint8_t Status (0 = Success) 
 */
uint8_t lsm6_get_temp(i2c_inst_t *i2c, float* temp);

/**
 * @brief Gets the acceleration. 
 * 
 * @param i2c I2C Instance
 * @param x_axis Data buffer for the X-Axis. 
 * @param y_axis Data buffer for the Y-Axis. 
 * @param z_axis Data buffer for the Z-Axis. 
 * @return uint8_t Status (0 = Success)
 */
uint8_t lsm6_get_accel(i2c_inst_t *i2c, float* x_axis, float* y_axis, float* z_axis);

/**
 * @brief Gets the gyroscope's data.  
 * 
 * @param i2c I2C Instance
 * @param x_axis Data buffer for the X-Axis. 
 * @param y_axis Data buffer for the Y-Axis. 
 * @param z_axis Data buffer for the Z-Axis. 
 * @return uint8_t Status (0 = Success)
 */
uint8_t lsm6_get_gyro(i2c_inst_t *i2c, float* x_axis, float* y_axis, float* z_axis);

#endif