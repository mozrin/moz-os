#include "net.h"

extern void print_string(const char* str, int line);

void net_init() {
    print_string("kernel: networking stack initialized", 19);
}

uint16_t checksum(void* data, int len) {
    uint16_t* p = (uint16_t*)data;
    uint32_t sum = 0;
    
    while (len > 1) {
        sum += *p++;
        len -= 2;
    }
    
    if (len) {
        sum += *(uint8_t*)p;
    }
    
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return (uint16_t)~sum;
}


extern void uart_puts(const char* s);

void net_send(const char* data, int len) {
    /* For now, just wrap uart_puts, assuming string data */
    uart_puts(data);
}
