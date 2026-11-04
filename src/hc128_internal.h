#ifndef HC128_INTERNAL_H
#define HC128_INTERNAL_H

#include "hc128.h"

#include <stdint.h>

/* TV02: 32-bit helper functions */
uint32_t hc128_load32_le(const uint8_t *p);
void hc128_store32_le(uint8_t *p, uint32_t x);

uint32_t hc128_f1(uint32_t x);
uint32_t hc128_f2(uint32_t x);

/* TV04: core functions */
uint32_t hc128_step_word(hc128_ctx *ctx);
void hc128_warmup(hc128_ctx *ctx);

#endif
