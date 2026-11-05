#include "hc128.h"

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FIELD_CAP 8193
#define SOURCE_CAP 2048
#define LINE_CAP (FIELD_CAP + SOURCE_CAP + 256)
#define MAX_STREAM_BYTES (1024U * 1024U)
#define MAX_VECTOR_BYTES (FIELD_CAP / 2U)

enum {
    SEEN_ID       = 1U << 0,
    SEEN_KEY      = 1U << 1,
    SEEN_IV       = 1U << 2,
    SEEN_EXPECTED  = 1U << 3,
    SEEN_OFFSET    = 1U << 4,
    SEEN_LENGTH    = 1U << 5,
    SEEN_INPUT     = 1U << 6,
    SEEN_SOURCE    = 1U << 7
};

#define REQUIRED_FIELDS \
    (SEEN_ID | SEEN_KEY | SEEN_IV | SEEN_EXPECTED | \
     SEEN_OFFSET | SEEN_LENGTH)

typedef struct {
    char id[128];
    char source[SOURCE_CAP];
    char key_hex[FIELD_CAP];
    char iv_hex[FIELD_CAP];
    char input_hex[FIELD_CAP];
    char expected_hex[FIELD_CAP];
    size_t offset_bytes;
    size_t length_bytes;
    unsigned int seen;
} kat_case;

static char *trim(char *s)
{
    char *end;

    while (isspace((unsigned char)*s))
        s++;

    end = s + strlen(s);
    while (end > s && isspace((unsigned char)end[-1]))
        *--end = '\0';

    return s;
}

static int hex_value(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

static int decode_hex(const char *hex, uint8_t *out,
                      size_t capacity, size_t *out_len)
{
    size_t n = strlen(hex);

    if (n % 2 != 0 || n / 2 > capacity)
        return -1;

    for (size_t i = 0; i < n / 2; i++) {
        int hi = hex_value(hex[2 * i]);
        int lo = hex_value(hex[2 * i + 1]);

        if (hi < 0 || lo < 0)
            return -1;

        out[i] = (uint8_t)((hi << 4) | lo);
    }

    *out_len = n / 2;
    return 0;
}

static int parse_size(const char *s, size_t *out)
{
    char *end;
    unsigned long long value;

    if (*s == '-' || *s == '+')
        return -1;

    if (*s == '\0')
        return -1;

    for (const char *p = s; *p != '\0'; p++) {
        if (!isdigit((unsigned char)*p))
            return -1;
    }

    errno = 0;
    value = strtoull(s, &end, 10);

    if (errno != 0 || end == s || *end != '\0' ||
        value > (unsigned long long)SIZE_MAX)
        return -1;

    *out = (size_t)value;
    return 0;
}

static int copy_field(char *dst, size_t capacity, const char *value)
{
    size_t length = strlen(value);

    if (length >= capacity)
        return -1;

    memcpy(dst, value, length + 1);
    return 0;
}

static int load_case(const char *path, kat_case *tc)
{
    FILE *fp;
    char line[LINE_CAP];
    size_t line_number = 0;

    fp = fopen(path, "r");
    if (fp == NULL) {
        perror(path);
        return -1;
    }

    memset(tc, 0, sizeof(*tc));

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *key;
        char *value;
        char *equals;
        unsigned int bit = 0;

        line_number++;

        if (strchr(line, '\n') == NULL && !feof(fp)) {
            fprintf(stderr, "%s:%zu: line too long\n",
                    path, line_number);
            fclose(fp);
            return -1;
        }

        key = trim(line);
        if (*key == '\0' || *key == '#')
            continue;

        equals = strchr(key, '=');
        if (equals == NULL) {
            fprintf(stderr, "%s:%zu: expected key=value\n",
                    path, line_number);
            fclose(fp);
            return -1;
        }

        *equals = '\0';
        value = trim(equals + 1);
        key = trim(key);

        if (strcmp(key, "id") == 0) {
            bit = SEEN_ID;
            if (copy_field(tc->id, sizeof(tc->id), value) != 0)
                goto field_too_long;
        } else if (strcmp(key, "source") == 0) {
            bit = SEEN_SOURCE;
            if (copy_field(tc->source, sizeof(tc->source), value) != 0)
                goto field_too_long;
        } else if (strcmp(key, "key_hex") == 0) {
            bit = SEEN_KEY;
            if (copy_field(tc->key_hex, sizeof(tc->key_hex), value) != 0)
                goto field_too_long;
        } else if (strcmp(key, "iv_hex") == 0) {
            bit = SEEN_IV;
            if (copy_field(tc->iv_hex, sizeof(tc->iv_hex), value) != 0)
                goto field_too_long;
        } else if (strcmp(key, "input_hex") == 0) {
            bit = SEEN_INPUT;
            if (copy_field(tc->input_hex, sizeof(tc->input_hex), value) != 0)
                goto field_too_long;
        } else if (strcmp(key, "expected_hex") == 0) {
            bit = SEEN_EXPECTED;
            if (copy_field(tc->expected_hex,
                           sizeof(tc->expected_hex), value) != 0)
                goto field_too_long;
        } else if (strcmp(key, "offset_bytes") == 0) {
            bit = SEEN_OFFSET;
            if (parse_size(value, &tc->offset_bytes) != 0) {
                fprintf(stderr, "%s:%zu: invalid offset_bytes\n",
                        path, line_number);
                fclose(fp);
                return -1;
            }
        } else if (strcmp(key, "length_bytes") == 0) {
            bit = SEEN_LENGTH;
            if (parse_size(value, &tc->length_bytes) != 0) {
                fprintf(stderr, "%s:%zu: invalid length_bytes\n",
                        path, line_number);
                fclose(fp);
                return -1;
            }
        } else {
            fprintf(stderr, "%s:%zu: unknown field '%s'\n",
                    path, line_number, key);
            fclose(fp);
            return -1;
        }

        if ((tc->seen & bit) != 0) {
            fprintf(stderr, "%s:%zu: duplicate field '%s'\n",
                    path, line_number, key);
            fclose(fp);
            return -1;
        }

        tc->seen |= bit;
        continue;

field_too_long:
        fprintf(stderr, "%s:%zu: value for '%s' is too long\n",
                path, line_number, key);
        fclose(fp);
        return -1;
    }

    if (ferror(fp)) {
        perror("Reading fixture");
        fclose(fp);
        return -1;
    }

    fclose(fp);

    if ((tc->seen & REQUIRED_FIELDS) != REQUIRED_FIELDS) {
        fprintf(stderr, "Fixture is missing a required field\n");
        return -1;
    }

    if (tc->id[0] == '\0' || tc->key_hex[0] == '\0' ||
        tc->iv_hex[0] == '\0' || tc->expected_hex[0] == '\0') {
        fprintf(stderr, "Fixture contains an empty required value\n");
        return -1;
    }

    if (tc->length_bytes == 0 ||
        tc->length_bytes > MAX_VECTOR_BYTES ||
        tc->offset_bytes > MAX_STREAM_BYTES ||
        tc->length_bytes > MAX_STREAM_BYTES - tc->offset_bytes) {
        fprintf(stderr, "Fixture offset/length is outside supported limits\n");
        return -1;
    }

    return 0;
}

static void print_hex(const uint8_t *data, size_t length)
{
    for (size_t i = 0; i < length; i++)
        printf("%02x", data[i]);
    putchar('\n');
}

static int run_case(const char *path)
{
    kat_case tc;
    uint8_t key[16];
    uint8_t iv[16];
    uint8_t input[MAX_VECTOR_BYTES];
    uint8_t expected[MAX_VECTOR_BYTES];
    uint8_t actual[MAX_VECTOR_BYTES];
    uint8_t discard[4096];
    size_t key_len;
    size_t iv_len;
    size_t input_len = 0;
    size_t expected_len;
    size_t remaining;
    hc128_ctx ctx;

    if (load_case(path, &tc) != 0)
        return 1;

    if (decode_hex(tc.key_hex, key, sizeof(key), &key_len) != 0 ||
        key_len != sizeof(key)) {
        fprintf(stderr, "%s: key_hex must encode exactly 16 bytes\n",
                tc.id);
        return 1;
    }

    if (decode_hex(tc.iv_hex, iv, sizeof(iv), &iv_len) != 0 ||
        iv_len != sizeof(iv)) {
        fprintf(stderr, "%s: iv_hex must encode exactly 16 bytes\n",
                tc.id);
        return 1;
    }

    if (decode_hex(tc.expected_hex, expected, sizeof(expected),
                   &expected_len) != 0 ||
        expected_len != tc.length_bytes) {
        fprintf(stderr, "%s: expected_hex length must equal length_bytes\n",
                tc.id);
        return 1;
    }

    if (tc.input_hex[0] != '\0') {
        if (decode_hex(tc.input_hex, input, sizeof(input),
                       &input_len) != 0 ||
            input_len != tc.length_bytes) {
            fprintf(stderr, "%s: input_hex length must equal length_bytes\n",
                    tc.id);
            return 1;
        }
    }

    printf("Case: %s\n", tc.id);
    printf("Source: %s\n",
           tc.source[0] != '\0' ? tc.source : "(not recorded)");
    printf("Key: ");
    print_hex(key, sizeof(key));
    printf("IV: ");
    print_hex(iv, sizeof(iv));
    printf("Offset: %zu bytes\n", tc.offset_bytes);
    printf("Length: %zu bytes\n", tc.length_bytes);
    printf("Input: ");
    if (input_len == 0)
        puts("(keystream mode)");
    else
        print_hex(input, input_len);

    if (hc128_init(&ctx, key, iv) != HC128_OK) {
        fprintf(stderr, "%s: hc128_init failed\n", tc.id);
        return 1;
    }

    remaining = tc.offset_bytes;
    while (remaining > 0) {
        size_t chunk = remaining < sizeof(discard)
                           ? remaining : sizeof(discard);

        if (hc128_keystream(&ctx, discard, chunk) != HC128_OK) {
            fprintf(stderr, "%s: failed while consuming offset\n", tc.id);
            return 1;
        }
        remaining -= chunk;
    }

    if (input_len == 0) {
        if (hc128_keystream(&ctx, actual, tc.length_bytes) != HC128_OK) {
            fprintf(stderr, "%s: hc128_keystream failed\n", tc.id);
            return 1;
        }
    } else {
        if (hc128_xor(&ctx, input, actual, tc.length_bytes) != HC128_OK) {
            fprintf(stderr, "%s: hc128_xor failed\n", tc.id);
            return 1;
        }
    }

    printf("Expected: ");
    print_hex(expected, expected_len);
    printf("Actual:   ");
    print_hex(actual, tc.length_bytes);

    for (size_t i = 0; i < tc.length_bytes; i++) {
        if (actual[i] != expected[i]) {
            fprintf(stderr,
                    "[FAIL] %s: first mismatch at absolute byte %zu, "
                    "segment byte %zu (expected %02x, actual %02x)\n",
                    tc.id, tc.offset_bytes + i, i,
                    expected[i], actual[i]);
            return 1;
        }
    }

    printf("[PASS] %s: %zu bytes matched\n", tc.id, tc.length_bytes);
    return 0;
}

int main(int argc, char **argv)
{
    int failed = 0;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <fixture-file> [fixture-file ...]\n",
                argv[0]);
        return 2;
    }

    for (int i = 1; i < argc; i++) {
        if (run_case(argv[i]) != 0)
            failed = 1;
    }

    return failed ? 1 : 0;
}
