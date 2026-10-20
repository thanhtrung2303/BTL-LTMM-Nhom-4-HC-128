
#include "hc128.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CASE_COUNT 100
#define MAX_LEN 16384
#define SEED 0x1282026u

static uint32_t rng_state = SEED;

static uint32_t next_random(void)
{
    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state = x;
    return x;
}

static void random_bytes(uint8_t *out, size_t len)
{
    for (size_t i = 0; i < len; ++i)
        out[i] = (uint8_t)next_random();
}

static size_t next_chunk(size_t remaining)
{
    size_t chunk = (size_t)(next_random() % 257u) + 1u;
    return chunk < remaining ? chunk : remaining;
}

static int test_chunk_consistency(
    const uint8_t key[16], const uint8_t iv[16], size_t len)
{
    uint8_t expected[MAX_LEN];
    uint8_t actual[MAX_LEN];
    hc128_ctx ctx;
    size_t offset = 0;

    if (hc128_init(&ctx, key, iv) != HC128_OK)
        return 1;

    if (hc128_keystream(&ctx, len ? expected : NULL, len) != HC128_OK)
        return 1;

    if (hc128_init(&ctx, key, iv) != HC128_OK)
        return 1;

    if (hc128_keystream(&ctx, NULL, 0) != HC128_OK)
        return 1;

    while (offset < len) {
        size_t chunk = next_chunk(len - offset);

        if (hc128_keystream(&ctx, actual + offset, chunk) != HC128_OK)
            return 1;

        offset += chunk;
    }

    if (len > 0 && memcmp(expected, actual, len) != 0) {
        for (size_t i = 0; i < len; ++i) {
            if (expected[i] != actual[i]) {
                fprintf(stderr,
                        "Chunk mismatch at byte %zu: expected=%02x actual=%02x\n",
                        i, expected[i], actual[i]);
                break;
            }
        }
        return 1;
    }

    return 0;
}

static int test_context_independence(
    const uint8_t key_a[16], const uint8_t iv_a[16],
    const uint8_t key_b[16], const uint8_t iv_b[16],
    size_t len_a, size_t len_b)
{
    uint8_t expected_a[MAX_LEN], expected_b[MAX_LEN];
    uint8_t actual_a[MAX_LEN], actual_b[MAX_LEN];
    hc128_ctx ctx_a, ctx_b;
    size_t pos_a = 0, pos_b = 0;

    if (hc128_init(&ctx_a, key_a, iv_a) != HC128_OK ||
        hc128_keystream(&ctx_a, len_a ? expected_a : NULL, len_a) != HC128_OK)
        return 1;

    if (hc128_init(&ctx_b, key_b, iv_b) != HC128_OK ||
        hc128_keystream(&ctx_b, len_b ? expected_b : NULL, len_b) != HC128_OK)
        return 1;

    if (hc128_init(&ctx_a, key_a, iv_a) != HC128_OK ||
        hc128_init(&ctx_b, key_b, iv_b) != HC128_OK)
        return 1;

    while (pos_a < len_a || pos_b < len_b) {
        if (pos_a < len_a) {
            size_t chunk = next_chunk(len_a - pos_a);
            if (hc128_keystream(&ctx_a, actual_a + pos_a, chunk) != HC128_OK)
                return 1;
            pos_a += chunk;
        }

        if (pos_b < len_b) {
            size_t chunk = next_chunk(len_b - pos_b);
            if (hc128_keystream(&ctx_b, actual_b + pos_b, chunk) != HC128_OK)
                return 1;
            pos_b += chunk;
        }
    }

    if ((len_a > 0 && memcmp(expected_a, actual_a, len_a) != 0) ||
        (len_b > 0 && memcmp(expected_b, actual_b, len_b) != 0)) {
        fprintf(stderr, "Interleaved context output mismatch\n");
        return 1;
    }

    return 0;
}

int main(void)
{
    uint8_t key_a[16], iv_a[16], key_b[16], iv_b[16];

    printf("TV09 stream tests\n");
    printf("Seed: 0x%08x\n", SEED);

    for (unsigned int i = 0; i < CASE_COUNT; ++i) {
        size_t len_a, len_b;

        random_bytes(key_a, sizeof(key_a));
        random_bytes(iv_a, sizeof(iv_a));
        random_bytes(key_b, sizeof(key_b));
        random_bytes(iv_b, sizeof(iv_b));

        len_a = (size_t)(next_random() % (MAX_LEN + 1u));
        len_b = (size_t)(next_random() % (MAX_LEN + 1u));

        if (test_chunk_consistency(key_a, iv_a, len_a) != 0) {
            fprintf(stderr, "[FAIL] chunk case %u, len=%zu\n", i, len_a);
            return 1;
        }

        if (test_context_independence(
                key_a, iv_a, key_b, iv_b, len_a, len_b) != 0) {
            fprintf(stderr,
                    "[FAIL] context case %u, len_a=%zu, len_b=%zu\n",
                    i, len_a, len_b);
            return 1;
        }

        printf("[PASS] case %u/%u\n", i + 1, CASE_COUNT);
    }

    puts("[PASS] All TV09 stream consistency tests passed.");
    return 0;
}
