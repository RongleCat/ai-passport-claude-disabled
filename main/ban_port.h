// main/ban_port.h —— 固件 / 主机预览共用的薄端口。
// UI 只走这里，不直接调用 BSP、FreeRTOS 或音效任务。
#pragma once

#include "ban_sfx_data.h"

#include <stdbool.h>
#include <stdint.h>

// 设备：音效任务、按键回调、esp_random。主机：空操作。
void ban_port_init(void);

// 可从按键回调或 LVGL 任务调用。只入队，不阻塞。
void ban_port_sfx(ban_sfx_id_t id);

// 非 LVGL 任务访问对象时使用。LVGL 任务内（定时器回调）不必再加锁。
bool ban_port_lvgl_lock(int timeout_ms);
void ban_port_lvgl_unlock(void);

// 0 上，1 下，2 确认。与 ban_input_t 的顺序一致。
void ban_port_post_key(int key);
bool ban_port_take_key(int *key);

// 电量 0..100。不可用时返回 -1，UI 隐藏图标。
int ban_port_battery_soc(void);

// 最多每 3 秒读一次电量计。从 LVGL 定时器调用，不要每帧调用。
void ban_port_battery_poll(uint32_t now_ms);

void ban_port_log(const char *msg);
void ban_port_log_heap(const char *where);
void ban_port_debug_dump(void);
