# MotorCombo THT — Kombi-Mini-Platine für Motorsensoren

Bedrahtete (THT) Variante der MotorCombo-Platine. Fasst die sechs typischen Motorraum-Signale auf einer Platine zusammen, statt sechs einzelne Mini-Platinen zu stecken.

Alle Bauteile sind bedrahtet — von Hand lötbar, ohne SMD-Ausrüstung. Für die SMD-Variante siehe [`../MotorCombo/`](../MotorCombo/).

---

## Kanäle

| Kanal | Stecker | Signalaufbereitung | Typischer Sensor |
|-------|---------|--------------------|------------------|
| Batterie 1 | `Batt1` | Spannungsteiler (R1 / R2) | Bordspannung 12 V |
| Batterie 2 | `Batt2` | Spannungsteiler (R3 / R4) | Zweite Batteriebank |
| Tank | `Tank1` | Pull-up nach 3.3 V (R5) | VDO-Tankgeber, resistiv |
| Temperatur | `Temp1` | Pull-up nach 3.3 V (R6) | VDO-Temperaturgeber, resistiv |
| Öldruck | `Oil1` | Pull-up nach 3.3 V (R7) | VDO-Druckgeber, resistiv |
| Drehzahl | `RPM1` | Optokoppler PC817 + ESP-01 | W-Klemme Lichtmaschine, Impulsgeber |

Jeder Kanal hat einen 100 nF Kondensator zur Signalglättung (C1–C6).

---

## Signalführung

Die JST-3-Pin-Stecker sind laut Netzliste als **Eingang / GND / Ausgang** belegt: Das Rohsignal kommt auf Pin 1 an, das aufbereitete Signal geht auf Pin 3 zurück zum Board.

```
Pin 1 (roh) → Aufbereitung → Pin 3 (aufbereitet)
Pin 2 = GND
```

**Spannungsteiler (Batt1, Batt2)**
```
Pin 1 ──[R1]──┬── Pin 3
              ├── C1 ── GND
              └──[R2]── GND
```

**Pull-up (Tank, Temp, Öl)**
```
3.3 V ──[R5]──┬── Pin 1 = Pin 3
              └── C3 ── GND
```
Pin 1 und Pin 3 sind durchverbunden — der resistive Geber bildet zusammen mit dem Pull-up-Widerstand den Spannungsteiler.

**Drehzahl (RPM1)**
```
Pin 1 ──[R8]── PC817 LED ── GND
               PC817 Transistor ── GPIO0 (ESP-01), Pull-up R9
               GPIO2 ──[R10]──┬── Pin 3
                              └── C6 ── GND
```
Der Optokoppler trennt den Impulseingang galvanisch. Der ESP-01 zählt die Impulse und gibt auf Pin 3 einen drehzahlproportionalen Wert aus.

---

## Anschlüsse

| Bezeichner | Typ | Funktion |
|------------|-----|----------|
| `Batt1`, `Batt2`, `Tank1`, `Temp1`, `Oil1`, `RPM1` | JST PH 3-Pin, 2.0 mm | Kanäle zum Mainboard |
| `J7` | JST PH 2-Pin, 2.0 mm | Versorgung 3.3 V / GND vom VCC-Board |
| `GND_Collector1` | Schraubklemme 5-polig, 5.08 mm | Sammelpunkt für Sensormassen |

---

## Platine

| | |
|---|---|
| Abmessungen | 75 × 55 mm |
| Lagen | 2 |
| Bauform | durchgehend THT |
| Gerber | [`gerber/`](gerber/) — bestellfertig, inkl. ZIP |

---

## Stückliste

| Pos. | Bauteil | Anzahl | Gehäuse |
|------|---------|--------|---------|
| 1 | Widerstand, axial | 10 | DIN0204, RM 7.62 mm |
| 2 | Kondensator 100 nF, keramisch | 6 | Scheibe D5 mm, RM 5.00 mm |
| 3 | PC817 Optokoppler | 1 | DIP-4 |
| 4 | ESP-01 (ESP8266) | 1 | ESP-01 Modul, 2×4 Buchsenleiste |
| 5 | JST PH Stiftleiste 3-Pin, vertikal | 6 | B3B-PH-K |
| 6 | JST PH Stiftleiste 2-Pin, vertikal | 1 | B2B-PH-K |
| 7 | Schraubklemme 5-polig | 1 | Phoenix MKDS-3/5-5.08 |

---

## Widerstandswerte

**Die Werte R1–R10 sind im Schaltplan bewusst offen gelassen.** VDO-Geber unterscheiden sich je nach Baujahr und Hersteller im Widerstandsbereich — ein Wert, der an einem Motor sauber funktioniert, kann am nächsten zu Verfälschungen führen. Statt eine Größe vorzugeben, bestückt jeder passend zu seinen eigenen Gebern.

Die folgenden Werte haben sich in der Praxis bewährt und sind ein guter Startpunkt:

| Kanal | Widerstände | Empfehlung | Alternative |
|-------|-------------|------------|-------------|
| Batterie 1 | R1 / R2 | 10 kΩ / 2.2 kΩ | 100 kΩ / 22 kΩ |
| Batterie 2 | R3 / R4 | 10 kΩ / 2.2 kΩ | 100 kΩ / 22 kΩ |
| Tank | R5 | 10 kΩ | je nach Geber anpassen |
| Temperatur | R6 | 10 kΩ | je nach Geber anpassen |
| Öldruck | R7 | 10 kΩ | je nach Geber anpassen |

**Spannungsteiler.** 10 kΩ / 2.2 kΩ ist getestet und funktioniert. Das Teilerverhältnis beträgt 2.2 / (10 + 2.2) = 0.180, aus 12 V werden also rund 2.16 V. Als Faktor in der Kanalkonfiguration ergibt das etwa **5.55**, die Obergrenze liegt bei 3.3 V Eingangsspannung entsprechend bei rund 18 V. Die Variante 100 kΩ / 22 kΩ hat dasselbe Verhältnis, belastet die Batterie aber zehnmal weniger — dafür ist sie empfindlicher gegen Einstreuungen.

**Pull-ups.** 10 kΩ hat sich in Tests als guter Kompromiss gezeigt. Nutzt ein Geber einen anderen Widerstandsbereich, sollte der Pull-up mit angepasst werden — am besten am eingebauten Geber durchmessen und den Wert so wählen, dass der Messbereich gut ausgenutzt wird.

**Optokoppler.** R8 bestimmt den LED-Strom des PC817, R9 ist der Pull-up am ESP-01. Diese Kombination ist noch nicht in der Praxis erprobt (siehe unten).

---

## ESP-01 Hinweise

Der ESP-01 wird **gesockelt** — er braucht zum Flashen ohnehin einen speziellen USB-Adapter und wird deshalb zum Programmieren aus der Platine genommen. `RST`, `URXD` und `UTXD` sind aus diesem Grund nicht beschaltet.

> 🚧 **Der Drehzahlkanal über PC817 und ESP-01 ist noch nicht in der Praxis getestet.** Die übrigen fünf Kanäle sind erprobt.

Beim Aufbau zu beachten: `GPIO0` und `GPIO2` legen beim ESP8266 gleichzeitig den Bootmodus fest, und beide sind hier beschaltet.

- **GPIO0** hängt am Optokoppler-Transistor. Liegt beim Einschalten gerade ein Impuls an, zieht der Optokoppler GPIO0 auf Masse und der ESP-01 startet in den Flash-Modus statt in die Firmware. Bei laufendem Motor also durchaus möglich.
- **GPIO2** treibt über R10 den Ausgang. C6 ist beim Einschalten entladen — je nach Wahl von R10 und C6 kann GPIO2 dadurch beim Booten zu lange auf Masse liegen.

Beides hängt an den Werten von R8, R9, R10 und C6 und lässt sich beim Erproben des Drehzahlkanals gemeinsam festlegen.

---

## Sicherheit

> ⚠️ **Vor dem Einschalten prüfen, welcher Kanal auf welcher Klemme liegt.**
>
> Die Batteriekanäle sind für 12 V über Spannungsteiler ausgelegt. Kommen 12 V versehentlich auf einen der Pull-up-Kanäle (Tank, Temperatur, Öl), zerstört das MUX, ADS1115 und ESP32 über eine I2C-Kettenreaktion. Siehe Sicherheitshinweis in der [Haupt-README](../../../README_de.md).

---

## Lizenz

GPL-3.0, wie das übrige Repo. Siehe [LICENSE](../../../LICENSE).
