"""Separate split-shell engineering concept; never replaces the reviewed master."""
import adsk.core,adsk.fusion,os,json,math,traceback
ROOT=r'D:\github\fusion360\space-mouse';OUT=os.path.join(ROOT,'output','split-concept')
P=adsk.core.Point3D.create;V=adsk.core.ValueInput.createByString
def col(items):
    c=adsk.core.ObjectCollection.create()
    for x in items:c.add(x)
    return c
def run(request):
    os.makedirs(OUT,exist_ok=True);report={};stage='preserve current master'
    def progress(s):
        with open(os.path.join(OUT,'progress.json'),'w') as f:json.dump({'stage':s},f)
    try:
        app=adsk.core.Application.get()
        if request.get('finish'):
            doc=app.activeDocument;d=adsk.fusion.Design.cast(app.activeProduct);root=d.rootComponent
            upper=root.occurrences.item(0).component;cap=next(o.component for o in root.occurrences if o.component.name=='Concept - curved palm cap')
            report=json.load(open(os.path.join(OUT,'report.json')));report.pop('error',None)
            ps=d.userParameters;body=cap.bRepBodies.item(0)
            def cut_profile(z,depth,name,circle=False):
                pi=cap.constructionPlanes.createInput();pi.setByOffset(cap.xYConstructionPlane,V(z));pl=cap.constructionPlanes.add(pi);pl.isLightBulbOn=False
                sk=cap.sketches.add(pl);sk.name=name
                x=-ps.itemByName('joystick_x').value;y=ps.itemByName('joystick_y').value
                if circle:sk.sketchCurves.sketchCircles.addByCenterRadius(P(x,y,0),2.68)
                else:sk.sketchCurves.sketchLines.addTwoPointRectangle(P(x-2.55,y-2.55,0),P(x+2.55,y+2.55,0))
                ei=cap.features.extrudeFeatures.createInput(sk.profiles.item(0),adsk.fusion.FeatureOperations.CutFeatureOperation);ei.setDistanceExtent(False,V(depth));ei.participantBodies=[body]
                cap.features.extrudeFeatures.add(ei).name=name;sk.isVisible=False
            if request.get('clearance'):
                cut_profile('flange_seat-flange_thickness','-40 mm','51 mm lower mechanism relief')
                cut_profile('flange_seat','-flange_thickness-0.1 mm','Flange underside relief 53.6 mm',True)
            if request.get('grounding'):
                frame=upper.bRepBodies.item(0)
                faces=[f for f in frame.faces if abs(f.boundingBox.maxPoint.z-f.boundingBox.minPoint.z)<1e-6 and any(abs(f.centroid.z-z)<1e-5 for z in (.52,.53))]
                for face in faces:
                    depth=face.centroid.z-.3
                    ei=upper.features.extrudeFeatures.createInput(face,adsk.fusion.FeatureOperations.JoinFeatureOperation);ei.setDistanceExtent(False,V(f'{-depth*10:.6f} mm'))
                    upper.features.extrudeFeatures.add(ei).name='Ground frame posts and ribs for support-free starts'
                base=root.occurrences.item(2).component
                # Remove only intersections of the new low ribs with the old alignment lip.
                bf=base.features.baseFeatures.add();bf.startEdit();tool=base.bRepBodies.add(adsk.fusion.TemporaryBRepManager.get().copy(frame),bf);bf.finishEdit()
                target=max(list(base.bRepBodies),key=lambda b:b.volume);tool=min(list(base.bRepBodies),key=lambda b:b.volume)
                ci=base.features.combineFeatures.createInput(target,col([tool]));ci.operation=adsk.fusion.FeatureOperations.CutFeatureOperation
                base.features.combineFeatures.add(ci).name='Match split frame ribs in existing base alignment lip'
            if request.get('grounding2'):
                frame=upper.bRepBodies.item(0)
                faces=[f for f in frame.faces if abs(f.boundingBox.maxPoint.z-f.boundingBox.minPoint.z)<1e-6 and any(abs(f.centroid.z-z)<1e-5 for z in (.52,.53))]
                for face in faces:
                    depth=face.centroid.z-.3
                    pi=upper.constructionPlanes.createInput();pi.setByOffset(upper.xYConstructionPlane,V('3 mm'));pl=upper.constructionPlanes.add(pi);pl.isLightBulbOn=False
                    sk=upper.sketches.add(pl);sk.name='Bed-start footprint for frame rib or post';sk.project(face)
                    profile=max(list(sk.profiles),key=lambda pr:pr.areaProperties().area)
                    ei=upper.features.extrudeFeatures.createInput(profile,adsk.fusion.FeatureOperations.JoinFeatureOperation);ei.setDistanceExtent(False,V(f'{depth*10+.01:.6f} mm'))
                    upper.features.extrudeFeatures.add(ei).name='Extend rib or post down to build plane';sk.isVisible=False
                base=root.occurrences.item(2).component
                bf=base.features.baseFeatures.add();bf.startEdit();base.bRepBodies.add(adsk.fusion.TemporaryBRepManager.get().copy(frame),bf);bf.finishEdit()
                target=max(list(base.bRepBodies),key=lambda b:b.volume);tool=min(list(base.bRepBodies),key=lambda b:b.volume)
                ci=base.features.combineFeatures.createInput(target,col([tool]));ci.operation=adsk.fusion.FeatureOperations.CutFeatureOperation
                base.features.combineFeatures.add(ci).name='Relieve alignment lip at grounded frame ribs'
            if request.get('trim_grounding'):
                base=root.occurrences.item(2).component
                target=max(list(base.bRepBodies),key=lambda b:b.volume);tool=min(list(base.bRepBodies),key=lambda b:b.volume)
                ci=base.features.combineFeatures.createInput(target,col([tool]));ci.operation=adsk.fusion.FeatureOperations.CutFeatureOperation
                base.features.combineFeatures.add(ci).name='Match split frame ribs in base alignment lip'
            ii=d.createInterferenceInput(col([root.occurrences.item(i) for i in (0,2,3,4,5,7)]+[next(o for o in root.occurrences if o.component==cap)]));ii.areCoincidentFacesIncluded=False
            clashes=[]
            for r in (d.analyzeInterference(ii) or []):
                if r.interferenceBody and r.interferenceBody.volume>1e-5:clashes.append(dict(one=r.entityOne.name,two=r.entityTwo.name,volume_mm3=r.interferenceBody.volume*1000))
            report['interference']=clashes
            with open(os.path.join(OUT,'interference.json'),'w') as f:json.dump(clashes,f,indent=2)
            finalize(app,doc,d,root,upper,cap,report)
            if request.get('save_cloud'):
                master=next(x for x in app.documents if x.dataFile and x.dataFile.id=='urn:adsk.wipprod:dm.lineage:BfuyUQ4XTJGKQyfE52KnBw')
                if doc.dataFile:assert doc.save('Refine split-shell review geometry')
                else:assert doc.saveAs('SpaceMouse - Split Shell Concept - Review',master.dataFile.parentFolder,'Separate split upper concept; printability and fasteners under review','')
                report['cloud_name']=doc.dataFile.name;report['cloud_id']=doc.dataFile.id
            with open(os.path.join(OUT,'report.json'),'w') as f:json.dump(report,f,indent=2)
            return
        if request.get('inspect'):
            doc=app.activeDocument;d=adsk.fusion.Design.cast(app.activeProduct)
        else:
            for olddoc in list(app.documents):
                if not olddoc.dataFile and olddoc.name.startswith('SpaceMouse - Split Shell Concept'):olddoc.close(False)
            doc=app.importManager.importToNewDocument(app.importManager.createFusionArchiveImportOptions(os.path.join(ROOT,'output','tpu-feet','SpaceMouse_Master.f3d')))
            doc.name='SpaceMouse - Split Shell Concept - Review'
            d=adsk.fusion.Design.cast(app.activeProduct)
        root=d.rootComponent;upper=root.occurrences.item(0).component;temp=adsk.fusion.TemporaryBRepManager.get()
        def bounds(b):return dict(name=b.name,min=[v*10 for v in b.boundingBox.minPoint.asArray()],max=[v*10 for v in b.boundingBox.maxPoint.asArray()])
        report['references']=[dict(component=o.component.name,bodies=[bounds(b) for b in o.component.bRepBodies]) for o in root.occurrences]
        if request.get('inspect'):
            with open(os.path.join(OUT,'inspection.json'),'w') as f:json.dump(report,f,indent=2)
            return
        stage='extract original smooth exterior';progress(stage)
        loft=next(f for f in upper.features.loftFeatures if f.name=='Nested smooth exterior with asymmetric palm crossfall')
        crown=min((f for f in upper.features.filletFeatures if f.timelineObject.index>loft.timelineObject.index),key=lambda f:f.timelineObject.index)
        d.timeline.markerPosition=crown.timelineObject.index+1
        outer=temp.copy(upper.bRepBodies.item(0));d.timeline.moveToEnd()
        transform=adsk.core.Matrix3D.create();transform.setCell(0,0,-1);assert temp.transform(outer,transform)
        occ=root.occurrences.addNewComponent(adsk.core.Matrix3D.create());cap=occ.component;cap.name='Concept - curved palm cap'
        bf=cap.features.baseFeatures.add();bf.startEdit();capbody=cap.bRepBodies.add(outer,bf);bf.finishEdit()
        capbody=cap.bRepBodies.item(0);capbody.name='Planar-back curved palm cap'
        ps=d.userParameters
        for name,expr,unit,note in [('split_height','22 mm','mm','Joint plane intercept'),('split_pitch','0.14','','Downward pitch toward palm end'),('split_crossfall','0.04','','Asymmetric joint crossfall'),('split_seam','0.25 mm','mm','Discreet cap/frame seam'),('split_ledge','3 mm','mm','Minimum joystick mounting ledge')]:
            ps.add(name,V(expr),unit,note)
        def val(e):
            if e in ('split_pitch','split_crossfall'):return ps.itemByName(e).value
            return d.unitsManager.evaluateExpression(e,'mm')
        # Plane z=height-pitch*x+crossfall*y, defined by expression-driven datum points.
        def joint_plane(c,gap,name):
            sk=c.sketches.add(c.xZConstructionPlane);sk.name=name+' pitch datum'
            # Construction point geometry is fixed at this concept's pitch; plane height is editable.
            pi=c.constructionPlanes.createInput();pi.setByAngle(c.yConstructionAxis,V('atan(split_pitch)'),c.xYConstructionPlane)
            pitch=c.constructionPlanes.add(pi);pitch.isLightBulbOn=False
            # Use three construction points for compound pitch/crossfall.
            sk3=c.sketches.add(c.xYConstructionPlane);sk3.is3D=True;sk3.name=name+' plane control points'
            points=[sk3.sketchPoints.add(P(x,y,val('split_height')-val('split_pitch')*x+val('split_crossfall')*y)) for x,y in [(0,0),(10,0),(0,10)]]
            pi=c.constructionPlanes.createInput();pi.setByThreePoints(*points);pl=c.constructionPlanes.add(pi);pl.name=name+' compound datum';pl.isLightBulbOn=False;sk3.isVisible=False;sk.isVisible=False
            if gap:
                pi=c.constructionPlanes.createInput();pi.setByOffset(pl,V(gap));pl=c.constructionPlanes.add(pi);pl.name=name;pl.isLightBulbOn=False
            return pl
        stage='split lower frame and flat-back cap';progress(stage)
        framebody=upper.bRepBodies.item(0);frameplane=joint_plane(upper,None,'Frame joint')
        capplane=joint_plane(cap,'split_seam','Palm cap flat print face')
        for c,b,pl,above in [(upper,framebody,frameplane,False),(cap,capbody,capplane,True)]:
            si=c.features.splitBodyFeatures.createInput(b,pl,True);c.features.splitBodyFeatures.add(si).name='Continuous discreet cap frame joint'
            for piece in list(c.bRepBodies):
                pt=piece.physicalProperties.centerOfMass
                signed=pt.z+val('split_pitch')*pt.x-val('split_crossfall')*pt.y-val('split_height')
                if (signed>0)!=above:c.features.removeFeatures.add(piece)
        upper.name='Concept - lower frame';capbody=cap.bRepBodies.item(0)
        def sketch(c,z,name):
            pi=c.constructionPlanes.createInput();pi.setByOffset(c.xYConstructionPlane,V(z));pl=c.constructionPlanes.add(pi);pl.isLightBulbOn=False
            sk=c.sketches.add(pl);sk.name=name;return sk
        def cut(sk,depth,name):
            ei=cap.features.extrudeFeatures.createInput(col(list(sk.profiles)),adsk.fusion.FeatureOperations.CutFeatureOperation);ei.setDistanceExtent(False,V(depth));ei.participantBodies=[capbody]
            f=cap.features.extrudeFeatures.add(ei);f.name=name;sk.isVisible=False
        stage='measured top mount';progress(stage)
        sk=sketch(cap,'0 mm','Retain joystick through opening')
        sk.sketchCurves.sketchCircles.addByCenterRadius(P(-val('joystick_x'),val('joystick_y'),0),val('fit_aperture')/2)
        cut(sk,'50 mm','Open joystick bore through flat cap')
        sk=sketch(cap,'palm_height','Retain flush measured ring pocket')
        sk.sketchCurves.sketchCircles.addByCenterRadius(P(-val('joystick_x'),val('joystick_y'),0),val('ring_pocket_diameter')/2)
        cut(sk,'-ring_stack','Recess measured ring and cushion')
        sk=sketch(cap,'0 mm','Four top access mounting screws')
        for sx in [-1,1]:
            for sy in [-1,1]:sk.sketchCurves.sketchCircles.addByCenterRadius(P(-val('joystick_x')+sx*val('flange_pitch_x')/2,val('joystick_y')+sy*val('flange_pitch_y')/2,0),val('fit_screw_clearance')/2)
        cut(sk,'50 mm','Retain verified screw centres')
        # Conservative mechanism cavity, stopping at the original flange underside.
        # Corner bridges surrounding the central through-bore must be checked in the slicer.
        sk=sketch(cap,'flange_seat','Mechanism clearance below structural ring ledge')
        sk.sketchCurves.sketchLines.addTwoPointRectangle(P(-val('joystick_x')-2.25,val('joystick_y')-2.25,0),P(-val('joystick_x')+2.25,val('joystick_y')+2.25,0))
        cut(sk,'-40 mm','Local mechanism clearance in cap')
        stage='connect frame posts and cap fasteners';progress(stage)
        # Low ribs tie the three formerly roof-supported posts into the perimeter.
        # A full exterior tool trims them exactly to the existing organic envelope.
        source_app=upper.bRepBodies.item(upper.bRepBodies.count-1).appearance
        for x,y,edge_y in [(22,39,60),(60,28,60),(66,-10,-50)]:
            sk=sketch(upper,'5.3 mm',f'Internal support rib {x}')
            sk.sketchCurves.sketchLines.addTwoPointRectangle(P((x-1.2)/10,min(y,edge_y)/10,0),P((x+1.2)/10,max(y,edge_y)/10,0))
            ei=upper.features.extrudeFeatures.createInput(sk.profiles.item(0),adsk.fusion.FeatureOperations.NewBodyFeatureOperation);ei.setDistanceExtent(False,V('3 mm'))
            upper.features.extrudeFeatures.add(ei);sk.isVisible=False
        target=max(list(upper.bRepBodies),key=lambda b:b.volume)
        tools=[b for b in upper.bRepBodies if b!=target]
        ci=upper.features.combineFeatures.createInput(target,col(tools));ci.operation=adsk.fusion.FeatureOperations.JoinFeatureOperation
        upper.features.combineFeatures.add(ci).name='Ribs join all frame screw posts'
        bf=upper.features.baseFeatures.add();bf.startEdit();tool=upper.bRepBodies.add(temp.copy(outer),bf);bf.finishEdit()
        tool=next(b for b in upper.bRepBodies if b!=target)
        ci=upper.features.combineFeatures.createInput(target,col([tool]));ci.operation=adsk.fusion.FeatureOperations.IntersectFeatureOperation
        upper.features.combineFeatures.add(ci).name='Trim internal ribs to original organic exterior'
        # Existing bottom screws remain for the removable base. Four additional short
        # screws are accessible inside the frame with the bottom removed.
        for i,(x,y) in enumerate([(-74,-12),(22,39),(60,28),(66,-10)],1):
            start=val('split_height')-val('split_pitch')*x/10+val('split_crossfall')*y/10
            sk=sketch(cap,f'{start*10+.27:.6f} mm',f'Cap underside pilot {i}')
            sk.sketchCurves.sketchCircles.addByCenterRadius(P(x/10,y/10,0),.10)
            cut(sk,'2.5 mm',f'Blind M2.5 cap pilot {i}')
            sk=sketch(upper,f'{start*10-3:.6f} mm',f'Frame screw access {i}')
            sk.sketchCurves.sketchCircles.addByCenterRadius(P(x/10,y/10,0),.14)
            ei=upper.features.extrudeFeatures.createInput(sk.profiles.item(0),adsk.fusion.FeatureOperations.CutFeatureOperation);ei.setDistanceExtent(False,V('5 mm'));ei.participantBodies=[upper.bRepBodies.item(0)]
            upper.features.extrudeFeatures.add(ei);sk.isVisible=False
        for b in upper.bRepBodies:b.appearance=source_app
        for b in cap.bRepBodies:b.appearance=source_app
        finalize(app,doc,d,root,upper,cap,report)
    except Exception:report.update(error=traceback.format_exc(),stage=stage);progress('failed')
    with open(os.path.join(OUT,'report.json'),'w') as f:json.dump(report,f,indent=2)


def finalize(app,doc,d,root,upper,cap,report):
    def bounds(b):return dict(name=b.name,min=[v*10 for v in b.boundingBox.minPoint.asArray()],max=[v*10 for v in b.boundingBox.maxPoint.asArray()])
    ps=d.userParameters;occ=next(o for o in root.occurrences if o.component==cap)
    d.computeAll();report['warnings']=[]
    for t in d.timeline:
        e=t.entity
        if not(hasattr(e,'isSuppressed') and e.isSuppressed) and hasattr(e,'healthState') and e.healthState!=adsk.fusion.FeatureHealthStates.HealthyFeatureHealthState:report['warnings'].append(dict(name=e.name,message=e.errorOrWarningMessage))
    assert not report['warnings'],report['warnings']
    report['parts']=[]
    for name,c in [('Lower_Frame',upper),('Palm_Cap',cap),('Bottom_Cover',root.occurrences.item(2).component)]:
        assert c.bRepBodies.count==1 and c.bRepBodies.item(0).isSolid
        b=c.bRepBodies.item(0);opt=d.exportManager.createSTLExportOptions(b,os.path.join(OUT,name+'_assembly.stl'));opt.meshRefinement=adsk.fusion.MeshRefinementSettings.MeshRefinementHigh;opt.surfaceDeviation=.001;opt.normalDeviation=math.radians(1);opt.sendToPrintUtility=False
        assert d.exportManager.execute(opt);report['parts'].append(dict(name=name,volume_cm3=b.volume,bounds=bounds(b)))
    app.userInterface.activeSelections.clear();vp=app.activeViewport;vp.visualStyle=adsk.core.VisualStyles.ShadedWithVisibleEdgesOnlyVisualStyle
    for o in root.occurrences:o.isLightBulbOn=not(o==root.occurrences.item(1) or o==root.occurrences.item(4))
    def shot(name,eye,target=(0,0,3.5)):
        cam=vp.camera;cam.isSmoothTransition=False;cam.cameraType=adsk.core.CameraTypes.OrthographicCameraType;cam.eye=P(*eye);cam.target=P(*target);cam.upVector=adsk.core.Vector3D.create(0,0,1);vp.camera=cam;vp.fit();vp.refresh();adsk.doEvents();vp.saveAsImageFile(os.path.join(OUT,name+'.png'),2200,1600)
    shot('Assembled',(-25,-32,25));shot('Wrist_edge',(0,-40,5));shot('Palm_end',(40,0,6))
    # Explode only the new cap, ring and joystick proxies for an honest assembly illustration.
    saved=[]
    for o in [occ,root.occurrences.item(5),root.occurrences.item(6),root.occurrences.item(7)]:
        m=o.transform2;saved.append((o,m.copy()));m.translation=adsk.core.Vector3D.create(0,0,3 if o==occ else 5);o.transform2=m
    shot('Exploded',(-25,-32,28),target=(0,0,5))
    for o,m in saved:o.transform2=m
    shot('Assembled',(-25,-32,25))
    assert d.exportManager.execute(d.exportManager.createFusionArchiveExportOptions(os.path.join(OUT,'SpaceMouse_Split_Concept.f3d')))
    report.update(complete=True,source='Preserved SpaceMouse master v18',status='Separate engineering concept; printability pending slicer audit',parameters={p.name:p.expression for p in ps if p.name.startswith('split_')})
    with open(os.path.join(OUT,'progress.json'),'w') as f:json.dump({'stage':'complete'},f)
