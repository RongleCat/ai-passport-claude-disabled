#!/usr/bin/env python3
"""Synthesize the five ban-o-meter sound effects.

Format follows fw/main/demo_audio.c (SAMPLE_RATE and bsp_audio_set_format).
The BSP demo and the hardware guide both use 16 kHz, 16-bit, mono. PCM is
emitted as const int16 arrays in fw/main/ban_sfx_data.c (Flash rodata) and as
listening copies under assets/sfx/*.wav.

Peak absolute sample is kept at or below 20000 so a later volume of 60
does not need to clip. Regeneration is deterministic: --check rebuilds the
C file in memory and compares it to disk.
"""

from __future__ import annotations

import argparse
import math
import re
import struct
import wave
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DEMO = ROOT / "main" / "demo_audio.c"
DATA_H = ROOT / "main" / "ban_sfx_data.h"
DATA_C = ROOT / "main" / "ban_sfx_data.c"
WAV_DIR = ROOT / "assets" / "sfx"

PEAK_LIMIT = 20000
CLIPS = ("click", "warning", "scan", "ban", "reject")

SINE = [int(round(math.sin(2 * math.pi * i / 256) * 32767)) for i in range(256)]


def audio_format() -> tuple[int, int, int]:
    text = DEMO.read_text(encoding="utf-8")
    rate = re.search(r"#define\s+SAMPLE_RATE\s+(\d+)", text)
    fmt = re.search(
        r"bsp_audio_set_format\(\s*SAMPLE_RATE\s*,\s*(\d+)\s*,\s*(\d+)\s*\)",
        text,
    )
    if not rate or not fmt:
        raise SystemExit(f"cannot read sample format from {DEMO}")
    hz, bits, channels = int(rate.group(1)), int(fmt.group(1)), int(fmt.group(2))
    if bits != 16 or channels != 1:
        raise SystemExit(f"unsupported BSP format {hz} Hz {bits}-bit {channels} ch")
    if hz < 8000 or hz > 48000:
        raise SystemExit(f"unexpected sample rate {hz}")
    return hz, bits, channels


def sine_at(phase: int) -> int:
    return SINE[(phase >> 24) & 255]


def step_for(hz: int, rate: int) -> int:
    return (hz << 32) // rate


def tone(rate: int, ms: int, hz0: int, hz1: int, amp: int) -> list[int]:
    total = rate * ms // 1000
    fade = max(1, rate * 4 // 1000)
    phase = 0
    out: list[int] = []
    for i in range(total):
        hz = hz0 + (hz1 - hz0) * i // max(total - 1, 1)
        phase = (phase + step_for(hz, rate)) & 0xFFFFFFFF
        env = amp
        if i < fade:
            env = env * i // fade
        elif total - i < fade:
            env = env * (total - i) // fade
        out.append(sine_at(phase) * env // 32767)
    return out


def silence(rate: int, ms: int) -> list[int]:
    return [0] * (rate * ms // 1000)


def synth_click(rate: int) -> list[int]:
    return tone(rate, 80, 2400, 1800, 9000)


def synth_warning(rate: int) -> list[int]:
    return tone(rate, 130, 880, 880, 11000) + silence(rate, 40) + tone(rate, 130, 1175, 1175, 11000)


def synth_scan(rate: int) -> list[int]:
    return tone(rate, 600, 480, 2400, 10000)


def synth_ban(rate: int) -> list[int]:
    total = rate * 800 // 1000
    noise = 0x1234567
    phase = 0
    raw: list[int] = []
    for i in range(total):
        t_ms = i * 1000 // rate
        if t_ms < 220:
            hz = 160 - (160 - 55) * t_ms // 220
        else:
            hz = 55
        phase = (phase + step_for(hz, rate)) & 0xFFFFFFFF
        # exp(-t * 5.5) approximated by a shift decay: env halves about every 126 ms.
        env = 16000
        decay_samples = i
        while decay_samples > rate * 126 // 1000 and env > 1:
            env = env * 3 // 4
            decay_samples -= rate * 126 // 1000
        sample = sine_at(phase) * env // 32767
        if t_ms < 50:
            noise = (noise * 1664525 + 1013904223) & 0xFFFFFFFF
            burst = ((noise >> 16) - 32768) * (50 - t_ms) // 50
            sample += burst * 7000 // 32768
        raw.append(sample)
    peak = max(1, max(abs(s) for s in raw))
    if peak > 16000:
        raw = [s * 16000 // peak for s in raw]
    return raw


def synth_reject(rate: int) -> list[int]:
    return tone(rate, 220, 523, 494, 11000) + silence(rate, 60) + tone(rate, 220, 349, 311, 10000)


def synthesize(rate: int) -> dict[str, list[int]]:
    clips = {
        "click": synth_click(rate),
        "warning": synth_warning(rate),
        "scan": synth_scan(rate),
        "ban": synth_ban(rate),
        "reject": synth_reject(rate),
    }
    expect = {
        "click": (1, rate // 10),
        "warning": (rate * 28 // 100, rate * 32 // 100),
        "scan": (rate * 58 // 100, rate * 62 // 100),
        "ban": (rate * 78 // 100, rate * 82 // 100),
        "reject": (rate * 48 // 100, rate * 52 // 100),
    }
    for name, samples in clips.items():
        lo, hi = expect[name]
        if not lo <= len(samples) <= hi:
            raise SystemExit(f"{name} duration {len(samples)} samples outside {lo}..{hi}")
        peak = max(abs(s) for s in samples) if samples else 0
        if peak > PEAK_LIMIT or peak == 0:
            raise SystemExit(f"{name} peak {peak} outside 1..{PEAK_LIMIT}")
        if any(s < -32768 or s > 32767 for s in samples):
            raise SystemExit(f"{name} left the int16 range")
    return clips


def c_array(name: str, samples: list[int]) -> str:
    lines = [f"static const int16_t s_{name}[] = {{"]
    row: list[str] = []
    for sample in samples:
        row.append(str(sample))
        if len(row) == 12:
            lines.append("    " + ", ".join(row) + ",")
            row = []
    if row:
        lines.append("    " + ", ".join(row) + ",")
    lines.append("};")
    return "\n".join(lines)


def render_h(rate: int, bits: int, channels: int) -> str:
    return f"""// Generated by tools/ban_sfx.py. Do not edit.
#pragma once

#include <stdint.h>

#define BAN_SFX_HZ {rate}
#define BAN_SFX_BITS {bits}
#define BAN_SFX_CHANNELS {channels}

typedef enum {{
    BAN_SFX_CLICK = 0,
    BAN_SFX_WARNING,
    BAN_SFX_SCAN,
    BAN_SFX_BAN,
    BAN_SFX_REJECT,
    BAN_SFX_COUNT
}} ban_sfx_id_t;

// Flash-resident PCM. *samples is the sample count (not bytes). Unknown ids
// return NULL and write 0.
const int16_t *ban_sfx_pcm(ban_sfx_id_t id, uint32_t *samples);
"""


def render_c(clips: dict[str, list[int]]) -> str:
    arrays = "\n\n".join(c_array(name, clips[name]) for name in CLIPS)
    rows = []
    for name in CLIPS:
        rows.append(
            f"    {{ s_{name}, (uint32_t)(sizeof(s_{name}) / sizeof(s_{name}[0])) }},"
        )
    table = "\n".join(rows)
    return f"""// Generated by tools/ban_sfx.py. Do not edit.
// const PCM stays in Flash (.rodata). Do not copy a whole clip into RAM.
#include "ban_sfx_data.h"

#include <stddef.h>

{arrays}

typedef struct {{
    const int16_t *pcm;
    uint32_t samples;
}} ban_sfx_clip_t;

static const ban_sfx_clip_t s_clips[BAN_SFX_COUNT] = {{
{table}
}};

const int16_t *ban_sfx_pcm(ban_sfx_id_t id, uint32_t *samples) {{
    if ((unsigned)id >= BAN_SFX_COUNT) {{
        if (samples) *samples = 0;
        return NULL;
    }}
    if (samples) *samples = s_clips[id].samples;
    return s_clips[id].pcm;
}}
"""


def write_wav(path: Path, rate: int, samples: list[int]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(path), "wb") as handle:
        handle.setnchannels(1)
        handle.setsampwidth(2)
        handle.setframerate(rate)
        handle.writeframes(struct.pack("<" + "h" * len(samples), *samples))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="compare outputs, do not write")
    args = parser.parse_args()
    rate, bits, channels = audio_format()
    clips = synthesize(rate)
    header = render_h(rate, bits, channels)
    source = render_c(clips)
    if args.check:
        if not DATA_H.is_file() or DATA_H.read_text(encoding="utf-8") != header:
            raise SystemExit("ban_sfx_data.h does not match the synthesizer")
        if not DATA_C.is_file() or DATA_C.read_text(encoding="utf-8") != source:
            raise SystemExit("ban_sfx_data.c does not match the synthesizer")
        for name in CLIPS:
            wav = WAV_DIR / f"{name}.wav"
            if not wav.is_file():
                raise SystemExit(f"missing {wav}")
            with wave.open(str(wav), "rb") as handle:
                if handle.getnchannels() != 1 or handle.getsampwidth() != 2 or handle.getframerate() != rate:
                    raise SystemExit(f"{wav.name} format drifted")
                raw = handle.readframes(handle.getnframes())
            got = list(struct.unpack("<" + "h" * (len(raw) // 2), raw))
            if got != clips[name]:
                raise SystemExit(f"{wav.name} does not match the synthesizer")
        total = sum(len(clips[name]) for name in CLIPS) * 2
        print(f"sfx check ok ({total} pcm bytes, {rate} Hz)")
        return

    DATA_H.write_text(header, encoding="utf-8")
    DATA_C.write_text(source, encoding="utf-8")
    for name in CLIPS:
        write_wav(WAV_DIR / f"{name}.wav", rate, clips[name])
        peak = max(abs(s) for s in clips[name])
        print(f"{name}: {len(clips[name])} samples, {len(clips[name]) * 1000 // rate} ms, peak {peak}")
    total = sum(len(clips[name]) for name in CLIPS) * 2
    print(f"pcm bytes {total}")


if __name__ == "__main__":
    main()
