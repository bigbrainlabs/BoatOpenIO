# BoatOpenIO – Roadmap

> Feature freeze in effect. The items below reflect planned directions, not commitments.  
> Pull requests welcome for anything on this list.

---

## Done – v2.1 (current)

**Hardware**
- [x] 4-board architecture: main board, input board, ESP32 adapter, VCC distributor
- [x] Single ADS1115 @ 0x48 + CD74HC4067 16:1 MUX (all 16 channels on one ADC)
- [x] Hardware protection on all 16 inputs: Bourns resistor network + SP0503BAHTG Zener array (3.3V clamp)
- [x] All components socketed — no direct soldering
- [x] 8× output channel header prepared on main board (GPIO 4/5/13/16–19/23)
- [x] Pluggable mini-boards: VT, PD, PU, ST, ISP, OPT, DIR, GND
- [x] Gerber files ready for JLCPCB (all 4 boards)

**Firmware**
- [x] 16-channel analog read via MUX (faktor + offset calibration per channel)
- [x] MPU6050 IMU: pitch / roll / impact detection with severity levels
- [x] MQTT: all sensor values, LWT, auth, configurable topic per channel
- [x] Signal K path suggestions (SK button in web UI)
- [x] OTA firmware updates (ArduinoOTA)
- [x] mDNS (`boatopenio.local`)
- [x] WiFiManager captive portal (AP on first boot)
- [x] LittleFS config persistence (config.json)
- [x] Watchdog (automatic restart on hang)
- [x] Web UI: bilingual DE/EN, dark theme, responsive, live values, calibration

---

## Planned – v2.x

**Firmware**
- [ ] Output channels: firmware support for 8× GPIO outputs (on/off, PWM)
- [ ] Output mini-boards: relay and MOSFET designs
- [ ] DS18B20 1-Wire temperature sensor support
- [ ] Web UI: inline config.json editor (raw JSON view/edit in browser)
- [ ] Web UI: OTA upload trigger (flash firmware directly from browser)
- [ ] Clean up legacy `ads` / `pin` fields from config schema and firmware

**Docs**
- [ ] Update `docs/setup.md` and `docs/configuration.md` to v2 accuracy
- [ ] Node-RED integration example (MQTT → dashboard)
- [ ] Signal K server setup guide (BoatOpenIO as data source)

---

## Ideas / Backlog

These are directions worth exploring — no timeline attached.

- [ ] **Calibration wizard** in web UI: enter a known reference value, auto-calculate faktor/offset
- [ ] **BoatOS auto-discovery** — announce all active topics and units on connect
- [ ] **Second ESP32 adapter board** — smaller footprint, or ESP32-S3 / C3 variant
- [ ] **PWM gauge output** — drive analog cockpit instruments from digital values
- [ ] **Data logging** — optional SD card module for offline value history
- [ ] **NMEA 2000 bridge** — output active channels to NMEA 2000 via CAN bus module
- [ ] **Per-channel alert thresholds** — configurable min/max with MQTT alert topic
- [ ] **Enclosure** — housing for all 4 boards with dedicated slots for mini-boards (3D print / DIN rail)

---

## Not Planned

To keep the project focused:

- No cloud dependency, no vendor accounts, no subscriptions
- No proprietary app — web UI runs entirely on the ESP32
- No support for sensors that require active current excitation (4–20mA loops) — out of scope for this hardware revision

---

## Contributing

Found a bug? Have a mini-board design? Open a pull request or issue on GitHub.

**[github.com/bigbrainlabs/BoatOpenIO](https://github.com/bigbrainlabs/BoatOpenIO)**
