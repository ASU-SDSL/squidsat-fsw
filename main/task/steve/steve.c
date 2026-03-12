#include <FreeRTOS.h>
#include <steve.h>

//Adds a new job to the scheduler if there are available space. 
void create_cheduler_job(const* job_name, uint8_t execute_time, uint8_t recur_time, job_function job_func){
    //if current number of jobs reached maximum or more, stop adding more jobs 
    if(g_steve_scheduler >= MAX_STEVE_JOBS){
        return;
    }
    //get the address of the next empty slot
    steve_job_t* job = &g_steve_scheudler.jobs[g_steve_scheduler.job_count];
    //Fill out the struct
    strncpy(job->name, job_name, MAX_STEVE_NAME);
    job->executing_job = execute_time;
    job->recurring_job = recur_time;
    job->function = function;

}
//main function loop
void scheduler_task(void *unused_args) {
    while (true){
        
        }
    }