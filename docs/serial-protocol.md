# Box ↔ host protocol

Plain text, **one line per message**, `\n` terminated (CrOS also accepts `\r\n`). The same lines
travel over two links:

| Board | Link |
|-------|------|
| ESP32 | **TCP port 7777** over WiFi (one host at a time, see [esp32.md](esp32.md)), and USB serial |
| Arduino Uno | USB serial, **57600 baud, 8N1** |

A host that connects over WiFi is greeted with `EVT STATUS ...`. Opening the USB port resets a
Uno (DTR); after any reset CrOS spends about 18 seconds parking the lid before it sends `EVT PHASE`.

## Events: box → host

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
| `EVT STATUS phase=<n> basket=<open\|closed> perch=<0\|1> queued=<n> uptime=<s>` | reply to `CMD STATUS`, and on every new WiFi connection |
| `EVT ERR <line>` | a command that was not understood or not allowed |

## Commands: host → box

| Line | Effect |
|------|--------|
| `CMD STATUS` | replies with `EVT STATUS ...` |
| `CMD PHASE <1-4>` | switches the training phase exactly like the button: LED blinks, saved to EEPROM, `EVT PHASE` |
| `CMD OPEN` | opens the lid; outside phase 1 it closes by itself after 15 s, as usual |
| `CMD CLOSE` | closes the lid now, in the usual slow safety steps. Refused in phase 1, which keeps the lid open |

Lines longer than 23 characters are dropped. Commands are read once per main loop (every 20 ms).
CrOS is busy and does not read commands while the lid closes (about 13 s) or while the LED
blinks the phase, so a command sent then waits in the receive buffer (64 bytes on a Uno).

## Switches in `cros_constants.h`

| Define | Default | |
|--------|---------|-|
| `CROS_USE_SERIAL_EVENTS` | on | `EVT` lines |
| `CROS_USE_SERIAL_COMMANDS` | on | `CMD` lines |
| `CROS_USE_WATCHDOG` | on | 8 s hardware watchdog, AVR boards only (Uno R3). Needs the optiboot bootloader, standard on genuine Uno R3; clones with the old bootloader may reboot endlessly after the first watchdog reset — undefine it then |
| `CROS_USE_SERIAL_DEBUG` | off | stock CrOS debug text, USB only (harmless, hosts only read `EVT` lines) |
