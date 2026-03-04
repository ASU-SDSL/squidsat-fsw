#include <FreeRTOS.h>
#include <steve.h>

void scheduler_task(void *unused_args) {
    initialize steve_scheduler();

    while (true){
        for(int i = 0;i < global_steve_info.job_count;i++){
            //get the job index
            steve_job_t* jobs = &global_steve_info.jobs[i];
            //check job is ready to run
            if(jobs->function != NULL){
                run_jobs(jobs);
            }
        }

        sleep_ms(SCHEDULER_CHECK_DELAY_MS);
    }
}