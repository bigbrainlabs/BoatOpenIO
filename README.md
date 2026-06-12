# BoatOpenIO

**Universal Marine IO System – Open Source, modular, fully configurable**

> *"Plug in, configure, done."*

BoatOpenIO is an open hardware platform for sensor integration on boats. Each of the 16 sensor terminals can be freely assigned to any of the up to 16 input channels via software. Pluggable mini-boards handle signal conditioning — no soldering, no rewiring.

Part of the **[Logbook Without Posing](https://www.amazon.com/s?k=logbook+without+posing)** series – Volume 3.

---

## 📚 Support the project

This project is 100% open source and free. The complete story – every idea, mistake, detour and solution – is documented in the book series **"Logbook Without Posing"**.

If you find this project useful and want to support it: buying the books is the best way.

> *"You find this useful? Buy the book series. Then we're even."* 😄

**[👉 Logbook Without Posing on Amazon](https://amzn.to/4e5swN6)**

---

## ⚠️ Safety Warning

**Always verify which mini-board is plugged into which channel before applying power.**

Connecting 12V directly to a pull-up channel (instead of through a voltage divider) will destroy the MUX, all ADS1115 and the ESP32 through I2C chain reaction.

The v1.0 PCB includes Zener diode protection (3.3V) on all 16 signal inputs — but correct mini-board assignment is still essential.

---

## Concept

Commercial marine IO systems cost hundreds of euros and are closed black boxes.

BoatOpenIO costs a fraction of that — and anyone can adapt, extend, and improve it.

```
Sensor → Mini-Board → JST 3-Pin → PCB → Zener Protection → MUX → ADS1115 → ESP32 → MQTT → BoatOS
```

---

## Hardware – v1.0 PCB

The v1.0 PCB is now available! Gerber files in the `hardware/pcb/` folder — order directly from JLCPCB or any PCB manufacturer.

### Main Board Components

| Component | Description | Socketed |
|-----------|-------------|----------|
| ESP32 DevKit V4 (30-pin) | Microcontroller, WiFi, I2C, MUX control | ✅ |
| CD74HC4067 Breakout (BOB-09056) | 16:1 analog multiplexer | ✅ |
| ADS1115 Breakout (×1–4) | 16-bit ADC, I2C addresses 0x48–0x4B | ✅ |
| MPU6050 Breakout (GY-521) | 6-DOF IMU, tilt/impact detection | ✅ |
| AMS1117-3.3 Breakout | 5V → 3.3V regulator, LED indicator | ✅ |

**Everything socketed** — no component soldered directly. Faulty ESP32? Swap it out. Done.

### Protection Circuit

Every signal input has a hardware protection circuit:

```
Mini-Board SIG OUT → 1kΩ resistor → Zener 3.3V → MUX input Cx
                                         ↓
                                        GND
```

16× BZX55C3V3 (3.3V Zener, DO-35 THT) + 16× 1kΩ resistor on the PCB.

### Signal Connector: JST 3-Pin (×16)

```
Pin 1: Signal IN   → raw input from sensor (up to 12V)
Pin 2: GND
Pin 3: Signal OUT  → conditioned signal to MUX (max. 3.3V after protection)
```

### VCC Rails (left side of PCB, JST 2-pin, pointing upward)

```
7× 3.3V rail  → for ESP01 mini-boards and passive boards
3× 5V rail    → for Arduino Nano mini-boards
```

### I2C Address Assignment (fixed on PCB)

| ADS | ADDR pin | I2C address |
|-----|----------|-------------|
| ADS1 | → GND | 0x48 |
| ADS2 | → VDD | 0x49 |
| ADS3 | → SDA | 0x4A |
| ADS4 | → SCL | 0x4B |

### GPIO Assignment (ESP32)

| GPIO | Function |
|------|----------|
| GPIO14 | MUX S0 |
| GPIO27 | MUX S1 |
| GPIO26 | MUX S2 |
| GPIO25 | MUX S3 |
| GPIO21 | I2C SDA |
| GPIO22 | I2C SCL |

---

## Mini-Boards (pluggable)

| Type | Code | Use case |
|------|------|----------|
| Voltage divider | VT | 12V VDO sensors → 3.3V |
| Pull-down | PD | Digital sensors, switches |
| Pull-up | PU | Open-collector outputs |
| Schmitt trigger | ST | Magnetic pickup sensors, 50Hz generators |
| Pulse board | ISP | RPM, flow (Arduino Nano / ESP01) |
| Optocoupler | OPT | Galvanic isolation (W-terminal RPM) |
| Direct / jumper | DIR | 3.3V signals directly |
| GND collector | GND | Multiple sensor ground loops → single reference |

**Design and submit your own mini-boards** — pull requests welcome!

---

## Multiplexer Logic

```
S3 S2 S1 S0 → active channel
 0  0  0  0 → terminal 1  (MUX C15 on PCB)
 0  0  0  1 → terminal 2  (MUX C14 on PCB)
 ...
 1  1  1  1 → terminal 16 (MUX C0 on PCB)
```



---

## Software Configuration

```json
{
  "kanaele": [
    {"klemme": 1, "ads": 1, "pin": "A0", "sensor": "battery1",     "einheit": "V",   "faktor": 4.7,  "offset": 0},
    {"klemme": 2, "ads": 2, "pin": "A3", "sensor": "oil_temp",     "einheit": "°C",  "faktor": 1.0,  "offset": 0},
    {"klemme": 3, "ads": 1, "pin": "A1", "sensor": "oil_pressure", "einheit": "bar", "faktor": 1.0,  "offset": 0},
    {"klemme": 4, "ads": 3, "pin": "A0", "sensor": "tank",         "einheit": "%",   "faktor": 1.0,  "offset": 0},
    {"klemme": 5, "ads": 2, "pin": "A1", "sensor": "rpm",          "einheit": "rpm", "faktor": 1.0,  "offset": 0}
  ]
}
```

---

## MQTT Topics

```
boat/io/battery1       → 12.43
boat/io/oil_temp       → 87
boat/io/oil_pressure   → 3.2
boat/io/tank           → 48
boat/io/rpm            → 1450
boat/io/pitch          → 1.2
boat/io/roll           → -0.5
boat/io/impact         → LIGHT
boatopenio/status      → online
```

---

## Firmware Features

| Feature | Description |
|---------|-------------|
| Web UI | Configuration at `192.168.4.1`, bilingual DE/EN |
| WiFiManager | AP on first start for WiFi setup |
| MQTT | Publish all sensor values, LWT, auth support |
| OTA | Over-the-air firmware updates |
| mDNS | Reachable as `boatopenio.local` |
| Impact Detection | MPU6050 G-force monitoring with severity levels |
| Watchdog | Automatic restart on firmware hang |
| LittleFS | Persistent configuration storage |

---

## Supported Sensors

| Sensor | Type | Mini-Board |
|--------|------|------------|
| VDO temperature (NTC) | Resistance | VT |
| VDO oil pressure | Linear resistance | VT |
| VDO tank sender | Potentiometer | VT |
| Battery voltage 12V | Voltage divider | VT (10k/2.2k) |
| RPM (W-terminal) | Pulse/AC | OPT + ISP |
| RPM (magnetic pickup) | AC signal | ST + ISP |
| Fuel flow | Pulse | ISP |
| Bilge sensor | Digital | PD |
| Door/hatch contact | Digital | PD |
| DS18B20 temperature | Digital 1-Wire | custom board |

---

## Bill of Materials – v1.0 PCB Kit

### Active Components (breakout boards, socketed)

| Qty | Component | Notes |
|-----|-----------|-------|
| 1× | ESP32 DevKit V4 (30-pin) | Main controller |
| 4× | ADS1115 Breakout (GY-ADS1115) | 16-bit ADC |
| 1× | CD74HC4067 Breakout (BOB-09056) | 16:1 MUX |
| 1× | MPU6050 Breakout (GY-521) | IMU |
| 1× | AMS1117-3.3 Breakout | Voltage regulator |

### Protection Circuit (THT, 16× per channel)

| Qty | Component | Value | Package |
|-----|-----------|-------|---------|
| 16× | Resistor | 1kΩ | THT axial |
| 16× | Zener diode | BZX55C3V3 (3.3V) | DO-35 |

### Connectors (THT)

| Qty | Component | Description |
|-----|-----------|-------------|
| 1× | Screw terminal 1×16 | Sensor terminals K1–K16, 5.08mm pitch |
| 1× | Pin header 2×16 | SIG IN/OUT, upward facing |
| 10× | JST PH 2-pin (2.0mm) | VCC/GND rails (7× 3.3V + 3× 5V) |
| 1× | JST PH 2-pin (2.0mm) | Power input 5V |

### Sockets

| Qty | Component | Description |
|-----|-----------|-------------|
| 2× | Female header 1×19 | ESP32 DevKit socket |
| 4× | Female header 1×08 | ADS1115 breakout sockets |
| 1× | Female header 1×08 | MPU6050 socket |
| 1× | Female header 1×16 | MUX breakout (top pins C0–C15) |
| 1× | Female header 1×08 | MUX breakout (bottom pins) |
| 1× | Female header 1×03 | AMS1117 breakout |

### PCB

| Qty | Component | Description |
|-----|-----------|-------------|
| 1× | BoatOpenIO v1.0 PCB | Gerber files in `hardware/pcb/` |

### Bill of Materials with Links

> **Note on prices:** All prices refer to sets and pack sizes as available. A single board costs considerably less – the sets are enough for many further projects.

| Component | Amazon | AliExpress | Amazon price | AliExpress price |
|-----------|--------|------------|-------------|-----------------|
| ESP32 DevKit V4 (30-pin) | [Amazon](https://amzn.to/4ged4iW) | [AliExpress](https://s.click.aliexpress.com/e/_c4pqHci1) | ~€9 | ~€5 |
| ADS1115 Breakout (4×) | [Amazon](https://amzn.to/4gg8mRQ) | [AliExpress](https://s.click.aliexpress.com/e/_c35sBSYd) | ~€12 | ~€7 |
| CD74HC4067 MUX Breakout | [Amazon](https://amzn.to/4vPmaHH) | [AliExpress](https://s.click.aliexpress.com/e/_c3RLw85R) | ~€6 | ~€5 |
| MPU6050 Breakout (GY-521) | [Amazon](https://amzn.to/3S3gGKJ) | [AliExpress](https://s.click.aliexpress.com/e/_c3tzzLRn) | ~€5 | ~€2 |
| AMS1117-3.3 Breakout | [Amazon](https://amzn.to/4gfqqeL) | [AliExpress](https://s.click.aliexpress.com/e/_c33Kg9MD) | ~€5 | ~€3 |
| 1kΩ Resistors (set) | [Amazon](https://amzn.to/4xpBFro) | [AliExpress](https://s.click.aliexpress.com/e/_c4BhD2g5) | ~€14 | ~€6 |
| BZX55C3V3 Zener Diodes (100×) | [Amazon](https://amzn.to/4v9OTqK) | [AliExpress](https://s.click.aliexpress.com/e/_c3ra9r0h) | ~€8 | ~€2 |
| Screw Terminals 5.08mm | [Amazon](https://amzn.to/4e6bJcN) | [AliExpress](https://s.click.aliexpress.com/e/_c4X0xKf3) | ~€8 | ~€7 |
| JST PH 2.0mm 2-pin | [Amazon](https://amzn.to/43y2Suh) | [AliExpress](https://s.click.aliexpress.com/e/_c4BoCB7P) | ~€6 | ~€3 |
| JST PH 2.0mm 3-pin | [Amazon](https://amzn.to/4ush82N) | [AliExpress](https://s.click.aliexpress.com/e/_c3LlQpYp) | ~€8 | ~€3 |
| Female Headers | [Amazon](https://amzn.to/3S1rXLD) | [AliExpress](https://s.click.aliexpress.com/e/_c3B9h4ip) | ~€9 | ~€2 |
| Perfboard (set) | [Amazon](https://amzn.to/3SAIJ4k) | [AliExpress](https://s.click.aliexpress.com/e/_c4dYUhKH) | ~€14 | ~€6 |
| PCB (5 pcs JLCPCB) | Gerber files in `hardware/pcb/` | | ~€7 | ~€7 |
| **Total** | | | **~€111** | **~€58** |

> Amazon: faster delivery. AliExpress: significantly cheaper, 2–4 weeks delivery.

Commercial marine IO systems: €200–500.

---

## Directory Structure

```
BoatOpenIO/
├── README.md                          ← English (this file)
├── README_de.md                       ← German version
├── LICENSE                            ← GPL-3.0
├── boatopenio_final_architektur.svg   ← system architecture diagram
├── firmware_src/
│   ├── firmware_src.ino               ← main firmware
│   ├── webui.h                        ← web UI & captive portal
│   └── data/
│       └── config.json                ← default channel config
├── hardware/
│   ├── boardopenio/                   ← KiCad PCB project
│   │   ├── boardopenio.kicad_sch
│   │   ├── boardopenio.kicad_pcb
│   │   ├── boardopenio.kicad_pro
│   │   └── boardopenio.kicad_prl
│   ├── mainboard/
│   │   ├── BoatOpenIO-v1.0-gerber.zip ← ready to order
│   │   └── schaltplan_mainboard.jpg
│   └── boatopenio-gehaeuse-draufsicht.svg
├── images/                            ← build photos
└── docs/
    ├── setup.md
    ├── setup_de.md
    ├── configuration.md
    └── configuration_de.md
```

---

## Integration with BoatOS

BoatOpenIO sends all values via MQTT. BoatOS auto-discovers all topics and displays them in the dashboard. No manual configuration needed.

**➡️ [github.com/bigbrainlabs/BoatOS](https://github.com/bigbrainlabs/BoatOS)**

---

## License

**GNU General Public License v3.0 (GPL-3.0)**

Free to use, modify, distribute — derivative works must also be GPL-3.0. See [LICENSE](LICENSE).

---

## Part of the Series

**Logbook Without Posing** — authentic DIY projects around boating.

> *"Build it yourself and you know what's in it — and what it cost."*

📖 [Book series on Amazon](https://amzn.to/4e5swN6) · 🔧 [BoatOS](https://github.com/bigbrainlabs/BoatOS) · ⚡ [BoatOpenIO](https://github.com/bigbrainlabs/BoatOpenIO)
