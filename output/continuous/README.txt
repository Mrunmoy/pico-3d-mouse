SpaceMouse continuous shoulder revision - 25 October 2026

The one cloud master remains SpaceMouse Concept C - Left Hand Recessed Mount R2,
inside the SpaceMouse project. SpaceMouse_Master.f3d is its local recovery backup.

Replaced the separate raised joystick island with one full-body loft. The low
perimeter and intermediate section create the downward finger-rest transition
on the long front edge, opposite USB. Upper palm-side bulk is pulled inward;
the base footprint remains compatible with the existing bottom cover.

Measured surface heights beside the joystick at X=-44.5 mm:
Y=-42: 23.32 -> 15.94 mm
Y=-35: 30.32 -> 20.41 mm
Y=-28: 39.34 -> 27.07 mm
These are fixed CAD samples, not a physical ergonomic validation.

Editable controls: finger_edge_height, finger_mid_height, finger_mid_inset,
finger_mid_shift, rear_mid_inset, continuous_mount_dia, seat_width,
seat_rear_bias, continuous_roof. Existing joystick and ring parameters remain.
Nominal shell is 3.2 mm; sampled vertical roof thicknesses exceed 3.2 mm.
The shallow spherical cradle from the former planar roof is replaced by the
continuous loft contour. Palm centre is somewhat higher than the former dish;
the front finger-rest and outer palm end are lower.

Retained 42 mm mounting plane, measured 54 mm ring recess / 4.46 mm stack,
32.5 mm square screw pattern, top screw access, rear USB, PCB holder and wiring loop.
Fusion reports no timeline warnings and no component interference against the
existing hardware proxies. Hardware proxies do not certify full moving travel.

Upper_Shell_print.stl: revised upper only, diagonal and open rim down / palm up.
High-resolution export: 0.005 mm surface deviation, 1 degree angular deviation,
1 mm maximum edge length; 973732 triangles. Independent watertight mesh check
passed; diagonal bounds about 165.36 x 160.49 x 39.00 mm.
Bottom_Cover_print.stl and PCB_Retainer_print.stl are byte-identical to the former
ergonomic revision and need no replacement for this change.

No print was sent. No slicer project is updated or approved for this new upper.
Internal supports and bed adhesion must be reviewed before a future print.
Older 3MF projects contain older geometry. Use the revised STL when preparing one.

views/ contains shaded CAD reference PNGs, not photorealistic renders.
