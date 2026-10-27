FROM python:3.12-slim

RUN apt-get update \
 && apt-get install -y --no-install-recommends ffmpeg \
 && rm -rf /var/lib/apt/lists/* \
 && pip install --no-cache-dir pyserial==3.5

COPY crowbox_feeder.py crowbox-stream.sh crowbox-cmd /usr/local/bin/

ENV CROWBOX_DATA=/data PYTHONUNBUFFERED=1
