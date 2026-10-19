#ifndef HC128_H
#define HC128_H

#include <stddef.h>
#include <stdint.h>

#define HC128_OK 0
#define HC128_ERR_ARGUMENT 1

typedef struct hc128_ctx {
    uint32_t p[512];
    uint32_t q[512];
    uint32_t counter;
    uint8_t byte_buffer[4];
    size_t byte_pos;
} hc128_ctx;

int hc128_init(hc128_ctx *ctx, const uint8_t key[16],
               const uint8_t iv[16]);
int hc128_keystream(hc128_ctx *ctx, uint8_t *out, size_t len);
int hc128_xor(hc128_ctx *ctx, const uint8_t *in,
              uint8_t *out, size_t len);

#endif
