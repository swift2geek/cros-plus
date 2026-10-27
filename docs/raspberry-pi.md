# Raspberry Pi: camera, stream, event log

Hardware: **Raspberry Pi 4** + **Camera Module 3 Wide** (CSI), Raspberry Pi OS Lite 64-bit.
Avoid the Pi 5 for this: it has no hardware H.264 encoder, so the camera stream would be
encoded on the CPU.

Only one process can hold the camera, so [MediaMTX](https://github.com/bluenviron/mediamtx)
holds it and everything else reads its RTSP stream:

```
Camera Module 3 ──CSI──▶ MediaMTX ──rtsp://localhost:8554/cam──┬──▶ crowbox-stream (ffmpeg) ──RTMP──▶ YouTube
                         (hardware H.264)                       ├──▶ crowbox_feeder (snapshot per coin)
                                                                ├──▶ OBS (optional)
                                                                └──▶ browser http://<pi>:8889/cam (WebRTC)

Arduino Uno (CrOS) ──USB──▶ crowbox_feeder ──▶ ~/crowbox-data/events.jsonl, snapshots, /run/crowbox/overlay.txt
                  ◀──USB── crowbox-cmd (via /run/crowbox/cmd)
```

All commands below run on the Pi from a clone of this repository.

## 0. Check the camera

Before MediaMTX is running (afterwards the camera is busy):

```bash
rpicam-hello --list-cameras
rpicam-still -o test.jpg
```

## 1. MediaMTX

`pi/mediamtx.yml`: 1080p30, 4 Mbit/s, keyframe every 2 s, continuous autofocus.

```bash
URL=$(curl -s https://api.github.com/repos/bluenviron/mediamtx/releases/latest \
  | grep -o 'https://[^"]*linux_arm64[^"]*\.tar\.gz' | head -1)
curl -L "$URL" | sudo tar -xz -C /usr/local/bin mediamtx
sudo install -Dm644 pi/mediamtx.yml /usr/local/etc/mediamtx.yml
sudo cp pi/mediamtx.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now mediamtx
```

Check: open `http://<pi>:8889/cam` in a browser.

## 2. Live stream to YouTube (24/7, straight from the Pi)

The video is copied, not re-encoded (`-c:v copy`), so the Pi barely notices.
YouTube often rejects streams without audio, hence the silent `anullsrc` track.

```bash
sudo apt update && sudo apt install -y ffmpeg
sudo install -m755 pi/crowbox-stream.sh /usr/local/bin/
sudo cp pi/crowbox-stream.service /etc/systemd/system/
echo 'STREAM_KEY=xxxx-xxxx-xxxx-xxxx' | sudo tee /etc/crowbox.env
sudo chmod 600 /etc/crowbox.env
sudo systemctl daemon-reload
sudo systemctl enable --now crowbox-stream
journalctl -u crowbox-stream -f
```

`/etc/crowbox.env` may also set `RTMP_URL` (another platform, e.g. Twitch) and `CAMERA_URL`.

## 3. Feeder bridge

`crowbox_feeder.py` reads the [serial protocol](serial-protocol.md), appends every event to
`~/crowbox-data/events.jsonl` (except the `ALIVE` heartbeat), grabs a snapshot from RTSP on
every coin, keeps the overlay text up to date, and forwards commands to the box.

```bash
sudo apt install -y python3-serial ffmpeg
sudo install -m755 pi/crowbox_feeder.py pi/crowbox-cmd /usr/local/bin/
sudo cp pi/crowbox-feeder.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now crowbox-feeder
journalctl -u crowbox-feeder -f
```

The serial port defaults to `/dev/ttyACM0` (genuine Uno); CH340 clones appear as `/dev/ttyUSB0`.
Set `Environment=CROWBOX_PORT=/dev/serial/by-id/...` in the service for a stable name.
Other settings: `CROWBOX_SNAPSHOT_URL`, `CROWBOX_DATA`.

Restarting the service resets the Uno (DTR): the box reboots and parks the lid, which is safe.

### Controlling the box

```bash
crowbox-cmd STATUS
crowbox-cmd PHASE 3
crowbox-cmd OPEN
crowbox-cmd CLOSE
tail -f ~/crowbox-data/events.jsonl
```

## Optional: OBS instead of streaming from the Pi

In OBS: **Sources → Media Source → untick "Local File"** → `rtsp://<pi>:8554/cam`, then stream
from OBS with overlays and scenes. Stop the Pi stream: `sudo systemctl disable --now crowbox-stream`.
The computer running OBS has to stay on, and the video crosses the network twice
(Pi → OBS → YouTube), so give that computer a wired connection.

## Optional: coin counter burned into the stream

ffmpeg can re-read a text file every frame. This needs a re-encode, which costs far more
than `-c:v copy`. Replace the `-map ... -c:v copy` line of `crowbox-stream.sh` with:

```bash
  -map 0:v -map 1:a \
  -vf "drawtext=textfile=/run/crowbox/overlay.txt:reload=1:fontcolor=white:fontsize=36:box=1:boxcolor=black@0.5:x=20:y=20" \
  -c:v h264_v4l2m2m -b:v 4M -g 60 -c:a aac -b:a 128k -shortest \
```

If the Pi can't keep up, drop to 720p in `mediamtx.yml` or do the overlay in OBS.

## Notes

- The stock CSI ribbon is ~15–20 cm; 30–50 cm ribbons are common. Keep the Pi next to the box
  in a weatherproof enclosure.
- Put a polycarbonate window in front of the lens against rain.
- The camera sees nothing at night without IR; corvids are active by day anyway.
