#include "hc128.h"

#include <stdio.h>

/* TV06 progress demo: public test key/IV, not secrets for real messages. */
enum { DEMO_BYTES = 64, DEMO_ERR_IO = 3, DEMO_ERR_API = 4 };

static int print_hex(const uint8_t *bytes, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        if (printf("%02x", (unsigned int)bytes[i]) < 0) {
            return -1;
        }
    }

    /* fflush catches buffered write errors before reporting success. */
    if (putchar('\n') == EOF || fflush(stdout) == EOF) {
        return -1;
    }
    return 0;
}

int main(void)
{
    const uint8_t key[16] = {0};
    const uint8_t iv[16] = {0};
    uint8_t output[DEMO_BYTES];
    hc128_ctx ctx;

    /* One initialization, followed by exactly 64 consecutive stream bytes. */
    if (hc128_init(&ctx, key, iv) != HC128_OK) {
        fputs("HC-128 demo: initialization failed.\n", stderr);
        return DEMO_ERR_API;
    }
    if (hc128_keystream(&ctx, output, sizeof(output)) != HC128_OK) {
        fputs("HC-128 demo: keystream generation failed.\n", stderr);
        return DEMO_ERR_API;
    }
    if (print_hex(output, sizeof(output)) != 0) {
        fputs("HC-128 demo: cannot write output.\n", stderr);
        return DEMO_ERR_IO;
    }

    return 0;
}
