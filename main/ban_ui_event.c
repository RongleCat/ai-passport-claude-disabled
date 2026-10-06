// main/ban_ui_event.c —— 二选一、自动检测、结果共用一个版面骨架。
// 上：64 图标 + 标题。中：正文或按钮。下：96 达里奥。
#include "ban_ui_internal.h"

#include <stdio.h>
#include <string.h>

#define EV_ICON_X 8
#define EV_ICON_Y 2
#define EV_TITLE_X 80
#define EV_TITLE_Y 6
#define EV_EXTRA_Y 36
#define EV_DARIO_X 72
#define EV_DARIO_Y 158
#define EV_BTN_X 8
#define EV_BTN_W 224
#define EV_BTN_H 32
#define EV_BTN_Y0 86
#define EV_BTN_Y1 122

static lv_obj_t *s_choice_box[2];
static lv_obj_t *s_choice_text[2];
static lv_obj_t *s_choice_cursor[2];
static lv_obj_t *s_dario;

static lv_obj_t *s_kicker;
static lv_obj_t *s_value;
static lv_obj_t *s_icon;
static lv_obj_t *s_flag;
static lv_obj_t *s_cap1;
static lv_obj_t *s_cap2;
static lv_obj_t *s_digit_box;
static lv_obj_t *s_digit;
static int s_digit_n;
static ban_event_id_t s_auto_id;

static lv_obj_t *s_res_title;
static lv_obj_t *s_res_l1;
static lv_obj_t *s_res_l2;
static lv_obj_t *s_delta;

typedef struct {
    lv_obj_t *icon;
    lv_obj_t *title;
    lv_obj_t *extra;
} ev_head_t;

static const lv_image_dsc_t *auto_icon(int step) {
    if (step <= 0) return &ban_img_icon_language_64;
    if (step == 1) return &ban_img_icon_timezone_64;
    return &ban_img_icon_wechat_64;
}

static const lv_image_dsc_t *flag_for(int step) {
    if (step <= 0) return &ban_img_flag_us_72;
    if (step == 1) return &ban_img_flag_jp_72;
    return &ban_img_flag_sg_72;
}

static ban_copy_id_t china_copy(int step) {
    if (step <= 0) return BAN_COPY_CHINA_LANG;
    if (step == 1) return BAN_COPY_CHINA_TZ;
    return BAN_COPY_CHINA_APP;
}

static const lv_image_dsc_t *choice_icon(ban_event_id_t id) {
    if (id == BAN_EVENT_EMAIL) return &ban_img_icon_mail_64;
    if (id == BAN_EVENT_GIFT) return &ban_img_icon_gift_64;
    return &ban_img_icon_warning_64;
}

static ev_head_t make_head(lv_obj_t *page, const lv_image_dsc_t *icon, const char *title,
                           const char *extra, uint32_t extra_hex) {
    ev_head_t head;

    head.icon = lv_image_create(page);
    lv_image_set_src(head.icon, icon);
    lv_obj_set_pos(head.icon, EV_ICON_X, EV_ICON_Y);
    lv_obj_remove_flag(head.icon, LV_OBJ_FLAG_CLICKABLE);
    head.title = ban_label(page, &ban_font_zh_24, BAN_PAL_CREAM, title ? title : "");
    lv_obj_set_pos(head.title, EV_TITLE_X, EV_TITLE_Y);
    lv_obj_set_width(head.title, BAN_SCR_W - EV_TITLE_X - BAN_SAFE);
    head.extra = NULL;
    if (extra && extra[0]) {
        head.extra = ban_label(page, &ban_font_zh_16, extra_hex, extra);
        lv_obj_set_pos(head.extra, EV_TITLE_X, EV_EXTRA_Y);
        lv_obj_set_width(head.extra, BAN_SCR_W - EV_TITLE_X - BAN_SAFE);
    }
    return head;
}

static void set_choice_style(int index, int on) {
    uint32_t bg = on ? BAN_PAL_ORANGE : BAN_PAL_CREAM;
    uint32_t fg = on ? BAN_PAL_CREAM : BAN_PAL_INK;

    lv_obj_set_style_bg_color(s_choice_box[index], ban_rgb(bg), 0);
    lv_obj_set_style_text_color(s_choice_text[index], ban_rgb(fg), 0);
    lv_obj_set_style_border_color(s_choice_box[index], ban_rgb(BAN_PAL_OUTLINE), 0);
    lv_obj_set_style_border_width(s_choice_box[index], on ? 3 : 2, 0);
    if (on) {
        lv_obj_remove_flag(s_choice_cursor[index], LV_OBJ_FLAG_HIDDEN);
        ban_cursor_nudge(s_choice_cursor[index], 10, 9);
    } else {
        lv_obj_add_flag(s_choice_cursor[index], LV_OBJ_FLAG_HIDDEN);
    }
}

void ban_view_choice_shutdown(void) {
    s_choice_box[0] = s_choice_box[1] = NULL;
    s_choice_text[0] = s_choice_text[1] = NULL;
    s_choice_cursor[0] = s_choice_cursor[1] = NULL;
    s_dario = NULL;
}

void ban_view_choice_create(lv_obj_t *page) {
    const ban_event_def_t *def = ban_event_def(ban_game_event());
    lv_obj_t *prompt;
    int i;

    make_head(page, choice_icon(def->id), ban_line((ban_copy_id_t)def->title, 0),
              ban_line((ban_copy_id_t)def->title, 1), BAN_PAL_YELLOW);

    prompt = ban_label(page, &ban_font_zh_16, BAN_PAL_BELLY, ban_line((ban_copy_id_t)def->prompt, 1));
    ban_label_mid(prompt, 58);

    for (i = 0; i < 2; i++) {
        int y = (i == 0) ? EV_BTN_Y0 : EV_BTN_Y1;
        ban_copy_id_t id = (i == 0) ? (ban_copy_id_t)def->choice_a : (ban_copy_id_t)def->choice_b;
        lv_obj_t *mark;

        s_choice_box[i] = ban_box(page, EV_BTN_X, y, EV_BTN_W, EV_BTN_H, BAN_PAL_CREAM, BAN_PAL_OUTLINE);
        mark = ban_cursor_new(s_choice_box[i]);
        lv_obj_set_pos(mark, 10, 9);
        s_choice_cursor[i] = mark;
        s_choice_text[i] = ban_label(s_choice_box[i], &ban_font_zh_16, BAN_PAL_INK, ban_line(id, 0));
        lv_obj_align(s_choice_text[i], LV_ALIGN_CENTER, 8, 0);
    }
    s_dario = ban_dario_new(page, EV_DARIO_X, EV_DARIO_Y, BAN_SPRITE_96);
}

void ban_view_choice_update(lv_obj_t *page) {
    int selected = ban_game_selected();

    (void)page;
    set_choice_style(0, selected == 0);
    set_choice_style(1, selected == 1);
    ban_dario_sync(s_dario, EV_DARIO_X, EV_DARIO_Y, BAN_SPRITE_96);
}

static void box_resize(void *obj, int32_t value) {
    lv_obj_set_size((lv_obj_t *)obj, value, value);
    lv_obj_set_x((lv_obj_t *)obj, (BAN_SCR_W - value) / 2);
}

static void pop_digit(void) {
    lv_anim_t anim;

    if (!s_digit_box) return;
    lv_anim_delete(s_digit_box, NULL);
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, s_digit_box);
    lv_anim_set_exec_cb(&anim, box_resize);
    lv_anim_set_values(&anim, 84, 68);
    lv_anim_set_duration(&anim, 280);
    lv_anim_set_path_cb(&anim, lv_anim_path_overshoot);
    lv_anim_start(&anim);
}

void ban_view_auto_shutdown(void) {
    if (s_digit_box) lv_anim_delete(s_digit_box, NULL);
    s_kicker = NULL;
    s_value = NULL;
    s_icon = NULL;
    s_flag = NULL;
    s_cap1 = NULL;
    s_cap2 = NULL;
    s_digit_box = NULL;
    s_digit = NULL;
    s_dario = NULL;
    s_digit_n = -1;
}

void ban_view_auto_create(lv_obj_t *page) {
    const ban_event_def_t *def = ban_event_def(ban_game_event());
    ev_head_t head;

    s_auto_id = def->id;
    s_digit_n = -1;
    if (def->id == BAN_EVENT_CHINA_SCAN) {
        head = make_head(page, auto_icon(0), ban_line((ban_copy_id_t)def->title, 0),
                         ban_line(BAN_COPY_CHINA_LANG, 0), BAN_PAL_BLUE);
        s_icon = head.icon;
        s_kicker = head.extra;
        s_value = ban_label(page, &ban_font_zh_24, BAN_PAL_CREAM, ban_line(BAN_COPY_CHINA_LANG, 1));
        ban_label_mid(s_value, 96);
    } else if (def->id == BAN_EVENT_VPN) {
        make_head(page, &ban_img_icon_warning_64, ban_line((ban_copy_id_t)def->title, 0), "", 0);
        s_flag = lv_image_create(page);
        lv_image_set_src(s_flag, &ban_img_flag_us_72);
        lv_obj_set_pos(s_flag, (BAN_SCR_W - 72) / 2, 68);
        lv_obj_remove_flag(s_flag, LV_OBJ_FLAG_CLICKABLE);
        s_cap1 = ban_label(page, &ban_font_zh_24, BAN_PAL_RED, ban_line(BAN_COPY_VPN_BODY, 0));
        ban_label_mid(s_cap1, 118);
        s_cap2 = ban_label(page, &ban_font_zh_16, BAN_PAL_CREAM, ban_line(BAN_COPY_VPN_BODY, 1));
        ban_label_mid(s_cap2, 140);
        lv_obj_add_flag(s_cap1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_cap2, LV_OBJ_FLAG_HIDDEN);
    } else {
        make_head(page, &ban_img_icon_warning_64, ban_line(BAN_COPY_IDLE_TITLE, 0),
                  ban_line(BAN_COPY_IDLE_TITLE, 1), BAN_PAL_YELLOW);
        s_digit_box = ban_box(page, 86, 76, 68, 68, BAN_PAL_CREAM, BAN_PAL_RED);
        s_digit = ban_label(s_digit_box, &ban_font_zh_24, BAN_PAL_RED, "");
        lv_obj_align(s_digit, LV_ALIGN_CENTER, 0, 0);
    }
    s_dario = ban_dario_new(page, EV_DARIO_X, EV_DARIO_Y, BAN_SPRITE_96);
}

void ban_view_auto_update(lv_obj_t *page) {
    (void)page;
    if (s_auto_id == BAN_EVENT_CHINA_SCAN && s_icon) {
        int step = ban_game_auto_step();
        ban_copy_id_t id = china_copy(step);
        const char *kick = ban_line(id, 0);
        const char *val = ban_line(id, 1);

        if (lv_image_get_src(s_icon) != auto_icon(step)) lv_image_set_src(s_icon, auto_icon(step));
        if (s_kicker && strcmp(lv_label_get_text(s_kicker), kick) != 0) lv_label_set_text(s_kicker, kick);
        if (s_value && strcmp(lv_label_get_text(s_value), val) != 0) lv_label_set_text(s_value, val);
    } else if (s_auto_id == BAN_EVENT_VPN && s_flag) {
        int step = ban_game_auto_step();
        if (lv_image_get_src(s_flag) != flag_for(step)) lv_image_set_src(s_flag, flag_for(step));
        if (step >= 2) {
            lv_obj_remove_flag(s_cap1, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(s_cap2, LV_OBJ_FLAG_HIDDEN);
        }
    } else if (s_digit_box) {
        int n = ban_game_countdown();
        char buf[16];

        if (n != s_digit_n) {
            s_digit_n = n;
            if (n > 0) {
                snprintf(buf, sizeof buf, "%d", n);
                lv_label_set_text(s_digit, buf);
                pop_digit();
            }
        }
        lv_obj_align(s_digit, LV_ALIGN_CENTER, 0, 0);
    }
    ban_dario_sync(s_dario, EV_DARIO_X, EV_DARIO_Y, BAN_SPRITE_96);
}

static int delta_bounce(uint32_t elapsed) {
    uint32_t t = elapsed % 700u;

    if (t < 70u) return -8;
    if (t < 140u) return -2;
    if (t < 210u) return -6;
    return 0;
}

static void risk_line(char *dst, unsigned dst_n, const char *src) {
    if (src && strstr(src, "风险")) {
        snprintf(dst, dst_n, "风险 +%d", ban_game_last_delta());
        return;
    }
    snprintf(dst, dst_n, "%s", src ? src : "");
}

void ban_view_result_shutdown(void) {
    s_res_title = NULL;
    s_res_l1 = NULL;
    s_res_l2 = NULL;
    s_delta = NULL;
    s_dario = NULL;
}

void ban_view_result_create(lv_obj_t *page) {
    ban_copy_id_t id = ban_game_result_copy();
    const char *l1 = ban_line(id, 1);
    const char *l2 = ban_line(id, 2);
    ev_head_t head;

    head = make_head(page, &ban_img_icon_warning_64, "", "", 0);
    s_res_title = head.title;
    lv_obj_set_style_text_color(s_res_title, ban_rgb(BAN_PAL_RED), 0);
    s_res_l1 = NULL;
    s_res_l2 = NULL;
    if (l1[0]) {
        s_res_l1 = ban_label(page, &ban_font_zh_16, BAN_PAL_CREAM, "");
        ban_label_mid(s_res_l1, 72);
    }
    if (l2[0]) {
        s_res_l2 = ban_label(page, &ban_font_zh_16, BAN_PAL_YELLOW, "");
        ban_label_mid(s_res_l2, 94);
    }
    s_delta = ban_label(page, &ban_font_zh_24, BAN_PAL_RED, "");
    ban_label_mid(s_delta, 120);
    s_dario = ban_dario_new(page, EV_DARIO_X, EV_DARIO_Y, BAN_SPRITE_96);
}

void ban_view_result_update(lv_obj_t *page) {
    ban_copy_id_t id = ban_game_result_copy();
    uint32_t elapsed = ban_game_elapsed();
    char line[32];
    char big[16];
    int y;

    (void)page;
    ban_type_set(s_res_title, ban_line(id, 0), elapsed, 40);
    if (s_res_l1) {
        risk_line(line, sizeof line, ban_line(id, 1));
        /* 数字行打得慢会在滑入结束时停在「+4」。16ms 一格，落定前写完。 */
        ban_type_set(s_res_l1, line, elapsed, 16);
    }
    if (s_res_l2) {
        risk_line(line, sizeof line, ban_line(id, 2));
        ban_type_set(s_res_l2, line, elapsed, 16);
    }
    snprintf(big, sizeof big, "+%d", ban_game_last_delta());
    if (s_delta && strcmp(lv_label_get_text(s_delta), big) != 0) lv_label_set_text(s_delta, big);
    y = 120 + delta_bounce(elapsed);
    if (s_delta && lv_obj_get_y(s_delta) != y) lv_obj_set_y(s_delta, y);
    ban_dario_sync(s_dario, EV_DARIO_X, EV_DARIO_Y, BAN_SPRITE_96);
}
