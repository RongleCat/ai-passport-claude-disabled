// main/ban_ui_home.c —— 首页。96 角色完整露出头、身体和白肚皮。
// 160 的不透明身高约 146，标题卡 + 按钮 + 三行声明约 144，256 的内容区排不下。
#include "ban_ui_internal.h"

#include <stdio.h>

static lv_obj_t *s_cursor;
static lv_obj_t *s_dario;

#define HOME_X 72
#define HOME_Y 0
#define HOME_CARD_Y 104
#define HOME_CARD_H 52
#define HOME_BTN_Y 166
#define HOME_NOTE_Y 208
/* 边框各 2px，创建时量内容宽会得到 0，字被裁掉。 */
#define HOME_INNER_W 220

void ban_view_home_shutdown(void) {
    s_cursor = NULL;
    s_dario = NULL;
}

void ban_view_home_create(lv_obj_t *page) {
    lv_obj_t *card;
    lv_obj_t *title;
    lv_obj_t *sub1;
    lv_obj_t *sub2;
    lv_obj_t *start;
    lv_obj_t *note;
    lv_obj_t *btn;
    char disc[64];

    s_dario = ban_dario_new(page, HOME_X, HOME_Y, BAN_SPRITE_96);

    card = ban_box(page, 8, HOME_CARD_Y, 224, HOME_CARD_H, BAN_PAL_CREAM, BAN_PAL_OUTLINE);
    title = ban_label(card, &ban_font_zh_16, BAN_PAL_INK, ban_line(BAN_COPY_HOME_TITLE, 1));
    lv_obj_set_width(title, HOME_INNER_W);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(title, 0, 0);
    sub1 = ban_label(card, &ban_font_zh_12, BAN_PAL_INK, ban_line(BAN_COPY_HOME_SUB, 1));
    lv_obj_set_width(sub1, HOME_INNER_W);
    lv_obj_set_style_text_align(sub1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(sub1, 0, 18);
    sub2 = ban_label(card, &ban_font_zh_12, BAN_PAL_BLUE, ban_line(BAN_COPY_HOME_SUB, 2));
    lv_obj_set_width(sub2, HOME_INNER_W);
    lv_obj_set_style_text_align(sub2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(sub2, 0, 32);

    btn = ban_box(page, 8, HOME_BTN_Y, 224, 32, BAN_PAL_ORANGE, BAN_PAL_OUTLINE);
    s_cursor = ban_cursor_new(btn);
    lv_obj_set_pos(s_cursor, 12, 9);
    start = ban_label(btn, &ban_font_zh_16, BAN_PAL_CREAM, ban_line(BAN_COPY_HOME_START, 0));
    lv_obj_align(start, LV_ALIGN_CENTER, 8, 0);

    snprintf(disc, sizeof disc, "%s\n%s\n%s",
             ban_line(BAN_COPY_DISCLAIMER, 0),
             ban_line(BAN_COPY_DISCLAIMER, 1),
             ban_line(BAN_COPY_DISCLAIMER, 2));
    note = ban_label(page, &ban_font_zh_12, BAN_PAL_BELLY, disc);
    lv_obj_set_width(note, 224);
    lv_obj_set_pos(note, 8, HOME_NOTE_Y);
    lv_obj_set_style_text_align(note, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(note, 0, 0);
}

void ban_view_home_update(lv_obj_t *page) {
    (void)page;
    ban_dario_sync(s_dario, HOME_X, HOME_Y, BAN_SPRITE_96);
    ban_cursor_nudge(s_cursor, 12, 9);
}
