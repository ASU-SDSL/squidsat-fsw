#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#define MAX_JOBS 10

typedef void (*job_function)(void *args);

typedef struct {
    TickType_t recurr_time; //tracks if the job is recurring or not
    TickType_t execute_time; //the time at which the job should be executed
    char name[20]; //name of the job
    job_function func; //function pointer to the job function
    void *args; //arguments to be passed to the job function
} scheduler_t;

typedef struct {
    scheduler_t *jobs[MAX_JOBS]; 
    size_t job_count; //number of jobs currently
} job_context_t;

job_context_t global_job_context;

//Scheduler task function
//void create_job();
//void delete_job();
void add_job(job);
void run_scheduler();