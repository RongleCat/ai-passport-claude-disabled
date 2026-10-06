#!/usr/bin/env python3
"""Build Dario's transparent sticker assets from selected magenta masters.

Run: python3 assets/dario/process.py
Requires Pillow only. Paths are relative to this file.

The published tree ships master/*_1024.png and the sized PNGs next to this
script. master/candidates/ is a local generation input and is not in the
repository. Without those candidate files this script cannot rebuild the
1024 masters. tools/ban_assets.py does not need them: it packs the sized PNGs.
"""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import math

ROOT = Path(__file__).resolve().parent
CANDIDATES = ROOT / "master" / "candidates"
MAGENTA = (255, 0, 255)
INK = (23, 19, 18)
ORANGE = (238, 119, 68)
CREAM = (248, 242, 231)
RED = (232, 65, 55)
BLUE = (55, 139, 214)
YELLOW = (248, 191, 57)
GREEN = (61, 180, 96)


def font(size, bold=True, cjk=False):
    if cjk:
        cjk_font = "/System/Library/Fonts/STHeiti Medium.ttc"
        if Path(cjk_font).exists():
            return ImageFont.truetype(cjk_font, size)
    candidates = [
        "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
    ]
    for name in candidates:
        if Path(name).exists():
            return ImageFont.truetype(name, size)
    return ImageFont.load_default()


def alpha_chroma_key(im):
    """Remove the dedicated magenta ground without leaving a colored border."""
    im = im.convert("RGBA")
    px = im.load()
    for y in range(im.height):
        for x in range(im.width):
            r, g, b, a = px[x, y]
            dist = math.sqrt((r - 255) ** 2 + g * g + (b - 255) ** 2)
            # The art contains no purple: remove the background and the blended
            # magenta fringe before downsampling produces fresh clean AA edges.
            if dist < 155:
                # Clear RGB as well as alpha: non-premultiplied resamplers would
                # otherwise leak invisible magenta RGB into semi-transparent AA.
                px[x, y] = (0, 0, 0, 0)
    return im


def clear_resample_fringe(im):
    """Purge the one-pixel low-alpha magenta halos introduced by Lanczos."""
    im = im.convert("RGBA")
    px = im.load()
    for y in range(im.height):
        for x in range(im.width):
            r, g, b, a = px[x, y]
            if a and r > 230 and b > 220 and g < 35:
                px[x, y] = (0, 0, 0, 0)
    return im


def master(source, name):
    im = Image.open(source)
    # The generator returns 1254 px square even for an explicit 1024 request.
    # Resizing the complete canvas preserves matching registration across states.
    im = clear_resample_fringe(alpha_chroma_key(im).resize((1024, 1024), Image.Resampling.LANCZOS))
    out = ROOT / "master" / f"{name}_1024.png"
    im.save(out)
    for size in (160, 96, 48):
        clear_resample_fringe(im.resize((size, size), Image.Resampling.LANCZOS)).save(ROOT / f"{name}_{size}.png")
    return im


def blink_from(normal):
    """Closed-eye animation frame: normal is unchanged except inside lenses."""
    im = normal.copy()
    d = ImageDraw.Draw(im)
    # Coordinates are proportional to the selected normal's 1024 registration.
    # Cream masks remove eyeballs while preserving frames and every other detail.
    d.ellipse((324, 312, 441, 397), fill=ORANGE)
    d.ellipse((524, 312, 642, 397), fill=ORANGE)
    w = 13
    d.arc((329, 335, 438, 394), 198, 342, fill=INK, width=w)
    d.arc((529, 335, 639, 394), 198, 342, fill=INK, width=w)
    out = ROOT / "master" / "dario_blink_1024.png"
    im.save(out)
    for size in (160, 96, 48):
        clear_resample_fringe(im.resize((size, size), Image.Resampling.LANCZOS)).save(ROOT / f"dario_blink_{size}.png")
    return im


def canvas(size=256):
    return Image.new("RGBA", (size, size), (0, 0, 0, 0))


def line(draw, pts, fill=INK, width=16, joint="curve"):
    draw.line(pts, fill=fill, width=width, joint=joint)


def round_rect(draw, box, radius, fill, outline=INK, width=14):
    draw.rounded_rectangle(box, radius, fill=fill, outline=outline, width=width)


def icon(kind):
    im = canvas(); d = ImageDraw.Draw(im)
    if kind == "gift":
        round_rect(d, (38, 88, 218, 218), 18, YELLOW)
        round_rect(d, (31, 65, 225, 112), 14, RED)
        d.rectangle((112, 65, 145, 218), fill=RED, outline=INK, width=10)
        d.ellipse((54, 25, 123, 88), fill=RED, outline=INK, width=13)
        d.ellipse((133, 25, 202, 88), fill=RED, outline=INK, width=13)
    elif kind == "mail":
        round_rect(d, (25, 53, 231, 204), 24, BLUE)
        line(d, [(38, 72), (128, 142), (218, 72)], width=15)
        line(d, [(38, 190), (103, 126)], width=13)
        line(d, [(218, 190), (153, 126)], width=13)
    elif kind == "warning":
        d.polygon([(128, 23), (235, 214), (21, 214)], fill=YELLOW, outline=INK, width=15)
        line(d, [(128, 78), (128, 145)], width=18)
        d.ellipse((117, 166, 139, 188), fill=INK)
    elif kind == "language":
        d.ellipse((27, 27, 229, 229), fill=BLUE, outline=INK, width=14)
        d.ellipse((79, 27, 177, 229), outline=INK, width=12)
        line(d, [(29, 128), (227, 128)], width=12)
        line(d, [(47, 76), (209, 76)], width=10)
        line(d, [(47, 180), (209, 180)], width=10)
        d.ellipse((116, 22, 140, 46), fill=YELLOW, outline=INK, width=6)
    elif kind == "timezone":
        d.ellipse((26, 26, 230, 230), fill=YELLOW, outline=INK, width=14)
        d.ellipse((109, 109, 147, 147), fill=CREAM, outline=INK, width=8)
        line(d, [(128, 128), (128, 67)], width=15)
        line(d, [(128, 128), (177, 150)], width=15)
    elif kind == "wechat":
        d.ellipse((24, 42, 196, 183), fill=GREEN, outline=INK, width=14)
        d.polygon([(74, 169), (67, 213), (112, 177)], fill=GREEN, outline=INK)
        d.ellipse((130, 104, 234, 198), fill=GREEN, outline=INK, width=14)
        d.polygon([(183, 186), (190, 223), (154, 192)], fill=GREEN, outline=INK)
        for x, y in [(78, 96), (139, 96), (160, 140), (202, 140)]:
            d.ellipse((x-9, y-9, x+9, y+9), fill=INK)
    elif kind == "ban_user":
        d.ellipse((24, 24, 232, 232), fill=CREAM, outline=INK, width=14)
        d.ellipse((91, 59, 164, 132), fill=ORANGE, outline=INK, width=11)
        d.arc((62, 112, 193, 212), 192, 348, fill=ORANGE, width=33)
        d.ellipse((62, 112, 193, 212), outline=INK, width=11)
        line(d, [(52, 53), (204, 205)], fill=RED, width=25)
        line(d, [(52, 53), (204, 205)], fill=INK, width=8)
    else:
        raise ValueError(kind)
    return im


def flags():
    assets = {}
    for country in ("us", "jp", "sg"):
        im = Image.new("RGBA", (360, 250), (0, 0, 0, 0)); d = ImageDraw.Draw(im)
        round_rect(d, (12, 18, 348, 232), 30, CREAM, width=18)
        mask = Image.new("L", im.size, 0); md = ImageDraw.Draw(mask)
        md.rounded_rectangle((24, 30, 336, 220), 19, fill=255)
        flag = Image.new("RGBA", im.size, (0, 0, 0, 0)); f = ImageDraw.Draw(flag)
        if country == "us":
            for i in range(7): f.rectangle((24, 30 + i*29, 336, 30 + (i+1)*29), fill=(190, 44, 52) if i % 2 == 0 else CREAM)
            f.rectangle((24, 30, 164, 132), fill=(45, 91, 169))
            for yy in (53, 80, 107):
                for xx in range(47, 150, 28): f.ellipse((xx-5, yy-5, xx+5, yy+5), fill=CREAM)
        elif country == "jp":
            f.rectangle((24, 30, 336, 220), fill=CREAM)
            f.ellipse((122, 70, 238, 186), fill=(203, 48, 58))
        else:
            f.rectangle((24, 30, 336, 125), fill=(220, 55, 63))
            f.rectangle((24, 125, 336, 220), fill=CREAM)
            f.ellipse((61, 49, 138, 126), fill=CREAM)
            f.ellipse((79, 49, 156, 126), fill=(220, 55, 63))
            for angle in (-90, -18, 54, 126, 198):
                cx, cy = 143, 88; r1, r2 = 18, 7
                pts=[]
                for a, r in [(angle, r1),(angle+36, r2),(angle+72, r1),(angle+108, r2),(angle+144, r1),(angle+180, r2),(angle+216, r1),(angle+252, r2),(angle+288, r1),(angle+324, r2)]:
                    pts.append((cx+math.cos(math.radians(a))*r, cy+math.sin(math.radians(a))*r))
                f.polygon(pts, fill=CREAM)
        im.alpha_composite(Image.composite(flag, Image.new("RGBA", im.size), mask))
        d = ImageDraw.Draw(im); d.rounded_rectangle((12, 18, 348, 232), 30, outline=INK, width=18)
        assets[country] = im.resize((72, 50), Image.Resampling.LANCZOS)
    return assets


def stamp(chinese=False):
    im = Image.new("RGBA", (800, 288), (0, 0, 0, 0)); d = ImageDraw.Draw(im)
    color = (209, 49, 45, 255)
    d.rounded_rectangle((36, 36, 764, 252), 28, outline=color, width=18)
    d.rounded_rectangle((61, 61, 739, 227), 18, outline=color, width=10)
    text = "封号" if chinese else "BANNED"
    f = font(184 if chinese else 112, cjk=chinese)
    box = d.textbbox((0, 0), text, font=f)
    x = (800 - (box[2]-box[0])) / 2
    y = (288 - (box[3]-box[1])) / 2 - 13
    d.text((x, y), text, fill=color, font=f, stroke_width=3 if chinese else 2, stroke_fill=color)
    # Sparse transparent worn flecks, staying well clear of legibility.
    for x, y in [(94,82),(188,205),(321,73),(626,200),(712,101),(538,234)]:
        d.rectangle((x, y, x+16, y+4), fill=(0,0,0,0))
    # The slight physical tilt gives both stamps a hand-applied sticker feeling.
    im = im.rotate(-4, resample=Image.Resampling.BICUBIC, expand=False)
    return im.resize((200, 72), Image.Resampling.LANCZOS)


def contact_sheet():
    names = ["dario_normal", "dario_suspicious", "dario_angry", "dario_banned", "dario_shock", "dario_blink"]
    w, h = 1400, 1030
    sheet = Image.new("RGBA", (w, h), CREAM + (255,))
    d = ImageDraw.Draw(sheet)
    d.rectangle((0, 0, w//2, h), fill=(20, 32, 27, 255))
    for i, name in enumerate(names):
        col, row = i % 3, i // 3
        x = 35 + col * 225 + (0 if col < 3 else 0)
        # Place one set on dark, mirrored on cream.
        im = Image.open(ROOT / f"{name}_160.png")
        y = 105 + row * 385
        sheet.alpha_composite(im, (x, y))
        sheet.alpha_composite(im, (x + 700, y))
        d.text((x, y+170), name.replace("dario_", ""), fill=CREAM, font=font(23))
        d.text((x+700, y+170), name.replace("dario_", ""), fill=INK, font=font(23))
    # Bottom row: all small UI assets on both backgrounds.
    icons = ["gift","mail","warning","language","timezone","wechat","ban_user"]
    for j, name in enumerate(icons):
        im = Image.open(ROOT / f"icon_{name}_40.png")
        x = 28 + j * 92
        sheet.alpha_composite(im, (x, 805))
        sheet.alpha_composite(im, (x+700, 805))
    for j, country in enumerate(("us", "jp", "sg")):
        im = Image.open(ROOT / f"flag_{country}_72.png")
        x = 48 + j * 100
        sheet.alpha_composite(im, (x, 875))
        sheet.alpha_composite(im, (x+700, 875))
    for j, name in enumerate(("ban_stamp_200.png", "ban_stamp_cn_200.png")):
        im = Image.open(ROOT / name)
        x = 260 + j * 200
        sheet.alpha_composite(im, (x, 940))
        sheet.alpha_composite(im, (x+700, 940))
    sheet.save(ROOT / "contact.png")


def preview():
    im = Image.new("RGBA", (240, 320), (20, 32, 27, 255))
    normal = Image.open(ROOT / "dario_normal_160.png")
    im.alpha_composite(normal, (40, 31))
    for i, name in enumerate(("gift", "warning", "ban_user")):
        im.alpha_composite(Image.open(ROOT / f"icon_{name}_40.png"), (42 + i*58, 247))
    im.save(ROOT / "preview_240x320.png")


def main():
    selected = {
        "dario_normal": CANDIDATES / "dario_normal_round2.png",
        "dario_suspicious": CANDIDATES / "dario_suspicious_round1.png",
        "dario_angry": CANDIDATES / "dario_angry_round1.png",
        "dario_banned": CANDIDATES / "dario_banned_round1.png",
        "dario_shock": CANDIDATES / "dario_shock_round1.png",
    }
    masters = {name: master(path, name) for name, path in selected.items()}
    blink_from(masters["dario_normal"])
    for name in ("gift", "mail", "warning", "language", "timezone", "wechat", "ban_user"):
        original = icon(name)
        original.resize((64, 64), Image.Resampling.LANCZOS).save(ROOT / f"icon_{name}_64.png")
        original.resize((40, 40), Image.Resampling.LANCZOS).save(ROOT / f"icon_{name}_40.png")
    for country, flag in flags().items(): flag.save(ROOT / f"flag_{country}_72.png")
    stamp(False).save(ROOT / "ban_stamp_200.png")
    stamp(True).save(ROOT / "ban_stamp_cn_200.png")
    contact_sheet(); preview()


if __name__ == "__main__":
    main()
