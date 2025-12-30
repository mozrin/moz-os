/*
* kernel.c - Minimal C Kernel for Moz-OS
*/

/* volatile to prevent compiler optimization */
volatile char* video_memory = (volatile char*) 0xb8000;

void print_string(const char* str, int line) {
    int offset = line * 160;
    int i = 0;
    while (str[i] != 0) {
        video_memory[offset + i * 2] = str[i];
        video_memory[offset + i * 2 + 1] = 0x0f; /* White on Black */
        i++;
    }
}

void kmain() {
    /* Print banner at line 7 */
    print_string("kernel: moz-os skeleton entry", 7);

    /* Halt */
    while (1) {
        __asm__("hlt");
    }
}
