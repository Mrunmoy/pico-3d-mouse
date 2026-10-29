"""Historical one-time R2 generator. Do not rerun over the authoritative
ConceptC_R2.py: that source contains subsequent validation/presentation fixes.
"""
from pathlib import Path
p=Path(__file__).parent
s=(p/'ConceptC_Complete.py').read_text()
s=s.replace("OUT=os.path.join(ROOT,'output','complete')", "OUT=os.path.join(ROOT,'output','revision2')")
s=s.replace("doc.name='SpaceMouse Concept C - Complete Prototype R1'", "doc.name='SpaceMouse Concept C - Left Hand Recessed Mount R2'")
s=s.replace("        ps.itemByName('bottom_thickness').expression='cover_thickness'", """        for name,exp,note in [
            ('ring_outer_diameter','53.46 mm','User caliper measurement'),
            ('ring_stack','4.46 mm','Measured ring with cushion seated inside'),
            ('ring_pocket_diameter','54 mm','0.27 mm radial fitting allowance'),
            ('mount_ledge','3 mm','Structural ledge under recessed ring'),
        ]:
            ps.add(name,V(exp),'mm',note)
        ps.itemByName('body_height').expression='42 mm'
        ps.itemByName('mount_reinforcement').expression='ring_stack+mount_ledge-wall'
        ps.itemByName('flange_seat').expression='palm_height-ring_stack-mount_ledge'
        ps.itemByName('pcb_x').expression='0 mm'
        ps.itemByName('pcb_y').expression='24 mm'
        ps.itemByName('bar_y').expression='pcb_y-17 mm'
        ps.itemByName('bottom_thickness').expression='cover_thickness'""")
s=s.replace("('12 mm','40 mm')", "('-22 mm','39 mm')")
s=s.replace("        stage='upper screw columns'", """        disk(upper,'palm_height+0.1 mm','joystick_x','joystick_y','ring_pocket_diameter',
             '-ring_stack-0.1 mm',CUT,'Measured flush ring and cushion pocket',upper.bRepBodies.item(0))
        stage='upper screw columns'""")
s=s.replace("        for i,(x,y) in enumerate(bosses):\n            disk(base", """        box(base,'cover_thickness','pcb_x-pcb_width/2-cradle_wall-0.5 mm',
            'pcb_y+pcb_length/2-1 mm','pcb_x+pcb_width/2+cradle_wall+0.5 mm',
            'body_depth/2+10 mm','lip_height+0.2 mm',CUT,'Rear lip relief for edge USB connector',base.bRepBodies.item(0))
        for i,(x,y) in enumerate(bosses):
            disk(base""",1)
s=s.replace("'-7 mm','4 mm','7 mm','9 mm'", "'-31 mm','-8 mm','-17 mm','-3 mm'")
s=s.replace("'-4 mm','3 mm','4 mm','10 mm'", "'-28 mm','-9 mm','-20 mm','-2 mm'")
s=s.replace("x1,'pcb_y+pcb_length/2','pcb_z-cover_thickness'", "x1,'pcb_y+pcb_length/2-2 mm','pcb_z-cover_thickness'")
s=s.replace("wx1,'pcb_y+pcb_length/2','pcb_thickness+0.5 mm'", "wx1,'pcb_y+pcb_length/2-2 mm','pcb_thickness+0.5 mm'")
rear_start=s.index('        # Two small rear corner stops')
rear_end=s.index('        for side in [-1,1]:\n            x=f\'pcb_x+({side})*bar_post_x\'',rear_start)
s=s[:rear_start]+"        # USB end stays open; front stop and screw-clamped edge shoes retain PCB.\n"+s[rear_end:]
# The shell must support a real threaded flange below the 3 mm printed ledge.
s=s.replace("disk(screws,'palm_height'", "disk(screws,'palm_height+0.2 mm'")
s=s.replace("'palm_height-8 mm'", "'palm_height+0.2 mm-12 mm'")
s=s.replace("'2.5 mm','8 mm',JOIN", "'2.5 mm','12 mm',JOIN")
s=s.replace('REF Four M2.5 x 8 flange screws','REF Four M2.5 x 12 flange screws')
s=s.replace("        stage='appearance'", """        ringocc,ringref=component('REF Measured ring and seated cushion - 53.46 x 4.46 mm')
        rs=sketch(ringref,'palm_height-ring_stack','Measured outside ring seating')
        circle(rs,'joystick_x','joystick_y','ring_outer_diameter')
        circle(rs,'joystick_x','joystick_y','fit_aperture')
        rp=next(p for p in rs.profiles if p.profileLoops.count==2)
        ri=ringref.features.extrudeFeatures.createInput(rp,NEW);ri.setDistanceExtent(False,V('ring_stack'))
        ringref.features.extrudeFeatures.add(ri).name='Flush ring stack envelope - boot motion not represented';rs.isVisible=False
        stage='mirror to user requested handedness'
        for c in (upper,base,bar,board,joy,screws,ringref):
            originals=list(c.bRepBodies)
            mi=c.features.mirrorFeatures.createInput(coll(originals),c.yZConstructionPlane)
            mi.patternComputeOption=adsk.fusion.PatternComputeOptions.IdenticalPatternCompute
            c.features.mirrorFeatures.add(mi).name='User requested reversed hand layout'
            for original in originals:c.features.removeFeatures.add(original)
        root.attributes.add('Engineering','R2','User physical feedback overrides original handedness brief; all bodies mirrored about YZ. Joystick is at negative X; palm support at positive X. USB at rear +Y.')
        stage='appearance'""")
s=s.replace("colour(screws,'Flange screws steel',(135,140,145))", "colour(screws,'Flange screws steel',(135,140,145))\n        colour(ringref,'Ring reference graphite',(35,37,40))")
s=s.replace('[upocc,baseocc,barocc,boardocc,joyocc]', '[upocc,baseocc,barocc,boardocc,joyocc,ringocc]')
s=s.replace("'cloud_saved':False}", "'cloud_saved':False,'revision':'R2 mirrored hand layout; measured flush ring recess; edge USB', 'joystick_centre_mm':[-val('joystick_x')*10,val('joystick_y')*10], 'ring_flush_depth_mm':val('ring_stack')*10, 'joystick_bottom_clearance_mm':(val('joystick_base_z')-val('cover_thickness'))*10}")
# Camera from front (-Y) makes handedness unambiguous in the saved image.
s=s.replace("(21,-30,23)","(0,-34,27)")
s=s.replace("upocc.isLightBulbOn=False;joyocc.isLightBulbOn=False", "upocc.isLightBulbOn=False;joyocc.isLightBulbOn=False;ringocc.isLightBulbOn=False;screwocc.isLightBulbOn=False")
s=s.replace("upocc.isLightBulbOn=True;joyocc.isLightBulbOn=True", "upocc.isLightBulbOn=True;joyocc.isLightBulbOn=True;ringocc.isLightBulbOn=True;screwocc.isLightBulbOn=True")
s=s.replace("(screwocc,val('exploded_gap')*2)","(screwocc,val('exploded_gap')*2),(ringocc,val('exploded_gap')*1.4)")
s=s.replace("(boardocc,1.5),(barocc,3.2)", "(boardocc,1.5),(barocc,3.2)")
(p/'ConceptC_R2.py').write_text(s)
print('Wrote explicit R2 builder',len(s),'characters')
