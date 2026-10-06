// 无头 240x320 预览。固定 RNG 跑三局，把关键帧写成 PNG。
#include "ban_logic.h"
#include "ban_port.h"
#include "ban_ui.h"
#include "png_write.h"

#include "lvgl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define SCR_W 240
#define SCR_H 320
#define STEP_MS 40
#define MAX_SHOTS 48

enum { KEY_UP = 0, KEY_DOWN = 1, KEY_OK = 2 };

/* prepare_round 的调用顺序：洗牌 4 次、概率 1 次、申诉 7 次。
 * 第 1、3 局是赠送 / 邮件 / 中国人。第 2 局是闲置 / VPN / 赠送。概率 97。 */
static const uint32_t FIXTURE[] = {
    2576980378u, 3221225472u, 2863311531u, 2147483648u, 3616814566u,
    2u, 1u, 2u, 1u, 1u, 2u, 1u,
    858993460u, 2147483648u, 0u, 0u, 3616814566u,
    2u, 1u, 2u, 1u, 1u, 2u, 1u,
    2576980378u, 3221225472u, 2863311531u, 2147483648u, 3616814566u,
    2u, 1u, 2u, 1u, 1u, 2u, 1u,
};
static int s_rng_i;

static uint32_t fixture_rng(void) {
    if (s_rng_i >= (int)(sizeof FIXTURE / sizeof FIXTURE[0])) return 1u;
    return FIXTURE[s_rng_i++];
}

static uint8_t s_fb[SCR_W * SCR_H * 2];
static char s_out_dir[512];

static int s_shot_n;
static char s_shot_name[MAX_SHOTS][64];
static uint8_t *s_shot_rgb[MAX_SHOTS];

static int s_round;
static int s_stage;
static ban_phase_t s_stage_phase;
static int s_stage_index = -1;

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px) {
    (void)area;
    (void)px;
    lv_display_flush_ready(disp);
}

static void rgb_from_fb(uint8_t *rgb) {
    int y;
    int x;

    for (y = 0; y < SCR_H; y++) {
        const uint8_t *row = s_fb + (size_t)y * SCR_W * 2u;
        for (x = 0; x < SCR_W; x++) {
            uint16_t c = (uint16_t)row[x * 2] | ((uint16_t)row[x * 2 + 1] << 8);
            uint8_t r = (uint8_t)((c >> 11) & 31u);
            uint8_t g = (uint8_t)((c >> 5) & 63u);
            uint8_t b = (uint8_t)(c & 31u);
            uint8_t *dst = rgb + ((size_t)y * SCR_W + (size_t)x) * 3u;
            dst[0] = (uint8_t)((r << 3) | (r >> 2));
            dst[1] = (uint8_t)((g << 2) | (g >> 4));
            dst[2] = (uint8_t)((b << 3) | (b >> 2));
        }
    }
}

static int have(const char *name) {
    int i;
    for (i = 0; i < s_shot_n; i++) {
        if (strcmp(s_shot_name[i], name) == 0) return 1;
    }
    return 0;
}

static void shoot(const char *name) {
    char path[640];
    uint8_t *rgb;

    if (have(name) || s_shot_n >= MAX_SHOTS) return;
    rgb = (uint8_t *)malloc((size_t)SCR_W * SCR_H * 3u);
    if (!rgb) return;
    rgb_from_fb(rgb);
    snprintf(path, sizeof path, "%s/%s.png", s_out_dir, name);
    if (png_write_rgb(path, rgb, SCR_W, SCR_H) != 0) {
        fprintf(stderr, "png failed: %s\n", path);
        free(rgb);
        return;
    }
    snprintf(s_shot_name[s_shot_n], sizeof s_shot_name[s_shot_n], "%s", name);
    s_shot_rgb[s_shot_n] = rgb;
    s_shot_n++;
    printf("shot %s\n", name);
    fflush(stdout);
}

static void note_visit(void) {
    ban_phase_t phase = ban_game_phase();
    int index = ban_game_event_index();

    if (phase != s_stage_phase || index != s_stage_index) {
        s_stage_phase = phase;
        s_stage_index = index;
        s_stage = 0;
    }
}

static void feed_choice(void) {
    ban_event_id_t id = ban_game_event();

    if (s_round == 1 && id == BAN_EVENT_GIFT) {
        if (s_stage == 0 && have("04_gift")) {
            ban_port_post_key(KEY_DOWN);
            s_stage = 1;
        } else if (s_stage == 1 && have("05_gift_down")) {
            ban_port_post_key(KEY_UP);
            s_stage = 2;
        } else if (s_stage == 2 && have("06_gift_up")) {
            ban_port_post_key(KEY_OK);
            s_stage = 3;
        }
        return;
    }
    if (s_round == 1 && id == BAN_EVENT_EMAIL) {
        if (s_stage == 0 && have("09_email")) {
            ban_port_post_key(KEY_OK);
            s_stage = 1;
        }
        return;
    }
    if (s_round == 2 && id == BAN_EVENT_GIFT) {
        if (s_stage == 0) {
            ban_port_post_key(KEY_DOWN);
            s_stage = 1;
        } else if (s_stage == 1) {
            ban_port_post_key(KEY_OK);
            s_stage = 2;
        }
        return;
    }
    if (s_round == 3 && id == BAN_EVENT_GIFT) {
        if (s_stage == 0) {
            ban_port_post_key(KEY_OK);
            s_stage = 1;
        }
        return;
    }
    if (s_round == 3 && id == BAN_EVENT_EMAIL) {
        if (s_stage == 0) {
            ban_port_post_key(KEY_DOWN);
            s_stage = 1;
        } else if (s_stage == 1) {
            ban_port_post_key(KEY_OK);
            s_stage = 2;
        }
    }
}

static void script_input(void) {
    ban_phase_t phase;

    if (!ban_ui_settled()) return;
    note_visit();
    phase = ban_game_phase();
    if (phase == BAN_PHASE_HOME && have("01_home") && have("01b_home_blink") && s_stage == 0) {
        ban_port_post_key(KEY_OK);
        s_stage = 1;
        return;
    }
    if (phase == BAN_PHASE_EVENT &&
        ban_event_def(ban_game_event())->type == BAN_EVENT_CHOICE) {
        feed_choice();
        return;
    }
    if (phase == BAN_PHASE_BAN && s_stage == 0) {
        if (s_round == 1 && !have("23_ban")) return;
        ban_port_post_key(KEY_OK);
        s_stage = 1;
        return;
    }
    if (phase == BAN_PHASE_GAME_OVER && s_stage == 0 && s_round < 3) {
        if (s_round == 1 && !have("30b_over_bits")) return;
        ban_port_post_key(KEY_OK);
        s_stage = 1;
    }
}

static int in_window(uint32_t elapsed, uint32_t lo, uint32_t hi) {
    return elapsed >= lo && elapsed < hi;
}

static void consider(void) {
    ban_phase_t phase = ban_game_phase();
    uint32_t elapsed = ban_game_elapsed();
    ban_event_id_t event = ban_game_event();
    int index = ban_game_event_index();

    if (s_round == 0 && phase == BAN_PHASE_HOME) {
        shoot("01_home");
        /* normal 脸每 4200ms 在末尾 120ms 切到 blink。 */
        if ((lv_tick_get() % 4200u) >= 4080u) shoot("01b_home_blink");
    }

    /* 深色页滑入的中段。180 ms 滑窗里大约第 3 帧。 */
    if (s_round == 1 && phase == BAN_PHASE_FINAL_REVIEW && !ban_ui_settled()) {
        static int wipe_frames;
        wipe_frames++;
        if (wipe_frames == 3) shoot("15w_final_wipe");
    }

    if (!ban_ui_settled() && phase != BAN_PHASE_BAN_ANIM) return;

    if (s_round == 1 && phase == BAN_PHASE_SCAN && in_window(elapsed, 360, 520)) shoot("02_scan");
    /* 红字闪的亮帧。打字在 2000+6*28 之后，2240 起 (elapsed/160) 为偶数。 */
    if (s_round == 1 && phase == BAN_PHASE_SCAN && ban_game_scan_alert() &&
        elapsed >= 2240u && ((elapsed / 160u) & 1u) == 0u) {
        shoot("03_scan_alert");
    }
    if (s_round == 1 && phase == BAN_PHASE_EVENT && event == BAN_EVENT_GIFT) {
        if (ban_game_selected() == 0 && !have("05_gift_down")) shoot("04_gift");
        if (ban_game_selected() == 1) shoot("05_gift_down");
        if (ban_game_selected() == 0 && have("05_gift_down")) shoot("06_gift_up");
    }
    if (s_round == 1 && phase == BAN_PHASE_EVENT_RESULT && index == 0 &&
        in_window(elapsed, 180, 280)) {
        shoot("07_gift_type");
    }
    if (s_round == 1 && phase == BAN_PHASE_EVENT_RESULT && index == 0 &&
        in_window(elapsed, 700, 980)) {
        shoot("08_gift_result");
    }
    if (s_round == 1 && phase == BAN_PHASE_EVENT && event == BAN_EVENT_EMAIL) shoot("09_email");
    if (s_round == 1 && phase == BAN_PHASE_EVENT_RESULT && index == 1 &&
        in_window(elapsed, 700, 980)) {
        shoot("10_email_result");
    }
    if (s_round == 1 && phase == BAN_PHASE_EVENT && event == BAN_EVENT_CHINA_SCAN) {
        int step = ban_game_auto_step();
        if (step == 0 && elapsed > 200) shoot("11_china_lang");
        if (step == 1) shoot("12_china_tz");
        if (step == 2) shoot("13_china_app");
    }
    if (s_round == 1 && phase == BAN_PHASE_EVENT_RESULT && index == 2 &&
        in_window(elapsed, 220, 420)) {
        shoot("14b_china_shock");
    }
    if (s_round == 1 && phase == BAN_PHASE_EVENT_RESULT && index == 2 &&
        in_window(elapsed, 700, 1100)) {
        shoot("14_china_result");
    }
    if (s_round == 1 && phase == BAN_PHASE_FINAL_REVIEW && in_window(elapsed, 200, 420)) {
        shoot("15_final_early");
    }
    if (s_round == 1 && phase == BAN_PHASE_FINAL_REVIEW && in_window(elapsed, 1400, 1750)) {
        shoot("16_final_late");
    }
    if (s_round == 1 && phase == BAN_PHASE_BAN_ANIM) {
        if (in_window(elapsed, 0, 90)) shoot("17_stamp_0");
        if (in_window(elapsed, 120, 210)) shoot("18_stamp_1");
        if (in_window(elapsed, 440, 520)) shoot("19_stamp_2");
        if (in_window(elapsed, 380, 470)) shoot("20_stamp_3");
        if (in_window(elapsed, 520, 620)) shoot("21_stamp_4");
        if (in_window(elapsed, 680, 800)) shoot("22_stamp_5");
    }
    /* 印章黄线亮着的那一拍，和封号页闪烁用同一公式。 */
    if (s_round == 1 && phase == BAN_PHASE_BAN && ((lv_tick_get() / 180u) & 1u) == 0u) {
        shoot("23_ban");
    }
    if (s_round == 1 && phase == BAN_PHASE_APPEAL) {
        int cursor = ban_game_appeal_cursor();
        /* 缓动可能一帧跨过某个下标，到达该档即拍，避免漏帧。 */
        if (cursor >= 0) shoot("24_appeal_0");
        if (cursor >= 2) shoot("25_appeal_2");
        if (cursor >= 4) shoot("26_appeal_4");
        if (cursor >= 6) shoot("27_appeal_6");
        /* 等滚轴动画落定再拍，避免停格还停在上一词。 */
        if (cursor >= 7 && elapsed >= 2900u) shoot("28_appeal_stop");
    }
    if (s_round == 1 && phase == BAN_PHASE_APPEAL_RESULT && elapsed > 200) shoot("29_appeal_fail");
    if (s_round == 1 && phase == BAN_PHASE_GAME_OVER) {
        /* 结束页不走 s_elapsed。40ms 一步，18 步约 720ms，中文章已经落定。 */
        static int over_frames;
        over_frames++;
        if (over_frames >= 18) shoot("30_over");
        if (over_frames >= 28) shoot("30b_over_bits");
    }

    if (s_round == 2 && phase == BAN_PHASE_EVENT && event == BAN_EVENT_IDLE) {
        int n = ban_game_countdown();
        if (n == 3 && in_window(elapsed, 1100, 1500)) shoot("31_idle_3");
        if (n == 2 && in_window(elapsed, 2100, 2500)) shoot("32_idle_2");
        if (n == 1 && in_window(elapsed, 3100, 3500)) shoot("33_idle_1");
    }
    if (s_round == 2 && phase == BAN_PHASE_EVENT_RESULT && index == 0 &&
        in_window(elapsed, 500, 900)) {
        shoot("34_idle_result");
    }
    if (s_round == 2 && phase == BAN_PHASE_EVENT && event == BAN_EVENT_VPN) {
        int step = ban_game_auto_step();
        if (step == 0 && elapsed > 180) shoot("35_vpn_us");
        if (step == 1) shoot("36_vpn_jp");
        if (step == 2 && elapsed > 2800) shoot("37_vpn_sg");
    }

    if (s_round == 3 && phase == BAN_PHASE_EVENT_RESULT && event == BAN_EVENT_CHINA_SCAN &&
        ban_game_risk() >= 90 && in_window(elapsed, 700, 1200)) {
        shoot("38_meter_watching");
    }
}

static int write_contact(void) {
    int w;
    int h = SCR_H;
    uint8_t *sheet;
    char path[640];
    int i;
    int y;

    if (s_shot_n <= 0) return -1;
    w = SCR_W * s_shot_n;
    sheet = (uint8_t *)malloc((size_t)w * (size_t)h * 3u);
    if (!sheet) return -1;
    for (i = 0; i < s_shot_n; i++) {
        for (y = 0; y < h; y++) {
            memcpy(sheet + ((size_t)y * (size_t)w + (size_t)i * SCR_W) * 3u,
                   s_shot_rgb[i] + (size_t)y * SCR_W * 3u,
                   SCR_W * 3u);
        }
    }
    snprintf(path, sizeof path, "%s/contact.png", s_out_dir);
    i = png_write_rgb(path, sheet, w, h);
    free(sheet);
    if (i == 0) printf("contact %s (%d frames)\n", path, s_shot_n);
    return i;
}

static const char *GALLERY[] = {
    "01_home",
    "02_scan",
    "04_gift",
    "08_gift_result",
    "11_china_lang",
    "14_china_result",
    "16_final_late",
    "19_stamp_2",
    "23_ban",
    "28_appeal_stop",
    "29_appeal_fail",
    "30_over",
};

static const uint8_t *find_shot(const char *name) {
    int i;

    for (i = 0; i < s_shot_n; i++) {
        if (strcmp(s_shot_name[i], name) == 0) return s_shot_rgb[i];
    }
    return NULL;
}

static int write_gallery(void) {
    enum { N = (int)(sizeof GALLERY / sizeof GALLERY[0]) };
    int w = SCR_W * N;
    int h = SCR_H;
    uint8_t *sheet;
    char path[640];
    int i;
    int y;

    sheet = (uint8_t *)malloc((size_t)w * (size_t)h * 3u);
    if (!sheet) return -1;
    for (i = 0; i < N; i++) {
        const uint8_t *rgb = find_shot(GALLERY[i]);

        if (!rgb) {
            fprintf(stderr, "gallery missing %s\n", GALLERY[i]);
            free(sheet);
            return -1;
        }
        for (y = 0; y < h; y++) {
            memcpy(sheet + ((size_t)y * (size_t)w + (size_t)i * SCR_W) * 3u,
                   rgb + (size_t)y * SCR_W * 3u,
                   SCR_W * 3u);
        }
    }
    snprintf(path, sizeof path, "%s/gallery.png", s_out_dir);
    i = png_write_rgb(path, sheet, w, h);
    free(sheet);
    if (i == 0) printf("gallery %s (%d frames)\n", path, N);
    return i;
}

/* 长局：游戏 RNG 与按键 RNG 分开。prepare_round 在结束页超时回首页时发生。 */
static uint32_t s_stress_state;
static int s_stress_mode;
static uint32_t s_input_state = 0x6D2B79F5u;

static uint32_t stress_rng(void) {
    uint32_t x = s_stress_state;

    if (s_stress_mode == 0) {
        x = x * 1664525u + 1013904223u;
        s_stress_state = x;
        return x;
    }
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    if (x == 0u) x = 0xA5A5u;
    s_stress_state = x;
    return x;
}

static uint32_t input_rng(void) {
    s_input_state = s_input_state * 1664525u + 1013904223u;
    return s_input_state;
}

static void arm_stress_rng(int game) {
    s_stress_mode = game & 1;
    if (s_stress_mode == 0) s_stress_state = 0xA5A5u ^ ((uint32_t)game * 0x9E3779B9u);
    else s_stress_state = 0xC0FFEEu ^ ((uint32_t)game * 0x9E3779B9u);
    if (s_stress_state == 0u) s_stress_state = 1u;
    ban_game_set_rng(stress_rng);
}

static void step_sim(uint32_t ms) {
    lv_tick_inc(ms);
    lv_timer_handler();
}

static uint32_t count_objs(const lv_obj_t *obj) {
    uint32_t n = 1;
    uint32_t i;
    uint32_t c;

    if (!obj) return 0;
    c = lv_obj_get_child_count(obj);
    for (i = 0; i < c; i++) n += count_objs(lv_obj_get_child(obj, (int32_t)i));
    return n;
}

static uint32_t count_timers(void) {
    uint32_t n = 0;
    lv_timer_t *timer = NULL;

    while ((timer = lv_timer_get_next(timer)) != NULL) n++;
    return n;
}

typedef struct {
    size_t used;
    size_t used_cnt;
    uint32_t objs;
    uint32_t anims;
    uint32_t timers;
} mem_snap_t;

static void dump_labels(lv_obj_t *obj) {
    uint32_t i;
    uint32_t n;

    if (!obj) return;
    if (lv_obj_check_type(obj, &lv_label_class)) {
        const char *text = lv_label_get_text(obj);
        fprintf(stderr, "  label %u [%s]\n", text ? (unsigned)strlen(text) : 0u, text ? text : "");
    }
    n = lv_obj_get_child_count(obj);
    for (i = 0; i < n; i++) dump_labels(lv_obj_get_child(obj, i));
}

static void take_snap(mem_snap_t *snap) {
    lv_mem_monitor_t mon;

    lv_mem_monitor(&mon);
    snap->used = mon.total_size - mon.free_size;
    snap->used_cnt = mon.used_cnt;
    snap->objs = count_objs(lv_screen_active()) + count_objs(lv_layer_top()) +
                 count_objs(lv_layer_sys());
    snap->anims = lv_anim_count_running();
    snap->timers = count_timers();
}

static int snap_same(const mem_snap_t *a, const mem_snap_t *b) {
    return a->used == b->used && a->used_cnt == b->used_cnt && a->objs == b->objs &&
           a->anims == b->anims && a->timers == b->timers;
}

static int phase_is_choice(void) {
    if (ban_game_phase() != BAN_PHASE_EVENT) return 0;
    return ban_event_def(ban_game_event())->type == BAN_EVENT_CHOICE;
}

static int wait_settled_home(int limit) {
    int n;

    for (n = 0; n < limit; n++) {
        if (ban_game_phase() == BAN_PHASE_HOME && ban_ui_settled() &&
            lv_anim_count_running() == 0) {
            return 0;
        }
        step_sim(80);
    }
    return -1;
}

static int play_one_game(int game) {
    int guard = 0;
    int started = 0;
    int choice_pause = 0;
    int ban_pause = 0;
    int burst = 0;

    while (ban_game_phase() != BAN_PHASE_GAME_OVER && guard < 6000) {
        ban_phase_t phase = ban_game_phase();

        if (ban_ui_settled()) {
            if (phase == BAN_PHASE_HOME && !started) {
                ban_port_post_key(KEY_OK);
                started = 1;
            } else if (phase == BAN_PHASE_SCAN && !burst && ban_game_elapsed() >= 400u &&
                       (game % 10) == 3) {
                int k;
                for (k = 0; k < 20; k++) ban_port_post_key((int)(input_rng() % 3u));
                burst = 1;
            } else if (phase_is_choice()) {
                uint32_t r = input_rng();
                if ((r % 6u) == 0u) ban_port_post_key(KEY_UP);
                else if ((r % 6u) == 1u) ban_port_post_key(KEY_DOWN);
                else choice_pause++;
                if (choice_pause >= 2 || (r % 6u) >= 4u) {
                    ban_port_post_key(KEY_OK);
                    choice_pause = 0;
                }
            } else if (phase == BAN_PHASE_BAN) {
                uint32_t r = input_rng();
                ban_pause++;
                if ((r % 4u) == 0u || ban_pause > 4) {
                    ban_port_post_key(KEY_OK);
                    ban_pause = 0;
                }
            } else if (phase != BAN_PHASE_HOME && (input_rng() % 5u) == 0u) {
                ban_port_post_key((int)(input_rng() % 3u));
            }
        }
        if (!phase_is_choice()) choice_pause = 0;
        if (phase != BAN_PHASE_BAN) ban_pause = 0;
        step_sim(80);
        guard++;
    }
    return ban_game_phase() == BAN_PHASE_GAME_OVER ? 0 : -1;
}

static int run_stress(void) {
    mem_snap_t base;
    mem_snap_t now;
    int game;
    int guard = 0;
    lv_display_t *disp = lv_display_get_default();

    /* 截图已经落盘。后面只查对象和堆，不再画整屏。 */
    lv_display_enable_invalidation(disp, false);
    while (ban_game_phase() != BAN_PHASE_GAME_OVER && guard < 6000) {
        ban_phase_t phase = ban_game_phase();
        if (ban_ui_settled()) {
            if (phase_is_choice()) ban_port_post_key(KEY_OK);
            if (phase == BAN_PHASE_BAN) ban_port_post_key(KEY_OK);
        }
        step_sim(80);
        guard++;
    }
    if (ban_game_phase() != BAN_PHASE_GAME_OVER) {
        fprintf(stderr, "stress: did not reach game over (phase %d)\n", (int)ban_game_phase());
        return 1;
    }
    arm_stress_rng(0);
    if (wait_settled_home(2000) != 0) {
        fprintf(stderr, "stress: warmup did not reach settled home\n");
        return 1;
    }
    step_sim(80);
    if (ban_game_phase() != BAN_PHASE_HOME || !ban_ui_settled() || lv_anim_count_running() != 0) {
        fprintf(stderr, "stress: home not quiet after warmup\n");
        return 1;
    }
    take_snap(&base);
    {
        lv_mem_monitor_t mon;
        lv_mem_monitor(&mon);
        printf("stress baseline used %zu cnt %zu objs %u anims %u timers %u max %zu\n",
               base.used, base.used_cnt, base.objs, base.anims, base.timers, mon.max_used);
    }
    fflush(stdout);

    for (game = 0; game < 200; game++) {
        if (play_one_game(game) != 0) {
            fprintf(stderr, "stress: game %d stuck phase %d\n", game, (int)ban_game_phase());
            return 1;
        }
        /* 下一局的洗牌发生在超时回首页，不发生在首页 OK。 */
        arm_stress_rng(game + 1);
        if (wait_settled_home(2000) != 0) {
            fprintf(stderr, "stress: game %d did not idle home\n", game);
            return 1;
        }
        take_snap(&now);
        /* 块数、对象、动画、定时器不容差。字节允许 TLSF 空闲块头晃动，见报告。
         * 截图关掉刷新后，第一局会把一块多出来的 24 字节拆成空闲块，块数不变。
         * 那一局记下稳态，后面 199 局仍按原容差查。 */
        if (game == 0 && now.used < base.used && now.used + 64u >= base.used &&
            now.used_cnt == base.used_cnt && now.objs == base.objs &&
            now.anims == base.anims && now.timers == base.timers) {
            printf("stress settle used %zu -> %zu\n", base.used, now.used);
            fflush(stdout);
            base = now;
            continue;
        }
        if (now.used_cnt != base.used_cnt || now.objs != base.objs || now.anims != base.anims ||
            now.timers != base.timers || now.used < base.used || now.used > base.used + 64u) {
            fprintf(stderr,
                    "stress leak game %d used %zu (base %zu) cnt %zu (base %zu) "
                    "objs %u (base %u) anims %u (base %u) timers %u (base %u)\n",
                    game, now.used, base.used, now.used_cnt, base.used_cnt,
                    now.objs, base.objs, now.anims, base.anims, now.timers, base.timers);
            dump_labels(lv_screen_active());
            return 1;
        }
        if ((game + 1) % 50 == 0) {
            printf("stress game %d used %zu cnt %zu objs %u anims %u timers %u\n",
                   game + 1, now.used, now.used_cnt, now.objs, now.anims, now.timers);
            fflush(stdout);
        }
    }
    printf("stress 200 games used_delta %zu cnt_delta 0 obj_delta 0 anim_delta 0 timer_delta 0\n",
           now.used - base.used);
    {
        lv_mem_monitor_t mon;
        lv_mem_monitor(&mon);
        printf("stress tolerance bytes 64 blocks 0 objs 0 anims 0 timers 0 max %zu\n", mon.max_used);
    }
    fflush(stdout);
    return 0;
}

static void setup_display(void) {
    lv_display_t *disp = lv_display_create(SCR_W, SCR_H);

    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(disp, s_fb, NULL, sizeof s_fb, LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_flush_cb(disp, flush_cb);
}

int main(int argc, char **argv) {
    ban_phase_t seen = BAN_PHASE_HOME;
    int steps = 0;
    const char *dir = (argc > 1) ? argv[1] : "out";

    snprintf(s_out_dir, sizeof s_out_dir, "%s", dir);
    mkdir(s_out_dir, 0755);
    lv_init();
    setup_display();
    ban_game_set_rng(fixture_rng);
    ban_ui_start();

    while (steps < 4000 && !have("38_meter_watching")) {
        ban_phase_t phase;

        script_input();
        lv_tick_inc(STEP_MS);
        lv_timer_handler();
        phase = ban_game_phase();
        if (phase == BAN_PHASE_SCAN && seen != BAN_PHASE_SCAN) {
            ban_event_id_t ev[BAN_ROUND_EVENTS];
            s_round++;
            ban_game_round(ev);
            printf("round %d events %d %d %d prob %d risk-now %d\n",
                   s_round, (int)ev[0], (int)ev[1], (int)ev[2],
                   ban_game_prob(), ban_game_risk());
            fflush(stdout);
        }
        seen = phase;
        consider();
        steps++;
    }

    printf("steps %d round %d shots %d phase %d\n",
           steps, s_round, s_shot_n, (int)ban_game_phase());
    ban_port_debug_dump();
    if (write_contact() != 0) return 1;
    if (write_gallery() != 0) return 1;
    if (!have("01_home") || !have("01b_home_blink") || !have("03_scan_alert") ||
        !have("23_ban") || !have("28_appeal_stop") || !have("30_over") ||
        !have("30b_over_bits") || !have("15w_final_wipe") || !have("17_stamp_0") ||
        !have("19_stamp_2") ||
        !have("22_stamp_5") || !have("31_idle_3") || !have("37_vpn_sg") ||
        !have("38_meter_watching")) {
        fprintf(stderr, "missing required shots\n");
        return 1;
    }
    return run_stress();
}
