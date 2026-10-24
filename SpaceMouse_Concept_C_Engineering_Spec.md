# DIY SpaceMouse — Concept C Engineering Specification

**Revision:** 0.1 (design brief / CAD handoff)  
**Date:** 2026-10-24  
**Target CAD:** Autodesk Fusion 360, scripted/parametric model  
**Printer:** Bambu Lab A1 Mini (180 × 180 × 180 mm build volume)  
**Primary material:** PLA (eSun PLA available); PETG optional after fit validation  
**Status:** Not yet mechanically validated or print-tested

## 1. Project objective

Design a compact, premium-looking, **left-hand-operated DIY CAD navigation controller** (informally “SpaceMouse”) around an existing **OMTERELEC OM300B two-axis analog joystick**, with a **Raspberry Pi Pico** inside. Deliver a **genuinely FDM-printable, editable Fusion 360 mechanical assembly**, not merely a visual concept or rectangular electronics box.

The user selected **Concept C — Compact Professional** as the preferred design direction: restrained, rounded, compact, and mechanically simple. Concepts A/B were considered potentially too prescriptive for the user's hand. **No Cherry MX switches or other buttons in V1.** Do not reserve conspicuous button openings.

> Functional limitation: the OM300B is a **2-axis joystick**, not a native 6-DOF SpaceMouse. Firmware/software mappings for orbit/pan/zoom may be added later; do not describe the mechanism as six independent motion axes.

## 2. Reference material / precedence

1. **`om300b(1).pdf`** — authoritative mechanical drawing of OM300B (one image-based page). Use the drawing directly; independently verify dimensions and which holes belong to which mounting plane.
2. **Actual hardware photographs** — user's left hand gripping the joystick, supplied in conversation (`IMG_8334.jpeg`, `IMG_8335.jpeg`, if copied locally). These establish natural hand placement and joystick orientation.
3. **`Left-Hand Space Mouse Design Board(1).png`** — appearance / concept reference only. It contains AI-rendered views and **unverified dimensions / internal details**. Never trace its internal construction as authoritative.
4. This specification — design intent and provisional engineering constraints.

**When sources conflict:** actual hardware measurement > manufacturer drawing > this brief's provisional dimensions > AI render.

## 3. Verified or source-visible OM300B information

The manufacturer's single-page mechanical drawing visibly labels the following (mm):

| Feature | Drawing label | Notes / CAD handling |
|---|---:|---|
| Upper circular mounting region / flange | Ø53 | Verify whether this is the outermost flange boundary in the specific view. |
| Upper flange fastener specification | M2.5 | Hole locations appear on a 32.5 × 32.5 pattern in the flange view; confirm from drawing before generating final bores. |
| Flange hole spacing | 32.5 | Shown horizontally and vertically in flange view; verify hole-centre interpretation. |
| Lower mechanism body width | 50 | Shown in lower side-view drawing; account for side projections. |
| Lower housing top-view dimension | 40.4 | Also appears on upper mechanical view; do not confuse with flange-hole pitch. |
| Lower body mounting / feature callout | M4 | Different mounting region from the M2.5 flange; do not mix these patterns. |
| Knob diameter | Ø35 | Drawing label; check the actual knob against this before ergonomic modelling. |
| Knob / upper assembly height label | 39.5 | Refers to a portion of the drawing, **not necessarily the full height above the desk**. |
| Overall vertical dimension label | 94 | Reference drawing shows this dimension; establish the reference planes before using it as a CAD constraint. |
| Additional body heights | 25.5, 32.3 | Drawing callouts; resolve their datum/reference faces in the CAD sketch. |
| Other circular callout | Ø16.5 | Do not assume this is the required panel cutout without interpreting the drawing. |

**Important:** Some dimensions were previously paraphrased imprecisely in chat. The actual drawing and physical joystick must control the CAD model. The drawing does **not** by itself prove the desired enclosure height or the clearance envelope at full joystick deflection.

## 4. Ergonomics: highest-priority design intent

- **Left-hand use only.** In top view with the front/wrist edge closest to the user, put the **joystick on the RIGHT** and the hand-support surface extending to its LEFT.
- The user grips the **existing cylindrical joystick knob** with the left hand, as shown in the photographs. Keep original knob, shaft and flexible rubber boot visible and functional.
- **The palm rests on the enclosure, not the desktop.** This is a firm requirement: if the palm is on the desk, the joystick feels too high.
- Create a **broad, gently raised palm support** left of the joystick, blending smoothly into the circular mounting zone. Avoid a steep, highly contoured palm hump that locks the hand into an assumed posture.
- Smooth rounded front/wrist edge, no hard edge under the palm. The support must be rigid enough not to noticeably flex under resting hand pressure.
- Position knob and support as one ergonomic system; adjust their relative heights by measuring / test-printing, **not by assuming the AI render is anatomically accurate**.
- Full joystick tilt must not collide with the enclosure or palm support; check swept envelope at maximum travel.
- Stable against joystick manipulation; broad footprint and rubber feet or equivalent anti-slip treatment.

## 5. Visual design — Concept C

- Compact Professional aesthetic: matte black/charcoal, quiet and functional, rounded perimeter, shallow organic curves, asymmetrical silhouette.
- Right-side joystick mount visually integrated into the body; left-side palm platform smoothly flowing into it.
- No square project-box walls, no sharp edges, no exposed rectangular Pico cutouts on the top.
- Avoid decorative seams, fake vents, lights, logos, buttons or other complexity.
- Use the concept board **for proportions and silhouette only**, not for hole coordinates or claimed manufacturing feasibility.

### Proposed starting envelope (PARAMETERS, not confirmed dimensions)

| Parameter | Initial value | Status |
|---|---:|---|
| Overall width | 155 mm | Proposed; refine for hand fit |
| Overall depth | 110 mm | Proposed; refine for hand fit |
| Overall enclosure height | **TBD from hardware** | Do not hard-code the concept render's 46 mm |
| Nominal wall thickness | 3.0 mm | Proposed FDM starting point |
| Top/structural mounting thickness | 3–5 mm locally | Proposed; verify stiffness |
| Mating clearance | 0.2–0.3 mm per side | Starting test value, tune to printer |
| Joystick X/Y centre | Parametric, right-biased | Set after checking flange-to-edge and palm clearance |
| Palm support height/contour | Parametric, **TBD** | Must be checked with hand-on-prototype test |

All components must individually fit inside **180 × 180 × 180 mm**.

## 6. Mechanical architecture

**Prefer two principal printed parts** (avoid tiny add-on trim pieces):

1. **Sculpted upper shell:** ergonomic exterior; structurally reinforced, precisely located joystick flange interface; underside supports for the joystick and Pico; integrated assembly screw locations.
2. **Removable bottom cover:** flat/stable underside, accessible fasteners, provision for adhesive anti-slip feet, no trapped hardware.

The joystick must be mechanically retained using the **correct OM300B flange fastener pattern**, with sufficient thickness and local support. Its entire lower housing, potentiometers and side projections must fit inside the shell. The joystick must be installable/removable using ordinary tools after printing.

Mount the Raspberry Pi Pico on internal standoffs or an accessible carrier area. **Pico PCB variant and exact USB connector position must be confirmed before finalising standoffs and rear cable opening.** Allow plug body and bend radius; don't assume USB-C, because many Pico boards use micro-USB.

Prefer heat-set inserts or mechanically sensible screw retention, with screw sizes/insert holes derived from the actual chosen hardware. Do not invent a final boss diameter without selecting fasteners.

## 7. FDM design-for-manufacture requirements

- Printer: Bambu A1 Mini; 0.4 mm nozzle assumed.
- PLA prototype first; use PETG only if needed.
- Prioritise low-support or support-free parts. **Determine print orientation in the slicer; do not blindly assume that printing the upper shell upside-down is optimal.** Organic external surfaces, top apertures and internal bosses may require a deliberate trade-off.
- Avoid large unsupported ceilings, floating ribs and inaccessible support pockets. Use self-supporting slopes where possible; **45° from vertical** is a conservative starting guideline, not an absolute universal printer limit.
- Minimise support scarring on the palm-contact surface.
- Use generous external fillets, reinforced load paths around joystick fasteners, and realistic internal fillets/chamfers.
- Avoid ultra-thin lips and snap fits in V1. Prefer accessible screws and robust two-part assembly.
- Include wire clearance and a rear cable exit with strain-relief strategy.
- Verify bottom-cover seating, screw access, tool access, and print-bed contact area.
- Check actual sliced geometry for thin walls, overhangs, supports, and collisions before export is considered complete.

## 8. Fusion 360 / Codex implementation requirements

Build **editable parametric CAD**, preferably using Fusion 360's supported scripting/API workflow on the user's local machine. Do not pretend a rendered mesh is a parametric CAD model.

Suggested design components:

- `SpaceMouse_V1` (root assembly)
- `Upper_Shell`
- `Bottom_Cover`
- `OM300B_Envelope` (reference geometry / non-printable)
- `Pico_Envelope` (reference geometry / non-printable)
- Optional `Fasteners_Reference` (non-printable)

Suggested user parameters: `body_width`, `body_depth`, `body_height`, `wall`, `bottom_thickness`, `palm_height`, `palm_blend_radius`, `joystick_x`, `joystick_y`, `flange_diameter`, `flange_pitch_x`, `flange_pitch_y`, `joystick_mount_thickness`, `assembly_clearance`, `pico_x`, `pico_y`, `usb_exit_width`, `usb_exit_height`, and fastener-dependent boss parameters.

**Do not hard-code dimensions that are still unresolved.** Keep the design robust to changes in joystick location, shell height and palm contour. Prefer sketches, construction planes, profiles, lofts and fillets that remain editable.

## 9. Work plan for local Codex agent

### Phase 1 — inspect and validate

1. Inspect the manufacturer's drawing and photographs. Make a dimensioned **reference sketch** with explicit datums.
2. Identify missing measurements: joystick flange thickness, mounting screw head clearance, actual below-flange depth, side protrusion envelope, maximum tilt/sweep, Pico variant, USB plug envelope.
3. Produce a **dimension checklist** and flag blockers rather than making up precision.

### Phase 2 — ergonomic massing

4. Create a simple parametric **Concept C outer form** with joystick on the right and a **supportive, gently raised left palm surface**.
5. Create a low-effort **ergonomic fit-test print** or partial top-shell test piece. Check left-hand grip, palm support height, knob reach and tilt clearance.
6. Refine palm contour/joystick centre using feedback before internal detailing.

### Phase 3 — engineered enclosure

7. Model the verified OM300B envelope and mounting interface.
8. Hollow the upper shell; design accessible fastener bosses and reinforcement.
9. Add Pico mounting and rear USB cable exit once the board/connector is confirmed.
10. Model removable bottom cover and anti-slip foot locations.
11. Run interference/clearance checks and assess structural load paths.

### Phase 4 — manufacture and handoff

12. Save native **`.f3d`** design and the **Fusion 360 script/source** used to create it.
13. Export **STEP** for geometry exchange and **separate watertight STLs** for upper shell and bottom cover.
14. Provide a brief assembly drawing, screw/insert BOM, and print-orientation guidance.
15. Inspect each STL in Bambu Studio; record recommended layer height, walls, infill, supports and any expected trouble spots.

## 10. Acceptance criteria

- [ ] Clearly resembles **Compact Professional / Concept C**, not a rectangular electronics box.
- [ ] Right-hand-positioned joystick in top view, operated by the **left hand**.
- [ ] Left palm rests comfortably **on the enclosure** while gripping knob.
- [ ] Joystick full motion is unobstructed.
- [ ] Actual OM300B mounting geometry matches datasheet/hardware.
- [ ] Complete joystick body and Pico fit, including wires and USB connector.
- [ ] Two principal FDM-printable parts; easy disassembly.
- [ ] Fits A1 Mini build volume; realistic support strategy.
- [ ] No switches or unused button holes in V1.
- [ ] Native parametric Fusion 360 model, source script, STEP, separate STLs.
- [ ] Mechanical and ergonomic assumptions explicitly documented.

## 11. Open decisions / measurements (do not silently assume)

1. Exact joystick flange-to-base vertical clearance and real-world mounting reference plane.
2. Full joystick travel angle and knob swept envelope.
3. Best palm support elevation **relative to the user's actual grip**.
4. Precise joystick X/Y location and desired hand-support footprint.
5. Raspberry Pi Pico model, PCB dimensions, USB connector and cable orientation.
6. Screw type, insert size and assembly method.
7. Whether the final upper shell can print without supports at acceptable cosmetic quality.

## 12. Direct instruction to Codex

> Work in the local Fusion 360 environment to create **Concept C** as a robust parametric mechanical assembly. Begin by interpreting the actual OM300B drawing and listing missing hardware measurements. Prioritise a physically comfortable raised palm-support surface on the LEFT, with the joystick on the RIGHT, for the user's LEFT hand. Do not add switches. Do not generate a generic box. Use staged ergonomic prototypes and only export production-intent STLs after interference, assembly and FDM printability checks. State clearly what was verified versus assumed.
