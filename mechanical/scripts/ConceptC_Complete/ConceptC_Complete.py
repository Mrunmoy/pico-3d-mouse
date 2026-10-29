"""Concept C complete prototype CAD. Execute inside Fusion 360."""
import adsk.core, adsk.fusion, os, math, json, traceback, struct

ROOT=os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..'))
OUT=os.path.join(ROOT,'output','complete')
V=adsk.core.ValueInput.createByString
P=adsk.core.Point3D.create
NEW=adsk.fusion.FeatureOperations.NewBodyFeatureOperation
JOIN=adsk.fusion.FeatureOperations.JoinFeatureOperation
CUT=adsk.fusion.FeatureOperations.CutFeatureOperation

def coll(items):
    c=adsk.core.ObjectCollection.create()
    for i in items: c.add(i)
    return c

def run(context):
    app=adsk.core.Application.get()
    os.makedirs(OUT,exist_ok=True)
    smooth_request=os.path.join(OUT,'smooth.request')
    if os.path.exists(smooth_request):
        os.replace(smooth_request,smooth_request+'.done')
        import importlib.util
        path=os.path.join(ROOT,'scripts','ConceptC_Complete','refine_upper.py')
        spec=importlib.util.spec_from_file_location('refine_upper',path)
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        module.run(context)
        return
    presentation=os.path.join(OUT,'views.request')
    if os.path.exists(presentation):
        os.replace(presentation,presentation+'.done')
        d=adsk.fusion.Design.cast(app.activeProduct);root=d.rootComponent
        def shot(filename,eye,target):
            app.userInterface.activeSelections.clear()
            cam=app.activeViewport.camera;cam.isSmoothTransition=False
            cam.eye=P(*eye);cam.target=P(*target);cam.upVector=adsk.core.Vector3D.create(0,0,1)
            app.activeViewport.camera=cam;app.activeViewport.fit();app.activeViewport.refresh();adsk.doEvents()
            app.activeViewport.saveAsImageFile(os.path.join(OUT,filename),1800,1300)
        saved=[]
        offsets={0:(0,0,4.5),2:(0,0,0),3:(-16,-4,5),4:(-16,-4,2.5),5:(0,0,10.8),6:(0,0,6.6)}
        for i,xyz in offsets.items():
            occ=root.occurrences.item(i);saved.append((occ,occ.transform))
            tr=adsk.core.Matrix3D.create();tr.translation=adsk.core.Vector3D.create(*xyz);occ.transform=tr
        shot('ConceptC_Exploded.png',(26,-38,31),(-2,0,7))
        d.exportManager.execute(d.exportManager.createFusionArchiveExportOptions(os.path.join(OUT,'ConceptC_Exploded.f3d')))
        for occ,tr in saved: occ.transform=tr
        root.occurrences.item(0).isLightBulbOn=False
        root.occurrences.item(5).isLightBulbOn=False
        root.occurrences.item(6).isLightBulbOn=False
        shot('ConceptC_Internal_Layout.png',(18,-27,30),(0,0,3))
        root.occurrences.item(0).isLightBulbOn=True
        root.occurrences.item(5).isLightBulbOn=True
        root.occurrences.item(6).isLightBulbOn=True
        shot('ConceptC_Full_Assembly.png',(21,-30,23),(0,0,3))
        # Show the reinforced underside without clutter from reference hardware.
        for i in (2,3,4,5,6):root.occurrences.item(i).isLightBulbOn=False
        shot('ConceptC_Upper_Underside.png',(20,-30,-24),(0,0,2))
        for i in (2,3,4,5,6):root.occurrences.item(i).isLightBulbOn=True
        shot('ConceptC_Full_Assembly.png',(21,-30,23),(0,0,3))
        d.exportManager.execute(d.exportManager.createFusionArchiveExportOptions(os.path.join(OUT,'ConceptC_Complete.f3d')))
        if app.activeDocument.dataFile:app.activeDocument.save('Assembled and exploded presentation views')
        for other in list(app.documents):
            if other != app.activeDocument and other.dataFile and other.name.startswith(('SpaceMouse Concept C', 'ConceptC_')):
                other.close(False)
        return
    diagnostic=os.path.join(OUT,'inspect.request')
    if os.path.exists(diagnostic):
        os.replace(diagnostic,diagnostic+'.done')
        d=adsk.fusion.Design.cast(app.activeProduct)
        ii=d.createInterferenceInput(coll([d.rootComponent.occurrences.item(0),d.rootComponent.occurrences.item(2)]))
        ii.areCoincidentFacesIncluded=False
        data=[]
        for r in d.analyzeInterference(ii):
            bb=r.interferenceBody.boundingBox
            data.append({'volume':r.interferenceBody.volume,'low':[v*10 for v in bb.minPoint.asArray()],'high':[v*10 for v in bb.maxPoint.asArray()]})
        with open(os.path.join(OUT,'interference_locations.json'),'w') as f:json.dump(data,f,indent=2)
        return
    stage='import'
    try:
        prior=app.activeDocument
        folder=prior.dataFile.parentFolder if prior and prior.dataFile else None
        if folder is None:
            for candidate in app.documents:
                if candidate.dataFile and 'Palm Support' in candidate.name:
                    folder=candidate.dataFile.parentFolder
                    break
        for candidate in list(app.documents):
            if candidate.name.startswith('SpaceMouse Concept C - Complete Prototype') and not candidate.dataFile:
                candidate.close(False)
        doc=app.importManager.importToNewDocument(app.importManager.createFusionArchiveImportOptions(
            os.path.join(ROOT,'output','palm25','ConceptC_Palm25.f3d')))
        doc.name='SpaceMouse Concept C - Complete Prototype R1'
        d=adsk.fusion.Design.cast(app.activeProduct)
        root=d.rootComponent
        upocc=root.occurrences.item(0)
        upper=upocc.component
        upper.name='01 Sculpted upper shell'
        originalref=root.occurrences.item(1)
        originalref.isLightBulbOn=False
        ps=d.userParameters
        parameters=[
            ('cover_thickness','3 mm','Removable bottom plate'),
            ('cover_gap','0.3 mm','Perimeter running clearance'),
            ('lip_height','2 mm','Bottom cover alignment lip'),
            ('lip_wall','1.8 mm','Alignment lip wall'),
            ('mount_reinforcement','2 mm','Additional material below original roof'),
            ('flange_seat','palm_height-wall-mount_reinforcement','Underside of reinforced roof; joystick flange top seats here'),
            ('flange_thickness','2 mm','PROVISIONAL: measure actual flange'),
            ('joystick_base_z','flange_seat-flange_thickness-lower_body_depth','Nominal mechanism bottom'),
            ('boss_diameter','9 mm','M3 screw columns'),
            ('cover_pilot','2.5 mm','M3 plastic thread-forming pilot; tune on printer'),
            ('cover_screw_clearance','3.3 mm','M3 bottom cover clearance'),
            ('cover_head_diameter','6.4 mm','M3 countersunk screw head allowance'),
            ('cover_head_depth','1.8 mm','90 degree countersink depth allowance'),
            ('cover_pilot_depth','10 mm','Blind plastic screw engagement space'),
            ('pcb_length','51 mm','User supplied board outline'),
            ('pcb_width','21 mm','User supplied board outline'),
            ('pcb_thickness','1.6 mm','PROVISIONAL board thickness'),
            ('pcb_clearance','0.3 mm','Sliding clearance each side'),
            ('pcb_x','-30 mm','Board under left palm'),
            ('pcb_y','17 mm','USB-C end faces rear +Y; clears curved rear wall'),
            ('pcb_z','8 mm','Board underside: 5 mm above cover for solder joints'),
            ('pcb_edge_land','0.6 mm','Support only narrow PCB edge strips; check pads'),
            ('cradle_wall','1.8 mm','PCB side fence'),
            ('bar_y','0 mm','Retaining bridge avoids USB-C end'),
            ('bar_top','18.5 mm','Raised bridge clears top soldered wiring'),
            ('bar_thickness','2.5 mm','PCB bridge section'),
            ('bar_width','5 mm','Bridge along board length'),
            ('bar_post_x','pcb_width/2+pcb_clearance+3.5 mm','Clamp screw post offset'),
            ('bar_seat','10.3 mm','Clamp shoulder clears 10.1 mm side fences'),
            ('bar_pilot','2 mm','M2.5 plastic thread pilot; tune on printer'),
            ('bar_clearance','2.8 mm','M2.5 clearance for clamp'),
            ('usb_width','14 mm','PROVISIONAL plug moulding envelope'),
            ('usb_height','10 mm','PROVISIONAL cable plug clearance'),
            ('usb_z','12 mm','Rear port centre above base datum'),
            ('foot_diameter','12.2 mm','12 mm adhesive rubber feet'),
            ('foot_recess','0.4 mm','Shallow foot locating recess'),
            ('exploded_gap','45 mm','Presentation separation'),
        ]
        for name,exp,note in parameters:
            if ps.itemByName(name): ps.itemByName(name).expression=exp
            else: ps.add(name,V(exp),'mm',note)
        ps.itemByName('bottom_thickness').expression='cover_thickness'
        def val(e): return d.unitsManager.evaluateExpression(e,'mm')
        def plane(c,z,name):
            i=c.constructionPlanes.createInput()
            i.setByOffset(c.xYConstructionPlane,V(z))
            p=c.constructionPlanes.add(i);p.name=name;p.isLightBulbOn=False
            return p
        def sketch(c,z,name):
            sk=c.sketches.add(plane(c,z,name+' datum'));sk.name=name
            return sk
        def distance(sk,pt,ex,ey):
            for e,ori in [(ex,adsk.fusion.DimensionOrientations.HorizontalDimensionOrientation),(ey,adsk.fusion.DimensionOrientations.VerticalDimensionOrientation)]:
                if abs(val(e))<1e-8: continue
                if pt.isFullyConstrained: continue
                dm=sk.sketchDimensions.addDistanceDimension(sk.originPoint,pt,ori,P(pt.geometry.x+.2,pt.geometry.y+.2,0))
                dm.parameter.expression=e if val(e)>0 else '-('+e+')'
        def circle(sk,x,y,dia):
            existing=any(abs(ci.centerSketchPoint.geometry.x-val(x))<1e-7 and abs(ci.centerSketchPoint.geometry.y-val(y))<1e-7 for ci in sk.sketchCurves.sketchCircles)
            ci=sk.sketchCurves.sketchCircles.addByCenterRadius(P(val(x),val(y),0),val(dia)/2)
            if not existing: distance(sk,ci.centerSketchPoint,x,y)
            dm=sk.sketchDimensions.addDiameterDimension(ci,P(val(x)+.3,val(y)+.3,0));dm.parameter.expression=dia
            return ci
        def rect(sk,x0,y0,x1,y1):
            lines=sk.sketchCurves.sketchLines.addTwoPointRectangle(P(val(x0),val(y0),0),P(val(x1),val(y1),0))
            # Dimensional expressions retain editability of independent PCB and port geometry.
            distance(sk,lines.item(0).startSketchPoint,x0,y0)
            dm=sk.sketchDimensions.addDistanceDimension(lines.item(0).startSketchPoint,lines.item(0).endSketchPoint,
                adsk.fusion.DimensionOrientations.HorizontalDimensionOrientation,P(val(x0),val(y0)-.3,0))
            dm.parameter.expression='('+x1+')-('+x0+')'
            dm=sk.sketchDimensions.addDistanceDimension(lines.item(1).startSketchPoint,lines.item(1).endSketchPoint,
                adsk.fusion.DimensionOrientations.VerticalDimensionOrientation,P(val(x1)+.3,val(y0),0))
            dm.parameter.expression='('+y1+')-('+y0+')'
        def extrude(c,sk,height,op,name,body=None):
            profiles=coll(list(sk.profiles))
            ei=c.features.extrudeFeatures.createInput(profiles,op)
            ei.setDistanceExtent(False,V(height))
            if body and op==CUT: ei.participantBodies=[body]
            f=c.features.extrudeFeatures.add(ei);f.name=name;sk.isVisible=False
            return f
        def disk(c,z,x,y,dia,height,op,name,body=None):
            sk=sketch(c,z,name);circle(sk,x,y,dia)
            return extrude(c,sk,height,op,name,body)
        def box(c,z,x0,y0,x1,y1,height,op,name,body=None):
            sk=sketch(c,z,name);rect(sk,x0,y0,x1,y1)
            return extrude(c,sk,height,op,name,body)
        def component(name):
            occ=root.occurrences.addNewComponent(adsk.core.Matrix3D.create());occ.component.name=name
            return occ,occ.component
        outline=[(.49,.10),(.44,.32),(.25,.435),(-.10,.49),(-.38,.42),(-.49,.21),(-.49,-.20),(-.37,-.43),(-.08,-.49),(.25,-.43),(.44,-.27),(.49,-.07)]
        def outline_curve(sk,inset):
            pts=adsk.core.ObjectCollection.create();exps=[]
            for x,y in outline:
                ex=f'{abs(x)}*(body_width-2*({inset}))';ey=f'{abs(y)}*(body_depth-2*({inset}))'
                pts.add(P(math.copysign(val(ex),x),math.copysign(val(ey),y),0));exps.append((ex,ey))
            pts.add(pts.item(0));s=sk.sketchCurves.sketchFittedSplines.add(pts);s.isClosed=True
            for n,(ex,ey) in enumerate(exps):
                if n==len(exps)-1:
                    pt=s.fitPoints.item(n)
                    link=sk.sketchCurves.sketchLines.addByTwoPoints(s.fitPoints.item(0),pt);link.isConstruction=True
                    sk.geometricConstraints.addVertical(link)
                    dm=sk.sketchDimensions.addDistanceDimension(sk.originPoint,pt,adsk.fusion.DimensionOrientations.VerticalDimensionOrientation,P(pt.geometry.x+.2,pt.geometry.y+.2,0));dm.parameter.expression=ey
                else: distance(sk,s.fitPoints.item(n),ex,ey)
            return s
        stage='joystick reinforcement'
        shell=upper.bRepBodies.item(0)
        sk=sketch(upper,'flange_seat','Reinforced OM300B seating ring')
        circle(sk,'joystick_x','joystick_y','flange_diameter+4 mm')
        circle(sk,'joystick_x','joystick_y','fit_aperture')
        ring=next(p for p in sk.profiles if p.profileLoops.count==2)
        ei=upper.features.extrudeFeatures.createInput(ring,JOIN);ei.setDistanceExtent(False,V('mount_reinforcement+0.2 mm'))
        upper.features.extrudeFeatures.add(ei).name='Two mm flange load spreader';sk.isVisible=False
        shell=upper.bRepBodies.item(0)
        for sx in [-1,1]:
            for sy in [-1,1]:
                sk=sketch(upper,'flange_seat',f'Confirmed flange clearance {sx},{sy}')
                circle(sk,f'joystick_x+({sx})*flange_pitch_x/2',f'joystick_y+({sy})*flange_pitch_y/2','fit_screw_clearance')
                extrude(upper,sk,'wall+mount_reinforcement+0.5 mm',CUT,'Continue M2.5 clearance',upper.bRepBodies.item(0))
        stage='upper screw columns'
        bosses=[('-66 mm','-20 mm'),('-60 mm','28 mm'),('74 mm','-24 mm'),('12 mm','40 mm')]
        for i,(x,y) in enumerate(bosses):
            disk(upper,'cover_thickness',x,y,'boss_diameter','palm_height-wall-cover_thickness+0.3 mm',JOIN,f'Cover screw column {i+1}')
            disk(upper,'cover_thickness',x,y,'cover_pilot','cover_pilot_depth',CUT,f'Blind M3 pilot {i+1}',upper.bRepBodies.item(0))
        stage='cover'
        baseocc,base=component('02 Bottom cover with PCB cradle')
        sk=sketch(base,'0 mm','Matching organic bottom cover perimeter')
        outline_curve(sk,'cover_gap')
        extrude(base,sk,'cover_thickness',NEW,'Bottom cover solid')
        basebody=base.bRepBodies.item(0);basebody.name='Bottom cover and cradle'
        sk=sketch(base,'cover_thickness','Inset alignment lip')
        outline_curve(sk,'wall+cover_gap+0.35 mm')
        outline_curve(sk,'wall+cover_gap+lip_wall+0.35 mm')
        lip=max(list(sk.profiles),key=lambda p:p.areaProperties().perimeter/p.areaProperties().area)
        ei=base.features.extrudeFeatures.createInput(lip,JOIN);ei.setDistanceExtent(False,V('lip_height'))
        base.features.extrudeFeatures.add(ei).name='Cover alignment rim';sk.isVisible=False
        disk(base,'cover_thickness',bosses[2][0],bosses[2][1],
            'boss_diameter+2*cover_gap','lip_height+0.2 mm',CUT,
            'Alignment lip relief around front-right screw column',base.bRepBodies.item(0))
        for i,(x,y) in enumerate(bosses):
            disk(base,'0 mm',x,y,'cover_screw_clearance','cover_thickness+lip_height+1 mm',CUT,f'M3 cover clearance {i+1}',base.bRepBodies.item(0))
            low=sketch(base,'0 mm',f'Screw {i+1} head mouth');circle(low,x,y,'cover_head_diameter')
            high=sketch(base,'cover_head_depth',f'Screw {i+1} head throat');circle(high,x,y,'cover_screw_clearance')
            li=base.features.loftFeatures.createInput(CUT)
            li.loftSections.add(low.profiles.item(0));li.loftSections.add(high.profiles.item(0))
            li.participantBodies=[base.bRepBodies.item(0)]
            base.features.loftFeatures.add(li).name=f'Recess M3 countersunk head {i+1}'
            low.isVisible=False;high.isVisible=False
        stage='cradle'
        # Long side rails touch only 0.6 mm edge strips, with 0.3 mm lateral clearance.
        for side in [-1,1]:
            if side<0:
                x0='pcb_x-pcb_width/2-pcb_clearance-cradle_wall';x1='pcb_x-pcb_width/2+pcb_edge_land'
                wx0=x0;wx1='pcb_x-pcb_width/2-pcb_clearance'
            else:
                x0='pcb_x+pcb_width/2-pcb_edge_land';x1='pcb_x+pcb_width/2+pcb_clearance+cradle_wall'
                wx0='pcb_x+pcb_width/2+pcb_clearance';wx1=x1
            box(base,'cover_thickness',x0,'pcb_y-pcb_length/2',x1,'pcb_y+pcb_length/2','pcb_z-cover_thickness',JOIN,f'PCB underside edge ledge {side}')
            box(base,'pcb_z',wx0,'pcb_y-pcb_length/2',wx1,'pcb_y+pcb_length/2','pcb_thickness+0.5 mm',JOIN,f'PCB side guide {side}')
        # Short non-USB end stop, with no tall wall over the solder pads.
        box(base,'cover_thickness','pcb_x-pcb_width/2-cradle_wall','pcb_y-pcb_length/2-pcb_clearance-cradle_wall',
            'pcb_x+pcb_width/2+cradle_wall','pcb_y-pcb_length/2-pcb_clearance','pcb_z+pcb_thickness-cover_thickness',JOIN,'PCB front stop')
        # Two small rear corner stops leave centre free for USB-C insertion.
        for side in [-1,1]:
            x='pcb_x'+('+' if side>0 else '-')+'(pcb_width/2-1 mm)'
            box(base,'cover_thickness',x+'-1 mm','pcb_y+pcb_length/2+pcb_clearance',x+'+1 mm',
                'pcb_y+pcb_length/2+pcb_clearance+1.5 mm','pcb_z+pcb_thickness-cover_thickness',JOIN,f'PCB rear corner stop {side}')
        for side in [-1,1]:
            x=f'pcb_x+({side})*bar_post_x'
            disk(base,'cover_thickness',x,'bar_y','6 mm','bar_seat-cover_thickness',JOIN,f'PCB clamp post {side}')
            disk(base,'bar_seat-5 mm',x,'bar_y','bar_pilot','5.2 mm',CUT,f'M2.5 clamp pilot {side}',base.bRepBodies.item(0))
        stage='wire strain relief'
        box(base,'cover_thickness','-7 mm','4 mm','7 mm','9 mm','5 mm',JOIN,'Joystick harness tie bridge')
        box(base,'cover_thickness+1.5 mm','-4 mm','3 mm','4 mm','10 mm','2 mm',CUT,'Cable tie tunnel',base.bRepBodies.item(0))
        stage='clamp bar'
        barocc,bar=component('03 Removable PCB retaining bridge')
        box(bar,'bar_top-bar_thickness','pcb_x-bar_post_x-3 mm','bar_y-bar_width/2','pcb_x+bar_post_x+3 mm','bar_y+bar_width/2','bar_thickness',NEW,'Raised solder clearance bridge')
        for side in [-1,1]:
            x=f'pcb_x+({side})*bar_post_x'
            box(bar,'bar_seat',x+'-3 mm','bar_y-bar_width/2',x+'+3 mm','bar_y+bar_width/2',
                'bar_top-bar_thickness-bar_seat+0.2 mm',JOIN,f'Clamp shoulder {side}')
            # Narrow pressure shoes touch board edges, outside through-hole pad centres.
            if side<0: a='pcb_x-pcb_width/2-0.1 mm';b='pcb_x-pcb_width/2+pcb_edge_land'
            else: a='pcb_x+pcb_width/2-pcb_edge_land';b='pcb_x+pcb_width/2+0.1 mm'
            box(bar,'pcb_z+pcb_thickness',a,'bar_y-bar_width/2',b,'bar_y+bar_width/2',
                'bar_top-bar_thickness-pcb_z-pcb_thickness+0.2 mm',JOIN,f'PCB edge pressure shoe {side}')
            disk(bar,'bar_seat',x,'bar_y','bar_clearance','bar_top-bar_seat+1 mm',CUT,f'M2.5 clamp clearance {side}',bar.bRepBodies.item(0))
        bar.bRepBodies.item(0).name='PCB retaining bridge'
        stage='USB opening'
        # Box cut through rear wall: includes plug body and board connector access.
        box(upper,'usb_z-usb_height/2','pcb_x-usb_width/2','pcb_y+pcb_length/2-6 mm',
            'pcb_x+usb_width/2','body_depth/2+10 mm','usb_height',CUT,'Rear USB-C plug and cable access',upper.bRepBodies.item(0))
        stage='preserve smooth exterior over screw columns'
        # Trim columns against a parametric copy of the original exterior envelope.
        # This prevents columns from creating small bumps through the rounded shoulder.
        shell=upper.bRepBodies.item(0)
        li=upper.features.loftFeatures.createInput(NEW)
        for n in (0,1,2): li.loftSections.add(upper.sketches.item(n).profiles.item(0))
        envloft=upper.features.loftFeatures.add(li);envloft.name='Exterior envelope for flush internal columns'
        env=envloft.bodies.item(0)
        top=max([f for f in env.faces if f.geometry.surfaceType==adsk.core.SurfaceTypes.PlaneSurfaceType],key=lambda f:f.centroid.z)
        fi=upper.features.filletFeatures.createInput()
        fi.edgeSetInputs.addConstantRadiusEdgeSet(coll(list(top.edges)),V('palm_blend_radius'),True)
        envfillet=upper.features.filletFeatures.add(fi);envfillet.name='Match exterior shoulder radius'
        env=envfillet.bodies.item(0)
        ci=upper.features.combineFeatures.createInput(shell,coll([env]))
        ci.operation=adsk.fusion.FeatureOperations.IntersectFeatureOperation
        ci.isKeepToolBodies=False
        upper.features.combineFeatures.add(ci).name='Keep screw columns inside smooth exterior'
        stage='feet'
        feet=[('-58 mm','-23 mm'),('-53 mm','26 mm'),('63 mm','-23 mm'),('7 mm','37 mm')]
        for i,(x,y) in enumerate(feet):
            disk(base,'0 mm',x,y,'foot_diameter','foot_recess',CUT,f'Adhesive foot recess {i+1}',base.bRepBodies.item(0))
        stage='hardware reference'
        boardocc,board=component('REF USB-C Pico 51 x 21 - do not print')
        box(board,'pcb_z','pcb_x-pcb_width/2','pcb_y-pcb_length/2','pcb_x+pcb_width/2','pcb_y+pcb_length/2','pcb_thickness',NEW,'User supplied PCB outline')
        box(board,'pcb_z+pcb_thickness','pcb_x-4.5 mm','pcb_y+pcb_length/2-6.5 mm','pcb_x+4.5 mm','pcb_y+pcb_length/2+1 mm','3.5 mm',NEW,'USB-C connector - provisional envelope')
        box(board,'pcb_z+pcb_thickness','pcb_x-4 mm','pcb_y-5 mm','pcb_x+4 mm','pcb_y+3 mm','1.5 mm',NEW,'RP2040 visual proxy')
        joyocc,joy=component('REF OM300B assembled envelope - do not print')
        disk(joy,'flange_seat-flange_thickness','joystick_x','joystick_y','flange_diameter','flange_thickness',NEW,'OM300B flange - thickness provisional')
        box(joy,'joystick_base_z','joystick_x-lower_projection_envelope/2','joystick_y-lower_projection_envelope/2',
            'joystick_x+lower_projection_envelope/2','joystick_y+lower_projection_envelope/2','lower_body_depth',JOIN,'Conservative lower body and terminal envelope')
        disk(joy,'flange_seat','joystick_x','joystick_y','shaft_proxy_diameter','20 mm',JOIN,'Shaft visual proxy')
        disk(joy,'flange_seat+17 mm','joystick_x','joystick_y','knob_diameter','knob_section_height',JOIN,'Existing knob - simplified grip envelope')
        screwocc,screws=component('REF Four M2.5 x 8 flange screws - do not print')
        for sx in [-1,1]:
            for sy in [-1,1]:
                x=f'joystick_x+({sx})*flange_pitch_x/2';y=f'joystick_y+({sy})*flange_pitch_y/2'
                disk(screws,'palm_height',x,y,'4.5 mm','1.6 mm',NEW,'M2.5 pan head visual reference')
                disk(screws,'palm_height-8 mm',x,y,'2.5 mm','8 mm',JOIN,'M2.5 screw shank reference')
        stage='appearance'
        def colour(component,name,rgb):
            source=next((a for a in d.appearances if a.name=='Concept C charcoal'),None)
            if source is None:
                for lib in app.materialLibraries:
                    source=next((a for a in lib.appearances if 'plastic' in a.name.lower()),None)
                    if source: break
            if source:
                ap=d.appearances.addByCopy(source,name)
                for key in ['opaque_albedo','generic_diffuse']:
                    prop=next((adsk.core.ColorProperty.cast(p) for p in ap.appearanceProperties if p.id==key),None)
                    if prop: prop.value=adsk.core.Color.create(*rgb,255)
                for b in component.bRepBodies: b.appearance=ap
        colour(upper,'Finished shell charcoal',(53,59,66))
        colour(base,'Bottom cover grey',(96,104,116))
        colour(bar,'Retainer accent',(190,125,45))
        colour(board,'Pico reference green',(28,112,68))
        colour(joy,'Joystick reference black',(24,27,30))
        colour(screws,'Flange screws steel',(135,140,145))
        stage='validation'
        d.computeAll()
        warnings=[]
        for item in d.timeline:
            e=item.entity
            if hasattr(e,'healthState') and e.healthState!=adsk.fusion.FeatureHealthStates.HealthyFeatureHealthState:
                warnings.append({'feature':e.name,'message':e.errorOrWarningMessage})
        printable=[('Upper_Shell',upper),('Bottom_Cover',base),('PCB_Retainer',bar)]
        for name,c in printable:
            assert c.bRepBodies.count==1,(name,c.bRepBodies.count)
            assert c.bRepBodies.item(0).isSolid,name
        if warnings: raise RuntimeError(str(warnings))
        # Inspect assembled solid overlaps. Touching intended seating faces are not interference.
        interference=[]
        try:
            ii=d.createInterferenceInput(coll([upocc,baseocc,barocc,boardocc,joyocc]))
            ii.areCoincidentFacesIncluded=False
            results=d.analyzeInterference(ii)
            for result in results:
                if result.interferenceBody and result.interferenceBody.volume>0.00001:
                    interference.append({'one':result.entityOne.name,'two':result.entityTwo.name,'volume_cm3':result.interferenceBody.volume})
        except Exception as exc: interference.append({'analysis_error':str(exc)})
        report={'status':'complete prototype CAD; unmeasured hardware clearances remain provisional',
                'parameters':{p.name:p.expression for p in ps},'warnings':warnings,'interference':interference,
                'timeline_count':d.timeline.count,'print_parts':[],'cloud_saved':False}
        stage='exports'
        exp=d.exportManager
        for name,c in printable:
            b=c.bRepBodies.item(0);bb=b.boundingBox
            stl=exp.createSTLExportOptions(b,os.path.join(OUT,name+'_assembly.stl'))
            stl.meshRefinement=adsk.fusion.MeshRefinementSettings.MeshRefinementHigh;stl.sendToPrintUtility=False
            if name=='Upper_Shell':
                stl.surfaceDeviation=0.0005
                stl.normalDeviation=math.radians(1)
                stl.maximumEdgeLength=0.1
            assert exp.execute(stl)
            report['print_parts'].append({'name':name,'solid':b.isSolid,'volume_cm3':b.volume,
                'bbox_mm':[(bb.maxPoint.x-bb.minPoint.x)*10,(bb.maxPoint.y-bb.minPoint.y)*10,(bb.maxPoint.z-bb.minPoint.z)*10]})
        def camera(filename,eye,up=(0,0,1),target=(0,0,3)):
            app.userInterface.activeSelections.clear()
            cam=app.activeViewport.camera;cam.isSmoothTransition=False
            cam.eye=P(*eye);cam.target=P(*target);cam.upVector=adsk.core.Vector3D.create(*up)
            app.activeViewport.camera=cam;app.activeViewport.fit();app.activeViewport.refresh();adsk.doEvents()
            assert app.activeViewport.saveAsImageFile(os.path.join(OUT,filename),1800,1300)
        camera('ConceptC_Full_Assembly.png',(21,-30,23))
        upocc.isLightBulbOn=False;joyocc.isLightBulbOn=False
        camera('ConceptC_Internal_Layout.png',(18,-27,30))
        upocc.isLightBulbOn=True;joyocc.isLightBulbOn=True
        assert exp.execute(exp.createSTEPExportOptions(os.path.join(OUT,'ConceptC_Complete_Assembly.step')))
        assert exp.execute(exp.createFusionArchiveExportOptions(os.path.join(OUT,'ConceptC_Complete.f3d')))
        stage='exploded view'
        saved=[]
        for occ,dz in [(upocc,val('exploded_gap')),(joyocc,val('exploded_gap')*2.4),(boardocc,1.5),(barocc,3.2),(screwocc,val('exploded_gap')*2)]:
            saved.append((occ,occ.transform))
            tr=adsk.core.Matrix3D.create();tr.translation=adsk.core.Vector3D.create(0,0,dz)
            occ.transform=tr
        camera('ConceptC_Exploded.png',(24,-34,28),target=(0,0,6))
        assert exp.execute(exp.createFusionArchiveExportOptions(os.path.join(OUT,'ConceptC_Exploded.f3d')))
        for occ,tr in saved: occ.transform=tr
        camera('ConceptC_Full_Assembly.png',(21,-30,23))
        # Re-export restored assembly and persist cloud copy beside the approved shell.
        assert exp.execute(exp.createFusionArchiveExportOptions(os.path.join(OUT,'ConceptC_Complete.f3d')))
        if folder and not interference:
            try: report['cloud_saved']=doc.saveAs(doc.name,folder,'Complete prototype assembly','Palm +25, reinforced flange, cover and solder-access PCB cradle')
            except: report['cloud_error']=traceback.format_exc()
        with open(os.path.join(OUT,'validation_report.json'),'w') as f:json.dump(report,f,indent=2)
        with open(os.path.join(OUT,'complete.ok'),'w') as f:f.write('CAD build and exports completed')
    except:
        with open(os.path.join(OUT,'build_error.txt'),'w') as f:f.write('Stage: '+stage+'\n'+traceback.format_exc())
        app.userInterface.messageBox('Complete CAD stopped at '+stage+'\n'+traceback.format_exc())
