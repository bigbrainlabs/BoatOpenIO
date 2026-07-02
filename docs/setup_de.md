# BoatOpenIO – Einrichtungsanleitung

## Voraussetzungen

- Arduino IDE 1.8+ oder 2.x
- ESP32-Board-Paket installiert (`espressif/arduino-esp32`)
- Benötigte Bibliotheken (über den Library Manager installieren):
  - `Adafruit ADS1X15`
  - `Adafruit MPU6050`
  - `Adafruit Unified Sensor`
  - `PubSubClient`
  - `ArduinoJson` (v7)
  - `ESP32 Arduino` (enthält `WebServer`, `ESPmDNS`, `ArduinoOTA`, `Preferences`, `LittleFS`)
- [ESP32 Sketch Data Upload Plugin](https://github.com/me-no-dev/arduino-esp32fs-plugin) für LittleFS

---

## 1. Firmware flashen

1. `firmware_src/firmware_src.ino` in der Arduino IDE öffnen
2. Board auswählen: **ESP32 Dev Module** (oder die eigene Variante)
3. Upload-Geschwindigkeit: **921600**
4. Richtigen COM-Port auswählen
5. **Upload** klicken

---

## 2. Kanalkonfiguration hochladen (LittleFS)

Die Standard-Kanalzuordnung liegt in `firmware_src/data/config.json`.

1. [ESP32 Sketch Data Upload Plugin](https://github.com/me-no-dev/arduino-esp32fs-plugin) installieren
2. In der Arduino IDE: **Tools → ESP32 Sketch Data Upload**
3. Der `data/`-Ordner wird auf das LittleFS des ESP32 hochgeladen

> **Hinweis:** Dieser Schritt ist nur beim ersten Flash oder nach einem Werksreset nötig. Änderungen über das Web-Portal werden automatisch in LittleFS gespeichert und überleben Neustarts.

---

## 3. Erster Start & Sicherheitseinrichtung

Beim ersten Start öffnet der ESP32 seinen Access Point mit den Standard-Zugangsdaten:

```
AP:       BoatOpenIO-Setup
Passwort: boatopenio
IP:       192.168.4.1
```

Mit dem AP verbinden. Auf Mobilgeräten öffnet sich die Einrichtungsseite automatisch (Captive Portal). Am Desktop den Browser auf `http://192.168.4.1/setup` öffnen.

> **Wichtig:** Das Portal ist erst zugänglich, nachdem die Ersteinrichtung abgeschlossen wurde. Alle Routen leiten auf `/setup` um, solange kein Portal-Passwort gesetzt ist.

### Ersteinrichtung (`/setup`)

Das Formular muss ausgefüllt werden, bevor irgendetwas anderes zugänglich ist:

| Feld | Anforderung | Beschreibung |
|------|-------------|--------------|
| Portal-Benutzer | erforderlich | Benutzername für den Portal-Login |
| Portal-Passwort | min. 6 Zeichen | Schützt das gesamte Konfigurationsportal |
| Passwort bestätigen | muss übereinstimmen | |
| AP-Name (SSID) | erforderlich | Name des vom ESP32 ausgestrahlten WLANs |
| AP-Passwort | min. 8 Zeichen | Schlüssel zum Verbinden mit dem AP |

**Einrichtung abschließen** klicken. Der ESP32 speichert die Zugangsdaten in NVS und startet neu.

> **Nach der Einrichtung:** Das Portal erfordert bei jedem Aufruf HTTP Basic Auth. Der Browser fragt Benutzername und Passwort ab — das sind die soeben gesetzten Zugangsdaten, nicht das AP-Passwort.

---

## 4. WiFi & MQTT konfigurieren

Portal öffnen (`http://192.168.4.1`, Zugangsdaten aus Schritt 3), nach unten zu **WiFi & MQTT** scrollen:

| Feld | Beschreibung |
|------|--------------|
| WiFi SSID | Name des Heim-/Boarnetzwerks |
| WiFi Passwort | Leer lassen = bestehendes Passwort beibehalten |
| MQTT Server | IP oder Hostname des MQTT-Brokers |
| MQTT Port | Standard: `1883` |
| MQTT User | Leer lassen wenn keine Authentifizierung nötig |
| MQTT Passwort | Leer lassen = bestehendes Passwort beibehalten |

**Speichern & Neustart** klicken. Der ESP32 startet neu und verbindet sich mit dem Router.  
Der AP bleibt unter `192.168.4.1` immer aktiv, unabhängig vom WiFi-Status.

---

## 5. Kanäle zuweisen

Im Portal zu **Kanalzuordnung** scrollen:

| Spalte | Beschreibung |
|--------|--------------|
| # | Klemmennummer (1–16, physischer Anschluss) |
| Sensor | Sensorname — wird als MQTT-Topic-Suffix verwendet, wenn kein eigener Topic gesetzt ist |
| Faktor | Multiplikator: `Ausgabe = Spannung × Faktor + Offset` |
| Offset | Additiver Offset nach dem Faktor |
| Einheit | Einheitenbezeichnung (nur Anzeige, nicht in MQTT) |
| Topic | Eigener MQTT-Topic. Leer lassen → publiziert auf `boat/io/<sensor>`. Mit dem **SK**-Button wird der passende Signal K-Pfad für gängige Sensornamen automatisch eingetragen. |
| Aktiv | Nur aktive Kanäle werden gelesen und publiziert |

> Alle 16 Kanäle teilen sich einen einzigen ADS1115 @ 0x48. Der CD74HC4067 MUX schaltet die Kanäle der Reihe nach durch — keine ADS- oder Pin-Auswahl nötig.

**Konfiguration speichern** klicken. Einstellungen werden sofort in LittleFS geschrieben, kein Neustart nötig.

---

## 6. Zugangsdaten ändern (Sicherheit)

Der Bereich **Sicherheit** im Portal erlaubt das Ändern von AP- und Portal-Zugangsdaten jederzeit.

| Feld | Anforderung | Wirkung |
|------|-------------|---------|
| AP-Name (SSID) | erforderlich | Neuer AP-Name nach Neustart |
| AP-Passwort | min. 8 Zeichen, leer = beibehalten | Neuer AP-Schlüssel nach Neustart |
| Portal-Benutzer | erforderlich | Neuer Portal-Login-Benutzername |
| Portal-Passwort | min. 6 Zeichen, leer = beibehalten | Neues Portal-Login-Passwort |
| Passwort bestätigen | muss übereinstimmen | |

> **Warnbanner:** Solange das AP-Passwort noch dem Werksstandard (`boatopenio`) entspricht, wird oben im Portal ein gelber Warnhinweis angezeigt. Hier ändern.

---

## 7. IMU kalibrieren (optional)

Das MPU6050 läuft mit zweistufiger Kalibrierung:

**Gyro-Bias** wird automatisch bei jedem Neustart gemessen (~0,5 s, Board während des Einschaltens ruhig halten).

**Montage-Offset** korrigiert den Einbauwinkel des Boards:

1. Im Portal zu **IMU Kalibrierung** scrollen
2. Boot auf ruhigem Wasser (oder einer ebenen Fläche) ausrichten
3. Den Live-Pitch/Roll-Wert beobachten bis er stabil ist
4. **Jetzt Null setzen** klicken

Der Offset wird in NVS gespeichert und auf alle zukünftigen Messwerte und MQTT-Publizierungen angewendet.

---

## 8. Test-Modus vs. Live-Modus

Die Firmware startet standardmäßig im **Test-Modus**.

| Modus | Verhalten |
|-------|-----------|
| TEST | Simulierte sinusförmige Sensorwerte — sicher für die Inbetriebnahme |
| LIVE | Echte ADC-Messwerte von den ADS1115-Chips |

Umschalten im Bereich **Aktionen** im Portal.

---

## 9. OTA Firmware-Updates

Sobald der ESP32 mit dem WLAN verbunden ist, kann die Firmware kabellos aktualisiert werden:

1. In der Arduino IDE den Netzwerk-Port auswählen: **BoatOpenIO bei \<IP\>**
2. Das **Portal-Passwort** (aus der Ersteinrichtung) eingeben, wenn danach gefragt wird
3. **Upload** klicken — kein USB-Kabel nötig

mDNS-Hostname: `boatopenio.local` (in Netzwerken mit mDNS-Unterstützung).

> **Hinweis:** Vor Abschluss der Ersteinrichtung wird `boatopenio` als Fallback-OTA-Passwort verwendet.

---

## 10. Werksreset

**Im Betrieb** — GPIO0-Taste 3 Sekunden gedrückt halten:  
→ Löscht nur die WiFi-Zugangsdaten. Alle anderen Einstellungen (MQTT, Kanäle, Portal-Zugangsdaten, IMU-Offset) bleiben erhalten.

**Beim Booten** — GPIO0-Taste beim Einschalten gedrückt halten, 3 Sekunden halten:  
→ Vollständiger Werksreset: löscht alle NVS-Einträge (WiFi, MQTT, Portal-Zugangsdaten, AP-Zugangsdaten, IMU-Offsets) und entfernt `config.json` aus LittleFS.

Nach einem vollständigen Werksreset ist erneut die Ersteinrichtung unter `/setup` erforderlich, bevor das Portal zugänglich wird. `config.json` über das LittleFS-Tool neu hochladen (Schritt 2).
