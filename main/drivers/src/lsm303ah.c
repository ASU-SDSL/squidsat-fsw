/**
 * @file lsm303ah.c
 * @author Aidan Doyle (Doyle-Squared)
 * @brief 
 * @version 0.1
 * @date 2026-05-12
 * 
 */
#include "lsm303ah.h"

// I2C Address
static const uint8_t LSM3_ADDR = 0x1E; // PLACEHOLDER

// Sensor's Registers
static const uint8_t REG_M_ODR = 0x60;
static const uint8_t REG_M_OFFSET = 0x61;

static const uint8_t REG_M_X_L = 0x68;   // Magnetometer X-Axis Low Register. Only one needed if you read 6 bytes consecutively. Registers go up to 0x6D

// Commands (Writing to registers)
uint8_t CMD_M_ODR = 0x00;
uint8_t CMD_M_OFFSET = 0x02;

// Other Constants 
static const int MAG_LSB = 1.5; 

uint8_t lsm3_config(i2c_inst_t *i2c){
    i2c_util_init();

    if(i2c_write_to_register(i2c, LSM3_ADDR, REG_M_ODR, &CMD_M_ODR, 1)){
        return 1;
    }

    if(i2c_write_to_register(i2c, LSM3_ADDR, REG_M_OFFSET, &CMD_M_OFFSET, 1)){
        return 1;
    }

}

uint8_t lsm3_get_mag(i2c_inst_t *i2c, int32_t* x_axis, int32_t* y_axis, int32_t* z_axis){
    uint8_t data[6]; // Dont forget to cast when working with the data as this stores as uint, but IMU calculates signed numbers

    if(i2c_read_from_register(i2c, LSM3_ADDR, REG_M_X_L, data, 6)){
        return 1;
    }

    int16_t raw_x = (int16_t)((data[1] << 8) | data[0]);
    int16_t raw_y = (int16_t)((data[3] << 8) | data[2]);
    int16_t raw_z = (int16_t)((data[5] << 8) | data[4]);

    // Hard and soft iron calibration NOT completed. Heading formula NOT completed. Optional tilt compensation NOT completed.
    *x_axis = raw_x * MAG_LSB; 
    *y_axis = raw_y * MAG_LSB;
    *z_axis = raw_z * MAG_LSB;

    return 0;
}
