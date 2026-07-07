# BoatOpenIO – Firmware- & Backend-Referenz

Diese Referenz beschreibt, **was die Firmware im Betrieb tut** – Startablauf, Mess- und
Publish-Verhalten, die HTTP-/MQTT-Schnittstellen, das Sicherheitsmodell und die
Persistenz. Sie ist als Nachschlagewerk und Bedienungsanleitung für das Backend gedacht.

Für die reine Erstinbetriebnahme siehe [`setup_de.md`](setup_de.md), für die
Feld-für-Feld-Konfigurationsreferenz [`configuration_de.md`](configuration_de.md).

> Stand: Firmware **v2.2**. Quelldateien: `firmware_src/firmware_src.ino` (Logik) und
> `firmware_src/webui.h` (Web-UI, HTTP-Handler).

---

## 1. Architektur auf einen Blick

```
              ┌──────────────────────────── ESP32-WROOM-32D ───────────────────────────┐
  Sensoren →  │  CD74HC4067 MUX ──► ADS1115 @ 0x48 (A0)      MPU6050 @ 0x68/0x69 (I²C)  │
  (16 Klemmen)│         ▲                                                                │
              │         │ S0–S3 (GPIO 14/27/26/25)                                       │
              │   ┌─────┴───── loop() (Core 1, non-blocking, millis()-getaktet) ─────┐   │
              │   │  Kanäle 2 s · IMU 50 Hz · Pitch/Roll 1 s · Heartbeat 10 s        │   │
              │   └──────────────────────────────────────────────────────────────────┘  │
              │      │                 │                    │                             │
              │   WebServer:80    PubSubClient (MQTT)   ArduinoOTA / mDNS                 │
              └──────┼─────────────────┼────────────────────┼─────────────────────────────┘
                     │                 │                    │
                Browser (AP/STA)   MQTT-Broker         Arduino IDE (OTA)
```

Es gibt **keine** FreeRTOS-Tasks in der Anwendung – alles läuft kooperativ in `loop()`.
Sämtliche Zeitsteuerung nutzt das overflow-feste `millis()`-Subtraktionsmuster, sodass
kein Timer nach 49,7 Tagen (millis-Überlauf) aus dem Tritt gerät.

---

## 2. Startablauf (`setup()`)

Reihenfolge beim Booten:

1. **Serial** (115200 Baud) – Statusausgaben.
2. **Werksreset-Prüfung** – ist GPIO0 beim Einschalten gedrückt und bleibt 3 s gedrückt,
   werden NVS und `config.json` gelöscht und das Gerät startet neu (siehe §11).
3. **I²C** (`Wire.begin`, SDA 21 / SCL 22) und **LittleFS** mounten.
4. **`loadNetConfig()`** – Netzwerk-/Portal-/IMU-Einstellungen aus NVS.
5. **`loadConfig()`** – Kanalkonfiguration aus `/config.json`. Bei Lese-/JSON-Fehler
   bleiben die Default-Kanäle aktiv und die Datei wird **nicht** überschrieben.
6. **I²C-Scan** – gefundene Adressen werden im Serial Monitor gelistet.
7. **ADS1115 @ 0x48**, Gain `GAIN_TWOTHIRDS` (±6,144 V Messbereich, passend zur 3,3-V-Beschaltung).
8. **MPU6050 @ 0x68 oder 0x69**, Accel ±8 g, Gyro ±250 °/s, Filterbandbreite **94 Hz**
   (hoch genug, um Aufprall-Transienten zu erfassen). Fehlt der Sensor, läuft die
   Fake-IMU (Demo-Bewegung).
9. **Gyro-Bias-Kalibrierung** – 100 Samples (~0,5 s), Board dabei ruhig halten.
10. **WiFi** im `WIFI_AP_STA`-Modus: der **AP startet immer sofort**; bei hinterlegten
    STA-Zugangsdaten wird zusätzlich max. 10 s auf eine Router-Verbindung gewartet.
11. **MQTT** – Server/Port gesetzt, Empfangspuffer auf **4096 Byte** (damit vollständige
    Konfig-Payloads ankommen).
12. **`ensureStaServices()`** – mDNS (`boatopenio.local`) + OTA, sobald STA verbunden ist.
13. **Webserver + Captive Portal** starten.
14. **Task-Watchdog** (10 s, Panik-Reset) auf die loop-Task registrieren.

---

## 3. Betriebsmodi: TEST vs. LIVE

| Modus | Kanalwerte | Zweck |
|-------|-----------|-------|
| **TEST** (Default) | Simulierte Sinuswerte (`2,5 V + sin(...)·0,3`) | Gefahrlose Inbetriebnahme ohne echte Sensoren |
| **LIVE** | Echte ADS1115-Messung über den MUX | Realbetrieb |

Umschalten im Portal unter **Aktionen**. Der Modus wird in NVS gespeichert und beim
Booten wiederhergestellt.

> **Wichtig:** Im TEST-Modus werden Aufprall-Alarme auf `boat/test/alarm/*` statt auf die
> echten `boat/alarm/*`-Topics publiziert – ein frisch aufgesetztes Gerät löst so keine
> realen Alarm-Anzeigen aus.

---

## 4. Messkette

### 4.1 Analogkanäle (alle 16)

Alle 16 Klemmen teilen sich **einen** ADS1115. Der CD74HC4067-Multiplexer routet die
gewählte Klemme auf ADS-Pin A0:

```
selectChannel(klemme-1)  →  S0–S3 setzen  →  500 µs Settling  →  ADS A0 lesen
```

Pro Messung werden **4 Samples gemittelt** (`ADS_SAMPLES`), um Störungen im Motorraum
(Lichtmaschine, Zündimpulse) zu glätten. Die Umrechnung in den publizierten Wert:

```
publizierter_wert = rohspannung × faktor + offset
```

Nur als **aktiv** markierte Kanäle werden gelesen und publiziert. Getaktet alle
**2 s** (`READ_INTERVAL`).

### 4.2 Lage & Aufprall (MPU6050)

- **50 Hz Sampling** (`IMU_INTERVAL` = 20 ms): Komplementärfilter (α = 0,98) aus Gyro +
  Accel liefert Pitch/Roll; parallel wird die Beschleunigungs­magnitude auf Aufprall geprüft.
- **Pitch/Roll-Publish** entkoppelt nur alle **1 s** (`IMU_PUBLISH_INTERVAL`), um den
  Broker nicht zu fluten. Ausgegeben werden montage-offset- und invertierungs-korrigierte Werte.
- **Aufprallerkennung:** übersteigt die Netto-Beschleunigung `|a|/9,81 − 1` den Schwellwert
  **0,5 g**, gilt ein Impact als aktiv; der Spitzenwert wird gehalten. Nach **8 s** ohne
  neuen Ausschlag (`IMPACT_DECAY`) wird der Alarm zurückgesetzt.

| Peak (g) | Schweregrad |
|----------|-------------|
| < 1,0 | `LEICHT` |
| < 3,0 | `MITTEL` |
| < 6,0 | `STARK` |
| ≥ 6,0 | `KRITISCH` |

---

## 5. MQTT-Schnittstelle

### 5.1 Verbindung

- Client-ID `BoatOpenIO`, optional mit User/Passwort.
- **Last Will (LWT):** `boatopenio/status` = `offline` (retained), bei Verbindung → `online`.
- **Reconnect-Backoff:** 5 s nach Erfolg, **30 s** nach Fehlversuch – ein toter Broker
  bremst so nicht dauerhaft die loop-Schleife.
- **Beim (Neu-)Verbinden** wird der retained Alarm-Zustand bereinigt (siehe §5.2), sofern
  gerade kein Impact aktiv ist – verhindert einen dauerhaft „hängenden" Alarm nach einem
  Spannungseinbruch mitten im Aufprall.

### 5.2 Publizierte Topics (ESP32 → Broker)

| Topic | Payload | Retained | Intervall |
|-------|---------|:--------:|-----------|
| `boat/io/<sensor>` oder eigener `topic` | Float-Wert | nein | 2 s |
| `boat/io/pitch` | Float °, korrigiert | ja | 1 s |
| `boat/io/roll` | Float °, korrigiert | ja | 1 s |
| `boatopenio/status` | `online` / `offline` | ja | 10 s + LWT |
| `boatopenio/uptime` | Sekunden seit Boot | nein | 10 s |
| `boatopenio/rssi` | WLAN-Signal (dBm) | nein | 10 s |
| `boatopenio/mode` | `TEST` / `LIVE` | ja | 10 s |
| `boat/alarm/impact_g` ¹ | Peak-G-Wert | ja | bei Ereignis |
| `boat/alarm/impact_severity` ¹ | `LEICHT`/`MITTEL`/`STARK`/`KRITISCH` | ja | bei Ereignis |
| `boat/alarm/impact_active` ¹ | `true` / `false` | ja | bei Ereignis |

¹ Im **TEST**-Modus lautet das Präfix `boat/test/alarm/` statt `boat/alarm/`.

### 5.3 Abonnierte Topics (Broker → ESP32)

**`boatopenio/config`** – vollständige oder partielle `config.json`-Struktur (JSON).
Überschreibt die Kanalkonfiguration im RAM und speichert sie (nur bei tatsächlicher
Änderung) in LittleFS. Es werden nur die im Payload vorhandenen Felder geändert.
Sensor- und Topic-Strings werden serverseitig gefiltert (nur `A–Z a–z 0–9 _ - . /`),
`klemme` wird auf 1–16 begrenzt. Nach Übernahme wird `boatopenio/status` = `config_updated`
gemeldet.

**Beispiel – einen Kanal mit Signal-K-Topic aktivieren:**
```json
{ "kanaele": [
  { "klemme": 3, "sensor": "oeldruck", "faktor": 1.25, "aktiv": true,
    "topic": "vessels/self/propulsion/engine/oilPressure" }
] }
```

> **Signal-K-Einheiten:** Die Firmware sendet **Rohwerte** (Grad, °C, bar, %), nicht die
> von Signal K erwarteten SI-Einheiten (rad, K, Pa, Ratio). Bei Bedarf über Faktor/Offset
> oder im SK-Server umrechnen.

---

## 6. Web-Portal & HTTP-API

### 6.1 Erreichbarkeit

| Adresse | Voraussetzung |
|---------|---------------|
| `http://192.168.4.1` | Immer – über den integrierten AP |
| `http://boatopenio.local` | Bei WLAN-Verbindung (mDNS) |
| `http://<IP>` | Bei WLAN-Verbindung (IP im Serial Monitor) |

### 6.2 Routen

| Route | Methode | Schutz | Funktion |
|-------|---------|--------|----------|
| `/` | GET | Auth | Dashboard (gesamte Web-UI) |
| `/setup` | GET | nur Ersteinrichtung, danach Auth | Ersteinrichtungs-Formular |
| `/dosetup` | POST | Ersteinrichtung/Auth + CSRF | Portal-/AP-Zugangsdaten setzen |
| `/save` | POST | Auth + CSRF | Kanalkonfiguration speichern |
| `/savenet` | POST | Auth + CSRF | WiFi/MQTT speichern (+ Neustart) |
| `/savesec` | POST | Auth + CSRF | AP-/Portal-Zugangsdaten (+ Neustart) |
| `/testmode` | POST | Auth + CSRF | TEST/LIVE umschalten (+ Neustart) |
| `/reboot` | POST | Auth + CSRF | Neustart |
| `/calibrate` | POST | Auth + CSRF | Aktuellen Pitch/Roll als Montage-Offset setzen |
| `/setinvert` | POST | Auth + CSRF | Pitch-/Roll-Invertierung setzen |
| `/api/values` | GET | Auth | Aktive Kanalwerte + Pitch/Roll (korrigiert), JSON |
| `/api/raw` | GET | Auth | Rohspannungen ADS A0–A3 (Diagnose), JSON |
| `/api/adc?ch=<1–16>` | GET | Auth | MUX auf Klemme wählen, gemittelte Spannung, JSON |
| `/api/imu` | GET | Auth | Pitch/Roll roh + korrigiert, Offsets, Gyro-Bias, JSON |

`/api/adc` liest immer die echte ADS-Hardware (unabhängig vom TEST-Modus) und dient dem
Kalibrierungs-Rechner (§8).

### 6.3 JSON-Antwortbeispiele

```
GET /api/values → {"batterie_starter":12.48,"tank_diesel":47.0,"pitch":1.2,"roll":-0.4}
GET /api/adc?ch=3 → {"ok":true,"v":1.8342}
GET /api/imu    → {"pitch_raw":1.4,"roll_raw":-0.3,"pitch_corr":1.2,...,"bias_x":0.0012,...}
```

---

## 7. Sicherheitsmodell

- **Ersteinrichtung erzwungen:** Solange kein Portal-Passwort gesetzt ist, leiten alle
  Routen auf `/setup` um. `/setup` und `/dosetup` sind nach abgeschlossener Einrichtung
  nur noch mit Admin-Login erreichbar.
- **HTTP Basic Auth** auf allen Routen (Dashboard, POST-Aktionen, JSON-APIs). Portal-User
  und -Passwort sind unabhängig vom AP-Passwort.
- **CSRF-Schutz:** POST-Requests werden nur akzeptiert, wenn `Origin`/`Referer` zum eigenen
  Host passen (Fremd-Seiten können so keine Aktionen auslösen).
- **Eingabe-Filterung:** Sensor-/Topic-Namen werden auf `A–Z a–z 0–9 _ - . /` beschränkt –
  schließt MQTT-Wildcards (`#`, `+`) und HTML/Script-Injection aus.
- **Ausgabe-Escaping:** Alle gespeicherten Werte werden beim Rendern HTML-escaped.
- **Werksschutz:** Solange das AP-Passwort dem Standard (`boatopenio`) entspricht, blendet
  das Portal oben ein gelbes Warnbanner ein.

---

## 8. Kalibrierungs-Rechner (Web-UI)

Zwei-Punkt-Linearkalibrierung, die **Faktor und Offset** aus zwei bekannten Zuständen
bestimmt (`Wert = Faktor·V + Offset`):

- Pro Punkt trägt man den **echten Wert** des Zustands (z. B. 40 °C und 100 °C, oder 50 %
  und 75 %) sowie die dazu gemessene ADS-Spannung ein.
- **„A0 lesen"** holt die aktuelle Spannung des gewählten Kanals per `/api/adc`.
- **„In Kanal übernehmen"** schreibt das Ergebnis direkt in die Faktor-/Offset-Felder der
  Kanaltabelle.

Berechnung:
```
Faktor = (Wert₂ − Wert₁) / (V₂ − V₁)
Offset = Wert₁ − Faktor · V₁
```

### Ω-Modus (resistive VDO-Geber)

Umschaltbar auf **Widerstands-Eingabe** für resistive Geber (z. B. Tank 10–180 Ω). Statt
Spannung gibt man je Punkt den **Widerstand** ein; die Firmware rechnet ihn über die
Teilerparameter in die ADS-Spannung um:

| Parameter | Default | Bedeutung |
|-----------|---------|-----------|
| Referenzspannung | 3,3 V | Speisespannung des Teilers |
| Vorwiderstand | 1000 Ω | Fester Widerstand (Bourns 4116R-1-102, 1 kΩ) |
| Schaltung | Pull-up | `Geber → GND` (Pull-up) oder `Geber → Vref` (Pull-down) |

Umrechnung (Pull-up): `V = Vref · R / (R_fix + R)`, Umkehrung `R = R_fix · V / (Vref − V)`.
Im Ω-Modus liefert „A0 lesen" den zurückgerechneten Widerstand.

---

## 9. Persistenz

### 9.1 NVS (Preferences, Namespace `boatopenio`)

Überlebt Firmware-Updates. Nur über das Portal änderbar oder per Werksreset löschbar.

| Schlüssel | Typ | Default | Beschreibung |
|-----------|-----|---------|--------------|
| `wifi_ssid` / `wifi_pass` | string | `""` | WLAN-Zugangsdaten |
| `mqtt_ip` | string | `192.168.1.100` | Broker-Adresse |
| `mqtt_port` | string | `1883` | Broker-Port (1–65535, validiert) |
| `mqtt_user` / `mqtt_pass` | string | `""` | MQTT-Auth |
| `testmode` | bool | `true` | TEST-Modus aktiv |
| `pitch_off` / `roll_off` | float | `0.0` | IMU-Montage-Offset (°) |
| `pitch_inv` / `roll_inv` | bool | `false` | Achsen-Invertierung |
| `ap_ssid` | string | `BoatOpenIO-Setup` | AP-Name |
| `ap_pass` | string | `boatopenio` | AP-Passwort |
| `portal_user` | string | `admin` | Portal-Benutzer |
| `portal_pass` | string | `""` | Portal-Passwort (leer = Ersteinrichtung offen) |

### 9.2 LittleFS `/config.json`

Kanalkonfiguration (16 Einträge). Wird beim Booten geladen und **nur bei tatsächlicher
Änderung** neu geschrieben (schont den Flash). Lokale Quelle: `firmware_src/data/config.json`.

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `klemme` | int 1–16 | Physische Klemme (auf 1–16 begrenzt) |
| `sensor` | string | Sensorname → Topic-Suffix, wenn `topic` leer |
| `einheit` | string | Einheitenkürzel (nur Anzeige) |
| `faktor` | float | Spannungsmultiplikator |
| `offset` | float | Additiver Offset |
| `aktiv` | bool | Kanal lesen/publizieren |
| `topic` | string | Eigener MQTT-Topic (leer = `boat/io/<sensor>`) |

Die Felder `ads`/`pin` existieren im Format aus historischen Gründen, werden aber im
Single-ADS-Design nicht mehr ausgewertet.

---

## 10. Netzwerk-Resilienz & OTA

- **AP immer aktiv:** Der Access Point läuft unabhängig vom WLAN-Status – so bleibt das
  Portal auch ohne Router erreichbar.
- **WiFi-Reconnect:** Ist STA konfiguriert aber getrennt, erzwingt `loop()` alle **30 s**
  einen neuen Verbindungsversuch. Sobald die Verbindung steht, werden mDNS und OTA
  automatisch nachgezogen – wichtig für den Kaltstart nach Stromausfall, wenn der Router
  langsamer bootet als der ESP32.
- **OTA-Update:** Bei WLAN-Verbindung über die Arduino IDE (Netzwerk-Port „BoatOpenIO bei
  \<IP\>"). Passwort = Portal-Passwort (vor der Ersteinrichtung `boatopenio` als Fallback).
  Während des Uploads wird die loop-Task vom Watchdog abgemeldet, damit der TWDT das Update
  nicht abbricht.
- **Watchdog:** 10 s Task-Watchdog mit Panik-Reset auf die loop-Task. Hängt die Schleife
  (z. B. blockierender I²C-Bus), rebootet das Gerät automatisch.

---

## 11. Werksreset

| Aktion | Auslöser | Wirkung |
|--------|----------|---------|
| **WiFi-Reset** | GPIO0 **im Betrieb** 3 s halten | Löscht nur `wifi_ssid`/`wifi_pass`, Neustart. Alles andere bleibt. |
| **Voll-Reset** | GPIO0 **beim Booten** gedrückt halten, 3 s | Löscht kompletten NVS + `config.json`. Danach ist erneut die Ersteinrichtung nötig. |

Nach einem Voll-Reset muss `config.json` ggf. neu ins LittleFS geladen werden
(Arduino IDE → *ESP32 Sketch Data Upload*).

---

## 12. Zeit- & Kennwerte-Referenz

| Konstante | Wert | Bedeutung |
|-----------|------|-----------|
| `READ_INTERVAL` | 2000 ms | Kanäle lesen/publizieren |
| `IMU_INTERVAL` | 20 ms (50 Hz) | IMU-Sampling / Aufprallprüfung |
| `IMU_PUBLISH_INTERVAL` | 1000 ms | Pitch/Roll per MQTT |
| `STATUS_INTERVAL` | 10 000 ms | Heartbeat (status/uptime/rssi/mode) |
| `ADS_SAMPLES` | 4 | Mittelung pro Kanalmessung |
| `WDT_TIMEOUT` | 10 s | Task-Watchdog |
| `IMPACT_THRESHOLD` | 0,5 g | Aufprall-Schwelle |
| `IMPACT_DECAY` | 8000 ms | Alarm-Rückfallzeit |
| MQTT-Puffer | 4096 B | Empfangspuffer (Config-Payloads) |
| WiFi-Reconnect | 30 s | Intervall bei getrennter STA |
| MQTT-Backoff | 5 s / 30 s | nach Erfolg / nach Fehlversuch |

---

## 13. Fehlersuche (Serial Monitor, 115200 Baud)

| Symptom | Ursache / Prüfung |
|---------|-------------------|
| `ADS1115 @ 0x48 nicht gefunden` | I²C-Verkabelung, Adress-Jumper, VCC/GND prüfen (I²C-Scan im Log). |
| `MPU6050 nicht gefunden – Fake-IMU aktiv` | Sensor auf 0x68/0x69? AD0-Pegel prüfen. Pitch/Roll sind dann simuliert. |
| Kanäle springen stark | LIVE-Modus im Motorraum ohne saubere Masse; ggf. Verkabelung/Abschirmung prüfen (4-fach-Mittelung ist bereits aktiv). |
| Keine MQTT-Daten | Broker-IP/Port, Erreichbarkeit; `boatopenio/status` = `online`? RSSI im Log. |
| Portal fragt Login, das AP-Passwort passt nicht | Portal-Login ≠ AP-Passwort. Portal-User/-Passwort aus der Ersteinrichtung nutzen. |
| POST wird mit `403 CSRF blocked` abgewiesen | Request kam nicht von der Portal-Seite selbst (fremder Origin). Direkt im Portal bedienen. |
| Config-Update per MQTT wirkt nicht | Payload > 4096 B, kein gültiges JSON, oder Sensor/Topic durch Filter geleert. |
| Aufprall-Alarm bleibt „true" hängen | Sollte durch Connect-Bereinigung verschwinden; im TEST-Modus liegen Alarme unter `boat/test/alarm/*`. |

---

*Teil der Serie „Logbook Without Posing" · [BoatOpenIO](https://github.com/bigbrainlabs/BoatOpenIO)*
