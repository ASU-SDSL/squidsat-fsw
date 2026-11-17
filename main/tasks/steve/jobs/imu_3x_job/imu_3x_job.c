#include "imu_3x_job.h"

// TODO - Write functions for getting LSM3 sensor data
// Check lsm6dsrx header files for description
void imu_get_mag(void *args){
    (void) args;

    // Using floats for now, since fixed-point arithmetic is a pain in the booty cheeks
    float mag_x;
    float mag_y;
    float mag_z;

    // Configuring IMU and setting up I2C conntection
    if(lsm3_config(i2c0) == 1){ 
        log_error("Error reading I2C line...\n");
    }

    // Functions to read the data registers
    lsm3_get_mag(i2c0, &mag_x, &mag_y, &mag_z);

    log_infof("Testing IMU Mag. X: %f, Y: %f, Z: %f\n", mag_x, mag_y, mag_z);
}

jobs_t imu_3x_mag = {
    .func = imu_get_mag,
    .recurr_time = 1000,
    .execute_time = 0, 
    .name = "IMU Magnetometer",
    .args = NULL,
};