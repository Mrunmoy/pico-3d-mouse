"""Concept C exterior review. Run inside Autodesk Fusion, not standalone Python.
All model lengths use expressions; API coordinates are centimetres.
No production mounting holes or print exports until fit validation.
"""
import adsk.core, adsk.fusion, traceback, json, os, math
OUT = os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..','output'))
V = adsk.core.ValueInput.createByString
P = adsk.core.Point3D.create

def collection(items):
    c = adsk.core.ObjectCollection.create()
    for x in items: c.add(x)
    return c

def run(context):
    app = adsk.core.Application.get()
    log = []
    try:
        doc = app.documents.add(adsk.core.DocumentTypes.FusionDesignDocumentType)
        doc.name = 'SpaceMouse Concept C - Exterior Review'
        design = adsk.fusion.Design.cast(app.activeProduct)
        design.designType = adsk.fusion.DesignTypes.ParametricDesignType
        root = design.rootComponent
        params = [
            ('body_width','155 mm','Proposed hand-fit envelope'),
            ('body_depth','110 mm','Proposed hand-fit envelope'),
            ('wall','3 mm','Prototype shell thickness; stiffness not yet validated'),
            ('bottom_thickness','3 mm','Reserved future cover thickness'),
            ('lower_body_depth','25.5 mm','Drawing: underside of flange to lower body base; confirm hardware datum'),
            ('terminal_allowance','8 mm','PROVISIONAL wire/terminal clearance'),
            ('body_height','lower_body_depth + bottom_thickness + wall + terminal_allowance','PROVISIONAL mounting plane above desk; hand-fit review'),
            ('palm_height','body_height','Broad palm platform elevation; adjust after hand-fit prototype'),
            ('palm_blend_radius','10 mm','Rounded wrist/contact edge'),
            ('shoulder_drop','12 mm','Shallow shoulder transition'),
            ('crown_inset','8 mm','Shoulder roll-in per side'),
            ('joystick_x','32 mm','Right of centre; +X is user right'),
            ('joystick_y','8 mm','+Y is rear; front/wrist edge is -Y'),
            ('flange_diameter','53 mm','Drawing flange boundary'),
            ('flange_pitch_x','32.5 mm','M2.5 upper flange pattern'),
            ('flange_pitch_y','32.5 mm','M2.5 upper flange pattern'),
            ('lower_body_width','40.4 mm','Drawing lower housing width'),
            ('lower_projection_envelope','50 mm','Drawing side-view projection; other direction not verified'),
            ('joystick_overall','84 mm','Drawing overall height, NOT 94 mm'),
            ('knob_diameter','35 mm','Drawing knob diameter'),
            ('knob_section_height','39.5 mm','Drawing upper knob/neck portion'),
            ('flange_proxy_thickness','2 mm','PROVISIONAL visual reference only; measure hardware'),
            ('shaft_proxy_diameter','8 mm','PROVISIONAL visual reference only; measure hardware'),
            ('assembly_clearance','0.25 mm','Reserved mating clearance; not used for unfinished cover'),
        ]
        for name, exp, comment in params:
            design.userParameters.add(name,V(exp),'mm',comment)
        def val(e): return design.unitsManager.evaluateExpression(e,'mm')
        def component(name):
            c=root.occurrences.addNewComponent(adsk.core.Matrix3D.create()).component
            c.name=name
            return c
        def plane(c, exp, name):
            i=c.constructionPlanes.createInput(); i.setByOffset(c.xYConstructionPlane,V(exp))
            p=c.constructionPlanes.add(i); p.name=name; p.isLightBulbOn=False
            return p
        def point_dims(sk, pt, ex, ey):
            for expr,ori in [(ex,adsk.fusion.DimensionOrientations.HorizontalDimensionOrientation),(ey,adsk.fusion.DimensionOrientations.VerticalDimensionOrientation)]:
                d=sk.sketchDimensions.addDistanceDimension(sk.originPoint,pt,ori,P(pt.geometry.x+0.2,pt.geometry.y+0.2,0))
                d.parameter.expression=expr if val(expr)>0 else f'-({expr})'
        # Periodic spline: broad left support, narrower rounded right mounting end.
        # Each fit point has expression-driven X/Y dimensions; splines stay editable.
        outline=[(.49,.10),(.44,.32),(.25,.435),(-.10,.49),(-.38,.42),(-.49,.21),(-.49,-.20),(-.37,-.43),(-.08,-.49),(.25,-.43),(.44,-.27),(.49,-.07)]
        def contour(c,z,inset,name):
            sk=c.sketches.add(plane(c,z,name+' plane')); sk.name=name
            points=adsk.core.ObjectCollection.create(); expressions=[]
            for x,y in outline:
                ex=f'{abs(x)} * (body_width - 2 * ({inset}))'
                ey=f'{abs(y)} * (body_depth - 2 * ({inset}))'
                points.add(P(math.copysign(val(ex),x),math.copysign(val(ey),y),0))
                expressions.append((ex,ey))
            points.add(points.item(0))
            spline=sk.sketchCurves.sketchFittedSplines.add(points)
            spline.isClosed=True
            for n,(ex,ey) in enumerate(expressions):
                if n == len(expressions)-1:
                    last=spline.fitPoints.item(n)
                    link=sk.sketchCurves.sketchLines.addByTwoPoints(spline.fitPoints.item(0),last)
                    link.isConstruction=True
                    sk.geometricConstraints.addVertical(link)
                    d=sk.sketchDimensions.addDistanceDimension(sk.originPoint,last,adsk.fusion.DimensionOrientations.VerticalDimensionOrientation,P(last.geometry.x+0.2,last.geometry.y+0.2,0))
                    d.parameter.expression=ey
                    continue
                try: point_dims(sk,spline.fitPoints.item(n),ex,ey)
                except Exception as e:
                    raise RuntimeError(f'fit point {n}/{spline.fitPoints.count}, coords {spline.fitPoints.item(n).geometry.asArray()}, constraints {[g.objectType for g in sk.geometricConstraints]}: {e}')
            sk.isVisible=False
            return sk
        upper=component('Upper_Shell')
        s0=contour(upper,'bottom_thickness','0 mm','01 Footprint - left palm / right joystick')
        s1=contour(upper,'palm_height - shoulder_drop','1 mm','02 Rounded shoulder')
        s2=contour(upper,'palm_height','crown_inset','03 Broad palm crown')
        li=upper.features.loftFeatures.createInput(adsk.fusion.FeatureOperations.NewBodyFeatureOperation)
        for s in [s0,s1,s2]: li.loftSections.add(s.profiles.item(0))
        loft=upper.features.loftFeatures.add(li); loft.name='Concept C continuous asymmetric exterior'
        body=loft.bodies.item(0); body.name='Upper shell - ergonomic review'
        top=max([f for f in body.faces if f.geometry.surfaceType==adsk.core.SurfaceTypes.PlaneSurfaceType],key=lambda f:f.centroid.z)
        fi=upper.features.filletFeatures.createInput()
        fi.edgeSetInputs.addConstantRadiusEdgeSet(collection(list(top.edges)),V('palm_blend_radius'),True)
        fil=upper.features.filletFeatures.add(fi); fil.name='Soft wrist and palm perimeter'
        body=fil.bodies.item(0)
        bottom=min([f for f in body.faces if f.geometry.surfaceType==adsk.core.SurfaceTypes.PlaneSurfaceType],key=lambda f:f.centroid.z)
        shi=upper.features.shellFeatures.createInput(collection([bottom]),True)
        shi.insideThickness=V('wall')
        shell=upper.features.shellFeatures.add(shi); shell.name='Open underside - nominal wall'
        body=shell.bodies.item(0)
        log.append('Upper shell loft, perimeter fillet, and hollow underside created.')
        # Non-printable, simplified joystick reference. Unknown boot/sweep intentionally omitted.
        ref=component('OM300B_Envelope_REFERENCE_NOT_PRINTABLE')
        def circle_sk(c,z,dia,name):
            sk=c.sketches.add(plane(c,z,name+' datum')); sk.name=name
            ci=sk.sketchCurves.sketchCircles.addByCenterRadius(P(val('joystick_x'),val('joystick_y'),0),val(dia)/2)
            point_dims(sk,ci.centerSketchPoint,'joystick_x','joystick_y')
            sk.sketchDimensions.addDiameterDimension(ci,P(val('joystick_x')+3,val('joystick_y'),0)).parameter.expression=dia
            return sk
        def cyl(z,dia,height,name):
            sk=circle_sk(ref,z,dia,name+' profile')
            f=ref.features.extrudeFeatures.addSimple(sk.profiles.item(0),V(height),adsk.fusion.FeatureOperations.NewBodyFeatureOperation)
            f.name=name; f.bodies.item(0).name=name; sk.isVisible=False
            return f.bodies.item(0)
        flange=cyl('body_height','flange_diameter','flange_proxy_thickness','Flange diameter reference - thickness PROVISIONAL')
        knob=cyl('body_height + joystick_overall - lower_body_depth - knob_section_height','knob_diameter','knob_section_height','Knob upper assembly envelope - simplified')
        shaft=cyl('body_height + flange_proxy_thickness','shaft_proxy_diameter','joystick_overall - lower_body_depth - knob_section_height - flange_proxy_thickness','Shaft visual reference - diameter PROVISIONAL')
        datum=circle_sk(ref,'body_height','flange_diameter','M2.5 flange interface - construction only')
        datum.sketchCurves.sketchCircles.item(0).isConstruction=True
        for sx in [-1,1]:
            for sy in [-1,1]:
                ex=f'joystick_x + ({sx}) * flange_pitch_x / 2'
                ey=f'joystick_y + ({sy}) * flange_pitch_y / 2'
                p=datum.sketchPoints.add(P(val(ex),val(ey),0)); point_dims(datum,p,ex,ey)
        datum.isVisible=False
        root.attributes.add('Engineering','Status','EXTERIOR REVIEW ONLY. Mounting opening, boot sweep, electronics, cover and fasteners not engineered.')
        design.computeAll()
        checks=[]
        for i in range(design.timeline.count):
            ent=design.timeline.item(i).entity
            if hasattr(ent,'healthState') and ent.healthState != adsk.fusion.FeatureHealthStates.HealthyFeatureHealthState:
                checks.append({'name':ent.name,'message':ent.errorOrWarningMessage})
        bb=body.boundingBox
        report={'status':'exterior_review_only','shell_bbox_mm':[(bb.maxPoint.x-bb.minPoint.x)*10,(bb.maxPoint.y-bb.minPoint.y)*10,(bb.maxPoint.z-bb.minPoint.z)*10], 'timeline_count':design.timeline.count,'feature_warnings':checks,'parameters':{p.name:p.expression for p in design.userParameters},'notes':log}
        app.activeViewport.camera
        cam=app.activeViewport.camera
        cam.isSmoothTransition=False
        cam.eye=P(18,-26,24); cam.target=P(0,0,4); cam.upVector=adsk.core.Vector3D.create(0,0,1)
        app.activeViewport.camera=cam; app.activeViewport.fit(); app.activeViewport.refresh()
        os.makedirs(OUT,exist_ok=True)
        app.activeViewport.saveAsImageFile(os.path.join(OUT,'ConceptC_exterior.png'),1600,1100)
        archive=design.exportManager.createFusionArchiveExportOptions(os.path.join(OUT,'SpaceMouse_ConceptC_Exterior.f3d'))
        report['archive_saved']=design.exportManager.execute(archive)
        with open(os.path.join(OUT,'build_report.json'),'w') as f: json.dump(report,f,indent=2)
        validate_existing(app)
    except:
        os.makedirs(OUT,exist_ok=True)
        with open(os.path.join(OUT,'build_error.txt'),'w') as f: f.write(traceback.format_exc())
        app.userInterface.messageBox('Concept C build stopped. See output/build_error.txt\n'+traceback.format_exc())

def validate_existing(app):
    design=adsk.fusion.Design.cast(app.activeProduct)
    root=design.rootComponent
    upper=root.occurrences.item(0).component
    ref=root.occurrences.item(1).component
    for name,exp in [('flange_proxy_thickness','2 mm'),('shaft_proxy_diameter','8 mm')]:
        if not design.userParameters.itemByName(name): design.userParameters.add(name,V(exp),'mm','PROVISIONAL visual reference only; measure hardware')
    ref.features.extrudeFeatures.item(0).extentOne.distance.expression='flange_proxy_thickness'
    ref.features.extrudeFeatures.item(2).extentOne.distance.expression='joystick_overall - lower_body_depth - knob_section_height - flange_proxy_thickness'
    for p in design.allParameters:
        if p.expression=='body_height + 2 mm': p.expression='body_height + flange_proxy_thickness'
    ref.sketches.item(2).sketchDimensions.item(2).parameter.expression='shaft_proxy_diameter'
    old_ref=ref.sketches.itemByName('Drawing datums - 84 overall / 25.5 seating / 32.3 upper plate')
    if old_ref: old_ref.deleteMe()
    if True:
        sk=ref.sketches.add(ref.xZConstructionPlane)
        sk.name='Drawing datums - 84 overall / 25.5 seating / 32.3 upper plate'
        def value(e): return design.unitsManager.evaluateExpression(e,'mm')
        pts=[]
        for zexpr in ['0 mm','lower_body_depth','32.3 mm','joystick_overall']:
            pt=sk.sketchPoints.add(P(0,value(zexpr),0))
            if value(zexpr)>0: pt.isFixed=True
            pts.append(pt)
        for a,b,expr in [(0,1,'lower_body_depth'),(0,2,'32.3 mm'),(0,3,'joystick_overall')]:
            sk.sketchDimensions.addDistanceDimension(pts[a],pts[b],adsk.fusion.DimensionOrientations.VerticalDimensionOrientation,P(1+b*0.3,3,0),False)
        sk.isVisible=False
    appearance_notes=[]
    for lib in app.materialLibraries:
        candidates=[a for a in lib.appearances if 'plastic' in a.name.lower() and 'black' in a.name.lower()]
        source=next((a for a in candidates if 'matte' in a.name.lower()),candidates[0] if candidates else None)
        if source:
            for comp,color,name in [(upper,(55,59,64),'Concept C charcoal'),(ref,(24,26,28),'Reference black')]:
                ap=next((a for a in design.appearances if a.name==name),None) or design.appearances.addByCopy(source,name)
                for key in ['opaque_albedo','generic_diffuse']:
                    prop=next((adsk.core.ColorProperty.cast(p) for p in ap.appearanceProperties if p.id==key),None)
                    if prop: prop.value=adsk.core.Color.create(*color,255)
                for b in comp.bRepBodies: b.appearance=ap
            appearance_notes.append(lib.name)
            break
    def health():
        result=[]
        for t in design.timeline:
            e=t.entity
            if hasattr(e,'healthState') and e.healthState != adsk.fusion.FeatureHealthStates.HealthyFeatureHealthState:
                result.append({'name':e.name,'message':e.errorOrWarningMessage})
        return result
    def info():
        b=upper.bRepBodies.item(0); bb=b.boundingBox
        return {'bbox_mm':[(bb.maxPoint.x-bb.minPoint.x)*10,(bb.maxPoint.y-bb.minPoint.y)*10,(bb.maxPoint.z-bb.minPoint.z)*10],
                'volume_cm3':b.volume,'solid':b.isSolid,'warnings':health(),
                'profiles':[{'name':s.name,'fully_constrained':s.isFullyConstrained,'dimension_count':s.sketchDimensions.count,'points_mm':[[v*10 for v in list(p.geometry.asArray())[:2]] for p in s.sketchCurves.sketchFittedSplines.item(0).fitPoints]} for s in upper.sketches]}
    result={'appearance_source':appearance_notes,'baseline':info(),'tests':[]}
    changes=[('body_width','160 mm'),('body_depth','115 mm'),('palm_height','42 mm'),('joystick_x','34 mm'),('palm_blend_radius','12 mm')]
    for name,expr in changes:
        p=design.userParameters.itemByName(name); old=p.expression
        try:
            p.expression=expr; design.computeAll()
            result['tests'].append({'parameter':name,'expression':expr,'result':info()})
        finally:
            p.expression=old; design.computeAll()
    result['restored']=info()
    def camera(eye,up,target=(0,0,3)):
        cam=app.activeViewport.camera; cam.isSmoothTransition=False
        cam.eye=P(*eye); cam.target=P(*target); cam.upVector=adsk.core.Vector3D.create(*up)
        app.activeViewport.camera=cam; app.activeViewport.fit(); app.activeViewport.refresh()
        adsk.doEvents()
    camera((0,0,35),(0,1,0))
    app.activeViewport.saveAsImageFile(os.path.join(OUT,'ConceptC_top.png'),1400,1100)
    camera((18,-26,-24),(0,0,1))
    app.activeViewport.saveAsImageFile(os.path.join(OUT,'ConceptC_underside.png'),1400,1100)
    camera((18,-26,24),(0,0,1))
    app.activeViewport.saveAsImageFile(os.path.join(OUT,'ConceptC_exterior.png'),1600,1100)
    result['archive_saved']=design.exportManager.execute(design.exportManager.createFusionArchiveExportOptions(os.path.join(OUT,'SpaceMouse_ConceptC_Exterior.f3d')))
    with open(os.path.join(OUT,'validation_report.json'),'w') as f: json.dump(result,f,indent=2)
