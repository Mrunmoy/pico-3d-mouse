ERGONOMIC UPPER REVISION - 25 OCTOBER 2026

One cloud master remains in SpaceMouse. Its existing R2 file name and identity
are retained; the ergonomic change is saved as a subsequent version of it.
SpaceMouse_Master.f3d is the local native backup, not a second cloud design.

USER DIRECTION
Lower the front long edge opposite USB for hypothenar and lower-palm contact.
Keep the physical hand layout, +25 mm support span and measured joystick mount.

PARAMETERS AND SHAPE
palm_mid_height = 28 mm, compared with the previous 42 mm flat platform.
palm_slope = 6 degrees, rising toward rear USB edge.
wrist_roll = 6 mm, rounded perimeter beneath the wrist and hypothenar.
cradle_depth = 0.8 mm; spherical radius 500 mm, approximately 56.5 mm wide.
The shallow dish is confined to the palm patch so it cannot cut the joystick
island. It provides modest location, not a guarantee against hand slipping.
palm_roof = 3.8 mm before the cradle; nominal minimum is approximately 3 mm.
Raised joystick island stays at 42 mm to retain lower mechanism clearance.
Ring pocket remains 54 mm diameter x 4.46 mm deep on the 3 mm mounting ledge.
Four confirmed mounting holes remain on the 32.5 mm square pitch.

HEIGHT CHECKS (above bottom-cover underside; add actual rubber-foot height)
At final X=29 mm, Y=-40 mm: contact surface approximately 23.79 mm.
At X=29 mm, Y=-12 mm (cradle centre): approximately 25.95 mm.
At X=29 mm, Y=+30 mm: approximately 31.15 mm.
Sampled vertical roof thickness is 3.03 mm at cradle centre.
See height_samples.json for mesh-based vertical intersections.

PART COMPATIBILITY
Upper_Shell_print.stl is the revised part. Existing matching R2 bottom cover
and PCB retainer geometry are unchanged and can be reused. Copies of their
STLs are included only for completeness. Earlier pre-R2 parts are not matched.
Added local internal clearance for the PCB/rails behind the USB mouth.
The cable tie anchor and PCB carrier remain on the unchanged bottom.

CHECKS / PRINTING
Native timeline recompute and reference-solid interference checks must pass
before this revision saves. Independently checked STL meshes are watertight.
Upper remains open-rim down / palm up, diagonally oriented at 100% mm scale.
Use supports inside and a brim; do not reuse the old upper 3MF because its
geometry is obsolete. This revision has not yet been sliced or physically
tested. No print job has been sent. Comfort, skin friction and actual hand fit
require a physical check; the slope and dish depth remain editable.

REBUILD
scripts/ConceptC_Complete/ergonomic_revision.py runs through ConceptC_FitTest
with queued.json {"action":"ergonomic"} for a temporary local trial.
Use {"action":"ergonomic","apply":true} only on the original R2 master before
this revision has been applied. Do not apply repeatedly to the revised master.
This appends new editable features and preserves earlier R2 timeline history.
