// main/ban_logic.c —— 见 ban_logic.h。
// RNG 调用顺序（可复现）：Fisher–Yates 4 次、概率 1 次、申诉前 BAN_APPEAL_LEN-1 次。
// 每次 rng_below 只取一次随机数的高位。
#include "ban_logic.h"

#include <stddef.h>

/* BAN_STRINGS_BEGIN */
static const ban_copy_t COPY[] = {
    { BAN_COPY_HOME_TITLE, "DARIO BAN-O-METER", "达里奥封号检测仪", "" },
    { BAN_COPY_HOME_SUB, "", "看看你能在", "Claude 活多久" },
    { BAN_COPY_HOME_START, "开始检测", "", "" },
    { BAN_COPY_SCAN_TITLE, "扫描中", "", "" },
    { BAN_COPY_SCAN_IP, "IP", "", "" },
    { BAN_COPY_SCAN_DEVICE, "设备", "", "" },
    { BAN_COPY_SCAN_LANG, "语言", "", "" },
    { BAN_COPY_SCAN_ID, "身份", "", "" },
    { BAN_COPY_SCAN_ALERT, "发现可疑信号", "", "" },
    { BAN_COPY_GIFT_TITLE, "赠送额度", "+$20", "" },
    { BAN_COPY_GIFT_PROMPT, "", "领取赠送额度?", "" },
    { BAN_COPY_GIFT_YES, "领取", "", "" },
    { BAN_COPY_GIFT_NO, "不要", "", "" },
    { BAN_COPY_GIFT_YES_RES, "领取成功", "贪心", "风险 +30" },
    { BAN_COPY_GIFT_NO_RES, "拒绝官方福利", "很可疑", "风险 +20" },
    { BAN_COPY_EMAIL_TITLE, "安全验证", "SECURITY", "" },
    { BAN_COPY_EMAIL_PROMPT, "", "验证你的账号?", "" },
    { BAN_COPY_EMAIL_YES, "点击验证", "", "" },
    { BAN_COPY_EMAIL_NO, "忽略", "", "" },
    { BAN_COPY_EMAIL_YES_RES, "乱点链接", "风险 +30", "" },
    { BAN_COPY_EMAIL_NO_RES, "拒绝验证", "风险 +25", "" },
    { BAN_COPY_CHINA_TITLE, "中国人检测", "", "" },
    { BAN_COPY_CHINA_LANG, "LANGUAGE", "中文", "" },
    { BAN_COPY_CHINA_TZ, "TIMEZONE", "GMT+8", "" },
    { BAN_COPY_CHINA_APP, "APP", "WeChat", "" },
    { BAN_COPY_CHINA_RES, "高度可疑", "风险 +40", "" },
    { BAN_COPY_VPN_TITLE, "VPN瞬移", "", "" },
    { BAN_COPY_VPN_BODY, "3 秒", "去了三个国家", "" },
    { BAN_COPY_VPN_RES, "可疑", "风险 +25", "" },
    { BAN_COPY_IDLE_TITLE, "安全检查", "请勿操作", "" },
    { BAN_COPY_IDLE_RES, "异常低活跃", "风险 +30", "" },
    { BAN_COPY_FINAL, "FINAL REVIEW", "", "" },
    { BAN_COPY_BANNED, "BANNED", "", "" },
    { BAN_COPY_PROB, "中国人概率", "", "" },
    { BAN_COPY_REASON_LABEL, "封号理由", "", "" },
    { BAN_COPY_REASON, "可疑", "", "" },
    { BAN_COPY_APPEAL_BTN, "申诉", "", "" },
    { BAN_COPY_APPEAL, "审核中", "", "" },
    { BAN_COPY_RESTORE, "恢复", "", "" },
    { BAN_COPY_REJECT, "拒绝", "", "" },
    { BAN_COPY_APPEAL_FAIL, "申诉失败", "决定为", "最终决定" },
    { BAN_COPY_CLOSED, "ACCOUNT CLOSED", "", "" },
    { BAN_COPY_SUCCESS, "封号成功", "", "" },
    { BAN_COPY_RETRY, "再测一次", "", "" },
    { BAN_COPY_DISCLAIMER, "内容声明", "本作为讽刺游戏", "不代表真实风控" },
    { BAN_COPY_TIER_SAFE, "SAFE", "", "" },
    { BAN_COPY_TIER_SUSPICIOUS, "SUSPICIOUS", "", "" },
    { BAN_COPY_TIER_HIGH, "HIGH RISK", "", "" },
    { BAN_COPY_TIER_WATCHING, "DARIO IS WATCHING", "", "" },
    { BAN_COPY_TIER_BANNED, "BANNED", "", "" },
};

_Static_assert(sizeof COPY / sizeof COPY[0] == BAN_COPY_COUNT, "copy table size");

static const ban_event_def_t EVENTS[BAN_EVENT_POOL] = {
    { BAN_EVENT_GIFT, BAN_EVENT_CHOICE, 30, 20,
      BAN_COPY_GIFT_TITLE, BAN_COPY_GIFT_PROMPT,
      BAN_COPY_GIFT_YES, BAN_COPY_GIFT_NO,
      BAN_COPY_GIFT_YES_RES, BAN_COPY_GIFT_NO_RES },
    { BAN_EVENT_EMAIL, BAN_EVENT_CHOICE, 30, 25,
      BAN_COPY_EMAIL_TITLE, BAN_COPY_EMAIL_PROMPT,
      BAN_COPY_EMAIL_YES, BAN_COPY_EMAIL_NO,
      BAN_COPY_EMAIL_YES_RES, BAN_COPY_EMAIL_NO_RES },
    { BAN_EVENT_CHINA_SCAN, BAN_EVENT_AUTO, 40, 0,
      BAN_COPY_CHINA_TITLE, BAN_COPY_CHINA_LANG,
      BAN_COPY_NONE, BAN_COPY_NONE,
      BAN_COPY_CHINA_RES, BAN_COPY_NONE },
    { BAN_EVENT_VPN, BAN_EVENT_AUTO, 25, 0,
      BAN_COPY_VPN_TITLE, BAN_COPY_VPN_BODY,
      BAN_COPY_NONE, BAN_COPY_NONE,
      BAN_COPY_VPN_RES, BAN_COPY_NONE },
    { BAN_EVENT_IDLE, BAN_EVENT_AUTO, 30, 0,
      BAN_COPY_IDLE_TITLE, BAN_COPY_IDLE_TITLE,
      BAN_COPY_NONE, BAN_COPY_NONE,
      BAN_COPY_IDLE_RES, BAN_COPY_NONE },
};

const char *ban_disclaimer_full(void) {
    return "本作品为讽刺游戏，不代表真实账号风控结果。";
}

const char *ban_disclaimer_en(void) {
    return "Satirical game based on public information and community discussions.";
}

const char *ban_runtime_glyphs(void) {
    return "0123456789%+ ";
}
/* BAN_STRINGS_END */

static uint32_t (*s_rng)(void);
static uint32_t s_lcg = 0x00C0FFEEu;

static ban_phase_t s_phase;
static uint32_t s_elapsed;
static uint32_t s_idle;
static int s_risk;
static int s_last_delta;
static int s_last_nominal;
static int s_selected;
static int s_event_index;
static ban_event_id_t s_round[BAN_ROUND_EVENTS];
static int s_prob;
static ban_appeal_word_t s_appeal[BAN_APPEAL_LEN];

static uint32_t next_rng(void) {
    if (s_rng) return s_rng();
    s_lcg = s_lcg * 1664525u + 1013904223u;
    return s_lcg;
}

/* 用高位而不是 r%n。LCG 的低位周期很短，取模会让洗牌偏到少数事件。 */
static uint32_t rng_below(uint32_t n) {
    if (n <= 1u) return 0;
    return (uint32_t)(((uint64_t)next_rng() * n) >> 32);
}

static void shuffle_round(void) {
    ban_event_id_t deck[BAN_EVENT_POOL];
    int i;

    for (i = 0; i < BAN_EVENT_POOL; i++) deck[i] = (ban_event_id_t)i;
    for (i = BAN_EVENT_POOL - 1; i > 0; i--) {
        uint32_t j = rng_below((uint32_t)i + 1u);
        ban_event_id_t tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }
    for (i = 0; i < BAN_ROUND_EVENTS; i++) s_round[i] = deck[i];
}

static void roll_appeal(void) {
    int i;
    bool any_restore = false;

    for (i = 0; i < BAN_APPEAL_LEN - 1; i++) {
        s_appeal[i] = (next_rng() & 1u) ? BAN_APPEAL_REJECT : BAN_APPEAL_RESTORE;
        if (s_appeal[i] == BAN_APPEAL_RESTORE) any_restore = true;
    }
    s_appeal[BAN_APPEAL_LEN - 1] = BAN_APPEAL_REJECT;
    if (!any_restore) s_appeal[0] = BAN_APPEAL_RESTORE;
}

static void prepare_round(void) {
    int span = BAN_PROB_MAX - BAN_PROB_MIN + 1;

    shuffle_round();
    s_prob = BAN_PROB_MIN + (int)rng_below((uint32_t)span);
    roll_appeal();
    s_risk = 0;
    s_last_delta = 0;
    s_last_nominal = 0;
    s_selected = 0;
    s_event_index = 0;
}

static const ban_event_def_t *current_def(void) {
    return &EVENTS[s_round[s_event_index]];
}

static void apply_risk(void) {
    const ban_event_def_t *def = current_def();
    int nominal = (def->type == BAN_EVENT_AUTO || s_selected == 0) ? def->risk_a : def->risk_b;
    int before = s_risk;

    s_last_nominal = nominal;
    s_risk = ban_risk_add(s_risk, nominal);
    s_last_delta = s_risk - before;
}

static void enter_phase(ban_phase_t phase) {
    s_phase = phase;
    s_elapsed = 0;
}

static void enter_result(void) {
    apply_risk();
    enter_phase(BAN_PHASE_EVENT_RESULT);
}

static uint32_t phase_limit(void) {
    switch (s_phase) {
    case BAN_PHASE_SCAN: return BAN_MS_SCAN;
    case BAN_PHASE_EVENT:
        return current_def()->type == BAN_EVENT_AUTO ? BAN_MS_AUTO_EVENT : 0;
    case BAN_PHASE_EVENT_RESULT: return BAN_MS_EVENT_RESULT;
    case BAN_PHASE_FINAL_REVIEW: return BAN_MS_FINAL_REVIEW;
    case BAN_PHASE_BAN_ANIM: return BAN_MS_BAN_ANIM;
    case BAN_PHASE_APPEAL: return BAN_MS_APPEAL;
    case BAN_PHASE_APPEAL_RESULT: return BAN_MS_APPEAL_RESULT;
    default: return 0;
    }
}

static void advance_timed(void) {
    switch (s_phase) {
    case BAN_PHASE_SCAN:
        s_event_index = 0;
        s_selected = 0;
        enter_phase(BAN_PHASE_EVENT);
        break;
    case BAN_PHASE_EVENT:
        enter_result();
        break;
    case BAN_PHASE_EVENT_RESULT:
        if (s_event_index + 1 < BAN_ROUND_EVENTS) {
            s_event_index++;
            s_selected = 0;
            enter_phase(BAN_PHASE_EVENT);
        } else {
            enter_phase(BAN_PHASE_FINAL_REVIEW);
        }
        break;
    case BAN_PHASE_FINAL_REVIEW:
        enter_phase(BAN_PHASE_BAN_ANIM);
        break;
    case BAN_PHASE_BAN_ANIM:
        enter_phase(BAN_PHASE_BAN);
        break;
    case BAN_PHASE_APPEAL:
        enter_phase(BAN_PHASE_APPEAL_RESULT);
        break;
    case BAN_PHASE_APPEAL_RESULT:
        enter_phase(BAN_PHASE_GAME_OVER);
        s_idle = 0;
        break;
    default:
        break;
    }
}

static int appeal_index_at(uint32_t elapsed) {
    /* 末尾停在「拒绝」上再切结果页。tick 在 elapsed==时长 时立刻离开本阶段，
       所以最后一格必须在时长之内出现，否则界面永远看不到落点。 */
    const uint32_t hold = 480u;
    uint32_t motion;
    uint32_t t;
    uint32_t inv;
    uint32_t inv3;
    uint32_t eased;
    int idx;

    if (elapsed >= BAN_MS_APPEAL) return BAN_APPEAL_LEN - 1;
    if (BAN_MS_APPEAL > hold && elapsed >= BAN_MS_APPEAL - hold) return BAN_APPEAL_LEN - 1;
    motion = (BAN_MS_APPEAL > hold) ? (BAN_MS_APPEAL - hold) : BAN_MS_APPEAL;
    t = (elapsed * 256u) / motion;
    if (t > 256u) t = 256u;
    inv = 256u - t;
    inv3 = inv * inv * inv;
    eased = 256u * 256u * 256u - inv3;
    idx = (int)((eased * (uint32_t)BAN_APPEAL_LEN) / (256u * 256u * 256u));
    if (idx < 0) idx = 0;
    if (idx > BAN_APPEAL_LEN - 1) idx = BAN_APPEAL_LEN - 1;
    return idx;
}

void ban_game_set_rng(uint32_t (*rng)(void)) {
    s_rng = rng;
}

void ban_game_reset(void) {
    prepare_round();
    s_phase = BAN_PHASE_HOME;
    s_elapsed = 0;
    s_idle = 0;
}

bool ban_game_input(ban_input_t input) {
    if (input != BAN_INPUT_UP && input != BAN_INPUT_DOWN && input != BAN_INPUT_OK) return false;
    s_idle = 0;

    if (s_phase == BAN_PHASE_HOME && input == BAN_INPUT_OK) {
        s_event_index = 0;
        s_selected = 0;
        s_risk = 0;
        s_last_delta = 0;
        s_last_nominal = 0;
        enter_phase(BAN_PHASE_SCAN);
        return true;
    }
    if (s_phase == BAN_PHASE_GAME_OVER && input == BAN_INPUT_OK) {
        prepare_round();
        enter_phase(BAN_PHASE_SCAN);
        s_idle = 0;
        return true;
    }
    if (s_phase == BAN_PHASE_BAN && input == BAN_INPUT_OK) {
        enter_phase(BAN_PHASE_APPEAL);
        return true;
    }
    if (s_phase == BAN_PHASE_EVENT && current_def()->type == BAN_EVENT_CHOICE) {
        if (input == BAN_INPUT_UP && s_selected > 0) {
            s_selected--;
            return true;
        }
        if (input == BAN_INPUT_DOWN && s_selected < 1) {
            s_selected++;
            return true;
        }
        if (input == BAN_INPUT_OK) {
            enter_result();
            return true;
        }
    }
    return false;
}

void ban_game_tick(uint32_t ms) {
    while (ms > 0) {
        if (s_phase == BAN_PHASE_HOME || s_phase == BAN_PHASE_GAME_OVER) {
            uint32_t remain = (s_idle >= BAN_MS_IDLE_RETURN) ? 0 : (BAN_MS_IDLE_RETURN - s_idle);
            if (ms < remain) {
                s_idle += ms;
                return;
            }
            ms -= remain;
            s_idle = 0;
            if (s_phase == BAN_PHASE_GAME_OVER) {
                prepare_round();
                s_phase = BAN_PHASE_HOME;
                s_elapsed = 0;
                continue;
            }
            return;
        }

        {
            uint32_t limit = phase_limit();
            uint32_t remain;

            if (limit == 0) return;
            remain = (s_elapsed >= limit) ? 0 : (limit - s_elapsed);
            if (ms < remain) {
                s_elapsed += ms;
                return;
            }
            ms -= remain;
            s_elapsed = limit;
            advance_timed();
        }
    }
}

ban_phase_t ban_game_phase(void) { return s_phase; }
int ban_game_event_index(void) { return s_event_index; }

ban_event_id_t ban_game_event(void) {
    return s_round[s_event_index];
}

void ban_game_round(ban_event_id_t out[BAN_ROUND_EVENTS]) {
    int i;
    for (i = 0; i < BAN_ROUND_EVENTS; i++) out[i] = s_round[i];
}

int ban_game_selected(void) { return s_selected; }
int ban_game_risk(void) { return s_risk; }
int ban_game_last_delta(void) { return s_last_delta; }
int ban_game_last_nominal(void) { return s_last_nominal; }
int ban_game_prob(void) { return s_prob; }
uint32_t ban_game_elapsed(void) { return s_elapsed; }

int ban_game_auto_step(void) {
    ban_event_id_t id;
    int step;

    if (s_phase != BAN_PHASE_EVENT) return 0;
    id = s_round[s_event_index];
    if (id != BAN_EVENT_CHINA_SCAN && id != BAN_EVENT_VPN) return 0;
    step = (int)((s_elapsed * 3u) / BAN_MS_AUTO_EVENT);
    if (step > 2) step = 2;
    return step;
}

int ban_game_countdown(void) {
    uint32_t into;
    int n;

    if (s_phase != BAN_PHASE_EVENT || s_round[s_event_index] != BAN_EVENT_IDLE) return 0;
    if (s_elapsed < BAN_MS_AUTO_EVENT - BAN_MS_COUNTDOWN) return 0;
    into = s_elapsed - (BAN_MS_AUTO_EVENT - BAN_MS_COUNTDOWN);
    n = 3 - (int)(into / BAN_MS_COUNTDOWN_STEP);
    if (n < 1) n = 1;
    if (n > 3) n = 3;
    return n;
}

bool ban_game_scan_alert(void) {
    return s_phase == BAN_PHASE_SCAN && s_elapsed >= BAN_MS_SCAN_ALERT;
}

int ban_game_appeal_len(void) { return BAN_APPEAL_LEN; }

ban_appeal_word_t ban_game_appeal_at(int index) {
    if (index < 0 || index >= BAN_APPEAL_LEN) return BAN_APPEAL_REJECT;
    return s_appeal[index];
}

int ban_game_appeal_cursor(void) {
    if (s_phase == BAN_PHASE_APPEAL) return appeal_index_at(s_elapsed);
    if (s_phase == BAN_PHASE_APPEAL_RESULT || s_phase == BAN_PHASE_GAME_OVER) {
        return BAN_APPEAL_LEN - 1;
    }
    return 0;
}

bool ban_game_appeal_rejected(void) { return true; }

const ban_copy_t *ban_copy_by_id(ban_copy_id_t id) {
    if ((int)id < 0 || id >= BAN_COPY_COUNT) return NULL;
    return &COPY[id];
}

const ban_copy_t *ban_copy_table(int *count) {
    if (count) *count = BAN_COPY_COUNT;
    return COPY;
}

const ban_event_def_t *ban_event_def(ban_event_id_t id) {
    if ((int)id < 0 || (int)id >= BAN_EVENT_POOL) return NULL;
    return &EVENTS[id];
}

int ban_event_risk(ban_event_id_t id, int choice) {
    const ban_event_def_t *def = ban_event_def(id);
    if (!def) return 0;
    if (def->type == BAN_EVENT_AUTO || choice == 0) return def->risk_a;
    return def->risk_b;
}

ban_copy_id_t ban_game_result_copy(void) {
    const ban_event_def_t *def = current_def();
    if (def->type == BAN_EVENT_AUTO || s_selected == 0) return (ban_copy_id_t)def->result_a;
    return (ban_copy_id_t)def->result_b;
}

int ban_risk_add(int risk, int delta) {
    if (delta < 0) delta = 0;
    if (risk < 0) risk = 0;
    if (risk >= BAN_RISK_CAP) return BAN_RISK_CAP;
    if (delta >= BAN_RISK_CAP - risk) return BAN_RISK_CAP;
    return risk + delta;
}

ban_risk_tier_t ban_risk_tier(int risk) {
    if (risk >= 100) return BAN_TIER_BANNED;
    if (risk >= 80) return BAN_TIER_WATCHING;
    if (risk >= 60) return BAN_TIER_HIGH;
    if (risk >= 30) return BAN_TIER_SUSPICIOUS;
    return BAN_TIER_SAFE;
}

int ban_tier_color(ban_risk_tier_t tier) {
    switch (tier) {
    case BAN_TIER_SAFE: return 0;
    case BAN_TIER_SUSPICIOUS: return 1;
    case BAN_TIER_HIGH: return 2;
    default: return 3;
    }
}

const char *ban_tier_name(ban_risk_tier_t tier) {
    switch (tier) {
    case BAN_TIER_SAFE: return COPY[BAN_COPY_TIER_SAFE].title;
    case BAN_TIER_SUSPICIOUS: return COPY[BAN_COPY_TIER_SUSPICIOUS].title;
    case BAN_TIER_HIGH: return COPY[BAN_COPY_TIER_HIGH].title;
    case BAN_TIER_WATCHING: return COPY[BAN_COPY_TIER_WATCHING].title;
    default: return COPY[BAN_COPY_TIER_BANNED].title;
    }
}

ban_face_t ban_face_for_risk(int risk) {
    if (risk > 70) return BAN_FACE_ANGRY;
    if (risk >= 30) return BAN_FACE_SUSPICIOUS;
    return BAN_FACE_NORMAL;
}

static ban_face_t elevate(ban_face_t face) {
    if (face == BAN_FACE_BANNED) return face;
    if (s_risk > 70) return BAN_FACE_ANGRY;
    return face;
}

ban_face_t ban_game_face(void) {
    ban_event_id_t id = s_round[s_event_index];

    switch (s_phase) {
    case BAN_PHASE_HOME:
    case BAN_PHASE_SCAN:
        return BAN_FACE_NORMAL;
    case BAN_PHASE_EVENT:
        if (id == BAN_EVENT_CHINA_SCAN) {
            static const ban_face_t seq[3] = {
                BAN_FACE_NORMAL, BAN_FACE_SUSPICIOUS, BAN_FACE_ANGRY,
            };
            return elevate(seq[ban_game_auto_step()]);
        }
        if (id == BAN_EVENT_VPN || id == BAN_EVENT_IDLE) return elevate(BAN_FACE_SUSPICIOUS);
        return elevate(ban_face_for_risk(s_risk));
    case BAN_PHASE_EVENT_RESULT:
        if (id == BAN_EVENT_CHINA_SCAN) return elevate(BAN_FACE_ANGRY);
        if (id == BAN_EVENT_GIFT || id == BAN_EVENT_EMAIL || id == BAN_EVENT_VPN || id == BAN_EVENT_IDLE) {
            return elevate(BAN_FACE_SUSPICIOUS);
        }
        return elevate(ban_face_for_risk(s_risk));
    case BAN_PHASE_FINAL_REVIEW:
        if (s_elapsed * 2u >= BAN_MS_FINAL_REVIEW) return BAN_FACE_BANNED;
        return s_risk > 70 ? BAN_FACE_ANGRY : BAN_FACE_SUSPICIOUS;
    case BAN_PHASE_BAN_ANIM:
    case BAN_PHASE_BAN:
    case BAN_PHASE_APPEAL:
    case BAN_PHASE_APPEAL_RESULT:
    case BAN_PHASE_GAME_OVER:
        return BAN_FACE_BANNED;
    default:
        return BAN_FACE_NORMAL;
    }
}

ban_risk_tier_t ban_game_tier(void) {
    return ban_risk_tier(s_risk);
}
