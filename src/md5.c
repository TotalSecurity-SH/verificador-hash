#include "hash.h"

/* MD5 · RFC 1321. Solo para verificar integridad: no usar para seguridad. */

#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static const u32 K[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

static const u8 S[64] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                         5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
                         4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                         6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

static void block(md5_ctx *c, const u8 *p) {
    u32 m[16], a = c->state[0], b = c->state[1], cc = c->state[2], d = c->state[3];
    for (int i = 0; i < 16; i++)
        m[i] = (u32)p[i * 4] | ((u32)p[i * 4 + 1] << 8) | ((u32)p[i * 4 + 2] << 16) | ((u32)p[i * 4 + 3] << 24);
    for (int i = 0; i < 64; i++) {
        u32 f;
        int g;
        if (i < 16) { f = (b & cc) | (~b & d); g = i; }
        else if (i < 32) { f = (d & b) | (~d & cc); g = (5 * i + 1) % 16; }
        else if (i < 48) { f = b ^ cc ^ d; g = (3 * i + 5) % 16; }
        else { f = cc ^ (b | ~d); g = (7 * i) % 16; }
        u32 tmp = d;
        d = cc;
        cc = b;
        b = b + ROTL(a + f + K[i] + m[g], S[i]);
        a = tmp;
    }
    c->state[0] += a; c->state[1] += b; c->state[2] += cc; c->state[3] += d;
}

void md5_init(md5_ctx *c) {
    c->state[0] = 0x67452301; c->state[1] = 0xefcdab89; c->state[2] = 0x98badcfe; c->state[3] = 0x10325476;
    c->len = 0; c->used = 0;
}

void md5_update(md5_ctx *c, const u8 *data, usize n) {
    c->len += n;
    while (n) {
        if (c->used == 0 && n >= 64) { block(c, data); data += 64; n -= 64; continue; }
        c->buf[c->used++] = *data++;
        n--;
        if (c->used == 64) { block(c, c->buf); c->used = 0; }
    }
}

void md5_final(md5_ctx *c, u8 out[16]) {
    u64 bits = c->len * 8;
    u8 pad = 0x80;
    md5_update(c, &pad, 1);
    pad = 0;
    while (c->used != 56) md5_update(c, &pad, 1);
    for (int i = 0; i < 8; i++) c->buf[56 + i] = (u8)(bits >> (8 * i)); /* little endian */
    block(c, c->buf);
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) out[i * 4 + j] = (u8)(c->state[i] >> (8 * j));
}
