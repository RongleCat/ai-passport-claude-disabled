// main/ban_ui_appeal.c —— 申诉老虎机。停在最后一格，逻辑保证那一格是「拒绝」。
#include "ban_ui_internal.h"

#define APPEAL_ROW 32
#define APPEAL_SLOTS (BAN_APPEAL_LEN + 2)

static lv_obj_t *s_col;
static lv_obj_t *s_word[APPEAL_SLOTS];
static lv_obj_t *s_dario;
static int s_shown = -1;

static const char *appeal_text(int index) {
    int n = BAN_APPEAL_LEN;
    int i = index;

    while (i < 0) i += n;
    while (i >= n) i -= n;
    return ban_game_appeal_at(i) == BAN_APPEAL_RESTORE
               ? ban_line(BAN_COPY_RESTORE, 0)
               : ban_line(BAN_COPY_REJECT, 0);
}

static int column_y(int cursor) {
    return -cursor * APPEAL_ROW;
}

static void col_y(void *obj, int32_t value) {
    lv_obj_set_y((lv_obj_t *)obj, value);
}

void ban_view_appeal_shutdown(void) {
    int i;

    if (s_col) lv_anim_delete(s_col, NULL);
    s_col = NULL;
    s_dario = NULL;
    s_shown = -1;
    for (i = 0; i < APPEAL_SLOTS; i++) s_word[i] = NULL;
}

void ban_view_appeal_create(lv_obj_t *page) {
    lv_obj_t *title;
    lv_obj_t *window;
    lv_obj_t *band;
    int i;
    int cursor = ban_game_appeal_cursor();

    title = ban_label(page, &ban_font_zh_16, BAN_PAL_YELLOW, ban_line(BAN_COPY_APPEAL, 0));
    ban_label_mid(title, 4);

    window = ban_box(page, 28, 30, 184, APPEAL_ROW * 3, BAN_PAL_INK, BAN_PAL_YELLOW);
    lv_obj_set_style_clip_corner(window, true, 0);

    band = lv_obj_create(window);
    ban_style_flat(band);
    lv_obj_set_pos(band, 4, APPEAL_ROW + 2);
    lv_obj_set_size(band, 176, APPEAL_ROW - 4);
    lv_obj_set_style_radius(band, 8, 0);
    lv_obj_set_style_bg_color(band, ban_rgb(BAN_PAL_RED), 0);
    lv_obj_set_style_bg_opa(band, LV_OPA_COVER, 0);
    lv_obj_remove_flag(band, LV_OBJ_FLAG_CLICKABLE);

    s_col = lv_obj_create(window);
    ban_style_flat(s_col);
    lv_obj_set_size(s_col, 184, APPEAL_ROW * APPEAL_SLOTS);
    lv_obj_set_style_bg_opa(s_col, LV_OPA_TRANSP, 0);
    lv_obj_set_y(s_col, column_y(cursor));
    lv_obj_remove_flag(s_col, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_move_foreground(s_col);

    for (i = -1; i <= BAN_APPEAL_LEN; i++) {
        int slot = i + 1;

        s_word[slot] = ban_label(s_col, &ban_font_zh_16, BAN_PAL_YELLOW, appeal_text(i));
        lv_obj_set_width(s_word[slot], 184);
        lv_obj_set_style_text_align(s_word[slot], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_pos(s_word[slot], 0, slot * APPEAL_ROW + 6);
    }
    s_shown = cursor;
    s_dario = ban_dario_new(page, 72, 146, BAN_SPRITE_96);
}

void ban_view_appeal_update(lv_obj_t *page) {
    int cursor = ban_game_appeal_cursor();
    int target;
    int i;

    (void)page;
    if (!s_col) return;
    if (cursor != s_shown) {
        lv_anim_t anim;

        target = column_y(cursor);
        lv_anim_delete(s_col, NULL);
        lv_anim_init(&anim);
        lv_anim_set_var(&anim, s_col);
        lv_anim_set_exec_cb(&anim, col_y);
        lv_anim_set_values(&anim, lv_obj_get_y(s_col), target);
        lv_anim_set_duration(&anim, 90);
        lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
        lv_anim_start(&anim);
        s_shown = cursor;
    }
    for (i = 0; i < APPEAL_SLOTS; i++) {
        uint32_t hex = (i - 1 == cursor) ? BAN_PAL_CREAM : BAN_PAL_YELLOW;
        if (!s_word[i]) continue;
        lv_obj_set_style_text_color(s_word[i], ban_rgb(hex), 0);
    }
    ban_dario_sync(s_dario, 72, 146, BAN_SPRITE_96);
}

void ban_view_appeal_done_shutdown(void) {
    s_dario = NULL;
}

void ban_view_appeal_done_create(lv_obj_t *page) {
    lv_obj_t *title;
    lv_obj_t *word;
    lv_obj_t *l1;
    lv_obj_t *l2;

    title = ban_label(page, &ban_font_zh_24, BAN_PAL_RED, ban_line(BAN_COPY_APPEAL_FAIL, 0));
    ban_label_mid(title, 8);
    word = ban_label(page, &ban_font_zh_24, BAN_PAL_YELLOW, ban_line(BAN_COPY_REJECT, 0));
    ban_label_mid(word, 44);
    l1 = ban_label(page, &ban_font_zh_16, BAN_PAL_CREAM, ban_line(BAN_COPY_APPEAL_FAIL, 1));
    ban_label_mid(l1, 80);
    l2 = ban_label(page, &ban_font_zh_16, BAN_PAL_CREAM, ban_line(BAN_COPY_APPEAL_FAIL, 2));
    ban_label_mid(l2, 104);
    s_dario = ban_dario_new(page, 72, 148, BAN_SPRITE_96);
}

void ban_view_appeal_done_update(lv_obj_t *page) {
    (void)page;
    ban_dario_sync(s_dario, 72, 148, BAN_SPRITE_96);
}
