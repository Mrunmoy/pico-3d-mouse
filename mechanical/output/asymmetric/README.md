# Asymmetric palm support with smooth sidewalls

This revision supersedes output/rounded. The authoritative CAD remains the one SpaceMouse master in the SpaceMouse Fusion project. SpaceMouse_Master.f3d is the local editable backup.

The intermediate exterior profiles now nest from the shortened base toward the fixed ring seat. The support section has an editable 4 degree crossfall toward the hand-entry long edge opposite USB. The lower sidewall starts vertically and blends into the upper shoulder, removing the earlier bulging/waisted silhouette. The crown has an editable 2 mm fillet. The measured ring recess and screw pattern remain fixed; the surrounding loft is slightly wider for mechanism-corner clearance. This is a continuous full-body surface, with no separate joystick island.

The 12 mm nominal hand-entry reduction is retained (11.88 mm at the bottom outline). Bottom cover, PCB cradle, retainer, cable loop and USB position match the previous shortened revision. Only the upper needs replacement if the shortened bottom is already printed. Older full-depth bottoms do not match.

The cavity uses matching editable loft sections, with 3.2 mm vertical roof allowance and profile insets plus local crown material. It is not a constant-normal-thickness shell. Clearance checks use the current conservative reference envelopes. Physical ergonomic comfort still needs a hand fit check.

Validation: healthy Fusion timeline, one solid per printed part, no volumetric component interference. Independent STL checks verify watertight manifold meshes. shape_audit.json compares the previous and current exported side outlines and actual roof heights: no outward reversal at sampled 0.5 mm height steps; 4.52 mm rear/front height difference at x=60 mm, y=+/-20 mm. The six tunnels in the connected upper mesh correspond to the central opening, four mounting screws and USB opening.

Upper_Shell_print.stl is oriented open rim down, palm up, diagonal on an A1 Mini bed. Supports and bed adhesion still require slicer review; no print has been submitted. Use views/Contact_Sheet_All_Views.png to review the final shape.
