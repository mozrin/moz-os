#ifndef STRATUM_H
#define STRATUM_H

#include <stdint.h>

typedef struct {
    uint8_t header[80];
    uint8_t target[32];
    uint32_t job_id;
} stratum_job_t;

typedef struct {
    uint32_t nonce;
    uint8_t hash[32];
    uint32_t job_id;
} stratum_share_t;

void stratum_init();
stratum_job_t stratum_ingest();
void stratum_ingest_job(uint8_t* header, uint8_t* target);
int stratum_submit(stratum_share_t share);

#endif
