"""R3 palm revision. Trial locally first; apply=True updates the single cloud master."""
import adsk.core, adsk.fusion, math, os, json, traceback
ROOT=os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..'))
OUT=os.path.join(ROOT,'output','ergonomic')
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
    try:
        if request.get('inspect'):
            d=adsk.fusion.Design.cast(app.activeProduct);root=d.rootComponent
            ii=d.createInterferenceInput(coll([root.occurrences.item(n) for n in (0,2,3,4,5,7)]));ii.areCoincidentFacesIncluded=False
            rows=[]
            for r in d.analyzeInterference(ii):
                if r.interferenceBody:
                    bb=r.interferenceBody.boundingBox
                    rows.append(dict(one=r.entityOne.name,two=r.entityTwo.name,low=[x*10 for x in bb.minPoint.asArray()],high=[x*10 for x in bb.maxPoint.asArray()],volume_mm3=r.interferenceBody.volume*1000))
            with open(os.path.join(OUT,'clash_locations.json'),'w') as f:json.dump(rows,f,indent=2)
            cam=app.activeViewport.camera;cam.isSmoothTransition=False;cam.eye=P(0,-32,20);cam.target=P(0,0,3);cam.upVector=adsk.core.Vector3D.create(0,0,1)
            app.activeViewport.camera=cam;app.activeViewport.fit();app.activeViewport.refresh();adsk.doEvents()
            app.activeViewport.saveAsImageFile(os.path.join(OUT,'Trial.png'),1800,1300)
            return
        if request.get('apply'):
            master=json.load(open(os.path.join(ROOT,'output','revision2','master_save_record.json')))
            doc=next(x for x in app.documents if x.dataFile and x.dataFile.id==master['data_file_id'])
            doc.activate()
            assert doc.dataFile and doc.dataFile.id==master['data_file_id']
            if doc.isModified:doc.save('R2 preserved before ergonomic upper revision')
        else:
            for other in list(app.documents):
                if not other.dataFile and other.name.startswith('Ergonomic trial'):other.close(False)
            doc=app.importManager.importToNewDocument(app.importManager.createFusionArchiveImportOptions(
                os.path.join(ROOT,'output','revision2','ConceptC_Complete.f3d')))
            doc.name='Ergonomic trial - lower front palm'
        d=adsk.fusion.Design.cast(app.activeProduct); root=d.rootComponent
        root.occurrences.item(0).isLightBulbOn=True
        root.occurrences.item(2).isLightBulbOn=True
        upper=root.occurrences.item(0).component; ps=d.userParameters
        if request.get('apply'):
            assert not ps.itemByName('palm_mid_height'), 'Ergonomic revision already exists; edit its parameters instead of reapplying'
        def val(e):return d.unitsManager.evaluateExpression(e,'mm')
        for n,e,u,note in [
            ('palm_mid_height','28 mm','mm','Palm plane elevation at Y=0; lower front long edge opposite USB'),
            ('palm_slope','6 deg','deg','Gentle rise toward rear USB edge'),
            ('wrist_roll','6 mm','mm','Soft outer palm and wrist perimeter'),
            ('palm_roof','3.8 mm','mm','Roof before 0.8 mm cradle; leaves nominal 3 mm'),
            ('cradle_depth','0.8 mm','mm','Shallow palm locating depression, not a retaining lip'),
            ('cradle_radius','500 mm','mm','Broad shallow dish curvature'),
            ('cradle_x','-29 mm','mm','Pre-mirror palm cradle centre'),
            ('cradle_y','-12 mm','mm','Lower palm / hypothenar position'),
            ('mount_pad_top','64 mm','mm','Flat raised joystick surround diameter'),
            ('mount_pad_mid','82 mm','mm','Smooth mounting island blend'),
            ('mount_pad_base','90 mm','mm','Broad mount load path; clipped to organic footprint'),
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
        def contour(sk,inset):
            pts=coll([]);exps=[]
            for x,y in outline:
                ex=f'{abs(x)}*(body_width-2*({inset}))';ey=f'{abs(y)}*(body_depth-2*({inset}))'
                pts.add(P(math.copysign(val(ex),x),math.copysign(val(ey),y),0));exps.append((ex,ey))
            pts.add(pts.item(0));sp=sk.sketchCurves.sketchFittedSplines.add(pts);sp.isClosed=True
            for n,(ex,ey) in enumerate(exps):
                if n==len(exps)-1:
                    pt=sp.fitPoints.item(n);line=sk.sketchCurves.sketchLines.addByTwoPoints(sp.fitPoints.item(0),pt);line.isConstruction=True
                    sk.geometricConstraints.addVertical(line)
                    dm=sk.sketchDimensions.addDistanceDimension(sk.originPoint,pt,adsk.fusion.DimensionOrientations.VerticalDimensionOrientation,P(pt.geometry.x+.2,pt.geometry.y+.2,0));dm.parameter.expression=ey
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
        i=upper.constructionPlanes.createInput();i.setByAngle(upper.xConstructionAxis,V('palm_slope'),upper.xYConstructionPlane)
        tilted=upper.constructionPlanes.add(i);tilted.name='6 degree palm slope axis';tilted.isLightBulbOn=False
        palmplane=plane('palm_mid_height*cos(palm_slope)','Palm elevation along tilted normal',tilted)
        stage='sloped organic exterior'
        s0=sketch('cover_thickness','R3 unchanged organic footprint');p0=contour(s0,'0 mm')
        s1=sketch('12 mm','R3 lower shoulder');p1=contour(s1,'1 mm')
        s2=upper.sketches.add(palmplane);s2.name='R3 sloped palm crown';p2=contour(s2,'5 mm')
        body=loft([p0,p1,p2],NEW,'Continuous organic shell with lowered wrist entry')
        for s in (s0,s1,s2):s.isVisible=False
        top=max([f for f in body.faces if f.geometry.surfaceType==adsk.core.SurfaceTypes.PlaneSurfaceType],key=lambda f:f.centroid.z)
        body=fillet(body,list(top.edges),'wrist_roll','Rounded low wrist and hypothenar edge')
        stage='raised joystick island'
        pp=[]
        for z,dia in [('18 mm','mount_pad_base'),('34 mm','mount_pad_mid'),('palm_height','mount_pad_top')]:
            sk=sketch(z,'Joystick island '+z);pp.append(circle(sk,'joystick_x','joystick_y',dia));sk.isVisible=False
        pad=loft(pp,NEW,'Smooth raised joystick mounting island')
        top=max([f for f in pad.faces if f.geometry.surfaceType==adsk.core.SurfaceTypes.PlaneSurfaceType],key=lambda f:f.centroid.z)
        pad=fillet(pad,list(top.edges),'3 mm','Soft raised mount perimeter')
        body=combine(body,pad,JOIN,'Blend raised joystick region into lower palm roof')
        # Clip the low edge of the island to the original organic footprint.
        sk=sketch('cover_thickness','Original footprint clipping envelope');pr=contour(sk,'0 mm')
        envelope=ext(sk,pr,'70 mm',NEW,'Organic footprint envelope').bodies.item(0)
        body=combine(body,envelope,adsk.fusion.FeatureOperations.IntersectFeatureOperation,'Preserve organic footprint and existing bottom interface')
        stage='mount transition blend'
        slopes=[f for f in body.faces if f.geometry.surfaceType==adsk.core.SurfaceTypes.PlaneSurfaceType and abs(f.geometry.normal.z)>.98 and 2<f.centroid.z<3.6]
        edges=[]
        for face in slopes:
            for loop in face.loops:
                for edge in loop.edges:
                    others=[f for f in edge.faces if f!=face]
                    p=edge.pointOnEdge
                    ok,a=face.evaluator.getNormalAtPoint(p)
                    for other in others:
                        good,b=other.evaluator.getNormalAtPoint(p)
                        if ok and good and abs(a.dotProduct(b))<.9999:edges.append(edge);break
        if edges:body=fillet(body,edges,'3 mm','Smooth transition between palm and raised mount')
        stage='hollow roof'
        outer=upper.features.copyPasteBodies.add(body).bodies.item(0);outer.name='R3 exterior trimming tool';outer.isLightBulbOn=False
        bottom=min([f for f in body.faces if f.geometry.surfaceType==adsk.core.SurfaceTypes.PlaneSurfaceType],key=lambda f:f.centroid.z)
        si=upper.features.shellFeatures.createInput(coll([bottom]),True);si.insideThickness=V('palm_roof')
        sf=upper.features.shellFeatures.add(si);sf.name='Hollow ergonomic roof with cradle allowance';body=sf.bodies.item(0)
        stage='palm cradle'
        # A revolved spherical tool gives a smooth shallow cradle without loft seams.
        center_y='cradle_y*cos(palm_slope)-(cradle_radius-cradle_depth)*sin(palm_slope)'
        center_z='palm_mid_height+cradle_y*sin(palm_slope)+(cradle_radius-cradle_depth)*cos(palm_slope)'
        yn=upper.xZConstructionPlane.geometry.normal.y
        dp=plane(f'({center_y})/({yn})','Cradle sphere centre section',upper.xZConstructionPlane)
        sk=upper.sketches.add(dp);sk.name='Parametric shallow spherical palm cradle'
        origin=sk.sketchToModelSpace(P(0,0,0));xp=sk.sketchToModelSpace(P(1,0,0));yp=sk.sketchToModelSpace(P(0,1,0))
        xexpr=f'cradle_x*({xp.x-origin.x})';zexpr=f'({center_z})*({yp.z-origin.z})'
        x=val(xexpr);z=val(zexpr);r=val('cradle_radius')
        arc=sk.sketchCurves.sketchArcs.addByCenterStartSweep(P(x,z,0),P(x,z+r,0),math.pi)
        axis=sk.sketchCurves.sketchLines.addByTwoPoints(arc.endSketchPoint,arc.startSketchPoint)
        sk.geometricConstraints.addVertical(axis)
        sk.geometricConstraints.addCoincident(arc.centerSketchPoint,axis)
        distance(sk,arc.centerSketchPoint,xexpr,zexpr)
        sk.sketchDimensions.addRadialDimension(arc,P(x-r*.5,z,0)).parameter.expression='cradle_radius'
        ri=upper.features.revolveFeatures.createInput(sk.profiles.item(0),axis,NEW)
        ri.setAngleExtent(False,V('360 deg'))
        rf=upper.features.revolveFeatures.add(ri);rf.name='Spherical cradle tool';sk.isVisible=False
        sphere=rf.bodies.item(0)
        limiterplane=plane('-2 mm','Limit dish to palm contact patch',palmplane)
        ls=upper.sketches.add(limiterplane);ls.name='Palm-only dish boundary'
        pr=circle(ls,'cradle_x','cradle_y','2*sqrt(2*cradle_radius*cradle_depth-cradle_depth^2)')
        limiter=ext(ls,pr,'20 mm',NEW,'Protect raised joystick region from cradle cut').bodies.item(0)
        tool=combine(sphere,limiter,adsk.fusion.FeatureOperations.IntersectFeatureOperation,'Restrict spherical dish to palm patch')
        body=combine(body,tool,CUT,'Smooth 0.8 mm palm-only spherical cradle')
        stage='ring ledge and holes'
        # Full mount annulus replaces the old roof locally, retaining measured R2 stack.
        body=disk('flange_seat','joystick_x','joystick_y','flange_diameter+4 mm','palm_height-flange_seat',JOIN,'Reinforced measured top-access mounting ledge').bodies.item(0)
        disk('flange_seat-0.1 mm','joystick_x','joystick_y','fit_aperture','palm_height-flange_seat+0.2 mm',CUT,'Joystick through aperture',body)
        disk('palm_height+0.1 mm','joystick_x','joystick_y','ring_pocket_diameter','-ring_stack-0.1 mm',CUT,'Retain measured flush ring recess',body)
        for sx in (-1,1):
            for sy in (-1,1):disk('flange_seat-0.1 mm',f'joystick_x+({sx})*flange_pitch_x/2',f'joystick_y+({sy})*flange_pitch_y/2','fit_screw_clearance','ring_stack+mount_ledge+0.2 mm',CUT,'Confirmed top screw access',body)
        stage='existing cover bosses'
        for n,(x,y) in enumerate([('-66 mm','-20 mm'),('-60 mm','28 mm'),('74 mm','-24 mm'),('-22 mm','39 mm')]):
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
        sk=sketch('pcb_z-0.5 mm','Internal relief around rear PCB and rail ends')
        sk.sketchCurves.sketchLines.addTwoPointRectangle(P(-1.35,val('pcb_y+pcb_length/2-7 mm'),0),P(1.35,val('pcb_y+pcb_length/2+0.7 mm'),0))
        ext(sk,sk.profiles.item(0),'pcb_thickness+1.2 mm',CUT,'Clear PCB and rails behind USB mouth',body)
        stage='handedness'
        mi=upper.features.mirrorFeatures.createInput(coll([body]),upper.yZConstructionPlane)
        mi.patternComputeOption=adsk.fusion.PatternComputeOptions.IdenticalPatternCompute
        mf=upper.features.mirrorFeatures.add(mi);mf.name='Preserve user-confirmed hand layout'
        upper.features.removeFeatures.add(body);body=upper.bRepBodies.item(0)
        body.appearance=appearance;body.name='Ergonomic upper - low front / raised joystick'
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
            assert doc.save('Lower front palm support, 6 degree slope and shallow cradle; joystick seat retained')
            report['cloud_master_id']=doc.dataFile.id;report['cloud_project']=doc.dataFile.parentProject.name
            report['cloud_version']=doc.dataFile.versionNumber
            for other in list(app.documents):
                if other!=doc and not other.dataFile and other.name.startswith('Ergonomic trial'):other.close(False)
        report['complete']=True
    except Exception:report.update(error=traceback.format_exc(),stage=stage)
    with open(os.path.join(OUT,'validation_report.json'),'w') as f:json.dump(report,f,indent=2)
