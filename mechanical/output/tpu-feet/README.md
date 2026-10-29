# Curved perimeter TPU feet

This revision replaces the four round adhesive-foot recesses with four elongated, rounded strips following the bottom-cover perimeter. The upper shell and electronics mounts are unchanged. The one authoritative SpaceMouse Fusion master contains the revised bottom and four separate editable TPU components. SpaceMouse_Master.f3d is its local backup.

Each strip follows a 40 mm sampled perimeter arc. Pockets are 6 mm wide, 1 mm deep, with their centreline 5.5 mm inside the perimeter. TPU inserts have 0.2 mm side clearance, are 1.6 mm thick, and project 0.8 mm below the cover when installed. A 0.2 mm allowance above each insert is for thin adhesive. These are adhesive-retained inserts, not snap fits. The bottom retains 2 mm nominal material above the pockets.

Print one Bottom_Cover_print.stl in the enclosure material and one each of TPU_Strip_1_print.stl through TPU_Strip_4_print.stl in TPU. Each strip differs in curvature; match it to its numbered pocket/component. The STLs lie flat at Z=0. The cover is rotated diagonally for the A1 Mini. No print has been sent.

The tpu_* user parameters control inset, width, depth, clearance, adhesive gap and projection. Spline-point coordinates retain expressions for inset and width; the sampled original perimeter and 40 mm arc locations are fixed references. Large changes to the overall cover perimeter require regenerating the strip paths with the script.

Use Bottom_with_TPU.png for the installed view and Bottom_pockets.png for the empty pockets. validation_report.json records Fusion health, interference and screw-head clearance checks; mesh_checks.json records independent manifold and volume checks. Adhesive thickness and actual TPU fit still need checking on the printed parts.
