// main/ban_ui_scan.c —— 扫描。状态字只用字体里有的 OK / 可疑 / ?。
#include "ban_ui_internal.h"

#include <string.h>

typedef struct {
    uint32_t at_ms;
    ban_copy_id_t name;
    const char *mark;
    uint32_t mark_hex;
} scan_row_t;

static const scan_row_t ROWS[4] = {
    { 120, BAN_COPY_SCAN_IP, "OK", BAN_PAL_GREEN },
    { 620, BAN_COPY_SCAN_DEVICE, "OK", BAN_PAL_GREEN },
    { 1120, BAN_COPY_SCAN_LANG, "可疑", BAN_PAL_ORANGE },
    { 1620, BAN_COPY_SCAN_ID, "?", BAN_PAL_YELLOW },
};

static lv_obj_t *s_title;
static lv_obj_t *s_name[4];
static lv_obj_t *s_mark[4];
static lv_obj_t *s_beam;
static lv_obj_t *s_dario;

#define SCAN_X 72
#define SCAN_Y 156

void ban_view_scan_shutdown(void) {
    int i;

    s_title = NULL;
    s_beam = NULL;
    s_dario = NULL;
    for (i = 0; i < 4; i++) {
        s_name[i] = NULL;
        s_mark[i] = NULL;
    }
}

void ban_view_scan_create(lv_obj_t *page) {
    int i;

    s_title = ban_label(page, &ban_font_zh_24, BAN_PAL_CREAM, "");
    ban_label_mid(s_title, 4);
    for (i = 0; i < 4; i++) {
        s_name[i] = ban_label(page, &ban_font_zh_16, BAN_PAL_CREAM, "");
        lv_obj_set_pos(s_name[i], BAN_SAFE + 8, 40 + i * 28);
        lv_obj_set_width(s_name[i], 120);
        s_mark[i] = ban_label(page, &ban_font_zh_16, ROWS[i].mark_hex, "");
        lv_obj_set_pos(s_mark[i], 168, 40 + i * 28);
        lv_obj_add_flag(s_mark[i], LV_OBJ_FLAG_HIDDEN);
    }
    s_beam = lv_obj_create(page);
    ban_style_flat(s_beam);
    lv_obj_set_size(s_beam, 208, 6);
    lv_obj_set_style_radius(s_beam, 3, 0);
    lv_obj_set_style_bg_color(s_beam, ban_rgb(BAN_PAL_GREEN), 0);
    lv_obj_set_style_bg_opa(s_beam, LV_OPA_COVER, 0);
    lv_obj_remove_flag(s_beam, LV_OBJ_FLAG_CLICKABLE);
    s_dario = ban_dario_new(page, SCAN_X, SCAN_Y, BAN_SPRITE_96);
}

void ban_view_scan_update(lv_obj_t *page) {
    uint32_t elapsed = ban_game_elapsed();
    int alert = ban_game_scan_alert();
    int i;
    int beam_y;

    (void)page;
    if (alert) {
        int on = ((elapsed / 160u) & 1u) == 0u;
        ban_type_set(s_title, ban_line(BAN_COPY_SCAN_ALERT, 0), elapsed - BAN_MS_SCAN_ALERT, 28);
        lv_obj_set_style_text_color(s_title, ban_rgb(on ? BAN_PAL_RED : BAN_PAL_YELLOW), 0);
    } else {
        ban_type_set(s_title, ban_line(BAN_COPY_SCAN_TITLE, 0), elapsed, 70);
        lv_obj_set_style_text_color(s_title, ban_rgb(BAN_PAL_CREAM), 0);
    }
    for (i = 0; i < 4; i++) {
        if (elapsed < ROWS[i].at_ms) continue;
        ban_type_set(s_name[i], ban_line(ROWS[i].name, 0), elapsed - ROWS[i].at_ms, 50);
        if (elapsed >= ROWS[i].at_ms + 160u) {
            if (strcmp(lv_label_get_text(s_mark[i]), ROWS[i].mark) != 0) {
                lv_label_set_text(s_mark[i], ROWS[i].mark);
            }
            lv_obj_remove_flag(s_mark[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    beam_y = 38 + (int)((elapsed % 900u) * 100u / 900u);
    lv_obj_set_pos(s_beam, 16, beam_y);
    ban_dario_sync(s_dario, SCAN_X, SCAN_Y, BAN_SPRITE_96);
}
