// main/ban_ui_over.c —— 结束页。96 角色，中文章落在身体下面，英文章只留标题旁的一枚字。
#include "ban_ui_internal.h"

#include <stdio.h>

#define BITS_N 8
#define CN_X 20
#define CN_REST 136
#define BAR_X 8
#define BAR_Y 210
#define BAR_W 224
#define BAR_H 22
#define BTN_Y 238
#define BTN_H 18
/* 边框 2 + 左右内边距 8 + 上下内边距 2。创建时量内容区会得到 0。 */
#define CLIP_W (BAR_W - 4 - 16)
#define CLIP_H (BAR_H - 4 - 4)

static lv_obj_t *s_dario;
static lv_obj_t *s_cursor;
static lv_obj_t *s_tick;
static lv_obj_t *s_cn;
static lv_obj_t *s_bit[BITS_N];
static int s_bit_y[BITS_N];
static int s_tick_x;
static int s_tick_span;
static int s_cn_hit;
static uint32_t s_born;

static const int BIT_X[BITS_N] = { 12, 28, 44, 188, 204, 220, 16, 208 };
static const uint32_t BIT_HEX[4] = {
    BAN_PAL_RED, BAN_PAL_YELLOW, BAN_PAL_CREAM, BAN_PAL_ORANGE,
};

static const char TICK_PHRASE[] = "封号成功   再测一次   申诉失败    ";

static const lv_point_precise_t s_check[] = {
    { 0, 8 },
    { 6, 14 },
    { 16, 1 },
};

static int cn_y(uint32_t elapsed) {
    uint32_t t;

    if (elapsed < 180u) return -72;
    t = elapsed - 180u;
    if (t >= 420u) return CN_REST;
    if (t < 240u) {
        uint32_t p = t * 256u / 240u;
        uint32_t q = p * p / 256u;
        return -72 + (int)((CN_REST + 88) * (int)q / 256);
    }
    {
        uint32_t p = (t - 240u) * 256u / 180u;
        return (CN_REST + 14) - (int)(14 * (int)p / 256);
    }
}

void ban_view_over_shutdown(void) {
    int i;

    s_dario = NULL;
    s_cursor = NULL;
    s_tick = NULL;
    s_cn = NULL;
    s_cn_hit = 0;
    s_tick_span = 0;
    s_born = 0;
    for (i = 0; i < BITS_N; i++) s_bit[i] = NULL;
}

void ban_view_over_create(lv_obj_t *page) {
    lv_obj_t *badge;
    lv_obj_t *word;
    lv_obj_t *kicker;
    lv_obj_t *ok;
    lv_obj_t *line;
    lv_obj_t *bar;
    lv_obj_t *clip;
    lv_obj_t *btn;
    lv_obj_t *label;
    lv_point_t sz;
    char tick[128];
    int i;

    badge = ban_box(page, 8, 0, 64, 16, BAN_PAL_RED, BAN_PAL_OUTLINE);
    word = ban_label(badge, &ban_font_latin_8, BAN_PAL_CREAM, ban_line(BAN_COPY_BANNED, 0));
    lv_obj_align(word, LV_ALIGN_CENTER, 0, 0);
    kicker = ban_label(page, &ban_font_latin_8, BAN_PAL_YELLOW, ban_line(BAN_COPY_CLOSED, 0));
    lv_obj_set_pos(kicker, 80, 4);

    ok = ban_label(page, &ban_font_zh_16, BAN_PAL_CREAM, ban_line(BAN_COPY_SUCCESS, 0));
    lv_obj_set_pos(ok, 70, 18);
    line = lv_line_create(page);
    lv_line_set_points(line, s_check, 3);
    lv_obj_set_pos(line, 142, 18);
    lv_obj_set_style_line_width(line, 3, 0);
    lv_obj_set_style_line_color(line, ban_rgb(BAN_PAL_GREEN), 0);
    lv_obj_set_style_line_rounded(line, true, 0);
    lv_obj_remove_flag(line, LV_OBJ_FLAG_CLICKABLE);

    s_dario = ban_dario_new(page, 72, 38, BAN_SPRITE_96);

    s_cn = lv_image_create(page);
    lv_image_set_src(s_cn, &ban_img_ban_stamp_cn_200);
    lv_obj_set_pos(s_cn, CN_X, -72);
    lv_obj_remove_flag(s_cn, LV_OBJ_FLAG_CLICKABLE);

    for (i = 0; i < BITS_N; i++) {
        int side = (i & 1) ? 4 : 3;

        s_bit_y[i] = 148 + (i * 9) % 40;
        s_bit[i] = lv_obj_create(page);
        ban_style_flat(s_bit[i]);
        lv_obj_set_size(s_bit[i], side, side);
        lv_obj_set_style_radius(s_bit[i], 2, 0);
        lv_obj_set_pos(s_bit[i], BIT_X[i], s_bit_y[i]);
        lv_obj_set_style_bg_color(s_bit[i], ban_rgb(BIT_HEX[i & 3]), 0);
        lv_obj_set_style_bg_opa(s_bit[i], LV_OPA_COVER, 0);
    }

    bar = ban_box(page, BAR_X, BAR_Y, BAR_W, BAR_H, BAN_PAL_RED, BAN_PAL_OUTLINE);
    lv_obj_set_style_pad_left(bar, 8, 0);
    lv_obj_set_style_pad_right(bar, 8, 0);
    lv_obj_set_style_pad_top(bar, 2, 0);
    lv_obj_set_style_pad_bottom(bar, 2, 0);
    clip = lv_obj_create(bar);
    ban_style_flat(clip);
    lv_obj_set_pos(clip, 0, 0);
    lv_obj_set_size(clip, CLIP_W, CLIP_H);
    lv_obj_set_style_bg_opa(clip, LV_OPA_TRANSP, 0);
    lv_obj_remove_flag(clip, LV_OBJ_FLAG_CLICKABLE);
    snprintf(tick, sizeof tick, "%s%s", TICK_PHRASE, TICK_PHRASE);
    s_tick = ban_label(clip, &ban_font_zh_12, BAN_PAL_CREAM, tick);
    lv_obj_set_pos(s_tick, 0, 0);
    lv_text_get_size(&sz, TICK_PHRASE, &ban_font_zh_12, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    s_tick_span = sz.x;
    s_tick_x = 0;

    btn = ban_box(page, 36, BTN_Y, 168, BTN_H, BAN_PAL_ORANGE, BAN_PAL_OUTLINE);
    s_cursor = ban_cursor_new(btn);
    lv_obj_set_pos(s_cursor, 6, 2);
    label = ban_label(btn, &ban_font_zh_12, BAN_PAL_INK, ban_line(BAN_COPY_RETRY, 0));
    lv_obj_align(label, LV_ALIGN_CENTER, 8, 0);
    s_cn_hit = 0;
    s_born = lv_tick_get();
}

void ban_view_over_update(lv_obj_t *page) {
    uint32_t elapsed = lv_tick_get() - s_born;
    int y;
    int i;

    (void)page;
    ban_dario_sync(s_dario, 72, 38, BAN_SPRITE_96);
    y = cn_y(elapsed);
    if (s_cn && lv_obj_get_y(s_cn) != y) lv_obj_set_y(s_cn, y);
    if (!s_cn_hit && elapsed >= 420u) {
        s_cn_hit = 1;
        ban_ui_shake(160);
    }
    for (i = 0; i < BITS_N; i++) {
        if (!s_bit[i]) continue;
        s_bit_y[i] += 2 + (i % 3);
        if (s_bit_y[i] > 200) s_bit_y[i] = 144;
        lv_obj_set_y(s_bit[i], s_bit_y[i]);
    }
    if (s_tick && s_tick_span > 0) {
        s_tick_x -= 2;
        if (s_tick_x <= -s_tick_span) s_tick_x += s_tick_span;
        lv_obj_set_x(s_tick, s_tick_x);
    }
    ban_cursor_nudge(s_cursor, 6, 2);
}
