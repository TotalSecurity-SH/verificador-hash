#ifndef HASH_H
#define HASH_H

/* Implementaciones sin libc para compilar a WebAssembly (wasm32) y probar en nativo. */

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef unsigned long usize;

typedef struct { u32 state[4]; u64 len; u8 buf[64]; usize used; } md5_ctx;
typedef struct { u32 state[5]; u64 len; u8 buf[64]; usize used; } sha1_ctx;
typedef struct { u32 state[8]; u64 len; u8 buf[64]; usize used; } sha256_ctx;
typedef struct { u64 state[8]; u64 len; u8 buf[128]; usize used; } sha512_ctx;

void md5_init(md5_ctx *c);
void md5_update(md5_ctx *c, const u8 *data, usize n);
void md5_final(md5_ctx *c, u8 out[16]);

void sha1_init(sha1_ctx *c);
void sha1_update(sha1_ctx *c, const u8 *data, usize n);
void sha1_final(sha1_ctx *c, u8 out[20]);

void sha256_init(sha256_ctx *c);
void sha256_update(sha256_ctx *c, const u8 *data, usize n);
void sha256_final(sha256_ctx *c, u8 out[32]);

void sha512_init(sha512_ctx *c);
void sha512_update(sha512_ctx *c, const u8 *data, usize n);
void sha512_final(sha512_ctx *c, u8 out[64]);

#endif
