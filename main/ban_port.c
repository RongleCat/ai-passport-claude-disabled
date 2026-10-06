// main/ban_port.c —— 设备端口。按键回调只入队。
#include "ban_port.h"

#include "ban_logic.h"
#include "ban_sfx.h"
#include "bsp_battery.h"
#include "bsp_button.h"
#include "bsp_display.h"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

static const char *TAG = "ban";

#define BAN_KEY_QUEUE 8

static QueueHandle_t s_keys;
static volatile bool s_keys_ready;
static int s_soc = -1;
static uint32_t s_soc_ms;

static uint32_t device_rng(void) {
    return esp_random();
}

// 单击才算一次输入。PRESS 再加 CLICK 会把同一次按下送进游戏两次。
static void on_key(bsp_btn_t btn, bsp_btn_ev_t ev, void *user) {
    int key;

    (void)user;
    if (!s_keys_ready || ev != BSP_BTN_CLICK) return;
    if (btn == BSP_BTN_UP) key = 0;
    else if (btn == BSP_BTN_DOWN) key = 1;
    else if (btn == BSP_BTN_OK) key = 2;
    else return;
    (void)xQueueSend(s_keys, &key, 0);
}

void ban_port_init(void) {
    ban_game_set_rng(device_rng);
    ban_sfx_init();
    s_keys = xQueueCreate(BAN_KEY_QUEUE, sizeof(int));
    if (!s_keys) {
        ESP_LOGE(TAG, "key queue alloc failed");
        return;
    }
    if (bsp_button_init(on_key, NULL) != ESP_OK) {
        ESP_LOGE(TAG, "button init failed; game will not see keys");
        return;
    }
    s_keys_ready = true;
    if (bsp_battery_init() != ESP_OK) {
        ESP_LOGW(TAG, "battery init failed; icon hidden");
    }
}

void ban_port_sfx(ban_sfx_id_t id) {
    ban_sfx_play(id);
}

bool ban_port_lvgl_lock(int timeout_ms) {
    return bsp_lvgl_lock(timeout_ms);
}

void ban_port_lvgl_unlock(void) {
    bsp_lvgl_unlock();
}

void ban_port_post_key(int key) {
    if (!s_keys) return;
    (void)xQueueSend(s_keys, &key, 0);
}

bool ban_port_take_key(int *key) {
    if (!s_keys || !key) return false;
    return xQueueReceive(s_keys, key, 0) == pdTRUE;
}

int ban_port_battery_soc(void) {
    return s_soc;
}

// LVGL 定时器里低频读 2 字节。不要每帧读 I2C。
void ban_port_battery_poll(uint32_t now_ms) {
    if (s_soc_ms != 0 && now_ms - s_soc_ms < 3000u) return;
    s_soc_ms = now_ms;
    s_soc = bsp_battery_soc();
}

void ban_port_log(const char *msg) {
    ESP_LOGI(TAG, "%s", msg ? msg : "");
}

void ban_port_log_heap(const char *where) {
    ESP_LOGI(TAG, "heap %s free=%u min=%u largest=%u",
             where ? where : "",
             (unsigned)esp_get_free_heap_size(),
             (unsigned)heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
}

void ban_port_debug_dump(void) {
}
