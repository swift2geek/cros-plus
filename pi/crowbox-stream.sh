#!/usr/bin/env bash
# CrowBox: camera from MediaMTX (RTSP) → YouTube Live. STREAM_KEY comes from /etc/crowbox.env (via systemd).
set -euo pipefail

: "${STREAM_KEY:?STREAM_KEY is not set}"
RTMP_URL="${RTMP_URL:-rtmp://a.rtmp.youtube.com/live2}"
CAMERA_URL="${CAMERA_URL:-rtsp://localhost:8554/cam}"

exec ffmpeg -hide_banner -loglevel warning \
  -rtsp_transport tcp -i "${CAMERA_URL}" \
  -f lavfi -i anullsrc=channel_layout=stereo:sample_rate=44100 \
  -map 0:v -map 1:a -c:v copy -c:a aac -b:a 128k -shortest \
  -f flv "${RTMP_URL}/${STREAM_KEY}"
