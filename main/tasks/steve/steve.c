#include <FreeRTOS.h>
#include <steve.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <log.h>
#include <sensor_job.h>


job_context_t global_job_context;  
static QueueHandle_t job_queue;
static SemaphoreHandle_t job_mutex;


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

void manager_task(){
    jobs_t *job_done;

    for(;;){
        TickType_t current_time = xTaskGetTickCount(); //get the current time in ticks(FreeRTOS)

        while(xQueueReceive(job_queue, &job_done,0) == pdTRUE){

            if(job_done->recurr_time != 0){
                job_done->execute_time = current_time + job_done->recurr_time;
            }
            else{
                delete_job(job_done);
            }
        }
        int run_index = find_ready_job(current_time);
        if(run_index < 0){
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        jobs_t *job_to_run = global_job_context.jobs[run_index];
        
    }
}
void worker_task(){
    jobs_t *job;

    for(;;){
        xQueueReceive(job_queue, &job, portMAX_DELAY);
        job->func(job->args); //run jobs
        xQueueSend(job_queue, &job, portMAX_DELAY);
    }
}

void scheduler_task(void *pvParameters){
    scheduler_init();
    sensor_setup();
    add_job(&led_blinking); 
    add_job(&heart_beat);
    worker_task();

}
