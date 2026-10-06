<p align="right">
  <a href="README.zh_CN.md">简体中文</a> · <strong>English</strong>
</p>

# Assets

Source art for Dario Ban-o-meter, plus the font files this tree redistributes, plus the upstream Passport photos that `docs/README.md` still embeds. Packed firmware bytes live in `main/`, not here. Do not commit credentials or a device QR secret into an image.

## Dario (`dario/`)

Sticker drawings made for this game. `tools/ban_assets.py` packs the sized PNGs into `main/ban_assets.c` as LVGL 9 RGB565A8. Alpha stays 0-255. The bytes are `const` and stay in flash. Faces are stored at 160, 96, and 48 px so the firmware does not scale them. Icons are 64 and 40. Flags are 72 by 50.

| Path | What it is |
| --- | --- |
| `dario/*_{160,96,48}.png` | Six faces: normal, suspicious, angry, banned, shock, blink |
| `dario/icon_*.png` | Gift, mail, warning, language, time zone, WeChat, ban |
| `dario/flag_{us,jp,sg}_72.png` | VPN-hop flags |
| `dario/ban_stamp_200.png`, `dario/ban_stamp_cn_200.png` | The two stamps |
| `dario/contact.png` | Both contact sheets. The dark half is `docs/assets/preview/dario-set.png` |
| `dario/master/*_1024.png` | Clean 1024 px masters |
| `dario/palette.json` | The colors copied into `main/ban_palette.h` |
| `dario/process.py` | Rebuilds sized PNGs from magenta masters. Needs Pillow |

`dario/master/candidates/` is a local generation input and is not in this repository. `process.py` cannot rebuild the 1024 masters without it. `ban_assets.py` does not call `process.py`. From a clone:

```sh
python3 tools/ban_assets.py --check
```

The drawings are distributed with this repository under the MIT license in [`../LICENSE`](../LICENSE). They are not licensed by Anthropic. Names of people and products on the screen are other parties' trademarks. See the project README.

## Fonts (`fonts/`)

| File | Source | License | Firmware subset |
| --- | --- | --- | --- |
| `fonts/PressStart2P-Regular.ttf` | Press Start 2P, copyright 2012 The Press Start 2P Project Authors. Reserved Font Name "Press Start 2P" | SIL OFL 1.1, [`fonts/OFL-PressStart2P.txt`](fonts/OFL-PressStart2P.txt) | `main/ban_font_latin_8.c`, 8 px, 1 bpp, ASCII used by the copy table |
| Noto Sans SC Regular | Version 2.004. Copyright 2014-2021 Adobe, Reserved Font Name "Source". The TTF is the copy already vendored with LVGL for its tests, about 10 MB, and is not copied into `assets/` | SIL OFL 1.1. After `idf.py build` the text is `managed_components/lvgl__lvgl/tests/src/test_files/fonts/noto/OFL.txt` | `main/ban_font_zh_12.c`, `ban_font_zh_16.c`, `ban_font_zh_24.c`, 4 bpp, uncompressed |

Generated symbol names are `ban_font_*`, not "Noto" or "Source" or "Press Start 2P", so the reserved names stay on the font software itself.

```sh
python3 tools/ban_fonts.py --check
```

That check fails if `main/ban_logic.c` uses a character the subset does not contain. Regenerating needs `npx` and `lv_font_conv@1.5.3`, and it needs the Noto file, which appears only after the LVGL component has been downloaded. Details are in [`fonts/SOURCE.txt`](fonts/SOURCE.txt).

Zpix is not used. Its publisher does not ship it under the OFL.

There is no 32 px Chinese face. The home title does not fit the 240 px content width at that size.

## Sound (`sfx/`)

`click.wav`, `warning.wav`, `scan.wav`, `ban.wav`, `reject.wav`. Synthesized by `tools/ban_sfx.py`. No third-party samples. 16 kHz, 16-bit, mono, the same format as `main/demo_audio.c`. Packed PCM is `main/ban_sfx_data.c`. Peak absolute sample stays at or below 20000.

```sh
python3 tools/ban_sfx.py --check
```

## Upstream photos (`images/`)

These are the FoloToy product photos and wordmarks that [`../docs/README.md`](../docs/README.md) embeds. They are not the game's art. `home.jpg` is 3840 by 2160. `readme-hardware-specs.png`, `logo.png`, and the two wordmarks are the same files as in the upstream tree.
