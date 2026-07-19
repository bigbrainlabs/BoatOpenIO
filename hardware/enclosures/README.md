# BoatOpenIO v2 — Enclosure Designs

Community-designed enclosures for the BoatOpenIO v2 PCB set.

## Contest

We're running a community design contest (August–September 2026). The winning designs go into the `winner/` folder and are free for everyone to print.

## Submitting

Designs are submitted via [GitHub issue](../../issues/new?template=enclosure-submission.yml). Alternatively, open a pull request placing the files directly in `community/<your-name>/`.

Please include:

- An STL for printing, plus the source format if possible (STEP, FCStd, f3d …) — that's the only way someone else can build on your design
- A short description: print settings, material, required screws / heat-set inserts
- A photo or render, if you have one

The template for this is in [`SUBMISSION_TEMPLATE.md`](SUBMISSION_TEMPLATE.md).

## Rights & License

> **By submitting a design you license it under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/).** Without that permission we cannot publish your design.

What this means in practice:

- **You keep your copyright.** You're only granting a license — you remain free to use and publish your design anywhere else.
- **Attribution (BY).** Anyone using or modifying your design must credit you. So tell us the name you want to be credited under.
- **Share-alike (SA).** Anyone who modifies and redistributes your design must license the result under CC BY-SA 4.0 as well. That includes us, should we develop a design into the official BoatOpenIO enclosure.
- **The license is irrevocable.** Copies already published stay usable even if you later have your design removed from the repo. Only submit what you're willing to release permanently.
- **It has to be your own work.** Watch out for imported third-party parts: manufacturer CAD for connectors or housings (STEP downloads) often may not be redistributed. Please remove such parts before submitting, or replace them with your own models.

You confirm these points explicitly when submitting — via checkbox in the issue form, or in the description of your pull request.

The rest of this repo is licensed GPL-3.0. The enclosure designs are deliberately excluded from that, because CC BY-SA 4.0 is the better fit for hardware and CAD work.

## Reference Scans

3D scans of all four assembled boards are in `reference_scans/`. Available as STL — import into your CAD software as reference geometry.

Provided by community member Bernd — attribution per CC BY-SA 4.0.

> **Note:** Scans are measurements, not engineering drawings. For fits and tolerances use the dimensions in the specs below, not the scan geometry.

## Specs

Four boards need to fit inside:

| Board | Dimensions | Mounting holes |
|-------|-----------|----------------|
| Main Board | 90×60mm | 4× M3, 4.2mm from edge |
| Input Board | 110×46mm | 4× M3, 4.5mm from edge |
| ESP32 Adapter | 80×30mm | 4× M3, 4.5mm (X) / 3.8–4.2mm (Y) from edge |
| VCC Board | 20×80mm | 2× M3, 4.2mm from edge |

**Accessible with enclosure closed:**
- 16× screw terminals SIG-IN
- 16× JST 3-pin (mini board connectors)
- 10× JST 2-pin (VCC rails)
- 1× 5V power input
- Cable gland for SIG-OUT extension (8-pin IDC ribbon cable)

**Additional requirements:**
- Boat mounting — vibration resistant, wall mount or DIN rail
- Optional: space for active fan (40 or 60mm)
- Material: 3D print, ASA recommended

## Community Designs

All submitted designs are in `community/`. Print them, improve them, share them — crediting the respective author.
