#include <FreeRTOS.h>
#include <steve.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <log.h>
#include <sensor_job.h>


job_context_t global_job_context;  


void add_job(jobs_t *job){

    if(job == NULL){ //if the job is null, we cannot add it to the job context
        log_error("Invalid job, cannot add to job context"); //print error message
        return; //invalid job
    }
    if(global_job_context.job_count >= MAX_JOBS){ //job context is full
        log_error("Job context is full, cannot add more jobs"); //print error message
        return;
    }
    global_job_context.jobs[global_job_context.job_count] = job; //Put the new job in the next open slot (which equals the job count)
    global_job_context.job_count++; //increment the job count for the next job
}

static int find_ready_job(TickType_t current_time){

    for(size_t i = 0; i < global_job_context.job_count;i++){
        if(global_job_context.jobs[i]->execute_time <= current_time){ //if the job is ready to run
            return (int)i; //return the index of the job to be run
        }
    }
    return -1; //return -1 if no job is ready 
}

static void reorganize_job(size_t job_index){
    for(size_t i = job_index;i+1 < global_job_context.job_count;i++){//check if the next slot is available
        global_job_context.jobs[i] = global_job_context.jobs[i+1]; //shift jobs to the left    
    }
    global_job_context.job_count--;
}

void delete_job(jobs_t * job){
    for(int i = 0; i <  global_job_context.job_count;i++){ //itterate through the jobs
        if(global_job_context.jobs[i] == job){ // check if it is the job we want to delete
            global_job_context.jobs[i] = NULL; //set it to null to remove job
            reorganize_job(i);
        }
    }
    log_error("job not found");
}

void scheduler_init(void) {
    global_job_context.job_count = 0; //initialize 
}

void run_scheduler(){

    for(;;){ //infinite loop to keep the scheduler running

        TickType_t current_time = xTaskGetTickCount(); //get the current time in ticks(FreeRTOS)

        int run_index = find_ready_job(current_time);

        if(run_index < 0){ //if no job is ready to run
            vTaskDelay(pdMS_TO_TICKS(10)); //delay for a short period before checking again
            continue; //nothing to run
        }
        //run job
        jobs_t *job_to_run = global_job_context.jobs[run_index]; //get the job to run
        job_to_run->func(job_to_run->args); //execute the job function with arguments

        if(job_to_run->recurr_time != 0){ //check if the job is a recurring job 
            job_to_run->execute_time = current_time + job_to_run->recurr_time; //update the execute time for the next run
        }
        else{
            printf("deleting '%s', count before=%d", job_to_run->name, global_job_context.job_count);
            delete_job(job_to_run); 
            //printf("after delete, count=%d", global_job_context.job_count);
        }
    }
}

void scheduler_task(void *pvParameters){
    scheduler_init();
    sensor_setup();
    add_job(&led_blinking); 
    add_job(&heart_beat);
    run_scheduler();

}
