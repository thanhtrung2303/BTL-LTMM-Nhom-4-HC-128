#include "hc128_internal.h"

/* Hongjun Wu, The Stream Cipher HC-128, sections 2.1--2.3.
 * Rotations below are only called with fixed counts in 1..31.
 * uint32_t arithmetic wraps modulo 2^32 on the supported GCC targets.
 */
static uint32_t rotr32(uint32_t x, unsigned int n)
{
    return (x >> n) | (x << (32U - n));
}

static uint32_t rotl32(uint32_t x, unsigned int n)
{
    return (x << n) | (x >> (32U - n));
}

static uint32_t g1(uint32_t x, uint32_t y, uint32_t z)
{
    return (rotr32(x, 10U) ^ rotr32(z, 23U)) + rotr32(y, 8U);
}

static uint32_t g2(uint32_t x, uint32_t y, uint32_t z)
{
    return (rotl32(x, 10U) ^ rotl32(z, 23U)) + rotl32(y, 8U);
}

/* h1 uses Q; h2 uses P. Extract bytes 0 and 2, not adjacent bytes. */
static uint32_t h(const uint32_t table[512], uint32_t x)
{
    return table[x & 255U] + table[256U + ((x >> 16U) & 255U)];
}

uint32_t hc128_step_word(hc128_ctx *ctx)
{
    const uint32_t j = ctx->counter & 511U;
    const uint32_t jm3 = (j + 512U - 3U) & 511U;
    const uint32_t jm10 = (j + 512U - 10U) & 511U;
    const uint32_t jm12 = (j + 512U - 12U) & 511U;
    /* (j - 511) modulo 512 equals (j + 1) modulo 512. */
    const uint32_t jm511 = (j + 1U) & 511U;
    uint32_t word;

    if (ctx->counter < 512U) {
        ctx->p[j] += g1(ctx->p[jm3], ctx->p[jm10], ctx->p[jm511]);
        word = ctx->p[j] ^ h(ctx->q, ctx->p[jm12]);
    } else {
        ctx->q[j] += g2(ctx->q[jm3], ctx->q[jm10], ctx->q[jm511]);
        word = ctx->q[j] ^ h(ctx->p, ctx->q[jm12]);
    }

    ctx->counter = (ctx->counter + 1U) & 1023U;
    return word;
}

void hc128_warmup(hc128_ctx *ctx)
{
    /* Precondition: P/Q loaded from W, counter == 0.
     * Each output REPLACES the updated entry before the next step.
     * Merely discarding 1024 normal outputs would be incorrect.
     */
    for (uint32_t j = 0; j < 512U; ++j) {
        const uint32_t word = hc128_step_word(ctx);
        ctx->p[j] = word;
    }
    for (uint32_t j = 0; j < 512U; ++j) {
        const uint32_t word = hc128_step_word(ctx);
        ctx->q[j] = word;
    }
    /* 1024 steps have wrapped counter back to zero. Byte state is untouched. */
}
