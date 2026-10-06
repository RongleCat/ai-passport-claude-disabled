// main/ban_ui_ban.c —— FINAL REVIEW、印章砸下、封号页。扫描线只铺在这三页后面。
#include "ban_ui_internal.h"

#include <stdio.h>

#define DEBRIS_N 18
#define STAMP_X 20
#define STAMP_REST 108
/* 封号页静止章落在胸腹，不压眼镜和红角。 */
#define BAN_STAMP_Y 80
#define BAN_ROW_X 66
#define BAN_ROW_Y 158

static lv_obj_t *s_fill;
static lv_obj_t *s_dario;
static lv_obj_t *s_stamp;
static lv_obj_t *s_flick;
static lv_obj_t *s_hx;
static lv_obj_t *s_vx;
static lv_obj_t *s_bit[DEBRIS_N];

static const int8_t BURST_X[DEBRIS_N] = {
    14, -14, 20, -18, 8, -8, 24, -22, 4, -4, 16, -16, 10, -20, 22, -6, 18, -12,
};
static const int8_t BURST_Y[DEBRIS_N] = {
    -10, -8, 4, 8, -16, 12, -2, 6, -14, 14, 2, -12, 10, -6, 8, 16, -4, 4,
};
static const uint32_t BURST_HEX[4] = {
    BAN_PAL_RED, BAN_PAL_YELLOW, BAN_PAL_CREAM, BAN_PAL_ORANGE,
};

static int stamp_y(uint32_t elapsed) {
    if (elapsed >= 520u || elapsed >= BAN_MS_BAN_ANIM) return STAMP_REST;
    if (elapsed < 260u) {
        uint32_t p = elapsed * 256u / 260u;
        uint32_t q = p * p / 256u;
        return -80 + (int)((STAMP_REST + 98) * (int)q / 256);
    }
    if (elapsed < 400u) {
        uint32_t p = (elapsed - 260u) * 256u / 140u;
        return (STAMP_REST + 16) - (int)(16 * (int)p / 256);
    }
    return STAMP_REST;
}

void ban_view_final_shutdown(void) {
    s_fill = NULL;
    s_dario = NULL;
}

void ban_view_final_create(lv_obj_t *page) {
    lv_obj_t *title;
    lv_obj_t *track;

    ban_scanlines(page);
    title = ban_label(page, &ban_font_zh_16, BAN_PAL_YELLOW, ban_line(BAN_COPY_FINAL, 0));
    ban_label_mid(title, 6);
    track = ban_box(page, 20, 32, 200, 16, BAN_PAL_INK, BAN_PAL_YELLOW);
    s_fill = lv_obj_create(track);
    ban_style_flat(s_fill);
    lv_obj_set_pos(s_fill, 2, 2);
    lv_obj_set_size(s_fill, 0, 12);
    lv_obj_set_style_radius(s_fill, 6, 0);
    lv_obj_set_style_bg_color(s_fill, ban_rgb(BAN_PAL_YELLOW), 0);
    lv_obj_set_style_bg_opa(s_fill, LV_OPA_COVER, 0);
    s_dario = ban_dario_new(page, 40, 64, BAN_SPRITE_160);
}

void ban_view_final_update(lv_obj_t *page) {
    uint32_t elapsed = ban_game_elapsed();
    int width = (int)(elapsed * 196u / BAN_MS_FINAL_REVIEW);
    uint32_t hex = elapsed * 2u >= BAN_MS_FINAL_REVIEW ? BAN_PAL_RED : BAN_PAL_YELLOW;

    (void)page;
    if (width > 196) width = 196;
    lv_obj_set_width(s_fill, width);
    lv_obj_set_style_bg_color(s_fill, ban_rgb(hex), 0);
    ban_dario_sync(s_dario, 40, 64, BAN_SPRITE_160);
}

static void hide_burst(void) {
    int i;

    if (s_hx) lv_obj_add_flag(s_hx, LV_OBJ_FLAG_HIDDEN);
    if (s_vx) lv_obj_add_flag(s_vx, LV_OBJ_FLAG_HIDDEN);
    for (i = 0; i < DEBRIS_N; i++) {
        if (s_bit[i]) lv_obj_add_flag(s_bit[i], LV_OBJ_FLAG_HIDDEN);
    }
}

void ban_view_stamp_shutdown(void) {
    int i;

    s_dario = NULL;
    s_stamp = NULL;
    s_hx = NULL;
    s_vx = NULL;
    for (i = 0; i < DEBRIS_N; i++) s_bit[i] = NULL;
}

void ban_view_stamp_create(lv_obj_t *page) {
    int i;

    ban_scanlines(page);
    s_dario = ban_dario_new(page, 40, 16, BAN_SPRITE_160);
    s_stamp = lv_image_create(page);
    lv_image_set_src(s_stamp, &ban_img_ban_stamp_200);
    lv_obj_set_pos(s_stamp, STAMP_X, -80);
    lv_obj_remove_flag(s_stamp, LV_OBJ_FLAG_CLICKABLE);

    s_hx = lv_obj_create(page);
    ban_style_flat(s_hx);
    lv_obj_set_style_bg_color(s_hx, ban_rgb(BAN_PAL_YELLOW), 0);
    lv_obj_set_style_bg_opa(s_hx, LV_OPA_COVER, 0);
    lv_obj_add_flag(s_hx, LV_OBJ_FLAG_HIDDEN);
    s_vx = lv_obj_create(page);
    ban_style_flat(s_vx);
    lv_obj_set_style_bg_color(s_vx, ban_rgb(BAN_PAL_RED), 0);
    lv_obj_set_style_bg_opa(s_vx, LV_OPA_COVER, 0);
    lv_obj_add_flag(s_vx, LV_OBJ_FLAG_HIDDEN);

    for (i = 0; i < DEBRIS_N; i++) {
        int side = (i % 3 == 0) ? 4 : 3;
        s_bit[i] = lv_obj_create(page);
        ban_style_flat(s_bit[i]);
        lv_obj_set_size(s_bit[i], side, side);
        lv_obj_set_style_radius(s_bit[i], side / 2, 0);
        lv_obj_set_style_bg_color(s_bit[i], ban_rgb(BURST_HEX[i & 3]), 0);
        lv_obj_set_style_bg_opa(s_bit[i], LV_OPA_COVER, 0);
        lv_obj_add_flag(s_bit[i], LV_OBJ_FLAG_HIDDEN);
    }
}

void ban_view_stamp_update(lv_obj_t *page) {
    uint32_t elapsed = ban_game_elapsed();
    int y = stamp_y(elapsed);
    int age;
    int i;

    (void)page;
    ban_dario_sync(s_dario, 40, 16, BAN_SPRITE_160);
    if (s_stamp && lv_obj_get_y(s_stamp) != y) lv_obj_set_y(s_stamp, y);
    age = (int)elapsed - 260;
    if (age < 0 || age > 260) {
        hide_burst();
        return;
    }
    {
        int span = 24 + age / 2;
        int step = age / 12;
        if (span > 200) span = 200;
        lv_obj_set_size(s_hx, span, 3);
        lv_obj_set_pos(s_hx, 120 - span / 2, y + 36);
        lv_obj_remove_flag(s_hx, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_size(s_vx, 3, span * 2 / 3);
        lv_obj_set_pos(s_vx, 119, y + 36 - span / 3);
        lv_obj_remove_flag(s_vx, LV_OBJ_FLAG_HIDDEN);
        for (i = 0; i < DEBRIS_N; i++) {
            int x = 120 + BURST_X[i] * step / 3;
            int py = y + 32 + BURST_Y[i] * step / 3;
            if (x < BAN_SAFE) x = BAN_SAFE;
            if (x > BAN_SCR_W - BAN_SAFE - 4) x = BAN_SCR_W - BAN_SAFE - 4;
            if (py < 2) py = 2;
            if (py > BAN_PAGE_H - 8) py = BAN_PAGE_H - 8;
            lv_obj_set_pos(s_bit[i], x, py);
            lv_obj_remove_flag(s_bit[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void ban_view_banned_shutdown(void) {
    s_dario = NULL;
    s_stamp = NULL;
    s_flick = NULL;
}

void ban_view_banned_create(lv_obj_t *page) {
    lv_obj_t *prob;
    lv_obj_t *pct;
    lv_obj_t *why;
    lv_obj_t *reason;
    lv_obj_t *btn;
    lv_obj_t *icon;
    lv_obj_t *label;
    char buf[16];

    ban_scanlines(page);
    s_dario = ban_dario_new(page, 40, 0, BAN_SPRITE_160);

    s_stamp = lv_image_create(page);
    lv_image_set_src(s_stamp, &ban_img_ban_stamp_200);
    lv_obj_set_pos(s_stamp, STAMP_X, BAN_STAMP_Y);
    lv_obj_remove_flag(s_stamp, LV_OBJ_FLAG_CLICKABLE);
    s_flick = lv_obj_create(page);
    ban_style_flat(s_flick);
    lv_obj_set_size(s_flick, 120, 3);
    lv_obj_set_pos(s_flick, 60, BAN_STAMP_Y + 58);
    lv_obj_set_style_radius(s_flick, 2, 0);
    lv_obj_set_style_bg_color(s_flick, ban_rgb(BAN_PAL_YELLOW), 0);
    lv_obj_set_style_bg_opa(s_flick, LV_OPA_COVER, 0);

    icon = lv_image_create(page);
    lv_image_set_src(icon, &ban_img_icon_ban_user_40);
    lv_obj_set_pos(icon, BAN_ROW_X, BAN_ROW_Y);
    lv_obj_remove_flag(icon, LV_OBJ_FLAG_CLICKABLE);
    prob = ban_label(page, &ban_font_zh_12, BAN_PAL_YELLOW, ban_line(BAN_COPY_PROB, 0));
    lv_obj_set_pos(prob, BAN_ROW_X + 48, BAN_ROW_Y + 2);
    snprintf(buf, sizeof buf, "%d%%", ban_game_prob());
    pct = ban_label(page, &ban_font_zh_24, BAN_PAL_RED, buf);
    lv_obj_set_pos(pct, BAN_ROW_X + 48, BAN_ROW_Y + 16);
    why = ban_label(page, &ban_font_zh_12, BAN_PAL_BELLY, ban_line(BAN_COPY_REASON_LABEL, 0));
    lv_obj_set_pos(why, 76, 210);
    reason = ban_label(page, &ban_font_zh_16, BAN_PAL_CREAM, ban_line(BAN_COPY_REASON, 0));
    lv_obj_set_pos(reason, 132, 208);

    btn = ban_box(page, 16, 232, 208, 22, BAN_PAL_RED, BAN_PAL_OUTLINE);
    label = ban_label(btn, &ban_font_zh_16, BAN_PAL_CREAM, ban_line(BAN_COPY_APPEAL_BTN, 0));
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void ban_view_banned_update(lv_obj_t *page) {
    int on = ((lv_tick_get() / 180u) & 1u) == 0;

    (void)page;
    ban_dario_sync(s_dario, 40, 0, BAN_SPRITE_160);
    if (!s_stamp) return;
    lv_obj_set_y(s_stamp, on ? BAN_STAMP_Y - 2 : BAN_STAMP_Y);
    if (s_flick) {
        lv_obj_set_y(s_flick, (on ? BAN_STAMP_Y - 2 : BAN_STAMP_Y) + 58);
        if (on) lv_obj_remove_flag(s_flick, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(s_flick, LV_OBJ_FLAG_HIDDEN);
    }
}
