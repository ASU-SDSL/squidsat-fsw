#include <FreeRTOS.h>
#include <steve.h>


//RTOS mutex declaration
static SemaphoreHandle_t scheduler_mutex;

void create_job(const char* job_name, uint8_t execute_time, uint8_t recur_time, job_function job_funct){
   //checks if the scheduler is full
   xSemaphoreTake(scheduler_mutex, portMAX_DELAY); //lock with mutex while checking job count
   if(scheduler.job_count >= MAX_JOBS){
       printf("Scheduler is full. Cannot add more jobs.\n");
       return;
   }
   xSemaphoreGive(scheduler_mutex);
   //check if the name is valid
   if(strlen(job_name) >= MAX_JOBS_NAME){
       printf("Job name is too long.\n");
       return;
   }
   //creates new job and adds it to the scheduler
   //use stack allocated memory for the new job
   jobs_t jobs;


void delete_job(jobs_t* job){

}

void initialize_job(){

}

void run_job(jobs_t* job){
    while(1){
        
    }
}