# BoatOpenIO

**Universal Marine IO System – Open Source, modular, fully configurable**

> *"Plug in, configure, done."*

BoatOpenIO is an open hardware platform for sensor integration on boats. Each of the 16 sensor terminals can be freely assigned to any of the up to 16 input channels via software. Pluggable mini-boards handle signal conditioning — no soldering, no rewiring.

Part of the **[Logbook Without Posing](https://www.amazon.com/s?k=logbook+without+posing)** series – Volume 3.

---

## 📚 Support the project

This project is 100% open source and free. The complete story – every idea, mistake, detour and solution – is documented in the book series **"Logbook Without Posing"**.

If you find this project useful and want to support it: buying the books is the best way.

> *"Found this useful? Grab a book — or the whole series. You'll be supporting further development."*

**[👉 Logbook Without Posing on Amazon](https://amzn.to/4e5swN6)**

---

## ⚠️ Safety Warning

**Always verify which mini-board is plugged into which channel before applying power.**

Connecting 12V directly to a pull-up channel (instead of through a voltage divider) will destroy the MUX, all ADS1115 and the ESP32 through I2C chain reaction.

The input board includes protection circuitry (3.3V) on all 16 signal inputs — but correct mini-board assignment is still essential.

---

## Concept

Commercial marine IO systems cost hundreds of euros and are closed black boxes.

BoatOpenIO costs a fraction of that — and anyone can adapt, extend, and improve it.

```
Sensor → Mini-Board → JST 3-Pin → PCB → Zener Protection → MUX → ADS1115 → ESP32 → MQTT → BoatOS
```

---

## Who Is This For?

**If your boat was built before 2000, this project was built for you.**

Boats from that era came with **analog VDO sensors** — oil pressure, oil temperature, coolant temperature, tank level, RPM. These sensors are robust, time-tested, and still doing their job perfectly. The problem: no modern system speaks their language.

NMEA 2000 gateways that bridge analog signals to modern boat networks exist — but they cost **300–500€**, are closed proprietary boxes, and give you zero insight or control over what happens inside.

**BoatOpenIO is an open analog marine gateway.**

Plug in your existing VDO sensors. Get clean MQTT output. Forward to Signal K or BoatOS. Done — without replacing a single sensor, without rewiring, without a black box.

```
VDO sensor → BoatOpenIO → MQTT → Signal K / BoatOS → modern dashboard
```

> **Built for boats built before 2000 that will sail for another 20 years — and deserve a modern dashboard without a full refit.**

---

## Hardware – v2 PCB

Four separate boards, connected via IDC ribbon cables: **input board**, **main board**, **ESP32 adapter board**, and **VCC distributor**. Gerber files ready to order in `hardware/v2/*/gerber/`.

```
Input Board ──20-pin ribbon──> Main Board <──16-pin ribbon── ESP32 Adapter Board
                                    │
                               3-pin JST
                                    │
                               VCC Board
```

### Main Board Components

| Component | Description | Socketed |
|-----------|-------------|----------|
| ESP32-DEVKITC-32D-F (38-pin) | Microcontroller, WiFi, I2C, MUX control – on its own adapter board | ✅ |
| CD74HC4067 Breakout | 16:1 analog multiplexer | ✅ |
| ADS1115 Breakout (1×) | 16-bit ADC | ✅ |
| MPU6050 Breakout (GY-521) | 6-DOF IMU, tilt/impact detection | ✅ |
| AMS1117-3.3 Breakout | 5V → 3.3V regulator | ✅ |

**Everything socketed** — no component soldered directly. Faulty ESP32? Swap the adapter board. Done.

The ESP32 sits on its own dedicated adapter board. The first adapter board is designed for the 38-pin ESP32 DevKitC-32D — swappable and ready for future ESP32 variants.

### Protection Circuit (Input Board)

Every one of the 16 signal inputs has a hardware protection circuit, directly on the input board:

```
Sensor → Screw terminal → R_Pack08 (resistor network) → SP0503BAHTG (Zener array, 3.3V) → MUX signal
```

2× Bourns 4116R-1-102LF (R_Pack08, DIL-16, isolated) + 6× SP0503BAHTG (SOT-143, 3 channels each).

> ⚠️ **Pull-up voltage is 3.3V** — the ADS1115 VDD is at 3.3V; higher voltages on the inputs risk component damage.

#### Running without the protection circuit (not recommended)

If the Bourns or Zener components are unavailable and you want to get the board running first, the resistor networks can be bypassed — but understand what you are giving up.

**How to bypass the Bourns 4116R-1-102LF:**  
The DIL-16 package contains 8 isolated resistors. Each resistor connects a pin on the left row to the opposing pin on the right row:

```
Pin 1  ──[R]──  Pin 16
Pin 2  ──[R]──  Pin 15
Pin 3  ──[R]──  Pin 14
  ...             ...
Pin 8  ──[R]──  Pin 9
```

To bypass: solder a short jumper wire across each pair (8 bridges per chip, 16 total for both R_Pack08). The signal then passes through without series resistance. The SP0503BAHTG Zener pads can simply be left empty.

> 🔴 **Without this protection, all 16 inputs are exposed.** A wrong mini-board, a wiring mistake, or any brief overvoltage can destroy the MUX, ADS1115, and ESP32 in a chain reaction — all three at once. Only bypass for bench tests with known, clean signals. Fit the real parts before installing on the boat.

### Input Board – Connectors

```
16× screw terminal 5.08mm   → raw sensor signal
20-pin IDC header            → connection to main board (SIG1–16 + GND)
```

### VCC Board – Connectors

The VCC board distributes power to the mini-boards via JST connectors. It connects to the main board via a 3-pin JST cable (5V, 3.3V, GND).

```
7× JST 2-pin (3.3V)         → VCC rail for passive/ESP01 mini-boards
3× JST 2-pin (5V)           → VCC rail for Arduino Nano mini-boards
3-pin JST                   → connection to main board (5V · 3.3V · GND)
```

### I2C Address

Only 1× ADS1115 fitted (the MUX routes all 16 channels onto a single signal line) → fixed address **0x48**.

### GPIO Assignment (ESP32, via 16-pin main board ↔ adapter board connector)

| GPIO | Function |
|------|----------|
| GPIO21 | I2C SDA |
| GPIO22 | I2C SCL |
| GPIO14 | MUX S0 |
| GPIO27 | MUX S1 |
| GPIO26 | MUX S2 |
| GPIO25 | MUX S3 |
| GPIO4 / 5 / 13 / 16 / 17 / 18 / 19 / 23 | OUT1–OUT8 (prepared for output channels) |

### Output Channels (prepared)

The main board already has a 10-pin header prepared for 8 signal output channels (+5V/GND) — e.g. for heaters, pumps, horns via relay or MOSFET mini-boards. Works on the same plug-in principle as the inputs.

### Compatible ESP32 Modules (first adapter board)

The footprint fits all modules in the 38-pin DevKitC layout: **ESP32-WROOM-32 / -32D / -32E / -32U / -32UE**, as well as **ESP32-SOLO-1**.

> ⚠️ **ESP32-WROVER is NOT compatible** — GPIO16/17 are internally reserved for PSRAM on WROVER and therefore blocked for output channels.

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
    {"klemme": 1, "sensor": "battery1",     "einheit": "V",   "faktor": 4.7, "offset": 0, "aktiv": true,  "topic": ""},
    {"klemme": 2, "sensor": "oil_temp",     "einheit": "°C",  "faktor": 1.0, "offset": 0, "aktiv": true,  "topic": "vessels/self/propulsion/engine/oilTemperature"},
    {"klemme": 3, "sensor": "oil_pressure", "einheit": "bar", "faktor": 1.0, "offset": 0, "aktiv": true,  "topic": "vessels/self/propulsion/engine/oilPressure"},
    {"klemme": 4, "sensor": "tank",         "einheit": "%",   "faktor": 1.0, "offset": 0, "aktiv": true,  "topic": ""},
    {"klemme": 5, "sensor": "rpm",          "einheit": "rpm", "faktor": 1.0, "offset": 0, "aktiv": false, "topic": ""}
  ]
}
```

> All 16 channels are routed via the MUX (CD74HC4067) to a single ADS1115 @ 0x48 on pin A0. No `ads` or `pin` fields needed — the channel number (`klemme`) maps directly to the MUX select lines.
>
> **`topic`** — optional. If empty (`""`), the firmware publishes to `boat/io/<sensor>` (default). Set a custom topic for Signal K compatibility or any other MQTT schema. The "SK" button in the web UI auto-fills the correct Signal K path for common sensor names.

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

## Bill of Materials – PCB Kit

**[👉 Complete parts list on Amazon](https://www.amazon.de/hz/wishlist/ls/2ZLKK0GZJ5RZE?&linkCode=ll2&tag=bigbrainlab08-21&linkId=caccb90fad5cfe3e58063e29c1bc7dd4&ref_=as_li_ss_tl)**

> **Note:** Items 7, 8, 13, and 14 are from the same set — one purchase covers all. Same applies to 12 and 15.

| # | Component | Notes | Amazon |
|---|-----------|-------|--------|
| 1 | ESP32 DevKitC-32D (38-pin) | Main controller | [Amazon](https://amzn.to/441f1bl) |
| 2 | CD74HC4067 Breakout | 16:1 MUX | [Amazon](https://amzn.to/4vPmaHH) |
| 3 | ADS1115 Breakout (1×) | 16-bit ADC | [Amazon](https://amzn.to/4gg8mRQ) |
| 4 | MPU6050 Breakout (GY-521) | IMU | [Amazon](https://amzn.to/3S3gGKJ) |
| 5 | AMS1117-3.3 Breakout | Voltage regulator | [Amazon](https://amzn.to/4gfqqeL) |
| 6 | Female header assortment | Sockets for all breakouts | [Amazon](https://amzn.to/3S1rXLD) |
| 7 | Pin header 2×10 (20-pin) | Main board ↔ input board | [Amazon](https://amzn.to/4f2awU6) |
| 8 | Pin header 2×08 (16-pin) | Main board ↔ ESP32 adapter board | [Amazon](https://amzn.to/4f2awU6) |
| 9 | Screw terminals 5.08mm | 16× sensor inputs | [Amazon](https://amzn.to/4e6bJcN) |
| 10 | Bourns 4116R-1-102LF (2×) | Resistor network DIL-16 | [Amazon](https://amzn.to/3SuSZeD) |
| 11 | SP0503BAHTG (6×) | Zener array SOT-143 | [Amazon](https://amzn.to/4aippiv) |
| 12 | JST 2-pin connector | VCC rails | [Amazon](https://amzn.to/43y2Suh) |
| 13 | Pin header 2×10 (20-pin) | Input board side | [Amazon](https://amzn.to/4f2awU6) |
| 14 | Pin header 2×08 (16-pin) | ESP32 adapter board side | [Amazon](https://amzn.to/4f2awU6) |
| 15 | JST 2-pin connector (10×) | VCC distribution | [Amazon](https://amzn.to/43y2Suh) |
| 16 | Ribbon cable + IDC 20-pin | Main board ↔ input board | [Amazon](https://amzn.to/4w6858M) |
| 17 | Ribbon cable + IDC 16-pin | Main board ↔ ESP32 adapter board | [Amazon](https://amzn.to/3SuZ3DQ) |
| 18 | Pin socket 2×05 (10-pin) | SIG-OUT prep on main board | — |
| 19 | JST 3-pin connector | VCC board ↔ main board | [Amazon](https://amzn.to/4ush82N) |
| 20 | PCB set (4 boards, JLCPCB) | Gerber files in `hardware/v2/` | — |

> Amazon: faster delivery. AliExpress links to follow.

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
│   ├── boatopenio-gehaeuse-draufsicht.svg  ← housing top-view
│   ├── mini-boards/                   ← mini-board designs
│   └── v2/                            ← KiCad PCB project (4 boards)
│       ├── main-board/
│       │   └── gerber/main-board.zip
│       ├── input-board/
│       │   └── gerber/input-board.zip
│       ├── esp32-adapter-board/
│       │   └── gerber/esp32-adapter-board.zip
│       └── vcc-board/
│           └── gerber/vcc-board.zip
├── images/                            ← build photos
└── docs/
    ├── setup.md              / setup_de.md          ← commissioning guide
    ├── configuration.md      / configuration_de.md  ← config reference
    └── backend.md            / backend_de.md        ← firmware & backend reference
```

---

## Roadmap

What's done, what's planned, what's on the backlog — see [ROADMAP.md](ROADMAP.md).

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
