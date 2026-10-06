// main/ban_logic.h —— 达里奥封号检测仪的纯逻辑。
// 不依赖 ESP-IDF / LVGL。固件侧在别处把 esp_random 包成 ban_game_set_rng 注入。
#pragma once

#include <stdbool.h>
#include <stdint.h>

/* 时长集中在这里，单位毫秒。自动路径大约 29 秒，加上封号页阅读落在 30–45 秒。 */
#define BAN_MS_SCAN              2500
#define BAN_MS_SCAN_ALERT        2000   /* 扫描末段切到「发现可疑信号」 */
#define BAN_MS_AUTO_EVENT        4000   /* 自动事件整段 */
#define BAN_MS_COUNTDOWN         3000   /* 其中 3-2-1 */
#define BAN_MS_COUNTDOWN_STEP    1000
#define BAN_MS_EVENT_RESULT      2200
#define BAN_MS_FINAL_REVIEW      2000
#define BAN_MS_BAN_ANIM           800
#define BAN_MS_APPEAL            3200
#define BAN_MS_APPEAL_RESULT     1800
#define BAN_MS_IDLE_RETURN      25000   /* 首页与结束页无操作回首页 */

#define BAN_RISK_CAP               100
#define BAN_PROB_MIN                81
#define BAN_PROB_MAX                99
#define BAN_EVENT_POOL               5
#define BAN_ROUND_EVENTS             3
#define BAN_APPEAL_LEN               8
#define BAN_COPY_NONE               (-1)

typedef enum {
    BAN_PHASE_HOME = 0,
    BAN_PHASE_SCAN,
    BAN_PHASE_EVENT,
    BAN_PHASE_EVENT_RESULT,
    BAN_PHASE_FINAL_REVIEW,
    BAN_PHASE_BAN_ANIM,
    BAN_PHASE_BAN,
    BAN_PHASE_APPEAL,
    BAN_PHASE_APPEAL_RESULT,
    BAN_PHASE_GAME_OVER,
} ban_phase_t;

typedef enum {
    BAN_EVENT_GIFT = 0,
    BAN_EVENT_EMAIL,
    BAN_EVENT_CHINA_SCAN,
    BAN_EVENT_VPN,
    BAN_EVENT_IDLE,
} ban_event_id_t;

typedef enum {
    BAN_EVENT_CHOICE = 0,
    BAN_EVENT_AUTO,
} ban_event_type_t;

typedef enum {
    BAN_INPUT_UP = 0,
    BAN_INPUT_DOWN,
    BAN_INPUT_OK,
} ban_input_t;

typedef enum {
    BAN_TIER_SAFE = 0,          /* 0–29  绿 */
    BAN_TIER_SUSPICIOUS,        /* 30–59 黄 */
    BAN_TIER_HIGH,              /* 60–79 橙 */
    BAN_TIER_WATCHING,          /* 80–99 红 */
    BAN_TIER_BANNED,            /* 100   红 */
} ban_risk_tier_t;

typedef enum {
    BAN_FACE_NORMAL = 0,
    BAN_FACE_SUSPICIOUS,
    BAN_FACE_ANGRY,
    BAN_FACE_BANNED,
} ban_face_t;

typedef enum {
    BAN_APPEAL_RESTORE = 0,
    BAN_APPEAL_REJECT,
} ban_appeal_word_t;

typedef enum {
    BAN_COPY_HOME_TITLE = 0,
    BAN_COPY_HOME_SUB,
    BAN_COPY_HOME_START,
    BAN_COPY_SCAN_TITLE,
    BAN_COPY_SCAN_IP,
    BAN_COPY_SCAN_DEVICE,
    BAN_COPY_SCAN_LANG,
    BAN_COPY_SCAN_ID,
    BAN_COPY_SCAN_ALERT,
    BAN_COPY_GIFT_TITLE,
    BAN_COPY_GIFT_PROMPT,
    BAN_COPY_GIFT_YES,
    BAN_COPY_GIFT_NO,
    BAN_COPY_GIFT_YES_RES,
    BAN_COPY_GIFT_NO_RES,
    BAN_COPY_EMAIL_TITLE,
    BAN_COPY_EMAIL_PROMPT,
    BAN_COPY_EMAIL_YES,
    BAN_COPY_EMAIL_NO,
    BAN_COPY_EMAIL_YES_RES,
    BAN_COPY_EMAIL_NO_RES,
    BAN_COPY_CHINA_TITLE,
    BAN_COPY_CHINA_LANG,
    BAN_COPY_CHINA_TZ,
    BAN_COPY_CHINA_APP,
    BAN_COPY_CHINA_RES,
    BAN_COPY_VPN_TITLE,
    BAN_COPY_VPN_BODY,
    BAN_COPY_VPN_RES,
    BAN_COPY_IDLE_TITLE,
    BAN_COPY_IDLE_RES,
    BAN_COPY_FINAL,
    BAN_COPY_BANNED,
    BAN_COPY_PROB,
    BAN_COPY_REASON_LABEL,
    BAN_COPY_REASON,
    BAN_COPY_APPEAL_BTN,
    BAN_COPY_APPEAL,
    BAN_COPY_RESTORE,
    BAN_COPY_REJECT,
    BAN_COPY_APPEAL_FAIL,
    BAN_COPY_CLOSED,
    BAN_COPY_SUCCESS,
    BAN_COPY_RETRY,
    BAN_COPY_DISCLAIMER,
    BAN_COPY_TIER_SAFE,
    BAN_COPY_TIER_SUSPICIOUS,
    BAN_COPY_TIER_HIGH,
    BAN_COPY_TIER_WATCHING,
    BAN_COPY_TIER_BANNED,
    BAN_COPY_COUNT,
} ban_copy_id_t;

/* 标题、两行正文。空串表示这一行不显示。汉字长度见 DESIGN.md §21。 */
typedef struct {
    ban_copy_id_t id;
    const char *title;
    const char *line1;
    const char *line2;
} ban_copy_t;

typedef struct {
    ban_event_id_t id;
    ban_event_type_t type;
    int risk_a;                 /* 选项 0，或自动事件的增量 */
    int risk_b;                 /* 选项 1；自动事件为 0 */
    int title;
    int prompt;
    int choice_a;
    int choice_b;
    int result_a;
    int result_b;
} ban_event_def_t;

/* rng 返回任意 uint32。NULL 恢复内置 LCG。须在 ban_game_reset 之前设置。 */
void ban_game_set_rng(uint32_t (*rng)(void));
void ban_game_reset(void);
bool ban_game_input(ban_input_t input);
void ban_game_tick(uint32_t ms);

ban_phase_t ban_game_phase(void);
int ban_game_event_index(void);
ban_event_id_t ban_game_event(void);
void ban_game_round(ban_event_id_t out[BAN_ROUND_EVENTS]);
int ban_game_selected(void);
int ban_game_risk(void);
int ban_game_last_delta(void);          /* 实际加上的值，封顶后可能小于名义值 */
int ban_game_last_nominal(void);
int ban_game_prob(void);                /* 局内固定，81–99 */
uint32_t ban_game_elapsed(void);
int ban_game_auto_step(void);           /* 自动事件 0..2 */
int ban_game_countdown(void);           /* 闲置事件 3/2/1，其余为 0 */
bool ban_game_scan_alert(void);

int ban_game_appeal_len(void);
ban_appeal_word_t ban_game_appeal_at(int index);
int ban_game_appeal_cursor(void);
bool ban_game_appeal_rejected(void);    /* 恒为 true：最终词必是拒绝 */

const ban_copy_t *ban_copy_by_id(ban_copy_id_t id);
const ban_copy_t *ban_copy_table(int *count);
const ban_event_def_t *ban_event_def(ban_event_id_t id);
int ban_event_risk(ban_event_id_t id, int choice);
ban_copy_id_t ban_game_result_copy(void);

/* §31 全文，给字体子集和小字声明。屏上两行短句见 BAN_COPY_DISCLAIMER。 */
const char *ban_disclaimer_full(void);
const char *ban_disclaimer_en(void);
const char *ban_runtime_glyphs(void);

int ban_risk_add(int risk, int delta);
ban_risk_tier_t ban_risk_tier(int risk);
int ban_tier_color(ban_risk_tier_t tier);   /* 0 绿 1 黄 2 橙 3 红 */
const char *ban_tier_name(ban_risk_tier_t tier);
ban_face_t ban_face_for_risk(int risk);     /* >70 愤怒，>=30 怀疑 */
ban_face_t ban_game_face(void);
ban_risk_tier_t ban_game_tier(void);
