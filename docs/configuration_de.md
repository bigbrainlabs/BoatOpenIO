# BoatOpenIO – Konfigurationsreferenz

Es gibt drei Wege BoatOpenIO zu konfigurieren. Sie ergänzen sich — nicht alle müssen genutzt werden.

---

## Übersicht

| Was | Wo gespeichert | Wie ändern |
|-----|---------------|------------|
| Kanalzuordnung | LittleFS (`config.json`) | Web-Portal · MQTT · LittleFS-Upload |
| WiFi-Zugangsdaten | NVS (Preferences) | Web-Portal |
| MQTT-Verbindung | NVS (Preferences) | Web-Portal |
| AP-Zugangsdaten (SSID + Passwort) | NVS (Preferences) | Web-Portal (Sicherheit) · Ersteinrichtung |
| Portal-Zugangsdaten (Benutzer + Passwort) | NVS (Preferences) | Web-Portal (Sicherheit) · Ersteinrichtung |
| Test/Live-Modus | NVS (Preferences) | Web-Portal |
| IMU-Montage-Offset | NVS (Preferences) | Web-Portal (IMU-Kalibrierung) |
| Gyro-Bias | Nur RAM | Automatisch bei jedem Neustart gemessen |

NVS überlebt Firmware-Updates. LittleFS `config.json` wird nur überschrieben, wenn explizit über das Portal, MQTT oder einen Datei-Upload gespeichert wird.

---

## 1. Web-Portal

### Erreichbarkeit

| Adresse | Voraussetzung |
|---------|---------------|
| `http://192.168.4.1` | Immer — über den integrierten AP |
| `http://boatopenio.local` | Bei WLAN-Verbindung im lokalen Netz (mDNS) |
| `http://<IP>` | Bei WLAN-Verbindung im lokalen Netz (IP im Serial Monitor) |

Das Portal erfordert **HTTP Basic Auth** nach der Ersteinrichtung. Portal-Benutzername und -Passwort eingeben (gesetzt bei `/setup`). Diese sind unabhängig vom AP-Passwort.

### Ersteinrichtung (`/setup`)

Beim ersten Start leiten alle Routen auf `/setup` um. Das Portal ist erst nach dem Setzen eines Portal-Passworts zugänglich.

| Feld | Anforderung |
|------|-------------|
| Portal-Benutzer | erforderlich |
| Portal-Passwort | min. 6 Zeichen |
| AP-Name (SSID) | erforderlich |
| AP-Passwort | min. 8 Zeichen |

---

### WiFi & MQTT

```
WiFi SSID       Name des Heim-/Bordsnetzes
WiFi Passwort   Leer = bestehendes Passwort beibehalten
MQTT Server     IP oder Hostname, z.B. 192.168.1.100
MQTT Port       Standard: 1883
MQTT User       Leer wenn keine Authentifizierung nötig
MQTT Passwort   Leer = bestehendes Passwort beibehalten
```

→ In NVS gespeichert. Wirksam nach Neustart.

---

### Kanalzuordnung

Jede der 16 Klemmen kann unabhängig konfiguriert werden:

```
Sensor    Sensorname — als MQTT-Topic-Suffix verwendet, wenn kein Topic gesetzt ist
Faktor    Ausgabe = Rohspannung × Faktor + Offset
Offset    Additiver Offset nach dem Faktor
Einheit   Einheitenkürzel, z.B. V · °C · bar · %  (nur Anzeige)
Topic     Eigener MQTT-Topic — leer lassen für boat/io/<sensor>
          SK-Button füllt Signal K-Pfad automatisch ein
Aktiv     Nur aktive Kanäle werden gelesen und an MQTT publiziert
```

> Alle 16 Kanäle teilen sich einen einzigen ADS1115 @ 0x48. Der CD74HC4067 MUX schaltet die Kanäle der Reihe nach durch — keine ADS- oder Pin-Auswahl nötig.

→ In LittleFS `config.json` gespeichert. Sofort wirksam, kein Neustart nötig.

---

### IMU-Kalibrierung

```
Jetzt Null setzen    Speichert aktuellen Pitch/Roll als Montage-Offset in NVS
                     Wird auf alle nachfolgenden Messwerte und MQTT-Publizierungen angewendet
```

Gyro-Bias (automatisch): beim Booten über 100 Samples gemessen (~0,5 s).  
In der Kalibrierungs-Card angezeigt: `Gyro-Bias: X=... Y=... Z=... rad/s`

Die Kalibrierungs-Card zeigt drei Zeilen für Pitch und Roll:

| Zeile | Beschreibung |
|-------|--------------|
| Rohwert | Gyro-Bias-korrigierter Winkel, vor Montage-Offset |
| Korrigiert | Endwert — wird an MQTT publiziert |
| Offset | Gespeicherter Montage-Offset |

---

### Sicherheit

AP- und Portal-Zugangsdaten können jederzeit in der **Sicherheit**-Card geändert werden.

| Feld | Anforderung | Gespeichert in |
|------|-------------|----------------|
| AP-Name (SSID) | erforderlich | NVS `ap_ssid` |
| AP-Passwort | min. 8 Zeichen, leer = beibehalten | NVS `ap_pass` |
| Portal-Benutzer | erforderlich | NVS `portal_user` |
| Portal-Passwort | min. 6 Zeichen, leer = beibehalten | NVS `portal_pass` |

Solange das AP-Passwort dem Werksstandard (`boatopenio`) entspricht, wird oben im Portal ein **gelber Warnhinweis** angezeigt.

---

### API-Endpunkte

Alle Endpunkte sind im authentifizierten Browser-Session ohne weitere Anmeldung erreichbar.

| Endpunkt | Methode | Liefert |
|----------|---------|---------|
| `/api/values` | GET | Aktive Kanalwerte + Pitch + Roll (korrigiert) |
| `/api/raw` | GET | Rohspannungen ADS A0–A3 (Diagnose) |
| `/api/adc?ch=<1–16>` | GET | MUX auf Klemme wählen, gemittelte Spannung |
| `/api/imu` | GET | Pitch/Roll roh, korrigiert, Offset, Gyro-Bias |
| `/calibrate` | POST | Setzt aktuellen Pitch/Roll als Montage-Offset, gibt neue Offsets zurück |
| `/setinvert` | POST | Pitch-/Roll-Invertierung setzen |

> POST-Endpunkte sind zusätzlich CSRF-geschützt (Origin/Referer muss zum Gerät passen).
> Eine vollständige Backend-Beschreibung steht in [`backend_de.md`](backend_de.md).

---

## 2. config.json (LittleFS)

Auf dem Flash-Dateisystem des ESP32 gespeichert. Wird beim Booten geladen.

**Pfad auf dem Gerät:** `/config.json`  
**Lokale Quelle:** `firmware_src/data/config.json`

### Struktur

```json
{
  "kanaele": [
    {
      "klemme":  1,
      "sensor":  "batterie_starter",
      "einheit": "V",
      "faktor":  3.127,
      "offset":  0.0,
      "aktiv":   true,
      "topic":   ""
    },
    {
      "klemme":  2,
      "sensor":  "oeltemperatur",
      "einheit": "°C",
      "faktor":  1.0,
      "offset":  0.0,
      "aktiv":   true,
      "topic":   "vessels/self/propulsion/engine/oilTemperature"
    }
  ]
}
```

### Feldreferenz

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `klemme` | int 1–16 | Physische Klemmennummer auf der Platine |
| `sensor` | string | Sensorname — als MQTT-Topic-Suffix verwendet, wenn `topic` leer ist |
| `einheit` | string | Einheitenkürzel (nur Anzeige) |
| `faktor` | float | Spannungsmultiplikator: `Wert = Spannung × faktor + offset` |
| `offset` | float | Additiver Offset nach dem Faktor |
| `aktiv` | bool | `true` = Kanal wird gelesen und publiziert |
| `topic` | string | Eigener MQTT-Topic. Leer = publiziert auf `boat/io/<sensor>` |

### Spannung → Physikalischer Wert

Den **Kalibrierungs-Rechner** im Web-Portal nutzen (Zwei-Punkt, linear):
```
Pro Punkt: echten Wert eines bekannten Zustands + gemessene ADS-Spannung eintragen
→ Faktor UND Offset werden berechnet und lassen sich direkt in den Kanal übernehmen
```

Manuelle Formel aus zwei Punkten (Wert₁@V₁, Wert₂@V₂):
```
faktor = (Wert₂ − Wert₁) / (V₂ − V₁)
offset = Wert₁ − faktor · V₁
```

Für resistive VDO-Geber gibt es zusätzlich einen **Ω-Modus** (Widerstand statt Spannung
eingeben, Umrechnung über Teilerparameter). Details in [`backend_de.md`](backend_de.md) §8.

### Hochladen

1. `firmware_src/data/config.json` bearbeiten
2. Arduino IDE → **Tools → ESP32 Sketch Data Upload**

Alternativ: Konfiguration über das Web-Portal — Änderungen werden automatisch in LittleFS gespeichert.

---

## 3. MQTT

### Publizieren (ESP32 → Broker)

| Topic | Payload | Retained | Intervall |
|-------|---------|----------|-----------|
| `boat/io/<sensor>` oder eigener `topic` | Float-Wert | nein | 2 s |
| `boat/io/pitch` | Float °, korrigiert | ja | 1 s |
| `boat/io/roll` | Float °, korrigiert | ja | 1 s |
| `boatopenio/status` | `online` / `offline` | ja | 10 s + LWT |
| `boatopenio/uptime` | Sekunden seit Boot | nein | 10 s |
| `boatopenio/rssi` | WLAN-Signal dBm | nein | 10 s |
| `boatopenio/mode` | `TEST` / `LIVE` | ja | 10 s |
| `boat/alarm/impact_g` ¹ | Peak-G-Wert | ja | bei Ereignis |
| `boat/alarm/impact_severity` ¹ | `LEICHT` / `MITTEL` / `STARK` / `KRITISCH` | ja | bei Ereignis |
| `boat/alarm/impact_active` ¹ | `true` / `false` | ja | bei Ereignis |

¹ Im **TEST**-Modus lautet das Präfix `boat/test/alarm/` statt `boat/alarm/`.

### Abonnieren (Broker → ESP32)

#### Kanalkonfiguration aktualisieren

```
Topic:   boatopenio/config
Payload: vollständige oder partielle config.json-Struktur (JSON)
```

Überschreibt die aktuelle Kanalkonfiguration im Speicher und speichert in LittleFS. Partielle Updates werden unterstützt — nur im Payload vorhandene Felder werden geändert.

**Beispiel — einen Kanal mit eigenem Signal K-Topic aktivieren:**
```json
{
  "kanaele": [
    {"klemme": 3, "sensor": "oeldruck", "faktor": 1.25, "aktiv": true, "topic": "vessels/self/propulsion/engine/oilPressure"}
  ]
}
```

---

## 4. NVS (Non-Volatile Storage)

Im NVS-Partition des ESP32 unter dem Namespace `boatopenio` gespeichert. Überlebt Firmware-Updates (außer bei vollständigem Werksreset).

| Schlüssel | Typ | Standard | Beschreibung |
|-----------|-----|---------|--------------|
| `wifi_ssid` | string | `""` | WLAN-Netzwerkname |
| `wifi_pass` | string | `""` | WLAN-Passwort |
| `mqtt_ip` | string | `192.168.1.100` | MQTT-Broker-Adresse |
| `mqtt_port` | string | `1883` | MQTT-Broker-Port |
| `mqtt_user` | string | `""` | MQTT-Benutzername |
| `mqtt_pass` | string | `""` | MQTT-Passwort |
| `testmode` | bool | `true` | Test-Modus aktiv |
| `pitch_off` | float | `0.0` | IMU Pitch-Montage-Offset (°) |
| `roll_off` | float | `0.0` | IMU Roll-Montage-Offset (°) |
| `ap_ssid` | string | `BoatOpenIO-Setup` | AP-Netzwerkname |
| `ap_pass` | string | `boatopenio` | AP-Passwort |
| `portal_user` | string | `admin` | Web-Portal-Benutzername |
| `portal_pass` | string | `""` | Web-Portal-Passwort (leer = Ersteinrichtung erforderlich) |

Der NVS ist **nicht** direkt zugänglich. Alle Werte werden über das Web-Portal verwaltet oder durch einen Werksreset gelöscht.

---

## 5. Faktor- & Offset-Berechnung

Umrechnungsformel für jeden Kanal:

```
publizierter_wert = rohspannung × faktor + offset
```

### Typische Beispiele

**12V-Batterie über Spannungsteiler (R1=30kΩ, R2=10kΩ):**
```
ADS misst max. 3,127V wenn Batterie bei 12,5V
faktor = 12,5 / 3,127 ≈ 4,0
offset = 0,0
```

**VDO-Öldruckgeber (0–150 psi, 10–180Ω):**
```
Zuerst Spannungsteiler-Platine (VT) erforderlich
Tatsächliche Spannung am ADS-Pin und Realdruck mit Manometer messen
faktor = realdruck / ads_spannung
```

**NTC-Temperatursensor (linearisierter Bereich):**
```
Kalibrierungs-Hilfsrechner im Web-Portal nutzen
Zwei Messpunkte messen, Faktor und Offset aus linearer Interpolation berechnen
```

**Direktes 3,3V-Signal:**
```
faktor = 1,0
offset = 0,0
```
