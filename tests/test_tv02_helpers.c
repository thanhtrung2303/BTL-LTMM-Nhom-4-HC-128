#include "hc128_internal.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(condition, name)                 \
    do {                                       \
        if (!(condition)) {                    \
            printf("[FAIL] %s\n", name);       \
            return 1;                          \
        }                                      \
        printf("[PASS] %s\n", name);           \
    } while (0)

int main(void)
{
    uint8_t bytes[4];
    uint32_t value;

    /* T02-01: Little-endian load */
    bytes[0] = 0x00;
    bytes[1] = 0x01;
    bytes[2] = 0x02;
    bytes[3] = 0x03;

    value = hc128_load32_le(bytes);
    CHECK(value == UINT32_C(0x03020100), "load32_le");

    /* T02-02: Little-endian store */
    memset(bytes, 0, sizeof(bytes));
    hc128_store32_le(bytes, UINT32_C(0x03020100));

    CHECK(bytes[0] == 0x00 &&
          bytes[1] == 0x01 &&
          bytes[2] == 0x02 &&
          bytes[3] == 0x03, "store32_le");

    /* T02-03: f1 and f2 */
    CHECK(hc128_f1(0) == 0, "f1 zero");
    CHECK(hc128_f2(0) == 0, "f2 zero");

    CHECK(hc128_f1(UINT32_MAX) == UINT32_C(0x1FFFFFFF),
          "f1 UINT32_MAX");
    CHECK(hc128_f2(UINT32_MAX) == UINT32_C(0x003FFFFF),
          "f2 UINT32_MAX");

    CHECK(hc128_f1(1) == UINT32_C(0x02004000),
          "f1 one-bit");
    CHECK(hc128_f2(1) == UINT32_C(0x0000A000),
          "f2 one-bit");

    CHECK(hc128_f1(UINT32_C(0x12345678)) ==
          UINT32_C(0xE7FCE6EE), "f1 12345678");
    CHECK(hc128_f2(UINT32_C(0x12345678)) ==
          UINT32_C(0xA1F78649), "f2 12345678");

    puts("All TV02 helper tests passed.");
    return 0;
}
