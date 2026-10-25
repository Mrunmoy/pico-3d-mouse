# Split shell review concept

The existing SpaceMouse R2 master v18 remains unchanged in the SpaceMouse project. A separate Fusion document, **SpaceMouse - Split Shell Concept - Review**, holds this concept. `SpaceMouse_Split_Concept.f3d` is its local native backup.

The smooth asymmetric exterior is retained. A compound planar joint separates the curved palm cap from the lower frame, leaving a narrow side seam and concealed cap fixings. The cap prints with its flat back on the bed and its curved palm surface upward. Use the matching concept bottom cover: its alignment lip was relieved around the grounded frame ribs. The existing TPU strips and PCB retainer are reused.

## Review images

- `Assembled.png`: assembled CAD appearance.
- `Exploded.png`: cap and joystick lifted to show construction.
- `Wrist_edge.png` and `Palm_end.png`: side profiles.

These are shaded CAD views, not predictions of printed surface finish. Some black curves on the roof are displayed CAD face boundaries.

## Validation and remaining work

All three exported part meshes pass watertight and positive-volume checks. The Fusion interference check found no interference among the checked enclosure, joystick and electronics references. No print was sent.

Bambu Studio audits used the A1 mini 0.4 mm nozzle, eSun PLA+, 0.12 mm layer height, three walls, 15% gyroid, 5 mm outer brim, and supports disabled:

| Part | Estimated total | Material | Slicer warning |
|---|---:|---:|---|
| Lower frame | 2 h 33 min | 37.22 g | Floating cantilever |
| Palm cap | 3 h 29 min | 49.83 g | Floating regions |

This is **not yet a qualified support-free design**. The local joystick relief bridges and frame cantilever need further refinement. The cap joint also needs positive registration/contact stops, final screw engagement and fit verification before release. The bottom cover audit package is prepared but was not sliced in this review. Audit 3MFs and STLs are engineering review files, not approved print jobs.

The inherited source history and new split/extrude/cut features are retained. The cap exterior is a native BRep snapshot of the parametric source envelope; its outer shape and fixed joint datum require regeneration when changed. The seam offset is editable. The build script records the initial build plus incremental clearance and grounding operations; the saved F3D is the authoritative state of this concept.

No vendor manufacturing qualification or JLCPCB submission has been performed.
