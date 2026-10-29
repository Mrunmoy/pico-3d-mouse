"""Continuous shoulder revision. Trial locally first; apply=True updates the single master."""
import adsk.core, adsk.fusion, math, os, json, traceback
ROOT=r'D:\github\fusion360\space-mouse'
OUT=os.path.join(ROOT,'output','rounded')
V=adsk.core.ValueInput.createByString
P=adsk.core.Point3D.create
NEW=adsk.fusion.FeatureOperations.NewBodyFeatureOperation
JOIN=adsk.fusion.FeatureOperations.JoinFeatureOperation
CUT=adsk.fusion.FeatureOperations.CutFeatureOperation

def coll(items):
    c=adsk.core.ObjectCollection.create()
    for item in items:c.add(item)
    return c

def run(request):
    app=adsk.core.Application.get(); os.makedirs(OUT,exist_ok=True)
    stage='open trial'; report={}
    with open(os.path.join(OUT,'progress.json'),'w') as f:json.dump({'status':'running'},f)
    try:
        if request.get('cavity_tune'):
            doc=app.activeDocument;d=adsk.fusion.Design.cast(app.activeProduct)
            upper=d.rootComponent.occurrences.item(0).component
            sk=next(sk for sk in upper.sketches if sk.name=='Inner cavity seat')
            for dm in sk.sketchDimensions:
                expr=dm.parameter.expression
                if '-8 mm' not in expr and ('seat_width-2*continuous_roof' in expr or 'continuous_mount_dia-2*continuous_roof' in expr):
                    dm.parameter.expression=expr.replace('seat_width-2*continuous_roof','seat_width-2*continuous_roof-8 mm').replace('continuous_mount_dia-2*continuous_roof','continuous_mount_dia-2*continuous_roof-8 mm')
            d.userParameters.itemByName('crown_radius').expression='4 mm'
            finalize(doc,request,report)
            with open(os.path.join(OUT,'validation_report.json'),'w') as f:json.dump(report,f,indent=2)
            with open(os.path.join(OUT,'progress.json'),'w') as f:json.dump({'status':'complete','stage':'local crown allowance'},f)
            return
        if request.get('finish'):
            doc=app.activeDocument
            expected=json.load(open(os.path.join(ROOT,'output','revision2','master_save_record.json')))['data_file_id']
            assert doc.dataFile and doc.dataFile.id==expected
            d=adsk.fusion.Design.cast(app.activeProduct)
            ps=d.userParameters
            if not ps.itemByName('crown_radius'):ps.add('crown_radius',V('4 mm'),'mm','Smooth exterior crown transition radius')
            upper=d.rootComponent.occurrences.item(0).component
            f=max(list(upper.features.filletFeatures),key=lambda f:f.timelineObject.index)
            f.timelineObject.rollTo(True);f.edgeSets.item(0).radius.expression='crown_radius';d.timeline.moveToEnd();d.computeAll()
            vp=app.activeViewport;cam=vp.camera;cam.isSmoothTransition=False
            cam.eye=P(-25,-32,25);cam.target=P(0,0,3.5);cam.upVector=adsk.core.Vector3D.create(0,0,1)
            cam.cameraType=adsk.core.CameraTypes.OrthographicCameraType;vp.camera=cam
            vp.visualStyle=adsk.core.VisualStyles.ShadedVisualStyle;vp.fit();vp.refresh();adsk.doEvents()
            assert all(not hasattr(t.entity,'healthState') or t.entity.healthState==adsk.fusion.FeatureHealthStates.HealthyFeatureHealthState for t in d.timeline)
            if doc.isModified:assert doc.save('Rounded crown and shortened hand-entry review presentation')
            assert d.exportManager.execute(d.exportManager.createFusionArchiveExportOptions(os.path.join(OUT,'SpaceMouse_Master.f3d')))
            with open(os.path.join(OUT,'save_record.json'),'w') as f:json.dump({'master_id':doc.dataFile.id,'project':doc.dataFile.parentProject.name,'version':doc.dataFile.versionNumber,'modified':doc.isModified},f,indent=2)
            with open(os.path.join(OUT,'progress.json'),'w') as f:json.dump({'status':'complete','stage':'final save'},f)
            return
        if request.get('inspect'):
            d=adsk.fusion.Design.cast(app.activeProduct);root=d.rootComponent
            ii=d.createInterferenceInput(coll([root.occurrences.item(n) for n in (0,2,3,4,5,7)]));ii.areCoincidentFacesIncluded=False
            rows=[]
            for r in d.analyzeInterference(ii):
                if r.interferenceBody:
                    bb=r.interferenceBody.boundingBox
                    rows.append(dict(one=r.entityOne.name,two=r.entityTwo.name,low=[x*10 for x in bb.minPoint.asArray()],high=[x*10 for x in bb.maxPoint.asArray()],volume_mm3=r.interferenceBody.volume*1000))
            with open(os.path.join(OUT,'clash_locations.json'),'w') as f:json.dump(rows,f,indent=2)
            body=root.occurrences.item(0).component.bRepBodies.item(0)
            joins=[]
            for face in body.faces:
                if face.geometry.surfaceType!=adsk.core.SurfaceTypes.PlaneSurfaceType or abs(face.centroid.z-4.2)>.001:continue
                for loop in face.loops:
                    if not loop.isOuter:continue
                    for edge in loop.edges:
                        pt=edge.pointOnEdge
                        ok,a=face.evaluator.getNormalAtPoint(pt)
                        for other in edge.faces:
                            if other==face:continue
                            good,b=other.evaluator.getNormalAtPoint(pt)
                            if ok and good:joins.append(math.degrees(math.acos(max(-1,min(1,a.dotProduct(b))))))
            with open(os.path.join(OUT,'crown_angles.json'),'w') as f:json.dump(joins,f)

            cam=app.activeViewport.camera;cam.isSmoothTransition=False;cam.eye=P(0,-32,20);cam.target=P(0,0,3);cam.upVector=adsk.core.Vector3D.create(0,0,1)
            app.activeViewport.camera=cam;app.activeViewport.fit();app.activeViewport.refresh();adsk.doEvents()
            app.activeViewport.saveAsImageFile(os.path.join(OUT,'Trial.png'),1800,1300)
            return
        if request.get('apply'):
            master=json.load(open(os.path.join(ROOT,'output','revision2','master_save_record.json')))
            doc=next(x for x in app.documents if x.dataFile and x.dataFile.id==master['data_file_id'])
            if request.get('reset_master'):
                datafile=doc.dataFile;doc.close(False);doc=app.documents.open(datafile,True)
            doc.activate()
            assert doc.dataFile and doc.dataFile.id==master['data_file_id']
            if doc.isModified:doc.save('R2 preserved before ergonomic upper revision')
        else:
            for other in list(app.documents):
                if not other.dataFile and other.name.startswith('Ergonomic trial'):other.close(False)
            doc=app.importManager.importToNewDocument(app.importManager.createFusionArchiveImportOptions(
                os.path.join(ROOT,'output','continuous','SpaceMouse_Master.f3d')))
            doc.name='Ergonomic trial - rounded shorter hand entry'
        d=adsk.fusion.Design.cast(app.activeProduct); root=d.rootComponent
        root.occurrences.item(0).isLightBulbOn=True
        root.occurrences.item(2).isLightBulbOn=True
        upper=root.occurrences.item(0).component; ps=d.userParameters
        if request.get('apply'):
            assert request.get('replace') or not ps.itemByName('finger_edge_height'), 'Continuous revision already exists'
        def val(e):return d.unitsManager.evaluateExpression(e,'mm')
        for n,e,u,note in [
            ('entry_boss_y1','-10 mm','mm','First front cover screw moved inward for shorter body'),
            ('entry_boss_y3','-12 mm','mm','Third cover screw moved inward for shorter body'),
            ('front_mid_inset','0 mm','mm','Retain joystick front-corner clearance inside shortened shell'),
            ('entry_trim','12 mm','mm','Shorten long hand-entry side opposite USB in plan; rear edge fixed'),
            ('crown_radius','4 mm','mm','Smooth crown transition, preserving enclosed ring seat'),
            ('crown_tangent_weight','0.6','', 'Tangent influence at mounting plane for smooth hand contact'),
            ('finger_mid_height','20 mm','mm','Lower middle section controls the long finger-rest and palm ramp'),
            ('finger_mid_inset','14 mm','mm','Pull upper thenar side inward while retaining existing base'),
            ('rear_mid_inset','2 mm','mm','Retain high rear roof clearance for mechanism corners'),
            ('finger_mid_shift','10 mm','mm','Bias middle crown toward the joystick'),
            ('finger_edge_height','12 mm','mm','Low perimeter shoulder, before the continuous rise to mounting seat'),
            ('finger_edge_inset','2 mm','mm','Small inset at low shoulder; existing base outline retained'),
            ('continuous_mount_dia','72 mm','mm','Flush ring surrounded by continuous full-body loft, no separate island'),
            ('seat_width','82 mm','mm','Roof width for lower mechanism corners; not a separate mounting island'),
            ('seat_rear_bias','4 mm','mm','Bias high roof toward rear mechanism corner, keeping finger side lower'),
            ('continuous_roof','3.2 mm','mm','Uniform hollow shell thickness'),
        ]:
            if ps.itemByName(n):ps.itemByName(n).expression=e
            else:ps.add(n,V(e),u,note)
        stage='replace upper shell'
        old=upper.bRepBodies.item(0); appearance=old.appearance
        upper.features.removeFeatures.add(old).name='Replace flat R2 upper with ergonomic upper'
        def plane(z,name,base=None):
            i=upper.constructionPlanes.createInput();i.setByOffset(base or upper.xYConstructionPlane,V(z))
            p=upper.constructionPlanes.add(i);p.name=name;p.isLightBulbOn=False;return p
        def sketch(z,name):
            sk=upper.sketches.add(plane(z,name+' datum'));sk.name=name;return sk
        def distance(sk,pt,x,y):
            for e,ori in [(x,adsk.fusion.DimensionOrientations.HorizontalDimensionOrientation),(y,adsk.fusion.DimensionOrientations.VerticalDimensionOrientation)]:
                if abs(val(e))<1e-8 or pt.isFullyConstrained:continue
                dm=sk.sketchDimensions.addDistanceDimension(sk.originPoint,pt,ori,P(pt.geometry.x+.2,pt.geometry.y+.2,0))
                dm.parameter.expression=e if val(e)>0 else '-('+e+')'
        outline=[(.49,.10),(.44,.32),(.25,.435),(-.10,.49),(-.38,.42),(-.49,.21),(-.49,-.20),(-.37,-.43),(-.08,-.49),(.25,-.43),(.44,-.27),(.49,-.07)]
        def contour(sk,inset,seat=False):
            pts=coll([]);exps=[]
            for x,y in outline:
                ex=f'{abs(x)}*(body_width-2*({inset}))';ey=f'{abs(y)}*(body_depth-2*({inset}))'
                if seat=='mid':
                    ex=f'finger_mid_shift+({x})*(body_width-2*finger_mid_inset)';ey=f'({y})*(body_depth-2*'+('rear_mid_inset' if y>0 else 'finger_mid_inset')+')'
                    if y<0:ey=f'({y})*(body_depth-entry_trim-2*front_mid_inset)+entry_trim/2'
                    pts.add(P(val(ex),val(ey),0))
                elif seat:
                    ex=f'joystick_x+({x})*seat_width';ey=f'joystick_y+seat_rear_bias+({y})*continuous_mount_dia'
                    pts.add(P(val(ex),val(ey),0))
                else:
                    ex=f'({x})*(body_width-2*({inset}))'
                    ey=f'({y})*(body_depth-entry_trim-2*({inset}))+entry_trim/2'
                    pts.add(P(val(ex),val(ey),0))
                exps.append((ex,ey))
            pts.add(pts.item(0));sp=sk.sketchCurves.sketchFittedSplines.add(pts);sp.isClosed=True
            for n,(ex,ey) in enumerate(exps):
                if n==len(exps)-1:
                    pt=sp.fitPoints.item(n);line=sk.sketchCurves.sketchLines.addByTwoPoints(sp.fitPoints.item(0),pt);line.isConstruction=True
                    sk.geometricConstraints.addVertical(line)
                    dm=sk.sketchDimensions.addDistanceDimension(sk.originPoint,pt,adsk.fusion.DimensionOrientations.VerticalDimensionOrientation,P(pt.geometry.x+.2,pt.geometry.y+.2,0));dm.parameter.expression=ey if val(ey)>0 else '-('+ey+')'
                else:distance(sk,sp.fitPoints.item(n),ex,ey)
            return sk.profiles.item(0)
        def circle(sk,x,y,dia):
            ci=sk.sketchCurves.sketchCircles.addByCenterRadius(P(val(x),val(y),0),val(dia)/2)
            distance(sk,ci.centerSketchPoint,x,y)
            sk.sketchDimensions.addDiameterDimension(ci,P(val(x)+.3,val(y)+.3,0)).parameter.expression=dia
            return sk.profiles.item(0)
        def ext(sk,profile,height,op,name,body=None):
            ei=upper.features.extrudeFeatures.createInput(profile,op);ei.setDistanceExtent(False,V(height))
            if body and op==CUT:ei.participantBodies=[body]
            f=upper.features.extrudeFeatures.add(ei);f.name=name;sk.isVisible=False;return f
        def disk(z,x,y,dia,height,op,name,body=None):
            sk=sketch(z,name);return ext(sk,circle(sk,x,y,dia),height,op,name,body)
        def combine(a,b,op,name):
            ci=upper.features.combineFeatures.createInput(a,coll([b]));ci.operation=op;ci.isKeepToolBodies=False
            f=upper.features.combineFeatures.add(ci);f.name=name;return f.bodies.item(0)
        def loft(profiles,op,name,body=None):
            li=upper.features.loftFeatures.createInput(op)
            for p in profiles:li.loftSections.add(p)
            if body:li.participantBodies=[body]
            f=upper.features.loftFeatures.add(li);f.name=name;return f.bodies.item(0)
        def fillet(body,edges,radius,name):
            fi=upper.features.filletFeatures.createInput();fi.edgeSetInputs.addConstantRadiusEdgeSet(coll(edges),V(radius),True)
            f=upper.features.filletFeatures.add(fi);f.name=name;return f.bodies.item(0)
        stage='continuous full-body exterior'
        s0=sketch('cover_thickness','Continuous original base interface');p0=contour(s0,'0 mm')
        s1=sketch('finger_edge_height','Low finger and wrist perimeter');p1=contour(s1,'finger_edge_inset')
        s2=sketch('palm_height','Fixed flush joystick seat');p2=contour(s2,'0 mm',True)
        sm=sketch('finger_mid_height','Lower broad palm and finger ramp');pm=contour(sm,'0 mm','mid')
        li=upper.features.loftFeatures.createInput(NEW)
        for pr in (p0,p1,pm,p2):section=li.loftSections.add(pr)
        section.setFreeEndCondition()
        lf=upper.features.loftFeatures.add(li);lf.name='Tangent rounded rise to flush mounting plane';body=lf.bodies.item(0)
        for sk in (s0,s1,sm,s2):sk.isVisible=False
        top=max([f for f in body.faces if f.geometry.surfaceType==adsk.core.SurfaceTypes.PlaneSurfaceType],key=lambda f:f.centroid.z)
        fi=upper.features.filletFeatures.createInput();fi.edgeSetInputs.addConstantRadiusEdgeSet(coll([e for loop in top.loops if loop.isOuter for e in loop.edges]),V('crown_radius'),True)
        body=upper.features.filletFeatures.add(fi).bodies.item(0)
        stage='hollow continuous roof'
        outer=upper.features.copyPasteBodies.add(body).bodies.item(0);outer.name='Continuous exterior trimming tool';outer.isLightBulbOn=False
        def inner_profile(sk,kind):
            pts=coll([]); expressions=[]
            for x,y in outline:
                if kind=='seat':
                    ex=f'joystick_x+({x})*(seat_width-2*continuous_roof-8 mm)'
                    ey=f'joystick_y+seat_rear_bias+({y})*(continuous_mount_dia-2*continuous_roof-8 mm)'
                elif kind=='mid':
                    ex=f'finger_mid_shift+({x})*(body_width-2*(finger_mid_inset+continuous_roof))'
                    ey=(f'({y})*(body_depth-2*(rear_mid_inset+continuous_roof))' if y>0 else f'({y})*(body_depth-entry_trim-2*(front_mid_inset+continuous_roof))+entry_trim/2')
                else:
                    ins='continuous_roof' if kind=='base' else 'finger_edge_inset+continuous_roof'
                    ex=f'({x})*(body_width-2*({ins}))'
                    ey=f'({y})*(body_depth-entry_trim-2*({ins}))+entry_trim/2'
                pts.add(P(val(ex),val(ey),0));expressions.append((ex,ey))
            pts.add(pts.item(0));sp=sk.sketchCurves.sketchFittedSplines.add(pts);sp.isClosed=True
            for n,(ex,ey) in enumerate(expressions):
                if n==len(expressions)-1:
                    pt=sp.fitPoints.item(n);line=sk.sketchCurves.sketchLines.addByTwoPoints(sp.fitPoints.item(0),pt);line.isConstruction=True
                    sk.geometricConstraints.addVertical(line)
                    dm=sk.sketchDimensions.addDistanceDimension(sk.originPoint,pt,adsk.fusion.DimensionOrientations.VerticalDimensionOrientation,P(pt.geometry.x+.2,pt.geometry.y+.2,0));dm.parameter.expression=ey if val(ey)>0 else '-('+ey+')'
                else:distance(sk,sp.fitPoints.item(n),ex,ey)
            return sk.profiles.item(0)
        sections=[]
        for z,kind in [('cover_thickness-0.2 mm','base'),('finger_edge_height','low'),('finger_mid_height','mid'),('palm_height-continuous_roof','seat')]:
            sk=sketch(z,'Inner cavity '+kind);sections.append(inner_profile(sk,kind));sk.isVisible=False
        body=loft(sections,CUT,'Parametric hollow interior with controlled roof',body)
        stage='ring ledge and holes'
        # Full mount annulus replaces the old roof locally, retaining measured R2 stack.
        body=disk('flange_seat','joystick_x','joystick_y','flange_diameter+4 mm','palm_height-flange_seat',JOIN,'Reinforced measured top-access mounting ledge').bodies.item(0)
        disk('flange_seat-0.1 mm','joystick_x','joystick_y','fit_aperture','palm_height-flange_seat+0.2 mm',CUT,'Joystick through aperture',body)
        disk('palm_height+0.1 mm','joystick_x','joystick_y','ring_pocket_diameter','-ring_stack-0.1 mm',CUT,'Retain measured flush ring recess',body)
        for sx in (-1,1):
            for sy in (-1,1):disk('flange_seat-0.1 mm',f'joystick_x+({sx})*flange_pitch_x/2',f'joystick_y+({sy})*flange_pitch_y/2','fit_screw_clearance','ring_stack+mount_ledge+0.2 mm',CUT,'Confirmed top screw access',body)
        stage='existing cover bosses'
        for n,(x,y) in enumerate([('-66 mm','entry_boss_y1'),('-60 mm','28 mm'),('74 mm','entry_boss_y3'),('-22 mm','39 mm')]):
            disk('cover_thickness',x,y,'boss_diameter','palm_height-cover_thickness',JOIN,'R3 cover column '+str(n))
            disk('cover_thickness',x,y,'cover_pilot','cover_pilot_depth',CUT,'Retain bottom screw pilot '+str(n),body)
        body=combine(body,outer,adsk.fusion.FeatureOperations.IntersectFeatureOperation,'Trim posts flush to sloped exterior')
        stage='bottom lip clearance'
        sk=sketch('cover_thickness-0.1 mm','Match existing R2 alignment lip');pr=contour(sk,'wall+0.05 mm')
        ext(sk,pr,'lip_height+0.3 mm',CUT,'Relieve thick roof walls around existing bottom lip',body)
        stage='USB opening'
        sk=sketch('usb_z-usb_height/2','Rear USB opening retained')
        x0=val('pcb_x-usb_width/2');x1=val('pcb_x+usb_width/2');y0=val('pcb_y+pcb_length/2-6 mm');y1=val('body_depth/2+10 mm')
        lines=sk.sketchCurves.sketchLines.addTwoPointRectangle(P(x0,y0,0),P(x1,y1,0))
        # R2 dimensions remain authoritative; constrain this repeated opening by expressions.
        distance(sk,lines.item(0).startSketchPoint,'pcb_x-usb_width/2','pcb_y+pcb_length/2-6 mm')
        sk.sketchDimensions.addDistanceDimension(lines.item(0).startSketchPoint,lines.item(0).endSketchPoint,adsk.fusion.DimensionOrientations.HorizontalDimensionOrientation,P(x0,y0-.3,0)).parameter.expression='usb_width'
        sk.sketchDimensions.addDistanceDimension(lines.item(1).startSketchPoint,lines.item(1).endSketchPoint,adsk.fusion.DimensionOrientations.VerticalDimensionOrientation,P(x1+.3,y0,0)).parameter.expression='body_depth/2+10 mm-(pcb_y+pcb_length/2-6 mm)'
        ext(sk,sk.profiles.item(0),'usb_height',CUT,'Rear edge USB access',body)
        sk=sketch('cover_thickness+lip_height','Internal relief around rear PCB and rail ends')
        sk.sketchCurves.sketchLines.addTwoPointRectangle(P(-1.35,val('pcb_y+pcb_length/2-7 mm'),0),P(1.35,val('pcb_y+pcb_length/2+0.7 mm'),0))
        ext(sk,sk.profiles.item(0),'pcb_z+pcb_thickness+0.7 mm-(cover_thickness+lip_height)',CUT,'Clear PCB and rails behind USB mouth',body)
        stage='handedness'
        mi=upper.features.mirrorFeatures.createInput(coll([body]),upper.yZConstructionPlane)
        mi.patternComputeOption=adsk.fusion.PatternComputeOptions.IdenticalPatternCompute
        mf=upper.features.mirrorFeatures.add(mi);mf.name='Preserve user-confirmed hand layout'
        upper.features.removeFeatures.add(body);body=upper.bRepBodies.item(0)
        body.appearance=appearance;body.name='Rounded upper - 12 mm shorter hand entry'
        stage='shorter matching bottom outline'
        base=root.occurrences.item(2).component
        if not any('entry_trim' in dm.parameter.expression for sk in base.sketches for dm in sk.sketchDimensions):
            for sk in base.sketches:
                if sk.name not in ('Matching organic bottom cover perimeter','Inset alignment lip'):continue
                for dm in sk.sketchDimensions:
                    expr=dm.parameter.expression
                    if 'body_depth' not in expr:continue
                    positive=dm.entityTwo.geometry.y>0
                    dm.parameter.expression='('+expr.replace('body_depth','body_depth-entry_trim')+')'+('+' if positive else '-')+'entry_trim/2'
        for sk in base.sketches:
            use=None
            if sk.name in ('M3 cover clearance 1','Screw 1 head mouth','Screw 1 head throat'):use='entry_boss_y1'
            if sk.name in ('M3 cover clearance 3','Screw 3 head mouth','Screw 3 head throat','Alignment lip relief around front-right screw column'):use='entry_boss_y3'
            if use:
                for dm in sk.sketchDimensions:
                    if hasattr(dm,'entityTwo') and hasattr(dm.entityTwo,'geometry') and hasattr(dm.entityTwo.geometry,'y'):
                        if abs(dm.parameter.value-abs(val('-20 mm' if use=='entry_boss_y1' else '-24 mm')))<1e-6:
                            dm.parameter.expression='-'+use
        finalize(doc,request,report)
    except Exception:report.update(error=traceback.format_exc(),stage=stage)
    with open(os.path.join(OUT,'validation_report.json'),'w') as f:json.dump(report,f,indent=2)
    with open(os.path.join(OUT,'progress.json'),'w') as f:json.dump({'status':'complete' if report.get('complete') else 'failed','stage':stage},f)


def finalize(doc,request,report):
    app=adsk.core.Application.get();d=adsk.fusion.Design.cast(app.activeProduct);root=d.rootComponent
    upper=root.occurrences.item(0).component;ps=d.userParameters;body=upper.bRepBodies.item(0)
    stage='validation';d.computeAll()
    warnings=[]
    for t in d.timeline:
        e=t.entity
        if hasattr(e,'healthState') and e.healthState!=adsk.fusion.FeatureHealthStates.HealthyFeatureHealthState:warnings.append(dict(name=e.name,message=e.errorOrWarningMessage))
    ii=d.createInterferenceInput(coll([root.occurrences.item(n) for n in (0,2,3,4,5,7)]));ii.areCoincidentFacesIncluded=False
    interference=[]
    for r in d.analyzeInterference(ii):
        if r.interferenceBody and r.interferenceBody.volume>0.00001:interference.append(dict(one=r.entityOne.name,two=r.entityTwo.name,volume_cm3=r.interferenceBody.volume))
    report.update(warnings=warnings,interference=interference,parameters={p.name:p.expression for p in ps},print_parts=[])
    assert upper.bRepBodies.count==1 and body.isSolid
    assert not warnings and not interference,(warnings,interference)
    stage='exports'
    for name,n in [('Upper_Shell',0),('Bottom_Cover',2),('PCB_Retainer',3)]:
        b=root.occurrences.item(n).component.bRepBodies.item(0);bb=b.boundingBox
        so=d.exportManager.createSTLExportOptions(b,os.path.join(OUT,name+'_assembly.stl'));so.sendToPrintUtility=False
        so.meshRefinement=adsk.fusion.MeshRefinementSettings.MeshRefinementHigh
        if n==0:so.surfaceDeviation=.0005;so.normalDeviation=math.radians(1);so.maximumEdgeLength=.1
        assert d.exportManager.execute(so)
        report['print_parts'].append(dict(name=name,solid=b.isSolid,volume_cm3=b.volume,bbox_mm=[(bb.maxPoint.asArray()[i]-bb.minPoint.asArray()[i])*10 for i in range(3)]))
    def shot(name,eye,target=(0,0,3)):
        app.userInterface.activeSelections.clear();cam=app.activeViewport.camera;cam.isSmoothTransition=False
        cam.eye=P(*eye);cam.target=P(*target);cam.upVector=adsk.core.Vector3D.create(0,0,1)
        app.activeViewport.camera=cam;app.activeViewport.fit();app.activeViewport.refresh();adsk.doEvents()
        app.activeViewport.saveAsImageFile(os.path.join(OUT,name),1800,1300)
    shot('Ergonomic_Full.png',(0,-34,22))
    for n in (5,6,7):root.occurrences.item(n).isLightBulbOn=False
    shot('Ergonomic_Palm.png',(12,-30,25));shot('Ergonomic_Side.png',(28,0,8))
    for n in (5,6,7):root.occurrences.item(n).isLightBulbOn=True
    shot('Ergonomic_Full.png',(0,-34,22))
    assert d.exportManager.execute(d.exportManager.createFusionArchiveExportOptions(os.path.join(OUT,'SpaceMouse_Master.f3d')))
    if request.get('apply'):
        assert doc.save('Rounded tangent crown and 12 mm shorter hand-entry footprint; matching bottom updated')
        report['cloud_master_id']=doc.dataFile.id;report['cloud_project']=doc.dataFile.parentProject.name
        report['cloud_version']=doc.dataFile.versionNumber
        for other in list(app.documents):
            if other!=doc and not other.dataFile and other.name.startswith('Ergonomic trial'):other.close(False)
    report['complete']=True
