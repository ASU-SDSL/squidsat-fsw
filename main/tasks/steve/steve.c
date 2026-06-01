#include <FreeRTOS.h>
#include <steve.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sensor_job.h>


void add_job(scheudler_t *job){

    if(job == NULL){ //if the job is null, we cannot add it to the job context
        printf("Invalid job, cannot add to job context"); //print error message
        return; //invalid job
    }
    if(global_job_context.job_count >= MAX_JOBS){ //job context is full
        printf("Job context is full, cannot add more jobs"); //print error message
        return;
    }
    global_job_context.jobs[global_job_context.job_count] = job; //Put the new job in the next open slot (which equals the job count)
    global_job_context.job_count++; //increment the job count for the next job
}

static int run_job(Ticktype_t current_time){

    for(size_t i = 0; i < global_job_context.job_count;i++){
        if(global_job_context.jobs[i]->execute_time <= current_time){ //if the job is ready to run
            return (int)i; //return the index of the job to be run
        }
    }
    return -1; //return -1 if no job is ready 
}

void run_scheduler(){

    for(;;){ //infinite loop to keep the scheduler running

        Ticktype_t current_time = xTaskGetTickCount(); //get the current time in ticks(FreeRTOS)

        int run_job = run_job(current_time);

        if(run_job < 0){ //if no job is ready to run
            vTaskDelay(pdMS_TO_TICKS(10)); //delay for a short period before checking again
            continue; //nothing to run
        }
        //if we get here, we have a job to run
        scheduler_t *job_to_run = global_job_context.jobs[run_job]; //get the job to run
        job_to_run->func(job_to_run->args); //execute the job function with arguments

        if(job_to_run->recurr_time != 0){ //check if the job is a recurring job 
            job_to_run->execute_time = current_time + job_to_run->recurr_time; //update the execute time for the next run
        }
        else{
            return; 
        }
    }
}