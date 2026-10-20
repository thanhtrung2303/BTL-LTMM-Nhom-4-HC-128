#ifndef HC128_INTERNAL_H
#define HC128_INTERNAL_H

#include "hc128.h"

/* Internal API agreed in docs/module-contracts.md.
 * TV02 may extend this header with shared helpers without changing these signatures.
 * Callers provide valid state; these functions do not validate pointers.
 */
uint32_t hc128_step_word(hc128_ctx *ctx);
void hc128_warmup(hc128_ctx *ctx);

#endif
