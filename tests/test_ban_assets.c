/* 素材目录和调色板键。不链接位图本体，避免把整份 RGB565A8 拉进主机测试。 */
#include "ban_asset_catalog.h"
#include "ban_palette.h"

#include <stdio.h>
#include <string.h>

static int g_fails;

#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        g_fails++; \
    } \
} while (0)

static const ban_asset_info_t EXPECTED[] = {
    {"dario_normal_160", 160, 160, 1},
    {"dario_normal_96", 96, 96, 1},
    {"dario_normal_48", 48, 48, 1},
    {"dario_suspicious_160", 160, 160, 1},
    {"dario_suspicious_96", 96, 96, 1},
    {"dario_suspicious_48", 48, 48, 1},
    {"dario_angry_160", 160, 160, 1},
    {"dario_angry_96", 96, 96, 1},
    {"dario_angry_48", 48, 48, 1},
    {"dario_banned_160", 160, 160, 1},
    {"dario_banned_96", 96, 96, 1},
    {"dario_banned_48", 48, 48, 1},
    {"dario_shock_160", 160, 160, 1},
    {"dario_shock_96", 96, 96, 1},
    {"dario_shock_48", 48, 48, 1},
    {"dario_blink_160", 160, 160, 1},
    {"dario_blink_96", 96, 96, 1},
    {"dario_blink_48", 48, 48, 1},
    {"icon_gift_64", 64, 64, 1},
    {"icon_gift_40", 40, 40, 1},
    {"icon_mail_64", 64, 64, 1},
    {"icon_mail_40", 40, 40, 1},
    {"icon_warning_64", 64, 64, 1},
    {"icon_warning_40", 40, 40, 1},
    {"icon_language_64", 64, 64, 1},
    {"icon_language_40", 40, 40, 1},
    {"icon_timezone_64", 64, 64, 1},
    {"icon_timezone_40", 40, 40, 1},
    {"icon_wechat_64", 64, 64, 1},
    {"icon_wechat_40", 40, 40, 1},
    {"icon_ban_user_64", 64, 64, 1},
    {"icon_ban_user_40", 40, 40, 1},
    {"flag_us_72", 72, 50, 1},
    {"flag_jp_72", 72, 50, 1},
    {"flag_sg_72", 72, 50, 1},
    {"ban_stamp_200", 200, 72, 1},
    {"ban_stamp_cn_200", 200, 72, 1},
};

static const ban_asset_info_t *find_name(const char *name) {
    int i;

    for (i = 0; i < BAN_ASSET_CATALOG_COUNT; i++) {
        if (strcmp(ban_asset_catalog[i].name, name) == 0) return &ban_asset_catalog[i];
    }
    return NULL;
}

int main(void) {
    int i;
    int expected_n = (int)(sizeof EXPECTED / sizeof EXPECTED[0]);

    CHECK(BAN_ASSET_CATALOG_COUNT == expected_n);
    CHECK(BAN_PAL_ORANGE == 0xEE7744u);
    CHECK(BAN_PAL_ORANGE_DARK == 0xC85D36u);
    CHECK(BAN_PAL_OUTLINE == 0x171312u);
    CHECK(BAN_PAL_HAIR == 0x35241Du);
    CHECK(BAN_PAL_BELLY == 0xF8F2E7u);
    CHECK(BAN_PAL_BLUE == 0x378BD6u);
    CHECK(BAN_PAL_RED == 0xE84137u);
    CHECK(BAN_PAL_YELLOW == 0xF8BF39u);
    CHECK(BAN_PAL_GREEN == 0x3DB460u);
    CHECK(BAN_PAL_CREAM == 0xF2EBD8u);
    CHECK(BAN_PAL_INK == 0x22201Cu);
    CHECK(BAN_PAL_CRT_BG == 0x14201Bu);

    for (i = 0; i < expected_n; i++) {
        const ban_asset_info_t *got = find_name(EXPECTED[i].name);

        if (!got) {
            fprintf(stderr, "missing asset %s\n", EXPECTED[i].name);
            g_fails++;
            continue;
        }
        CHECK(got->w == EXPECTED[i].w);
        CHECK(got->h == EXPECTED[i].h);
        CHECK(got->partial == 1);
        CHECK((got->w % 2) == 0);
        if (strstr(got->name, "_x2") != NULL) {
            fprintf(stderr, "old x2 asset still packed: %s\n", got->name);
            g_fails++;
        }
    }
    CHECK(find_name("dario_normal") == NULL);
    CHECK(find_name("gift") == NULL);
    CHECK(find_name("ban_stamp") == NULL);

    if (g_fails) {
        fprintf(stderr, "asset catalog: %d checks failed\n", g_fails);
        return 1;
    }
    return 0;
}
