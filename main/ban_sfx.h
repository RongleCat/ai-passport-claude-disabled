// main/ban_sfx.h —— 封号检测仪音效。PCM 在 ban_sfx_data.c（Flash const）。
// ban_sfx_play() 只入队，可从按键任务调用，不得在 ISR 里调用。
#pragma once

#include "ban_sfx_data.h"

// 创建长度 1 的队列和优先级 6 的工作任务。codec 在工作任务里初始化；
// 初始化失败则进入静音：播放调用直接返回，任务继续丢弃队列项。
void ban_sfx_init(void);

// 请求播放。新的音效覆盖队列里尚未取出的项，并让正在播放的音效在下一个
// 写入块边界停下。init 之前或静音模式下为空操作。
void ban_sfx_play(ban_sfx_id_t id);
