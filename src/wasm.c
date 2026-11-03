#include "hash.h"

/*
 * Interfaz exportada a JavaScript.
 * JS copia cada trozo del archivo en `buffer` y llama a hash_update; al final lee los digest de `output`.
 */

#define BUFFER_SIZE (1 << 20) /* 1 MiB por trozo */

enum { ALG_MD5 = 1, ALG_SHA1 = 2, ALG_SHA256 = 4, ALG_SHA512 = 8 };

static u8 buffer[BUFFER_SIZE];
static u8 output[16 + 20 + 32 + 64];
static u32 active;

static md5_ctx md5;
static sha1_ctx sha1;
static sha256_ctx sha256;
static sha512_ctx sha512;

__attribute__((export_name("buffer_ptr"))) u8 *buffer_ptr(void) { return buffer; }
__attribute__((export_name("buffer_size"))) u32 buffer_size(void) { return BUFFER_SIZE; }
__attribute__((export_name("output_ptr"))) u8 *output_ptr(void) { return output; }

__attribute__((export_name("hash_init"))) void hash_init(u32 algorithms) {
    active = algorithms;
    if (active & ALG_MD5) md5_init(&md5);
    if (active & ALG_SHA1) sha1_init(&sha1);
    if (active & ALG_SHA256) sha256_init(&sha256);
    if (active & ALG_SHA512) sha512_init(&sha512);
}

__attribute__((export_name("hash_update"))) void hash_update(u32 len) {
    if (len > BUFFER_SIZE) len = BUFFER_SIZE;
    if (active & ALG_MD5) md5_update(&md5, buffer, len);
    if (active & ALG_SHA1) sha1_update(&sha1, buffer, len);
    if (active & ALG_SHA256) sha256_update(&sha256, buffer, len);
    if (active & ALG_SHA512) sha512_update(&sha512, buffer, len);
}

/* Escribe los digest en orden fijo: MD5 (16) · SHA-1 (20) · SHA-256 (32) · SHA-512 (64). */
__attribute__((export_name("hash_final"))) void hash_final(void) {
    if (active & ALG_MD5) md5_final(&md5, output);
    if (active & ALG_SHA1) sha1_final(&sha1, output + 16);
    if (active & ALG_SHA256) sha256_final(&sha256, output + 36);
    if (active & ALG_SHA512) sha512_final(&sha512, output + 68);
}
