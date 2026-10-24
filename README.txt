SpaceMouse Concept C - local Fusion 360 engineering project

CURRENT REVISION: R2, output/revision2 (build validation completed).
Single cloud master: SpaceMouse project / SpaceMouse Concept C - Left Hand
Recessed Mount R2. Verified saved version 1; extra Fusion tabs closed.
Local master backup: output/revision2/ConceptC_Complete.f3d.
R1 is retained in output/complete for history; do not mix R1 and R2 print parts.
User physical feedback on 25 October 2026 supersedes the original brief's
handedness and joystick mounting arrangement.

R2 changes: mirror complete hand layout, measured 53.46 mm ring/cushion in a
54.0 mm x 4.46 mm top recess, 3 mm supporting ledge, underneath joystick with
top-access screws, Pico connector near rear wall, smooth STL export retained.
42 mm enclosure height preserves lower mechanism clearance with recessed ring.

Source builder: scripts/ConceptC_Complete/ConceptC_R2.py.
Run inside Fusion; ROOT is the local project path. Registered ConceptC_FitTest
can dispatch a request queued.json containing {"revision":"R2"} in
scripts/ConceptC_Complete. The R2 builder starts from the tracked native
output/palm25/ConceptC_Palm25.f3d. It does not send jobs to a printer.

Model units: mm in expressions, cm in API coordinates. Mirroring is a parametric
feature at the end of each component history. Pre-mirror dimensions remain in
their original coordinate system; the final joystick is on negative X and palm
support on positive X, with the cable opening on +Y. Check the front-view PNG.

Physical full-tilt boot sweep and actual threaded flange engagement remain to be
checked with hardware. Ring dimensions are measured; flexible boot shape and
connector body are conservative reference representations.
