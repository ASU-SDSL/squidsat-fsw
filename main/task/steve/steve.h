#include <semphr.h>

#define MAX_STEVE_JOBS 16 // Maximum number of jobs 
#define MAX_STEVE_JOB_NAME 16 // Maximum length of job name
#define SCHEDULER_CHECK_DELAY_MS 500// Delay between scheduler checks in milliseconds

typedef struct steve_job {
    char name[MAX_STEVE_JOB_NAME]; // Name of the job 
    // TODO: set proper tick delay later
    uint32_t execute_job; //how many ticks until the job should be executed
    uint32_t recuring_job; //how many ticks until the job should be executed again 
    job_function function;

} steve_job_t;

typedef struct steve_scheduler_info {
    steve_job_t jobs[MAX_STEVE_JOBS]; //Array containing MAX_STEVE_JOB instances of the steve_job_t struct
    size_t job_count; // Number of active jobs
} steve_scheduler_info_t;

//Scheduler global instance 
steve_scheduler_info_t global_steve_info;

typedef struct CAN_message {
    uint32_t can_id; // CAN identifier
    uint32_t can_length; // Length of the CAN data
    uint8_t can_data[8]; // CAN data (up to 8 bytes)
} CAN_message_t;

//Steve functions
void initialize_steve_scheduler();
void create_steve_scheduler_job(const char* name, uint32_t execute_job, uint32_t recuring_job, job_function function);
void run_steve_scheduler(steve_job_t* job);
void delete_steve_scheduler_job(steve_job_t* job);

//Steve managing functions
void kill_steve_scheduler(char* job_name);

//CAN functions
void scheduler_can_tx_message(const CAN_message_t* message); //send CAN message
void scheduler_can_rx_message(CAN_message_t* message); //recive CAN message

//Main task
void steve_task(void* unused_args);