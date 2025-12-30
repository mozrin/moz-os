// kernel/kernel.c
#include <stdint.h>

#define VGA ((volatile uint16_t*)0xB8000)
static inline void putc(int row,int col,char c){ VGA[row*80+col]=(uint16_t)c | (0x07<<8); }
static inline void puts(int row, int col, const char* s){ for(int i=0;s[i];++i) putc(row,col+i,s[i]); }
__attribute__((noreturn)) void halt(void){ for(;;){ __asm__("hlt"); } }

static const uint32_t K[64]={
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};
static inline uint32_t rotr(uint32_t x,int n){ return (x>>n)|(x<<(32-n)); }
static inline uint32_t Ch(uint32_t x,uint32_t y,uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t Maj(uint32_t x,uint32_t y,uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t S0(uint32_t x){ return rotr(x,2)^rotr(x,13)^rotr(x,22); }
static inline uint32_t S1(uint32_t x){ return rotr(x,6)^rotr(x,11)^rotr(x,25); }
static inline uint32_t s0(uint32_t x){ return rotr(x,7)^rotr(x,18)^(x>>3); }
static inline uint32_t s1(uint32_t x){ return rotr(x,17)^rotr(x,19)^(x>>10); }

typedef struct { uint32_t h[8]; } sha256_ctx;
static void sha256_init(sha256_ctx* c){
  c->h[0]=0x6a09e667; c->h[1]=0xbb67ae85; c->h[2]=0x3c6ef372; c->h[3]=0xa54ff53a;
  c->h[4]=0x510e527f; c->h[5]=0x9b05688c; c->h[6]=0x1f83d9ab; c->h[7]=0x5be0cd19;
}
static void sha256_compress(sha256_ctx* c, const uint8_t block[64]){
  uint32_t w[64];
  for(int i=0;i<16;i++){
    w[i]=(block[4*i]<<24)|(block[4*i+1]<<16)|(block[4*i+2]<<8)|(block[4*i+3]);
  }
  for(int i=16;i<64;i++){ w[i]=s1(w[i-2])+w[i-7]+s0(w[i-15])+w[i-16]; }
  uint32_t a=c->h[0],b=c->h[1],c0=c->h[2],d=c->h[3],e=c->h[4],f=c->h[5],g=c->h[6],h=c->h[7];
  for(int i=0;i<64;i++){
    uint32_t T1=h+S1(e)+Ch(e,f,g)+K[i]+w[i];
    uint32_t T2=S0(a)+Maj(a,b,c0);
    h=g; g=f; f=e; e=d+T1; d=c0; c0=b; b=a; a=T1+T2;
  }
  c->h[0]+=a; c->h[1]+=b; c->h[2]+=c0; c->h[3]+=d; c->h[4]+=e; c->h[5]+=f; c->h[6]+=g; c->h[7]+=h;
}
static void dsha256(const uint8_t hdr[80], uint8_t out[32]){
  sha256_ctx ctx; sha256_init(&ctx);
  uint8_t b1[64]={0}, b2[64]={0};
  for(int i=0;i<64;i++) b1[i]=hdr[i];
  sha256_compress(&ctx,b1);
  uint8_t tail[64]={0};
  for(int i=0;i<16;i++) tail[i]=hdr[64+i];
  tail[16]=0x80;
  tail[63]=640 & 0xFF; tail[62]=(640>>8)&0xFF;
  sha256_compress(&ctx,tail);
  uint8_t d1[32];
  for(int i=0;i<8;i++){
    d1[4*i]=(ctx.h[i]>>24)&0xFF; d1[4*i+1]=(ctx.h[i]>>16)&0xFF; d1[4*i+2]=(ctx.h[i]>>8)&0xFF; d1[4*i+3]=ctx.h[i]&0xFF;
  }
  sha256_init(&ctx);
  uint8_t b3[64]={0};
  for(int i=0;i<32;i++) b3[i]=d1[i];
  b3[32]=0x80; b3[63]=256 & 0xFF; b3[62]=(256>>8)&0xFF;
  sha256_compress(&ctx,b3);
  for(int i=0;i<8;i++){
    out[4*i]=(ctx.h[i]>>24)&0xFF; out[4*i+1]=(ctx.h[i]>>16)&0xFF; out[4*i+2]=(ctx.h[i]>>8)&0xFF; out[4*i+3]=ctx.h[i]&0xFF;
  }
}

extern void kmain(void);
void kmain(void){
  puts(0,0,"MINING-OS: long mode OK");
  static uint8_t header[80] = { /* fill with a real header for testing */ };
  uint8_t hash[32];
  for(uint32_t nonce=0; nonce<0xFFFFFFFF; nonce++){
    header[76]=nonce & 0xFF;
    header[77]=(nonce>>8)&0xFF;
    header[78]=(nonce>>16)&0xFF;
    header[79]=(nonce>>24)&0xFF;
    dsha256(header, hash);
    if(hash[0]==0x00){ puts(2,0,"FOUND (toy)"); halt(); }
  }
  halt();
}
