#include <FreeRTOS.h>
#include <steve.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>


//RTOS mutex declaration
static SemaphoreHandle_t scheduler_mutex;

void create_job(const char* job_name, uint8_t execute_job, uint8_t recur_job, job_function job_funct){
   //checks if the scheduler is full
   if(scheduler.job_count >= MAX_JOBS){
       printf("Scheduler is full. Cannot add more jobs.\n");
       return;
   }
   //check if the name is valid
   if(strlen(job_name) >= MAX_JOBS_NAME){
       printf("Job name is too long.\n");
       return;
   }
   //creates new job and adds it to the scheduler
   //use stack allocated memory for the new job
   jobs_t jobs;
   strncpy(jobs.name, job_name, MAX_JOBS_NAME);
   jobs.execute_job = execute_job
   jobs.recurring_job = recur_job;
   jobs.function = job_funct;

   //add jobs to the scheduler
   scheduler.jobs[scheduler.job_count++] = jobs;

}

void delete_job(jobs_t* job){
    bool job_found = false; //flag to check if the job is found in the scheduler
    //iterate through the scheduler to find the job and delete it
    for(int i  = 0; i < scheduler.job_count;i++){ 
        if(scheduler.jobs[i] == job){ 
            //if job is found, delete it by setting it to NULL
            scheduler.jobs[i] = NULL;
            job_found = true;   
            break;
        }
    }
    //if job is not found, print error message
    if(!job_found){
        printf("Job not found. Cannot delete job.\n");
        return;
    }
    //clean up the scheduler by shifting the jobs 
    for(int i = 0; i < scheduler.job_count; i++){
        if(scheduler.jobs[i] == NULL){ //check if the job is empty
            scheduler.jobs[i] = scheduler.jobs[scheduler.job_count - 1]; 
            scheduler.jobs[scheduler.job_count - 1] = NULL; //set the last job to NULL
            scheduler.job_count--; //decrease the job count 
        }
    }
}

void initialize_job(){
    //set job count to 0
    scheduler.job_count = 0;
    for(int i = 0; i < MAX_JOBS; i++){
        memset(scheduler.jobs[i].name, 0, MAX_JOBS_NAME);//reset the name of the job
        scheduler.jobs[i].execute_job = 0; //reset the execute time of the job
        scheduler.jobs[i].recuring_job = 0; //reset the recurring time of the job
        scheduler.jobs[i].function = NULL;  //reset the function pointer of the job
    }
}

uint32_t get_uptime(){
    //get the current tick count
    return xTaskGetTickCount();
}

void run_job(void *unused_arg){
    initialize_job(); //initialize the scheduler
    while (1) {
    for (int i = 0; i < g_steve_context.job_count; i++) {
        steve_job_t *job = &g_steve_context.jobs[i];

        if (get_uptime() >= job->execute_time) {
            // Run the job 
            job->func_ptr(job->data);

            if (job->recur_time > 0) {
                job->execute_time = get_uptime() + job->recur_time;
            } else {
                delete_steve_job(i);
                i--;
            }
        }
    }
}
}