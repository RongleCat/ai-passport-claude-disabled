<p align="right">
  <strong>简体中文</strong> · <a href="README.md">English</a>
</p>

# 资源

达里奥这套贴纸的源文件、本仓库可以再分发的字体，以及上游 Passport 文档还在用的产品照片。打进固件的字节在 `main/`，不在这里。图片里不要放凭证，也不要放设备二维码里的密钥。

## 达里奥（`dario/`）

为本游戏画的贴纸。`tools/ban_assets.py` 把各尺寸 PNG 打成 LVGL 9 的 RGB565A8，写进 `main/ban_assets.c`。透明通道保留 0–255。数据是 `const`，放在 flash。角色备了 160、96、48 三档，固件里不再缩放。图标 64 和 40。国旗 72×50。

| 路径 | 内容 |
| --- | --- |
| `dario/*_{160,96,48}.png` | 六个表情：normal、suspicious、angry、banned、shock、blink |
| `dario/icon_*.png` | 礼物、邮件、警告、语言、时区、微信、封禁 |
| `dario/flag_{us,jp,sg}_72.png` | VPN 瞬移用的三面旗 |
| `dario/ban_stamp_200.png`、`dario/ban_stamp_cn_200.png` | 两枚印章 |
| `dario/contact.png` | 深底、浅底两张总览。深底那一半是 `docs/assets/preview/dario-set.png` |
| `dario/master/*_1024.png` | 处理好的 1024 主图 |
| `dario/palette.json` | 写进 `main/ban_palette.h` 的颜色 |
| `dario/process.py` | 从品红底主图重出各尺寸。只依赖 Pillow |

`dario/master/candidates/` 是本地生成时的中间图，没有放进仓库。没有那些文件，`process.py` 重不出 1024 主图。`ban_assets.py` 不调用 `process.py`。克隆下来可以：

```sh
python3 tools/ban_assets.py --check
```

这些图随本仓库以 MIT 分发，许可文本在 [`../LICENSE`](../LICENSE)。没有获得 Anthropic 的授权。屏幕上的人名和产品名是各自的商标，见项目 README。

## 字体（`fonts/`）

| 文件 | 来源 | 许可 | 固件子集 |
| --- | --- | --- | --- |
| `fonts/PressStart2P-Regular.ttf` | Press Start 2P。版权 2012 The Press Start 2P Project Authors。保留字体名「Press Start 2P」 | SIL OFL 1.1，[`fonts/OFL-PressStart2P.txt`](fonts/OFL-PressStart2P.txt) | `main/ban_font_latin_8.c`，8 px，1 bpp，只收文案表里用到的 ASCII |
| Noto Sans SC Regular | 2.004。版权 2014–2021 Adobe，保留字体名「Source」。TTF 用 LVGL 测试资源里已有的那份，大约 10 MB，不在 `assets/` 再复制一份 | SIL OFL 1.1。`idf.py build` 下载 LVGL 之后，文本在 `managed_components/lvgl__lvgl/tests/src/test_files/fonts/noto/OFL.txt` | `main/ban_font_zh_12.c`、`ban_font_zh_16.c`、`ban_font_zh_24.c`，4 bpp，未压缩 |

生成出来的符号名是 `ban_font_*`，不用 Noto、Source 或 Press Start 2P，保留名留在字体软件自己身上。

```sh
python3 tools/ban_fonts.py --check
```

`main/ban_logic.c` 的文案里出现子集没有的字，这条检查会失败。重新生成需要 `npx` 和 `lv_font_conv@1.5.3`，还需要那份 Noto 文件，它要等 LVGL 组件下载下来才有。细节在 [`fonts/SOURCE.txt`](fonts/SOURCE.txt)。

没有用 Zpix。发布方不按 OFL 提供再分发。

没有做 32 px 的中文。首页标题在这个字号下放不进 240 宽的内容区。

## 音效（`sfx/`）

`click.wav`、`warning.wav`、`scan.wav`、`ban.wav`、`reject.wav`。由 `tools/ban_sfx.py` 合成，没有第三方采样。16 kHz、16 位、单声道，和 `main/demo_audio.c` 的格式一致。打进固件的 PCM 是 `main/ban_sfx_data.c`。采样绝对值不超过 20000。

```sh
python3 tools/ban_sfx.py --check
```

## 上游照片（`images/`）

FoloToy 的产品照片和字标，[`../docs/README.md`](../docs/README.md) 还在引用。不是本游戏的图。`home.jpg` 是 3840×2160。`readme-hardware-specs.png`、`logo.png` 和两张字标与上游仓库里的是同一批文件。
