#!/usr/bin/env python3
"""Pack sticker IP art into LVGL 9 RGB565A8 images.

Source directory is assets/dario/, relative to this repository root.
Alpha is kept (0–255) so the anti-aliased sticker edges survive.
Bytes stay const and live in flash.

--check rebuilds every output in memory and fails if disk differs.
A clone can run `python3 tools/ban_assets.py --check` with no files
outside this repository.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent.parent
IP_DIR = ROOT / "assets" / "dario"
PALETTE_JSON = IP_DIR / "palette.json"
ASSETS_H = ROOT / "main" / "ban_assets.h"
ASSETS_C = ROOT / "main" / "ban_assets.c"
PALETTE_H = ROOT / "main" / "ban_palette.h"
CATALOG_H = ROOT / "main" / "ban_asset_catalog.h"

# symbol, filename, width, height, placeholder label
FACES = ("normal", "suspicious", "angry", "banned", "shock", "blink")
FACE_SIZES = (160, 96, 48)
ICONS = ("gift", "mail", "warning", "language", "timezone", "wechat", "ban_user")
ICON_SIZES = (64, 40)
FLAGS = (("us", 72, 50), ("jp", 72, 50), ("sg", 72, 50))

FALLBACK_PALETTE = (
    ("ORANGE", 0xEE7744, "达里奥豆形主体"),
    ("ORANGE_DARK", 0xC85D36, "主体少量暗部、内轮廓"),
    ("OUTLINE", 0x171312, "角色与贴纸图标的粗描边"),
    ("HAIR", 0x35241D, "卷曲爆炸头发"),
    ("BELLY", 0xF8F2E7, "角色肚皮与浅色高光"),
    ("BLUE", 0x378BD6, "信息、语言与汗滴强调"),
    ("RED", 0xE84137, "封禁、错误、危险状态"),
    ("YELLOW", 0xF8BF39, "礼物、警告与时间强调"),
    ("GREEN", 0x3DB460, "原创对话气泡与成功状态"),
    ("CREAM", 0xF2EBD8, "米白界面底色"),
    ("INK", 0x22201C, "深色正文与辅助线条"),
    ("CRT_BG", 0x14201B, "深色终端背景"),
)


def warn(text: str) -> None:
    print(f"警告: {text}", file=sys.stderr)


def parse_hex(value: object) -> int | None:
    if not isinstance(value, str):
        return None
    text = value.strip().lstrip("#")
    if len(text) == 3:
        text = "".join(ch * 2 for ch in text)
    if len(text) != 6 or any(ch not in "0123456789abcdefABCDEF" for ch in text):
        return None
    return int(text, 16)


def macro_name(key: str) -> str:
    name = key.strip().upper().replace("-", "_").replace(" ", "_")
    if not name or not (name[0].isalpha() or name[0] == "_") or not name.replace("_", "").isalnum():
        raise SystemExit(f"palette key {key!r} is not a C identifier")
    return name


def load_palette() -> tuple[list[tuple[str, int, str]], str]:
    if not PALETTE_JSON.is_file():
        warn(f"未找到 {PALETTE_JSON.relative_to(ROOT).as_posix()}，使用脚本内的占位色")
        return list(FALLBACK_PALETTE), "fallback"
    try:
        raw = json.loads(PALETTE_JSON.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        warn(f"palette.json 无法解析 ({exc})，使用占位色")
        return list(FALLBACK_PALETTE), "fallback"
    if not isinstance(raw, dict) or not raw:
        warn("palette.json 不是对象，使用占位色")
        return list(FALLBACK_PALETTE), "fallback"
    items: list[tuple[str, int, str]] = []
    for key, value in raw.items():
        note = ""
        hex_value: object = value
        if isinstance(value, dict):
            hex_value = value.get("hex")
            use = value.get("use")
            note = use if isinstance(use, str) else ""
        parsed = parse_hex(hex_value)
        if parsed is None:
            warn(f"palette.json 的 {key} 没有合法 hex，跳过")
            continue
        items.append((macro_name(str(key)), parsed, note))
    if not items:
        warn("palette.json 没有可用颜色，使用占位色")
        return list(FALLBACK_PALETTE), "fallback"
    return items, PALETTE_JSON.relative_to(ROOT).as_posix()


def asset_rows() -> list[tuple[str, str, int, int, str]]:
    rows: list[tuple[str, str, int, int, str]] = []
    for face in FACES:
        for size in FACE_SIZES:
            symbol = f"dario_{face}_{size}"
            rows.append((symbol, f"{symbol}.png", size, size, face[:4].upper()))
    for icon in ICONS:
        for size in ICON_SIZES:
            symbol = f"icon_{icon}_{size}"
            rows.append((symbol, f"{symbol}.png", size, size, icon[:4].upper()))
    for code, width, height in FLAGS:
        symbol = f"flag_{code}_72"
        rows.append((symbol, f"{symbol}.png", width, height, code.upper()))
    rows.append(("ban_stamp_200", "ban_stamp_200.png", 200, 72, "BAN"))
    rows.append(("ban_stamp_cn_200", "ban_stamp_cn_200.png", 200, 72, "FENG"))
    return rows


def rgb_of(value: int) -> tuple[int, int, int, int]:
    return ((value >> 16) & 255, (value >> 8) & 255, value & 255, 255)


def color_map(items: list[tuple[str, int, str]]) -> dict[str, int]:
    return {name: value for name, value, _note in items}


def placeholder(w: int, h: int, label: str, colors: dict[str, int]) -> Image.Image:
    cream = rgb_of(colors.get("CREAM", 0xF2EBD8))
    ink = rgb_of(colors.get("INK", 0x22201C))
    orange = rgb_of(colors.get("ORANGE", 0xEE7744))
    image = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    draw.rounded_rectangle((1, 1, w - 2, h - 2), radius=max(4, min(w, h) // 8), fill=orange, outline=ink, width=2)
    draw.text((4, h // 2 - 4), label[:4], fill=cream)
    return image


def load_image(filename: str, w: int, h: int, label: str, colors: dict[str, int]) -> tuple[Image.Image, str]:
    path = IP_DIR / filename
    if not path.is_file():
        warn(f"缺少 {path.relative_to(ROOT).as_posix()}，使用圆角占位图")
        return placeholder(w, h, label, colors), "placeholder"
    image = Image.open(path).convert("RGBA")
    if image.size != (w, h):
        warn(f"{path.name} 尺寸 {image.size[0]}x{image.size[1]}，按 Lanczos 缩放到 {w}x{h}")
        image = image.resize((w, h), Image.Resampling.LANCZOS)
    return image, path.relative_to(ROOT).as_posix()


def rgb565(r: int, g: int, b: int) -> int:
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def to_rgb565a8(image: Image.Image) -> tuple[bytes, int]:
    width, height = image.size
    if width % 2:
        raise SystemExit(f"width {width} must be even so RGB565 stride stays aligned")
    rgb = bytearray(width * 2 * height)
    alpha = bytearray(width * height)
    pixels = image.tobytes()
    partial = 0
    for i in range(width * height):
        r, g, b, a = pixels[i * 4 : i * 4 + 4]
        if a == 0:
            r = g = b = 0
        elif a < 255:
            partial += 1
        value = rgb565(r, g, b)
        rgb[i * 2] = value & 0xFF
        rgb[i * 2 + 1] = (value >> 8) & 0xFF
        alpha[i] = a
    return bytes(rgb) + bytes(alpha), partial


def fmt_bytes(data: bytes) -> str:
    lines = []
    for i in range(0, len(data), 16):
        chunk = data[i : i + 16]
        lines.append("    " + ", ".join(f"0x{b:02x}" for b in chunk) + ",")
    return "\n".join(lines)


def c_comment(text: str) -> str:
    return text.replace("*/", "* /").replace("\n", " ")


def render_palette(items: list[tuple[str, int, str]], source: str) -> str:
    lines = [
        "// Generated by tools/ban_assets.py. Do not edit.",
        f"// source: {source}",
        "#pragma once",
        "#include <stdint.h>",
        "",
    ]
    for name, value, note in items:
        suffix = f" /* {c_comment(note)} */" if note else ""
        lines.append(f"#define BAN_PAL_{name} 0x{value:06X}u{suffix}")
    lines.append("")
    lines.append("static inline uint8_t ban_pal_r(uint32_t rgb) { return (uint8_t)((rgb >> 16) & 0xFFu); }")
    lines.append("static inline uint8_t ban_pal_g(uint32_t rgb) { return (uint8_t)((rgb >> 8) & 0xFFu); }")
    lines.append("static inline uint8_t ban_pal_b(uint32_t rgb) { return (uint8_t)(rgb & 0xFFu); }")
    lines.append("")
    return "\n".join(lines)


def render_catalog(packed: list[tuple[str, int, int, str, bytes, int]]) -> str:
    lines = [
        "// Generated by tools/ban_assets.py. Do not edit.",
        "// Host test catalog. Firmware does not include this header.",
        "#pragma once",
        "",
        "typedef struct {",
        "    const char *name;",
        "    int w;",
        "    int h;",
        "    int partial;",
        "} ban_asset_info_t;",
        "",
        "static const ban_asset_info_t ban_asset_catalog[] = {",
    ]
    for name, w, h, _origin, _data, partial in packed:
        lines.append(f'    {{"{name}", {w}, {h}, {1 if partial else 0}}},')
    lines.append("};")
    lines.append("")
    lines.append(f"#define BAN_ASSET_CATALOG_COUNT {len(packed)}")
    lines.append("")
    return "\n".join(lines)


def render_assets(packed: list[tuple[str, int, int, str, bytes, int]]) -> tuple[str, str]:
    decls = [
        "// Generated by tools/ban_assets.py. Do not edit.",
        "#pragma once",
        '#include "lvgl.h"',
        "",
    ]
    maps = [
        "// Generated by tools/ban_assets.py. Do not edit.",
        "// RGB565 plane, then A8 mask. Alpha stays 0-255 (anti-aliased edges).",
        "// const data is placed in flash (.rodata) and is not copied into RAM.",
        '#include "ban_assets.h"',
        "",
        '_Static_assert(LV_DRAW_BUF_STRIDE_ALIGN == 1, "ban assets assume stride align 1");',
        "",
    ]
    for name, w, h, origin, data, _partial in packed:
        symbol = f"ban_img_{name}"
        decls.append(f"extern const lv_image_dsc_t {symbol};")
        stride = w * 2
        size = stride * h + w * h
        if len(data) != size:
            raise SystemExit(f"{name} packed {len(data)} bytes, expected {size}")
        maps.append(f"/* source: {origin} */")
        maps.append(f"static const uint8_t {symbol}_map[] = {{")
        maps.append(fmt_bytes(data))
        maps.append("};")
        maps.append(f'_Static_assert(sizeof({symbol}_map) == {size}u, "{name} size");')
        maps.append(f"const lv_image_dsc_t {symbol} = {{")
        maps.append("    .header = {")
        maps.append("        .magic = LV_IMAGE_HEADER_MAGIC,")
        maps.append("        .cf = LV_COLOR_FORMAT_RGB565A8,")
        maps.append(f"        .w = {w},")
        maps.append(f"        .h = {h},")
        maps.append(f"        .stride = {stride},")
        maps.append("    },")
        maps.append(f"    .data_size = sizeof({symbol}_map),")
        maps.append(f"    .data = {symbol}_map,")
        maps.append("};")
        maps.append("")
    decls.append("")
    return "\n".join(decls), "\n".join(maps)


def build() -> tuple[str, str, str, str, int, int]:
    palette, source = load_palette()
    colors = color_map(palette)
    packed = []
    total = 0
    partial_px = 0
    for name, filename, w, h, label in asset_rows():
        image, origin = load_image(filename, w, h, label, colors)
        data, partial = to_rgb565a8(image)
        packed.append((name, w, h, origin, data, partial))
        total += len(data)
        partial_px += partial
    header, source_c = render_assets(packed)
    return render_palette(palette, source), header, source_c, render_catalog(packed), total, partial_px


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="compare outputs, do not write")
    args = parser.parse_args()
    palette_h, assets_h, assets_c, catalog_h, total, partial = build()
    files = (
        (PALETTE_H, palette_h),
        (ASSETS_H, assets_h),
        (ASSETS_C, assets_c),
        (CATALOG_H, catalog_h),
    )
    if args.check:
        for path, text in files:
            if not path.is_file() or path.read_text(encoding="utf-8") != text:
                raise SystemExit(f"{path.name} does not match the current sources")
        print(f"asset check ok ({total} image bytes, partial alpha samples {partial})")
        return
    for path, text in files:
        path.write_text(text, encoding="utf-8")
    print(f"image bytes {total}")
    print(f"partial alpha samples {partial}")


if __name__ == "__main__":
    main()
