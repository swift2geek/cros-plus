# Serial protocol

The Arduino Uno running CrOS talks to the Raspberry Pi over its USB cable:
**57600 baud, 8N1, one line per message**, `\n` terminated (CrOS also accepts `\r\n`).

Opening the port resets a Uno (DTR). After a reset CrOS spends about 15 seconds parking
the lid before it sends anything.

## Events: Arduino → Pi

Every event line starts with `EVT `. Anything else on the port (for example CrOS debug
output with `CROS_USE_SERIAL_DEBUG`) should be ignored.

| Line | When |
|------|------|
| `EVT PHASE <n>` | on boot, and whenever the training phase changes (button or `CMD PHASE`) |
| `EVT LAND` | a bird landed on the perch; repeats only after the perch was empty for 2 s |
| `EVT LEAVE` | the perch has been empty for 2 s |
| `EVT COIN` | a coin deposit was detected (in any phase; only phases 3–4 pay for it) |
| `EVT FEED` | the lid actually opened. Phase 1 keeps the lid open, so this happens once after boot |
| `EVT CLOSE` | the lid finished closing (also once while parking the lid on boot) |
| `EVT ALIVE <s>` | every 60 s, uptime in seconds |
| `EVT STATUS phase=<n> basket=<open\|closed> perch=<0\|1> queued=<n> uptime=<s>` | reply to `CMD STATUS` |
| `EVT ERR <line>` | a command that was not understood or not allowed |

## Commands: Pi → Arduino

| Line | Effect |
|------|--------|
| `CMD STATUS` | replies with `EVT STATUS ...` |
| `CMD PHASE <1-4>` | switches the training phase exactly like the button: LED blinks, saved to EEPROM, `EVT PHASE` |
| `CMD OPEN` | opens the lid; outside phase 1 it closes by itself after 15 s, as usual |
| `CMD CLOSE` | closes the lid now, in the usual slow safety steps. Refused in phase 1, which keeps the lid open |

Lines longer than 23 characters are dropped. Commands are read once per main loop (every 20 ms).
CrOS is busy and does not read commands while the lid closes (about 13 s) or while the LED
blinks the phase, so a command sent then waits in the Uno's 64-byte serial buffer.

## Switches in `cros_constants.h`

| Define | Default | |
|--------|---------|-|
| `CROS_USE_SERIAL_EVENTS` | on | `EVT` lines |
| `CROS_USE_SERIAL_COMMANDS` | on | `CMD` lines |
| `CROS_USE_WATCHDOG` | on | 8 s hardware watchdog |
| `CROS_USE_SERIAL_DEBUG` | off | stock CrOS debug text (harmless to the Pi, it only reads `EVT` lines) |
