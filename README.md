# cros-plus

Live-stream an **official, unmodified [CrowBox](https://www.thecrowbox.com/) 3rd Generation** (a coin-operated
feeder that teaches crows and magpies to trade coins for peanuts) with a **separate outdoor IP camera**,
streamed to YouTube from a NAS in Docker. The box itself is built exactly as the CrowBox documentation says;
the camera only watches it.

Not affiliated with the CrowBox project.

```
 outdoors                                            indoors (24/7, e.g. Synology NAS)
┌──────────────────────────┐
│ CrowBox 3rd Gen, stock   │                        ┌───────────────────────────────┐
│ (official BOM, Uno,      │                        │ Docker: stream                │
│  stock CrOS firmware)    │                        │ RTSP → YouTube Live           │──▶ YouTube
└──────────────────────────┘                        └───────────────────────────────┘
 IP camera (RTSP, H.264) ───────WiFi / PoE───────────────────▲
```

## 1. Build the box — the official way

Follow the CrowBox documentation as is: [BOM](https://thecrowbox.com/wiki/doku.php?id=kit:crowbox2_v20_bom),
[assembly videos](https://youtube.com/playlist?list=PL55RinDSPfVUPfaxLV2Ef9dEO8SEhEmbo),
[wiring guide and templates](https://thecrowbox.com/wiki/doku.php?id=kit:tools_and_helpers),
[stock CrOS firmware](https://thecrowbox.com/wiki/doku.php?id=kit:cros_software).
Get it working on the bench before adding anything.

## 2. Add the camera and the stream

- An outdoor IP camera with **RTSP** and **H.264** (not only H.265), keyframe interval ≤ 4 s, ideally IR for dusk.
  Fixed-focus cameras are sharp from roughly 60–100 cm: mount it that far from the perch, aimed at the perch and the coin slot.
  It does not connect to the box in any way.
- On any always-on Docker host (written for Synology):

  ```bash
  cd host
  cp .env.example .env     # STREAM_KEY and CAMERA_URL are all the stream needs
  docker compose up -d --build stream
  ```

  The video is copied, not re-encoded, so a small NAS handles it easily. A silent audio track is added
  (YouTube often rejects streams without audio), or the camera microphone with `STREAM_AUDIO=camera`.

Synology walkthrough and camera settings: [docs/synology.md](docs/synology.md).

**Status:** tested end to end in Docker against a simulated camera and a local RTMP server; not yet with a real camera.

## Coins in the eurozone

The CrowBox takes coins up to 25.75 mm. For training coins upstream recommends 22–25 mm, 1.5–2.5 mm thick;
users report that **50 euro cent** coins work very well, 20 cent coins occasionally jam
([CrowBox wiki](https://thecrowbox.com/wiki/doku.php?id=kit:training_coins)).

## Experimental — not tested on hardware

Everything below compiles but has **never run on a real box**. Don't use it for your first build;
the stock firmware is the reference.

`firmware/cros/` is a fork of CrOS v1.0. The first commit is the upstream zip, unmodified, so `git log` shows
exactly what changed; all changes are marked `cros-plus` in the source and can be switched off in `cros_constants.h`.
Behavior that keeps birds safe (slow, stepwise lid closing) is untouched.

| | |
|---|---|
| **Bug fixes for the stock Uno build** | wrap-safe timing (stock code breaks after ~49.7 days of uptime, a known `@@@BUG`); coins dropped in phases 1–2 no longer pay out a burst of lid openings in phase 3; a lid left open by phase 1 no longer stays open in phases 3–4; a race on the coin counter; an 8 s watchdog |
| **Events and commands** | `EVT LAND/LEAVE/COIN/FEED/CLOSE/PHASE/ALIVE` out, `CMD STATUS/PHASE/OPEN/CLOSE` in, over USB serial — [docs/serial-protocol.md](docs/serial-protocol.md) |
| **ESP32 instead of the Uno** | the same sketch builds for an ESP32 DevKit and talks over WiFi (TCP 7777) — [docs/esp32.md](docs/esp32.md). Note: ESP32 logic is 3.3 V, the Uno is 5 V; the HS-311 servo is not verified on 3.3 V signal and ESP32 pins must never see 5 V |
| **Feeder bridge** | `host/` service `feeder`: logs every event, grabs a camera snapshot per coin, forwards commands — needs the events above |

```bash
arduino-cli compile --fqbn arduino:avr:uno firmware/cros        # Uno (lib: Servo)
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/cros      # ESP32 (lib: ESP32Servo, needs wifi_config.h)
```

## Layout

```
host/            Docker: stream (camera → YouTube); experimental feeder bridge
docs/            Synology setup; experimental: protocol, ESP32
firmware/cros/   experimental CrOS fork for Arduino Uno / ESP32
```

## License

- `firmware/` — CrOS by the CrowBox project, modified; [CC BY-SA 4.0](firmware/cros/license).
- Everything else — [MIT](LICENSE).
