#pragma once

#include "gse.h"
#include "steve.h"
#include "log.h"

void adc_setup(void);
void read_temp_job_recurring(void *args);

extern jobs_t read_temp_recurring;