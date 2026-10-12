#include "imu_job.h"

// TODO - Write functions for getting LSM3 sensor data
// Check lsm6dsrx header files for description
void imu_get_accel(void *args){
    (void) args;

    // Using floats for now, since fixed-point arithmetic is a pain in the booty cheeks
    float accel_x;
    float accel_y;
    float accel_z;

    float gyro_x;
    float gyro_y;
    float gyro_z;

    float temp;

    // Configuring IMU and setting up I2C conntection
    if(lsm6_config(i2c0) == 1){ 
        log_error("Error reading I2C line...\n");
    }

    // Functions to read the data registers
    lsm6_get_accel(i2c0, &accel_x, &accel_y, &accel_z);
    lsm6_get_gyro(i2c0, &gyro_x, &gyro_y, &gyro_z);
    lsm6_get_temp(i2c0, &temp);

    // Printf used here since I dont think the custom log functions can take variables as of 10/12/26
    // printf("Testing IMU Accel. X: %f, Y: %f, Z: %f\n", accel_x, accel_y, accel_z);
    // printf("Testing IMU temp. %fC\n", temp);
    // printf("Testing IMU Gyro. X: %f, Y: %f, Z: %f\n\n", (gyro_x*0.001), (gyro_y*0.001), (gyro_z*0.001));    // Multiplying by 1 to turn milidegrees into degrees
}

jobs_t imu_accel = {
    .func = imu_get_accel,
    .recurr_time = 1000,
    .execute_time = 0, 
    .name = "IMU Accel",
    .args = NULL,
};