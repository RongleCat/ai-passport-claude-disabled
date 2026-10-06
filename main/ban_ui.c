// main/ban_ui.c —— 外壳、风险条、页面切换。定时器跑在 LVGL 任务里。
#include "ban_ui.h"

#include "ban_port.h"
#include "ban_ui_internal.h"

#include <stdio.h>
#include <string.h>

/* 6px 里 1px 略亮。只在 FINAL / BAN 页平铺，铺在角色后面，不盖住身体。 */
#define BAN_SCAN_W 8
#define BAN_SCAN_H 6

static const uint8_t s_scan_map[] = {
    0xc5, 0x21, 0xc5, 0x21, 0xc5, 0x21, 0xc5, 0x21, 0xc5, 0x21, 0xc5, 0x21, 0xc5, 0x21, 0xc5, 0x21,
    0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11,
    0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11,
    0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11,
    0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11,
    0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11, 0x03, 0x11,
};

static const lv_image_dsc_t s_scan_tile = {
    .header.magic = LV_IMAGE_HEADER_MAGIC,
    .header.cf = LV_COLOR_FORMAT_RGB565,
    .header.w = BAN_SCAN_W,
    .header.h = BAN_SCAN_H,
    .header.stride = BAN_SCAN_W * 2,
    .data_size = sizeof s_scan_map,
    .data = s_scan_map,
};

typedef struct {
    int slide;
    void (*create)(lv_obj_t *page);
    void (*update)(lv_obj_t *page);
    void (*shutdown)(void);
} ban_view_ops_t;

static const ban_view_ops_t OPS_HOME = {
    0, ban_view_home_create, ban_view_home_update, ban_view_home_shutdown,
};
static const ban_view_ops_t OPS_SCAN = {
    1, ban_view_scan_create, ban_view_scan_update, ban_view_scan_shutdown,
};
static const ban_view_ops_t OPS_CHOICE = {
    1, ban_view_choice_create, ban_view_choice_update, ban_view_choice_shutdown,
};
static const ban_view_ops_t OPS_AUTO = {
    1, ban_view_auto_create, ban_view_auto_update, ban_view_auto_shutdown,
};
static const ban_view_ops_t OPS_RESULT = {
    1, ban_view_result_create, ban_view_result_update, ban_view_result_shutdown,
};
static const ban_view_ops_t OPS_FINAL = {
    1, ban_view_final_create, ban_view_final_update, ban_view_final_shutdown,
};
static const ban_view_ops_t OPS_STAMP = {
    0, ban_view_stamp_create, ban_view_stamp_update, ban_view_stamp_shutdown,
};
static const ban_view_ops_t OPS_BANNED = {
    1, ban_view_banned_create, ban_view_banned_update, ban_view_banned_shutdown,
};
static const ban_view_ops_t OPS_APPEAL = {
    1, ban_view_appeal_create, ban_view_appeal_update, ban_view_appeal_shutdown,
};
static const ban_view_ops_t OPS_APPEAL_DONE = {
    1, ban_view_appeal_done_create, ban_view_appeal_done_update, ban_view_appeal_done_shutdown,
};
static const ban_view_ops_t OPS_OVER = {
    1, ban_view_over_create, ban_view_over_update, ban_view_over_shutdown,
};

static lv_obj_t *s_stage;
static lv_obj_t *s_content;
static lv_obj_t *s_page;
static lv_obj_t *s_flash;
static lv_obj_t *s_meter_face;
static lv_obj_t *s_meter_value;
static lv_obj_t *s_meter_tier;
static lv_obj_t *s_meter_float;
static lv_obj_t *s_meter_track;
static lv_obj_t *s_seg[4];
static lv_obj_t *s_bat_body;
static lv_obj_t *s_bat_fill;
static lv_obj_t *s_bat_nub;
static lv_timer_t *s_timer;
static const ban_view_ops_t *s_ops;
static uint32_t s_view_key;
static uint32_t s_last_tick;
static uint32_t s_shake_until;
static uint32_t s_shock_until;
static ban_phase_t s_shock_phase;
static int s_watch_on;
static int s_shown_risk;
static int s_rounds;
static int s_value_shown;
static int s_float_delta;
static char s_value_text[8] = "0%";
static char s_float_text[8];

static const uint32_t SEG_COLOR[4] = {
    BAN_PAL_GREEN, BAN_PAL_YELLOW, BAN_PAL_ORANGE, BAN_PAL_RED,
};
static const int SEG_AT[5] = { 0, 30, 60, 80, 100 };

lv_color_t ban_rgb(uint32_t hex) {
    return lv_color_make(ban_pal_r(hex), ban_pal_g(hex), ban_pal_b(hex));
}

void ban_style_flat(lv_obj_t *obj) {
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_outline_width(obj, 0, 0);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(obj, LV_LAYOUT_NONE);
}

const char *ban_line(ban_copy_id_t id, int which) {
    const ban_copy_t *copy = ban_copy_by_id(id);
    const char *text;

    if (!copy) return "";
    if (which == 0) text = copy->title;
    else if (which == 1) text = copy->line1;
    else text = copy->line2;
    return text ? text : "";
}

void ban_utf8_n(char *dst, unsigned dst_n, const char *src, int codepoints) {
    unsigned o = 0;

    if (!dst || dst_n == 0) return;
    if (!src || codepoints <= 0) {
        dst[0] = '\0';
        return;
    }
    while (*src && codepoints > 0 && o + 1 < dst_n) {
        unsigned char c = (unsigned char)*src;
        int len = 1;
        int i;

        if (c >= 0xF0) len = 4;
        else if (c >= 0xE0) len = 3;
        else if (c >= 0xC0) len = 2;
        if (o + (unsigned)len >= dst_n) break;
        for (i = 0; i < len; i++) {
            if (!src[i]) break;
            dst[o++] = src[i];
        }
        src += len;
        codepoints--;
    }
    dst[o] = '\0';
}

lv_obj_t *ban_label(lv_obj_t *parent, const lv_font_t *font, uint32_t hex, const char *text) {
    lv_obj_t *label = lv_label_create(parent);

    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, ban_rgb(hex), 0);
    lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0);
    lv_label_set_text(label, text ? text : "");
    lv_obj_remove_flag(label, LV_OBJ_FLAG_SCROLLABLE);
    return label;
}

void ban_label_mid(lv_obj_t *label, int y) {
    lv_obj_set_width(label, BAN_SCR_W);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(label, 0, y);
}

lv_obj_t *ban_box(lv_obj_t *parent, int x, int y, int w, int h, uint32_t bg, uint32_t edge) {
    lv_obj_t *box = lv_obj_create(parent);

    ban_style_flat(box);
    lv_obj_set_pos(box, x, y);
    lv_obj_set_size(box, w, h);
    lv_obj_set_style_radius(box, BAN_RADIUS, 0);
    lv_obj_set_style_bg_color(box, ban_rgb(bg), 0);
    lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(box, ban_rgb(edge), 0);
    lv_obj_set_style_border_width(box, 2, 0);
    return box;
}

lv_obj_t *ban_cursor_new(lv_obj_t *parent) {
    static const int8_t seg_w[5] = { 4, 8, 12, 8, 4 };
    lv_obj_t *box;
    int i;

    box = lv_obj_create(parent);
    ban_style_flat(box);
    lv_obj_set_size(box, 12, 10);
    lv_obj_set_style_bg_opa(box, LV_OPA_TRANSP, 0);
    lv_obj_remove_flag(box, LV_OBJ_FLAG_CLICKABLE);
    for (i = 0; i < 5; i++) {
        lv_obj_t *bar = lv_obj_create(box);

        ban_style_flat(bar);
        lv_obj_set_size(bar, seg_w[i], 2);
        lv_obj_set_pos(bar, 0, i * 2);
        lv_obj_set_style_bg_color(bar, ban_rgb(BAN_PAL_CREAM), 0);
        lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
        lv_obj_remove_flag(bar, LV_OBJ_FLAG_CLICKABLE);
    }
    return box;
}

void ban_cursor_nudge(lv_obj_t *cursor, int x, int y) {
    int nx;

    if (!cursor) return;
    nx = x + (((lv_tick_get() / 160u) & 1u) ? 4 : 0);
    if (lv_obj_get_x(cursor) != nx || lv_obj_get_y(cursor) != y) lv_obj_set_pos(cursor, nx, y);
}

void ban_scanlines(lv_obj_t *page) {
    lv_obj_t *img = lv_image_create(page);

    lv_image_set_src(img, &s_scan_tile);
    lv_image_set_inner_align(img, LV_IMAGE_ALIGN_TILE);
    lv_obj_set_size(img, BAN_SCR_W, BAN_PAGE_H);
    lv_obj_set_pos(img, 0, 0);
    lv_obj_remove_flag(img, LV_OBJ_FLAG_CLICKABLE);
}

static ban_face_t shown_face(void) {
    ban_phase_t phase = ban_game_phase();
    ban_risk_tier_t tier = ban_risk_tier(ban_game_risk());

    if (phase >= BAN_PHASE_BAN_ANIM || tier == BAN_TIER_BANNED) return BAN_FACE_BANNED;
    if (tier == BAN_TIER_HIGH || tier == BAN_TIER_WATCHING) return BAN_FACE_ANGRY;
    if (tier == BAN_TIER_SUSPICIOUS) return BAN_FACE_SUSPICIOUS;
    return BAN_FACE_NORMAL;
}

static int blink_on(ban_face_t face, uint32_t now) {
    uint32_t t;

    if (face != BAN_FACE_NORMAL) return 0;
    if (now < s_shock_until) return 0;
    t = now % 4200u;
    return t >= 4080u;
}

static const lv_image_dsc_t *pick_sprite(ban_face_t face, int size, int blink, int shock) {
    if (shock) {
        if (size >= BAN_SPRITE_160) return &ban_img_dario_shock_160;
        if (size >= BAN_SPRITE_96) return &ban_img_dario_shock_96;
        return &ban_img_dario_shock_48;
    }
    if (blink) {
        if (size >= BAN_SPRITE_160) return &ban_img_dario_blink_160;
        if (size >= BAN_SPRITE_96) return &ban_img_dario_blink_96;
        return &ban_img_dario_blink_48;
    }
    if (size >= BAN_SPRITE_160) {
        switch (face) {
        case BAN_FACE_SUSPICIOUS: return &ban_img_dario_suspicious_160;
        case BAN_FACE_ANGRY: return &ban_img_dario_angry_160;
        case BAN_FACE_BANNED: return &ban_img_dario_banned_160;
        default: return &ban_img_dario_normal_160;
        }
    }
    if (size >= BAN_SPRITE_96) {
        switch (face) {
        case BAN_FACE_SUSPICIOUS: return &ban_img_dario_suspicious_96;
        case BAN_FACE_ANGRY: return &ban_img_dario_angry_96;
        case BAN_FACE_BANNED: return &ban_img_dario_banned_96;
        default: return &ban_img_dario_normal_96;
        }
    }
    switch (face) {
    case BAN_FACE_SUSPICIOUS: return &ban_img_dario_suspicious_48;
    case BAN_FACE_ANGRY: return &ban_img_dario_angry_48;
    case BAN_FACE_BANNED: return &ban_img_dario_banned_48;
    default: return &ban_img_dario_normal_48;
    }
}

static int shock_on(uint32_t now) {
    /* +40 会直接跨过 100。惊吓脸要盖住封号脸，几百毫秒后再回到 banned。 */
    return now < s_shock_until;
}

lv_obj_t *ban_dario_new(lv_obj_t *parent, int x, int y, int size) {
    lv_obj_t *img = lv_image_create(parent);
    ban_face_t face = shown_face();
    uint32_t now = lv_tick_get();

    lv_image_set_src(img, pick_sprite(face, size, blink_on(face, now), shock_on(now)));
    lv_obj_set_pos(img, x, y);
    lv_obj_remove_flag(img, LV_OBJ_FLAG_CLICKABLE);
    return img;
}

int ban_ui_bob(void) {
    static const int seq[4] = { 0, 1, 2, 1 };
    return seq[(lv_tick_get() / 180u) & 3u];
}

void ban_ui_shake(uint32_t ms) {
    uint32_t until = lv_tick_get() + ms;
    if (until > s_shake_until) s_shake_until = until;
}

void ban_dario_sync(lv_obj_t *img, int x, int y, int size) {
    const lv_image_dsc_t *want;
    ban_face_t face;
    uint32_t now;
    int dx = 0;
    int yy;

    if (!img) return;
    face = shown_face();
    now = lv_tick_get();
    want = pick_sprite(face, size, blink_on(face, now), shock_on(now));
    if (lv_image_get_src(img) != want) lv_image_set_src(img, want);
    if (shock_on(now)) dx = 0;
    else if (face == BAN_FACE_ANGRY) dx = ((now / 45u) & 1u) ? 2 : -2;
    else if (size >= BAN_SPRITE_96) dx = ((now / 640u) & 1u) ? 1 : 0;
    yy = y + ban_ui_bob();
    if (lv_obj_get_x(img) != x + dx || lv_obj_get_y(img) != yy) lv_obj_set_pos(img, x + dx, yy);
}

void ban_type_set(lv_obj_t *label, const char *src, uint32_t elapsed_ms, uint32_t ms_per_cp) {
    char buf[48];
    int cps;

    if (!label) return;
    if (!src) src = "";
    if (ms_per_cp == 0) ms_per_cp = 1;
    cps = (int)(elapsed_ms / ms_per_cp);
    ban_utf8_n(buf, sizeof buf, src, cps);
    if (strcmp(lv_label_get_text(label), buf) != 0) lv_label_set_text(label, buf);
}

static void stop_tree(lv_obj_t *obj) {
    uint32_t i;
    uint32_t n;

    if (!obj) return;
    lv_anim_delete(obj, NULL);
    n = lv_obj_get_child_count(obj);
    for (i = 0; i < n; i++) stop_tree(lv_obj_get_child(obj, i));
}

static void slide_x(lv_obj_t *obj, int from, uint32_t ms) {
    lv_anim_t anim;

    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_values(&anim, from, 0);
    lv_anim_set_duration(&anim, ms);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
    lv_anim_start(&anim);
}

static void close_page(void) {
    if (s_ops && s_ops->shutdown) s_ops->shutdown();
    s_ops = NULL;
    if (!s_page) return;
    stop_tree(s_page);
    lv_obj_delete(s_page);
    s_page = NULL;
}

static const ban_view_ops_t *ops_for(ban_phase_t phase) {
    switch (phase) {
    case BAN_PHASE_HOME: return &OPS_HOME;
    case BAN_PHASE_SCAN: return &OPS_SCAN;
    case BAN_PHASE_EVENT:
        if (ban_event_def(ban_game_event())->type == BAN_EVENT_CHOICE) return &OPS_CHOICE;
        return &OPS_AUTO;
    case BAN_PHASE_EVENT_RESULT: return &OPS_RESULT;
    case BAN_PHASE_FINAL_REVIEW: return &OPS_FINAL;
    case BAN_PHASE_BAN_ANIM: return &OPS_STAMP;
    case BAN_PHASE_BAN: return &OPS_BANNED;
    case BAN_PHASE_APPEAL: return &OPS_APPEAL;
    case BAN_PHASE_APPEAL_RESULT: return &OPS_APPEAL_DONE;
    case BAN_PHASE_GAME_OVER: return &OPS_OVER;
    default: return &OPS_HOME;
    }
}

static uint32_t view_key(void) {
    ban_phase_t phase = ban_game_phase();
    uint32_t key = (uint32_t)phase + 1u;

    if (phase == BAN_PHASE_EVENT || phase == BAN_PHASE_EVENT_RESULT) {
        key |= (uint32_t)ban_game_event() << 8;
        key |= (uint32_t)ban_game_event_index() << 16;
    }
    return key;
}

static void open_page(void) {
    const ban_view_ops_t *ops = ops_for(ban_game_phase());
    int from = ops->slide ? BAN_SCR_W : 0;

    close_page();
    s_page = lv_obj_create(s_content);
    ban_style_flat(s_page);
    lv_obj_set_size(s_page, BAN_SCR_W, BAN_PAGE_H);
    lv_obj_set_pos(s_page, from, 0);
    lv_obj_set_style_bg_opa(s_page, LV_OPA_TRANSP, 0);
    lv_obj_remove_flag(s_page, LV_OBJ_FLAG_CLICKABLE);
    s_ops = ops;
    s_view_key = view_key();
    ops->create(s_page);
    if (from != 0) slide_x(s_page, from, 160);
}

bool ban_ui_settled(void) {
    return s_page && lv_obj_get_x(s_page) == 0;
}

static void seg_widths(int shown) {
    int gap = 2;
    int usable = 142 - gap * 3;
    int x = 62;
    int i;

    for (i = 0; i < 4; i++) {
        int zone = SEG_AT[i + 1] - SEG_AT[i];
        int zw = usable * zone / 100;
        int lit = 0;

        if (shown >= SEG_AT[i + 1]) lit = zw;
        else if (shown > SEG_AT[i]) lit = zw * (shown - SEG_AT[i]) / zone;
        lv_obj_set_pos(s_seg[i], x, 46);
        lv_obj_set_size(s_seg[i], lit, 8);
        x += zw + gap;
    }
}

static void note_shock(uint32_t now) {
    ban_phase_t phase = ban_game_phase();
    int risk = ban_game_risk();

    if (phase == BAN_PHASE_EVENT_RESULT && s_shock_phase != BAN_PHASE_EVENT_RESULT) {
        if (ban_game_last_delta() >= 40) s_shock_until = now + 420u;
    }
    if (risk >= 80 && risk < 100 && !s_watch_on) {
        s_watch_on = 1;
        s_shock_until = now + 420u;
    }
    if (risk < 80) s_watch_on = 0;
    s_shock_phase = phase;
}

static void meter_refresh(void) {
    int target = ban_game_risk();
    int jumped = 0;
    int tier_color;
    uint32_t tier_hex;
    ban_risk_tier_t tier;

    if (s_shown_risk < target) {
        int step = (target - s_shown_risk) / 4;
        if (step < 1) step = 1;
        s_shown_risk += step;
        if (s_shown_risk > target) s_shown_risk = target;
        jumped = 1;
    } else if (s_shown_risk > target) {
        s_shown_risk = target;
        jumped = 1;
    }

    if (s_shown_risk != s_value_shown) {
        s_value_shown = s_shown_risk;
        snprintf(s_value_text, sizeof s_value_text, "%d%%", s_shown_risk);
        lv_label_set_text_static(s_meter_value, s_value_text);
    }
    lv_obj_set_y(s_meter_value, jumped ? 4 : 8);
    seg_widths(s_shown_risk);
    tier = ban_risk_tier(s_shown_risk);
    tier_color = ban_tier_color(tier);
    tier_hex = SEG_COLOR[tier_color];
    lv_obj_set_style_text_color(s_meter_tier, ban_rgb(tier_hex), 0);
    lv_obj_set_style_text_color(s_meter_value, ban_rgb(tier_hex), 0);
    if (strcmp(lv_label_get_text(s_meter_tier), ban_tier_name(tier)) != 0) {
        lv_label_set_text_static(s_meter_tier, ban_tier_name(tier));
    }
    ban_dario_sync(s_meter_face, BAN_SAFE, BAN_SAFE, BAN_SPRITE_48);

    if (ban_game_phase() == BAN_PHASE_EVENT_RESULT && ban_game_last_delta() > 0) {
        int y = 36 - (int)(ban_game_elapsed() * 16u / BAN_MS_EVENT_RESULT);
        int delta = ban_game_last_delta();

        if (y < 18) y = 18;
        if (delta != s_float_delta) {
            s_float_delta = delta;
            snprintf(s_float_text, sizeof s_float_text, "+%d", delta);
            lv_label_set_text_static(s_meter_float, s_float_text);
        }
        lv_obj_set_pos(s_meter_float, 112, y);
        lv_obj_remove_flag(s_meter_float, LV_OBJ_FLAG_HIDDEN);
    } else if (s_float_delta != 0) {
        s_float_delta = 0;
        s_float_text[0] = '\0';
        lv_label_set_text_static(s_meter_float, s_float_text);
        lv_obj_add_flag(s_meter_float, LV_OBJ_FLAG_HIDDEN);
    }
}

static void battery_refresh(void) {
    int soc = ban_port_battery_soc();
    int fill;

    if (soc < 0) {
        lv_obj_add_flag(s_bat_body, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_bat_fill, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_bat_nub, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_obj_remove_flag(s_bat_body, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(s_bat_fill, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(s_bat_nub, LV_OBJ_FLAG_HIDDEN);
    fill = soc * 12 / 100;
    lv_obj_set_size(s_bat_fill, fill, 4);
    lv_obj_set_style_bg_color(s_bat_fill,
                              ban_rgb(soc > 20 ? BAN_PAL_GREEN : BAN_PAL_RED), 0);
}

static void apply_shake(uint32_t now) {
    int mag = 0;
    int x = 0;

    if (ban_game_phase() == BAN_PHASE_BAN_ANIM) s_shake_until = now + 40;
    if (now < s_shake_until) mag = 3;
    if (mag && ((now / 30u) & 1u)) x = mag;
    else if (mag) x = -mag;
    if (lv_obj_get_x(s_stage) != x) lv_obj_set_x(s_stage, x);
}

static void apply_flash(void) {
    ban_phase_t phase = ban_game_phase();
    uint32_t elapsed = ban_game_elapsed();
    int on = 0;

    if (phase == BAN_PHASE_FINAL_REVIEW && elapsed + 180u >= BAN_MS_FINAL_REVIEW) on = 1;
    if (phase == BAN_PHASE_BAN_ANIM &&
        ((elapsed >= 300 && elapsed < 380) || (elapsed >= 500 && elapsed < 600))) {
        on = 1;
    }
    if (on) lv_obj_remove_flag(s_flash, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(s_flash, LV_OBJ_FLAG_HIDDEN);
}

static void on_phase(ban_phase_t prev, ban_phase_t now) {
    char msg[48];

    snprintf(msg, sizeof msg, "phase %d -> %d risk %d", (int)prev, (int)now, ban_game_risk());
    ban_port_log(msg);
    if (now == BAN_PHASE_SCAN) ban_port_sfx(BAN_SFX_SCAN);
    if (now == BAN_PHASE_EVENT_RESULT && ban_game_last_delta() > 0) {
        ban_port_sfx(BAN_SFX_WARNING);
        s_shake_until = lv_tick_get() + 180;
    }
    if (now == BAN_PHASE_BAN_ANIM) ban_port_sfx(BAN_SFX_BAN);
    if (now == BAN_PHASE_APPEAL_RESULT) ban_port_sfx(BAN_SFX_REJECT);
    if (now == BAN_PHASE_GAME_OVER) {
        s_rounds++;
        snprintf(msg, sizeof msg, "game-over-%d", s_rounds);
        ban_port_log_heap(msg);
    }
    if (now == BAN_PHASE_HOME && prev == BAN_PHASE_GAME_OVER) {
        ban_port_log_heap("idle-home");
    }
}

static void drain_keys(void) {
    int key;

    while (ban_port_take_key(&key)) {
        if (key < 0 || key > 2) continue;
        ban_port_sfx(BAN_SFX_CLICK);
        (void)ban_game_input((ban_input_t)key);
    }
}

static void on_frame(lv_timer_t *timer) {
    uint32_t now = lv_tick_get();
    uint32_t dt = now - s_last_tick;
    ban_phase_t prev;
    uint32_t key;

    (void)timer;
    s_last_tick = now;
    if (dt > 80u) dt = 80u;
    ban_port_battery_poll(now);
    prev = ban_game_phase();
    drain_keys();
    if (ban_game_phase() != prev) {
        on_phase(prev, ban_game_phase());
        prev = ban_game_phase();
    }
    if (dt > 0) ban_game_tick(dt);
    if (ban_game_phase() != prev) on_phase(prev, ban_game_phase());
    note_shock(now);
    key = view_key();
    if (key != s_view_key) open_page();
    if (s_ops && s_page) s_ops->update(s_page);
    meter_refresh();
    battery_refresh();
    apply_shake(now);
    apply_flash();
}

static lv_obj_t *meter_label(lv_obj_t *parent, const lv_font_t *font, uint32_t hex) {
    lv_obj_t *label = ban_label(parent, font, hex, "");
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    return label;
}

static void build_shell(void) {
    lv_obj_t *screen = lv_screen_active();
    int i;

    ban_style_flat(screen);
    lv_obj_set_style_bg_color(screen, ban_rgb(BAN_PAL_CRT_BG), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_text_font(screen, &ban_font_zh_16, 0);

    s_stage = lv_obj_create(screen);
    ban_style_flat(s_stage);
    lv_obj_set_size(s_stage, BAN_SCR_W, BAN_SCR_H);
    lv_obj_set_pos(s_stage, 0, 0);
    lv_obj_set_style_bg_color(s_stage, ban_rgb(BAN_PAL_CRT_BG), 0);
    lv_obj_set_style_bg_opa(s_stage, LV_OPA_COVER, 0);
    lv_obj_remove_flag(s_stage, LV_OBJ_FLAG_CLICKABLE);

    s_meter_face = ban_dario_new(s_stage, BAN_SAFE, BAN_SAFE, BAN_SPRITE_48);

    {
        lv_obj_t *name = ban_label(s_stage, &ban_font_latin_8, BAN_PAL_BELLY, "BAN-O-METER");
        lv_obj_set_pos(name, 60, 8);
    }

    s_meter_value = meter_label(s_stage, &ban_font_zh_16, BAN_PAL_GREEN);
    lv_obj_set_width(s_meter_value, 56);
    lv_obj_set_style_text_align(s_meter_value, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_meter_value, 156, 8);
    lv_label_set_text_static(s_meter_value, s_value_text);

    s_meter_tier = meter_label(s_stage, &ban_font_latin_8, BAN_PAL_GREEN);
    lv_obj_set_width(s_meter_tier, 150);
    lv_obj_set_style_text_align(s_meter_tier, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(s_meter_tier, 60, 24);
    lv_label_set_text_static(s_meter_tier, ban_tier_name(BAN_TIER_SAFE));

    s_meter_track = lv_obj_create(s_stage);
    ban_style_flat(s_meter_track);
    lv_obj_set_pos(s_meter_track, 60, 44);
    lv_obj_set_size(s_meter_track, 148, 12);
    lv_obj_set_style_radius(s_meter_track, 6, 0);
    lv_obj_set_style_bg_color(s_meter_track, ban_rgb(BAN_PAL_INK), 0);
    lv_obj_set_style_bg_opa(s_meter_track, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(s_meter_track, ban_rgb(BAN_PAL_OUTLINE), 0);
    lv_obj_set_style_border_width(s_meter_track, 1, 0);

    for (i = 0; i < 4; i++) {
        s_seg[i] = lv_obj_create(s_stage);
        ban_style_flat(s_seg[i]);
        lv_obj_set_style_radius(s_seg[i], 3, 0);
        lv_obj_set_style_bg_color(s_seg[i], ban_rgb(SEG_COLOR[i]), 0);
        lv_obj_set_style_bg_opa(s_seg[i], LV_OPA_COVER, 0);
        lv_obj_set_size(s_seg[i], 0, 8);
    }

    s_meter_float = meter_label(s_stage, &ban_font_zh_12, BAN_PAL_RED);
    lv_obj_set_width(s_meter_float, 40);
    lv_obj_set_style_text_align(s_meter_float, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_add_flag(s_meter_float, LV_OBJ_FLAG_HIDDEN);

    s_bat_body = ban_box(s_stage, 214, 10, 16, 8, BAN_PAL_CRT_BG, BAN_PAL_BELLY);
    lv_obj_set_style_radius(s_bat_body, 2, 0);
    lv_obj_set_style_border_width(s_bat_body, 1, 0);
    s_bat_fill = lv_obj_create(s_stage);
    ban_style_flat(s_bat_fill);
    lv_obj_set_pos(s_bat_fill, 216, 12);
    lv_obj_set_size(s_bat_fill, 0, 4);
    lv_obj_set_style_bg_color(s_bat_fill, ban_rgb(BAN_PAL_GREEN), 0);
    lv_obj_set_style_bg_opa(s_bat_fill, LV_OPA_COVER, 0);
    s_bat_nub = lv_obj_create(s_stage);
    ban_style_flat(s_bat_nub);
    lv_obj_set_pos(s_bat_nub, 230, 12);
    lv_obj_set_size(s_bat_nub, 2, 4);
    lv_obj_set_style_bg_color(s_bat_nub, ban_rgb(BAN_PAL_BELLY), 0);
    lv_obj_set_style_bg_opa(s_bat_nub, LV_OPA_COVER, 0);

    s_content = lv_obj_create(s_stage);
    ban_style_flat(s_content);
    lv_obj_set_pos(s_content, 0, BAN_METER_H);
    lv_obj_set_size(s_content, BAN_SCR_W, BAN_PAGE_H);
    lv_obj_set_style_bg_opa(s_content, LV_OPA_TRANSP, 0);
    lv_obj_remove_flag(s_content, LV_OBJ_FLAG_CLICKABLE);

    s_flash = lv_obj_create(screen);
    ban_style_flat(s_flash);
    lv_obj_set_size(s_flash, BAN_SCR_W, BAN_SCR_H);
    lv_obj_set_pos(s_flash, 0, 0);
    lv_obj_set_style_bg_color(s_flash, ban_rgb(BAN_PAL_RED), 0);
    lv_obj_set_style_bg_opa(s_flash, LV_OPA_COVER, 0);
    lv_obj_remove_flag(s_flash, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(s_flash, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(s_flash);
}

void ban_ui_start(void) {
    ban_game_reset();
    s_view_key = 0;
    s_shown_risk = 0;
    s_rounds = 0;
    s_ops = NULL;
    s_page = NULL;
    s_shock_until = 0;
    s_shock_phase = BAN_PHASE_HOME;
    s_watch_on = 0;
    build_shell();
    open_page();
    s_last_tick = lv_tick_get();
    s_timer = lv_timer_create(on_frame, 20, NULL);
}
