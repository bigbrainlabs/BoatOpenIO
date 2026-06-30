# BoatOpenIO

**Universelles Marine-IO-System – Open Source, modular, vollständig konfigurierbar**

> *„Steck rein, konfiguriere, fertig."*

BoatOpenIO ist eine offene Hardware-Plattform für die Sensoranbindung auf Booten. Jede der 16 Sensorklemmen kann per Software frei einem der bis zu 16 Eingangskanäle zugewiesen werden. Steckbare Mini-Platinen übernehmen die Signalaufbereitung – ohne Löten, ohne Umbau.

Teil der **[Logbuch ohne Pose](https://www.amazon.de/s?k=logbuch+ohne+pose)** Serie – Band 3.

---

## 📚 Projekt unterstützen

Dieses Projekt ist 100% Open Source und kostenlos. Die gesamte Entstehungsgeschichte – alle Ideen, Fehler, Umwege und Lösungen – ist in der Buchreihe **„Logbuch ohne Pose"** dokumentiert.

Wer das Projekt nützlich findet und unterstützen möchte: Die Bücher kaufen ist der beste Weg.

> *„Nützlich gefunden? Schnapp dir ein Buch – oder die ganze Serie. Du unterstützt damit die Weiterentwicklung."*

**[👉 Logbuch ohne Pose auf Amazon](https://amzn.to/4e5swN6)**

---

## ⚠️ Sicherheitshinweis

**Immer prüfen welche Mini-Platine auf welchem Kanal steckt, bevor Spannung anliegt.**

12V direkt an einen Pull-up-Kanal (statt über Spannungsteiler) zerstört den MUX, alle ADS1115 und den ESP32 durch I2C-Kettenreaktion.

Das Eingangsboard enthält eine Schutzschaltung (3.3V) an allen 16 Signaleingängen – aber die korrekte Mini-Platinen-Zuordnung bleibt trotzdem essenziell.

---

## Das Konzept

Kommerzielle Marine-IO-Systeme kosten Hunderte Euro und sind geschlossene Black Boxes.

BoatOpenIO kostet einen Bruchteil – und jeder kann es anpassen, erweitern und verbessern.

```
Sensor → Mini-Platine → JST 3-Pin → PCB → Zenerschutz → MUX → ADS1115 → ESP32 → MQTT → BoatOS
```

---

## Hardware – v2 PCB

Vier separate Platinen, verbunden über IDC-Flachbandkabel: **Eingangsboard**, **Mainboard**, **ESP32-Adapterboard** und **VCC-Verteiler**. Bestellfertige Gerber-Dateien in `hardware/v2/*/gerber/`.

```
Eingangsboard ──20-pol Flachband──> Mainboard <──16-pol Flachband── ESP32-Adapterboard
                                        │
                                   3-pol JST
                                        │
                                   VCC-Board
```

### Mainboard-Komponenten

| Komponente | Beschreibung | Sockel |
|------------|-------------|--------|
| ESP32-DEVKITC-32D-F (38-Pin) | Mikrocontroller, WLAN, I2C, MUX-Steuerung – auf eigenem Adapterboard | ✅ |
| CD74HC4067 Breakout | 16:1 Analog-Multiplexer | ✅ |
| ADS1115 Breakout (1×) | 16-bit ADC | ✅ |
| MPU6050 Breakout (GY-521) | 6-DOF IMU, Neigung/Impact-Detection | ✅ |
| AMS1117-3.3 Breakout | 5V → 3.3V Regler | ✅ |

**Alles auf Sockeln** – kein Bauteil ist fest verlötet. Defekter ESP32? Adapterboard tauschen. Fertig.

Der ESP32 sitzt bewusst auf einem eigenen Adapterboard. Das erste Adapterboard ist für den 38-Pin ESP32 DevKitC-32D ausgelegt – austauschbar, falls später andere ESP32-Varianten zum Einsatz kommen sollen.

### Schutzschaltung (Eingangsboard)

Jeder der 16 Signaleingänge hat eine Hardware-Schutzschaltung, direkt auf dem Eingangsboard:

```
Sensor → Schraubklemme → R_Pack08 (Widerstandsnetzwerk) → SP0503BAHTG (Zener-Array, 3.3V) → MUX-Signal
```

2× Bourns 4116R-1-102LF (R_Pack08, DIL-16, isoliert) + 6× SP0503BAHTG (SOT-143, je 3 Kanäle).

> ⚠️ **Pull-up Spannung ist 3.3V** – die ADS1115 VDD liegt bei 3.3V, höhere Spannungen an den Eingängen riskieren Bauteilschäden.

### Eingangsboard – Anschlüsse

```
16× Schraubklemme 5.08mm  → Sensor-Rohsignal
20-pol Pfostenstecker     → Verbindung zum Mainboard (SIG1-16 + GND + 3.3V + 5V)
7× JST 2-Pin (3.3V)       → VCC-Leiste für passive/ESP01-Mini-Platinen
3× JST 2-Pin (5V)         → VCC-Leiste für Arduino-Nano-Mini-Platinen
```

### I2C-Adresse

Nur 1× ADS1115 verbaut (MUX legt alle 16 Kanäle auf eine gemeinsame Signalleitung) → feste Adresse **0x48**.

### GPIO-Belegung (ESP32, 16-pol Mainboard↔Adapterboard)

| GPIO | Funktion |
|------|----------|
| GPIO21 | I2C SDA |
| GPIO22 | I2C SCL |
| GPIO14 | MUX S0 |
| GPIO27 | MUX S1 |
| GPIO26 | MUX S2 |
| GPIO25 | MUX S3 |
| GPIO4 / 5 / 13 / 16 / 17 / 18 / 19 / 23 | OUT1–OUT8 (vorbereitet für Ausgangskanäle) |

### Ausgangskanäle (vorbereitet)

Das Mainboard hat bereits einen 10-pol Stecker für 8 Signal-Ausgangskanäle (+5V/GND) vorbereitet – z.B. für Heizung, Pumpe, Horn über Relais- oder MOSFET-Mini-Platinen. Funktioniert nach dem gleichen Baukastenprinzip wie die Eingänge.

### Kompatible ESP32-Module (erstes Adapterboard)

Der Footprint passt zu allen Modulen im 38-Pin DevKitC-Layout: **ESP32-WROOM-32 / -32D / -32E / -32U / -32UE**, sowie **ESP32-SOLO-1**.

> ⚠️ **ESP32-WROVER ist NICHT kompatibel** – GPIO16/17 sind bei WROVER intern für PSRAM reserviert und damit für die Ausgangskanäle blockiert.

---

## Mini-Platinen (steckbar)

| Typ | Kürzel | Verwendung |
|-----|--------|------------|
| Spannungsteiler | VT | 12V VDO-Sensoren → 3.3V |
| Pull-down | PD | Digitale Sensoren, Schalter |
| Pull-up | PU | Open-Collector Ausgänge |
| Schmitt-Trigger | ST | Magnetische Pickup-Sensoren, 50Hz Generatoren |
| Impuls-Board | ISP | Drehzahl, Durchfluss (Arduino Nano / ESP01) |
| Optokoppler | OPT | Galvanische Trennung (W-Klemme Drehzahl) |
| Jumper / Direkt | DIR | 3.3V Signale direkt |
| Masse-Sammler | GND | Mehrere Massekreise → gemeinsame Referenz |

**Eigene Mini-Platinen entwerfen und einreichen** – Pull Requests willkommen!

---

## Multiplexer-Logik

```
S3 S2 S1 S0 → aktiver Kanal
 0  0  0  0 → Klemme 1  (MUX C15 auf der Platine)
 0  0  0  1 → Klemme 2  (MUX C14 auf der Platine)
 ...
 1  1  1  1 → Klemme 16 (MUX C0 auf der Platine)
```



---

## Software-Konfiguration

```json
{
  "kanaele": [
    {"klemme": 1, "ads": 1, "pin": "A0", "sensor": "batterie1",    "einheit": "V",   "faktor": 4.7, "offset": 0},
    {"klemme": 2, "ads": 2, "pin": "A3", "sensor": "oeltemperatur","einheit": "°C",  "faktor": 1.0, "offset": 0},
    {"klemme": 3, "ads": 1, "pin": "A1", "sensor": "oeldruck",     "einheit": "bar", "faktor": 1.0, "offset": 0},
    {"klemme": 4, "ads": 3, "pin": "A0", "sensor": "tank",         "einheit": "%",   "faktor": 1.0, "offset": 0},
    {"klemme": 5, "ads": 2, "pin": "A1", "sensor": "drehzahl",     "einheit": "rpm", "faktor": 1.0, "offset": 0}
  ]
}
```

---

## MQTT-Topics

```
boat/io/batterie1       → 12.43
boat/io/oeltemperatur   → 87
boat/io/oeldruck        → 3.2
boat/io/tank            → 48
boat/io/drehzahl        → 1450
boat/io/pitch           → 1.2
boat/io/roll            → -0.5
boat/io/impact          → LIGHT
boatopenio/status       → online
```

---

## Firmware-Features

| Feature | Beschreibung |
|---------|-------------|
| Web-UI | Konfiguration unter `192.168.4.1`, zweisprachig DE/EN |
| WiFiManager | AP beim ersten Start für WLAN-Einrichtung |
| MQTT | Alle Sensorwerte, LWT, Auth-Unterstützung |
| OTA | Over-the-Air Firmware-Updates |
| mDNS | Erreichbar als `boatopenio.local` |
| Impact-Detection | MPU6050 G-Kraft-Überwachung mit Schweregrad |
| Watchdog | Automatischer Neustart bei Firmware-Hänger |
| LittleFS | Persistente Konfigurationsspeicherung |

---

## Unterstützte Sensoren

| Sensor | Typ | Mini-Platine |
|--------|-----|-------------|
| VDO Temperatur (NTC) | Widerstand | VT |
| VDO Öldruck | Widerstand linear | VT |
| VDO Tankgeber | Potentiometer | VT |
| Batteriespannung 12V | Spannungsteiler | VT (10k/2.2k) |
| Drehzahl (W-Klemme) | Impuls/AC | OPT + ISP |
| Drehzahl (magnetischer Pickup) | AC-Signal | ST + ISP |
| Kraftstoffdurchfluss | Impuls | ISP |
| Bilgensensor | Digital | PD |
| Tür-/Luken-Kontakt | Digital | PD |
| DS18B20 Temperatursensor | Digital 1-Wire | eigene Platine |

---

## Stückliste – PCB Bausatz

> **Hinweis:** Positionen 7, 8, 13 und 14 sind aus dem gleichen Set – einmal kaufen reicht. Gleiches gilt für 12 und 15.

| # | Komponente | Hinweis | Amazon |
|---|------------|---------|--------|
| 1 | ESP32 DevKitC-32D (38-Pin) | Hauptcontroller | [Amazon](https://amzn.to/441f1bl) |
| 2 | CD74HC4067 Breakout | 16:1 MUX | [Amazon](https://amzn.to/4vPmaHH) |
| 3 | ADS1115 Breakout (1×) | 16-bit ADC | [Amazon](https://amzn.to/4gg8mRQ) |
| 4 | MPU6050 Breakout (GY-521) | IMU | [Amazon](https://amzn.to/3S3gGKJ) |
| 5 | AMS1117-3.3 Breakout | Spannungsregler | [Amazon](https://amzn.to/4gfqqeL) |
| 6 | Buchsenleisten Sortiment | Sockel für alle Breakouts | [Amazon](https://amzn.to/3S1rXLD) |
| 7 | Pfostenstecker 2×10 (20-pol) | Mainboard ↔ Eingangsboard | [Amazon](https://amzn.to/4f2awU6) |
| 8 | Pfostenstecker 2×08 (16-pol) | Mainboard ↔ ESP32-Adapterboard | [Amazon](https://amzn.to/4f2awU6) |
| 9 | Schraubklemmen 5.08mm | 16× Sensoreingänge | [Amazon](https://amzn.to/4e6bJcN) |
| 10 | Bourns 4116R-1-102LF (2×) | Widerstandsnetzwerk DIL-16 | [Amazon](https://amzn.to/3SuSZeD) |
| 11 | SP0503BAHTG (6×) | Zener-Array SOT-143 | [Amazon](https://amzn.to/4aippiv) |
| 12 | JST 2-Pin Stecker | VCC-Leisten | [Amazon](https://amzn.to/43y2Suh) |
| 13 | Pfostenstecker 2×10 (20-pol) | Eingangsboard-Seite | [Amazon](https://amzn.to/4f2awU6) |
| 14 | Pfostenstecker 2×08 (16-pol) | ESP32-Adapterboard-Seite | [Amazon](https://amzn.to/4f2awU6) |
| 15 | JST 2-Pin Stecker (10×) | VCC-Verteilung | [Amazon](https://amzn.to/43y2Suh) |
| 16 | Flachbandkabel + IDC 20-pol | Mainboard ↔ Eingangsboard | [Amazon](https://amzn.to/4w6858M) |
| 17 | Flachbandkabel + IDC 16-pol | Mainboard ↔ ESP32-Adapterboard | [Amazon](https://amzn.to/3SuZ3DQ) |
| 18 | Pfostenbuchse 2×05 (10-pol) | SIG-OUT Vorbereitung Mainboard | — |
| 19 | JST 3-Pin Stecker | VCC-Board ↔ Mainboard | [Amazon](https://amzn.to/4ush82N) |
| 20 | PCB Set (4 Platinen, JLCPCB) | Gerber-Dateien in `hardware/v2/` | — |

> Amazon: schnelle Lieferung. AliExpress-Links folgen.

Kommerzielle Marine-IO-Systeme: 200–500€.

---

## Verzeichnisstruktur

```
BoatOpenIO/
├── README.md                          ← Englisch
├── README_de.md                       ← Deutsch (diese Datei)
├── LICENSE                            ← GPL-3.0
├── boatopenio_final_architektur.svg   ← Systemarchitektur-Diagramm
├── firmware_src/
│   ├── firmware_src.ino               ← Haupt-Firmware
│   ├── webui.h                        ← Web-UI & Captive Portal
│   └── data/
│       └── config.json                ← Standard-Kanalconfig
├── hardware/
│   ├── boatopenio-gehaeuse-draufsicht.svg  ← Gehäuse Draufsicht
│   ├── mini-boards/                   ← Mini-Platinen-Designs
│   └── v2/                            ← KiCad PCB-Projekt (4 Platinen)
│       ├── main-board/
│       │   └── gerber/main-board.zip
│       ├── input-board/
│       │   └── gerber/input-board.zip
│       ├── esp32-adapter-board/
│       │   └── gerber/esp32-adapter-board.zip
│       └── vcc-board/
│           └── gerber/vcc-board.zip
├── images/                            ← Aufbaufotos
└── docs/
    ├── setup.md
    ├── setup_de.md
    ├── configuration.md
    └── configuration_de.md
```

---

## Integration mit BoatOS

BoatOpenIO sendet alle Werte per MQTT. BoatOS erkennt alle Topics automatisch (Auto-Discovery) und zeigt sie im Dashboard an. Kein manuelles Konfigurieren nötig.

**➡️ [github.com/bigbrainlabs/BoatOS](https://github.com/bigbrainlabs/BoatOS)**

---

## Lizenz

**GNU General Public License v3.0 (GPL-3.0)**

Frei nutzbar, veränderbar und verteilbar – abgeleitete Werke müssen ebenfalls GPL-3.0 sein. Siehe [LICENSE](LICENSE).

---

## Teil der Serie

**Logbuch ohne Pose** – authentische DIY-Projekte rund ums Boot.

> *„Selbstgemacht ist wissen was drin ist und bezahlbar."*

📖 [Buchreihe auf Amazon](https://amzn.to/4e5swN6) · 🔧 [BoatOS](https://github.com/bigbrainlabs/BoatOS) · ⚡ [BoatOpenIO](https://github.com/bigbrainlabs/BoatOpenIO)
