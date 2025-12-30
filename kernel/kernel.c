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

/* 
 * Helper to write 32-bit word in Big Endian to byte array 
 */
void write_be32(uint8_t* buf, uint32_t val) {
    buf[0] = (val >> 24) & 0xff;
    buf[1] = (val >> 16) & 0xff;
    buf[2] = (val >> 8) & 0xff;
    buf[3] = (val) & 0xff;
}

/*
 * dsha256 - Double SHA-256 for 80-byte Bitcoin Header
 * Input: header[80]
 * Output: hash[32]
 */
void dsha256(const uint8_t header[80], uint8_t output[32]) {
    uint32_t H[8];
    uint8_t block[64];
    int i;
    
    /* ------------------------------------------------------------- */
    /* PASS 1: SHA-256(header) */
    /* ------------------------------------------------------------- */
    
    /* Init H */
    H[0] = 0x6a09e667; H[1] = 0xbb67ae85; H[2] = 0x3c6ef372; H[3] = 0xa54ff53a;
    H[4] = 0x510e527f; H[5] = 0x9b05688c; H[6] = 0x1f83d9ab; H[7] = 0x5be0cd19;
    
    /* Block 1: Header bytes 0..63 */
    for(i=0; i<64; i++) block[i] = header[i];
    sha256_transform(H, block);
    
    /* Block 2: Header bytes 64..79, then Padding */
    for(i=0; i<64; i++) block[i] = 0;
    
    /* Copy remaining 16 bytes */
    for(i=0; i<16; i++) block[i] = header[64 + i];
    
    /* Padding: 1 bit (0x80) */
    block[16] = 0x80;
    
    /* Length: 80 bytes * 8 = 640 bits = 0x0000000000000280 */
    /* Put at end (Big Endian) */
    block[62] = 0x02;
    block[63] = 0x80;
    sha256_transform(H, block);
    
    /* Result of Pass 1 (32 bytes) */
    /* We need to feed this into Pass 2. 
       Note: SHA-256 state H is 8x32-bit words. 
       We need to serialize them Big Endian to feed into next hash. 
    */
    uint8_t hash1[32];
    for (i=0; i<8; i++) {
        write_be32(&hash1[i*4], H[i]);
    }

    /* ------------------------------------------------------------- */
    /* PASS 2: SHA-256(hash1) */
    /* ------------------------------------------------------------- */
    
    /* Init H again */
    H[0] = 0x6a09e667; H[1] = 0xbb67ae85; H[2] = 0x3c6ef372; H[3] = 0xa54ff53a;
    H[4] = 0x510e527f; H[5] = 0x9b05688c; H[6] = 0x1f83d9ab; H[7] = 0x5be0cd19;
    
    /* Block 1: 32 bytes data + Padding */
    /* Length = 32 * 8 = 256 bits = 0x0100 */
    
    for(i=0; i<64; i++) block[i] = 0;
    
    /* Copy 32 bytes */
    for(i=0; i<32; i++) block[i] = hash1[i];
    
    /* Padding */
    block[32] = 0x80;
    
    /* Length at end */
    block[62] = 0x01;
    block[63] = 0x00;
    
    sha256_transform(H, block);
    
    /* Result to output */
    for (i=0; i<8; i++) {
        write_be32(&output[i*4], H[i]);
    }
}

/*
 * check_target - Compare hash against target (Little Endian comparison)
 * Returns 1 if hash <= target, 0 otherwise.
 * Indices 31 is MSB, 0 is LSB.
 */
int check_target(const uint8_t hash[32], const uint8_t target[32]) {
    int i;
    for (i = 31; i >= 0; i--) {
        if (hash[i] < target[i]) {
            return 1; /* Less than target -> Valid */
        }
        if (hash[i] > target[i]) {
            return 0; /* Greater than target -> Invalid */
        }
        /* If equal, continue to next byte */
    }
    return 1; /* Equal -> Valid */
}

void kmain() {
    /* Print banner at line 7 */
    print_string("kernel: moz-os skeleton entry", 7);
    
    /* --------------------------------------------------------------------- */
    /* Test Vector: Genesis Block Header (80 bytes) */
    /* --------------------------------------------------------------------- */
    uint8_t genesis_header[80] = {
        /* Version: 1 */
        0x01, 0x00, 0x00, 0x00,
        /* Prev Block: 0 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        /* Merkle Root: 4a5e1e... */
        0x3b, 0xa3, 0xed, 0xfd, 0x7a, 0x7b, 0x12, 0xb2, 0x7a, 0xc7, 0x2c, 0x3e,
        0x67, 0x76, 0x8f, 0x61, 0x7f, 0xc8, 0x1b, 0xc3, 0x88, 0x8a, 0x51, 0x32,
        0x3a, 0x9f, 0xb8, 0xaa, 0x4b, 0x1e, 0x5e, 0x4a,
        /* Time: 1231006505 (0x495fab29) -> LE in header: 29 ab 5f 49 */
        0x29, 0xab, 0x5f, 0x49,
        /* Bits: 0x1d00ffff -> LE: ff ff 00 1d */
        0xff, 0xff, 0x00, 0x1d,
        /* Nonce: 2083236893 (0x7c2bac1d) -> LE: 1d ac 2b 7c */
        0x1d, 0xac, 0x2b, 0x7c
    };
    
    /* Expected Double Hash (Big Endian) */
    /* 000000000019d6... (LE) -> Ends with 19 00 ... */
    /* Actual Bytes: 6f e2 8c 0a b6 f1 b3 72 c1 a6 a2 46 ae 63 f7 4f 93 1e 83 65 e1 5a 08 9c 68 d6 19 00 00 00 00 00 */
    uint8_t expected_hash[32] = {
        0x6f, 0xe2, 0x8c, 0x0a, 0xb6, 0xf1, 0xb3, 0x72,
        0xc1, 0xa6, 0xa2, 0x46, 0xae, 0x63, 0xf7, 0x4f,
        0x93, 0x1e, 0x83, 0x65, 0xe1, 0x5a, 0x08, 0x9c,
        0x68, 0xd6, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    
    uint8_t result[32];
    dsha256(genesis_header, result);
    
    int valid = 1;
    int i;
    for(i=0; i<32; i++) {
        if(result[i] != expected_hash[i]) {
            valid = 0;
            break;
        }
    }
    
    if (valid) {
        print_string("kernel: sha-256 compression validated", 8); /* Kept old banner */
        print_string("kernel: double sha-256 validated", 9);      /* New banner */
    } else {
        print_string("kernel: double sha-256 FAILED", 9);
    }
    
    /* --------------------------------------------------------------------- */
    /* Mining Loop (Full Target Check) with Midstate Optimization */
    /* --------------------------------------------------------------------- */
    
    print_string("kernel: midstate optimization active", 11);

    /* Define Target */
    /* Based on Genesis Hash: Bytes 26=0x19, 27..31=0x00 */
    /* Set Target slightly higher: Byte 26=0x20, 27..31=0x00, rest 0xFF */
    uint8_t target[32];
    for(i=0; i<32; i++) target[i] = 0xff;
    /* High bytes (LE index 27..31) must be 0 to match genesis difficulty approx */
    target[31] = 0x00;
    target[30] = 0x00;
    target[29] = 0x00;
    target[28] = 0x00;
    target[27] = 0x00;
    /* Byte 26: Genesis is 0x19. Target 0x20 is easier. */
    target[26] = 0x20;

    /* 1. Compute Midstate (SHA-256 state after first 64 bytes) */
    uint32_t midstate[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    uint8_t block1[64];
    for(i=0; i<64; i++) block1[i] = genesis_header[i];
    sha256_transform(midstate, block1);
    
    /* 2. Prepare Tail Block (Bytes 64-79 + Padding) */
    uint8_t tail_block[64];
    for(i=0; i<64; i++) tail_block[i] = 0;
    for(i=0; i<16; i++) tail_block[i] = genesis_header[64+i];
    tail_block[16] = 0x80;
    tail_block[62] = 0x02;
    tail_block[63] = 0x80;
    
    /* We reuse genesis_header structure for reference, but update tail_block directly */
    uint32_t nonce = 0;
    uint32_t H[8];
    uint8_t hash1[32];
    uint8_t final_hash[32];
    
    while (nonce < 0xffffffff) {
        /* Update Nonce in Tail Block */
        /* Nonce is at offset 76 in header. 
           In tail_block (which starts at header[64]), offset is 76-64 = 12. 
        */
        tail_block[12] = (nonce) & 0xff;
        tail_block[13] = (nonce >> 8) & 0xff;
        tail_block[14] = (nonce >> 16) & 0xff;
        tail_block[15] = (nonce >> 24) & 0xff;
        
        /* Copy Midstate */
        for(i=0; i<8; i++) H[i] = midstate[i];
        
        /* Pass 1 (Tail) */
        sha256_transform(H, tail_block);
        
        /* Serialize Pass 1 result for Pass 2 */
        for (i=0; i<8; i++) write_be32(&hash1[i*4], H[i]);
        
        /* Pass 2 (SHA-256 of hash1) */
        H[0] = 0x6a09e667; H[1] = 0xbb67ae85; H[2] = 0x3c6ef372; H[3] = 0xa54ff53a;
        H[4] = 0x510e527f; H[5] = 0x9b05688c; H[6] = 0x1f83d9ab; H[7] = 0x5be0cd19;
        
        /* Prepare Pass 2 Block */
        uint8_t p2_block[64];
        for(i=0; i<64; i++) p2_block[i] = 0;
        for(i=0; i<32; i++) p2_block[i] = hash1[i];
        p2_block[32] = 0x80;
        p2_block[62] = 0x01;
        p2_block[63] = 0x00;
        
        sha256_transform(H, p2_block);
        
        /* Serialize Final Hash */
        for (i=0; i<8; i++) write_be32(&final_hash[i*4], H[i]);
        
        /* Check Target (Full 256-bit) */
        if (check_target(final_hash, target)) {
            print_string("kernel: valid solution meets target", 12);
            break;
        }
        
        nonce++;
    }
    
    /* Halt */
    while (1) {
        __asm__("hlt");
    }
}
