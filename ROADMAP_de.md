# BoatOpenIO – Roadmap

> Feature Freeze aktiv. Die folgenden Punkte zeigen geplante Richtungen, keine verbindlichen Zusagen.  
> Pull Requests für alles auf dieser Liste sind willkommen.

---

## Fertig – v2.2 (aktuell)

**Sicherheit & Härtung**
- [x] Ersteinrichtungs-Routen (`/setup`, `/dosetup`) nach Abschluss auth-geschützt
- [x] JSON-APIs (`/api/values`, `/api/raw`, `/api/imu`) erfordern Authentifizierung
- [x] CSRF-Schutz auf allen POST-Endpunkten (Origin/Referer-Prüfung)
- [x] Eingabe-Filterung (Sensor/Topic-Whitelist) + HTML-Escaping gegen Injection

**Zuverlässigkeit im Feld**
- [x] Automatischer WiFi-Reconnect nach Ausfall (Kaltstart nach Stromausfall)
- [x] OTA-Update wird nicht mehr vom Watchdog abgebrochen
- [x] Config-Integrität: größere JSON-Puffer, overflow-sicheres Speichern (kein Datenverlust)
- [x] Retained Aufprall-Alarm wird beim MQTT-Connect bereinigt (kein „hängender" Alarm)
- [x] TEST-Modus publiziert Alarme auf `boat/test/alarm/*` statt echter Topics
- [x] Flash-Wear reduziert (Config nur bei Änderung schreiben) · ADC-Mittelung · Klemme-Validierung

**Kalibrierung**
- [x] Zwei-Punkt-Kalibrierungs-Rechner (berechnet Faktor **und** Offset)
- [x] Ω-Modus für resistive VDO-Geber (+ `/api/adc`-Endpunkt zum Live-Lesen)

**Dokumentation**
- [x] Neue Firmware-/Backend-Referenz (`docs/backend.md` / `backend_de.md`)
- [x] `setup.md` und `configuration.md` auf aktuellen Stand gebracht

---

## Fertig – v2.1

**Hardware**
- [x] 4-Platinen-Architektur: Mainboard, Eingangsboard, ESP32-Adapterboard, VCC-Verteiler
- [x] Einzelner ADS1115 @ 0x48 + CD74HC4067 16:1 MUX (alle 16 Kanäle über einen ADC)
- [x] Hardwareschutz an allen 16 Eingängen: Bourns Widerstandsnetzwerk + SP0503BAHTG Zener-Array (3,3V Clamp)
- [x] Alle Komponenten auf Sockeln — kein Bauteil fest verlötet
- [x] 8× Ausgangskanal-Header auf dem Mainboard vorbereitet (GPIO 4/5/13/16–19/23)
- [x] Steckbare Mini-Platinen: VT, PD, PU, ST, ISP, OPT, DIR, GND
- [x] Gerber-Dateien bestellfertig für JLCPCB (alle 4 Platinen)

**Firmware**
- [x] 16-Kanal-Analogmessung über MUX (Faktor + Offset-Kalibrierung pro Kanal)
- [x] MPU6050 IMU: Pitch / Roll / Aufprallerkennung mit Schweregradabstufung
- [x] MQTT: alle Sensorwerte, LWT, Auth, frei konfigurierbarer Topic pro Kanal
- [x] Signal K-Pfadvorschläge (SK-Button im Web-UI)
- [x] OTA-Firmware-Updates (ArduinoOTA)
- [x] mDNS (`boatopenio.local`)
- [x] WiFiManager Captive Portal (WLAN-Einrichtung beim ersten Start)
- [x] LittleFS Konfigurationsspeicher (config.json)
- [x] Watchdog (automatischer Neustart bei Firmware-Hänger)
- [x] Web-UI: zweisprachig DE/EN, dunkles Theme, responsiv, Live-Werte, Kalibrierung

---

## Geplant – v2.x

**Firmware**
- [ ] Ausgangskanäle: Firmware-Unterstützung für 8× GPIO-Ausgänge (An/Aus, PWM)
- [ ] Ausgangs-Mini-Platinen: Relais- und MOSFET-Designs
- [ ] DS18B20 1-Wire Temperatursensor-Unterstützung
- [ ] Web-UI: eingebetteter config.json-Editor (JSON direkt im Browser bearbeiten)
- [ ] Web-UI: OTA-Upload direkt aus dem Browser heraus
- [ ] Veraltete Felder `ads` / `pin` aus Config-Schema und Firmware entfernen

**Dokumentation**
- [ ] Node-RED Integrationsbeispiel (MQTT → Dashboard)
- [ ] Signal K Server Einrichtungsanleitung (BoatOpenIO als Datenquelle)

---

## Ideen / Backlog

Mögliche Richtungen ohne festen Zeitplan.

- [ ] **BoatOS Auto-Discovery** — alle aktiven Topics und Einheiten beim Verbinden ankündigen
- [ ] **Zweites ESP32-Adapterboard** — kleinerer Footprint oder ESP32-S3 / C3 Variante
- [ ] **PWM-Gauge-Ausgang** — analoge Cockpit-Instrumente aus digitalen Werten ansteuern
- [ ] **Datenprotokollierung** — optionales SD-Karten-Modul für lokale Wertehistorie
- [ ] **NMEA-2000-Bridge** — CAN-Bus-Modul, beide Richtungen:
  - [ ] **Senden** — aktive Kanäle auf NMEA 2000 ausgeben
  - [ ] **Empfangen** — bestehende NMEA-2000-Geräte am Bus mitlesen und als MQTT publizieren
- [ ] **Kanalbasierte Grenzwerte** — konfigurierbare Min/Max-Schwellen mit MQTT-Alarm-Topic
- [ ] **Gehäuse** — Aufnahme für alle 4 Platinen mit dedizierten Steckplätzen für Mini-Platinen (3D-Druck / Hutschiene)

---

## Nicht geplant

Um den Projektkern zu erhalten:

- Keine Cloud-Abhängigkeit, keine Herstellerkonten, keine Abonnements
- Keine proprietäre App — das Web-UI läuft vollständig auf dem ESP32
- Keine Unterstützung für Sensoren mit Stromspeisung (4–20mA-Schleifen) — außerhalb des Rahmens dieser Hardware-Revision

---

## Mitmachen

Fehler gefunden? Eigenes Mini-Platinen-Design? Pull Request oder Issue auf GitHub eröffnen.

**[github.com/bigbrainlabs/BoatOpenIO](https://github.com/bigbrainlabs/BoatOpenIO)**
