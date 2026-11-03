#include "hash.h"

/* SHA-1 · FIPS 180-4. Incluido por compatibilidad con sumas publicadas; preferir SHA-256. */

#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static void block(sha1_ctx *c, const u8 *p) {
    u32 w[80];
    for (int i = 0; i < 16; i++)
        w[i] = ((u32)p[i * 4] << 24) | ((u32)p[i * 4 + 1] << 16) | ((u32)p[i * 4 + 2] << 8) | p[i * 4 + 3];
    for (int i = 16; i < 80; i++) w[i] = ROTL(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    u32 a = c->state[0], b = c->state[1], cc = c->state[2], d = c->state[3], e = c->state[4];
    for (int i = 0; i < 80; i++) {
        u32 f, k;
        if (i < 20) { f = (b & cc) | (~b & d); k = 0x5a827999; }
        else if (i < 40) { f = b ^ cc ^ d; k = 0x6ed9eba1; }
        else if (i < 60) { f = (b & cc) | (b & d) | (cc & d); k = 0x8f1bbcdc; }
        else { f = b ^ cc ^ d; k = 0xca62c1d6; }
        u32 t = ROTL(a, 5) + f + e + k + w[i];
        e = d; d = cc; cc = ROTL(b, 30); b = a; a = t;
    }
    c->state[0] += a; c->state[1] += b; c->state[2] += cc; c->state[3] += d; c->state[4] += e;
}

void sha1_init(sha1_ctx *c) {
    c->state[0] = 0x67452301; c->state[1] = 0xefcdab89; c->state[2] = 0x98badcfe;
    c->state[3] = 0x10325476; c->state[4] = 0xc3d2e1f0;
    c->len = 0; c->used = 0;
}

void sha1_update(sha1_ctx *c, const u8 *data, usize n) {
    c->len += n;
    while (n) {
        if (c->used == 0 && n >= 64) { block(c, data); data += 64; n -= 64; continue; }
        c->buf[c->used++] = *data++;
        n--;
        if (c->used == 64) { block(c, c->buf); c->used = 0; }
    }
}

void sha1_final(sha1_ctx *c, u8 out[20]) {
    u64 bits = c->len * 8;
    u8 pad = 0x80;
    sha1_update(c, &pad, 1);
    pad = 0;
    while (c->used != 56) sha1_update(c, &pad, 1);
    for (int i = 0; i < 8; i++) c->buf[63 - i] = (u8)(bits >> (8 * i)); /* big endian */
    block(c, c->buf);
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 4; j++) out[i * 4 + j] = (u8)(c->state[i] >> (24 - 8 * j));
}
