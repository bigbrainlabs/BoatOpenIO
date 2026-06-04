# BoatOpenIO – Setup Guide

## Requirements

- Arduino IDE 1.8+ or 2.x
- ESP32 board package installed (`espressif/arduino-esp32`)
- Required libraries (install via Library Manager):
  - `Adafruit ADS1X15`
  - `Adafruit MPU6050`
  - `Adafruit Unified Sensor`
  - `PubSubClient`
  - `ArduinoJson` (v7)
  - `ESP32 Arduino` (includes `WebServer`, `ESPmDNS`, `ArduinoOTA`, `Preferences`, `LittleFS`)
- [ESP32 Sketch Data Upload plugin](https://github.com/me-no-dev/arduino-esp32fs-plugin) for LittleFS

---

## 1. Flash the Firmware

1. Open `firmware_src/firmware_src.ino` in Arduino IDE
2. Select board: **ESP32 Dev Module** (or your specific variant)
3. Set upload speed: **921600**
4. Select the correct COM port
5. Click **Upload**

---

## 2. Upload the Channel Configuration (LittleFS)

The default channel mapping is stored in `firmware_src/data/config.json`.

1. Install the [ESP32 Sketch Data Upload](https://github.com/me-no-dev/arduino-esp32fs-plugin) plugin
2. In Arduino IDE: **Tools → ESP32 Sketch Data Upload**
3. The `data/` folder is uploaded to LittleFS on the ESP32

> **Note:** This step is only needed on first flash or after a factory reset. Changes made via the web portal are saved automatically to LittleFS and survive reboots.

---

## 3. First Boot & Security Setup

On first boot the ESP32 starts its access point with default credentials:

```
AP:  BoatOpenIO-Setup
Key: boatopenio
IP:  192.168.4.1
```

Connect your phone or laptop to the AP. The browser opens the setup page automatically (captive portal). On desktop navigate to `http://192.168.4.1/setup`.

> **Important:** The portal is not accessible until the first-time setup is completed. All routes redirect to `/setup` until a portal password is set.

### First-Time Setup (`/setup`)

Fill in the form before anything else is accessible:

| Field | Requirement | Description |
|-------|-------------|-------------|
| Portal User | required | Username for web portal login |
| Portal Password | min. 6 chars | Protects the entire configuration portal |
| Confirm Password | must match | |
| AP Name (SSID) | required | Name of the WiFi access point broadcast by the ESP32 |
| AP Password | min. 8 chars | Key required to connect to the AP |

Click **Complete Setup**. The ESP32 saves credentials to NVS and restarts.

> **After setup:** The portal requires HTTP Basic Auth on every visit. The browser will prompt for username and password. These are the credentials you just set — not the AP password.

---

## 4. Configure WiFi & MQTT

Log in to the portal (`http://192.168.4.1`, credentials from step 3), then scroll to **WiFi & MQTT**:

| Field | Description |
|-------|-------------|
| WiFi SSID | Your router's network name |
| WiFi Password | Leave blank to keep the existing password |
| MQTT Server | IP or hostname of your MQTT broker |
| MQTT Port | Default: `1883` |
| MQTT User | Leave blank if no auth required |
| MQTT Password | Leave blank to keep the existing password |

Click **Save & Restart**. The ESP32 reboots and connects to your router.  
The AP remains active at `192.168.4.1` regardless of WiFi status.

---

## 5. Assign Channels

In the portal, scroll to **Channel Assignment**:

| Column | Description |
|--------|-------------|
| # | Terminal number (1–16, physical connector) |
| Sensor | MQTT topic suffix — becomes `boat/io/<sensor>` |
| ADS | Which ADS1115 chip (1–4, I2C 0x48–0x4B) |
| Pin | ADS input pin (A0–A3) |
| Factor | Multiplier: `output = voltage × factor + offset` |
| Offset | Additive offset after factor |
| Unit | Unit string (display only, not used in MQTT) |
| Active | Only active channels are read and published |

Click **Save Configuration**. Settings are written to LittleFS immediately, no restart needed.

---

## 6. Change Credentials (Security Card)

The **Security** section in the portal lets you update AP and portal credentials at any time.

| Field | Requirement | Effect |
|-------|-------------|--------|
| AP Name (SSID) | required | New AP name after restart |
| AP Password | min. 8 chars, leave blank to keep | New AP key after restart |
| Portal User | required | New portal login username |
| Portal Password | min. 6 chars, leave blank to keep | New portal login password |
| Confirm Password | must match portal password | |

> **Warning banner:** If the AP password is still the factory default (`boatopenio`), a yellow warning is shown at the top of the portal. Change it here.

---

## 7. Calibrate IMU (optional)

The MPU6050 runs two-stage calibration:

**Gyro bias** is measured automatically at every boot (~0.5 s, keep the board still during power-on).

**Mounting offset** corrects for the board's installation angle:

1. Scroll to **IMU Calibration** in the portal
2. Place the boat on calm water (or a level surface)
3. Watch the live Pitch/Roll readout stabilize
4. Click **Set Zero Now**

The offset is saved to NVS and applied to all future readings and MQTT publishes.

---

## 8. Test Mode vs. Live Mode

The firmware starts in **Test Mode** by default.

| Mode | Behaviour |
|------|-----------|
| TEST | Simulated sinusoidal sensor values — safe for commissioning |
| LIVE | Real ADC readings from the ADS1115 chips |

Toggle in the **Actions** section of the portal.

---

## 9. OTA Firmware Updates

Once the ESP32 is on your WiFi network, firmware can be updated over the air:

1. In Arduino IDE, select the network port: **BoatOpenIO at \<IP\>**
2. Enter the **portal password** (set during first-time setup) when prompted
3. Click **Upload** — no USB cable needed

mDNS hostname: `boatopenio.local` (on networks that support mDNS).

> **Note:** Before first-time setup is complete, the OTA password falls back to `boatopenio`.

---

## 10. Factory Reset

**At runtime** — hold GPIO0 button for 3 seconds:  
→ Clears WiFi credentials only. All other settings (MQTT, channels, portal credentials, IMU offset) are kept.

**At boot** — hold GPIO0 button while powering on, keep holding for 3 seconds:  
→ Full factory reset: clears all NVS entries (WiFi, MQTT, portal credentials, AP credentials, IMU offsets) and removes `config.json` from LittleFS.

After a full factory reset the first-time setup at `/setup` is required again before the portal becomes accessible. Re-upload `config.json` via the LittleFS tool (step 2).
