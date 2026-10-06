// main/ban_ui.h —— 游戏画面。只在 LVGL 任务里，或持有 ban_port_lvgl_lock 时调用。
#pragma once

#include <stdbool.h>

void ban_ui_start(void);

// 页面入场位移动画是否已经结束。主机预览用来等画面停稳再截。
bool ban_ui_settled(void);
