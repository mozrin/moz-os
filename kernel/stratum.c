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

static uint8_t parse_hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

static void parse_hex_string(const char* src, uint8_t* dst, int bytes) {
    int i;
    for (i = 0; i < bytes; i++) {
        dst[i] = (parse_hex_digit(src[i*2]) << 4) | parse_hex_digit(src[i*2+1]);
    }
}

extern uint8_t uart_getc();

void stratum_ingest_job(uint8_t* header, uint8_t* target) {
    print_string("kernel: waiting for stratum job...", 14);
    
    /* Read into buffer until '}' */
    char buf[512];
    int idx = 0;
    char c;
    while(1) {
        c = uart_getc();
        if (idx < 511) buf[idx++] = c;
        if (c == '}') break;
    }
    buf[idx] = 0;
    
    /* Minimal implementation: Find "header":" and "target":" */
    /* We assume the structure is fixed for this phase: */
    /* {"header":"<160_HEX>","target":"<64_HEX>"} */
    
    /* Find header val */
    int i;
    char* hdr_ptr = 0;
    char* tgt_ptr = 0;
    
    for(i=0; i<idx-10; i++) {
        /* check for "header":" */
        if (buf[i] == '"' && buf[i+1] == 'h' && buf[i+2] == 'e' && 
            buf[i+8] == '"' && buf[i+9] == ':') {
             if (buf[i+10] == '"') hdr_ptr = &buf[i+11];
        }
        /* check for "target":" */
        if (buf[i] == '"' && buf[i+1] == 't' && buf[i+2] == 'a' && 
            buf[i+8] == '"' && buf[i+9] == ':') {
             if (buf[i+10] == '"') tgt_ptr = &buf[i+11];
        }
    }
    
    if (hdr_ptr) {
        parse_hex_string(hdr_ptr, header, 80);
    }
    
    if (tgt_ptr) {
        parse_hex_string(tgt_ptr, target, 32);
    }
    
    print_string("kernel: stratum job received", 21);
}

int stratum_submit(stratum_share_t share) {
    print_string("kernel: stratum_submit stub called", 23);
    return 1; /* Success */
}
