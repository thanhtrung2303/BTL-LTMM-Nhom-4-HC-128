/* HC-128 key/IV initialization -- TV03.
 * Reference: Hongjun Wu, "The Stream Cipher HC-128", section 2.2.
 * Depends on TV02 shared helpers and TV04 hc128_warmup().
 */
#include "hc128.h"
#include "hc128_internal.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

int hc128_init(hc128_ctx *ctx, const uint8_t key[16],
               const uint8_t iv[16])
{
    uint32_t w[1280];
    size_t i;

    /* Check before any context mutation. Key/IV buffers must have 16 bytes. */
    if (ctx == NULL || key == NULL || iv == NULL) {
        return HC128_ERR_ARGUMENT;
    }

    /* W[0..7] = K[0..3] repeated; W[8..15] = IV[0..3] repeated. */
    for (i = 0; i < 4; ++i) {
        const uint32_t k = hc128_load32_le(key + 4 * i);
        const uint32_t v = hc128_load32_le(iv + 4 * i);

        w[i]      = k;
        w[i + 4] = k;
        w[i + 8] = v;
        w[i + 12] = v;
    }

    /* The uint32_t additions naturally wrap modulo 2^32. */
    for (i = 16; i < 1280; ++i) {
        w[i] = hc128_f2(w[i - 2])
             + w[i - 7]
             + hc128_f1(w[i - 15])
             + w[i - 16]
             + (uint32_t)i;
    }

    /* Copy the 1024 expanded words used for the two internal tables. */
    for (i = 0; i < 512; ++i) {
        ctx->p[i] = w[i + 256];
        ctx->q[i] = w[i + 768];
    }

    /* TV04 performs 1024 special feedback rounds -- not keystream discard. */
    ctx->counter = 0;
    hc128_warmup(ctx);

    /* Reset the byte interface so this context begins at byte 0. */
    ctx->counter = 0;
    memset(ctx->byte_buffer, 0, sizeof(ctx->byte_buffer));
    ctx->byte_pos = sizeof(ctx->byte_buffer);  /* 4 means empty */

    return HC128_OK;
}
