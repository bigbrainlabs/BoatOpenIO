# BoatOpenIO

**Universelles Marine-IO-System – Open Source, modular, vollständig konfigurierbar**

> *„Steck rein, konfiguriere, fertig."*

BoatOpenIO ist eine offene Hardware-Plattform für die Sensoranbindung auf Booten. Jede der 16 Sensorklemmen kann per Software frei einem der bis zu 16 Eingangskanäle zugewiesen werden. Steckbare Mini-Platinen übernehmen die Signalaufbereitung – ohne Löten, ohne Umbau.

Teil der **[Logbuch ohne Pose](https://github.com/bigbrainlabs/logbuch-ohne-pose)** Serie.

---

## Das Konzept

Kommerzielle Marine-IO-Systeme kosten Hunderte Euro und sind geschlossene Black Boxes.

BoatOpenIO kostet einen Bruchteil und jeder kann es anpassen, erweitern und verbessern.

```
Sensor → Klemme → Mini-Platine → MUX → ADS1115 → ESP32 → MQTT → BoatOS
```

---

## Hardware-Architektur

### Hauptplatine

| Komponente | Beschreibung | Sockel |
|------------|-------------|--------|
| ESP32 WROOM-32 | Mikrocontroller, WLAN, I2C, MUX-Steuerung | ✅ |
| CD74HC4067 | 16:1 Analog-Multiplexer | ✅ |
| ADS1115 (1–4x) | 16-bit ADC, I2C, Adressen 0x48–0x4B | ✅ |
| MPU6050 | 6-DOF IMU, Neigung/Beschleunigung | ✅ |

**Alles auf Sockeln** – kein Bauteil ist fest verlötet. Defekter ESP32? Neuen aufstecken. Fertig.

### 16 Sensorklemmen

Jede Klemme hat:
- 1x Eingang (Signal vom Sensor)
- 1x 4-Pin-Sockel für Mini-Platine
- Verbindung zum MUX-Eingang

**4-Pin-Sockel Belegung:**

```
Pin 1: Signal IN   → Eingang vom Sensor
Pin 2: Signal OUT  → Ausgang zum MUX
Pin 3: GND
Pin 4: VCC         → 3.3V oder 5V (per Jumper wählbar)
```

Passive Mini-Platinen (VT, PD, OPT) bestücken nur Pin 1–3, Pin 4 bleibt frei.
Aktive Mini-Platinen (ISP mit Arduino/ESP01) nutzen alle 4 Pins.

### Mini-Platinen (steckbar)

| Typ | Kürzel | Verwendung |
|-----|--------|------------|
| Spannungsteiler | VT | 12V VDO-Sensoren → 3.3V |
| Pull-down | PD | Digitale Sensoren, Schalter |
| Pull-up | PU | Open-Collector Ausgänge |
| Impuls-Board | ISP | Drehzahl, Durchfluss (Arduino/ESP01) |
| Optokoppler | OPT | Galvanische Trennung |
| Jumper / Direkt | DIR | 3.3V Signale direkt |

**Eigene Mini-Platinen entwerfen und einreichen** – Pull Requests willkommen!

### Multiplexer-Logik

Der CD74HC4067 hat 16 Eingänge und einen gemeinsamen SIG-Ausgang.

4 Steuerpins (S0–S3) vom ESP32 wählen per Binärcode den aktiven Kanal:

```
S3 S2 S1 S0 → aktiver Kanal
 0  0  0  0 → Klemme 1
 0  0  0  1 → Klemme 2
 ...
 1  1  1  1 → Klemme 16
```

Der SIG-Pin geht an alle ADS1115-Eingänge parallel. Per Software wählt man welchen ADS-Kanal man liest.

### Bis zu 4x ADS1115

```
ADS1 @ I2C 0x48  →  A0, A1, A2, A3  (4 Kanäle)
ADS2 @ I2C 0x49  →  A0, A1, A2, A3  (4 Kanäle)
ADS3 @ I2C 0x4A  →  A0, A1, A2, A3  (4 Kanäle)
ADS4 @ I2C 0x4B  →  A0, A1, A2, A3  (4 Kanäle)
                                      = 16 Ziele
```

---

## Software-Konfiguration

Jede Klemme wird in einer JSON-Konfiguration einem ADS-Kanal und Sensortyp zugewiesen:

```json
{
  "kanaele": [
    {"klemme": 1, "ads": 1, "pin": "A0", "typ": "spannungsteiler", "sensor": "batterie1", "einheit": "V"},
    {"klemme": 2, "ads": 2, "pin": "A3", "typ": "ntc", "sensor": "oeltemperatur", "einheit": "°C"},
    {"klemme": 3, "ads": 1, "pin": "A1", "typ": "linear", "sensor": "oeldruck", "einheit": "bar"},
    {"klemme": 4, "ads": 3, "pin": "A0", "typ": "spannungsteiler", "sensor": "tank", "einheit": "%"},
    {"klemme": 5, "ads": 2, "pin": "A1", "typ": "impuls", "sensor": "drehzahl", "einheit": "rpm"}
  ]
}
```

Konfiguration ändern → neu flashen → fertig. Kein Umbau, kein Löten.

---

## MQTT-Topics

Jeder Kanal bekommt automatisch ein eigenes MQTT-Topic:

```
boat/io/batterie1       → 12.43
boat/io/oeltemperatur   → 87
boat/io/oeldruck        → 3.2
boat/io/tank            → 48
boat/io/drehzahl        → 1450
```

Kompatibel mit **[BoatOS](https://github.com/bigbrainlabs/BoatOS)** – alle Topics werden automatisch erkannt und im Dashboard angezeigt.

---

## Impuls-Boards (ISP)

Für Drehzahl und Durchfluss – Signale die schnelle Impulszählung brauchen – gibt es steckbare Impuls-Boards.

Ein Arduino Nano oder ESP01 sitzt auf der Mini-Platine, zählt Impulse, berechnet den Wert und gibt ihn als analoge Spannung an den MUX weiter.

```
Klemme (Rohimpuls) → Arduino Nano (ISP) → berechneter Wert → MUX → ADS1115 → ESP32
```

**Vorteil:** Der ESP32 wird nicht durch Interrupts belastet. Timing-kritische Aufgaben werden ausgelagert.

Eigene ISP-Boards für andere Impulsquellen: Pull Request willkommen.

---

## Unterstützte Sensoren

| Sensor | Typ | Mini-Platine |
|--------|-----|-------------|
| VDO Temperatur (NTC) | Widerstand | VT |
| VDO Öldruck | Widerstand linear | VT |
| VDO Tankgeber | Potentiometer | VT |
| Batteriespannung 12V | Spannung | VT |
| Drehzahl (W-Klemme) | Impuls | ISP |
| Kraftstoffdurchfluss | Impuls | ISP |
| Bilgensensor | Digital | PD |
| Türkontakt | Digital | PD |
| Temperatursensor DS18B20 | Digital 1-Wire | eigene Platine |
| ... | ... | eigene Platine |

---

## Kosten

| Komponente | Preis |
|------------|-------|
| ESP32 WROOM-32 | ~5€ |
| CD74HC4067 MUX | ~1€ |
| ADS1115 (4x) | ~8€ |
| MPU6050 | ~2€ |
| Sockel, Klemmen, Platine | ~10€ |
| **Gesamt** | **~26€** |

Kommerzielle Marine-IO-Systeme: 200–500€.

---

## Verzeichnisstruktur

```
BoatOpenIO/
├── README.md
├── hardware/
│   ├── hauptplatine/          ← KiCad-Dateien
│   ├── mini-platinen/
│   │   ├── VT-spannungsteiler/
│   │   ├── PD-pulldown/
│   │   ├── ISP-impuls/
│   │   ├── OPT-optokoppler/
│   │   └── DIR-direkt/
│   └── images/
├── firmware/
│   ├── src/
│   │   ├── main.cpp
│   │   ├── mux.cpp
│   │   ├── ads.cpp
│   │   └── mqtt.cpp
│   ├── config/
│   │   └── kanaele.json       ← Kanal-Konfiguration
│   └── platformio.ini
└── docs/
    ├── aufbau.md
    ├── kalibrierung.md
    └── mini-platinen.md       ← Anleitung eigene Platinen
```

---

## Eigene Mini-Platinen

BoatOpenIO ist eine offene Plattform. Jeder kann eigene Mini-Platinen entwerfen und einreichen.

**Anforderungen:**
- 4-Pin-Sockel: Signal IN (1), Signal OUT (2), GND (3), VCC (4)
- Passive Platinen: nur Pin 1–3 bestücken, Pin 4 einfach weglassen
- Aktive Platinen (ISP): alle 4 Pins bestücken
- VCC per Jumper auf Hauptplatine wählbar: 3.3V oder 5V

**Einreichen:**
1. KiCad-Dateien in `hardware/mini-platinen/DEIN-TYP/`
2. Kurze Beschreibung in `docs/mini-platinen.md`
3. Pull Request

---

## Integration mit BoatOS

BoatOpenIO sendet alle Werte per MQTT. BoatOS erkennt neue Topics automatisch (Auto-Discovery) und zeigt sie im Dashboard an.

Kein manuelles Konfigurieren nötig. Sensor anschließen, MQTT läuft, Dashboard zeigt an.

**➡️ BoatOS: [github.com/bigbrainlabs/BoatOS](https://github.com/bigbrainlabs/BoatOS)**

---

## Lizenz

**Hardware & Software:** MIT License – frei nutzbar, modifizierbar, verteilbar, auch kommerziell.

---

## Teil der Serie

**Logbuch ohne Pose** – authentische DIY-Projekte rund ums Boot.

> *„Selbstgemacht ist wissen was drin ist und bezahlbar."*

📖 [Buchreihe auf Amazon](https://www.amazon.de/s?k=logbuch+ohne+pose) · 🔧 [BoatOS](https://github.com/bigbrainlabs/BoatOS) · 📡 [BoatOpenIO](https://github.com/bigbrainlabs/BoatOpenIO)
