#ifndef HC128_INTERNAL_H
#define HC128_INTERNAL_H

#include "hc128.h"

/* Minimal helpers required by TV03 init; review ownership remains with TV02.
 * Hongjun Wu, The Stream Cipher HC-128, sections 2.1/2.2.
 * Fixed rotation counts avoid shifting by 0 or 32 bits.
 */
static inline uint32_t hc128_load32_le(const uint8_t bytes[4])
{
    return (uint32_t)bytes[0]
         | ((uint32_t)bytes[1] << 8U)
         | ((uint32_t)bytes[2] << 16U)
         | ((uint32_t)bytes[3] << 24U);
}

static inline uint32_t hc128_f1(uint32_t x)
{
    return ((x >> 7U) | (x << 25U))
         ^ ((x >> 18U) | (x << 14U)) ^ (x >> 3U);
}

static inline uint32_t hc128_f2(uint32_t x)
{
    return ((x >> 17U) | (x << 15U))
         ^ ((x >> 19U) | (x << 13U)) ^ (x >> 10U);
}

/* Internal API agreed in docs/module-contracts.md.
 * TV02 may extend this header with shared helpers without changing these signatures.
 * Callers provide valid state; these functions do not validate pointers.
 */
uint32_t hc128_step_word(hc128_ctx *ctx);
void hc128_warmup(hc128_ctx *ctx);

#endif
