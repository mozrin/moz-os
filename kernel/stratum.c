#include "stratum.h"

extern void print_string(const char* str, int line);

void stratum_init() {
    /* No heavy initialization for skeleton yet */
}

stratum_job_t stratum_ingest() {
    print_string("kernel: stratum_ingest stub called", 22);
    stratum_job_t dummy = {0};
    return dummy;
}

int stratum_submit(stratum_share_t share) {
    print_string("kernel: stratum_submit stub called", 23);
    return 1; /* Success */
}
