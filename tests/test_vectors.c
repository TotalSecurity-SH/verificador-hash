/* Pruebas nativas con vectores oficiales (RFC 1321, FIPS 180-4) y con archivos contra sha*sum. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/hash.h"

static void hex(const u8 *d, int n, char *out) {
    for (int i = 0; i < n; i++) sprintf(out + i * 2, "%02x", d[i]);
}

static int check(const char *name, const char *got, const char *want) {
    int ok = strcmp(got, want) == 0;
    printf("%s %-10s %s\n", ok ? "OK  " : "FAIL", name, ok ? "" : got);
    return ok;
}

static void all(const u8 *data, usize n, char md5h[33], char s1h[41], char s256h[65], char s512h[129]) {
    u8 o[64];
    md5_ctx a; md5_init(&a); md5_update(&a, data, n); md5_final(&a, o); hex(o, 16, md5h);
    sha1_ctx b; sha1_init(&b); sha1_update(&b, data, n); sha1_final(&b, o); hex(o, 20, s1h);
    sha256_ctx c; sha256_init(&c); sha256_update(&c, data, n); sha256_final(&c, o); hex(o, 32, s256h);
    sha512_ctx d; sha512_init(&d); sha512_update(&d, data, n); sha512_final(&d, o); hex(o, 64, s512h);
}

int main(int argc, char **argv) {
    char m[33], s1[41], s256[65], s512[129];
    int ok = 1;

    if (argc > 1) { /* modo archivo: imprime los cuatro digest */
        FILE *f = fopen(argv[1], "rb");
        if (!f) return 2;
        fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
        u8 *buf = malloc(n ? n : 1); fread(buf, 1, n, f); fclose(f);
        all(buf, n, m, s1, s256, s512);
        printf("%s\n%s\n%s\n%s\n", m, s1, s256, s512);
        return 0;
    }

    all((const u8 *)"", 0, m, s1, s256, s512);
    ok &= check("md5('')", m, "d41d8cd98f00b204e9800998ecf8427e");
    ok &= check("sha1('')", s1, "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    ok &= check("sha256('')", s256, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    ok &= check("sha512('')", s512, "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");

    all((const u8 *)"abc", 3, m, s1, s256, s512);
    ok &= check("md5(abc)", m, "900150983cd24fb0d6963f7d28e17f72");
    ok &= check("sha1(abc)", s1, "a9993e364706816aba3e25717850c26c9cd0d89d");
    ok &= check("sha256(abc)", s256, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    ok &= check("sha512(abc)", s512, "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f");

    const char *q = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    all((const u8 *)q, strlen(q), m, s1, s256, s512);
    ok &= check("sha256(448b)", s256, "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
    ok &= check("sha1(448b)", s1, "84983e441c3bd26ebaae4aa1f95129e5e54670f1");

    printf(ok ? "\nTodas las pruebas pasaron\n" : "\nHay pruebas fallidas\n");
    return ok ? 0 : 1;
}
