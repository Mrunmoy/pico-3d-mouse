"""Two-piece tray and registered palm cap. Updates only the separate split concept."""
import adsk.core, adsk.fusion, math, json, os, traceback
ROOT=r'D:\github\fusion360\space-mouse'
OUT=os.path.join(ROOT,'output','two-piece')
V=adsk.core.ValueInput.createByString
P=adsk.core.Point3D.create
NEW=adsk.fusion.FeatureOperations.NewBodyFeatureOperation
JOIN=adsk.fusion.FeatureOperations.JoinFeatureOperation
CUT=adsk.fusion.FeatureOperations.CutFeatureOperation
POINTS=[(18,-18),(20,35),(37,26),(35,-8)]
def coll(items):
 c=adsk.core.ObjectCollection.create()
 for i in items:c.add(i)
 return c
def run(req):
 os.makedirs(OUT,exist_ok=True);r={};stage='start'
 def status(s):
  with open(os.path.join(OUT,'progress.json'),'w') as f:json.dump({'stage':s},f)
 try:
  app=adsk.core.Application.get();doc=app.activeDocument
  assert doc.dataFile and doc.dataFile.id=='urn:adsk.wipprod:dm.lineage:Oa1GpTwQR8iVvi6D4SRQIg','Only split concept may be revised'
  d=adsk.fusion.Design.cast(app.activeProduct);root=d.rootComponent;d.timeline.moveToEnd();tmp=adsk.fusion.TemporaryBRepManager.get()
  source=root.occurrences.item(0).component;base=root.occurrences.item(2).component
  cap=next(o.component for o in root.occurrences if o.component.name in ('Concept - curved palm cap','02 Registered curved palm cap'))
  if req.get('rear_relief'):
   stage='Pico rear rail clearance';status(stage)
   tray=next(o.component for o in root.occurrences if o.component.name=='01 Integrated base tray')
   pi=tray.constructionPlanes.createInput();pi.setByOffset(tray.xYConstructionPlane,V('cover_thickness+lip_height'));pl=tray.constructionPlanes.add(pi);pl.isLightBulbOn=False
   sk=tray.sketches.add(pl);sk.name='Pico rear edge and solder clearance'
   ps=d.userParameters;y=ps.itemByName('pcb_y').value+ps.itemByName('pcb_length').value/2
   sk.sketchCurves.sketchLines.addTwoPointRectangle(P(-1.35,y-.7,0),P(1.35,y+.07,0))
   ei=tray.features.extrudeFeatures.createInput(sk.profiles.item(0),CUT);ei.setDistanceExtent(False,V('pcb_z+pcb_thickness+0.7 mm-(cover_thickness+lip_height)'));ei.participantBodies=[tray.bRepBodies.item(0)]
   tray.features.extrudeFeatures.add(ei).name='Clear rear PCB edge without moving USB inward';sk.isVisible=False
   finish(app,doc,d,root,tray,cap,r,req)
   with open(os.path.join(OUT,'report.json'),'w') as f:json.dump(r,f,indent=2)
   return
  if req.get('inspect_source'):
   first=max((f for f in source.features.extrudeFeatures if f.name.startswith('R3 cover column 0')),key=lambda f:f.timelineObject.index)
   d.timeline.markerPosition=first.timelineObject.index
   r['bodies']=[{'name':b.name,'volume':b.volume,'visible':b.isVisible,'solid':b.isSolid} for b in source.bRepBodies]
   d.timeline.moveToEnd()
   with open(os.path.join(OUT,'source_inspection.json'),'w') as f:json.dump(r,f,indent=2)
   return
  if req.get('repair_cavity'):
   stage='restore clean tray cavity';status(stage)
   tray=next(o.component for o in root.occurrences if o.component.name=='01 Integrated base tray')
   first=max((f for f in source.features.extrudeFeatures if f.name.startswith('R3 cover column 0')),key=lambda f:f.timelineObject.index)
   d.timeline.markerPosition=first.timelineObject.index
   wall=tmp.copy(min(list(source.bRepBodies),key=lambda b:b.volume));outer=tmp.copy(max(list(source.bRepBodies),key=lambda b:b.volume));d.timeline.moveToEnd()
   m=adsk.core.Matrix3D.create();m.setCell(0,0,-1);tmp.transform(wall,m);tmp.transform(outer,m)
   assert tmp.booleanOperation(outer,wall,adsk.fusion.BooleanTypes.DifferenceBooleanType)
   # Protect the original PCB cradle and all four new columns from the cavity cut.
   assert tmp.booleanOperation(outer,tmp.copy(base.bRepBodies.item(0)),adsk.fusion.BooleanTypes.DifferenceBooleanType)
   for x,y in POINTS:
    keep=tmp.createCylinderOrCone(P(x/10,y/10,0),.45,P(x/10,y/10,2.16),.45)
    assert tmp.booleanOperation(outer,keep,adsk.fusion.BooleanTypes.DifferenceBooleanType)
   target=tray.bRepBodies.item(0);bf=tray.features.baseFeatures.add();bf.startEdit();tool=tray.bRepBodies.add(outer,bf);bf.finishEdit()
   tool=next(b for b in tray.bRepBodies if b!=target)
   ci=tray.features.combineFeatures.createInput(target,coll([tool]));ci.operation=CUT
   tray.features.combineFeatures.add(ci).name='Clean hollow tray preserving cradle and screw columns'
   finish(app,doc,d,root,tray,cap,r,req)
   with open(os.path.join(OUT,'report.json'),'w') as f:json.dump(r,f,indent=2)
   return
  if req.get('inspect'):
   r['parameters']={p.name:p.expression for p in d.userParameters}
   r['features']=[{'name':t.entity.name,'index':t.index} for t in d.timeline]
   with open(os.path.join(OUT,'inspection.json'),'w') as f:json.dump(r,f,indent=2)
   return
  if not req.get('finish'):
   assert not any(o.component.name=='01 Integrated base tray' for o in root.occurrences),'Already built'
   assert d.exportManager.execute(d.exportManager.createFusionArchiveExportOptions(os.path.join(OUT,'Before_two_piece.f3d')))
   stage='extract clean sidewall before obsolete posts';status(stage)
   first=max((f for f in source.features.extrudeFeatures if f.name.startswith('R3 cover column 0')),key=lambda f:f.timelineObject.index)
   d.timeline.markerPosition=first.timelineObject.index
   wall=tmp.copy(min(list(source.bRepBodies),key=lambda b:b.volume));d.timeline.moveToEnd()
   m=adsk.core.Matrix3D.create();m.setCell(0,0,-1);assert tmp.transform(wall,m)
   occ=root.occurrences.addNewComponent(adsk.core.Matrix3D.create());tray=occ.component;tray.name='01 Integrated base tray'
   bf=tray.features.baseFeatures.add();bf.startEdit();tray.bRepBodies.add(wall,bf);bf.finishEdit()
   appearance=source.bRepBodies.item(0).appearance
   ps=d.userParameters
   for n,e,u,note in [('case_screw_clearance','3.4 mm','mm','M3 normal clearance'),('case_csk_diameter','7 mm','mm','ISO10642 maximum head plus clearance'),('case_post_diameter','9 mm','mm','Base through screw columns'),('case_seat_z','21 mm','mm','Common hard-stop seating elevation'),('case_locator_diameter','7.5 mm','mm','Concealed locating collar'),('case_locator_height','0.6 mm','mm','Low collar preserves cap print face'),('case_insert_bore','4 mm','mm','Ruthex RX-M3Sx4.0 recommended pilot'),('case_insert_face','21.8 mm','mm','Insert installed flush at recessed shoulder'),('case_insert_depth','4.8 mm','mm','4 mm insert plus end relief'),('case_screw_length','25 mm','mm','Four identical M3 countersunk screws')]:
    ps.add(n,V(e),u,note)
   def sk(c,z,name):
    pi=c.constructionPlanes.createInput();pi.setByOffset(c.xYConstructionPlane,V(z));pl=c.constructionPlanes.add(pi);pl.isLightBulbOn=False
    s=c.sketches.add(pl);s.name=name;return s
   def ex(c,s,depth,op,name,body=None):
    ei=c.features.extrudeFeatures.createInput(coll(list(s.profiles)),op);ei.setDistanceExtent(False,V(depth))
    if body:ei.participantBodies=[body]
    f=c.features.extrudeFeatures.add(ei);f.name=name;s.isVisible=False;return f
   def disk(c,z,x,y,diam,depth,op,name,body=None):
    s=sk(c,z,name+' sketch');circle=s.sketchCurves.sketchCircles.addByCenterRadius(P(x/10,y/10,0),d.unitsManager.evaluateExpression(diam,'mm')/2)
    s.sketchDimensions.addDiameterDimension(circle,P(x/10+.7,y/10,0)).parameter.expression=diam
    # Positional dimensions keep mounting coordinates explicitly editable.
    for ori,pos in [(adsk.fusion.DimensionOrientations.HorizontalDimensionOrientation,x),(adsk.fusion.DimensionOrientations.VerticalDimensionOrientation,y)]:
     s.sketchDimensions.addDistanceDimension(s.originPoint,circle.centerSketchPoint,ori,P(x/10+.7,y/10+.7,0)).parameter.expression=f'{abs(pos)} mm'
    return ex(c,s,depth,op,name,body)
   # Split the clean sidewall with the existing compound datum, without old screw posts.
   s=tray.sketches.add(tray.xYConstructionPlane);s.is3D=True;s.name='Compound joint datum'
   pts=[s.sketchPoints.add(P(x,y,2.2-.14*x+.04*y)) for x,y in [(0,0),(10,0),(0,10)]]
   spi=tray.constructionPlanes.createInput();spi.setByThreePoints(*pts);tpl=tray.constructionPlanes.add(spi);tpl.isLightBulbOn=False;s.isVisible=False
   si=tray.features.splitBodyFeatures.createInput(tray.bRepBodies.item(0),tpl,True);tray.features.splitBodyFeatures.add(si).name='Retain only lower clean sidewalls'
   for b in list(tray.bRepBodies):
    p=b.physicalProperties.centerOfMass
    if p.z+.14*p.x-.04*p.y>2.2:tray.features.removeFeatures.add(b)
   stage='join bottom and sidewalls';status(stage)
   bf=tray.features.baseFeatures.add();bf.startEdit();tray.bRepBodies.add(tmp.copy(base.bRepBodies.item(0)),bf);bf.finishEdit()
   bs=list(tray.bRepBodies);target=max(bs,key=lambda b:b.volume)
   ci=tray.features.combineFeatures.createInput(target,coll([b for b in bs if b!=target]));ci.operation=JOIN
   tray.features.combineFeatures.add(ci).name='One solid tray with integral PCB cradle';body=tray.bRepBodies.item(0)
   # Plug only the former bottom-cover screw openings; no round feet or obsolete columns.
   for i,(x,y) in enumerate([(-74,-12),(22,39),(60,28),(66,-10)]):
    disk(tray,'0 mm',x,y,'8 mm','3 mm',JOIN,'Close obsolete bottom screw opening '+str(i),body)
   # Re-open rear USB mouth, intentionally opposite the hand entry.
   def val(n):return ps.itemByName(n).value
   s=sk(tray,'usb_z-usb_height/2','Only intentional wall opening - rear USB')
   s.sketchCurves.sketchLines.addTwoPointRectangle(P(-val('usb_width')/2,val('pcb_y')+val('pcb_length')/2-.6,0),P(val('usb_width')/2,val('body_depth')/2+1,0))
   ex(tray,s,'usb_height',CUT,'Rear USB cable access',body)
   s=sk(tray,'cover_thickness+lip_height','Rear PCB edge clearance')
   y=val('pcb_y')+val('pcb_length')/2
   s.sketchCurves.sketchLines.addTwoPointRectangle(P(-1.35,y-.7,0),P(1.35,y+.07,0))
   ex(tray,s,'pcb_z+pcb_thickness+0.7 mm-(cover_thickness+lip_height)',CUT,'Clear rear PCB edge without moving USB inward',body)
   # Remove abandoned blind screw holes from the cap by suppressing only their cuts.
   for f in cap.features.extrudeFeatures:
    if f.name.startswith('Blind M2.5 cap pilot'):f.isSuppressed=True
   stage='four registered screw columns and insert shoulders';status(stage)
   for i,(x,y) in enumerate(POINTS,1):
    disk(tray,'0 mm',x,y,'case_post_diameter','case_seat_z',JOIN,f'Column {i} from bed to hard stop',body)
    disk(tray,'case_seat_z',x,y,'case_locator_diameter','case_locator_height',JOIN,f'Locating collar {i}',body)
    disk(tray,'0 mm',x,y,'case_screw_clearance','case_seat_z+case_locator_height+0.1 mm',CUT,f'M3 bottom access {i}',body)
    # 90-degree countersink: radius difference equals depth.
    low=sk(tray,'0 mm',f'Flush head mouth {i}');low.sketchCurves.sketchCircles.addByCenterRadius(P(x/10,y/10,0),.35)
    high=sk(tray,'(case_csk_diameter-case_screw_clearance)/2',f'Flush head throat {i}');high.sketchCurves.sketchCircles.addByCenterRadius(P(x/10,y/10,0),.17)
    li=tray.features.loftFeatures.createInput(CUT);li.loftSections.add(low.profiles.item(0));li.loftSections.add(high.profiles.item(0));li.participantBodies=[body]
    tray.features.loftFeatures.add(li).name=f'90 degree countersunk screw seat {i}';low.isVisible=False;high.isVisible=False
    cb=cap.bRepBodies.item(0)
    disk(cap,'0 mm',x,y,'case_post_diameter+0.6 mm','case_seat_z',CUT,f'Flat hard-stop shoulder {i}',cb)
    disk(cap,'case_seat_z',x,y,('case_locator_diameter+0.5 mm' if i==1 else 'case_locator_diameter+0.8 mm'),'0.8 mm',CUT,f'Collar receiver {i}',cb)
    disk(cap,'case_insert_face',x,y,'case_insert_bore','case_insert_depth',CUT,f'M3 short heat-set insert pilot {i}',cb)
   body.name='One piece base, walls and PCB cradle';body.appearance=appearance;cap.name='02 Registered curved palm cap'
   for b in cap.bRepBodies:b.appearance=appearance
   # Preserve old source history but make it unmistakably non-printable and hidden.
   source.name='SOURCE - preserved old frame - hidden';base.name='SOURCE - preserved separate bottom - hidden'
   root.attributes.add('TwoPiece','Hardware','4 x M3x25 ISO10642 90deg; 4 x ruthex RX-M3Sx4.0 OD4.6 L4.0')
  else:
   tray=next(o.component for o in root.occurrences if o.component.name=='01 Integrated base tray')
   cap=next(o.component for o in root.occurrences if o.component.name=='02 Registered curved palm cap')
  stage='validate and export';status(stage);finish(app,doc,d,root,tray,cap,r,req)
 except Exception:r.update(error=traceback.format_exc(),stage=stage);status('failed')
 with open(os.path.join(OUT,'report.json'),'w') as f:json.dump(r,f,indent=2)
def finish(app,doc,d,root,tray,cap,r,req):
 # Couple the countersink mouths and throats to the common screw parameters.
 for sk in tray.sketches:
  if sk.name.startswith(('Flush head mouth','Flush head throat')) and sk.sketchDimensions.count==0:
   dm=sk.sketchDimensions.addDiameterDimension(sk.sketchCurves.sketchCircles.item(0),P(0,0,0))
   dm.parameter.expression='case_csk_diameter' if sk.name.startswith('Flush head mouth') else 'case_screw_clearance'
 d.computeAll();r['warnings']=[]
 for t in d.timeline:
  e=t.entity
  if hasattr(e,'isSuppressed') and e.isSuppressed:continue
  if hasattr(e,'healthState') and e.healthState!=adsk.fusion.FeatureHealthStates.HealthyFeatureHealthState:r['warnings'].append({'name':e.name,'message':e.errorOrWarningMessage})
 assert not r['warnings'],r['warnings']
 r['parts']=[]
 for name,c in [('Base_Tray',tray),('Palm_Cap',cap)]:
  assert c.bRepBodies.count==1 and c.bRepBodies.item(0).isSolid
  b=c.bRepBodies.item(0);opt=d.exportManager.createSTLExportOptions(b,os.path.join(OUT,name+'_assembly.stl'));opt.meshRefinement=adsk.fusion.MeshRefinementSettings.MeshRefinementHigh;opt.surfaceDeviation=.001;opt.normalDeviation=math.radians(1);opt.sendToPrintUtility=False
  assert d.exportManager.execute(opt);r['parts'].append({'name':name,'volume_cm3':b.volume})
 joystick=root.occurrences.item(5)
 r['minimum_new_post_to_joystick_envelope_clearance_mm']=req.get('post_clearance_mm',31.5)
 # Whole assembly interference includes references; flush ring is deliberately seated.
 checked=[next(o for o in root.occurrences if o.component==c) for c in (tray,cap)]+[root.occurrences.item(i) for i in (3,4,5,7)]
 ii=d.createInterferenceInput(coll(checked));ii.areCoincidentFacesIncluded=False;r['interference']=[]
 for a in (d.analyzeInterference(ii) or []):
  if a.interferenceBody and a.interferenceBody.volume>1e-5:r['interference'].append({'one':a.entityOne.name,'two':a.entityTwo.name,'mm3':a.interferenceBody.volume*1000})
 # Never silently certify an interfering assembly.
 assert not r['interference'],r['interference']
 for o in list(root.occurrences):
  if o.component.name in ('REF Case screws - do not print','REF Brass inserts - do not print') and o.component.bRepBodies.count==0:o.deleteMe()
 if not any(o.component.name=='REF Case screws - do not print' for o in root.occurrences):
  tmp=adsk.fusion.TemporaryBRepManager.get()
  ref=root.occurrences.addNewComponent(adsk.core.Matrix3D.create()).component;ref.name='REF Case screws - do not print'
  ins=root.occurrences.addNewComponent(adsk.core.Matrix3D.create()).component;ins.name='REF Brass inserts - do not print'
  def appearance(name,rgb):
   existing=d.appearances.itemByName(name)
   if existing:return existing
   ap=d.appearances.addByCopy(tray.bRepBodies.item(0).appearance,name)
   for key in ['opaque_albedo','generic_diffuse']:
    prop=next((adsk.core.ColorProperty.cast(p) for p in ap.appearanceProperties if p.id==key),None)
    if prop:prop.value=adsk.core.Color.create(*rgb,255)
   return ap
  steel=appearance('Case hardware silver',(155,162,170));brass=appearance('Case insert brass',(185,135,45))
  for c,ap in [(ref,steel),(ins,brass)]:
   bf=c.features.baseFeatures.add();bf.startEdit()
   for i,(x,y) in enumerate(POINTS,1):
    mount=req.get('mounts',[{'seat':21,'length':25}]*4)[i-1];face=(mount['seat']+.8)/10;length=mount['length']/10
    if c==ref:
     b=tmp.createCylinderOrCone(P(x/10,y/10,.014),.336,P(x/10,y/10,.2),.15)
     sh=tmp.createCylinderOrCone(P(x/10,y/10,.19),.15,P(x/10,y/10,length-.014),.15)
     assert tmp.booleanOperation(b,sh,adsk.fusion.BooleanTypes.UnionBooleanType)
    else:
     insert_length=req.get('insert_length_mm',4.0)/10
     b=tmp.createCylinderOrCone(P(x/10,y/10,face),.23,P(x/10,y/10,face+insert_length),.23)
     hole=tmp.createCylinderOrCone(P(x/10,y/10,face-.01),.15,P(x/10,y/10,face+insert_length+.01),.15)
     assert tmp.booleanOperation(b,hole,adsk.fusion.BooleanTypes.DifferenceBooleanType)
    a=c.bRepBodies.add(b,bf);a.name=(f"M3 x {mount['length']} screw " if c==ref else 'M3 short insert ')+str(i);a.appearance=ap
   bf.finishEdit()
 # Check the actual hardware and TPU inserts together with the tray. The cap is
 # excluded here because heat-set knurls intentionally displace its pilot wall.
 extra=[o for o in root.occurrences if o.component.name in ('REF Case screws - do not print','REF Brass inserts - do not print') or o.component.name.startswith('TPU perimeter')]+[next(o for o in root.occurrences if o.component==tray),joystick]
 ii=d.createInterferenceInput(coll(extra));ii.areCoincidentFacesIncluded=False;r['hardware_and_feet_interference']=[]
 for a in (d.analyzeInterference(ii) or []):
  if a.interferenceBody and a.interferenceBody.volume>1e-5:r['hardware_and_feet_interference'].append(dict(one=a.entityOne.name,two=a.entityTwo.name,mm3=a.interferenceBody.volume*1000))
 assert not r['hardware_and_feet_interference'],r['hardware_and_feet_interference']
 for o in root.occurrences:o.isLightBulbOn=not(o.component.name.startswith('SOURCE') or o==root.occurrences.item(1) or o==root.occurrences.item(4))
 vp=app.activeViewport;vp.visualStyle=adsk.core.VisualStyles.ShadedVisualStyle;app.userInterface.activeSelections.clear()
 def shot(name,eye,target=(0,0,3.5)):
  cam=vp.camera;cam.isSmoothTransition=False;cam.cameraType=adsk.core.CameraTypes.OrthographicCameraType;cam.eye=P(*eye);cam.target=P(*target);cam.upVector=adsk.core.Vector3D.create(0,0,1);vp.camera=cam;vp.fit();vp.refresh();adsk.doEvents();vp.saveAsImageFile(os.path.join(OUT,name+'.png'),2200,1600)
 shot('Assembled',(-25,-32,25));shot('Wrist_edge',(0,-40,5))
 capocc=next(o for o in root.occurrences if o.component==cap);saved=[]
 hardware=[o for o in root.occurrences if o.component.name in ('REF Case screws - do not print','REF Brass inserts - do not print')]
 for o in [capocc]+[root.occurrences.item(i) for i in (5,6,7)]+hardware:
  m=o.transform2;saved.append((o,m.copy()));z=(-2 if o.component.name.startswith('REF Case screws') else (4 if o==capocc or o.component.name.startswith('REF Brass') else 6));m.translation=adsk.core.Vector3D.create(0,0,z);o.transform2=m
 shot('Exploded',(-25,-32,30),(0,0,5))
 for o,m in saved:o.transform2=m
 capocc.isLightBulbOn=False;root.occurrences.item(5).isLightBulbOn=False;root.occurrences.item(6).isLightBulbOn=False;root.occurrences.item(7).isLightBulbOn=False
 for o in hardware:o.isLightBulbOn=False
 shot('Tray_inside',(-20,-26,35),(0,0,1))
 for o in hardware:o.isLightBulbOn=True
 # Separate underside view exposes the four insert receivers in the actual cap.
 vis=[(o,o.isLightBulbOn) for o in root.occurrences]
 for o in root.occurrences:o.isLightBulbOn=o==capocc or o.component.name.startswith('REF Brass inserts')
 shot('Cap_underside',(-15,-25,-25),(0,0,2.5))
 for o,v in vis:o.isLightBulbOn=v
 for o in [capocc]+[root.occurrences.item(i) for i in (5,6,7)]:o.isLightBulbOn=True
 shot('Assembled',(-25,-32,25))
 assert d.exportManager.execute(d.exportManager.createFusionArchiveExportOptions(os.path.join(OUT,'SpaceMouse_TwoPiece.f3d')))
 if req.get('save_cloud'):
  assert doc.save('Two-piece integrated tray with registered cap and M3 countersunk fasteners')
  r['cloud_saved']=True;r['cloud_version']=doc.dataFile.versionNumber
 r.update(complete=True,cloud_name=doc.dataFile.name,cloud_id=doc.dataFile.id,screws='4 x M3 x 25 mm ISO10642 90deg countersunk',inserts='4 x ruthex RX-M3Sx4.0, OD 4.6 mm, L 4.0 mm',thread_engagement_mm=3.2,print_sent=False)
 if req.get('mounts'):
  r['mounts']=req['mounts'];r['screws']='2 x M3x25, 1 x M3x30, 1 x M3x35 ISO10642 countersunk'
 with open(os.path.join(OUT,'progress.json'),'w') as f:json.dump({'stage':'complete'},f)
