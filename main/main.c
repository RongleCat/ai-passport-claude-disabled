// main/main.c —— 直接进入达里奥封号检测仪。不再打开 demo 菜单。
#include "ban_port.h"
#include "ban_ui.h"
#include "bsp_display.h"
#include "bsp_i2c.h"
#include "bsp_pins.h"

#include "esp_log.h"

static const char *TAG = "main";

void app_main(void) {
    ESP_LOGI(TAG, "达里奥封号检测仪启动");
    bsp_i2c_init();
    if (bsp_display_init() != ESP_OK || !bsp_lvgl_init()) {
        ESP_LOGE(TAG, "显示初始化失败 MOSI=%d SCLK=%d CS=%d DC=%d BL=%d",
                 BSP_LCD_MOSI, BSP_LCD_SCLK, BSP_LCD_CS, BSP_LCD_DC, BSP_LCD_BL);
        return;
    }
    bsp_display_backlight(100);
    ban_port_init();
    if (!ban_port_lvgl_lock(1000)) {
        ESP_LOGE(TAG, "LVGL 锁超时，游戏没有启动");
        return;
    }
    ban_ui_start();
    ban_port_lvgl_unlock();
}
