#include "ban_font_charset.h"
#include "ban_logic.h"

#include <stdio.h>

static int g_fails;

#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        g_fails++; \
    } \
} while (0)

static bool has_cp(uint32_t cp) {
    int lo = 0;
    int hi = BAN_FONT_CODEPOINT_COUNT;

    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        uint32_t cur = ban_font_codepoints[mid];
        if (cur == cp) return true;
        if (cur < cp) lo = mid + 1;
        else hi = mid;
    }
    return false;
}

static void walk(const char *label, const char *text) {
    const unsigned char *p = (const unsigned char *)text;

    if (!p) return;
    while (*p) {
        uint32_t cp = 0;
        if (*p < 0x80) {
            cp = *p++;
        } else if ((*p & 0xE0) == 0xC0 && (p[1] & 0xC0) == 0x80) {
            cp = ((uint32_t)(*p & 0x1F) << 6) | (p[1] & 0x3F);
            p += 2;
        } else if ((*p & 0xF0) == 0xE0 && (p[1] & 0xC0) == 0x80 && (p[2] & 0xC0) == 0x80) {
            cp = ((uint32_t)(*p & 0x0F) << 12) | ((uint32_t)(p[1] & 0x3F) << 6) | (p[2] & 0x3F);
            p += 3;
        } else if ((*p & 0xF8) == 0xF0 && p[1] && p[2] && p[3]) {
            cp = ((uint32_t)(*p & 0x07) << 18) | ((uint32_t)(p[1] & 0x3F) << 12)
               | ((uint32_t)(p[2] & 0x3F) << 6) | (p[3] & 0x3F);
            p += 4;
        } else {
            fprintf(stderr, "bad utf-8 in %s\n", label);
            g_fails++;
            return;
        }
        if (cp < 0x20) continue;
        if (!has_cp(cp)) {
            fprintf(stderr, "missing U+%04X from %s\n", cp, label);
            g_fails++;
        }
    }
}

int main(void) {
    int count = 0;
    int i;
    const ban_copy_t *table = ban_copy_table(&count);

    CHECK(count == BAN_COPY_COUNT);
    CHECK(BAN_FONT_CODEPOINT_COUNT > 16);
    for (i = 1; i < BAN_FONT_CODEPOINT_COUNT; i++) {
        CHECK(ban_font_codepoints[i] > ban_font_codepoints[i - 1]);
    }
    for (i = 0; i < count; i++) {
        walk(table[i].title, table[i].title);
        walk(table[i].line1, table[i].line1);
        walk(table[i].line2, table[i].line2);
    }
    walk("disclaimer", ban_disclaimer_full());
    walk("disclaimer-en", ban_disclaimer_en());
    walk("runtime", ban_runtime_glyphs());
    CHECK(has_cp(0x4E2D));
    CHECK(!has_cp(0x9F98));
    if (g_fails) {
        fprintf(stderr, "font coverage: %d checks failed\n", g_fails);
        return 1;
    }
    return 0;
}
