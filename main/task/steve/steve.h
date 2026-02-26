#include <semphr.h>

#define MAX_STEVE_JOBS 16 // Maximum number of jobs 
#define MAX_STEVE_JOB_NAME 16 // Maximum length of job name
#define SCHEDULER_CHECK_DELAY_MS  // Delay between scheduler checks in milliseconds

typedef struct steve_job {
    char name[MAX_STEVE_JOB_NAME]; // Name of the job 
    TickType_t execute_job;
    TickType_t reccuring_job;
    job_function function;

} steve_job_t;

typedef struct steve_scheduler {
    steve_job_t jobs[MAX_STEVE_JOBS]; //Array containing MAX_STEVE_JOB instances of the steve_job_t struct
    size_t job_count; // Number of active jobs
} steve_scheduler_t;

//Steve functions
void initialize_steve_scheduler();
void create_steve_scheduler_job(const char* name, TickType_t execute_job, TickType_t reccuring_job, job_function function);
void run_steve_scheduler(steve_job_t* job);
void delete_steve_scheduler_job(steve_job_t* job);

//Steve managing functions
void kill_steve_scheduler(char* job_name);

//Main task
void steve_task(void* unused_args);