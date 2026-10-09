#include <FreeRTOS.h>
#include <steve.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "log.h"
#include "sensor_job/sensor_job.h"

/**
 * @authors: Koowum Joshi, Quan Le
 * @breif: STEVE Scheduler Implementation.
 * @version: 1.0
 * @date: 2026-09-25
 */

job_context_t global_job_context;  
static SemaphoreHandle_t job_mutex;

void add_job(jobs_t *job){

    if(job == NULL){ //if the job is null, we cannot add it to the job context
        log_error("Invalid job, cannot add to job context\n"); //print error message
        return; //invalid job
    }
    if(global_job_context.job_count >= MAX_JOBS){ //job context is full
        log_error("Job context is full, cannot add more jobs\n"); //print error message
        return;
    }

    if(xSemaphoreTake(job_mutex, portMAX_DELAY)){
        global_job_context.jobs[global_job_context.job_count] = job; //Put the new job in the next open slot (which equals the job count)
        global_job_context.job_count++; //increment the job count for the next job

        xSemaphoreGive(job_mutex);
    }
}

static jobs_t *find_ready_job(TickType_t current_time){
    jobs_t *ready = NULL;
    if(xSemaphoreTake(job_mutex, portMAX_DELAY) == pdTRUE){
        for(int i = 0; i < global_job_context.job_count; i++){
            jobs_t *job = global_job_context.jobs[i];
            if((int32_t)(current_time - job->execute_time) >= 0){
                ready = job;          // grab the pointer while still locked
                break;
            }
        }
        xSemaphoreGive(job_mutex);    // single exit point, always released
    }
    return ready;
}

// Function CANNOT be called outside of delete job or a race condition will happen
static void reorganize_job(int job_index){
    for(int i = job_index; i < global_job_context.job_count - 1; i++){
        if(global_job_context.jobs[i] == NULL){
            global_job_context.jobs[i] = global_job_context.jobs[i+1];
            global_job_context.jobs[i+1] = NULL;
        }
    }

    global_job_context.job_count--;
}

void delete_job(jobs_t * job){
    if(xSemaphoreTake(job_mutex, portMAX_DELAY)){
        for(int i = 0; i < global_job_context.job_count;i++){ //iterate through the jobs
            if(global_job_context.jobs[i] == job){ // check if it is the job we want to delete
                char msg[64];
                snprintf(msg, sizeof(msg), "deleting %s, count before deletion: %d", global_job_context.jobs[i]->name, global_job_context.job_count);
                log_warning(msg);
                global_job_context.jobs[i] = NULL; //set it to null to remove job
                reorganize_job(i);
            }
        }
        xSemaphoreGive(job_mutex);
    }
}

void scheduler_init(void) {
    global_job_context.job_count = 0; //initialize
    job_mutex = xSemaphoreCreateMutex();
}

void run_scheduler(){
    for(;;){
        TickType_t current_time = xTaskGetTickCount();
        jobs_t *job_to_run = find_ready_job(current_time);

        if(job_to_run == NULL){
            vTaskDelay(pdMS_TO_TICKS(10));   // no lock held here
            continue;
        }

        job_to_run->func(job_to_run->args);  // no lock held while the job runs

        if(job_to_run->recurr_time != 0){
            job_to_run->execute_time = current_time + job_to_run->recurr_time;
        } else {
            delete_job(job_to_run);          // takes and releases the lock itself
        }
    }
}

void scheduler_task(void *pvParameters){
    scheduler_init();

    sensor_setup();
    // add_job(&led_blinking_once); 
    // add_job(&led_blinking_recurr); 
    // add_job(&led_blinking_onoff); 
    // add_job(&led_blinking_fast_blink); 
    
    run_scheduler();
}