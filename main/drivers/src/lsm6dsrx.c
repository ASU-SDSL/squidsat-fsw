/**
 * @file lsm6dsrx.c
 * @author Aidan Doyle (Doyle-Squared)
 * @brief 
 * @version 0.1
 * @date 2026-05-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "lsm6dsrx.h"

// I2C Address
static const uint8_t LSM6_ADDR = 0x44; //PLACEHOLDER

// Sensor's registers
static const uint8_t REG_XL_ODR = 0x10; // Accelerometer Output Data Rate (aka refresh rate) Set to 52hz, and Low-Pass Filter 2 is enabled
static const uint8_t REG_BDU = 0x12;    // Block Data Update - Can be enabled to prevent writing to a register while reading it
static const uint8_t REG_WHO_AM_I = 0x0f;     

static const uint8_t REG_TEMP_L = 0x20;
static const uint8_t REG_TEMP_H = 0x21;

static const uint8_t REG_XL_X_L = 0x28; // Probably your starting/only address, if calling i2c_read to read in 6 bits
static const uint8_t REG_XL_X_H = 0x29; 
static const uint8_t REG_XL_Y_L = 0x2A; 
static const uint8_t REG_XL_Y_H = 0x2B; 
static const uint8_t REG_XL_Z_L = 0x2C; 
static const uint8_t REG_XL_Z_H = 0x2D; 

// Other Constants
 uint8_t CMD_BDU = 0x84;
 uint8_t CMD_XL_ODR = 0x32;             // Set to 52hz, and Low-Pass Filter 2 is enabled

int lsm6_config(i2c_inst_t *i2c){
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
    
    return 0;
}

uint8_t lsm6_get_temp(i2c_inst_t *i2c, int8_t* whole, uint8_t* fraction){
    int16_t temp_raw;
    int8_t temp_raw_whole;
    uint8_t temp_raw_fraction;
    
    if(i2c_read_from_register(i2c, LSM6_ADDR, REG_TEMP_H, &temp_raw_whole, 1)){
        return 1;
    }

    if(i2c_read_from_register(i2c, LSM6_ADDR, REG_TEMP_L, &temp_raw_fraction, 1)){
        return 1;
    }

    temp_raw =  (int16_t)(((int16_t)temp_raw_whole << 8) | temp_raw_fraction);
    temp_raw_whole = (int8_t)(temp_raw >> 8) + 25;
    temp_raw_fraction = ((uint16_t)(temp_raw & 0xFF) * 100) / 256;

    *whole = temp_raw_whole;
    *fraction = temp_raw_fraction;
    
    return 0;
}

uint8_t lsm6_get_accel(i2c_inst_t *i2c, int32_t* x_axis, int32_t* y_axis, int32_t* z_axis){
    uint8_t data[6]; // Dont forget to cast when working with the data as this stores as uint, but IMU calculates signed numbers

    if(i2c_read_from_register(i2c, LSM6_ADDR, REG_XL_X_L, data, 6)){
        return 1;
    }

    int16_t raw_x = (int16_t)((data[1] << 8) | data[0]);
    int16_t raw_y = (int16_t)((data[3] << 8) | data[2]);
    int16_t raw_z = (int16_t)((data[5] << 8) | data[4]);

    *x_axis = raw_x * 61; // Since the full scale we selected for is 0.061m(g)/LSB and we use 61 to avoid using floats
    *y_axis = raw_y * 61;
    *z_axis = raw_z * 61;

    return 0;
}
