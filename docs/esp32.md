# ESP32 instead of the Arduino Uno

The same CrOS firmware runs on an **ESP32 DevKit** (ESP32-WROOM-32). The box then reports to
the host over **WiFi** instead of a USB cable, so nothing but power has to reach the box.

Everything else of the CrowBox 3rd Gen stays as built: perch switch, copper coin sensor, phase
button, Hitec HS-311 servo, 5 V 2 A supply.

## Wiring

| Function | Uno pin | ESP32 GPIO |
|----------|---------|------------|
| Perch switch (to GND) | 2 | **25** |
| Coin sensor (to GND) | 3 | **26** |
| Phase button (to GND) | 4 | **27** |
| Servo signal | 9 | **18** |
| Status LED | 13 | **2** (on-board LED of most DevKit clones) |
| 5 V from the 2 A supply | 5V | **5V / VIN** |
| Ground | GND | **GND** (supply, servo and switches share it) |

The switches use the internal pull-ups, exactly as on the Uno. The servo gets its power from the
5 V supply and only the signal from GPIO 18. A 3.3 V signal drives most hobby servos, including
the HS-311 in practice, but try it on the bench before closing the box.

An ESP32 DevKit does not fit the 170-point mini breadboard of the stock BOM. A screw-terminal
breakout board for the DevKit is the easy replacement and holds wires better outdoors.

## Flashing

```bash
arduino-cli config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32
arduino-cli lib install ESP32Servo

cp firmware/cros/wifi_config.example.h firmware/cros/wifi_config.h   # fill in SSID and password
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/cros
arduino-cli upload  --fqbn esp32:esp32:esp32 -p <port> firmware/cros
```

`wifi_config.h` is ignored by git, so the password never ends up in a commit.

## On the network

- The ESP32 joins the WiFi as `crowbox` (`CROS_WIFI_HOSTNAME`) and listens on **TCP port 7777**.
  Give it a fixed address (DHCP reservation) in the router; the host connects to `socket://<that address>:7777`.
- Joining WiFi never blocks feeding: the box works without a network and keeps reconnecting.
- One host at a time. A new connection replaces the old one, and the box greets it with
  `EVT STATUS ...` so the host knows the phase right away.
- The USB port keeps working as well: events go to both, commands are accepted from both.
- 2.4 GHz only. Check the signal where the box will stand.

## Differences from the Uno build

- No hardware watchdog yet (`CROS_USE_WATCHDOG` only acts on AVR boards).
- The training phase is kept in flash-emulated EEPROM; it is only written when the phase changes.
