#pragma once
#include "lsm6dsrx.h"
#include "steve.h"
#include "log.h"

void imu_get_accel(void *args);

extern jobs_t imu_accel;


