# BoatOpenIO

**Universal Marine IO System – Open Source, modular, fully configurable**

> *"Plug in, configure, done."*

BoatOpenIO is an open hardware platform for sensor integration on boats. Each of the 16 sensor terminals can be freely assigned to any of the up to 16 input channels via software. Pluggable mini-boards handle signal conditioning — no soldering, no rewiring.

Part of the **[Logbuch ohne Pose](https://github.com/bigbrainlabs/logbuch-ohne-pose)** series.

---

## Concept

Commercial marine IO systems cost hundreds of euros and are closed black boxes.

BoatOpenIO costs a fraction of that — and anyone can adapt, extend, and improve it.

```
Sensor → Mini-Board Housing → JST → Main Board → MUX → ADS1115 → ESP32 → MQTT → BoatOS
```

---

## Hardware Architecture

### Main Board

| Component | Description | Socketed |
|-----------|-------------|----------|
| ESP32 WROOM-32 | Microcontroller, WiFi, I2C, MUX control | ✅ |
| CD74HC4067 | 16:1 analog multiplexer | ✅ |
| ADS1115 (1–4×) | 16-bit ADC, I2C, addresses 0x48–0x4B | ✅ |
| MPU6050 | 6-DOF IMU, tilt/acceleration | ✅ |

**Everything socketed** — no component is soldered directly. Faulty ESP32? Swap it out. Done.

### Housing Concept

Main board and mini-boards are housed in **separate enclosures**:

- **Main housing:** ESP32, MUX, ADS1115, MPU6050 — closed, protected
- **Mini-board housing:** one per channel — pluggable on the outside of the main housing
- **Connection:** JST 2-pin connector — just unplug, swap, done

### 16 Channels – Connector Layout

**Signal connector: JST 2-pin** (×16, one per channel):
```
Pin 1: Signal IN   → input from sensor (raw, can be 12V)
Pin 2: Signal OUT  → conditioned signal to MUX (max. 3.3V)
```

**VCC rails on main housing** (only for active mini-boards):
```
Rail A: JST 2-pin → GND + 3.3V  (for ESP01 boards)
Rail B: JST 2-pin → GND + 5V    (for Arduino Nano boards)
```

**Who needs what:**

| Mini-Board | Signal JST | VCC JST |
|---|---|---|
| DIR Direct | ✅ 2-pin | – |
| VT Voltage divider | ✅ 2-pin | Rail A/B (VCC blind) |
| PD/PU Pull-down/-up | ✅ 2-pin | Rail A/B (VCC blind) |
| OPT Optocoupler | ✅ 2-pin | Rail A/B (VCC blind) |
| ISP Arduino Nano | ✅ 2-pin | Rail B (5V) |
| ISP ESP01 | ✅ 2-pin | Rail A (3.3V) |

Maximum 2 connectors per channel, minimum 1. Passive mini-boards that only need GND use Rail A or B with a blind VCC pin.

### Mini-Boards (pluggable)

| Type | Code | Use case |
|------|------|----------|
| Voltage divider | VT | 12V VDO sensors → 3.3V |
| Pull-down | PD | Digital sensors, switches |
| Pull-up | PU | Open-collector outputs |
| Schmitt trigger | ST | Magnetic pickup sensors, 50Hz generators |
| Pulse board | ISP | RPM, flow (Arduino Nano / ESP01) |
| Optocoupler | OPT | Galvanic isolation |
| Direct / jumper | DIR | 3.3V signals directly |

**Design and submit your own mini-boards** — pull requests welcome!

### Multiplexer Logic

The CD74HC4067 has 16 inputs and a shared SIG output.

4 control pins (S0–S3) from the ESP32 select the active channel via binary code:

```
S3 S2 S1 S0 → active channel
 0  0  0  0 → terminal 1
 0  0  0  1 → terminal 2
 ...
 1  1  1  1 → terminal 16
```

The SIG pin is connected to all ADS1115 inputs in parallel. Software selects which ADS channel to read.

### Up to 4× ADS1115

```
ADS1 @ I2C 0x48  →  A0, A1, A2, A3  (4 channels)
ADS2 @ I2C 0x49  →  A0, A1, A2, A3  (4 channels)
ADS3 @ I2C 0x4A  →  A0, A1, A2, A3  (4 channels)
ADS4 @ I2C 0x4B  →  A0, A1, A2, A3  (4 channels)
                                      = 16 targets
```

---

## Software Configuration

Each terminal is assigned to an ADS channel and sensor type via a JSON config:

```json
{
  "kanaele": [
    {"klemme": 1, "ads": 1, "pin": "A0", "sensor": "battery1",     "einheit": "V"},
    {"klemme": 2, "ads": 2, "pin": "A3", "sensor": "oil_temp",     "einheit": "°C"},
    {"klemme": 3, "ads": 1, "pin": "A1", "sensor": "oil_pressure", "einheit": "bar"},
    {"klemme": 4, "ads": 3, "pin": "A0", "sensor": "tank",         "einheit": "%"},
    {"klemme": 5, "ads": 2, "pin": "A1", "sensor": "rpm",          "einheit": "rpm"}
  ]
}
```

Change configuration → reflash → done. No rewiring, no soldering.

---

## MQTT Topics

Every channel automatically gets its own MQTT topic:

```
boat/io/battery1       → 12.43
boat/io/oil_temp       → 87
boat/io/oil_pressure   → 3.2
boat/io/tank           → 48
boat/io/rpm            → 1450
boat/io/pitch          → 1.2
boat/io/roll           → -0.5
boatopenio/status      → online
boatopenio/uptime      → 3600
```

Compatible with **[BoatOS](https://github.com/bigbrainlabs/BoatOS)** — all topics are auto-discovered and shown in the dashboard.

---

## Firmware Features

| Feature | Description |
|---------|-------------|
| Web UI | Configuration portal at `192.168.4.1`, bilingual DE/EN |
| Captive Portal | Auto-opens on mobile when connecting to the AP |
| WiFi AP+STA | Always-on access point + optional router connection |
| MQTT | Publish all sensor values, LWT, auth support |
| OTA | Over-the-air firmware updates via ArduinoOTA |
| mDNS | Reachable as `boatopenio.local` on the local network |
| Test mode | Simulated sensor data for commissioning without hardware |
| IMU calibration | Gyro bias at boot + mounting offset via web UI |
| ADS diagnostics | Live raw voltage readout for all 16 ADC inputs |
| Watchdog | Automatic restart on firmware hang |
| Factory reset | GPIO0 hold at boot clears all settings |

---

## Pulse Boards (ISP)

For RPM and flow — signals that require fast pulse counting — there are pluggable pulse boards.

An Arduino Nano or ESP01 sits in the mini-board housing, counts pulses, calculates the value and outputs it as an analog voltage to the MUX.

```
Sensor (raw pulse) → ISP housing (Arduino Nano) → calculated value → JST → MUX → ADS1115 → ESP32
```

**Advantage:** The ESP32 is not burdened by interrupts. Timing-critical tasks are offloaded.

Custom ISP boards for other pulse sources (e.g. 50Hz generator, magnetic pickup): pull request welcome.

---

## Supported Sensors

| Sensor | Type | Mini-Board |
|--------|------|------------|
| VDO temperature (NTC) | Resistance | VT |
| VDO oil pressure | Linear resistance | VT |
| VDO tank sender | Potentiometer | VT |
| Battery voltage 12V | Voltage | VT |
| RPM (W-terminal) | Pulse | ISP |
| RPM (magnetic pickup) | AC signal | ST + ISP |
| 50Hz generator | AC signal | ST + ISP |
| Fuel flow | Pulse | ISP |
| Bilge sensor | Digital | PD |
| Door contact | Digital | PD |
| DS18B20 temperature | Digital 1-Wire | custom board |
| ... | ... | custom board |

---

## Cost

| Component | Price |
|-----------|-------|
| ESP32 WROOM-32 | ~€5 |
| CD74HC4067 MUX | ~€1 |
| ADS1115 (4×) | ~€8 |
| MPU6050 | ~€2 |
| JST 2-pin connectors (×16 + rails) | ~€3 |
| Sockets, housing, PCB | ~€10 |
| **Total** | **~€29** |

Commercial marine IO systems: €200–500.

---

## Directory Structure

```
BoatOpenIO/
├── README.md
├── README_de.md                        ← German version
├── LICENSE
├── .gitignore
├── boatopenio_final_architektur.svg    ← architecture diagram
├── images/                             ← board photos
│   ├── IMG_20260604_112443.jpg
│   ├── IMG_20260604_133528.jpg
│   ├── IMG_20260604_133741.jpg
│   └── IMG_20260604_133746.jpg
├── firmware_src/
│   ├── firmware_src.ino                ← main firmware (v2.2)
│   ├── webui.h                         ← web UI, captive portal, REST API
│   └── data/
│       └── config.json                 ← default channel config (LittleFS)
└── docs/
    ├── setup.md                        ← setup guide (EN)
    ├── setup_de.md                     ← setup guide (DE)
    ├── configuration.md                ← config reference (EN)
    └── configuration_de.md             ← config reference (DE)
```

---

## Custom Mini-Boards

BoatOpenIO is an open platform. Anyone can design and submit custom mini-boards.

**Requirements:**
- JST 2-pin signal connector: Signal IN (pin 1), Signal OUT (pin 2)
- GND and VCC optionally from the rails on the main housing
- Passive boards: only signal JST required, GND from rail if needed
- Active boards (ISP): signal JST + VCC JST from Rail A (3.3V) or B (5V)

**Submission:**
1. KiCad files in `hardware/mini-platinen/YOUR-TYPE/`
2. Short description in `docs/mini-boards.md`
3. Pull request

---

## Integration with BoatOS

BoatOpenIO sends all values via MQTT. BoatOS auto-discovers new topics and displays them in the dashboard.

No manual configuration needed. Connect the sensor, MQTT runs, dashboard shows.

**➡️ BoatOS: [github.com/bigbrainlabs/BoatOS](https://github.com/bigbrainlabs/BoatOS)**

---

## License

This project is licensed under the **GNU General Public License v3.0 (GPL-3.0)**.

You are free to use, modify, and distribute this software and hardware designs, provided that any derivative work is also released under the GPL-3.0. See the [LICENSE](LICENSE) file for the full license text.

---

## Part of the Series

**Logbuch ohne Pose** — authentic DIY projects around boating.

> *"Build it yourself and you know what's in it — and what it cost."*

📖 [Book series on Amazon](https://www.amazon.de/s?k=logbuch+ohne+pose) · 🔧 [BoatOS](https://github.com/bigbrainlabs/BoatOS) · 📡 [BoatOpenIO](https://github.com/bigbrainlabs/BoatOpenIO)
