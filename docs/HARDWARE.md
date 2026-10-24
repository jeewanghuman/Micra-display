# Hardware

The board this fork runs on, in detail. The short version lives in the
README's [Hardware](../README.md#supported-hardware) section.

## Waveshare ESP32‑P4‑WIFI6‑Touch‑LCD‑3.4C

[Product page](https://www.waveshare.com/esp32-p4-wifi6-touch-lcd-3.4c.htm) ·
[Waveshare wiki](https://www.waveshare.com/wiki/ESP32-P4-WIFI6-Touch-LCD-3.4C)

| | |
|---|---|
| **Display** | 3.4" round IPS, 800×800, MIPI‑DSI, behind 115 mm round glass. Capacitive touch (GT9271). Dimmable backlight. |
| **Processor** | ESP32‑P4 with 32 MB flash and 32 MB PSRAM. |
| **Radio** | Wi‑Fi 6 and Bluetooth LE through an on‑board ESP32‑C6. Tondo keeps the C6's firmware matched to its own, so the first boot after flashing may restart once while it brings the C6 up to date. |
| **Audio** | ES8311 codec and a speaker amplifier — the warm‑up chime and button clicks. |
| **Storage** | microSD slot (use a FAT32 card) — shot history and settings backup. |
| **Header** | 40‑pin, laid out like a Raspberry Pi's. The paddle wiring uses three of its pins — see the [wiring guide](WIRING.md). |
| **Power** | USB‑C. No battery circuit. |

The 3.4C is electronically Waveshare's rectangular ESP32‑P4‑WIFI6‑Touch‑LCD‑4.3
(the board upstream Apollo 2 supports) behind round glass: same processor,
radio, audio and SD slot, a different panel.

### Chip revision

ESP32‑P4 chips come in two generations that need different firmware images:
the earlier **rev v1.x** and the newer **rev v3**. Waveshare doesn't change the
product name between them, so check yours with `esptool chip-id` before the
first flash. The published image (and the web flasher) is for rev v1.x —
the unit this port was developed on is a v1.3. A rev v3 board builds
with `make flash-p4-3-4c-rev3`; that image exists but hasn't been tested on
hardware. Flashing the wrong one does no harm: the screen stays dark until
the right image is flashed.

## Power and the clock

The board runs from **USB‑C power**; a phone charger and a cable are all it
needs. There is no battery circuit, so Tondo shows no battery indicator.

There's no clock chip either. With **Wi‑Fi + NTP** (Settings → Tondo → WiFi)
the clock sets itself on every boot. Without Wi‑Fi, Tondo restores the time
it last saved (it saves hourly) when it powers up — late by however long the
power was off — so set it under **Settings → Tondo → Time & date** after a
long power‑off. Shot history needs a real date to record new shots.

The connector Waveshare labels **RTC** backs the processor's low‑power domain;
Tondo doesn't rely on it.

## Scales

A supported Bluetooth scale (Bookoo Themis, Acaia Umbra / Lunar / Prochef /
Pyxis, or Varia Aku — Pyxis and Aku untested) is optional but unlocks the shot
timer, flow graph, and brew‑by‑weight features.

## Paddle wiring (Auto shot)

Optional: without it you still get the full brew‑by‑weight experience via
**Shot detect**, and only the automatic stop at target weight needs the wire.
The 3.4C needs an external opto‑isolator module, like upstream's P4 boards.
Step‑by‑step instructions (with photos of the Micra's paddle loom) are in the
**[wiring guide](WIRING.md)**.
