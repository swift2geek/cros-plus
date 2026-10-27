# Host: stream, event log and snapshots in Docker

`host/` runs on anything with Docker that is on 24/7 — written for a **Synology NAS** (DSM 7.2+,
Container Manager), works the same on any Linux box. Two containers from one small image:

| Container | Does |
|-----------|------|
| `stream` | IP camera (RTSP) → YouTube Live, video copied without re-encoding |
| `feeder` | connects to the CrowBox, appends every event to `data/events.jsonl`, grabs a camera snapshot on every coin, forwards commands |

```
IP camera ──RTSP──┬──▶ stream ──RTMP──▶ YouTube
                  └──▶ feeder (snapshot per coin) ──▶ data/events.jsonl, data/snapshots/
CrowBox (ESP32) ◀──TCP 7777──▶ feeder ◀── crowbox-cmd
```

Load on a DS225+-class NAS is a few percent of one core: nothing is re-encoded.

## Camera requirements

- **RTSP** (often listed as RTSP/ONVIF). Without it the NAS can't read the stream.
- **H.264**, not H.265: YouTube ingests H.264 over RTMP. Set the stream codec in the camera app.
- **Keyframe (I-frame) interval ≤ 4 s**; 2 s is ideal, e.g. 50 at 25 fps.
- 1080p at 3–6 Mbit/s is plenty. Most cameras offer a main and a sub stream; pick the H.264 one.
- Fixed-focus cameras are sharp from roughly 60–100 cm. Mount the camera that far from the perch,
  aimed at the perch and the coin slot.

## Setup on Synology

1. **Package Center** → install **Container Manager** (and **Git Server**, which provides the `git` command).
2. Enable SSH (**Control Panel → Terminal & SNMP**), then on the NAS:

   ```bash
   cd /volume1/docker
   git clone https://github.com/swift2geek/cros-plus.git
   cd cros-plus/host
   cp .env.example .env
   vi .env
   sudo docker compose up -d --build
   ```

   Or skip SSH: **Container Manager → Project → Create**, path `/volume1/docker/cros-plus/host`,
   it picks up `compose.yaml`.
3. Logs: `sudo docker compose logs -f`. Data: `/volume1/docker/cros-plus/host/data/`.

`.env` keys: `STREAM_KEY`, `CAMERA_URL`, `STREAM_AUDIO` (`camera` to stream the microphone),
`CROWBOX_PORT` (`socket://<box address>:7777`), `TZ`, optional `RTMP_URL`.

## Controlling the box

```bash
sudo docker compose exec feeder crowbox-cmd STATUS
sudo docker compose exec feeder crowbox-cmd PHASE 3
sudo docker compose exec feeder crowbox-cmd OPEN
tail -f data/events.jsonl
```

## Behavior worth knowing

- The feeder exits when the box disconnects or stays silent for 3 minutes (it sends `EVT ALIVE`
  every 60 s); Docker restarts it and it reconnects.
- The coin counter in the overlay text starts from zero when the feeder restarts.
- With an Arduino Uno instead of an ESP32 the box needs a USB cable to the host. Pass the device
  into the `feeder` container (`devices: ["/dev/ttyACM0"]`) and set `CROWBOX_PORT=/dev/ttyACM0`.

## Optional: coin counter burned into the stream

The feeder keeps `/run/crowbox/overlay.txt` up to date inside its container. Showing it in the
video needs a re-encode (ffmpeg `drawtext`), which costs far more CPU than copying. The simpler
route is an overlay in OBS or YouTube's own tools.
