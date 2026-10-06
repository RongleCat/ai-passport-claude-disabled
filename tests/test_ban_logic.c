#include "ban_logic.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int g_fails;

#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        g_fails++; \
    } \
} while (0)

static int han_count(const char *s) {
    int n = 0;
    const unsigned char *p = (const unsigned char *)s;

    if (!p) return 0;
    while (*p) {
        uint32_t cp;
        if (*p < 0x80) {
            cp = *p++;
        } else if ((*p & 0xE0) == 0xC0 && p[1]) {
            cp = ((uint32_t)(*p & 0x1F) << 6) | (p[1] & 0x3F);
            p += 2;
        } else if ((*p & 0xF0) == 0xE0 && p[1] && p[2]) {
            cp = ((uint32_t)(*p & 0x0F) << 12) | ((uint32_t)(p[1] & 0x3F) << 6) | (p[2] & 0x3F);
            p += 3;
        } else if ((*p & 0xF8) == 0xF0 && p[1] && p[2] && p[3]) {
            cp = ((uint32_t)(*p & 0x07) << 18) | ((uint32_t)(p[1] & 0x3F) << 12)
               | ((uint32_t)(p[2] & 0x3F) << 6) | (p[3] & 0x3F);
            p += 4;
        } else {
            return -1;
        }
        if ((cp >= 0x3400 && cp <= 0x4DBF) || (cp >= 0x4E00 && cp <= 0x9FFF)) n++;
    }
    return n;
}

static uint32_t s_lcg;

static uint32_t lcg_rng(void) {
    s_lcg = s_lcg * 1664525u + 1013904223u;
    return s_lcg;
}

static uint32_t g_seed_gift;
static uint32_t g_seed_idle;
static uint32_t g_seed_china;

static void ensure_seeds(void) {
    uint32_t seed;

    if (g_seed_gift && g_seed_idle && g_seed_china) return;
    ban_game_set_rng(lcg_rng);
    for (seed = 1; seed < 200000u && !(g_seed_gift && g_seed_idle && g_seed_china); seed++) {
        ban_event_id_t round[BAN_ROUND_EVENTS];

        s_lcg = seed;
        ban_game_reset();
        ban_game_round(round);
        if (!g_seed_gift && round[0] == BAN_EVENT_GIFT && round[1] == BAN_EVENT_EMAIL
            && round[2] == BAN_EVENT_CHINA_SCAN) {
            g_seed_gift = seed;
        }
        if (!g_seed_idle && round[0] == BAN_EVENT_IDLE) g_seed_idle = seed;
        if (!g_seed_china && round[0] == BAN_EVENT_CHINA_SCAN) g_seed_china = seed;
    }
    CHECK(g_seed_gift != 0);
    CHECK(g_seed_idle != 0);
    CHECK(g_seed_china != 0);
}

static void reset_seed(uint32_t seed) {
    ban_game_set_rng(lcg_rng);
    s_lcg = seed;
    ban_game_reset();
}

static void reset_gift_email_china(void) {
    ensure_seeds();
    reset_seed(g_seed_gift);
}

static void reset_idle_first(void) {
    ensure_seeds();
    reset_seed(g_seed_idle);
}

static void reset_china_first(void) {
    ensure_seeds();
    reset_seed(g_seed_china);
}

static void test_tables(void) {
    int i;
    int count = 0;
    const ban_copy_t *table = ban_copy_table(&count);

    CHECK(count == BAN_COPY_COUNT);
    for (i = 0; i < count; i++) {
        const ban_copy_t *row = ban_copy_by_id((ban_copy_id_t)i);
        int title = han_count(row->title);
        int line1 = han_count(row->line1);
        int line2 = han_count(row->line2);
        int lines = 0;

        CHECK(row == &table[i]);
        CHECK(row->id == (ban_copy_id_t)i);
        CHECK(title >= 0 && title <= 8);
        CHECK(line1 >= 0 && line1 <= 10);
        CHECK(line2 >= 0 && line2 <= 10);
        if (line1 > 0 || row->line1[0] != '\0') lines++;
        if (row->line2[0] != '\0') lines++;
        CHECK(lines <= 2);
    }
    CHECK(ban_copy_by_id((ban_copy_id_t)BAN_COPY_COUNT) == NULL);

    CHECK(ban_event_def(BAN_EVENT_GIFT)->type == BAN_EVENT_CHOICE);
    CHECK(ban_event_def(BAN_EVENT_EMAIL)->type == BAN_EVENT_CHOICE);
    CHECK(ban_event_def(BAN_EVENT_CHINA_SCAN)->type == BAN_EVENT_AUTO);
    CHECK(ban_event_def(BAN_EVENT_VPN)->type == BAN_EVENT_AUTO);
    CHECK(ban_event_def(BAN_EVENT_IDLE)->type == BAN_EVENT_AUTO);
    CHECK(ban_event_risk(BAN_EVENT_GIFT, 0) == 30);
    CHECK(ban_event_risk(BAN_EVENT_GIFT, 1) == 20);
    CHECK(ban_event_risk(BAN_EVENT_EMAIL, 0) == 30);
    CHECK(ban_event_risk(BAN_EVENT_EMAIL, 1) == 25);
    CHECK(ban_event_risk(BAN_EVENT_CHINA_SCAN, 0) == 40);
    CHECK(ban_event_risk(BAN_EVENT_VPN, 0) == 25);
    CHECK(ban_event_risk(BAN_EVENT_IDLE, 0) == 30);
    for (i = 0; i < BAN_EVENT_POOL; i++) CHECK(ban_event_def((ban_event_id_t)i)->id == (ban_event_id_t)i);
}

static void test_tiers_and_cap(void) {
    CHECK(ban_risk_add(0, 0) == 0);
    CHECK(ban_risk_add(80, 30) == 100);
    CHECK(ban_risk_add(100, 40) == 100);
    CHECK(ban_risk_add(99, 1) == 100);
    CHECK(ban_risk_add(10, 5) == 15);
    CHECK(ban_risk_tier(0) == BAN_TIER_SAFE);
    CHECK(ban_risk_tier(29) == BAN_TIER_SAFE);
    CHECK(ban_risk_tier(30) == BAN_TIER_SUSPICIOUS);
    CHECK(ban_risk_tier(59) == BAN_TIER_SUSPICIOUS);
    CHECK(ban_risk_tier(60) == BAN_TIER_HIGH);
    CHECK(ban_risk_tier(79) == BAN_TIER_HIGH);
    CHECK(ban_risk_tier(80) == BAN_TIER_WATCHING);
    CHECK(ban_risk_tier(99) == BAN_TIER_WATCHING);
    CHECK(ban_risk_tier(100) == BAN_TIER_BANNED);
    CHECK(ban_tier_color(BAN_TIER_SAFE) == 0);
    CHECK(ban_tier_color(BAN_TIER_SUSPICIOUS) == 1);
    CHECK(ban_tier_color(BAN_TIER_HIGH) == 2);
    CHECK(ban_tier_color(BAN_TIER_WATCHING) == 3);
    CHECK(ban_tier_color(BAN_TIER_BANNED) == 3);
    CHECK(strcmp(ban_tier_name(BAN_TIER_SAFE), "SAFE") == 0);
    CHECK(strcmp(ban_tier_name(BAN_TIER_SUSPICIOUS), "SUSPICIOUS") == 0);
    CHECK(strcmp(ban_tier_name(BAN_TIER_HIGH), "HIGH RISK") == 0);
    CHECK(strcmp(ban_tier_name(BAN_TIER_WATCHING), "DARIO IS WATCHING") == 0);
    CHECK(strcmp(ban_tier_name(BAN_TIER_BANNED), "BANNED") == 0);
    CHECK(ban_face_for_risk(0) == BAN_FACE_NORMAL);
    CHECK(ban_face_for_risk(29) == BAN_FACE_NORMAL);
    CHECK(ban_face_for_risk(30) == BAN_FACE_SUSPICIOUS);
    CHECK(ban_face_for_risk(70) == BAN_FACE_SUSPICIOUS);
    CHECK(ban_face_for_risk(71) == BAN_FACE_ANGRY);
}

static void test_choice_risks_and_flow(void) {
    ban_event_id_t round[BAN_ROUND_EVENTS];

    reset_gift_email_china();
    ban_game_round(round);
    CHECK(round[0] == BAN_EVENT_GIFT);
    CHECK(round[1] == BAN_EVENT_EMAIL);
    CHECK(round[2] == BAN_EVENT_CHINA_SCAN);
    CHECK(ban_game_phase() == BAN_PHASE_HOME);
    CHECK(ban_game_prob() >= BAN_PROB_MIN && ban_game_prob() <= BAN_PROB_MAX);
    CHECK(ban_game_appeal_at(BAN_APPEAL_LEN - 1) == BAN_APPEAL_REJECT);
    CHECK(ban_game_appeal_rejected());

    CHECK(ban_game_input(BAN_INPUT_OK));
    CHECK(ban_game_phase() == BAN_PHASE_SCAN);
    ban_game_tick(BAN_MS_SCAN_ALERT - 1);
    CHECK(!ban_game_scan_alert());
    ban_game_tick(1);
    CHECK(ban_game_scan_alert());
    CHECK(ban_game_phase() == BAN_PHASE_SCAN);
    ban_game_tick(BAN_MS_SCAN - BAN_MS_SCAN_ALERT);
    CHECK(ban_game_phase() == BAN_PHASE_EVENT);
    CHECK(ban_game_event() == BAN_EVENT_GIFT);
    CHECK(ban_game_selected() == 0);

    ban_game_tick(60000);
    CHECK(ban_game_phase() == BAN_PHASE_EVENT);
    CHECK(ban_game_risk() == 0);

    reset_gift_email_china();
    CHECK(ban_game_input(BAN_INPUT_OK));
    ban_game_tick(BAN_MS_SCAN);
    CHECK(ban_game_input(BAN_INPUT_DOWN));
    CHECK(ban_game_selected() == 1);
    CHECK(!ban_game_input(BAN_INPUT_DOWN));
    CHECK(ban_game_selected() == 1);
    CHECK(ban_game_input(BAN_INPUT_UP));
    CHECK(ban_game_selected() == 0);
    CHECK(ban_game_input(BAN_INPUT_DOWN));
    CHECK(ban_game_input(BAN_INPUT_OK));
    CHECK(ban_game_phase() == BAN_PHASE_EVENT_RESULT);
    CHECK(ban_game_risk() == 20);
    CHECK(ban_game_last_nominal() == 20);
    CHECK(ban_game_last_delta() == 20);
    CHECK(ban_game_result_copy() == BAN_COPY_GIFT_NO_RES);
    CHECK(ban_game_face() == BAN_FACE_SUSPICIOUS);

    ban_game_tick(BAN_MS_EVENT_RESULT);
    CHECK(ban_game_phase() == BAN_PHASE_EVENT);
    CHECK(ban_game_event() == BAN_EVENT_EMAIL);
    CHECK(ban_game_input(BAN_INPUT_DOWN));
    CHECK(ban_game_input(BAN_INPUT_OK));
    CHECK(ban_game_risk() == 45);
    CHECK(ban_game_last_nominal() == 25);

    ban_game_tick(BAN_MS_EVENT_RESULT);
    CHECK(ban_game_event() == BAN_EVENT_CHINA_SCAN);
    CHECK(ban_game_phase() == BAN_PHASE_EVENT);
    CHECK(!ban_game_input(BAN_INPUT_OK));
    ban_game_tick(BAN_MS_AUTO_EVENT - 1);
    CHECK(ban_game_phase() == BAN_PHASE_EVENT);
    ban_game_tick(1);
    CHECK(ban_game_phase() == BAN_PHASE_EVENT_RESULT);
    CHECK(ban_game_risk() == 85);
    CHECK(ban_game_tier() == BAN_TIER_WATCHING);
    CHECK(ban_tier_color(ban_game_tier()) == 3);
    CHECK(ban_game_face() == BAN_FACE_ANGRY);

    ban_game_tick(BAN_MS_EVENT_RESULT);
    CHECK(ban_game_phase() == BAN_PHASE_FINAL_REVIEW);
    CHECK(ban_game_face() == BAN_FACE_ANGRY);
    ban_game_tick(BAN_MS_FINAL_REVIEW / 2);
    CHECK(ban_game_phase() == BAN_PHASE_FINAL_REVIEW);
    CHECK(ban_game_face() == BAN_FACE_BANNED);
    ban_game_tick(BAN_MS_FINAL_REVIEW / 2);
    CHECK(ban_game_phase() == BAN_PHASE_BAN_ANIM);
    ban_game_tick(BAN_MS_BAN_ANIM - 1);
    CHECK(ban_game_phase() == BAN_PHASE_BAN_ANIM);
    ban_game_tick(1);
    CHECK(ban_game_phase() == BAN_PHASE_BAN);
    CHECK(ban_game_face() == BAN_FACE_BANNED);
    CHECK(ban_game_input(BAN_INPUT_OK));
    CHECK(ban_game_phase() == BAN_PHASE_APPEAL);
    CHECK(ban_game_appeal_cursor() == 0);
    ban_game_tick(BAN_MS_APPEAL);
    CHECK(ban_game_phase() == BAN_PHASE_APPEAL_RESULT);
    CHECK(ban_game_appeal_cursor() == BAN_APPEAL_LEN - 1);
    CHECK(ban_game_appeal_at(ban_game_appeal_cursor()) == BAN_APPEAL_REJECT);
    CHECK(ban_game_face() == BAN_FACE_BANNED);
    ban_game_tick(BAN_MS_APPEAL_RESULT);
    CHECK(ban_game_phase() == BAN_PHASE_GAME_OVER);
    ban_game_tick(BAN_MS_IDLE_RETURN - 1);
    CHECK(ban_game_phase() == BAN_PHASE_GAME_OVER);
    ban_game_tick(1);
    CHECK(ban_game_phase() == BAN_PHASE_HOME);
    ban_game_tick(BAN_MS_IDLE_RETURN);
    CHECK(ban_game_phase() == BAN_PHASE_HOME);
}

static void test_cap_path(void) {
    reset_gift_email_china();
    CHECK(ban_game_input(BAN_INPUT_OK));
    ban_game_tick(BAN_MS_SCAN);
    CHECK(ban_game_input(BAN_INPUT_OK));
    CHECK(ban_game_risk() == 30);
    CHECK(ban_game_result_copy() == BAN_COPY_GIFT_YES_RES);
    ban_game_tick(BAN_MS_EVENT_RESULT);
    CHECK(ban_game_input(BAN_INPUT_OK));
    CHECK(ban_game_risk() == 60);
    CHECK(ban_game_tier() == BAN_TIER_HIGH);
    CHECK(ban_tier_color(ban_game_tier()) == 2);
    ban_game_tick(BAN_MS_EVENT_RESULT);
    ban_game_tick(BAN_MS_AUTO_EVENT);
    CHECK(ban_game_risk() == 100);
    CHECK(ban_game_last_delta() == 40);
    CHECK(ban_game_tier() == BAN_TIER_BANNED);
    ban_game_tick(100000);
    CHECK(ban_game_risk() == 100);
}

static void test_idle_countdown(void) {
    reset_idle_first();
    CHECK(ban_game_event() == BAN_EVENT_IDLE);
    CHECK(ban_game_input(BAN_INPUT_OK));
    ban_game_tick(BAN_MS_SCAN);
    CHECK(ban_game_phase() == BAN_PHASE_EVENT);
    CHECK(ban_game_countdown() == 0);
    CHECK(ban_game_face() == BAN_FACE_SUSPICIOUS);
    ban_game_tick(BAN_MS_AUTO_EVENT - BAN_MS_COUNTDOWN - 1);
    CHECK(ban_game_countdown() == 0);
    ban_game_tick(1);
    CHECK(ban_game_countdown() == 3);
    ban_game_tick(BAN_MS_COUNTDOWN_STEP);
    CHECK(ban_game_countdown() == 2);
    ban_game_tick(BAN_MS_COUNTDOWN_STEP);
    CHECK(ban_game_countdown() == 1);
    ban_game_tick(BAN_MS_COUNTDOWN_STEP);
    CHECK(ban_game_phase() == BAN_PHASE_EVENT_RESULT);
    CHECK(ban_game_risk() == 30);
    CHECK(ban_game_result_copy() == BAN_COPY_IDLE_RES);
}

static void test_china_faces(void) {
    reset_china_first();
    CHECK(ban_game_event() == BAN_EVENT_CHINA_SCAN);
    CHECK(ban_game_input(BAN_INPUT_OK));
    ban_game_tick(BAN_MS_SCAN);
    CHECK(ban_game_auto_step() == 0);
    CHECK(ban_game_face() == BAN_FACE_NORMAL);
    ban_game_tick(1333);
    CHECK(ban_game_auto_step() == 0);
    ban_game_tick(1);
    CHECK(ban_game_auto_step() == 1);
    CHECK(ban_game_face() == BAN_FACE_SUSPICIOUS);
    ban_game_tick(1333);
    CHECK(ban_game_auto_step() == 2);
    CHECK(ban_game_face() == BAN_FACE_ANGRY);
}

static void test_distribution_and_repro(void) {
    enum { N = 3000 };
    int presence[BAN_EVENT_POOL] = { 0 };
    int slot[BAN_ROUND_EVENTS][BAN_EVENT_POOL] = { { 0 } };
    int masks[1 << BAN_EVENT_POOL] = { 0 };
    int seed;
    ban_event_id_t a[BAN_ROUND_EVENTS];
    ban_event_id_t b[BAN_ROUND_EVENTS];
    int appeal_a[BAN_APPEAL_LEN];
    int prob_a;
    int i;

    ban_game_set_rng(lcg_rng);
    for (seed = 1; seed <= N; seed++) {
        ban_event_id_t round[BAN_ROUND_EVENTS];
        int mask = 0;
        int seen[BAN_EVENT_POOL] = { 0 };

        /* 连续种子的 LCG 低位相关，先混一下再当种子，否则第一抽会偏。 */
        s_lcg = (uint32_t)seed * 0x9E3779B9u;
        ban_game_reset();
        ban_game_round(round);
        CHECK(ban_game_prob() >= BAN_PROB_MIN && ban_game_prob() <= BAN_PROB_MAX);
        CHECK(ban_game_appeal_at(BAN_APPEAL_LEN - 1) == BAN_APPEAL_REJECT);
        CHECK(ban_game_appeal_rejected());
        for (i = 0; i < BAN_APPEAL_LEN; i++) {
            CHECK(ban_game_appeal_at(i) == BAN_APPEAL_RESTORE || ban_game_appeal_at(i) == BAN_APPEAL_REJECT);
        }
        for (i = 0; i < BAN_ROUND_EVENTS; i++) {
            CHECK((int)round[i] >= 0 && (int)round[i] < BAN_EVENT_POOL);
            CHECK(!seen[round[i]]);
            seen[round[i]] = 1;
            presence[round[i]]++;
            slot[i][round[i]]++;
            mask |= 1 << (int)round[i];
        }
        masks[mask]++;
        {
            int prob = ban_game_prob();
            ban_game_tick(1000);
            CHECK(ban_game_prob() == prob);
            CHECK(ban_game_phase() == BAN_PHASE_HOME);
        }
    }

    for (i = 0; i < BAN_EVENT_POOL; i++) {
        if (presence[i] <= 1650 || presence[i] >= 1950 || slot[0][i] <= 480 || slot[0][i] >= 720) {
            fprintf(stderr, "event %d presence %d slot0 %d\n", i, presence[i], slot[0][i]);
        }
        CHECK(presence[i] > 1650 && presence[i] < 1950);
        CHECK(slot[0][i] > 480 && slot[0][i] < 720);
    }
    {
        int combos = 0;
        for (i = 0; i < (1 << BAN_EVENT_POOL); i++) {
            int bits = 0;
            int b;
            for (b = 0; b < BAN_EVENT_POOL; b++) if (i & (1 << b)) bits++;
            if (bits == BAN_ROUND_EVENTS) {
                CHECK(masks[i] > 0);
                combos++;
            } else {
                CHECK(masks[i] == 0);
            }
        }
        CHECK(combos == 10);
    }

    s_lcg = 42;
    ban_game_reset();
    ban_game_round(a);
    prob_a = ban_game_prob();
    for (i = 0; i < BAN_APPEAL_LEN; i++) appeal_a[i] = (int)ban_game_appeal_at(i);
    s_lcg = 42;
    ban_game_reset();
    ban_game_round(b);
    CHECK(ban_game_prob() == prob_a);
    for (i = 0; i < BAN_ROUND_EVENTS; i++) CHECK(a[i] == b[i]);
    for (i = 0; i < BAN_APPEAL_LEN; i++) CHECK(appeal_a[i] == (int)ban_game_appeal_at(i));

    s_lcg = 42;
    ban_game_reset();
    s_lcg = 99;
    ban_game_reset();
    ban_game_round(b);
    {
        int same = 1;
        for (i = 0; i < BAN_ROUND_EVENTS; i++) if (a[i] != b[i]) same = 0;
        CHECK(!same || ban_game_prob() != prob_a);
    }
}

static void test_restart_keeps_pace(void) {
    reset_gift_email_china();
    CHECK(ban_game_input(BAN_INPUT_OK));
    ban_game_tick(BAN_MS_SCAN);
    CHECK(ban_game_input(BAN_INPUT_OK));
    ban_game_tick(BAN_MS_EVENT_RESULT);
    CHECK(ban_game_input(BAN_INPUT_OK));
    ban_game_tick(BAN_MS_EVENT_RESULT);
    ban_game_tick(BAN_MS_AUTO_EVENT + BAN_MS_EVENT_RESULT + BAN_MS_FINAL_REVIEW + BAN_MS_BAN_ANIM);
    CHECK(ban_game_phase() == BAN_PHASE_BAN);
    CHECK(ban_game_input(BAN_INPUT_OK));
    ban_game_tick(BAN_MS_APPEAL + BAN_MS_APPEAL_RESULT);
    CHECK(ban_game_phase() == BAN_PHASE_GAME_OVER);
    CHECK(ban_game_input(BAN_INPUT_OK));
    CHECK(ban_game_phase() == BAN_PHASE_SCAN);
    CHECK(ban_game_risk() == 0);
    CHECK(ban_game_event_index() == 0);
}

static uint32_t zero_rng(void) { return 0; }
static uint32_t ones_rng(void) { return 0xFFFFFFFFu; }

static void test_input_spam_and_home_idle(void) {
    ban_event_id_t before[BAN_ROUND_EVENTS];
    ban_event_id_t after[BAN_ROUND_EVENTS];
    int prob;
    int i;

    reset_gift_email_china();
    CHECK(!ban_game_input(BAN_INPUT_UP));
    CHECK(!ban_game_input(BAN_INPUT_DOWN));
    CHECK(ban_game_phase() == BAN_PHASE_HOME);
    CHECK(ban_game_input(BAN_INPUT_OK));
    CHECK(ban_game_phase() == BAN_PHASE_SCAN);
    for (i = 0; i < 30; i++) {
        CHECK(!ban_game_input(BAN_INPUT_UP));
        CHECK(!ban_game_input(BAN_INPUT_DOWN));
        CHECK(!ban_game_input(BAN_INPUT_OK));
    }
    CHECK(ban_game_phase() == BAN_PHASE_SCAN);
    CHECK(ban_game_risk() == 0);
    ban_game_tick(BAN_MS_SCAN);
    CHECK(ban_game_phase() == BAN_PHASE_EVENT);
    CHECK(ban_game_selected() == 0);
    for (i = 0; i < 8; i++) CHECK(!ban_game_input(BAN_INPUT_UP));
    CHECK(ban_game_selected() == 0);
    CHECK(ban_game_input(BAN_INPUT_DOWN));
    CHECK(ban_game_selected() == 1);
    for (i = 0; i < 8; i++) CHECK(!ban_game_input(BAN_INPUT_DOWN));
    CHECK(ban_game_selected() == 1);
    CHECK(ban_game_input(BAN_INPUT_OK));
    CHECK(ban_game_phase() == BAN_PHASE_EVENT_RESULT);
    CHECK(ban_game_risk() == 20);
    CHECK(ban_game_last_delta() == 20);
    for (i = 0; i < 8; i++) CHECK(!ban_game_input(BAN_INPUT_OK));
    CHECK(ban_game_phase() == BAN_PHASE_EVENT_RESULT);
    CHECK(ban_game_risk() == 20);

    reset_gift_email_china();
    ban_game_round(before);
    prob = ban_game_prob();
    ban_game_tick(BAN_MS_IDLE_RETURN * 3u);
    CHECK(ban_game_phase() == BAN_PHASE_HOME);
    CHECK(ban_game_prob() == prob);
    ban_game_round(after);
    for (i = 0; i < BAN_ROUND_EVENTS; i++) CHECK(before[i] == after[i]);
}

static void test_rng_extremes(void) {
    ban_event_id_t round[BAN_ROUND_EVENTS];
    int seen[BAN_EVENT_POOL];
    int restores;
    int i;

    ban_game_set_rng(zero_rng);
    ban_game_reset();
    ban_game_round(round);
    memset(seen, 0, sizeof seen);
    for (i = 0; i < BAN_ROUND_EVENTS; i++) {
        CHECK((int)round[i] >= 0 && (int)round[i] < BAN_EVENT_POOL);
        CHECK(!seen[round[i]]);
        seen[round[i]] = 1;
    }
    CHECK(ban_game_prob() >= BAN_PROB_MIN && ban_game_prob() <= BAN_PROB_MAX);
    CHECK(ban_game_appeal_at(BAN_APPEAL_LEN - 1) == BAN_APPEAL_REJECT);
    restores = 0;
    for (i = 0; i < BAN_APPEAL_LEN; i++) {
        if (ban_game_appeal_at(i) == BAN_APPEAL_RESTORE) restores++;
    }
    CHECK(restores >= 1);
    CHECK(ban_game_appeal_rejected());

    ban_game_set_rng(ones_rng);
    ban_game_reset();
    ban_game_round(round);
    memset(seen, 0, sizeof seen);
    for (i = 0; i < BAN_ROUND_EVENTS; i++) {
        CHECK(!seen[round[i]]);
        seen[round[i]] = 1;
    }
    CHECK(ban_game_appeal_at(0) == BAN_APPEAL_RESTORE);
    CHECK(ban_game_appeal_at(BAN_APPEAL_LEN - 1) == BAN_APPEAL_REJECT);
    for (i = 1; i < BAN_APPEAL_LEN - 1; i++) {
        CHECK(ban_game_appeal_at(i) == BAN_APPEAL_REJECT);
    }
    CHECK(ban_game_prob() >= BAN_PROB_MIN && ban_game_prob() <= BAN_PROB_MAX);
}

int main(void) {
    test_tables();
    test_tiers_and_cap();
    test_choice_risks_and_flow();
    test_cap_path();
    test_idle_countdown();
    test_china_faces();
    test_distribution_and_repro();
    test_restart_keeps_pace();
    test_input_spam_and_home_idle();
    test_rng_extremes();
    if (g_fails) {
        fprintf(stderr, "ban logic: %d checks failed\n", g_fails);
        return 1;
    }
    return 0;
}
