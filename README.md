# cros-plus

**Unofficial** fork of [CrOS](https://thecrowbox.com/wiki/doku.php?id=kit:cros_software), the firmware of the
[CrowBox](https://www.thecrowbox.com/) 3rd Generation (a coin-operated feeder that teaches crows and magpies
to trade coins for peanuts). It runs the box on an **ESP32 over WiFi**, reports every visit to a host and
lets an outdoor **IP camera stream the box live** to YouTube, with the stream and the log running in Docker
on a NAS. Stock **Arduino Uno** hardware keeps working too.

Not affiliated with the CrowBox project. Upstream publishes CrOS as a zip on its wiki; the first commit here
is that zip, unmodified, so `git log` shows exactly what changed.

```
 outdoors                                            indoors (24/7, e.g. Synology NAS)
┌──────────────────────────┐                        ┌───────────────────────────────┐
│ CrowBox + ESP32 (CrOS)   │◀──WiFi, TCP 7777─────▶│ feeder: events.jsonl,         │
│ perch, coin sensor, servo│   EVT … / CMD …        │ snapshot per coin, commands   │
└──────────────────────────┘                        │                               │
 IP camera (RTSP, H.264) ───────WiFi / PoE─────────▶│ stream: RTSP → YouTube Live   │──▶ YouTube
                                                    └───────────────────────────────┘
```

## What's different from stock CrOS v1.0

| | |
|---|---|
| **ESP32 + WiFi** | the same firmware builds for an ESP32 DevKit and talks to the host over TCP; nothing but power has to reach the box ([docs/esp32.md](docs/esp32.md)) |
| **Reports events** | one line per event: `EVT LAND`, `LEAVE`, `COIN`, `FEED`, `CLOSE`, `PHASE <n>`, `ALIVE <uptime>` |
| **Takes commands** | `CMD STATUS`, `CMD PHASE <1-4>`, `CMD OPEN`, `CMD CLOSE`: change the phase or open the lid without walking out to the box |
| **Runs for months** | stock timing broke after ~49.7 days (`millis()` wrap, a known `@@@BUG`); all timing is now wrap-safe |
| **Watchdog** | on the Uno, a hung board reboots by itself and parks the lid |
| **Phase change fixes** | coins dropped in phases 1–2 no longer pile up and pay out a burst of lid openings in phase 3; a lid left open by phase 1 no longer stays open forever in phases 3–4 |

All changes are marked `cros-plus` in the source and can be switched off in `cros_constants.h`. Behavior that keeps birds safe (slow, stepwise lid closing) is untouched.
Protocol: [docs/serial-protocol.md](docs/serial-protocol.md).

## Layout

```
firmware/cros/   CrOS sketch for ESP32 DevKit or Arduino Uno (Arduino IDE / arduino-cli)
host/            Docker: IP camera → YouTube, feeder bridge, crowbox-cmd
docs/            protocol, ESP32 wiring and flashing, host setup
```

## Hardware

The stock CrowBox 3rd Gen ([BOM](https://thecrowbox.com/wiki/doku.php?id=kit:crowbox2_v20_bom)), with two swaps:

- **ESP32 DevKit** (ESP32-WROOM-32) instead of the Arduino Uno — wiring in [docs/esp32.md](docs/esp32.md).
  A Uno works too; it then needs a USB cable to the host.
- An outdoor **IP camera** with RTSP and H.264, mounted 60–100 cm from the perch.

## Firmware

```bash
cp firmware/cros/wifi_config.example.h firmware/cros/wifi_config.h   # ESP32: WiFi name and password
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/cros           # or arduino:avr:uno
arduino-cli upload  --fqbn esp32:esp32:esp32 -p <port> firmware/cros
```

Toolchain setup and details: [docs/esp32.md](docs/esp32.md). For a Uno: `arduino-cli lib install Servo`,
FQBN `arduino:avr:uno`, no `wifi_config.h` needed.

## Host

```bash
cd host
cp .env.example .env     # stream key, camera RTSP URL, box address
docker compose up -d --build
docker compose exec feeder crowbox-cmd STATUS
```

Synology walkthrough and camera settings: [docs/synology.md](docs/synology.md).

## Coins in the eurozone

The CrowBox takes coins up to 25.75 mm. For training coins upstream recommends 22–25 mm, 1.5–2.5 mm thick;
users report that **50 euro cent** coins work very well, 20 cent coins occasionally jam
([CrowBox wiki](https://thecrowbox.com/wiki/doku.php?id=kit:training_coins)).

## Status

Compiles for the ESP32 DevKit and the Arduino Uno; the host bridge is tested against a simulated box.
Not yet run on real hardware: issues and reports are welcome.

## License

- `firmware/` — CrOS by the CrowBox project, modified; [CC BY-SA 4.0](firmware/cros/license).
- Everything else — [MIT](LICENSE).
