#!/usr/bin/env bash
# CrowBox: IP camera (RTSP) → YouTube Live without re-encoding the video.
# STREAM_KEY and CAMERA_URL come from the environment (.env next to compose.yaml).
# STREAM_AUDIO=camera streams the camera microphone, anything else a silent track
# (YouTube often rejects streams without audio).
set -euo pipefail

: "${STREAM_KEY:?STREAM_KEY is not set}"
: "${CAMERA_URL:?CAMERA_URL is not set}"
RTMP_URL="${RTMP_URL:-rtmp://a.rtmp.youtube.com/live2}"

if [[ "${STREAM_AUDIO:-silent}" == "camera" ]]; then
  audio=(-map 0:v -map 0:a)
else
  audio=(-f lavfi -i anullsrc=channel_layout=stereo:sample_rate=44100 -map 0:v -map 1:a -shortest)
fi

exec ffmpeg -hide_banner -loglevel warning \
  -rtsp_transport tcp -i "${CAMERA_URL}" \
  "${audio[@]}" -c:v copy -c:a aac -b:a 128k \
  -f flv "${RTMP_URL}/${STREAM_KEY}"
