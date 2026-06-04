# BoatOpenIO – Configuration Reference

There are three ways to configure BoatOpenIO. They complement each other — you don't have to use all of them.

---

## Overview

| What | Where stored | How to change |
|------|-------------|---------------|
| Channel mapping | LittleFS (`config.json`) | Web portal · MQTT · LittleFS upload |
| WiFi credentials | NVS (Preferences) | Web portal |
| MQTT connection | NVS (Preferences) | Web portal |
| AP credentials (SSID + password) | NVS (Preferences) | Web portal (Security card) · first-time setup |
| Portal credentials (user + password) | NVS (Preferences) | Web portal (Security card) · first-time setup |
| Test/Live mode | NVS (Preferences) | Web portal |
| IMU mounting offset | NVS (Preferences) | Web portal (IMU calibration) |
| Gyro bias | RAM only | Measured automatically at every boot |

NVS survives firmware updates. LittleFS `config.json` is only overwritten when you explicitly save via portal, MQTT, or re-upload the file.

---

## 1. Web Portal

### Access

| Address | Condition |
|---------|-----------|
| `http://192.168.4.1` | Always — via the built-in AP |
| `http://boatopenio.local` | When connected to local network (mDNS) |
| `http://<IP>` | When connected to local network (IP shown in Serial Monitor) |

The portal requires **HTTP Basic Auth** after first-time setup. Enter the portal username and password set during `/setup`. This is separate from the AP password.

### First-Time Setup (`/setup`)

On first boot, all routes redirect to `/setup`. The portal is not accessible until a portal password is set here.

| Field | Requirement |
|-------|-------------|
| Portal User | required |
| Portal Password | min. 6 characters |
| AP Name (SSID) | required |
| AP Password | min. 8 characters |

---

### WiFi & MQTT

```
WiFi SSID       your router's network name
WiFi Password   leave blank = keep existing
MQTT Server     IP or hostname, e.g. 192.168.1.100
MQTT Port       default 1883
MQTT User       leave blank if no auth
MQTT Password   leave blank = keep existing
```

→ Saved to NVS. Takes effect after restart.

---

### Channel Assignment

Each of the 16 terminals can be configured independently:

```
Sensor    MQTT topic suffix → boat/io/<sensor>
ADS       ADS1115 chip number: 1 (0x48) · 2 (0x49) · 3 (0x4A) · 4 (0x4B)
Pin       ADS input: A0 · A1 · A2 · A3
Factor    output = raw_voltage × factor + offset
Offset    additive offset (applied after factor)
Unit      string label, e.g. V · °C · bar · %  (display only)
Active    only active channels are read and published to MQTT
```

→ Saved to LittleFS `config.json`. Takes effect immediately, no restart needed.

---

### IMU Calibration

```
Set Zero Now    stores current pitch/roll as mounting offset in NVS
                applied to all subsequent readings and MQTT publishes
```

Gyro bias (automatic): measured at boot over 100 samples (~0.5 s).  
Shown in the calibration card: `Gyro Bias: X=... Y=... Z=... rad/s`

The calibration card shows three rows for Pitch and Roll:

| Row | Description |
|-----|-------------|
| Raw | Gyro-bias-corrected angle, before mounting offset |
| Corrected | Final value — what gets published to MQTT |
| Offset | Stored mounting offset |

---

### Security

AP and portal credentials can be changed at any time in the **Security** card.

| Field | Requirement | Stored in |
|-------|-------------|-----------|
| AP Name (SSID) | required | NVS `ap_ssid` |
| AP Password | min. 8 chars, blank = keep | NVS `ap_pass` |
| Portal User | required | NVS `portal_user` |
| Portal Password | min. 6 chars, blank = keep | NVS `portal_pass` |

A **yellow warning banner** is shown at the top of the portal as long as the AP password equals the factory default (`boatopenio`).

---

### API Endpoints

All endpoints are accessible without additional auth once the browser session is authenticated via Basic Auth.

| Endpoint | Method | Returns |
|----------|--------|---------|
| `/api/values` | GET | Active channel values + pitch + roll (corrected) |
| `/api/raw` | GET | Raw voltages for all 16 ADS inputs (all 4×4 pins) |
| `/api/imu` | GET | Pitch/roll raw, corrected, offset, gyro bias |
| `/calibrate` | POST | Sets current pitch/roll as mounting offset, returns new offsets |

---

## 2. config.json (LittleFS)

Stored on the ESP32's flash filesystem. Loaded at boot.

**Location on device:** `/config.json`  
**Local source:** `firmware_src/data/config.json`

### Structure

```json
{
  "kanaele": [
    {
      "klemme":  1,
      "ads":     1,
      "pin":     0,
      "sensor":  "battery_starter",
      "einheit": "V",
      "faktor":  3.127,
      "offset":  0.0,
      "aktiv":   true
    },
    {
      "klemme":  2,
      "ads":     1,
      "pin":     1,
      "sensor":  "battery_consumer",
      "einheit": "V",
      "faktor":  3.127,
      "offset":  0.0,
      "aktiv":   false
    }
  ]
}
```

### Field Reference

| Field | Type | Description |
|-------|------|-------------|
| `klemme` | int 1–16 | Physical terminal number on the board |
| `ads` | int 1–4 | ADS1115 chip: 1=0x48, 2=0x49, 3=0x4A, 4=0x4B |
| `pin` | int 0–3 | ADS input pin: 0=A0, 1=A1, 2=A2, 3=A3 |
| `sensor` | string | MQTT topic suffix, published as `boat/io/<sensor>` |
| `einheit` | string | Unit label (display only) |
| `faktor` | float | Voltage multiplier: `value = voltage × faktor + offset` |
| `offset` | float | Additive offset after factor |
| `aktiv` | bool | `true` = channel is read and published |

### Voltage → Physical Value

Use the **Calibration Calculator** in the web portal:
```
Enter: real value (measured with multimeter) + ADS pin voltage
→ factor is calculated automatically
```

Manual formula:
```
faktor = real_value / ads_pin_voltage
```

### How to Upload

1. Edit `firmware_src/data/config.json`
2. Arduino IDE → **Tools → ESP32 Sketch Data Upload**

Or: configure via the web portal — changes are saved to LittleFS automatically on every save.

---

## 3. MQTT

### Publish (ESP32 → Broker)

| Topic | Payload | Retained | Interval |
|-------|---------|----------|----------|
| `boat/io/<sensor>` | float value | no | 2 s |
| `boat/io/pitch` | float °, corrected | yes | 200 ms |
| `boat/io/roll` | float °, corrected | yes | 200 ms |
| `boatopenio/status` | `online` / `offline` | yes | 10 s + LWT |
| `boatopenio/uptime` | seconds since boot | no | 10 s |
| `boatopenio/rssi` | WiFi signal dBm | no | 10 s |
| `boatopenio/mode` | `TEST` / `LIVE` | yes | 10 s |
| `boat/alarm/impact_g` | peak G value | yes | on event |
| `boat/alarm/impact_severity` | `LEICHT` / `MITTEL` / `STARK` / `KRITISCH` | yes | on event |
| `boat/alarm/impact_active` | `true` / `false` | yes | on event |

### Subscribe (Broker → ESP32)

#### Update channel config

```
Topic:   boatopenio/config
Payload: full or partial config.json structure (JSON)
```

Overwrites the current channel configuration in memory and saves to LittleFS. Partial updates are supported — only fields present in the payload are changed.

**Example — activate one channel:**
```json
{
  "kanaele": [
    {"klemme": 3, "sensor": "oil_pressure", "faktor": 1.25, "aktiv": true}
  ]
}
```

#### Commands

```
Topic:   boatopenio/cmd/#
```

Reserved for future commands. Currently subscribed but not yet processed.

---

## 4. NVS (Non-Volatile Storage)

Stored in the ESP32's NVS partition under namespace `boatopenio`. Survives firmware flashing (unless a full factory reset is performed).

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `wifi_ssid` | string | `""` | WiFi network name |
| `wifi_pass` | string | `""` | WiFi password |
| `mqtt_ip` | string | `192.168.1.100` | MQTT broker address |
| `mqtt_port` | string | `1883` | MQTT broker port |
| `mqtt_user` | string | `""` | MQTT username |
| `mqtt_pass` | string | `""` | MQTT password |
| `testmode` | bool | `true` | Test mode active |
| `pitch_off` | float | `0.0` | IMU pitch mounting offset (°) |
| `roll_off` | float | `0.0` | IMU roll mounting offset (°) |
| `ap_ssid` | string | `BoatOpenIO-Setup` | AP network name |
| `ap_pass` | string | `boatopenio` | AP password |
| `portal_user` | string | `admin` | Web portal username |
| `portal_pass` | string | `""` | Web portal password (empty = first-time setup required) |

NVS is **not** accessible directly. All values are managed via the web portal or cleared by a factory reset.

---

## 5. Factor & Offset Calculation

The conversion formula for every channel:

```
published_value = raw_voltage × faktor + offset
```

### Common examples

**12V battery via resistor divider (R1=30kΩ, R2=10kΩ):**
```
ADS reads max 3.127V when battery is at 12.5V
faktor = 12.5 / 3.127 ≈ 4.0
offset = 0.0
```

**VDO oil pressure sender (0–150 psi, 10–180Ω):**
```
Requires voltage divider board (VT) first
Measure actual voltage at ADS pin and real pressure with gauge
faktor = real_pressure / ads_voltage
```

**NTC temperature sensor (linearized range):**
```
Use the calibration calculator in the web portal
Measure two points, calculate factor and offset from the linear fit
```

**Direct 3.3V signal:**
```
faktor = 1.0
offset = 0.0
```
