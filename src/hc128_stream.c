/*
 * hc128_stream.c - lop byte cua HC-128 (TV05).
 *
 * Nhiem vu: bien tung word 32 bit do core sinh ra thanh dong byte
 * little-endian, giu lai byte du giua cac loi goi public API.
 * Khong truy cap P/Q, khong tu init lai context (xem module-contracts.md).
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "hc128.h"
#include "hc128_internal.h"

/*
 * Lay word ke tiep tu core, luu little-endian vao byte_buffer va dat
 * byte_pos = 0. Chi goi khi buffer da het (byte_pos == 4).
 * Dich bit tren uint32_t da ep kieu, khong dich 32 bit nen khong co UB.
 */
static void hc128_stream_refill(hc128_ctx *ctx)
{
    uint32_t w = hc128_step_word(ctx);

    ctx->byte_buffer[0] = (uint8_t)(w & 0xFFu);
    ctx->byte_buffer[1] = (uint8_t)((w >> 8) & 0xFFu);
    ctx->byte_buffer[2] = (uint8_t)((w >> 16) & 0xFFu);
    ctx->byte_buffer[3] = (uint8_t)((w >> 24) & 0xFFu);
    ctx->byte_pos = 0;
}

/*
 * Ghi dung len byte dong khoa lien tiep vao out. Byte du cua word gan nhat
 * duoc dung truoc; chi goi core khi buffer rong. Cuoi loi goi, byte chua
 * dung van nam trong context cho lan sau (khong bo byte du).
 */
int hc128_keystream(hc128_ctx *ctx, uint8_t *out, size_t len)
{
    size_t done = 0;

    if (ctx == NULL) {
        return HC128_ERR_ARGUMENT;
    }
    if (len == 0) {
        return HC128_OK; /* khong ghi output, khong tien trang thai */
    }
    if (out == NULL) {
        return HC128_ERR_ARGUMENT; /* loi phat hien duoc: chua doi gi */
    }

    while (done < len) {
        size_t avail;
        size_t take;

        if (ctx->byte_pos >= 4) {
            hc128_stream_refill(ctx);
        }

        avail = 4 - ctx->byte_pos;
        take = (len - done < avail) ? (len - done) : avail;

        memcpy(out + done, ctx->byte_buffer + ctx->byte_pos, take);
        ctx->byte_pos += take;
        done += take;
    }

    return HC128_OK;
}

/*
 * hc128_xor: chua trien khai 
 * Co y khong viet stub tra thanh cong gia. Khi lam, dung chung co che
 * lay byte o tren (tach thanh helper static dung cho ca keystream va xor).
 */