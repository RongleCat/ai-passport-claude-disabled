#!/usr/bin/env python3
"""Build LVGL subsets for the ban-o-meter copy table.

Chinese sizes come from Noto Sans SC Regular (SIL OFL 1.1), the face already
vendored under managed_components for LVGL. Zpix is the sharper pixel face,
but its publisher sells a commercial license and does not grant OFL-style
redistribution, so it is not embedded.

Latin letters and digits use Press Start 2P (SIL OFL 1.1) at 8 px, 1 bpp.
Chinese sizes are 12 / 16 / 24 px at 4 bpp so the outline stays readable.
"""

from __future__ import annotations

import argparse
import re
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LOGIC = ROOT / "main" / "ban_logic.c"
NOTO = (
    ROOT
    / "managed_components"
    / "lvgl__lvgl"
    / "tests"
    / "src"
    / "test_files"
    / "fonts"
    / "noto"
    / "NotoSansSC-Regular.ttf"
)
LATIN = ROOT / "assets" / "fonts" / "PressStart2P-Regular.ttf"
CHARSET_H = ROOT / "main" / "ban_font_charset.h"
CONV = ["npx", "--yes", "lv_font_conv@1.5.3"]

FONTS = (
    # name, source, size, bpp, extra args, include_all
    ("ban_font_zh_12", NOTO, 12, 4, [], True),
    ("ban_font_zh_16", NOTO, 16, 4, [], True),
    ("ban_font_zh_24", NOTO, 24, 4, [], True),
    ("ban_font_latin_8", LATIN, 8, 1, ["--autohint-off", "--no-kerning"], False),
)

STRING_RE = re.compile(r'"((?:\\.|[^"\\])*)"')
UPLUS_RE = re.compile(r"U\+([0-9A-Fa-f]+)")
TINY_RE = re.compile(
    r"\.range_start = (\d+), \.range_length = (\d+), \.glyph_id_start = \d+,\s*"
    r"\.unicode_list = NULL, \.glyph_id_ofs_list = NULL, \.list_length = 0, "
    r"\.type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY"
)


def unescape(text: str) -> str:
    return (
        text.replace(r"\\", "\\")
        .replace(r"\"", '"')
        .replace(r"\n", "\n")
        .replace(r"\t", "\t")
    )


def copy_strings() -> list[str]:
    text = LOGIC.read_text(encoding="utf-8")
    begin = text.find("BAN_STRINGS_BEGIN")
    end = text.find("BAN_STRINGS_END")
    if begin < 0 or end < begin:
        raise SystemExit("ban_logic.c is missing BAN_STRINGS_BEGIN/END")
    body = text[begin:end]
    return [unescape(part) for part in STRING_RE.findall(body)]


def codepoints(strings: list[str]) -> list[int]:
    found: set[int] = set()
    for item in strings:
        found.update(ord(ch) for ch in item)
    found.discard(0)
    return sorted(cp for cp in found if cp >= 0x20)


def read_u16(data: bytes, offset: int) -> int:
    return struct.unpack_from(">H", data, offset)[0]


def read_u32(data: bytes, offset: int) -> int:
    return struct.unpack_from(">I", data, offset)[0]


def table(data: bytes, tag: bytes) -> tuple[int, int]:
    count = read_u16(data, 4)
    for i in range(count):
        base = 12 + i * 16
        if data[base : base + 4] == tag:
            return read_u32(data, base + 8), read_u32(data, base + 12)
    raise SystemExit(f"{tag!r} table missing")


def cmap_codepoints(path: Path) -> set[int]:
    data = path.read_bytes()
    start, _length = table(data, b"cmap")
    subtables = read_u16(data, start + 2)
    found: set[int] = set()
    for i in range(subtables):
        rec = start + 4 + i * 8
        offset = start + read_u32(data, rec + 4)
        fmt = read_u16(data, offset)
        if fmt == 4:
            found.update(cmap_format4(data, offset))
        elif fmt == 12:
            found.update(cmap_format12(data, offset))
    if not found:
        raise SystemExit(f"no cmap glyphs in {path}")
    return found


def cmap_format4(data: bytes, offset: int) -> set[int]:
    seg_count = read_u16(data, offset + 6) // 2
    end_base = offset + 14
    start_base = end_base + 2 * seg_count + 2
    delta_base = start_base + 2 * seg_count
    offset_base = delta_base + 2 * seg_count
    found: set[int] = set()
    for i in range(seg_count):
        end = read_u16(data, end_base + 2 * i)
        start = read_u16(data, start_base + 2 * i)
        delta = struct.unpack_from(">h", data, delta_base + 2 * i)[0]
        range_off = read_u16(data, offset_base + 2 * i)
        if start == 0xFFFF:
            continue
        for cp in range(start, end + 1):
            if range_off == 0:
                gid = (cp + delta) & 0xFFFF
            else:
                pos = offset_base + 2 * i + range_off + 2 * (cp - start)
                gid = read_u16(data, pos)
                if gid:
                    gid = (gid + delta) & 0xFFFF
            if gid:
                found.add(cp)
    return found


def cmap_format12(data: bytes, offset: int) -> set[int]:
    groups = read_u32(data, offset + 12)
    found: set[int] = set()
    for i in range(groups):
        base = offset + 16 + i * 12
        start = read_u32(data, base)
        end = read_u32(data, base + 4)
        gid = read_u32(data, base + 8)
        for cp in range(start, end + 1):
            if gid + (cp - start) != 0:
                found.add(cp)
    return found


def symbols_for(cps: list[int], ascii_only: bool) -> str:
    chosen = [cp for cp in cps if (cp < 0x80) or not ascii_only]
    if ascii_only:
        chosen = [cp for cp in cps if cp < 0x80]
    return "".join(chr(cp) for cp in chosen)


def font_path(name: str) -> Path:
    return ROOT / "main" / f"{name}.c"


def covered_in_generated(path: Path) -> set[int]:
    text = path.read_text(encoding="utf-8", errors="replace")
    found = {int(item, 16) for item in UPLUS_RE.findall(text)}
    for start, length in TINY_RE.findall(text):
        base = int(start)
        found.update(range(base, base + int(length)))
    return found


def relativize_generated(path: Path) -> None:
    """lv_font_conv records the absolute --font and --output paths."""
    text = path.read_text(encoding="utf-8")
    root = str(ROOT.resolve())
    text = text.replace(root + "/", "")
    path.write_text(text, encoding="utf-8")


def write_charset(cps: list[int]) -> None:
    lines = [
        "// Generated by tools/ban_fonts.py. Do not edit.",
        "#pragma once",
        "#include <stdint.h>",
        f"#define BAN_FONT_CODEPOINT_COUNT {len(cps)}",
        "static const uint32_t ban_font_codepoints[BAN_FONT_CODEPOINT_COUNT] = {",
    ]
    row: list[str] = []
    for cp in cps:
        row.append(f"0x{cp:04X}")
        if len(row) == 8:
            lines.append("    " + ", ".join(row) + ",")
            row = []
    if row:
        lines.append("    " + ", ".join(row) + ",")
    lines.append("};")
    lines.append("")
    CHARSET_H.write_text("\n".join(lines), encoding="utf-8")


def generate(cps: list[int]) -> None:
    for name, source, size, bpp, extra, full in FONTS:
        if not source.is_file():
            raise SystemExit(f"missing font file: {source}")
        needed = cps if full else [cp for cp in cps if cp < 0x80]
        have = cmap_codepoints(source)
        missing = [cp for cp in needed if cp not in have]
        if missing:
            shown = " ".join(f"U+{cp:04X}" for cp in missing[:12])
            raise SystemExit(f"{source.name} missing {len(missing)} glyphs: {shown}")
        out = font_path(name)
        cmd = [
            *CONV,
            "--font",
            str(source),
            "--symbols",
            symbols_for(cps, not full),
            "--size",
            str(size),
            "--bpp",
            str(bpp),
            "--format",
            "lvgl",
            "--no-compress",
            "--lv-include",
            "lvgl.h",
            "--lv-font-name",
            name,
            "--output",
            str(out),
            *extra,
        ]
        print(" ".join(cmd[:6]), f"... {name}")
        subprocess.run(cmd, check=True)
        relativize_generated(out)
        got = covered_in_generated(out)
        still = [cp for cp in needed if cp not in got]
        if still:
            shown = " ".join(f"U+{cp:04X}" for cp in still[:12])
            raise SystemExit(f"{name} generated without {shown}")
        print(f"{name}: {len(needed)} glyphs, {out.stat().st_size} bytes source")
    write_charset(cps)


def check(cps: list[int]) -> None:
    if not CHARSET_H.is_file():
        raise SystemExit("missing ban_font_charset.h; run without --check")
    header = CHARSET_H.read_text(encoding="utf-8")
    listed = [int(item, 16) for item in re.findall(r"0x([0-9A-Fa-f]+)", header)]
    if listed != cps:
        raise SystemExit("ban_font_charset.h does not match ban_logic.c strings")
    for name, _source, _size, _bpp, _extra, full in FONTS:
        path = font_path(name)
        if not path.is_file():
            raise SystemExit(f"missing {path.name}")
        got = covered_in_generated(path)
        needed = cps if full else [cp for cp in cps if cp < 0x80]
        missing = [cp for cp in needed if cp not in got]
        if missing:
            shown = " ".join(f"U+{cp:04X}" for cp in missing[:12])
            raise SystemExit(f"{name} missing {shown}")
    print(f"font check ok ({len(cps)} code points)")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="verify generated fonts, do not rebuild")
    args = parser.parse_args()
    cps = codepoints(copy_strings())
    if not cps:
        raise SystemExit("no copy-table characters")
    if args.check:
        check(cps)
    else:
        generate(cps)


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as exc:
        sys.exit(exc.returncode)
