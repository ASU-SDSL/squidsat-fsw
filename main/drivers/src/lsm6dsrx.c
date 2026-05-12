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
static const uint8_t REG_XL_ODR = 0x10; // Accelerometer Output Data Rate (aka refresh rate);
static const uint8_t REG_BDU = 0x12;    // Block Data Update - Can be enabled to prevent writing to a register while reading it
static const uint8_t REG_TEMP_L = 0x20;
static const uint8_t REG_TEMP_H = 0x21;

// Other Constants
 uint8_t CMD_BDU = 0x84;
 uint8_t CMD_XL_ODR = 0x20;

int lsm6_config(i2c_inst_t *i2c){
    i2c_util_init();

    if(i2c_write_to_register(i2c, LSM6_ADDR, REG_BDU, &CMD_BDU, 1)){    // Initializing the BDU Register
        return 1;
    }   

    if(i2c_write_to_register(i2c, LSM6_ADDR, REG_XL_ODR, &CMD_XL_ODR, 1)){ // Initializing the Acceleration ODR
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

}
