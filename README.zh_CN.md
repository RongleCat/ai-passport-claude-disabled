<p align="right">
  <a href="README.md">English</a> · <strong>简体中文</strong>
</p>

<h1 align="center">达里奥封号检测仪</h1>

<p align="center">DARIO BAN-O-METER™ · 看看你能在 Claude 活多久</p>

<p align="center">
  <img src="https://img.shields.io/badge/ESP32--C3-AI%20Passport-E7352C" alt="ESP32-C3" />
  <img src="https://img.shields.io/badge/ESP--IDF-5.5.3-E7352C" alt="ESP-IDF 5.5.3" />
  <img src="https://img.shields.io/badge/LVGL-9.5-1D4ED8" alt="LVGL 9.5" />
  <img src="https://img.shields.io/badge/license-MIT-16a34a" alt="MIT" />
  <a href="https://x.com/cgnot996"><img src="https://img.shields.io/badge/X-铁柱AGI%20%40cgnot996-black?logo=x&logoColor=white" alt="X 铁柱AGI" /></a>
  <img src="https://img.shields.io/badge/微信公众号-铁柱AGI-07C160?logo=wechat&logoColor=white" alt="微信公众号 铁柱AGI" />
</p>

跑在 [FoloToy AI Passport](https://github.com/FoloToy/ai-passport) 上的一局讽刺小游戏。右侧三个键，屏幕 240×320，大约半分钟。先扫描，再从五个事件里抽三个，然后盖章、封号、申诉。申诉最后一格固定是拒绝。屏幕上的检测全是写好的动画，不读网络、语言、时区、已装应用或账号。

> **非官方。** 与 Anthropic、FoloToy 无关。刷入会换掉设备上现有的固件。还想留着原来的镜像，请先按下文备份。风险自负。

板级支持来自 FoloToy 的 MIT 固件。本游戏的界面、规则、图和音效都在这个仓库里，使用同一份 MIT 许可（[`LICENSE`](LICENSE)）。玩法全文在 [`docs/game-design.zh_CN.md`](docs/game-design.zh_CN.md)，英文摘要在 [`docs/game-design.md`](docs/game-design.md)。

## 画面

<p align="center">
  <img src="docs/assets/preview/gallery.png" alt="十二张关键页面的主机预览，六列两行。不是真机拍照。" />
</p>

<p align="center">首页、扫描、赠送额度、领取结果、中国人检测、检测结果、最终审查、印章、封号页、申诉滚动、申诉失败、结束页。</p>

<p align="center">
  <img src="docs/assets/preview/play.gif" alt="一局脚本对局的主机预览动图。不是真机拍照。" width="480" />
</p>

<p align="center">
  <img src="docs/assets/preview/dario-set.png" alt="深色底上的六个达里奥表情、图标、国旗和印章。" width="420" />
</p>

<p align="center"><strong>这些是主机预览渲染</strong>，来自 <code>tools/preview</code>，不是 Passport 的实拍。动图用的是预览里固定的一局（赠送额度、安全验证、中国人检测）的关键帧，停顿是为了能看清，不是按固件时钟录的。真机屏幕有圆角，这些帧是直角的 240×320。</p>

## 玩法

| | |
|---|---|
| 首页 | **OK** 开始。放着不动就停在首页 |
| 扫描 | 一共 2.5 秒，最后约 0.5 秒换成「发现可疑信号」。IP、设备、语言、身份那几行是写死的 |
| 三个事件 | 从下面五个里抽，同一局不重复 |
| 最终审查 | 2 秒，然后印章 0.8 秒。风险是多少都会走到这一步 |
| 封号页 | 中国人概率在 81 到 99 之间，理由是固定的那句。停在这里等 **OK** |
| 申诉 | 3.2 秒，八格。最后一格固定是拒绝。结果再停 1.8 秒 |
| 结束页 | **OK** 直接再开一局扫描。25 秒没有按键就回到首页 |

| 事件 | 操作 | 风险 |
|---|---|---|
| 赠送额度 | 领取，或不要 | +30 或 +20 |
| 安全验证 | 点击验证，或忽略 | +30 或 +25 |
| 中国人检测 | 不用按。依次是中文、GMT+8、WeChat | +40 |
| VPN 瞬移 | 不用按。旗的顺序是美国、日本、新加坡 | +25 |
| 什么都不做 | 不用按。倒计时 3、2、1 | +30 |

没有安全选项。风险显示到 100 为止。自动事件跑 4 秒，结果再停 2.2 秒。二选一的事件会等你按。

## 风险档

| 风险 | 屏幕上的字 | 颜色 |
|---|---|---|
| 0–29 | SAFE | 绿 |
| 30–59 | SUSPICIOUS | 黄 |
| 60–79 | HIGH RISK | 橙 |
| 80–99 | DARIO IS WATCHING | 红 |
| 100 | BANNED | 红 |

## 按键

三个键在右侧，共用 GPIO0 上的一组 ADC 分压。

| 键 | |
|---|---|
| **UP** | 光标上移。只有赠送额度和安全验证这两页用得到 |
| **DOWN** | 光标下移。同上 |
| **OK** | 首页：开始。二选一：确认。封号页：进入申诉。结束页：再开一局扫描 |

别的页面不吃按键。复位前先把键松开。GPIO0 也是芯片的启动脚，复位时按住会进下载模式。

## 角色

六张贴纸，打包了 160、96、48 三档，固件里不再缩放。源文件在 [`assets/dario/`](assets/README.zh_CN.md)。

| 表情 | 用在 |
|---|---|
| normal | 首页、扫描、风险低于 30。这张会眨眼 |
| suspicious | 风险 30 到 70，以及大多数事件结果 |
| angry | 风险高于 70。角色左右晃两个像素 |
| banned | 最终审查的后半、印章、封号页、申诉、结束页 |
| shock | 单次结果加了 40 或以上时大约 420 毫秒；风险第一次进入 80–99 时也闪一下 |
| blink | 正常表情的短眨眼，不是单独的一种心情 |

## 硬件

| | |
|---|---|
| 芯片 | ESP32-C3，单核 160 MHz，约 400 KB SRAM，**无 PSRAM** |
| Flash | 8 MB 内置，DIO |
| 屏幕 | 240×320 ST7789P3，SPI，圆角 |
| 音频 | ES8311 编解码器，麦克风和扬声器走 I2S。本游戏只播放烧在 flash 里的五声短音，不录音 |
| 按键 | 右侧 UP、DOWN、OK，共用 GPIO0 上的一组 ADC 分压 |
| 电池 | CW2017 电量计，500 mAh |
| USB | 芯片自带的 USB Serial/JTAG（macOS 为 `/dev/cu.usbmodem*`，Linux 为 `/dev/ttyACM*`） |

引脚、屏幕初始化参数和按键电压窗口来自 FoloToy 的 BSP（`components/bsp/include/bsp_pins.h`）。更细的硬件说明在 [`docs/hardware-design/AI_HARDWARE_DEVELOPMENT_GUIDE.zh_CN.md`](docs/hardware-design/AI_HARDWARE_DEVELOPMENT_GUIDE.zh_CN.md)。

## 准备

- 一块 FoloToy AI Passport，以及一根能传数据的 USB 线。
- macOS 或 Linux。
- [ESP-IDF v5.5.3](https://docs.espressif.com/projects/esp-idf/en/v5.5.3/esp32c3/get-started/index.html)，并安装 `esp32c3` 工具链。不支持其他版本。不要用 Muse 那份固件的 ESP-IDF 6.0.1 环境来编这个仓库。

安装工具链（只需一次）：

```sh
git clone -b v5.5.3 --recursive https://github.com/espressif/esp-idf.git ~/esp/esp-idf-v5.5.3
~/esp/esp-idf-v5.5.3/install.sh esp32c3
. ~/esp/esp-idf-v5.5.3/export.sh
```

每开一个新终端都要再执行一次 `export.sh`。`idf.py --version` 必须打印 `v5.5.3`。

拉下这份固件：

```sh
git clone https://github.com/RongleCat/ai-passport-claude-disabled.git
cd ai-passport-claude-disabled
```

## 编译

这个游戏没有 Wi-Fi 凭证，也没有密钥头文件。不要把 SSID 或密码写进 `sdkconfig.defaults`，也不要写进任何会跟踪的文件。

```sh
. ~/esp/esp-idf-v5.5.3/export.sh
idf.py set-target esp32c3
idf.py build
```

新克隆下来的目录必须先 `idf.py set-target esp32c3`。之后的增量编译可以跳过。第一次编译会按 `components/bsp/idf_component.yml` 把 LVGL 9.5.0 等依赖下载到 `managed_components/`，这个目录被 git 忽略。`dependencies.lock` 把 LVGL 钉在 9.5.0。不要改里面下载来的源码。`sdkconfig` 是生成文件，同样被忽略。仓库里跟踪的是 `sdkconfig.defaults`。

不连设备、也不用先加载 IDF 的主机检查：

```sh
./tools/validate.sh --static
```

`./tools/validate.sh --firmware` 会另开一次编译，并打出一个合并镜像，给归档检查用。不要刷那个合并镜像。把它写到 `0x0` 会盖掉 NVS。

## 刷入

三段镜像，三个地址。bootloader `0x0`，分区表 `0x8000`，应用 `0x10000`。这就是 `build/flash_args` 里的内容（`dio`、80 MHz、`--flash-size 8MB`）。NVS 在 `0x9000`，PHY 数据在 `0xf000`，这次写入不覆盖它们。同一套分区再刷一次，这两块还在。

**不要加 `--erase-all`，不要运行 `idf.py erase-flash`。** 那两步都会清掉 NVS。

打包这棵树的机器上，ESP-IDF 5.5.3 自带的是 esptool.py v4.12.0。它的帮助文本用下划线：`--before default_reset`、`--after hard_reset`、`write_flash`、`--flash_mode`、`--flash_freq`、`--flash_size`。esptool v5 把这些改成了连字符：`--before default-reset`、`--after hard-reset`、`write-flash`、`--flash-mode`、`--flash-freq`、`--flash-size`。你本机 `python -m esptool version` 打印哪一版，就用哪一套拼法。这里没有跑过 v5。

先松开全部按键。先开机。Passport 关机时不会出现在 USB 设备里。查端口：macOS 用 `ls /dev/cu.usb*`，Linux 用 `ls /dev/ttyACM*`。下面的端口只是例子。

### a. 从 Release 下载三个 bin

GitHub Release 发出去之后，里面是这三个 bin 和 `SHA256SUMS`。git 仓库本身不带这些文件。`dist/` 被 git 忽略，是本地编完、准备打 Release 的人把文件拷进去的目录。名字和地址：

| 文件 | 地址 |
|---|---|
| `dario-ban-o-meter-bootloader.bin` | `0x0` |
| `dario-ban-o-meter-partition-table.bin` | `0x8000` |
| `dario-ban-o-meter-app.bin` | `0x10000` |

esptool v4，在放着这三个文件的目录里：

```sh
python -m esptool --chip esp32c3 -p /dev/cu.usbmodem1101 -b 460800 \
  --before default_reset --after hard_reset \
  write_flash --flash_mode dio --flash_freq 80m --flash_size 8MB \
  0x0 dario-ban-o-meter-bootloader.bin \
  0x8000 dario-ban-o-meter-partition-table.bin \
  0x10000 dario-ban-o-meter-app.bin
```

esptool v5，地址和文件相同：

```sh
python -m esptool --chip esp32c3 -p /dev/cu.usbmodem1101 -b 460800 \
  --before default-reset --after hard-reset \
  write-flash --flash-mode dio --flash-freq 80m --flash-size 8MB \
  0x0 dario-ban-o-meter-bootloader.bin \
  0x8000 dario-ban-o-meter-partition-table.bin \
  0x10000 dario-ban-o-meter-app.bin
```

`dist/flash.sh` 是 v4 这条命令，第一个参数是端口。每一段都要看到 `Hash of data verified`。

### b. 自己编译再刷

在 `build` 目录里，v4 直接吃 ESP-IDF 5.5.3 写出的 `flash_args`：

```sh
cd build
python -m esptool --chip esp32c3 -p /dev/cu.usbmodem1101 -b 460800 \
  --before default_reset --after hard_reset \
  write_flash @flash_args
```

ESP-IDF 5.5.3 上，`idf.py -p /dev/cu.usbmodem1101 flash` 写入的是同样的三段。Muse 那份说明里不让用 `idf.py flash`，是因为 ESP-IDF 6.0.1 会把 `SERIAL_TOOL` 拆开。那条只适用于 6.0.1，不适用于本仓库。整片擦除仍然不要做。

### c. 备份，以及写回去

备份文件不要放进 git。v4：

```sh
python -m esptool --chip esp32c3 -p /dev/cu.usbmodem1101 read_flash 0 0x800000 ai-passport-backup.bin
```

把备份写回去，v4：

```sh
python -m esptool --chip esp32c3 -p /dev/cu.usbmodem1101 write_flash 0 ai-passport-backup.bin
```

v5 把这两条写成 `read-flash` 和 `write-flash`：

```sh
python -m esptool --chip esp32c3 -p /dev/cu.usbmodem1101 read-flash 0 0x800000 ai-passport-backup.bin
python -m esptool --chip esp32c3 -p /dev/cu.usbmodem1101 write-flash 0 ai-passport-backup.bin
```

打开串口经常会让板子复位。看日志放在刷完之后：

```sh
. ~/esp/esp-idf-v5.5.3/export.sh
idf.py -p /dev/cu.usbmodem1101 monitor
```

用 `Ctrl-]` 退出。

## 注意事项

- 不要改 `partitions.csv`，也不要改上面三个刷写地址。换回别的固件，需要你自己事先保存的整片备份，从 `0x0` 整片写回。
- 如果这块板上次刷的是另一套分区，例如 Muse 移植（应用在 `0x20000`），需要先整片擦除一次，这个镜像才能启动。擦除会清掉 NVS。之后恢复分段写入，不要每次都擦。
- 没有 PSRAM。短音是 flash 里的采样，不是语音合成。本工程的 LVGL 堆是 48 KB。
- 刷机找不到端口时，先松开全部按键再试。仍然没有，按住按键、点一下复位、再松开，让芯片进入下载模式。
- 不要用 v5.5.3 以外的 ESP-IDF 来编这个仓库。同一块硬件上的 Muse 固件用的是 v6.0.1。两个环境不要共一个终端。
- 上面的刷写命令对过 `build/flash_args` 和 esptool.py v4.12.0 的帮助文本。这次发布检查没有在真机上执行。

## 项目结构

| 路径 | |
|---|---|
| `main/ban_logic.c` | 规则、文案、随机数。不依赖 ESP-IDF，也不依赖 LVGL |
| `main/ban_ui*.c` | 各页界面 |
| `main/ban_port.c` | 按键、节拍、LVGL 锁 |
| `main/ban_assets.c` | 打好包的图 |
| `main/ban_font_*.c` | 字体子集 |
| `main/ban_sfx*.c` | 五声短音 |
| `main/main.c` | 直接进游戏。不再打开上游的 demo 菜单 |
| `main/demo_*.c`、`main/ui_pixel.c` | 上游 demo 的源文件。没有写进 `main/CMakeLists.txt`，不会链进固件 |
| `tools/ban_assets.py`、`tools/ban_fonts.py`、`tools/ban_sfx.py` | 重新生成，以及 `--check` |
| `tools/preview/` | 主机预览 |
| `tools/validate.sh` | 静态检查，另有一条固件检查 |
| `tests/` | 静态脚本会编译的主机测试 |
| `assets/dario/`、`assets/sfx/`、`assets/fonts/` | 源文件和许可。见 [`assets/README.zh_CN.md`](assets/README.zh_CN.md) |
| `components/bsp/` | 上游板级支持 |
| `docs/game-design.zh_CN.md` | 原设计稿 |

## 主机预览

不连真机，也不开 SDL 窗口。在 `tools/preview` 下：

```sh
./render.sh
```

帧写到 `tools/preview/out/`，这个目录被 git 忽略。本 README 里的图是 `docs/assets/preview/` 下整理过的那几张。脚本还会查什么、怎么渲染到别的目录，写在 [`tools/preview/README.zh_CN.md`](tools/preview/README.zh_CN.md)。

## 隐私与内容说明

扫描、语言、时区、WeChat、三面旗，以及那个概率，是一段固定动画加上 81 到 99 里的一个随机整数。固件不读 IP、GPS、Wi-Fi、系统语言、时区、输入法、已装应用、账号、邮箱、设备标识或浏览记录，也不联网。

设备里的两句：

> 本作品为讽刺游戏，不代表真实账号风控结果。

> Satirical game based on public information and community discussions.

中文那句是 `ban_disclaimer_full()` 的返回值。英文那句是 `ban_disclaimer_en()` 的返回值。

## 关注作者

<p align="center">
  <a href="https://x.com/cgnot996"><img src="https://img.shields.io/badge/X-铁柱AGI%20%40cgnot996-black?logo=x&logoColor=white" alt="X 铁柱AGI" /></a>
  <img src="https://img.shields.io/badge/微信公众号-铁柱AGI-07C160?logo=wechat&logoColor=white" alt="微信公众号 铁柱AGI" />
</p>

<p align="center">
  <a href="https://x.com/cgnot996">关注 X @cgnot996</a>
  ·
  微信搜一搜「铁柱AGI」，或扫下面的码关注公众号
</p>

<p align="center">
  <img src="docs/assets/wechat/mp-search-scan.png" alt="微信搜一搜 铁柱AGI，扫码关注公众号" width="420" />
</p>

## 许可

基线固件和 BSP 归 FoloToy，使用 MIT 许可（[`LICENSE`](LICENSE)，版权 2026 FoloToy）。本仓库里的游戏使用同一许可（版权 2026 RongleCat）。两行都在 `LICENSE` 里，原来的 FoloToy 那行没有删。

| 路径 | 来源 | 许可 |
|---|---|---|
| `components/bsp/`、上游 demo 源文件 | FoloToy AI Passport | MIT |
| `main/ban_*`、`tools/ban_*`、`tools/preview/`、`assets/dario/`、`assets/sfx/` | 本游戏 | 随本仓库以 MIT 分发 |
| `assets/fonts/PressStart2P-Regular.ttf`、`main/ban_font_latin_8.c` | Press Start 2P | SIL OFL 1.1 |
| `main/ban_font_zh_12.c`、`ban_font_zh_16.c`、`ban_font_zh_24.c` | Noto Sans SC Regular 的子集 | SIL OFL 1.1。完整字库是 LVGL 自带的那份，没有再复制进 `assets/` |

字体文件、保留名和生成命令在 [`assets/README.zh_CN.md`](assets/README.zh_CN.md)。编译时下载的依赖保留各自的许可。

FoloToy 和 AI Passport 归 FoloToy 所有。Claude、Anthropic 及相关名称归 Anthropic 所有。本项目未获任何一方认可或背书。
