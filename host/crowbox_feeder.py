#!/usr/bin/env python3
"""CrowBox feeder bridge: CrOS events → log, snapshots, stream overlay; commands → CrOS.

The box is reached through CROWBOX_PORT, any pyserial URL:
    socket://192.168.1.50:7777   ESP32 over WiFi
    /dev/ttyACM0                 Arduino Uno over USB (CH340 clones: /dev/ttyUSB0)

Protocol: docs/serial-protocol.md.
Commands for the box are written as lines to the FIFO /run/crowbox/cmd (see crowbox-cmd).
"""
import datetime as dt
import errno
import json
import os
import pathlib
import subprocess
import time

import serial

PORT = os.environ["CROWBOX_PORT"]
BAUD = 57600  # CROS_SERIAL_BAUD_RATE, ignored for socket://
CAMERA_URL = os.environ["CAMERA_URL"]
SNAPSHOT_TIMEOUT = 15
SILENCE_LIMIT = 180  # the box sends EVT ALIVE every 60 s; longer silence means a dead link
DATA = pathlib.Path(os.environ.get("CROWBOX_DATA", pathlib.Path.home() / "crowbox-data"))
RUN = pathlib.Path(os.environ.get("CROWBOX_RUN", "/run/crowbox"))
OVERLAY = RUN / "overlay.txt"
COMMANDS = RUN / "cmd"
UNLOGGED = {"ALIVE"}


def snapshot(name: str) -> None:
    cmd = [
        "ffmpeg", "-hide_banner", "-loglevel", "error", "-y",
        "-rtsp_transport", "tcp", "-i", CAMERA_URL,
        "-frames:v", "1", "-q:v", "2", str(DATA / "snapshots" / name),
    ]
    try:
        subprocess.run(cmd, check=False, timeout=SNAPSHOT_TIMEOUT)
    except subprocess.TimeoutExpired:
        print(f"snapshot: no frame from the camera within {SNAPSHOT_TIMEOUT} s", flush=True)


def write_overlay(coins: int, phase: str) -> None:
    tmp = OVERLAY.with_suffix(".tmp")
    try:
        tmp.write_text(f"Phase {phase} · coins today: {coins}")
        tmp.replace(OVERLAY)
    except OSError as e:
        print(f"overlay: {e}", flush=True)


def open_command_fifo() -> int:
    if not COMMANDS.exists():
        os.mkfifo(COMMANDS, 0o620)
    # O_RDWR keeps a writer open ourselves, so reads return EAGAIN instead of EOF when idle
    return os.open(COMMANDS, os.O_RDWR | os.O_NONBLOCK)


def read_commands(fd: int, pending: bytes) -> tuple[list[str], bytes]:
    try:
        pending += os.read(fd, 1024)
    except OSError as e:
        if e.errno != errno.EAGAIN:
            raise
    *lines, pending = pending.split(b"\n")
    commands = [line.decode(errors="ignore").strip() for line in lines]
    return [c for c in commands if c.startswith("CMD ")], pending


def main() -> None:
    (DATA / "snapshots").mkdir(parents=True, exist_ok=True)
    RUN.mkdir(parents=True, exist_ok=True)
    coins_today, phase, day = 0, "?", dt.date.today()
    write_overlay(coins_today, phase)
    fifo, pending = open_command_fifo(), b""
    with serial.serial_for_url(PORT, BAUD, timeout=1) as box, open(DATA / "events.jsonl", "a") as log:
        last_heard = time.monotonic()
        while True:
            commands, pending = read_commands(fifo, pending)
            for command in commands:
                box.write(command.encode() + b"\n")

            line = box.readline().decode(errors="ignore").strip()
            if not line:
                if time.monotonic() - last_heard > SILENCE_LIMIT:
                    raise SystemExit(f"no data from the box for {SILENCE_LIMIT} s, reconnecting")
                continue
            last_heard = time.monotonic()
            if not line.startswith("EVT "):
                continue
            now = dt.datetime.now()
            if now.date() != day:
                coins_today, day = 0, now.date()
            parts = line.split()
            kind = parts[1]
            if kind == "COIN":
                coins_today += 1
                snapshot(now.strftime("%Y%m%d-%H%M%S") + ".jpg")
            elif kind == "PHASE" and len(parts) > 2:
                phase = parts[2]
            elif kind == "STATUS":
                phase = next((p.split("=", 1)[1] for p in parts[2:] if p.startswith("phase=")), phase)
            if kind not in UNLOGGED:
                log.write(json.dumps({"ts": now.isoformat(timespec="seconds"), "event": kind, "raw": line}) + "\n")
                log.flush()
            write_overlay(coins_today, phase)


if __name__ == "__main__":
    main()
