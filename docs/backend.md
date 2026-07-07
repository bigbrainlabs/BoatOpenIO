# BoatOpenIO – Firmware & Backend Reference

This reference describes **what the firmware does at runtime** – boot sequence, measurement
and publishing behaviour, the HTTP/MQTT interfaces, the security model and persistence.
It is meant as a lookup and operating manual for the backend.

For first-time commissioning see [`setup.md`](setup.md); for the field-by-field
configuration reference see [`configuration.md`](configuration.md).

> As of firmware **v2.2**. Source files: `firmware_src/firmware_src.ino` (logic) and
> `firmware_src/webui.h` (web UI, HTTP handlers).

---

## 1. Architecture at a glance

```
              ┌──────────────────────────── ESP32-WROOM-32D ───────────────────────────┐
  Sensors  →  │  CD74HC4067 MUX ──► ADS1115 @ 0x48 (A0)      MPU6050 @ 0x68/0x69 (I²C)  │
  (16 terms)  │         ▲                                                                │
              │         │ S0–S3 (GPIO 14/27/26/25)                                       │
              │   ┌─────┴───── loop() (Core 1, non-blocking, millis()-paced) ────────┐   │
              │   │  channels 2 s · IMU 50 Hz · pitch/roll 1 s · heartbeat 10 s      │   │
              │   └──────────────────────────────────────────────────────────────────┘  │
              │      │                 │                    │                             │
              │   WebServer:80    PubSubClient (MQTT)   ArduinoOTA / mDNS                 │
              └──────┼─────────────────┼────────────────────┼─────────────────────────────┘
                     │                 │                    │
                Browser (AP/STA)   MQTT broker         Arduino IDE (OTA)
```

There are **no** application FreeRTOS tasks – everything runs cooperatively in `loop()`.
All timing uses the overflow-safe `millis()` subtraction pattern, so no timer drifts after
the 49.7-day millis rollover.

---

## 2. Boot sequence (`setup()`)

Order at boot:

1. **Serial** (115200 baud) – status output.
2. **Factory-reset check** – holding GPIO0 at power-on for 3 s clears NVS and `config.json`
   and restarts (see §11).
3. **I²C** (`Wire.begin`, SDA 21 / SCL 22) and **LittleFS** mount.
4. **`loadNetConfig()`** – network/portal/IMU settings from NVS.
5. **`loadConfig()`** – channel config from `/config.json`. On a read/JSON error the default
   channels stay active and the file is **not** overwritten.
6. **I²C scan** – discovered addresses are listed in the serial monitor.
7. **ADS1115 @ 0x48**, gain `GAIN_TWOTHIRDS` (±6.144 V range, matching the 3.3 V front-end).
8. **MPU6050 @ 0x68 or 0x69**, accel ±8 g, gyro ±250 °/s, filter bandwidth **94 Hz**
   (high enough to catch impact transients). If absent, the fake IMU (demo motion) runs.
9. **Gyro-bias calibration** – 100 samples (~0.5 s), keep the board still.
10. **WiFi** in `WIFI_AP_STA` mode: the **AP always starts immediately**; if STA credentials
    exist it additionally waits up to 10 s for a router connection.
11. **MQTT** – server/port set, receive buffer **4096 bytes** (so full config payloads arrive).
12. **`ensureStaServices()`** – mDNS (`boatopenio.local`) + OTA as soon as STA is connected.
13. **Web server + captive portal** start.
14. **Task watchdog** (10 s, panic reset) registered on the loop task.

---

## 3. Operating modes: TEST vs. LIVE

| Mode | Channel values | Purpose |
|------|---------------|---------|
| **TEST** (default) | Simulated sine values (`2.5 V + sin(...)·0.3`) | Safe commissioning without real sensors |
| **LIVE** | Real ADS1115 readings via the MUX | Production |

Toggle in the portal under **Actions**. The mode is stored in NVS and restored at boot.

> **Important:** In TEST mode impact alarms are published to `boat/test/alarm/*` instead of
> the real `boat/alarm/*` topics – a freshly configured device won't trigger real alarm
> displays.

---

## 4. Measurement chain

### 4.1 Analog channels (all 16)

All 16 terminals share **one** ADS1115. The CD74HC4067 multiplexer routes the selected
terminal onto ADS pin A0:

```
selectChannel(terminal-1)  →  set S0–S3  →  500 µs settle  →  read ADS A0
```

Each reading **averages 4 samples** (`ADS_SAMPLES`) to smooth engine-bay noise
(alternator, ignition). Conversion to the published value:

```
published_value = raw_voltage × factor + offset
```

Only channels marked **active** are read and published, every **2 s** (`READ_INTERVAL`).

### 4.2 Attitude & impact (MPU6050)

- **50 Hz sampling** (`IMU_INTERVAL` = 20 ms): a complementary filter (α = 0.98) of gyro +
  accel yields pitch/roll; in parallel the acceleration magnitude is checked for impacts.
- **Pitch/roll publishing** is decoupled to every **1 s** (`IMU_PUBLISH_INTERVAL`) to avoid
  flooding the broker. Values are mounting-offset and inversion corrected.
- **Impact detection:** when net acceleration `|a|/9.81 − 1` exceeds **0.5 g** an impact is
  active; the peak is held. After **8 s** without a new spike (`IMPACT_DECAY`) the alarm resets.

| Peak (g) | Severity |
|----------|----------|
| < 1.0 | `LEICHT` (light) |
| < 3.0 | `MITTEL` (medium) |
| < 6.0 | `STARK` (strong) |
| ≥ 6.0 | `KRITISCH` (critical) |

---

## 5. MQTT interface

### 5.1 Connection

- Client ID `BoatOpenIO`, optionally with user/password.
- **Last Will (LWT):** `boatopenio/status` = `offline` (retained), on connect → `online`.
- **Reconnect backoff:** 5 s after success, **30 s** after a failed attempt – a dead broker
  won't keep stalling the loop.
- **On (re)connect** the retained alarm state is cleared (see §5.2), unless an impact is
  currently active – this prevents a permanently "stuck" alarm after a brownout mid-impact.

### 5.2 Published topics (ESP32 → broker)

| Topic | Payload | Retained | Interval |
|-------|---------|:--------:|----------|
| `boat/io/<sensor>` or custom `topic` | float value | no | 2 s |
| `boat/io/pitch` | float °, corrected | yes | 1 s |
| `boat/io/roll` | float °, corrected | yes | 1 s |
| `boatopenio/status` | `online` / `offline` | yes | 10 s + LWT |
| `boatopenio/uptime` | seconds since boot | no | 10 s |
| `boatopenio/rssi` | WiFi signal (dBm) | no | 10 s |
| `boatopenio/mode` | `TEST` / `LIVE` | yes | 10 s |
| `boat/alarm/impact_g` ¹ | peak g value | yes | on event |
| `boat/alarm/impact_severity` ¹ | `LEICHT`/`MITTEL`/`STARK`/`KRITISCH` | yes | on event |
| `boat/alarm/impact_active` ¹ | `true` / `false` | yes | on event |

¹ In **TEST** mode the prefix is `boat/test/alarm/` instead of `boat/alarm/`.

### 5.3 Subscribed topics (broker → ESP32)

**`boatopenio/config`** – full or partial `config.json` structure (JSON). Overwrites the
in-RAM channel config and stores it (only when actually changed) to LittleFS. Only fields
present in the payload are changed. Sensor and topic strings are filtered server-side (only
`A–Z a–z 0–9 _ - . /`), `klemme` is clamped to 1–16. After applying, `boatopenio/status` =
`config_updated` is reported.

**Example – activate a channel with a Signal K topic:**
```json
{ "kanaele": [
  { "klemme": 3, "sensor": "oeldruck", "faktor": 1.25, "aktiv": true,
    "topic": "vessels/self/propulsion/engine/oilPressure" }
] }
```

> **Signal K units:** the firmware sends **raw values** (degrees, °C, bar, %), not the SI
> units Signal K expects (rad, K, Pa, ratio). Convert via factor/offset or in the SK server
> if needed.

---

## 6. Web portal & HTTP API

### 6.1 Reachability

| Address | Requirement |
|---------|-------------|
| `http://192.168.4.1` | Always – via the built-in AP |
| `http://boatopenio.local` | When on WiFi (mDNS) |
| `http://<IP>` | When on WiFi (IP in serial monitor) |

### 6.2 Routes

| Route | Method | Protection | Function |
|-------|--------|-----------|----------|
| `/` | GET | Auth | Dashboard (full web UI) |
| `/setup` | GET | setup only, Auth afterwards | First-time setup form |
| `/dosetup` | POST | setup/Auth + CSRF | Set portal/AP credentials |
| `/save` | POST | Auth + CSRF | Save channel config |
| `/savenet` | POST | Auth + CSRF | Save WiFi/MQTT (+ restart) |
| `/savesec` | POST | Auth + CSRF | AP/portal credentials (+ restart) |
| `/testmode` | POST | Auth + CSRF | Toggle TEST/LIVE (+ restart) |
| `/reboot` | POST | Auth + CSRF | Restart |
| `/calibrate` | POST | Auth + CSRF | Set current pitch/roll as mounting offset |
| `/setinvert` | POST | Auth + CSRF | Set pitch/roll inversion |
| `/api/values` | GET | Auth | Active channel values + pitch/roll (corrected), JSON |
| `/api/raw` | GET | Auth | Raw ADS A0–A3 voltages (diagnostics), JSON |
| `/api/adc?ch=<1–16>` | GET | Auth | Select MUX terminal, averaged voltage, JSON |
| `/api/imu` | GET | Auth | Pitch/roll raw + corrected, offsets, gyro bias, JSON |

`/api/adc` always reads real ADS hardware (regardless of TEST mode) and serves the
calibration calculator (§8).

### 6.3 JSON response examples

```
GET /api/values → {"batterie_starter":12.48,"tank_diesel":47.0,"pitch":1.2,"roll":-0.4}
GET /api/adc?ch=3 → {"ok":true,"v":1.8342}
GET /api/imu    → {"pitch_raw":1.4,"roll_raw":-0.3,"pitch_corr":1.2,...,"bias_x":0.0012,...}
```

---

## 7. Security model

- **Enforced first-time setup:** while no portal password is set, all routes redirect to
  `/setup`. After setup, `/setup` and `/dosetup` require admin login.
- **HTTP Basic Auth** on every route (dashboard, POST actions, JSON APIs). Portal user and
  password are independent of the AP password.
- **CSRF protection:** POST requests are only accepted when `Origin`/`Referer` match the own
  host (foreign pages can't trigger actions).
- **Input filtering:** sensor/topic names are restricted to `A–Z a–z 0–9 _ - . /` – ruling
  out MQTT wildcards (`#`, `+`) and HTML/script injection.
- **Output escaping:** all stored values are HTML-escaped when rendered.
- **Factory guard:** while the AP password is still the default (`boatopenio`), the portal
  shows a yellow warning banner at the top.

---

## 8. Calibration calculator (web UI)

Two-point linear calibration that derives **factor and offset** from two known states
(`value = factor·V + offset`):

- For each point you enter the **real value** of that state (e.g. 40 °C and 100 °C, or 50 %
  and 75 %) and the ADS voltage measured for it.
- **"Read A0"** grabs the current voltage of the selected channel via `/api/adc`.
- **"Apply to channel"** writes the result straight into the factor/offset fields of the
  channel table.

Calculation:
```
Factor = (value₂ − value₁) / (V₂ − V₁)
Offset = value₁ − Factor · V₁
```

### Ω mode (resistive VDO senders)

Switchable to **resistance input** for resistive senders (e.g. fuel 10–180 Ω). Instead of
voltage you enter each point's **resistance**; the firmware converts it to ADS voltage via
the divider parameters:

| Parameter | Default | Meaning |
|-----------|---------|---------|
| Reference voltage | 3.3 V | Divider supply voltage |
| Fixed resistor | 1000 Ω | Fixed resistor (Bourns 4116R-1-102, 1 kΩ) |
| Circuit | pull-up | `sender → GND` (pull-up) or `sender → Vref` (pull-down) |

Conversion (pull-up): `V = Vref · R / (R_fix + R)`, inverse `R = R_fix · V / (Vref − V)`.
In Ω mode "Read A0" returns the back-calculated resistance.

---

## 9. Persistence

### 9.1 NVS (Preferences, namespace `boatopenio`)

Survives firmware updates. Only changeable via the portal or clearable by a factory reset.

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `wifi_ssid` / `wifi_pass` | string | `""` | WiFi credentials |
| `mqtt_ip` | string | `192.168.1.100` | Broker address |
| `mqtt_port` | string | `1883` | Broker port (1–65535, validated) |
| `mqtt_user` / `mqtt_pass` | string | `""` | MQTT auth |
| `testmode` | bool | `true` | TEST mode active |
| `pitch_off` / `roll_off` | float | `0.0` | IMU mounting offset (°) |
| `pitch_inv` / `roll_inv` | bool | `false` | Axis inversion |
| `ap_ssid` | string | `BoatOpenIO-Setup` | AP name |
| `ap_pass` | string | `boatopenio` | AP password |
| `portal_user` | string | `admin` | Portal user |
| `portal_pass` | string | `""` | Portal password (empty = setup pending) |

### 9.2 LittleFS `/config.json`

Channel config (16 entries). Loaded at boot and **only rewritten when actually changed**
(saves flash). Local source: `firmware_src/data/config.json`.

| Field | Type | Description |
|-------|------|-------------|
| `klemme` | int 1–16 | Physical terminal (clamped to 1–16) |
| `sensor` | string | Sensor name → topic suffix when `topic` empty |
| `einheit` | string | Unit label (display only) |
| `faktor` | float | Voltage multiplier |
| `offset` | float | Additive offset |
| `aktiv` | bool | Read/publish channel |
| `topic` | string | Custom MQTT topic (empty = `boat/io/<sensor>`) |

The `ads`/`pin` fields exist in the format for historical reasons but are no longer used in
the single-ADS design.

---

## 10. Network resilience & OTA

- **AP always on:** the access point runs regardless of WiFi status – the portal stays
  reachable even without a router.
- **WiFi reconnect:** if STA is configured but disconnected, `loop()` forces a new attempt
  every **30 s**. Once connected, mDNS and OTA are brought up automatically – important for
  cold start after a power outage when the router boots slower than the ESP32.
- **OTA update:** over WiFi via the Arduino IDE (network port "BoatOpenIO at \<IP\>").
  Password = portal password (before setup, `boatopenio` as fallback). During upload the
  loop task is de-registered from the watchdog so the TWDT doesn't abort the update.
- **Watchdog:** 10 s task watchdog with panic reset on the loop task. If the loop hangs
  (e.g. a stuck I²C bus) the device reboots automatically.

---

## 11. Factory reset

| Action | Trigger | Effect |
|--------|---------|--------|
| **WiFi reset** | Hold GPIO0 **during operation** for 3 s | Clears only `wifi_ssid`/`wifi_pass`, restarts. Everything else stays. |
| **Full reset** | Hold GPIO0 **at boot** for 3 s | Clears all NVS + `config.json`. First-time setup required afterwards. |

After a full reset `config.json` may need re-uploading to LittleFS
(Arduino IDE → *ESP32 Sketch Data Upload*).

---

## 12. Timing & constants reference

| Constant | Value | Meaning |
|----------|-------|---------|
| `READ_INTERVAL` | 2000 ms | Read/publish channels |
| `IMU_INTERVAL` | 20 ms (50 Hz) | IMU sampling / impact check |
| `IMU_PUBLISH_INTERVAL` | 1000 ms | Pitch/roll over MQTT |
| `STATUS_INTERVAL` | 10 000 ms | Heartbeat (status/uptime/rssi/mode) |
| `ADS_SAMPLES` | 4 | Averaging per channel reading |
| `WDT_TIMEOUT` | 10 s | Task watchdog |
| `IMPACT_THRESHOLD` | 0.5 g | Impact threshold |
| `IMPACT_DECAY` | 8000 ms | Alarm fall-back time |
| MQTT buffer | 4096 B | Receive buffer (config payloads) |
| WiFi reconnect | 30 s | Interval when STA disconnected |
| MQTT backoff | 5 s / 30 s | after success / after failed attempt |

---

## 13. Troubleshooting (serial monitor, 115200 baud)

| Symptom | Cause / check |
|---------|---------------|
| `ADS1115 @ 0x48 nicht gefunden` | Check I²C wiring, address jumper, VCC/GND (see I²C scan in log). |
| `MPU6050 nicht gefunden – Fake-IMU aktiv` | Sensor at 0x68/0x69? Check AD0 level. Pitch/roll are then simulated. |
| Channels jump a lot | LIVE mode in the engine bay without a clean ground; check wiring/shielding (4-sample averaging is already on). |
| No MQTT data | Broker IP/port, reachability; is `boatopenio/status` = `online`? Check RSSI in log. |
| Portal asks for login, AP password doesn't work | Portal login ≠ AP password. Use the portal user/password from setup. |
| POST rejected with `403 CSRF blocked` | Request didn't originate from the portal page itself (foreign origin). Operate from the portal directly. |
| MQTT config update has no effect | Payload > 4096 B, invalid JSON, or sensor/topic emptied by the filter. |
| Impact alarm stuck at "true" | Should clear on reconnect; in TEST mode alarms live under `boat/test/alarm/*`. |

---

*Part of the "Logbook Without Posing" series · [BoatOpenIO](https://github.com/bigbrainlabs/BoatOpenIO)*
