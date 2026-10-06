<p align="right">
  <a href="game-design.zh_CN.md">简体中文</a> · <strong>English</strong>
</p>

# Dario Ban-o-meter

Short design note for the firmware in this repository. The original spec, including every on-screen line, is [`game-design.zh_CN.md`](game-design.zh_CN.md). That file is kept as written. Where it still says "pixel art" or lists four faces, the firmware has moved on. The rules below are what `main/ban_logic.h` and `main/ban_logic.c` do.

## What it is

A satirical half-minute game on the FoloToy AI Passport. Three physical keys. One screen at a time. No network, no account, no real region check. The joke is that every choice raises the ban risk, the review still ends in a ban, and the appeal is a slot machine whose last cell is reject.

Unofficial. Not affiliated with Anthropic or FoloToy.

The English line stored for the disclaimer is: "Satirical game based on public information and community discussions." The Chinese line is in the Chinese design note and in `ban_disclaimer_full()`.

## Round

```text
home
  OK
scan (2.5 s; the alert line takes the last 0.5 s)
event 1
event 2
event 3
final review (2 s)
ban stamp (0.8 s)
ban page (waits for OK)
appeal reel (3.2 s, 8 cells, last cell is always reject)
appeal result (1.8 s)
game over
```

Three events are drawn from the pool of five, without replacement. A choice event waits for OK. An auto event runs for 4 seconds, then shows its result for 2.2 seconds. After the third result the final review starts even if the total is low. The displayed risk never goes above 100.

On the game-over page, OK starts another scan. Twenty-five seconds with no key returns to the home page and deals a new round. The ban page does not time out. The home page stays up.

## Events

| Event | Kind | Risk |
|---|---|---|
| Gift credit | choice | take +30, refuse +20 |
| Security mail | choice | open the link +30, ignore +25 |
| China check | auto | +40. Scripted language, GMT+8, then WeChat |
| VPN hop | auto | +25. Flags: US, then JP, then SG |
| Do nothing | auto | a 3-2-1 countdown, then +30 |

There is no safe choice. The China-check rows are animation. The firmware does not read IP, language, time zone, installed apps, or identity.

## Risk

| Risk | Name on the meter | Color |
|---|---|---|
| 0-29 | SAFE | green |
| 30-59 | SUSPICIOUS | yellow |
| 60-79 | HIGH RISK | orange |
| 80-99 | DARIO IS WATCHING | red |
| 100 | BANNED | red |

The ban page shows a "Chinese person" probability from 81 to 99. It is a random integer for the joke. The stated reason is the fixed suspicious line.

## Keys

UP and DOWN move the highlight on the two choice events. OK starts a round from home, confirms a choice, opens the appeal from the ban page, and starts the next scan from game over. Other phases ignore the keys.

## Faces

The shipped art is a sticker drawing, packed from `assets/dario/` at 160, 96, and 48 px. It is not scaled at runtime.

| Face | When it shows |
|---|---|
| normal | home, scan, and risk under 30. This face blinks |
| suspicious | risk from 30 through 70, and most event results |
| angry | risk above 70. The sprite shakes by a couple of pixels |
| banned | the second half of the final review, the stamp, the ban page, the appeal, and game over |
| shock | about 420 ms when a result adds 40 or more, and the first time the meter enters 80-99 |
| blink | a short stand-in for normal. Not a separate mood |

## What the firmware does not do

- No Wi-Fi, no microphone recording, no account login.
- No full-screen photo, no video.
- Pages are created and destroyed. The LVGL heap in this tree is 48 KB.
- Chinese glyphs are a subset of Noto Sans SC. Latin letters and digits are Press Start 2P. See [`../assets/README.md`](../assets/README.md).
