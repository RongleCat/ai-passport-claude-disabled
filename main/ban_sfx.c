// main/ban_sfx.c —— 见 ban_sfx.h。
// PCM 留在 Flash，按 256 个采样分块交给 bsp_audio_write()。驱动会拷进 DMA，
// 这里不再把整段音效复制到 RAM。
#include "ban_sfx.h"

#include "bsp_audio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include <stddef.h>

static const char *TAG = "ban_sfx";

#define BAN_SFX_CHUNK      256
#define BAN_SFX_VOLUME     60
#define BAN_SFX_TASK_STACK 3072
#define BAN_SFX_TASK_PRIO  6

typedef struct {
    uint8_t id;
    uint32_t gen;
} ban_sfx_cmd_t;

static QueueHandle_t s_queue;
static TaskHandle_t s_task;
static volatile uint32_t s_gen;
static volatile bool s_muted;

static void play_clip(const ban_sfx_cmd_t *cmd) {
    uint32_t samples = 0;
    const int16_t *pcm = ban_sfx_pcm((ban_sfx_id_t)cmd->id, &samples);
    uint32_t done = 0;

    if (!pcm || samples == 0) return;
    while (done < samples) {
        uint32_t n = samples - done;
        if (cmd->gen != s_gen) return;
        if (n > BAN_SFX_CHUNK) n = BAN_SFX_CHUNK;
        if (bsp_audio_write(pcm + done, n * sizeof(int16_t)) != ESP_OK) {
            ESP_LOGW(TAG, "write failed at %u", (unsigned)done);
            return;
        }
        done += n;
    }
}

static void ban_sfx_task(void *arg) {
    ban_sfx_cmd_t cmd;
    esp_err_t err;

    (void)arg;
    err = bsp_audio_init();
    if (err == ESP_OK) {
        err = bsp_audio_set_format(BAN_SFX_HZ, BAN_SFX_BITS, BAN_SFX_CHANNELS);
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "audio init failed: %s (muted)", esp_err_to_name(err));
        s_muted = true;
        for (;;) {
            (void)xQueueReceive(s_queue, &cmd, portMAX_DELAY);
        }
    }
    bsp_audio_set_volume(BAN_SFX_VOLUME);

    for (;;) {
        if (xQueueReceive(s_queue, &cmd, portMAX_DELAY) != pdTRUE) continue;
        if (cmd.gen != s_gen) continue;
        if (cmd.id >= BAN_SFX_COUNT) continue;
        play_clip(&cmd);
    }
}

void ban_sfx_init(void) {
    if (s_task) return;
    s_queue = xQueueCreate(1, sizeof(ban_sfx_cmd_t));
    if (!s_queue) {
        s_muted = true;
        ESP_LOGE(TAG, "queue alloc failed (muted)");
        return;
    }
    if (xTaskCreate(ban_sfx_task, "ban_sfx", BAN_SFX_TASK_STACK, NULL,
                    BAN_SFX_TASK_PRIO, &s_task) != pdPASS) {
        vQueueDelete(s_queue);
        s_queue = NULL;
        s_muted = true;
        ESP_LOGE(TAG, "task create failed (muted)");
    }
}

void ban_sfx_play(ban_sfx_id_t id) {
    ban_sfx_cmd_t cmd;

    if (!s_queue || s_muted || id >= BAN_SFX_COUNT) return;
    cmd.id = (uint8_t)id;
    cmd.gen = ++s_gen;
    (void)xQueueOverwrite(s_queue, &cmd);
}
