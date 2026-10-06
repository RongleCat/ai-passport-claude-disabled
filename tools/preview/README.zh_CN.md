<p align="right">
  <strong>简体中文</strong> · <a href="README.md">English</a>
</p>

# 封号检测仪主机预览

用同一份 `fw/main/ban_ui_*.c` 在 macOS 上做无头 LVGL 软件渲染，把 240×320 帧写成 PNG。不连接真机，不打开 SDL 窗口。

在 `fw/tools/preview` 下执行一条命令即可重跑。`render.sh` 使用 PATH 里的 cmake，找不到时改用 `~/.espressif/tools/cmake` 或 Homebrew。

```bash
./render.sh
```

产物在 `fw/tools/preview/out/`。`contact.png` 把每一帧按 1 倍横向拼在一起。`gallery.png` 按流程顺序拼 12 张关键页，同样是 1 倍。固定随机数序列让第 1 局和第 3 局抽到赠送额度、安全验证、中国人检测，第 2 局抽到什么都不做、VPN 瞬移、赠送额度。

只看某一目录时：

```bash
./build/ban_preview /tmp/ban-preview-out
```

脚本截图之后，同一进程再跑 200 局（偶数局用一种 LCG，奇数局用 xorshift32），并用 `lv_mem_monitor` 核对对象数、动画数和定时器数。字节容差 64（TLSF 空闲块头），块数、对象数、动画数、定时器数容差为 0。
