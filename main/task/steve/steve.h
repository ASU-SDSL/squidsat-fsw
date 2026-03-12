#include <semphr.h>

#define MAX_STEVE_JOBS 16 // Maximum number of jobs 
#define MAX_STEVE_NAME 16 // Maximum length of job name
#define SCHEDULER_CHECK_DELAY_MS 500// Delay between scheduler checks in milliseconds


typedef struct steve_job {
    char name[MAX_STEVE_NAME]; 
    // TODO: set proper tick delay later
    uint32_t execute_job; //how many ticks until the job should be executed
    uint32_t recuring_job; //how many ticks until the job should be executed again 
    job_function function;
} steve_job_t;

typedef struct steve_scheduler {
    steve_job_t jobs[MAX_STEVE_JOBS]; //numbers of jobs
    size_t job_count; // Number of active jobs
} steve_scheduler_t;

//Scheduler global instance 
steve_scheduler_t g_steve_scheduler;

//Steve functions
void initialize_scheduler_job();
void create_scheduler_job(const char* job_name, uint8_t execute_time, uint8_t recur_time, job_function job_funct);
void run_cheduler_job(steve_job_t* job);
void delete_scheduler_job(steve_job_t* job);


//Main task
void steve_task(void* unused_args);