# BoatOpenIO v2 — Gehäuse-Designs

Community-Designs für das BoatOpenIO v2 Platinensatz-Gehäuse.

## Wettbewerb

Wir veranstalten einen Community-Design-Wettbewerb (August–September 2026). Die Gewinner-Designs landen im Ordner `winner/` und sind für alle zum Nachdrucken frei verfügbar.

## Einreichen

Designs werden über ein [GitHub-Issue eingereicht](../../issues/new?template=enclosure-submission.yml). Alternativ per Pull Request mit den Dateien direkt in `community/<dein-name>/`.

Bitte mitliefern:

- STL zum Drucken, plus wenn möglich das Quellformat (STEP, FCStd, f3d …) — nur so kann jemand dein Design weiterentwickeln
- Kurze Beschreibung: Druckeinstellungen, Material, benötigte Schrauben/Einschmelzmuttern
- Foto oder Render, wenn vorhanden

Die Vorlage dafür liegt in [`SUBMISSION_TEMPLATE.md`](SUBMISSION_TEMPLATE.md).

## Rechte & Lizenz

> **Mit dem Einreichen eines Designs stellst du es unter [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/deed.de).** Ohne diese Zustimmung können wir dein Design nicht veröffentlichen.

Was das konkret bedeutet:

- **Du behältst dein Urheberrecht.** Du erteilst lediglich eine Nutzungslizenz — du darfst dein Design weiterhin überall sonst verwenden und veröffentlichen.
- **Namensnennung (BY).** Jeder, der dein Design nutzt oder verändert, muss dich nennen. Sag uns deshalb, unter welchem Namen du genannt werden willst.
- **Weitergabe unter gleichen Bedingungen (SA).** Wer dein Design verändert und weitergibt, muss das Ergebnis wieder unter CC BY-SA 4.0 stellen. Das gilt auch für uns, falls wir ein Design zum offiziellen BoatOpenIO-Gehäuse weiterentwickeln.
- **Die Lizenz ist unwiderruflich.** Einmal veröffentlichte Kopien bleiben nutzbar, auch wenn du dein Design später aus dem Repo entfernen lässt. Bitte reiche nur ein, was du dauerhaft freigeben willst.
- **Es muss deine eigene Arbeit sein.** Achtung bei importierten Fremdteilen: Hersteller-CAD von Steckern oder Gehäusen (STEP-Downloads) darf oft nicht weiterverbreitet werden. Solche Teile bitte vor dem Einreichen entfernen oder durch eigene Nachbauten ersetzen.

Beim Einreichen bestätigst du diese Punkte einmal ausdrücklich — im Issue-Formular per Checkbox, beim Pull Request in der Beschreibung.

Der übrige Inhalt dieses Repos steht unter GPL-3.0. Die Gehäuse-Designs sind davon bewusst ausgenommen, weil CC BY-SA 4.0 die passendere Lizenz für Hardware- und CAD-Werke ist.

## Referenz-Scans

3D-Scans aller vier bestückten Platinen befinden sich in `reference_scans/`. Als STL verfügbar — einfach in die CAD-Software als Referenzgeometrie importieren.

Bereitgestellt von Community-Mitglied Bernd — Namensnennung gemäß CC BY-SA 4.0.

> **Hinweis:** Scans sind Messwerte, keine Konstruktionszeichnungen. Für Passungen und Toleranzen gelten die Maße aus der Spezifikation unten, nicht die Scan-Geometrie.

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

Alle eingereichten Designs befinden sich in `community/`. Nachdrucken, verbessern, teilen — unter Nennung des jeweiligen Urhebers.
