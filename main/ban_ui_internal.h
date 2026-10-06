// main/ban_ui_internal.h —— 页面文件共用的样式和控件。不给 main.c 用。
#pragma once

#include "ban_assets.h"
#include "ban_fonts.h"
#include "ban_logic.h"
#include "ban_palette.h"

#include "lvgl.h"

#define BAN_SCR_W 240
#define BAN_SCR_H 320
#define BAN_SAFE 8
#define BAN_METER_H 64
#define BAN_PAGE_H (BAN_SCR_H - BAN_METER_H)
#define BAN_RADIUS 10

#define BAN_SPRITE_160 160
#define BAN_SPRITE_96 96
#define BAN_SPRITE_48 48

lv_color_t ban_rgb(uint32_t hex);
void ban_style_flat(lv_obj_t *obj);
const char *ban_line(ban_copy_id_t id, int which);
void ban_utf8_n(char *dst, unsigned dst_n, const char *src, int codepoints);

lv_obj_t *ban_label(lv_obj_t *parent, const lv_font_t *font, uint32_t hex, const char *text);
void ban_label_mid(lv_obj_t *label, int y);
lv_obj_t *ban_box(lv_obj_t *parent, int x, int y, int w, int h, uint32_t bg, uint32_t edge);

/* 选中标记：右指三角，左右轻跳。父级是按钮。 */
lv_obj_t *ban_cursor_new(lv_obj_t *parent);
void ban_cursor_nudge(lv_obj_t *cursor, int x, int y);

/* size 是 160、96 或 48，对应已经打包的那一档，运行时不再缩放。 */
lv_obj_t *ban_dario_new(lv_obj_t *parent, int x, int y, int size);
void ban_dario_sync(lv_obj_t *img, int x, int y, int size);
int ban_ui_bob(void);
void ban_ui_shake(uint32_t ms);
void ban_scanlines(lv_obj_t *page);
void ban_type_set(lv_obj_t *label, const char *src, uint32_t elapsed_ms, uint32_t ms_per_cp);

void ban_view_home_create(lv_obj_t *page);
void ban_view_home_update(lv_obj_t *page);
void ban_view_home_shutdown(void);

void ban_view_scan_create(lv_obj_t *page);
void ban_view_scan_update(lv_obj_t *page);
void ban_view_scan_shutdown(void);

void ban_view_choice_create(lv_obj_t *page);
void ban_view_choice_update(lv_obj_t *page);
void ban_view_choice_shutdown(void);

void ban_view_auto_create(lv_obj_t *page);
void ban_view_auto_update(lv_obj_t *page);
void ban_view_auto_shutdown(void);

void ban_view_result_create(lv_obj_t *page);
void ban_view_result_update(lv_obj_t *page);
void ban_view_result_shutdown(void);

void ban_view_final_create(lv_obj_t *page);
void ban_view_final_update(lv_obj_t *page);
void ban_view_final_shutdown(void);

void ban_view_stamp_create(lv_obj_t *page);
void ban_view_stamp_update(lv_obj_t *page);
void ban_view_stamp_shutdown(void);

void ban_view_banned_create(lv_obj_t *page);
void ban_view_banned_update(lv_obj_t *page);
void ban_view_banned_shutdown(void);

void ban_view_appeal_create(lv_obj_t *page);
void ban_view_appeal_update(lv_obj_t *page);
void ban_view_appeal_shutdown(void);

void ban_view_appeal_done_create(lv_obj_t *page);
void ban_view_appeal_done_update(lv_obj_t *page);
void ban_view_appeal_done_shutdown(void);

void ban_view_over_create(lv_obj_t *page);
void ban_view_over_update(lv_obj_t *page);
void ban_view_over_shutdown(void);
