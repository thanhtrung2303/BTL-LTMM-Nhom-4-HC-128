#include "hc128.h"
#include "hc128_internal.h"

#include <stdio.h>
#include <string.h>

/* Independent expected words: Hongjun Wu, The Stream Cipher HC-128,
 * Appendix A, printed page 9. Convert each word to little-endian bytes.
 * The same published cases are used by TV04's standalone core tests.
 */
static const uint32_t expected_words[3][16] = {
    {0x73150082, 0x3bfd03a0, 0xfb2fd77f, 0xaa63af0e,
     0xde122fc6, 0xa7dc29b6, 0x62a68527, 0x8b75ec68,
     0x9036db1e, 0x81896005, 0x00ade078, 0x491fbf9a,
     0x1cdc3013, 0x6c3d6e24, 0x90f664b2, 0x9cd57102},
    {0xc01893d5, 0xb7dbe958, 0x8f65ec98, 0x64176604,
     0x36fc6724, 0xc82c6eec, 0x1b1c38a7, 0xc9b42a95,
     0x323ef123, 0x0a6a908b, 0xce757b68, 0x9f14f7bb,
     0xe4cde011, 0xaeb5173f, 0x89608c94, 0xb5cf46ca},
    {0x518251a4, 0x04b4930a, 0xb02af931, 0x0639f032,
     0xbcb4a47a, 0x5722480b, 0x2bf99f72, 0xcdc0e566,
     0x310f0c56, 0xd3cc83e8, 0x663db8ef, 0x62dfe07f,
     0x593e1790, 0xc5ceaa9c, 0xab03806f, 0xc9a6e5a0}
};

static int test_vectors(void)
{
    for (size_t c = 0; c < 3; ++c) {
        uint8_t key[16] = {0};
        uint8_t iv[16] = {0};
        uint8_t actual[64];
        hc128_ctx ctx;

        if (c == 1) {
            iv[0] = 1;
        } else if (c == 2) {
            key[0] = 0x55;
        }
        if (hc128_init(&ctx, key, iv) != HC128_OK ||
            hc128_keystream(&ctx, actual, sizeof(actual)) != HC128_OK) {
            fprintf(stderr, "FAIL API call in vector %zu\n", c + 1);
            return 1;
        }
        for (size_t i = 0; i < sizeof(actual); ++i) {
            const uint8_t expected =
                (uint8_t)(expected_words[c][i / 4] >> (8U * (i % 4)));
            if (actual[i] != expected) {
                fprintf(stderr, "FAIL vector %zu byte %zu: expected %02x, actual %02x\n",
                        c + 1, i, (unsigned int)expected, (unsigned int)actual[i]);
                return 1;
            }
        }
        printf("PASS public API Appendix A vector %zu: 64 bytes\n", c + 1);
    }
    return 0;
}

static int test_chunks_and_reset(void)
{
    const uint8_t key[16] = {0};
    const uint8_t iv[16] = {0};
    const size_t chunks[] = {1, 3, 5, 55};
    uint8_t whole[64], split[64];
    size_t offset = 0;
    hc128_ctx ctx;

    if (hc128_init(&ctx, key, iv) != HC128_OK ||
        hc128_keystream(&ctx, whole, sizeof(whole)) != HC128_OK ||
        hc128_init(&ctx, key, iv) != HC128_OK) {
        return 1;
    }
    for (size_t i = 0; i < sizeof(chunks) / sizeof(chunks[0]); ++i) {
        if (hc128_keystream(&ctx, NULL, 0) != HC128_OK ||
            hc128_keystream(&ctx, split + offset, chunks[i]) != HC128_OK) {
            return 1;
        }
        offset += chunks[i];
    }
    if (memcmp(whole, split, sizeof(whole)) != 0) {
        fputs("FAIL chunked output\n", stderr);
        return 1;
    }
    /* Leave one byte buffered, then verify init discards the old byte state. */
    if (hc128_keystream(&ctx, split, 3) != HC128_OK ||
        hc128_init(&ctx, key, iv) != HC128_OK ||
        hc128_keystream(&ctx, split, sizeof(split)) != HC128_OK ||
        memcmp(whole, split, sizeof(whole)) != 0) {
        fputs("FAIL reset with buffered byte\n", stderr);
        return 1;
    }
    puts("PASS chunks 1+3+5+55, zero-length calls, reset with buffered byte");
    return 0;
}

static int test_helpers_and_errors(void)
{
    const uint8_t bytes[4] = {0x00, 0x01, 0x02, 0x83};
    const uint8_t key[16] = {0};
    const uint8_t iv[16] = {0};
    hc128_ctx ctx;
    unsigned char snapshot[sizeof(ctx)];

    if (hc128_load32_le(bytes) != UINT32_C(0x83020100)) {
        fputs("FAIL unsigned little-endian load\n", stderr);
        return 1;
    }
    memset(&ctx, 0xa5, sizeof(ctx));
    if (hc128_init(&ctx, key, iv) != HC128_OK) {
        return 1;
    }
    memcpy(snapshot, &ctx, sizeof(ctx));
    if (hc128_init(NULL, key, iv) != HC128_ERR_ARGUMENT ||
        hc128_init(&ctx, NULL, iv) != HC128_ERR_ARGUMENT ||
        hc128_init(&ctx, key, NULL) != HC128_ERR_ARGUMENT ||
        hc128_keystream(NULL, NULL, 0) != HC128_ERR_ARGUMENT ||
        hc128_keystream(&ctx, NULL, 1) != HC128_ERR_ARGUMENT ||
        hc128_keystream(&ctx, NULL, 0) != HC128_OK ||
        memcmp(snapshot, &ctx, sizeof(ctx)) != 0) {
        fputs("FAIL NULL/zero-length call changed state or returned wrong status\n", stderr);
        return 1;
    }
    puts("PASS unsigned little-endian load and NULL/zero-length state preservation");
    return 0;
}

int main(void)
{
    if (test_vectors() || test_chunks_and_reset() || test_helpers_and_errors()) {
        return 1;
    }
    puts("PASS all TV06 demo integration tests");
    return 0;
}
