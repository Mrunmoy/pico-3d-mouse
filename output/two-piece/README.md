# SpaceMouse two-piece enclosure

This revision replaces the three-piece construction in the existing **SpaceMouse - Split Shell Concept - Review** document in the SpaceMouse project. The separate original R2 master remains unchanged. `SpaceMouse_TwoPiece.f3d` is the native local backup of this revision.

## Parts and assembly

The two main enclosure parts are `Base_Tray_print.stl` and `Palm_Cap_print.stl`. The bottom, curved sidewalls and Pico cradle are a single tray. Reuse the existing small PCB retainer and four TPU strips; those are additional small printed accessories, not enclosure sections. Do not print source-history or hardware reference components.

Four integral tray columns are distributed across the joystick and palm sides, with front/rear separation. Their flat shoulders are at Z=26, 31, 21 and 21 mm and carry 0.6 mm locating collars. Matching cap counterbores seat on these shoulders. The first collar receiver has 0.25 mm radial clearance; the other receivers have 0.4 mm radial clearance to avoid an unnecessarily tight four-point fit. The collars locate the cap while the screws are started. No enclosure glue is required.

Install four heat-set inserts into the cap underside, each flush with its recessed pilot mouth 0.8 mm above its corresponding shoulder. Mount the joystick to the cap and install the Pico and retainer in the tray. Connect the wiring, lower the cap vertically onto the four locating collars, and tighten the four case screws evenly from the underside. Keep wiring off the seating shoulders. Do not use the screws to force a misaligned cap into place.

## Case hardware to order

| Quantity | Item |
|---:|---|
| 2 | M3 x 25 mm ISO 10642 socket countersunk screw, 90 degrees, 0.5 mm pitch |
| 1 | M3 x 30 mm ISO 10642 socket countersunk screw, 90 degrees |
| 1 | M3 x 35 mm ISO 10642 socket countersunk screw, 90 degrees |
| 4 (plus spares) | ruthex RX-M3Sx4.0 short M3 brass heat-set insert, 4.6 mm outside diameter, 4.0 mm length |

The CAD uses a 7.0 mm countersink mouth, 3.4 mm through clearance, and 4.0 mm insert pilot with 4.8 mm depth. Countersunk screw length includes the head. Nominal screw penetration into the insert is 3.2 mm; allowing the approximately 0.14 mm head recess gives about 3.1 mm engagement. Screw ends remain inside the insert, clear of the plastic pilot end. This case hardware is separate from the existing M2.5 joystick flange hardware and PCB clamp hardware.

Sources: [ruthex short insert](https://www.ruthex.de/en/products/ruthex-gewindeeinsatz-m3s-100stuck-rx-m3x4-0-short-messing-gewindebuchsen-fur-3d-druck), [ISO 10642 M3 head dimensions from Accu](https://www.accu.co.uk/countersunk-socket-head-screws/153243-SSK-M3-25-A2-BL). Heat-set inserts are for the thermoplastic prototype. A future resin/vendor print requires a suitable revised insert/fastening process.

## Checks and print orientation

Both enclosure bodies are single closed solids, with no Fusion feature health warnings. Assembly interference checks pass for the tray, cap, Pico, PCB retainer, joystick and ring references. The new 9 mm diameter case columns have at least 3 mm conservative clearance from the joystick reference bounding envelope. The small rear PCB overlap found during construction was relieved. The obsolete case posts and bottom fastener openings were removed from the final tray; the hand-entry opening/artifact is absent in the new exterior view. The rear USB opening remains intentional.

Both high-resolution print STLs pass manifold/watertight and positive-volume checks. They fit the A1 mini bed diagonally: tray about 159 x 155 mm, cap about 154 x 145 mm. Tray underside faces down; cap flat compound joint face faces down so the curved palm surface faces up. Existing TPU recesses and countersinks interrupt the otherwise broad tray bed contact (about 13,535 mm2).

Bambu Studio 02.08.02.61 sliced both final parts for the A1 mini 0.4 mm nozzle, eSun PLA+, textured PEI, 0.12 mm layers, 3 walls, 15% gyroid and a 5 mm outer brim. The recommended sliced projects are:

| File | Estimated total time | Filament |
|---|---:|---:|
| `PRINT_Base_Tray_A1mini.3mf` | 4 h 24 min | 74.95 g |
| `PRINT_Palm_Cap_A1mini.3mf` | 4 h 05 min | 53.55 g |

Both recommended projects use automatic tree supports and finish slicing without warnings. Support adds approximately 0.72 g to the tray and 2.93 g to the cap. Cap support-interface coordinates were checked: all lie inside the joystick cavity or fixing pockets, with none on the exterior palm surface. The cap prints curved surface upward. The no-support audits flag a tray cantilever and cap floating regions, so this design is not qualified as fully support-free. Those audit files are not the recommended print projects. See `slicer_audit.json` and `support_contacts.json` for results.

Assembly interference and separate hardware/TPU-foot interference checks pass. The native Fusion concept was saved as cloud version 3 and backed up locally. Physical fit, screw strength and bed adhesion have not yet been validated on this revision. No print job was sent.

Fastener locations (assembly XY, mm) and lengths:

| Fixing | X | Y | Shoulder Z | Screw length |
|---|---:|---:|---:|---:|
| Joystick/front | -28 | -26 | 26 | 30 |
| Joystick/rear | -23.5 | 41 | 31 | 35 |
| Palm/rear | 37 | 30 | 21 | 25 |
| Palm/front | 37 | -10 | 21 | 25 |


The source exterior remains a preserved native BRep derived from the earlier parametric loft. New fastener diameters, clearances, seating and insert depths are editable parameters/features. Altering the exterior envelope or compound joint plane requires regeneration of the derived geometry. Hidden SOURCE components retain earlier history; only the two named final enclosure components are exported.

## Views

- `Assembled.png`: smooth exterior and side seam.
- `Exploded.png`: cap and inserts lifted, case screws lowered to show assembly direction.
- `Tray_inside.png`: tray with hardware removed for clarity.
- `Cap_underside.png`: insert locations and mating shoulders.
- `Joint_section.png`: explanatory section diagram of one 21 mm palm-side screw column (not a direct Fusion section screenshot).
- `Wrist_edge.png`: hand-entry profile.
