#include <stdint.h>
#include <stddef.h>

#define MAX_JOBS 16 // Maximum number of jobs 
#define MAX_JOBS_NAME 16 // Maximum length of job name
#define CHECK_DELAY_MS 500// Delay between scheduler 


typedef void (*job_function)(void);

//one single job function
typedef struct jobs {
    char name[MAX_JOBS_NAME]; 
    uint32_t execute_job; //how many ticks until the job should be executed
    uint32_t recuring_job; //how many ticks until the job should be executed again 
    job_function function;
    void* data; //pointer to data 
} jobs_t;

//scheduler struct for multiple jobs
typedef struct steve_scheduler {
    jobs_t jobs[MAX_JOBS]; //numbers of jobs
    size_t job_count; // Number of active jobs
} scheduler_t;

scheduler_t scheduler; // Global scheduler instance

//Scheduler functions
void initialize_job();
void create_job(const char* job_name, uint8_t execute_time, uint8_t recur_time, job_function job_funct);
void delete_job(jobs_t* job);
void run_job(void *unused_arg);
uint32_t get_uptime();