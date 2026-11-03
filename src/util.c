#include "hash.h"

/* El compilador puede emitir llamadas a memset/memcpy aun sin libc. */
void *memset(void *dst, int v, usize n) {
    u8 *d = (u8 *)dst;
    while (n--) *d++ = (u8)v;
    return dst;
}

void *memcpy(void *dst, const void *src, usize n) {
    u8 *d = (u8 *)dst;
    const u8 *s = (const u8 *)src;
    while (n--) *d++ = *s++;
    return dst;
}
