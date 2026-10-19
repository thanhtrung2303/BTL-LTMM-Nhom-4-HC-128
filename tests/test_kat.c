#include "hc128.h"

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FIELD_CAP 8193
#define MAX_STREAM_BYTES (1024U * 1024U)

typedef struct {
    char id[128];
    char key_hex[FIELD_CAP];
    char iv_hex[FIELD_CAP];
    char expected_hex[FIELD_CAP];
    size_t offset_bytes;
    size_t length_bytes;
    int has_offset;
    int has_length;
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

static int decode_hex(const char *hex, uint8_t *out, size_t capacity,
                      size_t *out_len)
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

    errno = 0;
    value = strtoull(s, &end, 10);

    if (errno != 0 || s == end || *trim(end) != '\0' ||
        value > (unsigned long long)SIZE_MAX)
        return -1;

    *out = (size_t)value;
    return 0;
}

static int load_case(const char *path, kat_case *tc)
{
    FILE *fp = fopen(path, "r");
    char line[FIELD_CAP + 256];

    if (fp == NULL) {
        perror(path);
        return -1;
    }

    memset(tc, 0, sizeof(*tc));

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *key;
        char *value;
        char *equals;

        key = trim(line);
        if (*key == '\0' || *key == '#')
            continue;

        equals = strchr(key, '=');
        if (equals == NULL) {
            fprintf(stderr, "Invalid fixture line: %s\n", key);
            fclose(fp);
            return -1;
        }

        *equals = '\0';
        value = trim(equals + 1);
        key = trim(key);

        if (strcmp(key, "id") == 0) {
            snprintf(tc->id, sizeof(tc->id), "%s", value);
        } else if (strcmp(key, "key_hex") == 0) {
            snprintf(tc->key_hex, sizeof(tc->key_hex), "%s", value);
        } else if (strcmp(key, "iv_hex") == 0) {
            snprintf(tc->iv_hex, sizeof(tc->iv_hex), "%s", value);
        } else if (strcmp(key, "expected_hex") == 0) {
            snprintf(tc->expected_hex, sizeof(tc->expected_hex),
                     "%s", value);
        } else if (strcmp(key, "offset_bytes") == 0) {
            if (parse_size(value, &tc->offset_bytes) != 0) {
                fprintf(stderr, "Invalid offset_bytes\n");
                fclose(fp);
                return -1;
            }
            tc->has_offset = 1;
        } else if (strcmp(key, "length_bytes") == 0) {
            if (parse_size(value, &tc->length_bytes) != 0) {
                fprintf(stderr, "Invalid length_bytes\n");
                fclose(fp);
                return -1;
            }
            tc->has_length = 1;
        }
    }

    if (ferror(fp)) {
        perror("Reading fixture");
        fclose(fp);
        return -1;
    }

    fclose(fp);

    if (tc->id[0] == '\0' || tc->key_hex[0] == '\0' ||
        tc->iv_hex[0] == '\0' || tc->expected_hex[0] == '\0' ||
        !tc->has_offset || !tc->has_length) {
        fprintf(stderr, "Fixture is missing a required field\n");
        return -1;
    }

   
ÈO
