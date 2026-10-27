# Host: camera stream in Docker

`host/` runs on anything with Docker that is on 24/7 — written for a **Synology NAS** (DSM 7.2+,
Container Manager), works the same on any Linux box. For a stock CrowBox you need one container:

| Container | Does |
|-----------|------|
| `stream` | IP camera (RTSP) → YouTube Live, video copied without re-encoding |

```
IP camera ──RTSP──▶ stream ──RTMP──▶ YouTube
```

The box and the camera know nothing about each other. Load on a DS225+-class NAS is a few percent
of one core: nothing is re-encoded.

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
   sudo docker compose up -d --build stream
   ```

   Always name the service: a bare `docker compose up` also starts the experimental `feeder`,
   which without a box running the fork restarts forever.
   Creating a project in **Container Manager → Project** has the same effect — stop and disable
   the `feeder` container there afterwards.
3. Logs: `sudo docker compose logs -f stream`.

`.env` keys for the stream: `STREAM_KEY`, `CAMERA_URL`, `STREAM_AUDIO` (`silent` adds a silent track,
which YouTube often requires; `camera` streams the microphone), optional `RTMP_URL` for another platform.
`CROWBOX_PORT` and `TZ` are only read by the feeder; leave them as they are.

## Checking the camera

```bash
sudo docker run --rm --entrypoint ffprobe crowbox-host -rtsp_transport tcp "<CAMERA_URL>"
```

The video stream must show `h264`. Then start a test broadcast on YouTube (visibility: unlisted)
and check that the perch and the coin slot are in frame.

## Experimental: feeder — not tested on hardware

The `feeder` service needs the [experimental firmware](../README.md#experimental--not-tested-on-hardware):
stock CrOS sends no events. It connects to the box, appends every event to `data/events.jsonl`,
grabs a camera snapshot on every coin and forwards commands.

```
IP camera ──RTSP──▶ feeder (snapshot per coin) ──▶ data/events.jsonl, data/snapshots/
CrowBox (fork) ◀──TCP 7777 or USB──▶ feeder ◀── crowbox-cmd
```

```bash
sudo docker compose up -d --build feeder
sudo docker compose exec feeder crowbox-cmd STATUS
sudo docker compose exec feeder crowbox-cmd PHASE 3
sudo docker compose exec feeder crowbox-cmd OPEN
tail -f data/events.jsonl
```

- `CROWBOX_PORT`: `socket://<box address>:7777` for the ESP32 build. With an Uno the box needs a USB
  cable to the host: pass the device into the container (`devices: ["/dev/ttyACM0"]`) and set
  `CROWBOX_PORT=/dev/ttyACM0`.
- The feeder exits when the box disconnects or stays silent for 3 minutes (the fork sends `EVT ALIVE`
  every 60 s); Docker restarts it and it reconnects.
- It keeps a coin counter in `/run/crowbox/overlay.txt` inside its container, starting from zero on
  every restart. Burning it into the video needs a re-encode (ffmpeg `drawtext`), which costs far
  more CPU than copying; an overlay in OBS or YouTube's own tools is simpler.
