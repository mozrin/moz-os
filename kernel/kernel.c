/*
* kernel.c - Minimal C Kernel for Moz-OS with SHA-256
*/

#include <stdint.h>

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

/* -------------------------------------------------------------------------- */
/* SHA-256 Constants and Helpers                                              */
/* -------------------------------------------------------------------------- */

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define SHR(x, n)  ((x) >> (n))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define BSIG0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define BSIG1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SSIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define SSIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

void sha256_transform(uint32_t state[8], const uint8_t data[64]) {
    uint32_t W[64];
    uint32_t a, b, c, d, e, f, g, h, T1, T2;
    int i;
    
    /* Prepare message schedule (Big Endian load) */
    for (i = 0; i < 16; i++) {
        W[i] = ((uint32_t)data[i * 4] << 24) |
               ((uint32_t)data[i * 4 + 1] << 16) |
               ((uint32_t)data[i * 4 + 2] << 8) |
               ((uint32_t)data[i * 4 + 3]);
    }
    
    for (i = 16; i < 64; i++) {
        W[i] = SSIG1(W[i - 2]) + W[i - 7] + SSIG0(W[i - 15]) + W[i - 16];
    }
    
    /* Initialize working variables */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    f = state[5];
    g = state[6];
    h = state[7];
    
    /* Main Loop */
    for (i = 0; i < 64; i++) {
        T1 = h + BSIG1(e) + CH(e, f, g) + K[i] + W[i];
        T2 = BSIG0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }
    
    /* Add to state */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

void kmain() {
    /* Print banner at line 7 */
    print_string("kernel: moz-os skeleton entry", 7);
    
    /* SHA-256 Validation for "abc" */
    /* "abc" = 0x61, 0x62, 0x63, 0x80 (padding start), then zeros, then length bits at end */
    /* length = 24 bits = 0x18 */
    /* block[64] */
    uint8_t block[64];
    int i;
    /* Initialize block with zeros */
    for(i=0; i<64; i++) block[i] = 0;
    
    /* Set "abc" and padding */
    block[0] = 'a';
    block[1] = 'b';
    block[2] = 'c';
    block[3] = 0x80;
    /* Length in bits (Big Endian) 64-bit size at end */
    block[63] = 24; 
    
    /* Initial Hash Values */
    uint32_t H[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    
    /* Process Block */
    sha256_transform(H, block);
    
    /* Expected Hash ("abc") */
    /* ba7816bf 8f01cfea 414140de 5dae2223 b00361a3 96177a9c b410ff61 f20015ad */
    uint32_t expected[8] = {
        0xba7816bf, 0x8f01cfea, 0x414140de, 0x5dae2223,
        0xb00361a3, 0x96177a9c, 0xb410ff61, 0xf20015ad
    };
    
    /* Verify */
    int valid = 1;
    for(i=0; i<8; i++) {
        if (H[i] != expected[i]) {
            valid = 0;
            break;
        }
    }
    
    if (valid) {
        print_string("kernel: sha-256 compression validated", 8);
    } else {
        print_string("kernel: sha-256 validation FAILED", 8);
    }

    /* Halt */
    while (1) {
        __asm__("hlt");
    }
}
