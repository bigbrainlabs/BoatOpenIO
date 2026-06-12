# BoatOpenIO

**Universelles Marine-IO-System – Open Source, modular, vollständig konfigurierbar**

> *„Steck rein, konfiguriere, fertig."*

BoatOpenIO ist eine offene Hardware-Plattform für die Sensoranbindung auf Booten. Jede der 16 Sensorklemmen kann per Software frei einem der bis zu 16 Eingangskanäle zugewiesen werden. Steckbare Mini-Platinen übernehmen die Signalaufbereitung – ohne Löten, ohne Umbau.

Teil der **[Logbuch ohne Pose](https://www.amazon.de/s?k=logbuch+ohne+pose)** Serie – Band 3.

---

## 📚 Projekt unterstützen

Dieses Projekt ist 100% Open Source und kostenlos. Die gesamte Entstehungsgeschichte – alle Ideen, Fehler, Umwege und Lösungen – ist in der Buchreihe **„Logbuch ohne Pose"** dokumentiert.

Wer das Projekt nützlich findet und unterstützen möchte: Die Bücher kaufen ist der beste Weg.

> *„Du findest das nützlich? Kauf die Buchreihe. Dann sind wir quitt."* 😄

**[👉 Logbuch ohne Pose auf Amazon](https://amzn.to/4e5swN6)**

---

## ⚠️ Sicherheitshinweis

**Immer prüfen welche Mini-Platine auf welchem Kanal steckt, bevor Spannung anliegt.**

12V direkt an einen Pull-up-Kanal (statt über Spannungsteiler) zerstört den MUX, alle ADS1115 und den ESP32 durch I2C-Kettenreaktion.

Die v1.0 PCB enthält Zenerdioden-Schutzschaltung (3.3V) an allen 16 Signaleingängen – aber die korrekte Mini-Platinen-Zuordnung bleibt trotzdem essenziell.

---

## Das Konzept

Kommerzielle Marine-IO-Systeme kosten Hunderte Euro und sind geschlossene Black Boxes.

BoatOpenIO kostet einen Bruchteil – und jeder kann es anpassen, erweitern und verbessern.

```
Sensor → Mini-Platine → JST 3-Pin → PCB → Zenerschutz → MUX → ADS1115 → ESP32 → MQTT → BoatOS
```

---

## Hardware – v1.0 PCB

Die v1.0 PCB ist jetzt verfügbar! Gerber-Dateien im Ordner `hardware/pcb/` – direkt bei JLCPCB oder einem anderen PCB-Hersteller bestellen.

### Hauptplatinen-Komponenten

| Komponente | Beschreibung | Sockel |
|------------|-------------|--------|
| ESP32 DevKit V4 (30-Pin) | Mikrocontroller, WLAN, I2C, MUX-Steuerung | ✅ |
| CD74HC4067 Breakout (BOB-09056) | 16:1 Analog-Multiplexer | ✅ |
| ADS1115 Breakout (×1–4) | 16-bit ADC, I2C Adressen 0x48–0x4B | ✅ |
| MPU6050 Breakout (GY-521) | 6-DOF IMU, Neigung/Impact-Detection | ✅ |
| AMS1117-3.3 Breakout | 5V → 3.3V Regler, LED-Anzeige | ✅ |

**Alles auf Sockeln** – kein Bauteil ist fest verlötet. Defekter ESP32? Neuen aufstecken. Fertig.

### Schutzschaltung

Jeder Signaleingang hat eine Hardware-Schutzschaltung:

```
Mini-Platine SIG OUT → 1kΩ Widerstand → Zener 3.3V → MUX Eingang Cx
                                              ↓
                                             GND
```

16× BZX55C3V3 (3.3V Zener, DO-35 THT) + 16× 1kΩ Widerstand auf der Platine.

### Signal-Stecker: JST 3-Pin (×16)

```
Pin 1: Signal IN   → Roheingang vom Sensor (bis 12V)
Pin 2: GND
Pin 3: Signal OUT  → Aufbereitetes Signal zum MUX (max. 3.3V nach Schutz)
```

### VCC-Leisten (linke Seite der Platine, JST 2-Pin, nach oben raus)

```
7× 3.3V Leiste  → für ESP01-Mini-Platinen und passive Boards
3× 5V Leiste    → für Arduino Nano Mini-Platinen
```

### I2C Adressen (fest auf der Platine)

| ADS | ADDR-Pin | I2C Adresse |
|-----|----------|-------------|
| ADS1 | → GND | 0x48 |
| ADS2 | → VDD | 0x49 |
| ADS3 | → SDA | 0x4A |
| ADS4 | → SCL | 0x4B |

### GPIO-Belegung (ESP32)

| GPIO | Funktion |
|------|----------|
| GPIO14 | MUX S0 |
| GPIO27 | MUX S1 |
| GPIO26 | MUX S2 |
| GPIO25 | MUX S3 |
| GPIO21 | I2C SDA |
| GPIO22 | I2C SCL |

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

## Stückliste – v1.0 PCB Bausatz

### Aktive Komponenten (Breakout-Boards, gesockelt)

| Anz. | Komponente | Hinweis |
|------|------------|---------|
| 1× | ESP32 DevKit V4 (30-Pin) | Hauptcontroller |
| 4× | ADS1115 Breakout (GY-ADS1115) | 16-bit ADC |
| 1× | CD74HC4067 Breakout (BOB-09056) | 16:1 MUX |
| 1× | MPU6050 Breakout (GY-521) | IMU |
| 1× | AMS1117-3.3 Breakout | Spannungsregler |

### Schutzschaltung (THT, je 16× pro Kanal)

| Anz. | Komponente | Wert | Gehäuse |
|------|------------|------|---------|
| 16× | Widerstand | 1kΩ | THT axial |
| 16× | Zenerdiode | BZX55C3V3 (3.3V) | DO-35 |

### Anschlüsse (THT)

| Anz. | Komponente | Beschreibung |
|------|------------|-------------|
| 1× | Schraubklemme 1×16 | Sensorklemmen K1–K16, Rastermaß 5.08mm |
| 1× | Stiftleiste 2×16 | SIG IN/OUT, nach oben raus |
| 10× | JST PH 2-Pin (2.0mm) | VCC/GND Leisten (7× 3.3V + 3× 5V) |
| 1× | JST PH 2-Pin (2.0mm) | Stromeingang 5V |

### Sockel

| Anz. | Komponente | Beschreibung |
|------|------------|-------------|
| 2× | Buchsenleiste 1×19 | ESP32 DevKit Sockel |
| 4× | Buchsenleiste 1×08 | ADS1115 Breakout Sockel |
| 1× | Buchsenleiste 1×08 | MPU6050 Sockel |
| 1× | Buchsenleiste 1×16 | MUX Breakout (oben, C0–C15) |
| 1× | Buchsenleiste 1×08 | MUX Breakout (unten) |
| 1× | Buchsenleiste 1×03 | AMS1117 Breakout |

### Platine

| Anz. | Komponente | Beschreibung |
|------|------------|-------------|
| 1× | BoatOpenIO v1.0 PCB | Gerber-Dateien in `hardware/pcb/` |

### Stückliste mit Links

> **Hinweis zu den Preisen:** Alle Preise beziehen sich auf Sets und Packungsgrößen. Ein einzelnes Board kostet deutlich weniger – die Sets reichen für viele weitere Projekte.

| Komponente | Amazon | AliExpress | Amazon Preis | AliExpress Preis |
|------------|--------|------------|-------------|-----------------|
| ESP32 DevKit V4 (30-Pin) | [Amazon](https://amzn.to/4ged4iW) | [AliExpress](https://s.click.aliexpress.com/e/_c4pqHci1) | ~9€ | ~5€ |
| ADS1115 Breakout (4×) | [Amazon](https://amzn.to/4gg8mRQ) | [AliExpress](https://s.click.aliexpress.com/e/_c35sBSYd) | ~9€ | ~7€ |
| CD74HC4067 MUX Breakout | [Amazon](https://amzn.to/4vPmaHH) | [AliExpress](https://s.click.aliexpress.com/e/_c3RLw85R) | ~6€ | ~5€ |
| MPU6050 Breakout (GY-521) | [Amazon](https://amzn.to/3S3gGKJ) | [AliExpress](https://s.click.aliexpress.com/e/_c3tzzLRn) | ~5€ | ~2€ |
| AMS1117-3.3 Breakout | [Amazon](https://amzn.to/4gfqqeL) | [AliExpress](https://s.click.aliexpress.com/e/_c33Kg9MD) | ~5€ | ~3€ |
| 1kΩ Widerstände (Set) | [Amazon](https://amzn.to/4xpBFro) | [AliExpress](https://s.click.aliexpress.com/e/_c4BhD2g5) | ~14€ | ~6€ |
| BZX55C3V3 Zenerdioden (100×) | [Amazon](https://amzn.to/4v9OTqK) | [AliExpress](https://s.click.aliexpress.com/e/_c3ra9r0h) | ~8€ | ~2€ |
| Schraubklemmen 5.08mm | [Amazon](https://amzn.to/4e6bJcN) | [AliExpress](https://s.click.aliexpress.com/e/_c4X0xKf3) | ~8€ | ~7€ |
| JST PH 2.0mm 2-Pin | [Amazon](https://amzn.to/43y2Suh) | [AliExpress](https://s.click.aliexpress.com/e/_c4BoCB7P) | ~6€ | ~3€ |
| JST PH 2.0mm 3-Pin | [Amazon](https://amzn.to/4ush82N) | [AliExpress](https://s.click.aliexpress.com/e/_c3LlQpYp) | ~8€ | ~3€ |
| Buchsenleisten | [Amazon](https://amzn.to/3S1rXLD) | [AliExpress](https://s.click.aliexpress.com/e/_c3B9h4ip) | ~9€ | ~2€ |
| Lochraster (Set) | [Amazon](https://amzn.to/3SAIJ4k) | [AliExpress](https://s.click.aliexpress.com/e/_c4dYUhKH) | ~14€ | ~6€ |
| PCB (5 Stück JLCPCB) | Gerber-Dateien in `hardware/pcb/` | | ~7€ | ~7€ |
| **Gesamt** | | | **~108€** | **~58€** |

> Amazon: schnelle Lieferung. AliExpress: deutlich günstiger, 2–4 Wochen Lieferzeit.

Kommerzielle Marine-IO-Systeme: 200–500€.

---

## Verzeichnisstruktur

```
BoatOpenIO/
├── README.md                   ← Englisch
├── README_de.md                ← Deutsch (diese Datei)
├── LICENSE                     ← GPL-3.0
├── hardware/
│   ├── pcb/
│   │   ├── boardopenio.kicad_sch
│   │   ├── boardopenio.kicad_pcb
│   │   └── gerber/             ← bestellfertig
│   ├── mini-boards/
│   │   ├── VT-spannungsteiler/
│   │   ├── PU-pullup/
│   │   └── ...
│   └── bom.md
├── firmware/
│   └── BoatOpenIO_Firmware.ino
├── images/
└── docs/
    ├── setup.md
    ├── setup_de.md
    └── troubleshooting.md
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
