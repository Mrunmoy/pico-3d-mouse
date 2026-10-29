"""Four perimeter-following TPU inserts and mating pockets in the single master."""
import adsk.core, adsk.fusion, os, json, math, traceback
ROOT=os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..'))
OUT=os.path.join(ROOT,'output','tpu-feet')
V=adsk.core.ValueInput.createByString
P=adsk.core.Point3D.create
def collection(items):
    c=adsk.core.ObjectCollection.create()
    for x in items:c.add(x)
    return c
def run(request):
    os.makedirs(OUT,exist_ok=True);report={};stage='start'
    def status(s):
        with open(os.path.join(OUT,'progress.json'),'w') as f:json.dump({'stage':s},f)
    try:
        app=adsk.core.Application.get();doc=app.activeDocument
        expected=json.load(open(os.path.join(ROOT,'output','asymmetric','save_record.json')))['master_id']
        assert doc.dataFile.id==expected
        d=adsk.fusion.Design.cast(app.activeProduct);root=d.rootComponent;d.timeline.moveToEnd()
        base=root.occurrences.item(2).component;body=base.bRepBodies.item(0)
        if request.get('finish'):
            report=json.load(open(os.path.join(OUT,'validation_report.json')))
            report.pop('error',None);report.pop('stage',None)
            inserts=[o for o in root.occurrences if o.component.name.startswith('TPU perimeter')]
            assert len(inserts)==4
            print_parts=[('Bottom_Cover',body)]+[(f'TPU_Strip_{i+1}',o.component.bRepBodies.item(0)) for i,o in enumerate(inserts)]
            finish(app,doc,d,root,base,body,inserts,print_parts,report)
            with open(os.path.join(OUT,'validation_report.json'),'w') as f:json.dump(report,f,indent=2)
            return
        assert not any(o.component.name.startswith('TPU perimeter') for o in root.occurrences),'Already built'
        ps=d.userParameters
        for name,expr,comment in [('tpu_edge_inset','5.5 mm','Strip centre inset from sampled cover perimeter'),('tpu_strip_width','6 mm','Mating pocket width'),('tpu_pocket_depth','1 mm','Pocket depth; leaves 2 mm plate roof'),('tpu_side_clearance','0.2 mm','Clearance on each side of insert'),('tpu_adhesive_gap','0.2 mm','Thin adhesive allowance above TPU'),('tpu_projection','0.8 mm','TPU below hard cover underside')]:
            if not ps.itemByName(name):ps.add(name,V(expr),'mm',comment)
        def val(e):return d.unitsManager.evaluateExpression(e,'mm')
        stage='remove round recesses';status(stage)
        old=[]
        for feat in base.features.extrudeFeatures:
            if feat.name.startswith('Adhesive foot recess'):
                old.append({'name':feat.name,'was_suppressed':feat.isSuppressed})
                if not feat.isSuppressed:feat.isSuppressed=True
        d.timeline.moveToEnd();d.computeAll();body=base.bRepBodies.item(0)
        report['old_recesses']=old
        face=max((f for f in body.faces if abs(f.boundingBox.minPoint.z)<1e-6 and abs(f.boundingBox.maxPoint.z)<1e-6),key=lambda f:f.area)
        loop=next(l for l in face.loops if l.isOuter)
        points=[]
        for edge in loop.edges:
            ev=edge.evaluator;ok,a,b=ev.getParameterExtents();ok,pts=ev.getStrokes(a,b,.002)
            assert ok
            if points and points[-1].distanceTo(pts[-1])<points[-1].distanceTo(pts[0]):pts=list(reversed(pts))
            points.extend(pts if not points else pts[1:])
        if points[-1].distanceTo(points[0])>1e-6:points.append(points[0])
        xy=[(p.x,p.y) for p in points];arc=[0]
        for a,b in zip(xy,xy[1:]):arc.append(arc[-1]+math.dist(a,b))
        perimeter=arc[-1];area=sum(a[0]*b[1]-b[0]*a[1] for a,b in zip(xy,xy[1:]));sign=1 if area>0 else -1
        def sample(s):
            s%=perimeter
            j=next(i for i in range(len(arc)-1) if arc[i]<=s<=arc[i+1])
            a,b=xy[j:j+2];length=arc[j+1]-arc[j];t=(s-arc[j])/length
            tx,ty=(b[0]-a[0])/length,(b[1]-a[1])/length
            return (a[0]+t*(b[0]-a[0]),a[1]+t*(b[1]-a[1]),-ty*sign,tx*sign,tx,ty)
        holes=[]
        for l in face.loops:
            if l.isOuter:continue
            pts2=[e.pointOnEdge for e in l.edges]
            # Circular head-mouth loops use exact circle centre.
            for e in l.edges:
                if isinstance(e.geometry,adsk.core.Circle3D):
                    holes.append((e.geometry.center.x,e.geometry.center.y,e.geometry.radius));break
        assert len(holes)==4,holes
        # Pick four distributed arcs with the best screw-head clearance within each quarter.
        paths=[];length=4.0;inset=val('tpu_edge_inset');width=val('tpu_strip_width')
        for q in range(4):
            candidates=[]
            for k in range(21):
                centre=perimeter*(q+.25+k*.025)/4
                rows=[sample(centre-length/2+length*j/8) for j in range(9)]
                clearance=min(math.hypot(x+nx*inset-hx,y+ny*inset-hy)-hr-width/2 for x,y,nx,ny,tx,ty in rows for hx,hy,hr in holes)
                candidates.append((clearance,rows))
            clearance,rows=max(candidates,key=lambda x:x[0]);assert clearance>.15,clearance
            paths.append(rows)
            report.setdefault('strips',[]).append({'number':q+1,'centreline_length_mm':40,'screw_head_clearance_mm':clearance*10})
        def sketch(c,z,name):
            pi=c.constructionPlanes.createInput();pi.setByOffset(c.xYConstructionPlane,V(z))
            pl=c.constructionPlanes.add(pi);pl.isLightBulbOn=False
            sk=c.sketches.add(pl);sk.name=name;return sk
        def ribbon(sk,rows,insert=False):
            sk.isComputeDeferred=True
            width_expr='tpu_strip_width/2-tpu_side_clearance' if insert else 'tpu_strip_width/2'
            # Preserve expression-driven spline fit points; perimeter samples are fixed design references.
            curves=[]
            for side in (-1,1):
                expressions=[];pts=[]
                for x,y,nx,ny,tx,ty in rows:
                    ex=f'{x*10:.8f} mm+({nx:.9f})*(tpu_edge_inset+({side})*({width_expr}))'
                    ey=f'{y*10:.8f} mm+({ny:.9f})*(tpu_edge_inset+({side})*({width_expr}))'
                    expressions.append((ex,ey));pts.append(P(val(ex),val(ey),0))
                curve=sk.sketchCurves.sketchFittedSplines.add(collection(pts));curves.append(curve)
                for pt,(ex,ey) in zip(curve.fitPoints,expressions):
                    for expr,ori in [(ex,adsk.fusion.DimensionOrientations.HorizontalDimensionOrientation),(ey,adsk.fusion.DimensionOrientations.VerticalDimensionOrientation)]:
                        dm=sk.sketchDimensions.addDistanceDimension(sk.originPoint,pt,ori,P(pt.geometry.x+.1,pt.geometry.y+.1,0))
                        dm.parameter.expression=expr if val(expr)>=0 else '-('+expr+')'
            for j,direction in [(0,-1),(-1,1)]:
                x,y,nx,ny,tx,ty=rows[j];r=val(width_expr)
                a=curves[0].startSketchPoint if j==0 else curves[0].endSketchPoint
                b=curves[1].startSketchPoint if j==0 else curves[1].endSketchPoint
                sk.sketchCurves.sketchArcs.addByThreePoints(a,P(x+nx*inset+direction*tx*r,y+ny*inset+direction*ty*r,0),b)
            sk.isComputeDeferred=False
            assert sk.profiles.count==1,sk.profiles.count
        def extrude(c,sk,height,op,name,target=None):
            ei=c.features.extrudeFeatures.createInput(sk.profiles.item(0),op);ei.setDistanceExtent(False,V(height))
            if target:ei.participantBodies=[target]
            f=c.features.extrudeFeatures.add(ei);f.name=name;sk.isVisible=False;return f
        stage='curved pockets and inserts';status(stage)
        print_parts=[('Bottom_Cover',body)]
        inserts=[]
        for i,rows in enumerate(paths,1):
            status(f'curved pocket and TPU insert {i} of 4')
            sk=sketch(base,'0 mm',f'TPU perimeter pocket {i} - editable spline');ribbon(sk,rows)
            extrude(base,sk,'tpu_pocket_depth',adsk.fusion.FeatureOperations.CutFeatureOperation,f'TPU perimeter pocket {i}',body)
            occ=root.occurrences.addNewComponent(adsk.core.Matrix3D.create());c=occ.component;c.name=f'TPU perimeter strip {i}'
            sk=sketch(c,'-tpu_projection',f'TPU strip {i} matching perimeter');ribbon(sk,rows,True)
            f=extrude(c,sk,'tpu_pocket_depth-tpu_adhesive_gap+tpu_projection',adsk.fusion.FeatureOperations.NewBodyFeatureOperation,f'TPU insert {i} thickness')
            b=f.bodies.item(0);b.name=f'TPU strip {i}';inserts.append(occ);print_parts.append((f'TPU_Strip_{i}',b))
            appearance=app.materialLibraries.itemByName('Fusion 360 Appearance Library')
            if appearance:
                a=appearance.appearances.itemByName('Rubber - Silicone (Red)')
                if a:b.appearance=a
        finish(app,doc,d,root,base,body,inserts,print_parts,report)
    except Exception:report.update(error=traceback.format_exc(),stage=stage);status('failed')
    with open(os.path.join(OUT,'validation_report.json'),'w') as f:json.dump(report,f,indent=2)


def finish(app,doc,d,root,base,body,inserts,print_parts,report):
    ps=d.userParameters
    d.computeAll()
    warnings=[]
    for t in d.timeline:
        e=t.entity
        if not (hasattr(e,'isSuppressed') and e.isSuppressed) and hasattr(e,'healthState') and e.healthState!=adsk.fusion.FeatureHealthStates.HealthyFeatureHealthState:warnings.append({'name':e.name,'message':e.errorOrWarningMessage})
    assert not warnings,warnings
    ii=d.createInterferenceInput(collection([root.occurrences.item(2)]+inserts));ii.areCoincidentFacesIncluded=False
    clashes=[]
    for r in d.analyzeInterference(ii):
        if r.interferenceBody and r.interferenceBody.volume>1e-7:clashes.append(r.interferenceBody.volume)
    assert not clashes,clashes
    report.update(warnings=warnings,interference=clashes,parameters={p.name:p.expression for p in ps if p.name.startswith('tpu_')},print_parts=[])
    for name,b in print_parts:
        assert b.isSolid
        opt=d.exportManager.createSTLExportOptions(b,os.path.join(OUT,name+'_assembly.stl'));opt.sendToPrintUtility=False
        opt.meshRefinement=adsk.fusion.MeshRefinementSettings.MeshRefinementHigh;opt.surfaceDeviation=.0005;opt.normalDeviation=math.radians(1)
        assert d.exportManager.execute(opt)
        report['print_parts'].append({'name':name,'solid':b.isSolid,'volume_cm3':b.volume})
    # Show underside with the four separate insert components installed.
    vp=app.activeViewport;cam=vp.camera;cam.isSmoothTransition=False;cam.cameraType=adsk.core.CameraTypes.OrthographicCameraType
    cam.eye=P(0,0,-35);cam.target=P(0,0,0);cam.upVector=adsk.core.Vector3D.create(0,-1,0)
    for o in root.occurrences:o.isLightBulbOn=(o.component==base or o in inserts)
    vp.camera=cam;vp.visualStyle=adsk.core.VisualStyles.ShadedWithVisibleEdgesOnlyVisualStyle;vp.fit();vp.refresh();adsk.doEvents()
    vp.saveAsImageFile(os.path.join(OUT,'Bottom_with_TPU.png'),2000,1400)
    for o in inserts:o.isLightBulbOn=False
    vp.refresh();adsk.doEvents();vp.saveAsImageFile(os.path.join(OUT,'Bottom_pockets.png'),2000,1400)
    for o in root.occurrences:o.isLightBulbOn=(o.component!=root.occurrences.item(1).component and not o.component.name.startswith('REF USB'))
    cam=vp.camera;cam.eye=P(-25,-32,25);cam.target=P(0,0,3.5);cam.upVector=adsk.core.Vector3D.create(0,0,1);vp.camera=cam;vp.visualStyle=adsk.core.VisualStyles.ShadedVisualStyle;vp.fit();vp.refresh()
    assert doc.save('Replace round feet with four curved perimeter TPU inserts and adhesive pockets')
    assert d.exportManager.execute(d.exportManager.createFusionArchiveExportOptions(os.path.join(OUT,'SpaceMouse_Master.f3d')))
    report.update(complete=True,master_id=doc.dataFile.id,project=doc.dataFile.parentProject.name,version=doc.dataFile.versionNumber,modified=doc.isModified)
    with open(os.path.join(OUT,'progress.json'),'w') as f:json.dump({'stage':'complete'},f)
