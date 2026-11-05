#include "hc128_internal.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

/* Standalone TV04 tests. This setup helper is NOT the production TV03 init.
 * Specification: sections 2.1/2.2. Expected words: Appendix A, pp. 9/10.
 * See docs/tv04-core.md for pinned source and SHA-256.
 */
static uint32_t ror(uint32_t x, unsigned int n)
{
    return (x >> n) | (x << (32U - n));
}

static void prepare(hc128_ctx *ctx, uint32_t key0, uint32_t iv0)
{
    uint32_t w[1280] = {0};
    memset(ctx, 0, sizeof(*ctx));
    w[0] = w[4] = key0;
    w[8] = w[12] = iv0;
    for (uint32_t i = 16; i < 1280; ++i) {
        const uint32_t x = w[i - 15];
        const uint32_t y = w[i - 2];
        w[i] = (ror(y, 17) ^ ror(y, 19) ^ (y >> 10)) + w[i - 7]
             + (ror(x, 7) ^ ror(x, 18) ^ (x >> 3)) + w[i - 16] + i;
    }
    memcpy(ctx->p, w + 256, sizeof(ctx->p));
    memcpy(ctx->q, w + 768, sizeof(ctx->q));
    /* Sentinel detects accidental writes to byte-layer state. */
    memset(ctx->byte_buffer, 0xa5, sizeof(ctx->byte_buffer));
    ctx->byte_pos = 3;
}

static int byte_state_unchanged(const hc128_ctx *ctx)
{
    return ctx->byte_pos == 3 && ctx->byte_buffer[0] == 0xa5 &&
           ctx->byte_buffer[1] == 0xa5 && ctx->byte_buffer[2] == 0xa5 &&
           ctx->byte_buffer[3] == 0xa5;
}

static int test_vectors(void)
{
    static const uint32_t expected[3][16] = {
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
    for (unsigned int c = 0; c < 3; ++c) {
        hc128_ctx ctx;
        prepare(&ctx, c == 2 ? 0x55U : 0U, c == 1 ? 1U : 0U);
        hc128_warmup(&ctx);
        if (ctx.counter != 0 || !byte_state_unchanged(&ctx)) {
            fprintf(stderr, "FAIL warmup state, case %u\n", c + 1);
            return 1;
        }
        for (unsigned int i = 0; i < 16; ++i) {
            const uint32_t actual = hc128_step_word(&ctx);
            if (actual != expected[c][i]) {
                fprintf(stderr, "FAIL vector %u word %u: expected %08" PRIx32
                        ", actual %08" PRIx32 "\n", c + 1, i, expected[c][i], actual);
                return 1;
            }
        }
        printf("PASS Appendix A vector %u: 16 words (64 bytes)\n", c + 1);
    }
    return 0;
}

static int test_state_boundaries(void)
{
    hc128_ctx ctx;
    prepare(&ctx, 0, 0);
    hc128_warmup(&ctx);
    for (uint32_t n = 0; n < 4096; ++n) {
        const hc128_ctx before = ctx;
        const uint32_t j = before.counter & 511U;
        (void)hc128_step_word(&ctx);
        if (ctx.counter != ((n + 1U) & 1023U) || !byte_state_unchanged(&ctx)) {
            fprintf(stderr, "FAIL counter/byte state at word %" PRIu32 "\n", n);
            return 1;
        }
        for (uint32_t k = 0; k < 512; ++k) {
            if ((before.counter >= 512U || k != j) && ctx.p[k] != before.p[k]) {
                fprintf(stderr, "FAIL unexpected P write at word %" PRIu32 "\n", n);
                return 1;
            }
            if ((before.counter < 512U || k != j) && ctx.q[k] != before.q[k]) {
                fprintf(stderr, "FAIL unexpected Q write at word %" PRIu32 "\n", n);
                return 1;
            }
        }
    }
    puts("PASS 4096 steps: table ownership, 511/512, 1023/0, byte state");
    return 0;
}

static int test_long_vector(void)
{
    static const uint32_t expected[16] = {
        0xa4eac026, 0x7e491126, 0x6a2a384f, 0x5c4e1329,
        0xda407fa1, 0x55e6b1ae, 0x05c6fdf3, 0xbbdc8a86,
        0x7a699aa0, 0x1a4dc117, 0x63658ccc, 0xd3e62474,
        0x9cf8236f, 0x0131be21, 0xc3a51de9, 0xd12290de
    };
    uint32_t actual[16] = {0};
    hc128_ctx ctx;
    prepare(&ctx, 0, 0);
    hc128_warmup(&ctx);
    for (uint32_t block = 0; block < (1U << 20); ++block) {
        for (unsigned int i = 0; i < 16; ++i) {
            actual[i] ^= hc128_step_word(&ctx);
        }
    }
    for (unsigned int i = 0; i < 16; ++i) {
        if (actual[i] != expected[i]) {
            fprintf(stderr, "FAIL long KAT accumulator %u: expected %08" PRIx32
                    ", actual %08" PRIx32 "\n", i, expected[i], actual[i]);
            return 1;
        }
    }
    if (ctx.counter != 0 || !byte_state_unchanged(&ctx)) {
        fputs("FAIL long KAT final state\n", stderr);
        return 1;
    }
    puts("PASS Appendix A long KAT: 16777216 words (64 MiB), 16384 counter cycles");
    return 0;
}

int main(void)
{
    if (test_vectors() || test_state_boundaries() || test_long_vector()) {
        return 1;
    }
    puts("PASS all TV04 standalone core tests");
    return 0;
}
