// Host stand-in for ban_port.c. No FreeRTOS, no audio, no ADC.
#include "ban_port.h"

#include <stdio.h>

#define Q_CAP 16

static int s_q[Q_CAP];
static int s_head;
static int s_tail;
static int s_count;
static int s_sfx[BAN_SFX_COUNT];

void ban_port_init(void) {}

void ban_port_sfx(ban_sfx_id_t id) {
    if ((unsigned)id < BAN_SFX_COUNT) s_sfx[id]++;
}

bool ban_port_lvgl_lock(int timeout_ms) {
    (void)timeout_ms;
    return true;
}

void ban_port_lvgl_unlock(void) {}

void ban_port_post_key(int key) {
    if (s_count >= Q_CAP) return;
    s_q[s_tail] = key;
    s_tail = (s_tail + 1) % Q_CAP;
    s_count++;
}

bool ban_port_take_key(int *key) {
    if (!key || s_count == 0) return false;
    *key = s_q[s_head];
    s_head = (s_head + 1) % Q_CAP;
    s_count--;
    return true;
}

int ban_port_battery_soc(void) {
    return 80;
}

void ban_port_battery_poll(uint32_t now_ms) {
    (void)now_ms;
}

void ban_port_log(const char *msg) {
    printf("ban: %s\n", msg ? msg : "");
    fflush(stdout);
}

void ban_port_log_heap(const char *where) {
    printf("heap %s free=host min=host largest=host\n", where ? where : "");
    fflush(stdout);
}

void ban_port_debug_dump(void) {
    printf("sfx click=%d warning=%d scan=%d ban=%d reject=%d\n",
           s_sfx[BAN_SFX_CLICK], s_sfx[BAN_SFX_WARNING], s_sfx[BAN_SFX_SCAN],
           s_sfx[BAN_SFX_BAN], s_sfx[BAN_SFX_REJECT]);
}
