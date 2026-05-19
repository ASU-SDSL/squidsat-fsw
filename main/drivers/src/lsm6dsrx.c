/**
 * @file lsm6dsrx.c
 * @author Aidan Doyle (Doyle-Squared)
 * @brief 
 * @version 0.3
 * @date 2026-05-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "lsm6dsrx.h"

// I2C Address
static const uint8_t LSM6_ADDR = 0x44; //PLACEHOLDER

// Sensor's registers
static const uint8_t REG_XL_ODR = 0x10; // Accelerometer Output Data Rate (aka refresh rate) and sensitivity
static const uint8_t REG_G_ODR = 0x11;  // Gyroscope output data rate and sensitivity
static const uint8_t REG_BDU = 0x12;    // Block Data Update - Can be enabled to prevent writing to a register while reading it
static const uint8_t REG_WHO_AM_I = 0x0F;     

static const uint8_t REG_TEMP_L = 0x20;
static const uint8_t REG_TEMP_H = 0x21;
static const uint8_t REG_G_X_L = 0x22;  // Gyroscope X-Axis Low Register. Only one needed if you read 6 bytes consecutively. Registers go up to 0x27
static const uint8_t REG_XL_X_L = 0x28; // Accelerometer X-Axis Low Register. Only one needed if you read 6 bytes consecutively. Registers go up to 0x2D

// Commands (Writing to register)
 uint8_t CMD_BDU = 0x84;
 uint8_t CMD_XL_ODR = 0x32;             // Set to 52Hz, +-2g and Low-Pass Filter 2 is enabled
 uint8_t CMD_G_ODR = 0x32;              // Set to 52H, and z+- 125dps

// Other Constants 
static const float GYRO_LSB = 4.375;       // Since the full scale we selected for is 4.375 milidegrees/LSB 
static const float ACCEL_LSB = 0.061;        // Since the full scale we selected for is 0.061m(g)/LSB 


uint8_t lsm6_config(i2c_inst_t *i2c){
    i2c_util_init();
    uint8_t identify;                   // Buffer to check that i2c is working via WHO_AM_I register on the lsm6

    if(i2c_read_from_register(i2c, LSM6_ADDR, REG_WHO_AM_I, &identify, 1)){
        return 1;
    }

    if(identify != 0x6B){
        printf("LSM6 I2C connection not found... \n");
        return 1;
    }

    if(i2c_write_to_register(i2c, LSM6_ADDR, REG_BDU, &CMD_BDU, 1)){        // Initializing the BDU Register
        return 1;
    }   

    if(i2c_write_to_register(i2c, LSM6_ADDR, REG_XL_ODR, &CMD_XL_ODR, 1)){  // Initializing the Acceleration ODR
        return 1;
    }
    
    if(i2c_write_to_register(i2c, LSM6_ADDR, REG_G_ODR, &CMD_G_ODR, 1)){    // Initializing the Gyroscope ODR
        return 1;
    }   
    
    return 0;
}

uint8_t lsm6_get_temp(i2c_inst_t *i2c, float *temp){
    int16_t temp_raw;
    int8_t temp_raw_whole;
    uint8_t temp_raw_fraction;
    
    if(i2c_read_from_register(i2c, LSM6_ADDR, REG_TEMP_H, &temp_raw_whole, 1)){
        return 1;
    }

    if(i2c_read_from_register(i2c, LSM6_ADDR, REG_TEMP_L, &temp_raw_fraction, 1)){
        return 1;
    }

    temp_raw = ((temp_raw_whole << 8) | temp_raw_fraction);
    *temp = (temp_raw / 256.0) + 25.0;

    //Old fixed point arithmetic code. Not too sure if I should delete this, but look how much simpler using floats is
    // temp_raw =  (int16_t)(((int16_t)temp_raw_whole << 8) | temp_raw_fraction);
    // temp_raw_whole = (int8_t)(temp_raw >> 8) + 25;
    // temp_raw_fraction = ((uint16_t)(temp_raw & 0xFF) * 100) / 256;

    // *whole = temp_raw_whole;
    // *fraction = temp_raw_fraction;
    
    return 0;
}

uint8_t lsm6_get_accel(i2c_inst_t *i2c, float* x_axis, float* y_axis, float* z_axis){
    uint8_t data[6]; // Dont forget to cast when working with the data as this stores as uint, but IMU calculates signed numbers

    if(i2c_read_from_register(i2c, LSM6_ADDR, REG_XL_X_L, data, 6)){
        return 1;
    }

    int16_t raw_x = (int16_t)((data[1] << 8) | data[0]);
    int16_t raw_y = (int16_t)((data[3] << 8) | data[2]);
    int16_t raw_z = (int16_t)((data[5] << 8) | data[4]);

    *x_axis = ((raw_x * ACCEL_LSB) / 1000.0) * 9.80665;     // Units are in G's after dividing by 1000. Units are in m/s^2 after mult by 9.80665
    *y_axis = ((raw_y * ACCEL_LSB) / 1000.0)* 9.80665;
    *z_axis = ((raw_z * ACCEL_LSB) / 1000.0)* 9.80665;

    return 0;
}

uint8_t lsm6_get_gyro(i2c_inst_t *i2c, float* x_axis, float* y_axis, float* z_axis){
    uint8_t data[6]; // Dont forget to cast when working with the data as this stores as uint, but IMU calculates signed numbers

    if(i2c_read_from_register(i2c, LSM6_ADDR, REG_G_X_L, data, 6)){
        return 1;
    }

    int16_t raw_x = (int16_t)((data[1] << 8) | data[0]);
    int16_t raw_y = (int16_t)((data[3] << 8) | data[2]);
    int16_t raw_z = (int16_t)((data[5] << 8) | data[4]);

    *x_axis = raw_x * GYRO_LSB;                             // CALIBRATION OFFSETS HAVE NOT BEEN APPLIED
    *y_axis = raw_y * GYRO_LSB;
    *z_axis = raw_z * GYRO_LSB;

    return 0;
}
