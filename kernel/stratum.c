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

extern void net_send(const char* data, int len);
extern void uart_putc(char c);

static void put_digit(char* buf, int* idx, uint8_t d) {
    if (d < 10) buf[(*idx)++] = '0' + d;
    else buf[(*idx)++] = 'a' + (d - 10);
}

static void put_hex(char* buf, int* idx, uint32_t val) {
    int i;
    for(i=28; i>=0; i-=4) {
        put_digit(buf, idx, (val >> i) & 0xF);
    }
}

static void put_hex_bytes(char* buf, int* idx, const uint8_t* src, int len) {
    int i;
    for(i=0; i<len; i++) {
        put_digit(buf, idx, (src[i] >> 4) & 0xF);
        put_digit(buf, idx, src[i] & 0xF);
    }
}

void stratum_submit_share(uint32_t nonce, const uint8_t* hash) {
    char msg[256];
    int idx = 0;
    
    /* {"nonce":" */
    const char* p1 = "{\"nonce\":\"";
    int i = 0;
    while(p1[i]) msg[idx++] = p1[i++];
    
    /* NONCE HEX */
    put_hex(msg, &idx, nonce);
    
    /* ","hash":" */
    const char* p2 = "\",\"hash\":\"";
    i = 0;
    while(p2[i]) msg[idx++] = p2[i++];
    
    /* HASH HEX */
    put_hex_bytes(msg, &idx, hash, 32);
    
    /* "}\n */
    const char* p3 = "\"}\n";
    i = 0;
    while(p3[i]) msg[idx++] = p3[i++];
    
    msg[idx] = 0;
    
    net_send(msg, idx);
    print_string("kernel: stratum share submitted", 25);
}

int stratum_submit(stratum_share_t share) {
    print_string("kernel: stratum_submit stub called", 23);
    return 1; /* Success */
}
