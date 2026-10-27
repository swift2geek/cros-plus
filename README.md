# crowbox-cros-pi

**Unofficial** fork of [CrOS](https://thecrowbox.com/wiki/doku.php?id=kit:cros_software), the firmware of the
[CrowBox](https://www.thecrowbox.com/) 3rd Generation (a coin-operated feeder that teaches crows and magpies
to trade coins for peanuts), plus a Raspberry Pi module that **streams the box live** and logs every visit.

Not affiliated with the CrowBox project. Upstream publishes CrOS as a zip on its wiki; the first commit here
is that zip, unmodified, so `git log` shows exactly what changed.

## What's different from stock CrOS v1.0

| | |
|---|---|
| **Talks to a Raspberry Pi** | one-line events over USB serial: `EVT LAND`, `LEAVE`, `COIN`, `FEED`, `CLOSE`, `PHASE <n>`, `ALIVE <uptime>` |
| **Takes commands** | `CMD STATUS`, `CMD PHASE <1-4>`, `CMD OPEN`, `CMD CLOSE`: change the phase or open the lid without walking out to the box |
| **Runs for months** | stock timing broke after ~49.7 days (`millis()` wrap, a known `@@@BUG`); all timing is now wrap-safe |
| **Watchdog** | a hung Arduino reboots by itself and parks the lid |
| **Phase change fixes** | coins dropped in phases 1–2 no longer pile up and pay out a burst of lid openings in phase 3; a lid left open by phase 1 no longer stays open forever in phases 3–4 |

All changes are marked `CrOS-Pi` in the source and can be switched off in `cros_constants.h`.
Behavior that keeps birds safe (slow, stepwise lid closing) is untouched.
Details: [docs/serial-protocol.md](docs/serial-protocol.md).

## Layout

```
firmware/cros/     CrOS for Arduino Uno (Arduino IDE / arduino-cli sketch)
pi/                Raspberry Pi: MediaMTX camera, YouTube stream, feeder bridge, systemd units
docs/              serial protocol, Raspberry Pi setup
```

## Firmware

Hardware and wiring are exactly the stock CrowBox 3rd Gen ([BOM](https://thecrowbox.com/wiki/doku.php?id=kit:crowbox2_v20_bom)):
perch on pin 2, coin sensor on pin 3, phase button on pin 4, servo on pin 9. Connect the Uno to the Pi with a USB A–B cable.

```bash
arduino-cli core install arduino:avr
arduino-cli lib install Servo
arduino-cli compile --fqbn arduino:avr:uno firmware/cros
arduino-cli upload  --fqbn arduino:avr:uno -p <port> firmware/cros
arduino-cli monitor -p <port> -c baudrate=57600     # EVT PHASE <n> appears ~18 s after boot
```

Or open `firmware/cros/cros.ino` in the Arduino IDE.

## Raspberry Pi

Raspberry Pi 4 + Camera Module 3 Wide → MediaMTX → ffmpeg → YouTube Live, plus an event log with a
snapshot for every coin. Setup: [docs/raspberry-pi.md](docs/raspberry-pi.md).

## Coins in the eurozone

The CrowBox takes coins up to 25.75 mm. For training coins upstream recommends 22–25 mm, 1.5–2.5 mm thick;
users report that **50 euro cent** coins work very well, 20 cent coins occasionally jam
([CrowBox wiki](https://thecrowbox.com/wiki/doku.php?id=kit:training_coins)).

## Status

Compiles for the Arduino Uno (`arduino:avr` 1.8.8). Not yet run on real hardware: issues and reports are welcome.

## License

- `firmware/` — CrOS by the CrowBox project, modified; [CC BY-SA 4.0](firmware/cros/license).
- Everything else — [MIT](LICENSE).
