<p align="right">
  <a href="README.zh_CN.md">简体中文</a> · <strong>English</strong>
</p>

# Ban-o-meter host preview

Headless LVGL software renderer for the same `fw/main/ban_ui_*.c` sources. It writes 240 by 320 frames as PNG on macOS. It does not open a device port and it does not open an SDL window.

From `fw/tools/preview`, one command rebuilds and reruns. `render.sh` uses `cmake` on `PATH`. If that is missing it falls back to `~/.espressif/tools/cmake` or Homebrew.

```bash
./render.sh
```

Frames land in `fw/tools/preview/out/`. `contact.png` places every frame side by side at 1x. `gallery.png` is 12 key pages in flow order, also at 1x. A fixed RNG fixture makes rounds 1 and 3 draw the gift, mail, and China checks. Round 2 draws the idle countdown, the VPN hop, and the gift check.

To render into another directory:

```bash
./build/ban_preview /tmp/ban-preview-out
```

After the scripted shots, the same process plays 200 more rounds (even games use one LCG, odd games use xorshift32) and checks `lv_mem_monitor` plus object, animation, and timer counts. The checker allows 64 bytes of TLSF block-header slack and zero change in block count, object count, running animations, and timers.
