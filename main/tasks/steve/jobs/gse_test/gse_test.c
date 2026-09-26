#include "gse_test.h"

void steve_test(void *args){
    (void) args;

    log_info("Steve works on multiple task (Core Safe)");
}

jobs_t test_steve = {
    .func = steve_test,
    .recurr_time = 1000,
    .execute_time = 0,
    .name = "GSE Print Task",
    .args = NULL
};
