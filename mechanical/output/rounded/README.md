# Rounded, shorter hand-entry revision

Use the single SpaceMouse cloud master in the SpaceMouse project. SpaceMouse_Master.f3d is its local editable backup; the STL files are print exports, not additional CAD masters.

The long hand-entry edge opposite USB is shortened by nominal 12 mm (11.88 mm measured at the exported bottom outline). The matching bottom and two front screw positions are updated. This upper and bottom must be used together; the earlier bottom no longer matches. The PCB retainer is unchanged and can be reused.

The raised crown now has an 4 mm fillet into the surrounding curved loft, removing the previous sharp projecting lip. The low perimeter and broad ramp remain editable sketches and loft features. The internal cavity is an independent parametric loft, with nominal 3.2 mm roof allowance and section insets; this is not a constant-normal-thickness shell. The measured flush ring recess, confirmed hole pattern, left-hand layout and USB/Pico positions are retained.

Validation: healthy Fusion timeline, one solid per printed component, no volumetric interference against the current electronics/hardware reference envelopes. Exported meshes are manifold and watertight. The clearance check uses reference envelopes rather than a manufacturer detailed component solid. Physical comfort and final assembled fit still need evaluation.

Print exports: Upper_Shell_print.stl and Bottom_Cover_print.stl. The upper is open rim down, palm up; internal support and bed adhesion must be reviewed in the slicer. These files have not been sent to the printer. Do not print the top face down if a smooth hand-contact surface is wanted.

See views/ for the updated orthographic and perspective views; dimension_audit.json, validation_report.json and mesh_checks.json record the checks.
