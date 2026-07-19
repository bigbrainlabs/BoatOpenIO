# BoatOpenIO v2 — Gehäuse-Designs

Community-Designs für das BoatOpenIO v2 Platinensatz-Gehäuse.

## Wettbewerb

Wir haben einen Community-Design-Wettbewerb durchgeführt (August–September 2026). Die drei Gewinner-Designs liegen im Ordner `winner/` und sind für alle zum Nachdrucken frei verfügbar.

## Referenz-Scans

3D-Scans aller vier bestückten Platinen (bereitgestellt von Community-Mitglied Bernd) befinden sich in `reference-scans/`. Als STL verfügbar — einfach in die CAD-Software als Referenzgeometrie importieren.

## Spezifikation

Vier Boards müssen ins Gehäuse passen:

| Board | Abmessungen | Befestigungsbohrungen |
|-------|-------------|----------------------|
| Mainboard | 90×60mm | 4× M3, je 4,2mm vom Rand |
| Eingangsboard | 110×46mm | 4× M3, je 4,5mm vom Rand |
| ESP32-Adapterboard | 80×30mm | 4× M3, je 4,5mm (X) / 3,8–4,2mm (Y) vom Rand |
| VCC-Verteilerboard | 20×80mm | 2× M3, je 4,2mm vom Rand |

**Bei geschlossenem Gehäuse zugänglich:**
- 16× Schraubklemmen SIG-IN
- 16× JST 3-Pin (Mini-Platinen Signalstecker)
- 10× JST 2-Pin (VCC-Leisten)
- 1× 5V Stromeingang
- Kabeldurchführung für SIG-OUT Erweiterungsmodul (8-pol IDC Flachbandkabel)

**Weitere Anforderungen:**
- Bootsmontage berücksichtigen — vibrationssicher, Wandmontage oder Hutschiene
- Optional: Platz für aktiven Lüfter (40 oder 60mm)
- Material: 3D-Druck, ASA empfohlen

## Community-Designs

Alle eingereichten Designs befinden sich in `community/`. Nachdrucken, verbessern, teilen.

## Lizenz

Alle Designs in diesem Ordner stehen unter der **CC BY-SA 4.0** Lizenz — frei nutzbar, veränderbar und weiterzugeben mit Namensnennung.
