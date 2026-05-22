#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

typedef void (*job_function)(void *args);

typedef struct {
    Ticktype_t recurr_time; //tracks if the job is recurring or not
    Ticktype_t execute_time; //the time at which the job should be executed
    char name[20]; //name of the job
    job_function func; //function pointer to the job function
    void *args; //arguments to be passed to the job function
} scheduler_t;

typedef struct {
    uint8_t id;
    Ticktype_t job_start; //the time at which the worker will be available to execute the next job
    volatile uint8_t is_busy; //flag to indicate if the worker is currently executing a job
} worker_t;

Worker_t workers[2]; //2 workers for the scheduler

QueueHandle_t job_queue;

//Scheduler task function
void create_job();
void add_to_workers();
void delete_from_workers();
void run_scheduler();