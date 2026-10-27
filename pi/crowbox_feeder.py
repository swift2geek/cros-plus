#!/usr/bin/env python3
"""CrowBox feeder bridge: CrOS events over USB serial → log, snapshots, stream overlay; commands → CrOS.

Serial protocol: docs/serial-protocol.md.
Commands for the box are written as lines to the FIFO /run/crowbox/cmd (see crowbox-cmd).
"""
import datetime as dt
import errno
import json
import os
import pathlib
import subprocess

import serial  # sudo apt install python3-serial

PORT = os.environ.get("CROWBOX_PORT", "/dev/ttyACM0")  # genuine Uno; CH340 clones show up as /dev/ttyUSB0
BAUD = 57600  # CROS_SERIAL_BAUD_RATE
SNAPSHOT_URL = os.environ.get("CROWBOX_SNAPSHOT_URL", "rtsp://localhost:8554/cam")
SNAPSHOT_TIMEOUT = 15
DATA = pathlib.Path(os.environ.get("CROWBOX_DATA", pathlib.Path.home() / "crowbox-data"))
RUN = pathlib.Path("/run/crowbox")
OVERLAY = RUN / "overlay.txt"
COMMANDS = RUN / "cmd"
UNLOGGED = {"ALIVE"}

DATA.mkdir(parents=True, exist_ok=True)
(DATA / "snapshots").mkdir(exist_ok=True)


def snapshot(name: str) -> None:
    cmd = [
        "ffmpeg", "-hide_banner", "-loglevel", "error", "-y",
        "-rtsp_transport", "tcp", "-i", SNAPSHOT_URL,
        "-frames:v", "1", "-q:v", "2", str(DATA / "snapshots" / name),
    ]
    try:
        subprocess.run(cmd, check=False, timeout=SNAPSHOT_TIMEOUT)
    except subprocess.TimeoutExpired:
        print(f"snapshot: no frame from {SNAPSHOT_URL} within {SNAPSHOT_TIMEOUT} s", flush=True)


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
    coins_today, phase, day = 0, "?", dt.date.today()
    write_overlay(coins_today, phase)
    fifo, pending = open_command_fifo(), b""
    with serial.Serial(PORT, BAUD, timeout=1) as ser, open(DATA / "events.jsonl", "a") as log:
        while True:
            commands, pending = read_commands(fifo, pending)
            for command in commands:
                ser.write(command.encode() + b"\n")

            line = ser.readline().decode(errors="ignore").strip()
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
            if kind not in UNLOGGED:
                log.write(json.dumps({"ts": now.isoformat(timespec="seconds"), "event": kind, "raw": line}) + "\n")
                log.flush()
            write_overlay(coins_today, phase)


if __name__ == "__main__":
    main()
