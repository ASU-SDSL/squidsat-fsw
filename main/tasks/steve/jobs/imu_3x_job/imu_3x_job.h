#pragma once
#include "lsm303ah.h"
#include "steve.h"
#include "log.h"

void imu_get_mag(void *args);

extern jobs_t imu_3x_mag;


