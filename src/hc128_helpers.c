#include "hc128_internal.h"

static uint32_t hc128_rotr32(uint32_t x, unsigned int n)
{
    n &= 31U;

    if (n == 0U) {
        return x;
    }

    return (x >> n) | (x << (32U - n));
}

uint32_t hc128_load32_le(const uint8_t *p)
{
    return ((uint32_t)p[0])
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

void hc128_store32_le(uint8_t *p, uint32_t x)
{
    p[0] = (uint8_t)x;
    p[1] = (uint8_t)(x >> 8);
    p[2] = (uint8_t)(x >> 16);
    p[3] = (uint8_t)(x >> 24);
}

uint32_t hc128_f1(uint32_t x)
{
    return hc128_rotr32(x, 7)
         ^ hc128_rotr32(x, 18)
         ^ (x >> 3);
}

uint32_t hc128_f2(uint32_t x)
{
    return hc128_rotr32(x, 17)
         ^ hc128_rotr32(x, 19)
         ^ (x >> 10);
}